#pragma once
#include <stdint.h>
#include "config.h"
#include "assets/spritesheet_bricks.h"

// Structure représentant une brique
struct Brick {
    int x, y;             // Position écran
    int hp;               // Points de vie : 3 = intact, 2 = abîmé, 1 = très abîmé
    bool alive;           // True si la brique est encore présente
    bool indestructible;  // True si brique indestructible
    int color_index;      // 0..4 (5 couleurs disponibles)
    bool selected;        // Utilisé par l’éditeur de niveau
};

// --- Fonctions utilitaires ---
// Ces fonctions sont définies inline dans spritesheet_bricks.h
// et permettent d'obtenir soit un index pour atlas_bricks,
// soit un pointeur vers les pixels pour lcd_draw_bitmap.

inline int get_brick_sprite_index(uint8_t color_index, int hp, bool indestructible) {
    return ::get_brick_sprite_index(color_index, hp, indestructible);
}

inline const uint16_t* get_brick_sprite_pixels(uint8_t color_index, int hp, bool indestructible) {
    return ::get_brick_sprite_pixels(color_index, hp, indestructible);
}



