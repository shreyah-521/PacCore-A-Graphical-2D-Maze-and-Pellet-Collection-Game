#include "raylib.h"
#include <cmath>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>

const int TILE_SIZE = 30;
const int GRID_ROWS = 21;
const int GRID_COLS = 21;
const int HUD_HEIGHT = 50;

const int SCREEN_WIDTH = GRID_COLS * TILE_SIZE + 80;
const int SCREEN_HEIGHT = GRID_ROWS * TILE_SIZE + HUD_HEIGHT + 50;

const float DEATH_ANIM_DURATION = 1.0f;
const int OFFSET_X = (SCREEN_WIDTH - (GRID_COLS * TILE_SIZE)) / 2;
const int OFFSET_Y = HUD_HEIGHT + (SCREEN_HEIGHT - HUD_HEIGHT - (GRID_ROWS * TILE_SIZE)) / 2;

int maze[GRID_ROWS][GRID_COLS];

enum GameState
{
    PLAYING,
    DYING,
    WON,
    LOST
};

Vector2 CellCenter(int gx, int gy)
{
    return {OFFSET_X + gx * TILE_SIZE + TILE_SIZE / 2.0f,
            OFFSET_Y + gy * TILE_SIZE + TILE_SIZE / 2.0f};
}

int CountPellets()
{
    int count = 0;
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++)
            if (maze[r][c] == 2 || maze[r][c] == 3)
                count++;
    return count;
}

// ---------- Procedural maze generation ----------

struct GridPoint
{
    int r, c;
};

// Cells that must always stay open for PacMan/ghost spawns
static const int GHOST_HOUSE_R0 = 7, GHOST_HOUSE_R1 = 11;
static const int GHOST_HOUSE_C0 = 8, GHOST_HOUSE_C1 = 11;
static const int PACMAN_SPAWN_R = 16, PACMAN_SPAWN_C = 9;

// Carves a random spanning-tree maze (recursive backtracker)
static void CarveSpanningTree()
{
    static bool visited[GRID_ROWS][GRID_COLS];
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++)
        {
            maze[r][c] = 1;
            visited[r][c] = false;
        }

    std::vector<GridPoint> stack;
    maze[1][1] = 0;
    visited[1][1] = true;
    stack.push_back({1, 1});

    while (!stack.empty())
    {
        GridPoint cur = stack.back();

        int dirs[4][2] = {{-2, 0}, {2, 0}, {0, -2}, {0, 2}};
        for (int i = 3; i > 0; i--)
        {
            int j = GetRandomValue(0, i);
            int tmp0 = dirs[i][0], tmp1 = dirs[i][1];
            dirs[i][0] = dirs[j][0];
            dirs[i][1] = dirs[j][1];
            dirs[j][0] = tmp0;
            dirs[j][1] = tmp1;
        }

        bool moved = false;
        for (int i = 0; i < 4; i++)
        {
            int nr = cur.r + dirs[i][0];
            int nc = cur.c + dirs[i][1];
            if (nr > 0 && nr < GRID_ROWS - 1 && nc > 0 && nc < GRID_COLS - 1 && !visited[nr][nc])
            {
                maze[cur.r + dirs[i][0] / 2][cur.c + dirs[i][1] / 2] = 0;
                maze[nr][nc] = 0;
                visited[nr][nc] = true;
                stack.push_back({nr, nc});
                moved = true;
                break;
            }
        }
        if (!moved)
            stack.pop_back();
    }
}

// Knocks down some extra walls so the maze has loops, not just one path
static void AddLoops()
{
    for (int r = 1; r < GRID_ROWS - 1; r++)
    {
        for (int c = 1; c < GRID_COLS - 1; c++)
        {
            if (maze[r][c] != 1)
                continue;
            bool horizOpen = (maze[r][c - 1] == 0 && maze[r][c + 1] == 0);
            bool vertOpen = (maze[r - 1][c] == 0 && maze[r + 1][c] == 0);
            if ((horizOpen || vertOpen) && GetRandomValue(0, 99) < 20)
                maze[r][c] = 0;
        }
    }
}

