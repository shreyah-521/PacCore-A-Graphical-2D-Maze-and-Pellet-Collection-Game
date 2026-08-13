#pragma once
#include "raylib.h"
#include "core/gamestate.h"

class PacMan
{
private:
    float x, y;
    int gridX, gridY;
    int dirX, dirY;
    int nextDirX, nextDirY;
    float speed;
    static int score;
    static int highScore;
    int lives;

    float mouthAngle;
    float mouthSpeed;
    int rotationAngle;
    Sound chompSound;

    float StepToward(float current, float target) const;

public:
    PacMan(int startGridX, int startGridY);
    ~PacMan();

    bool AudioLoadedOK() const;

    void HandleInput();
    bool Update();
    void StopAudio();

    void Draw() const;
    void DrawDying(float t) const;

    void LoseLife();
    void ResetPosition(int gx, int gy);
    void ResetGame(int startGridX, int startGridY, GameState state);

    Vector2 GetPosition() const;
    int GetGridX() const;
    int GetGridY() const;
    int GetDirX() const;
    int GetDirY() const;
    int GetScore() const;
    int GethighScore() const;
    int GetLives() const;
};
