#pragma once
#include "core/utils.h"

extern int maze[GRID_ROWS][GRID_COLS];

extern const int GHOST_HOUSE_R0, GHOST_HOUSE_R1;
extern const int GHOST_HOUSE_C0, GHOST_HOUSE_C1;
extern const int PACMAN_SPAWN_R, PACMAN_SPAWN_C;

void ResetMaze();
int CountPellets();
void DrawMaze();
