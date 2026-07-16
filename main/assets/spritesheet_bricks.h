
#pragma once
#include <stdint.h>
#include "config.h"

// Dimensions de la planche (Option A : 4 colonnes × 4 lignes)
constexpr int bricks_sheet_cols = 4;
constexpr int bricks_sheet_rows = 4;
constexpr int bricks_sheet_w = bricks_sheet_cols * BRICK_W;
constexpr int bricks_sheet_h = bricks_sheet_rows * BRICK_H;
constexpr int bricks_sheet_size = bricks_sheet_w * bricks_sheet_h;

// Tableau RGB565 généré depuis ton image
extern const uint16_t bricks_pixels[bricks_sheet_size];

// Pour atlas_bricks (retourne un index 0..15)
inline int get_brick_sprite_index(int color_index, int hp, bool indestructible) {
    int col = 0, row = 0;

    if (indestructible) {
        col = 3; row = 3; // case spéciale
    } else {
        if (hp >= 3) row = 0;       // intact
        else if (hp == 2) row = 1;  // abîmé
        else row = 2;               // très abîmé

        // couleurs 0..3 sur les 3 premières lignes
        if (color_index < 4) {
            col = color_index;
        } else {
            // 5ème couleur → ligne 3, colonnes 0..2
            row = 3;
            col = (hp >= 3 ? 0 : (hp == 2 ? 1 : 2));
        }
    }

    return row * 4 + col;
}

// Pour lcd_draw_bitmap
inline const uint16_t* get_brick_sprite_pixels(int color_index, int hp, bool indestructible) {
    int idx = get_brick_sprite_index(color_index, hp, indestructible);
    return &bricks_pixels[idx * BRICK_W * BRICK_H];
}