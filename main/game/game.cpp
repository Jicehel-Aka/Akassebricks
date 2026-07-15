	#include "game.h"
	#include "input.h"
	#include "config.h"
	#include "graphics.h"
	#include "assets.h"
	#include "powerup.h"
	#include "paddle.h"
	#include "audio.h"
	#include "graphics.h"
	#include "graphics_basic.h"
	#include <cmath> 
	#include <algorithm>
	#include <string>	
	#include <ctime>
	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"
	
	///--------------------------------
	// --- Initialisation ---
	///--------------------------------
	
	 void spawn_sticky_ball(GameState& g) {
		float bx = g.paddle.x + g.paddle.w/2.0f - BALL_SIZE/2.0f;
		float by = g.paddle.y - BALL_SIZE;

		g.balls.clear();
		g.balls.push_back(Ball(bx, by, BALL_SPEED_INIT, -BALL_SPEED_INIT, false));

		// Sticky initial : actif et non décrémenté
		g.paddle.bonus_flags |= BONUS_GLUE;
		g.paddle.sticky_timer = -1;
	}
	
	void launch_ball(GameState& g) {
    if (!g.balls.empty()) {
        auto& b = g.balls[0];
        b.active = true;
        b.vx = BALL_SPEED_INIT;
        b.vy = -BALL_SPEED_INIT;
        b.last_vx = b.vx;
        b.last_vy = b.vy;
    }

    snd_launch.play_tone(659.0f, 120);

    // Si c’était un sticky initial ou que le timer est écoulé, on le retire immédiatement
    if (g.paddle.sticky_timer == -1) {
        g.paddle.bonus_flags &= ~BONUS_GLUE;
        g.paddle.sticky_timer = 0;
    }

    g.state = GameState::State::Playing;
}
	
	
	void game_init(GameState& g){
		std::srand(static_cast<unsigned>(std::time(nullptr)));

		// Paddle
		g.paddle = {SCREEN_W/2 - PADDLE_W/2, PADDLE_Y, PADDLE_W, PADDLE_H};
		g.paddle.bonus_flags = BONUS_NONE;
		g.paddle.sticky_timer = 0;
		g.paddle.laser_timer  = 0;

		// State
		g.lives = 3;
		g.score = 0;

		// Entities
		g.falling.clear();
		g.shots.clear();

		// Level
		g.levelIndex = 0; // si tu utilises levelIndex
		g.level.generate_grid(BRICK_ROWS, BRICK_COLS, 1);

		// Balle collée initiale
		spawn_sticky_ball(g);
		g.state = GameState::State::WaitingBall;

		if (debug) {
			gfx_text(10, 10, ("DEBUG: Init - Etat=" + std::to_string((int)g.state)).c_str(), color_yellow);
			gfx_text(10, 30, ("DEBUG: Init - Vies=" + std::to_string(g.lives)).c_str(), color_yellow);
			gfx_text(10, 50, "DEBUG: Init - generate_grid termine", color_yellow);
			gfx_flush();
			vTaskDelay(500 / portTICK_PERIOD_MS);
		}
	}

// --- Collisions plus robustes ---

// Teste si le segment de déplacement d’un objet (old→new) traverse un rectangle cible
bool swept_overlap(float oldX, float oldY, float newX, float newY,
                   int objW, int objH,
                   int rectX, int rectY, int rectW, int rectH) {
    // Rectangle de l’objet (old et new)
    float oldLeft   = oldX;
    float oldRight  = oldX + objW;
    float oldTop    = oldY;
    float oldBottom = oldY + objH;

    float newLeft   = newX;
    float newRight  = newX + objW;
    float newTop    = newY;
    float newBottom = newY + objH;

    // Rectangle cible
    int rx1 = rectX, ry1 = rectY;
    int rx2 = rectX + rectW, ry2 = rectY + rectH;

    bool overlapX = (std::max(oldLeft, newLeft) < rx2 &&
                     std::min(oldRight, newRight) > rx1);
    bool overlapY = (std::max(oldTop, newTop) < ry2 &&
                     std::min(oldBottom, newBottom) > ry1);

    return overlapX && overlapY;
}



	// Balle ↔ Raquette
	bool collision_with_paddle(const Ball& b, const Paddle& p, const Ball& oldBall) {
		return (b.vy > 0) &&
			   swept_overlap(oldBall.x, oldBall.y, b.x, b.y,
							 b.size, b.size,
							 p.x, p.y, p.w, p.h);
	}

	// Balle ↔ Brique
	bool collision_ball_brick(const Ball& b, const Brick& brick, const Ball& oldBall) {
		return swept_overlap(oldBall.x, oldBall.y, b.x, b.y,
							 b.size, b.size,
							 brick.x, brick.y, BRICK_W, BRICK_H);
	}

	// PowerUp ↔ Raquette
	bool collision_powerup_paddle(const PowerUp& p, const Paddle& pad, const PowerUp& oldP) {
		return swept_overlap(oldP.x, oldP.y, p.x, p.y,
							 POWERUP_W, POWERUP_H,
							 pad.x, pad.y, pad.w, pad.h);
	}

	bool collision_projectile_brick(const Projectile& shot,
									const Brick& brick,
									const Projectile& oldShot) {
		return swept_overlap(oldShot.x, oldShot.y, shot.x, shot.y,
							 shot.w, shot.h,
							 brick.x, brick.y, BRICK_W, BRICK_H);
	} 

	PowerType choose_random_powerup() {
		int count = static_cast<int>(PowerType::POWERUP_COUNT);
		return static_cast<PowerType>(rand() % count);
	}


	///--------------------------------
	// --- Mise à jour ---
	///--------------------------------