// Flood-fills from spawn and connects any isolated pockets
static void EnsureConnected()
{
    static bool reached[GRID_ROWS][GRID_COLS];
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++)
            reached[r][c] = false;

    std::vector<GridPoint> stack;
    stack.push_back({PACMAN_SPAWN_R, PACMAN_SPAWN_C});
    reached[PACMAN_SPAWN_R][PACMAN_SPAWN_C] = true;

    while (!stack.empty())
    {
        GridPoint cur = stack.back();
        stack.pop_back();
        int dr[4] = {-1, 1, 0, 0};
        int dc[4] = {0, 0, -1, 1};
        for (int i = 0; i < 4; i++)
        {
            int nr = cur.r + dr[i], nc = cur.c + dc[i];
            if (nr >= 0 && nr < GRID_ROWS && nc >= 0 && nc < GRID_COLS &&
                maze[nr][nc] != 1 && !reached[nr][nc])
            {
                reached[nr][nc] = true;
                stack.push_back({nr, nc});
            }
        }
    }

    for (int r = 1; r < GRID_ROWS - 1; r++)
    {
        for (int c = 1; c < GRID_COLS - 1; c++)
        {
            if (maze[r][c] == 1 || reached[r][c])
                continue;

            int bestR = PACMAN_SPAWN_R, bestC = PACMAN_SPAWN_C, bestDist = 1 << 30;
            for (int rr = 0; rr < GRID_ROWS; rr++)
                for (int cc = 0; cc < GRID_COLS; cc++)
                    if (reached[rr][cc])
                    {
                        int d = abs(rr - r) + abs(cc - c);
                        if (d < bestDist)
                        {
                            bestDist = d;
                            bestR = rr;
                            bestC = cc;
                        }
                    }

            int rr = r, cc = c;
            while (cc != bestC)
            {
                cc += (bestC > cc) ? 1 : -1;
                maze[rr][cc] = 0;
                reached[rr][cc] = true;
            }
            while (rr != bestR)
            {
                rr += (bestR > rr) ? 1 : -1;
                maze[rr][cc] = 0;
                reached[rr][cc] = true;
            }
            reached[r][c] = true;
        }
    }
}

// Drops a power pellet near each of the four corners
static void PlacePowerPellets()
{
    int targets[4][2] = {
        {1, 1}, {1, GRID_COLS - 2}, {GRID_ROWS - 2, 1}, {GRID_ROWS - 2, GRID_COLS - 2}};

    for (int t = 0; t < 4; t++)
    {
        int tr = targets[t][0], tc = targets[t][1];
        int bestR = -1, bestC = -1, bestDist = 1 << 30;
        for (int r = 0; r < GRID_ROWS; r++)
            for (int c = 0; c < GRID_COLS; c++)
                if (maze[r][c] == 2)
                {
                    int d = abs(r - tr) + abs(c - tc);
                    if (d < bestDist)
                    {
                        bestDist = d;
                        bestR = r;
                        bestC = c;
                    }
                }
        if (bestR != -1)
            maze[bestR][bestC] = 3;
    }
}

void ResetMaze()
{
    CarveSpanningTree();
    AddLoops();

    // stamp ghost house + spawn tile open
    for (int r = GHOST_HOUSE_R0; r <= GHOST_HOUSE_R1; r++)
        for (int c = GHOST_HOUSE_C0; c <= GHOST_HOUSE_C1; c++)
            maze[r][c] = 0;

    maze[PACMAN_SPAWN_R][PACMAN_SPAWN_C] = 0;

    EnsureConnected();

    // fill open floor with pellets, except ghost house and spawn tile
    for (int r = 1; r < GRID_ROWS - 1; r++)
    {
        for (int c = 1; c < GRID_COLS - 1; c++)
        {
            if (maze[r][c] != 0)
                continue;
            bool inGhostHouse = (r >= GHOST_HOUSE_R0 && r <= GHOST_HOUSE_R1 &&
                                 c >= GHOST_HOUSE_C0 && c <= GHOST_HOUSE_C1);
            bool isPacmanSpawn = (r == PACMAN_SPAWN_R && c == PACMAN_SPAWN_C);
            if (!inGhostHouse && !isPacmanSpawn)
                maze[r][c] = 2;
        }
    }

    PlacePowerPellets();
}

class PacMan
{
private:
    float x, y;
    int gridX, gridY;
    int dirX, dirY;
    int nextDirX, nextDirY;
    float speed;
    int score;
    int lives;

    float mouthAngle;
    float mouthSpeed;
    int rotationAngle;
    Sound chompSound;

