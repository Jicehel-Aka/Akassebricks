#include "sprite_atlas.h"
#include "spritesheet_bricks.h"
#include "graphics.h"
#include "lib/LCD.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdio> 

void SpriteAtlas::load(const uint16_t* p, int sw, int sh, int w, int h) {
    pixels = p;
    sheetW = sw;
    sheetH = sh;
    spriteW = w;
    spriteH = h;
    cols = sheetW / spriteW;
}

void SpriteAtlas::draw(int index, int dx, int dy) {
    if (!pixels) return;
    int sx = (index % cols) * spriteW;
    int sy = (index / cols) * spriteH;
    lcd_draw_partial_bitmap(pixels, sheetW, sheetH, sx, sy, spriteW, spriteH, dx, dy);
}

extern SpriteAtlas atlas_bricks;

void test_bricks_visual() {
    lcd_clear(color_black);

    // 1. Afficher la planche entière brute
    lcd_draw_bitmap(bricks_pixels, bricks_sheet_w, bricks_sheet_h, 0, 0);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    lcd_clear(color_black);

    // 2. Afficher chaque sprite individuellement avec son index
    int dx = 0, dy = 0;
    for (int idx = 0; idx < bricks_sheet_cols * bricks_sheet_rows; ++idx) {
        atlas_bricks.draw(idx, dx, dy);

        // Affiche l’index sous la brique
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", idx);
        gfx_text(dx, dy + BRICK_H + 2, buf, color_white);

        dx += BRICK_W + 4; // espacement horizontal
        if (dx + BRICK_W > SCREEN_W) {
            dx = 0;
            dy += BRICK_H + 20; // retour à la ligne avec espace pour le texte
        }
    }

    lcd_refresh();
}
