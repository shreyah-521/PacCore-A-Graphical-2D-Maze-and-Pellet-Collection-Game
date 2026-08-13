#pragma once // prevents multiple inclusion of header
#include "raylib.h"

const int TILE_SIZE = 30;
const int GRID_ROWS = 21;
const int GRID_COLS = 21;
const int HUD_HEIGHT = 50;

const int SCREEN_WIDTH = GRID_COLS * TILE_SIZE + 80;
const int SCREEN_HEIGHT = GRID_ROWS * TILE_SIZE + HUD_HEIGHT + 50;

const float DEATH_ANIM_DURATION = 1.0f;

const int OFFSET_X = (SCREEN_WIDTH - (GRID_COLS * TILE_SIZE)) / 2;
const int OFFSET_Y = HUD_HEIGHT + (SCREEN_HEIGHT - HUD_HEIGHT - (GRID_ROWS * TILE_SIZE)) / 2;

Vector2 CellCenter(int gx, int gy);

int LoadHighScore();
void SaveHighScore(int hs);
