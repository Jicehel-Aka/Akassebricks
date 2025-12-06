#pragma once
#pragma once
#include <cstdint>

struct Paddle {
    int x, y;
    int w, h;
    bool sticky = false;    // ✅ bonus StickyPaddle
    bool laser  = false;    // ✅ bonus Laser
	int sticky_timer = 0;	// ✅ timer StickyPaddle
    int laser_timer = 0;	// ✅ timer Laser
};

void paddle_move(Paddle& p, bool left, bool right);
