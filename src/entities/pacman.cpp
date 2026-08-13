#include "entities/pacman.h"
#include "core/maze.h"
#include "core/utils.h"
#include <cmath>

int PacMan::score = 0;
int PacMan::highScore = 0;

float PacMan::StepToward(float current, float target) const
{
    if (current < target)
        return (current + speed > target) ? target : current + speed;
    if (current > target)
        return (current - speed < target) ? target : current - speed;
    return current;
}

PacMan::PacMan(int startGridX, int startGridY)
{
    gridX = startGridX;
    gridY = startGridY;
    Vector2 c = CellCenter(gridX, gridY);
    x = c.x;
    y = c.y;
    dirX = dirY = nextDirX = nextDirY = 0;
    speed = 2.5f;
    lives = 3;
    mouthAngle = 0.0f;
    mouthSpeed = 8.0f;
    rotationAngle = 0;
    chompSound = LoadSound("assets/audio/chomp.mp3");

    highScore = LoadHighScore();
}

PacMan::~PacMan()
{
    if (chompSound.frameCount > 0)
        UnloadSound(chompSound);
}

bool PacMan::AudioLoadedOK() const { return chompSound.frameCount > 0; }

void PacMan::HandleInput()
{
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
    {
        nextDirX = 1;
        nextDirY = 0;
    }
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
    {
        nextDirX = -1;
        nextDirY = 0;
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
    {
        nextDirX = 0;
        nextDirY = -1;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
    {
        nextDirX = 0;
        nextDirY = 1;
    }
}

bool PacMan::Update()
{
    HandleInput();
    bool ateSomething = false;
    Vector2 target = CellCenter(gridX, gridY);

    if (fabs(x - target.x) < speed && fabs(y - target.y) < speed)
    {
        x = target.x;
        y = target.y;

        int &cell = maze[gridY][gridX];
        if (cell == 2 || cell == 3)
        {
            score += (cell == 3) ? 50 : 10;
            cell = 0;

            if (score > highScore)
            {
                highScore = score;
                SaveHighScore(highScore);
            }

            if (chompSound.frameCount > 0)
            {
                PlaySound(chompSound);
                ateSomething = true;
            }
        }

        if ((nextDirX != 0 || nextDirY != 0) && maze[gridY + nextDirY][gridX + nextDirX] != 1)
        {
            dirX = nextDirX;
            dirY = nextDirY;
        }

        if (maze[gridY + dirY][gridX + dirX] == 1)
        {
            dirX = dirY = 0;
        }
        else
        {
            gridX += dirX;
            gridY += dirY;
        }
    }

    if (dirX == 0 && dirY == 0 && chompSound.frameCount > 0)
        StopSound(chompSound);

    if (dirX != 0)
        rotationAngle = (dirX == 1) ? 0 : 180;
    if (dirY != 0)
        rotationAngle = (dirY == -1) ? 270 : 90;

    if (dirX != 0 || dirY != 0)
    {
        mouthAngle += mouthSpeed;
        if (mouthAngle >= 45.0f || mouthAngle <= 0.0f)
            mouthSpeed = -mouthSpeed;
    }
    else
    {
        mouthAngle = 20.0f;
    }

    target = CellCenter(gridX, gridY);
    x = StepToward(x, target.x);
    y = StepToward(y, target.y);

    return ateSomething;
}

void PacMan::StopAudio()
{
    if (chompSound.frameCount > 0)
        StopSound(chompSound);
}

void PacMan::Draw() const
{
    Vector2 center = {x, y};
    float radius = (float)TILE_SIZE / 2.0f - 2.0f;
    DrawCircleSector(center, radius, rotationAngle + mouthAngle, rotationAngle + (360.0f - mouthAngle), 32, YELLOW);

    float eyeAngleOffset = (rotationAngle == 180) ? 60.0f : -60.0f;
    float eyeAngleRad = (rotationAngle + eyeAngleOffset) * DEG2RAD;
    Vector2 eyeCenter = {x + cosf(eyeAngleRad) * (radius * 0.45f), y + sinf(eyeAngleRad) * (radius * 0.45f)};
    DrawCircleV(eyeCenter, radius * 0.18f, BLACK);
}

void PacMan::DrawDying(float t) const
{
    float radius = ((float)TILE_SIZE / 2.0f - 2.0f) * (1.0f - t);
    float mouth = fminf(179.0f, 20.0f + t * 160.0f);
    DrawCircleSector({x, y}, radius, (t * 720.0f) + mouth, (t * 720.0f) + (360.0f - mouth), 32, YELLOW);
}

void PacMan::LoseLife() { lives--; }

void PacMan::ResetPosition(int gx, int gy)
{
    gridX = gx;
    gridY = gy;
    Vector2 c = CellCenter(gridX, gridY);
    x = c.x;
    y = c.y;
    dirX = dirY = nextDirX = nextDirY = 0;
    mouthAngle = 0.0f;
}

void PacMan::ResetGame(int startGridX, int startGridY, GameState state)
{
    if (state == LOST || lives <= 0)
    {
        score = 0;
        lives = 3;
    }
    ResetPosition(startGridX, startGridY);
    StopAudio();
    mouthAngle = 0.0f;
}

Vector2 PacMan::GetPosition() const { return {x, y}; }
int PacMan::GetGridX() const { return gridX; }
int PacMan::GetGridY() const { return gridY; }
int PacMan::GetDirX() const { return dirX; }
int PacMan::GetDirY() const { return dirY; }
int PacMan::GetScore() const { return score; }
int PacMan::GethighScore() const { return highScore; }
int PacMan::GetLives() const { return lives; }
