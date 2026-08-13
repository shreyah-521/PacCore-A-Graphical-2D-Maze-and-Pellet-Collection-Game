#include "core/utils.h"
#include <fstream>

Vector2 CellCenter(int gx, int gy)
{
    return {OFFSET_X + gx * TILE_SIZE + TILE_SIZE / 2.0f,
            OFFSET_Y + gy * TILE_SIZE + TILE_SIZE / 2.0f};
}

int LoadHighScore()
{
    int hs = 0;
    std::ifstream file("assets/data/highscore.txt");
    if (file.is_open())
    {
        file >> hs;
        file.close();
    }
    return hs;
}

void SaveHighScore(int hs)
{
    std::ofstream file("assets/data/highscore.txt");
    if (file.is_open())
    {
        file << hs;
        file.close();
    }
}
