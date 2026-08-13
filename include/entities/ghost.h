#pragma once
#include "raylib.h"

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
    int behavior;

    float StepToward(float current, float target) const;

public:
    Ghost(int startGX, int startGY, Color c, int behaviorMode = 0);

    void ResetPosition(int gx, int gy);
    void Update(bool isStunned, int aheadX, int aheadY, int behindX, int behindY);
    void Draw(bool isStunned) const;

    Vector2 GetPosition() const;
};
