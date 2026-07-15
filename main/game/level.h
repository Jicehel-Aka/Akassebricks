#pragma once
#include <vector>
#include "brick.h"

struct Level {
    int id;
    std::vector<Brick> bricks;
    void generate_grid(int rows,int cols,int hp);
};
