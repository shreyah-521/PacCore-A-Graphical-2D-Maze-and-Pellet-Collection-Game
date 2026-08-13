#include "core/game.h"
#include "core/maze.h"
#include "core/utils.h"
#include <cmath>
#include <ctime>

namespace
{
    const float SIREN_OVERLAP_INTERVAL = 0.4f;
}

void DrawHUD(int score, int highScore, int lives, float empCooldown, bool isEmpActive)
{
    DrawText(TextFormat("SCORE: %d", score), 30, 15, 20, WHITE);
    DrawText(TextFormat("High Score: %d", highScore), 30, 35, 20, WHITE);
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

Game::Game()
    : pacman(PACMAN_SPAWN_C, PACMAN_SPAWN_R),
      ghosts{
          Ghost(9, 8, RED, 1),
          Ghost(10, 8, PINK, 2),
          Ghost(9, 10, SKYBLUE),
          Ghost(10, 10, ORANGE)}
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pac-Man - EMP Edition");
    InitAudioDevice();
    SetMasterVolume(1.0f);
    SetTargetFPS(60);
    SetRandomSeed((unsigned int)time(NULL));

    int startXs[4] = {9, 10, 9, 10};
    int startYs[4] = {8, 8, 10, 10};
    for (int i = 0; i < 4; i++)
    {
        ghostStartX[i] = startXs[i];
        ghostStartY[i] = startYs[i];
    }

    deathSound = LoadSound("assets/audio/death.mp3");
    sirenSound = LoadSound("assets/audio/siren.mp3");
    deathSoundOk = (deathSound.frameCount > 0);
    sirenOk = (sirenSound.frameCount > 0);

    state = PLAYING;
    ResetMaze();
    pelletsLeft = CountPellets();
    dyingTimer = 0.0f;
    sirenTimer = 1.0f;

    empCooldown = 0.0f;
    isEmpActive = false;
    empTimer = 0.0f;
}

Game::~Game()
{
    if (deathSoundOk)
        UnloadSound(deathSound);
    if (sirenOk)
        UnloadSound(sirenSound);

    CloseAudioDevice();
    CloseWindow();
}

void Game::UpdatePlaying(float dt)
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

    int aheadX = pacman.GetGridX() + pacman.GetDirX();
    int aheadY = pacman.GetGridY() + pacman.GetDirY();
    int behindX = pacman.GetGridX() - pacman.GetDirX();
    int behindY = pacman.GetGridY() - pacman.GetDirY();

    for (auto &g : ghosts)
        g.Update(isEmpActive, aheadX, aheadY, behindX, behindY);

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

void Game::UpdateDying(float dt)
{
    dyingTimer += dt;
    if (dyingTimer >= DEATH_ANIM_DURATION)
    {
        if (pacman.GetLives() <= 0)
        {
            state = LOST;
        }
        else
        {
            pacman.ResetPosition(PACMAN_SPAWN_C, PACMAN_SPAWN_R);
            for (int i = 0; i < (int)ghosts.size(); i++)
                ghosts[i].ResetPosition(ghostStartX[i], ghostStartY[i]);
            state = PLAYING;
        }
    }
}

void Game::HandleReplayInput()
{
    if (IsKeyPressed(KEY_R))
    {
        ResetMaze();
        pacman.ResetGame(PACMAN_SPAWN_C, PACMAN_SPAWN_R, state);
        for (int i = 0; i < (int)ghosts.size(); i++)
            ghosts[i].ResetPosition(ghostStartX[i], ghostStartY[i]);
        pelletsLeft = CountPellets();
        empCooldown = 0.0f;
        isEmpActive = false;
        empTimer = 0.0f;
        state = PLAYING;
    }
}

void Game::DrawOverlayText() const
{
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
}

void Game::Run()
{
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (state == PLAYING)
            UpdatePlaying(dt);
        else if (state == DYING)
            UpdateDying(dt);
        else if (sirenOk)
            StopSound(sirenSound);

        if (state == LOST || state == WON)
        {
            HandleReplayInput();
            if (IsKeyPressed(KEY_Q))
                break;
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

        DrawHUD(pacman.GetScore(), pacman.GethighScore(), pacman.GetLives(), empCooldown, isEmpActive);
        DrawOverlayText();

        EndDrawing();
    }
}
