#include "level.h"
#include <cstdlib>  
#include <string>       // pour std::to_string
#include "config.h"     // pour debug
#include "graphics.h"   // pour gfx_text, gfx_flush
#include "LCD.h"        // pour color_yellow, color_green

void Level::generate_grid(int rows,int cols,int hp){
    bricks.clear();
    // if (debug) gfx_text(10, 10, "DEBUG: generate_grid start", color_yellow);

    // Calcul des marges horizontales pour centrer
    int total_width = cols * BRICK_W + (cols - 1) * BRICK_MARGIN_X;
    int offset_x = (SCREEN_W - total_width) / 2;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            Brick b;
            b.x = offset_x + c * (BRICK_W + BRICK_MARGIN_X);
            b.y = BRICK_TOP + r * (BRICK_H + BRICK_MARGIN_Y);
            b.hp = hp > 0 ? hp : 2;
            b.alive = true;
            b.indestructible = false;
            b.color_index = std::rand() % BRICK_COLOR_COUNT;
            b.selected = false;

            bricks.push_back(b);
        }
    }
    if (debug) gfx_text(10, 50, "DEBUG: generate_grid end", color_yellow);
}
