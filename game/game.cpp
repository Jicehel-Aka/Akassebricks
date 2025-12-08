	#include "game.h"
	#include "input.h"
	#include "config.h"
	#include "graphics.h"
	#include "assets.h"
	#include "powerup.h"
	#include "paddle.h"
	#include "LCD.h"
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

	void game_init(GameState& g){
		std::srand(static_cast<unsigned>(std::time(nullptr)));
		g.paddle = {SCREEN_W/2 - PADDLE_W/2, PADDLE_Y, PADDLE_W, PADDLE_H};

		g.paddle.bonus_flags = BONUS_NONE;
		g.paddle.sticky_timer = 0;
		g.paddle.laser_timer  = 0;


		g.lives = 3;
		g.score = 0;

		g.balls.clear();
		g.balls.push_back({SCREEN_W/2, SCREEN_H/2, 2.6f, -2.6f, true});

		g.falling.clear();
		g.shots.clear();
		
		// Initialise le niveau

		g.level.generate_grid(BRICK_ROWS, BRICK_COLS, 1);
		
		if (debug) {
			gfx_text(10, 10, ("DEBUG: Init - Etat=" + std::to_string((int)g.state)).c_str(), color_yellow);
			gfx_text(10, 30, ("DEBUG: Init - Vies=" + std::to_string(g.lives)).c_str(), color_yellow);
			gfx_text(10, 50, "DEBUG: Init - generate_grid termine", color_yellow);
			gfx_flush();              // force l’affichage immédiat
			vTaskDelay(500 / portTICK_PERIOD_MS); // ~0,5 seconde de pause
		}

	}

	bool collision_with_paddle(const Ball& b, const Paddle& p, int oldBallBottom) {
		return (b.vy > 0 &&                          // balle descend
				oldBallBottom <= p.y &&              // balle était au-dessus
				b.y + b.size >= p.y &&               // balle est passée en dessous
				b.x + b.size > p.x &&                // chevauchement horizontal
				b.x < p.x + p.w);
	}

	bool collision_ball_brick(const Ball& ball, const Brick& brick) {
		int r = ball.size / 2; // si size = diamètre, sinon mets directement ball.size
		return !(ball.x + r < brick.x ||
				 ball.x - r > brick.x + BRICK_W ||
				 ball.y + r < brick.y ||
				 ball.y - r > brick.y + BRICK_H);
	}

	bool collision_powerup_paddle(const PowerUp& p, const Paddle& pad) {
		return !(p.x > pad.x + pad.w ||
				 p.x + POWERUP_W < pad.x ||
				 p.y > pad.y + pad.h ||
				 p.y + POWERUP_H < pad.y);
	}

	bool collision_projectile_brick(const Projectile& shot, const Brick& brick) {
		return !(shot.x > brick.x + BRICK_W ||
				 shot.x + shot.w < brick.x ||
				 shot.y > brick.y + BRICK_H ||
				 shot.y + shot.h < brick.y);
	}

	// Helper: pick a random power-up type within enum bounds
	static PowerType choose_random_powerup() {
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
    paddle_move(g.paddle, k.left, k.right);
    if (g.paddle.x < 0) g.paddle.x = 0;
    if (g.paddle.x + g.paddle.w > SCREEN_W) g.paddle.x = SCREEN_W - g.paddle.w;
	
	// Si le paddle bouge et qu'une balle est collée (inactive), elle doit suivre
	if (g.paddle.bonus_flags & BONUS_GLUE) {
		for (auto& b : g.balls) {
			if (!b.active) {
				b.x = g.paddle.x + g.paddle.w/2 - BALL_SIZE/2;
				b.y = g.paddle.y - BALL_SIZE;
			}
		}
	}

    // Mise à jour des balles
    for (auto& b : g.balls) {
        if (b.active) {
            int oldBallBottom = b.y + b.size;
            ball_update(b);

            // Si la balle sort de l'écran, on la désactive (perte potentielle)
            if (b.y > SCREEN_H) {
                b.active = false;
                continue;
            }

            // Collision prédictive avec la raquette
            if (collision_with_paddle(b, g.paddle, oldBallBottom)) {
                if (g.paddle.bonus_flags & BONUS_GLUE) {
                    // Balle collée (inactive mais conservée)
                    b.active = false;
                    b.x = g.paddle.x + g.paddle.w/2 - BALL_SIZE/2;
                    b.y = g.paddle.y - BALL_SIZE;
                } else {
                    b.vy = -fabs(b.vy);
                    float hitPos = (b.x + b.size/2) - (g.paddle.x + g.paddle.w/2);
                    b.vx += hitPos / (g.paddle.w/2);
                }
            }
        }
    }

    // Supprimer uniquement les balles inactives qui sont réellement perdues (hors écran)
    g.balls.erase(std::remove_if(g.balls.begin(), g.balls.end(),
                                 [](const Ball& b){ return !b.active && b.y > SCREEN_H; }),
                  g.balls.end());

    // Si plus aucune balle en jeu (y compris collée), vie perdue
    if (g.balls.empty()) {
        g.lives--;
        paddle_reset(g.paddle);
        g.state = GameState::State::WaitingBall;
    }

    // Collisions balle/brique + spawn powerups
    bool allDestroyed = true;
    for (auto& b : g.balls) {
        for (auto& brick : g.level.bricks) {
            if (!brick.alive) continue;
            allDestroyed = false;

            if (collision_ball_brick(b, brick)) {
                brick.hp--;
                if (brick.hp <= 0) {
                    brick.alive = false;
                    g.score += 100;

                    if (rand() % 6 == 0) {
                        PowerType type = choose_random_powerup();
                        PowerUp p{(float)brick.x, (float)brick.y, type, 0, 0, true};
                        g.falling.push_back(p);
                    }
                }
                b.vy = -b.vy;
                break;
            }
        }
    }

    if (allDestroyed) {
        g.levelIndex++;
        g.level.generate_grid(BRICK_ROWS, BRICK_COLS, 2);
        g.balls.clear();
        paddle_reset(g.paddle);
        g.state = GameState::State::WaitingBall;
    }

    // Powerups qui tombent
    for (auto& p : g.falling) {
        if (!p.active) continue;
        p.y += 2;
        p.animFrame = (p.animFrame + 1) % 4;

        if (collision_powerup_paddle(p, g.paddle)) {
            powerup_apply(g, p);
        }
        if (p.y > SCREEN_H) p.active = false;
    }
    g.falling.erase(std::remove_if(g.falling.begin(), g.falling.end(),
                                   [](const PowerUp& p){ return !p.active; }),
                    g.falling.end());

    // Laser : tir si actif
    if ((g.paddle.bonus_flags & BONUS_LASER) && k.A) {
        Projectile shot{(float)(g.paddle.x + g.paddle.w/2),
                        (float)g.paddle.y,
                        0.0f, -3.0f,
                        2, 6,
                        true};
        g.shots.push_back(shot);
    }

    // Mise à jour projectiles
    for (auto& s : g.shots) {
        if (!s.active) continue;
        s.y -= 4;
        if (s.y < 0) { s.active = false; continue; }
        for (auto& brick : g.level.bricks) {
            if (brick.alive && collision_projectile_brick(s, brick)) {
                brick.hp--;
                if (brick.hp <= 0) { brick.alive = false; g.score += 100; }
                s.active = false;
                break;
            }
        }
    }
    g.shots.erase(std::remove_if(g.shots.begin(), g.shots.end(),
                                 [](const Projectile& s){ return !s.active; }),
                  g.shots.end());

    // Relance Sticky: ne pas retirer le flag, le timer gère la durée
    if ((g.paddle.bonus_flags & BONUS_GLUE) && k.A) {
        for (auto& b : g.balls) {
            if (!b.active) {
                b.active = true;
                b.vx = 2.6f; b.vy = -2.6f;
            }
        }
        // Laisse BONUS_GLUE actif; il expirera avec le timer
    }

    // Timers des bonus (expiration)
    if (g.paddle.bonus_flags & BONUS_GLUE) {
        if (--g.paddle.sticky_timer <= 0) {
            g.paddle.bonus_flags &= ~BONUS_GLUE;
            g.paddle.sticky_timer = 0;
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
