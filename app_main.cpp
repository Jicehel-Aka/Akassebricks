	#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_psram.h"
#include "common.h"
#include "driver/gpio.h" 

// Librairies matérielles
#include "lib/expander.h"
#include "lib/LCD.h"
#include "lib/graphics_basic.h"
#include "lib/audio.h"
#include "lib/audio_basic.h"
#include "core/input.h"
#include "core/graphics.h"
#include "core/audio.h" 
#include "core/persist.h" 

// Composants du jeu 
#include "game/config.h" 
#include "ui/title_screen.h" 
#include "ui/menu.h" 
#include "ui/highscores.h" 
#include "ui/hud.h" 
#include "game/game.h" 
#include "game/level_editor.h" 
#include "assets/assets.h"

extern "C" void app_main() {
		
    // Initialisation matériel
    lcd_init_pwm();
    lcd_update_pwm(64);
    adc_init();
    expander_init();
    LCD_init();

    audio_init();
    audio_set_volume(128);
    input_init();
    init_assets();

    GameState g;
    g.state = GameState::State::Title;   // état initial choisi ici

    while (true) {
        Keys k;
        input_poll(k);   // lecture des touches
		if (debug) gfx_text(10, 10, ("State=" + std::to_string((int)g.state)).c_str(), color_white);

        switch (g.state) {
			
		case GameState::State::Title:
			title_screen_show();
			if (debug) {
				gfx_text(10, 10, "DEBUG: Etat=Title", color_yellow);
			}
			if (k.A) {
				if (debug) { 
					gfx_text(10, 30, "DEBUG: A detecte -> Playing", color_yellow);
					gfx_flush();              // force l’affichage immédiat
					vTaskDelay(500 / portTICK_PERIOD_MS); // ~0,5 seconde de pause
				}
				game_init(g);
				g.state = GameState::State::Playing;
				gfx_clear(color_black);
				gfx_flush();
			}
			/* if (k.MENU) {
				//test_paddle_visual();
				test_paddle_column();
				vTaskDelay(1000 / portTICK_PERIOD_MS); 
			} */	
			break;

		case GameState::State::Playing:
			game_update(g);
			game_draw(g);
			if (debug) {
				gfx_text(10, 10, ("DEBUG: Ppst draw - Vies=" + std::to_string(g.lives)).c_str(), color_yellow);
				gfx_text(10, 30, ("DEBUG: Post draw - Nb balles=" + std::to_string(g.balls.size())).c_str(), color_yellow);
				gfx_flush();              // force l’affichage immédiat
				vTaskDelay(500 / portTICK_PERIOD_MS); // ~0,5 seconde de pause
			}
			// hud_draw(g);

			if (k.RUN) {
				if (debug) gfx_text(10, 50, "DEBUG: RUN detecte -> Pause", color_yellow);
				g.state = GameState::State::Paused;
			}
			if (game_is_over(g)) {
				if (debug) {
					gfx_text(10, 70, "DEBUG: game_is_over -> Highscores", color_yellow);
					gfx_flush();              // force l’affichage immédiat
					vTaskDelay(500 / portTICK_PERIOD_MS); // ~0,5 seconde de pause
				}
				highscores_submit(g.score);
				g.state = GameState::State::Highscores;
			}
			break;

            case GameState::State::Paused:
                gfx_text(20, 160, "Pause - Appuyez sur A pour reprendre", color_white);
                gfx_flush();
                if (k.A) {
                    g.state = GameState::State::Playing; // reprise
                }
				// Vérifier si c'est un appui long sur RUN
				if (isLongPress(k, EXPANDER_KEY_RUN)) {
					g.state = GameState::State::GameOver;
				}
				lcd_refresh();
                break;
				
			case GameState::State::WaitingBall:
                // Affiche message relance
                gfx_text( 50, 160, "Appuyez sur A pour lancer", color_yellow);
                Keys k2;
                input_poll(k2);
                if (k2.A) {
                    float bx = g.paddle.x + g.paddle.w/2.0f - BALL_SIZE/2.0f;
                    float by = g.paddle.y - BALL_SIZE;
                    g.balls.push_back({bx, by, BALL_SPEED_INIT, -BALL_SPEED_INIT, true});
                    g.state = GameState::State::Playing;
                }
				lcd_refresh();
                break;	

            case GameState::State::Highscores:
                highscores_show();
				if (debug) gfx_text(10, 10, "DEBUG: Etat=GameOver", color_yellow);
                if (k.B) {
                    g.state = GameState::State::Title; // retour menu
                }
                break;

			case GameState::State::GameOver:
				gfx_text(20, 160, "Game Over - Appuyez sur A pour recommencer", color_red);
				if (debug) gfx_text(10, 10, "DEBUG: Etat=GameOver", color_yellow);

				// Attente d'une touche pour revenir au titre
				if (k.A || k.B) {
					g.state = GameState::State::Title;
				}
				lcd_refresh();
				break;

            default:
                g.state = GameState::State::Title;
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(20)); // petite pause pour éviter 100% CPU
    }
} 



// Emple de bascule pour un débugage visuel en vérifiant par exemple que les briques sont bien récupérées

/* extern void test_bricks_visual(); */

/* extern "C" void app_main(void) {
	// Initialisation matériel
    lcd_init_pwm();
    lcd_update_pwm(64);
    adc_init();
    expander_init();
    LCD_init();

    audio_init();
    audio_set_volume(128);
    input_init();
*//*
    init_assets(); // charge atlas_bricks
    test_bricks_visual(); // lance le test visuel */
	
 /*   // 1. On force la couleur active à rouge
    gfx_set_text_color(COLOR_RED);

    // 2. On dessine un seul caractère
    lcd_draw_char(10, 160, 'A');

    // 3. On log la valeur de la variable globale
    printf("current_text_color = 0x%04X\n", current_text_color);

    // 4. On log quelques pixels du framebuffer autour du caractère
    for (int i = 0; i < 16; i++) {
        lcd_printf("fb[%d] = 0x%04X\n", (160*320+10)+i,
               framebuffer[(160*320+10)+i]);
    }

    // 5. On flush pour voir le résultat à l’écran
    gfx_flush();
	
    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
} 
*/
