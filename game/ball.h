#pragma once
#include "config.h"

struct Ball {
    float x, y;
    float vx, vy;
    int size = 4;
    bool active = true;
	float last_vx = 0;
    float last_vy = -BALL_SPEED_INIT;
	
	Ball(float x_, float y_, float vx_, float vy_, bool active_)
    : x(x_), y(y_), vx(vx_), vy(vy_), size(BALL_SIZE), active(active_),
          last_vx(vx_), last_vy(vy_) {}
};

void ball_update(Ball& b);
