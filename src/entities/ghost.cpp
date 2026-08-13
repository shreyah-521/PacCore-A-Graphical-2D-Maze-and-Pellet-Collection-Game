#include "entities/ghost.h"
#include "core/maze.h"
#include "core/utils.h"
#include <cmath>

float Ghost::StepToward(float current, float target) const
{
    if (current < target)
        return (current + speed > target) ? target : current + speed;
    if (current > target)
        return (current - speed < target) ? target : current - speed;
    return current;
}

Ghost::Ghost(int startGX, int startGY, Color c, int behaviorMode)
{
    startGridX = startGX;
    startGridY = startGY;
    color = c;
    speed = 1.8f;
    behavior = behaviorMode;
    bobPhase = (float)GetRandomValue(0, 628) / 100.0f;
    ResetPosition(startGridX, startGridY);
}

void Ghost::ResetPosition(int gx, int gy)
{
    gridX = gx;
    gridY = gy;
    Vector2 c = CellCenter(gridX, gridY);
    x = c.x;
    y = c.y;
    dirX = 0;
    dirY = -1;
}

void Ghost::Update(bool isStunned, int aheadX, int aheadY, int behindX, int behindY)
{
    if (isStunned)
        return;

    int targetX = (behavior == 1) ? aheadX : behindX;
    int targetY = (behavior == 1) ? aheadY : behindY;

    Vector2 target = CellCenter(gridX, gridY);
    if (x == target.x && y == target.y)
    {
        const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        int options[4][2];
        int count = 0;

        for (const auto &d : dirs)
        {
            int nx = gridX + d[0], ny = gridY + d[1];
            bool isReverse = (d[0] == -dirX && d[1] == -dirY);
            if (nx >= 0 && nx < GRID_COLS && ny >= 0 && ny < GRID_ROWS)
            {
                if (maze[ny][nx] != 1 && !isReverse)
                {
                    options[count][0] = d[0];
                    options[count][1] = d[1];
                    count++;
                }
            }
        }

        if (count == 0)
        {
            dirX = -dirX;
            dirY = -dirY;
        }
        else if (behavior == 0)
        {
            int pick = GetRandomValue(0, count - 1);
            dirX = options[pick][0];
            dirY = options[pick][1];
        }
        else
        {
            int bestPick = 0, bestDist = 1 << 30;
            for (int i = 0; i < count; i++)
            {
                int nx = gridX + options[i][0];
                int ny = gridY + options[i][1];
                int d = abs(nx - targetX) + abs(ny - targetY);
                if (d < bestDist)
                {
                    bestDist = d;
                    bestPick = i;
                }
            }
            dirX = options[bestPick][0];
            dirY = options[bestPick][1];
        }
        gridX += dirX;
        gridY += dirY;
    }

    target = CellCenter(gridX, gridY);
    x = StepToward(x, target.x);
    y = StepToward(y, target.y);
}

void Ghost::Draw(bool isStunned) const
{
    float r = TILE_SIZE / 2.0f - 2.0f;
    float bob = sinf((float)GetTime() * 6.0f + bobPhase) * 1.5f;
    float cy = y + bob;

    Color drawColor = isStunned ? ((int)(GetTime() * 10) % 2 == 0 ? SKYBLUE : DARKBLUE) : color;

    DrawCircleSector({x, cy}, r, 180.0f, 360.0f, 16, drawColor);
    DrawRectangle((int)(x - r), (int)cy, (int)(r * 2), (int)(r * 0.85f), drawColor);

    const int scallops = 4;
    float top = cy + r * 0.85f;
    float scallopW = (r * 2.0f) / scallops;
    for (int i = 0; i < scallops; i++)
    {
        Vector2 p1 = {x - r + i * scallopW, top};
        Vector2 p2 = {x - r + (i + 0.5f) * scallopW, top + r * 0.4f};
        Vector2 p3 = {x - r + (i + 1) * scallopW, top};
        DrawTriangle(p1, p3, p2, drawColor);
    }

    Vector2 leftEye = {x - 4.0f, cy - 3.0f};
    Vector2 rightEye = {x + 4.0f, cy - 3.0f};
    DrawCircleV(leftEye, 3.5f, WHITE);
    DrawCircleV(rightEye, 3.5f, WHITE);
    DrawCircleV({leftEye.x + dirX * 1.5f, leftEye.y + dirY * 1.5f}, 1.5f, DARKBLUE);
    DrawCircleV({rightEye.x + dirX * 1.5f, rightEye.y + dirY * 1.5f}, 1.5f, DARKBLUE);
}

Vector2 Ghost::GetPosition() const { return {x, y}; }
