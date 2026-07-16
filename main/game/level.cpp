#include "level.h"
#include <cstdlib>  
#include <string>       // pour std::to_string
#include "config.h"     // pour debug
#include "graphics.h"   // pour gfx_text, gfx_flush
#include "graphics.h"        // pour color_yellow, color_green

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

// Vraie progression de niveaux : motifs varies, difficulte croissante, et
// briques indestructibles a partir des niveaux avances. levelIndex commence a 0.
void Level::generate(int levelIndex) {
    bricks.clear();

    const int cols = BRICK_COLS;
    int total_width = cols * BRICK_W + (cols - 1) * BRICK_MARGIN_X;
    int offset_x = (SCREEN_W - total_width) / 2;

    int rows = 3 + (levelIndex % 4);            // 3..6 rangees
    if (rows > BRICK_ROWS) rows = BRICK_ROWS;
    int baseHp = 1 + (levelIndex / 3);          // +1 hp tous les 3 niveaux
    if (baseHp > 3) baseHp = 3;
    int pattern = levelIndex % 5;               // 5 motifs cycliques
    bool useIndes = (levelIndex >= 3);          // incassables des le niveau 4

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            bool place = true;
            switch (pattern) {
                case 0: place = true; break;                                   // plein
                case 1: place = ((r + c) % 2 == 0); break;                     // damier
                case 2: { int m = r; place = (c >= m && c < cols - m); } break;// pyramide
                case 3: place = (c % 3 != 2); break;                           // colonnes trouees
                case 4: place = (r == 0 || r == rows - 1 ||                    // cadre + diagonale
                                 c == 0 || c == cols - 1 || (r + c) % 2 == 0); break;
            }
            if (!place) continue;

            bool indes = false;
            if (useIndes) {
                // Quelques incassables : centre de la rangee du haut, et motifs
                // reguliers au milieu pour les niveaux "cadre".
                if (r == 0 && c == cols / 2) indes = true;
                if (pattern == 4 && r == rows / 2 && (c % 4 == 0)) indes = true;
            }

            int hp = baseHp + (r == 0 ? 1 : 0); // rangee du haut plus resistante
            if (hp > 3) hp = 3;

            Brick b;
            b.x = offset_x + c * (BRICK_W + BRICK_MARGIN_X);
            b.y = BRICK_TOP + r * (BRICK_H + BRICK_MARGIN_Y);
            b.hp = indes ? 1 : hp;
            b.alive = true;
            b.indestructible = indes;
            b.color_index = indes ? 0 : (std::rand() % BRICK_COLOR_COUNT);
            b.selected = false;
            bricks.push_back(b);
        }
    }

    // Securite : un niveau ne doit JAMAIS etre compose uniquement d'incassables
    // (ou vide), sinon il ne peut pas se terminer.
    bool anyDestructible = false;
    for (const auto& b : bricks) if (!b.indestructible) { anyDestructible = true; break; }
    if (!anyDestructible) {
        for (int c = 0; c < cols; c++) {
            Brick b;
            b.x = offset_x + c * (BRICK_W + BRICK_MARGIN_X);
            b.y = BRICK_TOP;
            b.hp = baseHp; b.alive = true; b.indestructible = false;
            b.color_index = std::rand() % BRICK_COLOR_COUNT; b.selected = false;
            bricks.push_back(b);
        }
    }

    id = levelIndex;
}
