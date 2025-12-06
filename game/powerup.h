#pragma once
#include <cstdint>
#include "game.h"

enum class PowerType {
    ExpandPaddle,
    ShrinkPaddle,
    SlowBall,
    FastBall,
    MultiBall,
    StickyPaddle,
    Laser,
    ExtraLife,
    ScoreBoost,
	POWERUP_COUNT
};

struct GameState;

struct PowerUp {
    float x,y;
    PowerType type;
    int animFrame = 0;
	int timer = 0;
    bool active = false;
};

// Indexation graphique
int get_powerup_sprite_index(PowerType type,int frame);

// Application des effets
void powerup_apply(GameState& g, PowerUp& p);
