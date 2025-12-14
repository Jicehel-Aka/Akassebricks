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
#include "lib/sdcard.h"
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

int volume = 128; // valeur initiale 	
struct ColorDef {
    const char* name;
    uint16_t value;
};

// Vérification de la palette 
void lcd_test_colors() {
    ColorDef colors[] = {
        {"WHITE", COLOR_WHITE}, {"GRAY", COLOR_GRAY}, {"DARKGRAY", COLOR_DARKGRAY},
        {"BLACK", COLOR_BLACK}, {"PURPLE", COLOR_PURPLE}, {"PINK", COLOR_PINK},
        {"RED", COLOR_RED}, {"ORANGE", COLOR_ORANGE}, {"BROWN", COLOR_BROWN},
        {"BEIGE", COLOR_BEIGE}, {"YELLOW", COLOR_YELLOW}, {"LIGHTGREEN", COLOR_LIGHTGREEN},
        {"GREEN", COLOR_GREEN}, {"DARKBLUE", COLOR_DARKBLUE}, {"BLUE", COLOR_BLUE},
        {"LIGHTBLUE", COLOR_LIGHTBLUE}, {"SILVER", COLOR_SILVER}, {"GOLD", COLOR_GOLD}
    };

    lcd_clear(COLOR_BLACK);
    graphics_basic gfx;

    int rect_w = 40, rect_h = 15, spacing_y = 25;
    int col_x[2] = {10, 120};   // deux colonnes
    int col = 0, row = 0;

    for (int i = 0; i < sizeof(colors)/sizeof(colors[0]); i++) {
        int x = col_x[col];
        int y = 10 + row * spacing_y;

        gfx.setColor(colors[i].value);
        gfx.fillRect(x, y, rect_w, rect_h);
        gfx_text(x + rect_w + 5, y + rect_h/2, colors[i].name, COLOR_WHITE);

        // Avancer dans la colonne
        row++;
        if (row >= 9) { // 9 lignes max par colonne
            row = 0;
            col++;
        }
    }

    lcd_refresh();
}


extern "C" void app_main() {
		
    // Initialisation matériel
    lcd_init_pwm();
    lcd_update_pwm(64);
    adc_init();
    expander_init();
    LCD_init();
	sd_init();
    audio_init();
	audio_game_init();   // enregistre toutes les pistes
    audio_set_volume(volume);
    input_init();
    init_assets();
	highscores_init(); // crée le fichier AKAsseBrick.sco si absent
	snd_level_start.play_tone(440.0f, 250);
    GameState g;
    g.state = GameState::State::Title;   // état initial choisi ici

    while (true) {
        Keys k;
        input_poll(k);   // lecture des touches
		if (debug) gfx_text(10, 10, ("State=" + std::to_string((int)g.state)).c_str(), color_white);
		player.pool();   // permet de jouer les sons

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
				snd_level_start.play_tone(523.0, 80);
                snd_level_start.play_tone(659.0, 80);
                snd_level_start.play_tone(784.0, 120);
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
			if (k.MENU) {
				g.state = GameState::State::Options;
			}
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
				snd_gameover.play_tone(130.0, 400);
				highscores_submit(g.score);
				g.state = GameState::State::Highscores;
			} 
			// Test palette
			// lcd_test_colors();
			break;
			
			case GameState::State::Options:
				gfx_clear(color_black);
				gfx_text(20, 80,  "=== Options ===", color_yellow);
				gfx_text(20, 100, ("Volume: " + std::to_string(volume)).c_str(), color_white);
				gfx_text(20, 120, "LEFT/RIGHT pour regler", color_white);
				gfx_text(20, 140, "A pour consulter les scores", color_white);
				gfx_text(20, 160, "B pour retour", color_yellow);
				gfx_flush();

				if (k.left && volume > 0) volume -= 8;
				if (k.right && volume < 255) volume += 8;
				audio_set_volume(volume);

				// Nouvelle option : afficher les scores
				if (k.A) {
					highscores_show();
					// Attente d'une touche pour revenir aux options
					while (true) {
						Keys ks;
						input_poll(ks);
						if (ks.B) break;
						vTaskDelay(pdMS_TO_TICKS(100));
					}
				}

				if (k.B) g.state = GameState::State::Title;
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
				gfx_text(50, 160, "Appuyez sur A pour lancer", color_yellow);
				gfx_flush();   // force l’affichage immédiat

				Keys k2;
				input_poll(k2);

				if (k2.A) {
					launch_ball(g);
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
				snd_gameover.play_tone(130.0, 400);
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
