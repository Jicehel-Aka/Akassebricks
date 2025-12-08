#pragma once
#include "sprite_atlas.h"
#include <cstdint>

// Bricks
extern SpriteAtlas atlas_bricks;

// Powerups
extern SpriteAtlas atlas_powerups;

// Ball
extern const uint16_t ball_pixels[];

// Initialisation
void init_assets();