    float StepToward(float current, float target) const
    {
        if (current < target)
            return (current + speed > target) ? target : current + speed;
        if (current > target)
            return (current - speed < target) ? target : current - speed;
        return current;
    }

public:
    PacMan(int startGridX, int startGridY)
    {
        gridX = startGridX;
        gridY = startGridY;
        Vector2 c = CellCenter(gridX, gridY);
        x = c.x;
        y = c.y;
        dirX = dirY = nextDirX = nextDirY = 0;
        speed = 2.5f;
        score = 0;
        lives = 3;
        mouthAngle = 0.0f;
        mouthSpeed = 8.0f;
        rotationAngle = 0;
        chompSound = LoadSound("chomp.mp3");
    }

    bool AudioLoadedOK() const { return chompSound.frameCount > 0; }
    ~PacMan()
    {
        if (chompSound.frameCount > 0)
            UnloadSound(chompSound);
    }

    void HandleInput()
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

    bool Update()
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

    void StopAudio()
    {
        if (chompSound.frameCount > 0)
            StopSound(chompSound);
    }

    void Draw() const
    {
        Vector2 center = {x, y};
        float radius = (float)TILE_SIZE / 2.0f - 2.0f;
        DrawCircleSector(center, radius, rotationAngle + mouthAngle, rotationAngle + (360.0f - mouthAngle), 32, YELLOW);

        float eyeAngleOffset = (rotationAngle == 180) ? 60.0f : -60.0f;
        float eyeAngleRad = (rotationAngle + eyeAngleOffset) * DEG2RAD;
        Vector2 eyeCenter = {x + cosf(eyeAngleRad) * (radius * 0.45f), y + sinf(eyeAngleRad) * (radius * 0.45f)};
        DrawCircleV(eyeCenter, radius * 0.18f, BLACK);
    }

    void DrawDying(float t) const
    {
        float radius = ((float)TILE_SIZE / 2.0f - 2.0f) * (1.0f - t);
        float mouth = fminf(179.0f, 20.0f + t * 160.0f);
        DrawCircleSector({x, y}, radius, (t * 720.0f) + mouth, (t * 720.0f) + (360.0f - mouth), 32, YELLOW);
    }

    void LoseLife() { lives--; }
    void ResetPosition(int gx, int gy)
    {
        gridX = gx;
        gridY = gy;
        Vector2 c = CellCenter(gridX, gridY);
        x = c.x;
        y = c.y;
        dirX = dirY = nextDirX = nextDirY = 0;
        mouthAngle = 0.0f;
    }

    void ResetGame(int startGridX, int startGridY)
    {
        score = 0;
        lives = 3;
        ResetPosition(startGridX, startGridY);
        StopAudio();
        mouthAngle = 0.0f;
    }

    Vector2 GetPosition() const { return {x, y}; }
    int GetScore() const { return score; }
    int GetLives() const { return lives; }
};

class Ghost
{
private:
    float x, y;
    int gridX, gridY;
    int dirX, dirY;
    int startGridX, startGridY;
    float speed;
    Color color;
    float bobPhase;

    float StepToward(float current, float target) const
    {
        if (current < target)
            return (current + speed > target) ? target : current + speed;
        if (current > target)
            return (current - speed < target) ? target : current - speed;
        return current;
    }

public:
    Ghost(int startGX, int startGY, Color c)
    {
        startGridX = startGX;
        startGridY = startGY;
        color = c;
        speed = 1.8f;
        bobPhase = (float)GetRandomValue(0, 628) / 100.0f;
        ResetPosition(startGridX, startGridY);
    }

    void ResetPosition(int gx, int gy)
    {
        gridX = gx;
        gridY = gy;
        Vector2 c = CellCenter(gridX, gridY);
        x = c.x;
        y = c.y;
        dirX = 0;
        dirY = -1;
    }

    void Update(bool isStunned)
    {
        if (isStunned)
            return;

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
            else
            {
                int pick = GetRandomValue(0, count - 1);
                dirX = options[pick][0];
                dirY = options[pick][1];
            }
            gridX += dirX;
            gridY += dirY;
        }

        target = CellCenter(gridX, gridY);
        x = StepToward(x, target.x);
        y = StepToward(y, target.y);
    }

    void Draw(bool isStunned) const
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

    Vector2 GetPosition() const { return {x, y}; }
};

