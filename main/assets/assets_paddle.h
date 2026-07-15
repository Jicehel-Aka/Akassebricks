#pragma once
#include <cstdint>
#include "spritesheet_paddle.h"
#include"config.h"

// Hauteur commune de ligne 
inline constexpr int paddle_row_h = 12;

// Déclarations des buffers RG565 pour chaque ligne
// Chaque buffer contient 4 colonnes côte à côte (NONE, GLUE, LASER, GLUE+LASER)
// La largeur de la ligne = 4 * PADDLE_WIDTHS[size], la hauteur = row_h constant.

extern const uint16_t paddle_row_verysmall_pixels[4 * PADDLE_WIDTHS[VERYSMALL] * paddle_row_h];
extern const uint16_t paddle_row_shrink_pixels   [4 * PADDLE_WIDTHS[SHRINK]    * paddle_row_h];
extern const uint16_t paddle_row_normal_pixels   [4 * PADDLE_WIDTHS[NORMAL]    * paddle_row_h];
extern const uint16_t paddle_row_expand_pixels   [4 * PADDLE_WIDTHS[EXPAND]    * paddle_row_h];
extern const uint16_t paddle_row_verybig_pixels  [4 * PADDLE_WIDTHS[VERYBIG]   * paddle_row_h];

void init_assets();