void game_update(GameState& g){
    Keys k;
    input_poll(k);

    // Déplacement paddle (clamp avec largeur dynamique)
    paddle_move(g.paddle, k.left, k.right, k.joxx);
    if (g.paddle.x < 0) g.paddle.x = 0;
    if (g.paddle.x + g.paddle.w > SCREEN_W) g.paddle.x = SCREEN_W - g.paddle.w;
    
    // Si le paddle bouge et qu'une balle est collée (inactive), elle doit suivre
    if (g.paddle.bonus_flags & BONUS_GLUE) {
        for (auto& b : g.balls) {
            if (!b.active) {
                b.last_vx = b.vx;
                b.last_vy = b.vy;
                b.x = g.paddle.x + g.paddle.w/2 - BALL_SIZE/2;
                b.y = g.paddle.y - BALL_SIZE;
            }
        }
    }

    // Mise à jour des balles
    for (auto& b : g.balls) {
        if (b.active) {
            Ball oldBall = b; // snapshot avant update
            ball_update(b);

            // Si la balle sort de l'écran
            if (b.y > SCREEN_H) { b.active = false; continue; }

            // Collision raquette (robuste)
            if (collision_with_paddle(b, g.paddle, oldBall)) {
                if (g.paddle.bonus_flags & BONUS_GLUE) {
                    b.active = false;
                    b.x = g.paddle.x + g.paddle.w/2 - BALL_SIZE/2;
                    b.y = g.paddle.y - BALL_SIZE;
                    snd_stick.play_tone(440.0, 210);
                } else {
                    // Rebond facon Arkanoid : c'est la POSITION du contact qui
                    // commande l'ANGLE, pas la vitesse. On conserve une vitesse
                    // constante (avec une legere montee en regime), et on BORNE
                    // l'angle pour que la balle ne reparte jamais a plat.
                    const float half = g.paddle.w * 0.5f;
                    float hit = ((b.x + b.size * 0.5f) - (g.paddle.x + half)) / half;
                    if (hit < -1.0f) hit = -1.0f;      // clamp bord de raquette
                    if (hit >  1.0f) hit =  1.0f;

                    constexpr float MAX_ANGLE = 1.0472f;   // 60 deg depuis la verticale
                    const float angle = hit * MAX_ANGLE;

                    // Vitesse courante -> montee en regime bornee (BALL_SPEED_INC /
                    // BALL_SPEED_MAX, qui n'etaient pas cablees jusqu'ici).
                    float speed = sqrtf(b.vx * b.vx + b.vy * b.vy);
                    speed += BALL_SPEED_INC;
                    if (speed > BALL_SPEED_MAX)  speed = BALL_SPEED_MAX;
                    if (speed < BALL_SPEED_INIT) speed = BALL_SPEED_INIT;

                    b.vx =  speed * sinf(angle);
                    b.vy = -speed * cosf(angle);   // toujours vers le haut ; cos(60)=0.5 -> jamais a plat
                    snd_paddle.play_tone(523.0, 140);
                }
            }

            // Collision briques (robuste)
            for (auto& brick : g.level.bricks) {
                if (!brick.alive) continue;
                if (collision_ball_brick(b, brick, oldBall)) {
                    brick.hp--;
                    if (brick.hp <= 0) {
                        brick.alive = false;
                        g.score += 100;
                        if (rand() % 6 == 0) {
                            PowerType type = choose_random_powerup();
                            g.falling.push_back({(float)brick.x, (float)brick.y, type, 0, 0, true});
                        }
                        snd_brick_break.play_tone(220.0, 150);
                    } else {
                        snd_brick_hit.play_tone(392.0, 60);
                    }

                    // Détermine l'axe de rebond à partir de la position du centre de
                    // la balle avant son déplacement (oldBall), par rapport au centre
                    // de la brique. Cela évite de se fier uniquement au signe de vy,
                    // qui peut entraîner un double-rebond annulé visuellement si la
                    // balle reste chevauchée avec une brique encore vivante (hp > 1)
                    // pendant plusieurs frames consécutives (cf. tunneling niveau 2+).
                    float ballCenterX = oldBall.x + oldBall.size / 2.0f;
                    float ballCenterY = oldBall.y + oldBall.size / 2.0f;
                    float brickCenterX = brick.x + BRICK_W / 2.0f;
                    float brickCenterY = brick.y + BRICK_H / 2.0f;

                    float dx = ballCenterX - brickCenterX;
                    float dy = ballCenterY - brickCenterY;

                    // Pénétration relative sur chaque axe (normalisée par la demi-étendue)
                    float overlapX = (BRICK_W / 2.0f + b.size / 2.0f) - fabs(dx);
                    float overlapY = (BRICK_H / 2.0f + b.size / 2.0f) - fabs(dy);

                    if (overlapX < overlapY) {
                        // Rebond horizontal : on sort la balle à gauche ou à droite de la brique
                        b.vx = -b.vx;
                        b.x = (dx > 0) ? (brick.x + BRICK_W + 0.1f) : (brick.x - b.size - 0.1f);
                    } else {
                        // Rebond vertical : on sort la balle au-dessus ou en dessous de la brique
                        b.vy = -b.vy;
                        b.y = (dy > 0) ? (brick.y + BRICK_H + 0.1f) : (brick.y - b.size - 0.1f);
                    }
                    break;
                }
            }
        }
    }

    // Supprimer les balles inactives hors écran
    g.balls.erase(std::remove_if(g.balls.begin(), g.balls.end(),
                                 [](const Ball& b){ return !b.active && b.y > SCREEN_H; }),
                  g.balls.end());

    // Si plus aucune balle → vie perdue
    if (g.balls.empty()) {
        g.lives--;
        paddle_reset(g.paddle);
        snd_lost_life.play_tone(196.0, 200);
        spawn_sticky_ball(g);
        g.state = GameState::State::WaitingBall;
        return;
    }

    // Vérifier si des briques restent
    bool anyBrickAlive = false;
    for (const auto& brick : g.level.bricks) {
        if (brick.alive) { anyBrickAlive = true; break; }
    }

    // Niveau terminé
    if (!anyBrickAlive) {
        g.levelIndex++;
        g.level.generate_grid(BRICK_ROWS, BRICK_COLS, 2);
        paddle_reset(g.paddle);
        snd_level_start.play_tone(523.0, 80);
        snd_level_start.play_tone(659.0, 80);
        snd_level_start.play_tone(784.0, 120);
        spawn_sticky_ball(g);
        g.state = GameState::State::WaitingBall;
        return;
    }
    
    // Powerups
    for (auto& p : g.falling) {
        if (!p.active) continue;
        PowerUp oldP = p; // snapshot avant update
        p.y += 2;
        p.animFrame = (p.animFrame + 1) % 4;

        if (collision_powerup_paddle(p, g.paddle, oldP)) {
            powerup_apply(g, p);
        }
        if (p.y > SCREEN_H) p.active = false;
    }
    g.falling.erase(std::remove_if(g.falling.begin(), g.falling.end(),
                                   [](const PowerUp& p){ return !p.active; }),
                    g.falling.end());

    // Gestion laser
    if (g.paddle.laser_cooldown > 0) g.paddle.laser_cooldown--;
    if ((g.paddle.bonus_flags & BONUS_LASER) && k.A && g.paddle.laser_cooldown == 0) {
        g.shots.push_back({(float)(g.paddle.x + g.paddle.w/2),
                           (float)g.paddle.y,
                           0.0f, -3.0f,
                           2, 6,
                           true});
        g.paddle.laser_cooldown = 30;
    }

    // Projectiles
	for (auto& s : g.shots) {
		if (!s.active) continue;

		Projectile oldShot = s;   // snapshot avant déplacement
		s.y -= 4;
		if (s.y < 0) { s.active = false; continue; }

		for (auto& brick : g.level.bricks) {
			if (brick.alive && collision_projectile_brick(s, brick, oldShot)) {
				brick.hp--;
				if (brick.hp <= 0) {
					brick.alive = false;
					g.score += 100;
				}
				s.active = false;
				break;
			}
		}
	}
    g.shots.erase(std::remove_if(g.shots.begin(), g.shots.end(),
                                 [](const Projectile& s){ return !s.active; }),
                  g.shots.end());

    // Relance Sticky
    if ((g.paddle.bonus_flags & BONUS_GLUE) && k.A) {
        for (auto& b : g.balls) {
            if (!b.active) {
                b.active = true;
                b.vx = (fabs(b.vx) > 0.01f) ? b.vx : 0.0f;
                b.vy = -fabs(b.vy);
            }
        }
    }

    // Timers bonus
    if (g.paddle.bonus_flags & BONUS_GLUE) {
        if (g.paddle.sticky_timer > 0) {
            if (--g.paddle.sticky_timer <= 0) {
                bool hasInactive = false;
                for (const auto& b : g.balls) {
                    if (!b.active) { hasInactive = true; break; }
                }
                if (hasInactive) {
                    g.paddle.sticky_timer = -1;
                } else {
                    g.paddle.bonus_flags &= ~BONUS_GLUE;
                    g.paddle.sticky_timer = 0;
                }
            }
        }
    }
    if (g.paddle.bonus_flags & BONUS_LASER) {
        if (--g.paddle.laser_timer <= 0) {
            g.paddle.bonus_flags &= ~BONUS_LASER;
            g.paddle.laser_timer = 0;
        }
    }
}

	///--------------------------------
	// --- Dessin ---
	///--------------------------------

	void game_draw(const GameState& g){
		
		lcd_clear(color_black);
		graphics_basic gfx;   // création d’un objet pour le debug

		// Paddle
		draw_paddle(g.paddle);
		
		// Alternative pour debug - Objet Paddle
		/* gfx.setColor(color_lightgreen);
		gfx.fillRect(g.paddle.x, g.paddle.y, g.paddle.w, g.paddle.h); */

		
		// Balls
		for (const auto& b : g.balls) {
			 lcd_draw_bitmap(ball_pixels, BALL_SIZE, BALL_SIZE, (int)b.x, (int)b.y);
		}
		
		// Alternative pour debug - Objet Ball
		/* for (const auto& b : g.balls) {
			gfx.setColor(color_white);
			gfx.fillRect((int)b.x, (int)b.y, BALL_SIZE, BALL_SIZE);
		} */

		
		// Bricks (via Level)
		for (const auto& b : g.level.bricks) {
			 if (!b.alive) continue;
			 int idx = get_brick_sprite_index(b.color_index, b.hp, b.indestructible);
			 atlas_bricks.draw(idx, b.x, b.y);
		}

			// Alternative pour debug - Objet Briques
		/* for (const auto& b : g.level.bricks) {
			if (!b.alive) continue;
			gfx.setColor(color_yellow);
			gfx.fillRect(b.x, b.y, BRICK_W, BRICK_H);
		} */

		
		// Powerups
		for (const auto& p : g.falling) {
			 if (!p.active) continue;
			 int idx = get_powerup_sprite_index(p.type, p.animFrame);
			 atlas_powerups.draw(idx, (int)p.x, (int)p.y);
		}
		
		// Alternative pour debug - Objet Powerups
		/* for (const auto& p : g.falling) {
			if (!p.active) continue;
			gfx.setColor(color_green);
			gfx.fillRect((int)p.x, (int)p.y, 8, 8);
		} */

		
		// Projectiles laser
		for (const auto& s : g.shots) {
			gfx.setColor(color_red);
			gfx.fillRect((int)s.x, (int)s.y, 2, 6);
		}

		
		// Score
		lcd_draw_text(2, 2, ("Score: " + std::to_string(g.score)).c_str());
		
		// Vies
		lcd_draw_text(120, 2, ("Vies: " + std::to_string(g.lives)).c_str());

		// Alternative pour debug - objet score
		/* char buf[32];
		std::snprintf(buf, sizeof(buf), "Score: %d", g.score); */

		lcd_refresh();
	}

	bool game_is_over(const GameState& g){
		return g.lives <= 0;
	}
