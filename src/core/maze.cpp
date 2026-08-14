#include "core/maze.h"
#include "raylib.h"
#include <vector>
#include <cmath>

using std::vector;

int maze[GRID_ROWS][GRID_COLS];

const int GHOST_HOUSE_R0 = 7, GHOST_HOUSE_R1 = 11;
const int GHOST_HOUSE_C0 = 8, GHOST_HOUSE_C1 = 11;
const int PACMAN_SPAWN_R = 16, PACMAN_SPAWN_C = 9;

namespace
{
    struct GridPoint
    {
        int r, c;
    };

    void CarveSpanningTree()
    {
        static bool visited[GRID_ROWS][GRID_COLS];
        for (int r = 0; r < GRID_ROWS; r++)
            for (int c = 0; c < GRID_COLS; c++)
            {
                maze[r][c] = 1;
                visited[r][c] = false;
            }

        vector<GridPoint> stack;
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

    void AddLoops()
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

    void EnsureConnected()
    {
        static bool reached[GRID_ROWS][GRID_COLS];
        for (int r = 0; r < GRID_ROWS; r++)
            for (int c = 0; c < GRID_COLS; c++)
                reached[r][c] = false;

        vector<GridPoint> stack;
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

    void PlacePowerPellets()
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
}

void ResetMaze()
{
    CarveSpanningTree();
    AddLoops();

    for (int r = GHOST_HOUSE_R0; r <= GHOST_HOUSE_R1; r++)
        for (int c = GHOST_HOUSE_C0; c <= GHOST_HOUSE_C1; c++)
            maze[r][c] = 0;

    maze[PACMAN_SPAWN_R][PACMAN_SPAWN_C] = 0;

    EnsureConnected();

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

int CountPellets()
{
    int count = 0;
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++)
            if (maze[r][c] == 2 || maze[r][c] == 3)
                count++;
    return count;
}

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
