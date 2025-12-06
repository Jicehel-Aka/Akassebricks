#pragma once
struct Ball {
    float x, y;
    float vx, vy;
    int size = 4;
    bool active = true;
};
void ball_update(Ball& b);
