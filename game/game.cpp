#include "game.h"
#include "input.h"
#include "config.h"
#include "graphics.h"
#include "assets.h"
#include "powerup.h"
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

    g.paddle.sticky = false;
    g.paddle.laser  = false;

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
    input_poll(k);   // remplit la structure Keys	

    // Déplacement paddle
    paddle_move(g.paddle, k.left, k.right);

	// --- DEBUG infos principales ---
    /* if (debug) {
        gfx_text(10, 10, ("DEBUG: Run - Etat=" + std::to_string((int)g.state)).c_str(), color_blue);
        gfx_text(10, 30, ("DEBUG: Run - Vies=" + std::to_string(g.lives)).c_str(), color_blue);
        gfx_text(10, 50, ("DEBUG: Run - Nb balles=" + std::to_string(g.balls.size())).c_str(), color_blue);
		gfx_flush();              // force l’affichage immédiat
		vTaskDelay(500 / portTICK_PERIOD_MS); // ~0,5 seconde de pause
    } */

    // Mise à jour des balles
    for(auto& b : g.balls){
		if (b.active) {
			// Sauvegarde de la position précédente
			int oldBallBottom = b.y + b.size;

			// Mise à jour de la balle
			ball_update(b);

			// Collision prédictive avec la raquette
			if (collision_with_paddle(b, g.paddle, oldBallBottom)) {
				if (g.paddle.sticky) {
					b.active = false;
					b.x = g.paddle.x + g.paddle.w/2 - BALL_SIZE/2;
					b.y = g.paddle.y - BALL_SIZE;
				} else {
					b.vy = -fabs(b.vy);

					// Ajustement horizontal selon point d’impact
					float hitPos = (b.x + b.size/2) - (g.paddle.x + g.paddle.w/2);
					b.vx += hitPos / (g.paddle.w/2);
				}
			}
		}
		
		// suppression des balles inactives
		g.balls.erase(std::remove_if(g.balls.begin(), g.balls.end(),
									 [](const Ball& b){ return !b.active; }),
					  g.balls.end());
    }
	
	// si la dernière balle est perdue → décrémenter les vies
	if (g.balls.empty()) {
		g.lives--;
		g.state = GameState::State::WaitingBall;
		if (debug) gfx_text(10, 90, "DEBUG: Vie perdue", color_green);
	}
		
    // Ball–brick collisions
	bool allDestroyed = true;
    for (auto& b : g.balls) {
        for (auto& brick : g.level.bricks) {
            if (!brick.alive) continue;
			allDestroyed = false;
            if (collision_ball_brick(b, brick)) {
                // Damage and removal
                brick.hp--;
                if (brick.hp <= 0) {
                    brick.alive = false;
                    g.score += 100;

                    // Random power-up: 1 chance out of 6 (same as before)
                    if (rand() % 6 == 0) {
                        PowerType type = choose_random_powerup();

                        PowerUp p;
                        p.x = static_cast<float>(brick.x);
                        p.y = static_cast<float>(brick.y);
                        p.type = type;
                        p.animFrame = 0;
                        p.timer = 0;
                        p.active = true;
                        g.falling.push_back(p);
                        // If you add a constructor to PowerUp, you can switch to:
                        // g.falling.emplace_back(p.x, p.y, type, 0, 0, true);
                    }
                }

                // Simple bounce: invert vertical velocity, then stop checking more bricks
                b.vy = -b.vy;
                break;
            }
        }
		
		if (debug) {
			gfx_text(10, 10, ("DEBUG: Run - Etat=" + std::to_string((int)g.state)).c_str(), color_blue);
			gfx_text(10, 30, ("DEBUG: Run - Vies=" + std::to_string(g.lives)).c_str(), color_blue);
			gfx_text(10, 50, ("DEBUG: Run - Nb balles=" + std::to_string(g.balls.size())).c_str(), color_blue);
			gfx_flush();              // force l’affichage immédiat
			vTaskDelay(500 / portTICK_PERIOD_MS); // ~0,5 seconde de pause
		}
    }

	if (allDestroyed) {
		g.levelIndex++;
		g.level.generate_grid(BRICK_ROWS, BRICK_COLS, 2); // ou paramètres dynamiques
		g.balls.clear();
		g.state = GameState::State::WaitingBall;
	}

    // Powerups qui tombent
    for(auto& p : g.falling){
        if(p.active){
            p.y += 2; // vitesse de chute
            p.animFrame = (p.animFrame+1)%4;
            if (collision_powerup_paddle(p, g.paddle)) {
				powerup_apply(g, p);
			}
            if(p.y>SCREEN_H) p.active=false;
        }
    }
    g.falling.erase(std::remove_if(g.falling.begin(), g.falling.end(),
                                   [](const PowerUp& p){return !p.active;}), g.falling.end());

    // Laser : tir si actif
    if(g.paddle.laser && (k.A)){
        Projectile shot{(float)(g.paddle.x + g.paddle.w/2),
                (float)g.paddle.y,
                0.0f, -3.0f,   // vx, vy
                2, 6,          // w, h
                true};         // active
        g.shots.push_back(shot);
    }

    // Mise à jour projectiles
    for(auto& s : g.shots){
        if(s.active){
            s.y -= 4;
            if(s.y<0) s.active=false;
            for(auto& brick : g.level.bricks){
                if(brick.alive && collision_projectile_brick(s,brick)){
                    brick.hp--;
                    if(brick.hp<=0){
                        brick.alive=false;
                        g.score+=100;
                    }
                    s.active=false;
                    break;
                }
            }
        }
    }
    g.shots.erase(std::remove_if(g.shots.begin(), g.shots.end(),
                                 [](const Projectile& s){return !s.active;}), g.shots.end());

    // Relance Sticky
    if(g.paddle.sticky && (k.A)){
        for(auto& b : g.balls){
            if(!b.active){
                b.active=true;
                b.vx=2.6f; b.vy=-2.6f;
            }
        }
        g.paddle.sticky=false;
    }
}


///--------------------------------
// --- Dessin ---
///--------------------------------

void game_draw(const GameState& g){
    
    lcd_clear(color_black);
	graphics_basic gfx;   // création d’un objet pour le debug

    // Paddle
    lcd_draw_bitmap(paddle_pixels, g.paddle.w, g.paddle.h, g.paddle.x, g.paddle.y);
    
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
