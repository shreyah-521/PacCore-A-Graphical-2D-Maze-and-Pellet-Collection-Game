#pragma once
#include "raylib.h"
#include <vector>
#include "core/gamestate.h"
#include "entities/pacman.h"
#include "entities/ghost.h"

class Game
{
private:
    GameState state;

    PacMan pacman;
    std::vector<Ghost> ghosts;
    int ghostStartX[4];
    int ghostStartY[4];

    Sound deathSound;
    Sound sirenSound;
    bool deathSoundOk;
    bool sirenOk;

    int pelletsLeft;
    float dyingTimer;
    float sirenTimer;

    float empCooldown;
    bool isEmpActive;
    float empTimer;

    void UpdatePlaying(float dt);
    void UpdateDying(float dt);
    void HandleReplayInput();

    void DrawOverlayText() const;

public:
    Game();
    ~Game();

    void Run();
};

void DrawHUD(int score, int highScore, int lives, float empCooldown, bool isEmpActive);
