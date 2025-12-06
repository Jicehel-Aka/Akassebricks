#pragma once
#include "sprite_atlas.h"
#include <cstdint>

// Bricks
extern SpriteAtlas atlas_bricks;

// Powerups
extern SpriteAtlas atlas_powerups;

// Ball
extern const uint16_t ball_pixels[];

// Paddle
extern const uint16_t paddle_pixels[];


void init_assets();