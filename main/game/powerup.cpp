#include "powerup.h"
#include "config.h"
#include "assets/assets.h"
#include "assets_paddle.h"
#include "game.h"
#include <algorithm>
#include <cmath>

int get_powerup_sprite_index(PowerType type, int frame) {
    int base = 0;
    switch (type) {
        case PowerType::ExpandPaddle: base = 0; break;
        case PowerType::ShrinkPaddle: base = 4; break;
        case PowerType::SlowBall:     base = 8; break;
        case PowerType::FastBall:     base = 12; break;
        case PowerType::MultiBall:    base = 16; break;
        case PowerType::StickyPaddle: base = 20; break;
        case PowerType::Laser:        base = 24; break;
        case PowerType::ExtraLife:    base = 28; break;
        case PowerType::ScoreBoost:   base = 32; break;
		case PowerType::POWERUP_COUNT:
        default:
            return 0; // valeur neutre
    }
    return base + (frame % 4);
}

// Helpers de taille bornée
inline PaddleSize size_step_up(PaddleSize s) {
    return (s < VERYBIG) ? static_cast<PaddleSize>(static_cast<int>(s) + 1) : s;
}
inline PaddleSize size_step_down(PaddleSize s) {
    return (s > VERYSMALL) ? static_cast<PaddleSize>(static_cast<int>(s) - 1) : s;
}

// Met à jour la largeur et recentre le paddle pour conserver le centre X
inline void apply_paddle_size(GameState& g, PaddleSize newSize) {
    if (newSize == g.paddle.size) return; // déjà à la borne

    int oldW = g.paddle.w;
    g.paddle.size = newSize;
    g.paddle.w = PADDLE_WIDTHS[newSize];
    g.paddle.h = paddle_row_h; // hauteur fixe = 12

    // Recentrage pour ne pas « sauter » visuellement
    g.paddle.x += (oldW - g.paddle.w) / 2;

    // Clamping écran (évite de dépasser à gauche/droite)
    if (g.paddle.x < 0) g.paddle.x = 0;
    if (g.paddle.x + g.paddle.w > SCREEN_W) g.paddle.x = SCREEN_W - g.paddle.w;
}

void powerup_apply(GameState& g, PowerUp& p) {
    switch (p.type) {
        case PowerType::ExpandPaddle: {
            PaddleSize next = size_step_up(g.paddle.size);
            apply_paddle_size(g, next);
        } break;

        case PowerType::ShrinkPaddle: {
            PaddleSize prev = size_step_down(g.paddle.size);
            apply_paddle_size(g, prev);
        } break;

        case PowerType::SlowBall:
            for (auto& b : g.balls) {
                b.vx *= 0.7f;
                b.vy *= 0.7f;
                if (std::fabs(b.vx) > BALL_SPEED_MAX) b.vx = (b.vx > 0 ? BALL_SPEED_MAX : -BALL_SPEED_MAX);
                if (std::fabs(b.vy) > BALL_SPEED_MAX) b.vy = (b.vy > 0 ? BALL_SPEED_MAX : -BALL_SPEED_MAX);
            }
            break;

        case PowerType::FastBall:
            for (auto& b : g.balls) {
                b.vx *= 1.3f;
                b.vy *= 1.3f;
                if (std::fabs(b.vx) > BALL_SPEED_MAX) b.vx = (b.vx > 0 ? BALL_SPEED_MAX : -BALL_SPEED_MAX);
                if (std::fabs(b.vy) > BALL_SPEED_MAX) b.vy = (b.vy > 0 ? BALL_SPEED_MAX : -BALL_SPEED_MAX);
            }
            break;

        case PowerType::StickyPaddle:
            g.paddle.bonus_flags |= BONUS_GLUE;
            g.paddle.sticky_timer = 1500; 
            break;

        case PowerType::Laser:
            g.paddle.bonus_flags |= BONUS_LASER;
            g.paddle.laser_timer = 1000;
            break;

        case PowerType::MultiBall:
            if (!g.balls.empty()) {
                Ball newBall = g.balls[0];
                newBall.vx = -newBall.vx;
                g.balls.push_back(newBall);
            }
            break;

        case PowerType::ExtraLife:
            g.lives += 1;
            break;

        case PowerType::ScoreBoost:
            g.score += 500;
            break;

        default:
            break;
    }

    p.active = false; // consommé
}
