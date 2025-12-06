#include "powerup.h"
#include "config.h"
#include "assets/assets.h"
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

void powerup_apply(GameState& g, PowerUp& p) {
    switch (p.type) {
        case PowerType::SlowBall:
            for (auto& b : g.balls) {
                b.vx *= 0.7f;
                b.vy *= 0.7f;
                if (std::fabs(b.vx) > BALL_SPEED_MAX)
                    b.vx = (b.vx > 0 ? BALL_SPEED_MAX : -BALL_SPEED_MAX);
                if (std::fabs(b.vy) > BALL_SPEED_MAX)
                    b.vy = (b.vy > 0 ? BALL_SPEED_MAX : -BALL_SPEED_MAX);
            }
            break;

        case PowerType::FastBall:
            for (auto& b : g.balls) {
                b.vx *= 1.3f;
                b.vy *= 1.3f;
                if (std::fabs(b.vx) > BALL_SPEED_MAX)
                    b.vx = (b.vx > 0 ? BALL_SPEED_MAX : -BALL_SPEED_MAX);
                if (std::fabs(b.vy) > BALL_SPEED_MAX)
                    b.vy = (b.vy > 0 ? BALL_SPEED_MAX : -BALL_SPEED_MAX);
            }
            break;

        case PowerType::StickyPaddle:
            g.paddle.sticky = true;
            g.paddle.sticky_timer = 1000;
            break;

        case PowerType::Laser:
            g.paddle.laser = true;
            g.paddle.laser_timer = 800;
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

    p.active = false;
}
