#pragma once
#include <cstdint>
#include "config.h"

// Tailles
enum PaddleSize : uint8_t { VERYSMALL, SHRINK, NORMAL, EXPAND, VERYBIG };

// Bonus en flags (combinaisons possibles)
enum PaddleBonusFlags : uint8_t {
    BONUS_NONE  = 0,
    BONUS_GLUE  = 1 << 0,
    BONUS_LASER = 1 << 1
};

struct Paddle {
    int x, y;
    int w, h;

    PaddleSize size = NORMAL;
    uint8_t bonus_flags = BONUS_NONE; // GLUE, LASER, GLUE|LASER

    // Timers associés aux bonus
    int sticky_timer = 0;
    int laser_timer  = 0;
};

// Mouvement DPAD or joy
void paddle_move(Paddle& p, bool left, bool right, int joyx);
// Reset complet (début de partie, changement de niveau, perte de vie)
void paddle_reset(Paddle& p);

// Met à jour dimensions (via frame) après un changement d’état
void paddle_update_sprite(Paddle& p);

// Mise à jour des timers et des flags de bonus
void update_paddle_bonus(Paddle& p);

// Activation des bonus
void activate_sticky(Paddle& p, int duration);
void activate_laser(Paddle& p, int duration);

// Dessin
struct SpriteSheet;
void draw_paddle(const Paddle& p);

// Fonction de test visuel
void test_paddle_visual();

// Fonction de test en changeant le mode de transmission de la coueur
void test_paddle_endian();

// Fonction de test pour vérifier les colonnes
void test_paddle_column();