void DrawMaze()
{
    bool blinkOn = ((int)(GetTime() * 4.0f) % 2) == 0;
    for (int r = 0; r < GRID_ROWS; r++)
    {
        for (int c = 0; c < GRID_COLS; c++)
        {
            int posX = OFFSET_X + c * TILE_SIZE;
            int posY = OFFSET_Y + r * TILE_SIZE;
            if (maze[r][c] == 1)
            {
                DrawRectangle(posX, posY, TILE_SIZE, TILE_SIZE, DARKBLUE);
                DrawRectangleLines(posX, posY, TILE_SIZE, TILE_SIZE, BLUE);
            }
            else if (maze[r][c] == 2)
            {
                DrawCircle(posX + TILE_SIZE / 2, posY + TILE_SIZE / 2, 3, GOLD);
            }
            else if (maze[r][c] == 3 && blinkOn)
            {
                DrawCircle(posX + TILE_SIZE / 2, posY + TILE_SIZE / 2, 6, GOLD);
            }
        }
    }
}

void DrawHUD(int score, int lives, float empCooldown, bool isEmpActive)
{
    DrawText(TextFormat("SCORE: %d", score), 30, 15, 20, WHITE);
    DrawText(TextFormat("LIVES: %d", lives), SCREEN_WIDTH - 140, 15, 20, WHITE);

    int barX = 220, barY = 15, barW = 200, barH = 20;
    DrawRectangleLines(barX, barY, barW, barH, SKYBLUE);

    if (isEmpActive)
    {
        DrawRectangle(barX + 2, barY + 2, barW - 4, barH - 4, GOLD);
        DrawText("EMP ACTIVE!", barX + 45, barY + 3, 14, BLACK);
    }
    else if (empCooldown <= 0.0f)
    {
        DrawRectangle(barX + 2, barY + 2, barW - 4, barH - 4, SKYBLUE);
        DrawText("EMP READY [SPACE]", barX + 20, barY + 3, 14, BLACK);
    }
    else
    {
        float fillRatio = 1.0f - (empCooldown / 10.0f);
        DrawRectangle(barX + 2, barY + 2, (int)((barW - 4) * fillRatio), barH - 4, DARKGRAY);
        DrawText("EMP CHARGING", barX + 40, barY + 3, 14, RAYWHITE);
    }
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pac-Man - EMP Edition");
    InitAudioDevice();
    SetMasterVolume(1.0f);
    SetTargetFPS(60);
    SetRandomSeed((unsigned int)time(NULL));

    const int PACMAN_START_X = 9, PACMAN_START_Y = 16;
    PacMan pacman(PACMAN_START_X, PACMAN_START_Y);

    std::vector<Ghost> ghosts = {
        Ghost(9, 8, RED), Ghost(10, 8, PINK), Ghost(9, 10, SKYBLUE), Ghost(10, 10, ORANGE)};
    const int ghostStartX[4] = {9, 10, 9, 10};
    const int ghostStartY[4] = {8, 8, 10, 10};

    Sound deathSound = LoadSound("death.mp3");
    Sound sirenSound = LoadSound("siren.mp3");

    bool deathSoundOk = (deathSound.frameCount > 0);
    bool sirenOk = (sirenSound.frameCount > 0);

    GameState state = PLAYING;
    ResetMaze();
    int pelletsLeft = CountPellets();
    float dyingTimer = 0.0f;
    float sirenTimer = 1.0f;

    float empCooldown = 0.0f;
    bool isEmpActive = false;
    float empTimer = 0.0f;
    const float SIREN_OVERLAP_INTERVAL = 0.4f;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (state == PLAYING)
        {
            if (sirenOk)
            {
                sirenTimer += dt;
                if (sirenTimer >= SIREN_OVERLAP_INTERVAL)
                {
                    PlaySound(sirenSound);
                    sirenTimer = 0.0f;
                }
            }

            if (empCooldown > 0.0f)
                empCooldown -= dt;
            if (isEmpActive)
            {
                empTimer -= dt;
                if (empTimer <= 0.0f)
                    isEmpActive = false;
            }

            if (IsKeyPressed(KEY_SPACE) && empCooldown <= 0.0f && !isEmpActive)
            {
                isEmpActive = true;
                empTimer = 3.0f;
                empCooldown = 10.0f;
            }

            pacman.Update();
            pelletsLeft = CountPellets();
            for (auto &g : ghosts)
                g.Update(isEmpActive);

            if (pelletsLeft <= 0)
                state = WON;

            if (state == PLAYING && !isEmpActive)
            {
                Vector2 pp = pacman.GetPosition();
                for (auto &g : ghosts)
                {
                    Vector2 gp = g.GetPosition();
                    float dist = sqrtf((pp.x - gp.x) * (pp.x - gp.x) + (pp.y - gp.y) * (pp.y - gp.y));
                    if (dist < TILE_SIZE * 0.6f)
                    {
                        pacman.LoseLife();
                        pacman.StopAudio();
                        if (deathSoundOk)
                            PlaySound(deathSound);
                        if (sirenOk)
                            StopSound(sirenSound);

                        state = DYING;
                        dyingTimer = 0.0f;
                        break;
                    }
                }
            }
        }
        else if (state == DYING)
        {
            dyingTimer += dt;
            if (dyingTimer >= DEATH_ANIM_DURATION)
            {
                if (pacman.GetLives() <= 0)
                {
                    if (CountPellets() <= 0)
                        state = WON;
                    else
                        state = LOST;
                }
                else
                {
                    pacman.ResetPosition(PACMAN_START_X, PACMAN_START_Y);
                    for (int i = 0; i < (int)ghosts.size(); i++)
                        ghosts[i].ResetPosition(ghostStartX[i], ghostStartY[i]);
                    state = PLAYING;
                }
            }
        }
        else
        {
            if (sirenOk)
                StopSound(sirenSound);
        }

        if (state == LOST || state == WON)
        {
            if (IsKeyPressed(KEY_R))
            {
                ResetMaze();
                pacman.ResetGame(PACMAN_START_X, PACMAN_START_Y);
                for (int i = 0; i < (int)ghosts.size(); i++)
                    ghosts[i].ResetPosition(ghostStartX[i], ghostStartY[i]);
                pelletsLeft = CountPellets();
                empCooldown = 0.0f;
                isEmpActive = false;
                empTimer = 0.0f;
                state = PLAYING;
            }
            if (IsKeyPressed(KEY_Q))
            {
                break;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        DrawMaze();

        if (state == DYING)
        {
            pacman.DrawDying(dyingTimer / DEATH_ANIM_DURATION);
        }
        else
        {
            pacman.Draw();
            for (const auto &g : ghosts)
                g.Draw(isEmpActive);
        }

        DrawHUD(pacman.GetScore(), pacman.GetLives(), empCooldown, isEmpActive);

        if (state == WON)
        {
            const char *winText = "YOU WIN!";
            const char *replayTxt = "PRESS R TO REPLAY";
            const char *exitTxt = "PRESS Q TO EXIT";
            int winW = MeasureText(winText, 40);
            int rpW = MeasureText(replayTxt, 20);
            int exW = MeasureText(exitTxt, 20);
            DrawText(winText, SCREEN_WIDTH / 2 - winW / 2, SCREEN_HEIGHT / 2, 40, GREEN);
            DrawText(replayTxt, SCREEN_WIDTH / 2 - rpW / 2, SCREEN_HEIGHT / 2 + 40, 20, WHITE);
            DrawText(exitTxt, SCREEN_WIDTH / 2 - exW / 2, SCREEN_HEIGHT / 2 + 70, 20, WHITE);
        }
        if (state == LOST)
        {
            const char *gameOver = "GAME OVER";
            const char *replayTxt = "PRESS R TO REPLAY";
            const char *exitTxt = "PRESS Q TO EXIT";
            int goW = MeasureText(gameOver, 40);
            int rpW = MeasureText(replayTxt, 20);
            int exW = MeasureText(exitTxt, 20);
            DrawText(gameOver, SCREEN_WIDTH / 2 - goW / 2, SCREEN_HEIGHT / 2, 40, RED);
            DrawText(replayTxt, SCREEN_WIDTH / 2 - rpW / 2, SCREEN_HEIGHT / 2 + 40, 20, WHITE);
            DrawText(exitTxt, SCREEN_WIDTH / 2 - exW / 2, SCREEN_HEIGHT / 2 + 70, 20, WHITE);
        }

        EndDrawing();
    }

    if (deathSoundOk)
        UnloadSound(deathSound);
    if (sirenOk)
        UnloadSound(sirenSound);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}