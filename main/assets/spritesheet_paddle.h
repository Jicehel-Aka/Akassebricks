#pragma once
#include <cstdint>
#include "paddle.h"   // BONUS_* flags, PaddleSize
#include "config.h"   // PADDLE_WIDTHS

// Sous-image dans une planche (ligne)
struct Frame { int x, y, w, h; };

// Une ligne de sprites (une définition par taille de paddle)
struct PaddleRowSheet {
    const uint16_t* pixels;
    int sheetW;
    int sheetH;
    int spriteW;
    int spriteH;

    // Constructeurs pour permettre l'initialisation avec accolades
    PaddleRowSheet()
        : pixels(nullptr), sheetW(0), sheetH(0), spriteW(0), spriteH(0) {}

    PaddleRowSheet(const uint16_t* p, int sw, int sh, int w, int h)
        : pixels(p), sheetW(sw), sheetH(sh), spriteW(w), spriteH(h) {}
};


// 5 tailles × 4 états (NONE, GLUE, LASER, GLUE+LASER)
extern Frame paddle_frames[5][4];

// 5 lignes: VERYSMALL..VERYBIG
extern PaddleRowSheet paddle_rows[5];

// Flags → index visuel 0..3
inline int bonus_to_index(uint8_t flags) {
    switch (flags) {
        case BONUS_NONE:               return 0;
        case BONUS_GLUE:               return 1;
        case BONUS_LASER:              return 2;
        case BONUS_GLUE | BONUS_LASER: return 3;
        default:                       return 0;
    }
}

// Initialisations
void init_paddle_rows();    // initialise paddle_rows[] (pixels + w/h)
void initPaddleFrames();    // construit paddle_frames[][] à partir de PADDLE_WIDTHS
