#include "paddle.h"
#include "config.h"
#include <algorithm>
#include "spritesheet_paddle.h"
#include "graphics.h"
#include "graphics.h"
#include <cstdio>
#include <vector>
#include "assets_paddle.h"

// Reset complet
void paddle_reset(Paddle& p) {
    p.size = NORMAL;
    p.bonus_flags = BONUS_NONE;
    p.sticky_timer = 0;		// reset Glue timer
    p.laser_timer  = 0;  	// reset Laser timer
    p.laser_cooldown = 0; 	// reset Laser cooldown
    // Dimensions via frame (assure cohérence avec planche)
    p.w = PADDLE_WIDTHS[NORMAL];
    p.h = paddle_row_h;

    p.x = (SCREEN_W - p.w) / 2;
    p.y = PADDLE_Y;
}

// Mouvement
void paddle_move(Paddle& p, bool left, bool right, int joyx) {
    int paddle_speed = PADDLE_SPEED; // synamic speed if joy used, fixed with DPAD
    if ( !left && !right ) // evaluate use joy
    {
        if ( joyx<JOYX_MID )
            left = true;
        if ( joyx>JOYX_MID )
            right = true;
        paddle_speed = 2*PADDLE_SPEED*(joyx-JOYX_MID)/JOYX_MID; // 0 to 2 x PADDLE_SPEED
        if ( paddle_speed < 0)
            paddle_speed = -paddle_speed;
    }

    if (left)  p.x = std::max(0, p.x - paddle_speed);
    if (right) p.x = std::min(SCREEN_W - p.w, p.x + paddle_speed);
}


// Activation bonus

void activate_sticky(Paddle& p, int duration) {
    p.bonus_flags |= BONUS_GLUE;
    p.sticky_timer = duration;
    paddle_update_sprite(p);
}


void activate_laser(Paddle& p, int duration) {
    p.bonus_flags |= BONUS_LASER;
    p.laser_timer = duration;
    paddle_update_sprite(p);
}


// Mise à jour des timers
void update_paddle_bonus(Paddle& p) {
    if (p.sticky_timer > 0 && --p.sticky_timer == 0) {
        p.bonus_flags &= ~BONUS_GLUE;
        paddle_update_sprite(p);
    }
    if (p.laser_timer > 0 && --p.laser_timer == 0) {
        p.bonus_flags &= ~BONUS_LASER;
        paddle_update_sprite(p);
    }
}


// Dessin
// États possibles : NONE=0, GLUE=1, LASER=2, BOTH=3
// state = indice de la colonne à afficher
void draw_paddle(const Paddle& p) {
    int w = PADDLE_WIDTHS[p.size];   // largeur du sprite
    int h = paddle_row_h;            // hauteur fixe (12)

    // Déterminer la colonne en fonction des flags
    int column = 0;
    if (p.bonus_flags == BONUS_GLUE) {
        column = 1;
    } else if (p.bonus_flags == BONUS_LASER) {
        column = 2;
    } else if (p.bonus_flags == (BONUS_GLUE | BONUS_LASER)) {
        column = 3;
    }

    int srcX = column * w;

    const uint16_t* buf = paddle_rows[p.size].pixels;

    lcd_draw_partial_bitmap(buf,
                            4 * w, h,   // largeur totale = 4 colonnes
                            srcX, 0, w, h,
                            p.x, p.y);
}


void test_paddle_visual() {
    lcd_clear(color_black);

    int dx = 0, dy = 0;

    for (int s = VERYSMALL; s <= VERYBIG; ++s) {
        int w = PADDLE_WIDTHS[s];
        int h = paddle_row_h;

        for (int b = 0; b < 4; ++b) {
            const PaddleRowSheet& sheet = paddle_rows[s];

            // Décalage horizontal = colonne (bonus) * largeur
            int srcX = b * w;

            lcd_draw_partial_bitmap(sheet.pixels,
                                    4 * w, h,   // largeur totale = 4 colonnes
                                    srcX, 0, w, h,
                                    dx, dy);

            // Label (taille + bonus)
            char buf[32];
            snprintf(buf, sizeof(buf), "Size=%d Bonus=%d", s, b);
            gfx_text(dx, dy + h + 2, buf, color_white);

            dx += w + 8; // espacement horizontal
            if (dx + w > SCREEN_W) {
                dx = 0;
                dy += h + 20; // retour à la ligne
            }
        }
    }

    lcd_refresh();
}


// Patch de test endian/couleur
void test_paddle_endian() {
    lcd_clear(color_black);

    int w = PADDLE_WIDTHS[NORMAL]; // largeur dynamique
    int h = paddle_row_h;

    static uint16_t red[90*12]; // buffer assez grand pour le plus gros paddle
    for (int i = 0; i < w * h; ++i) {
        red[i] = 0xF800; // rouge RGB565
    }

    lcd_draw_bitmap(red, w, h, 40, 40);
    gfx_text(40, 40 + h + 2, "Test rectangle rouge", color_white);

    lcd_refresh();
}


// Vérification des colonnes
void test_paddle_column() {
   lcd_clear(color_black);

    const uint16_t* buf = paddle_row_normal_pixels;
    int w = PADDLE_WIDTHS[NORMAL]; // 60
    int h = paddle_row_h;          // 12
    int fullW = 4 * w;             // 240

    // Affiche les 4 colonnes côte à côte
    for (int col = 0; col < 4; ++col) {
        int srcX = col * w;
        int dstX = 20 + col * (w + 10);

        lcd_draw_partial_bitmap(buf, fullW, h,
                                srcX, 0, w, h,
                                dstX, 20);

        char label[16];
        snprintf(label, sizeof(label), "Col %d", col);
        gfx_text(dstX, 20 + h + 2, label, color_white);
    }

    lcd_refresh();
}