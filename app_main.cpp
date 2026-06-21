/**
 * @file app_main.cpp
 * @brief Point d'entrée principal du jeu AKAsseBricks pour la console AKA (ESP32‑S3).
 *
 * Ce module gère :
 *   - L’initialisation du matériel (LCD, PWM, ADC, expander, SD, audio).
 *   - L’initialisation des couches core (input, graphics, audio, persist).
 *   - Le chargement des assets et du système de highscores.
 *   - La machine à états principale du jeu (Title, Playing, Paused, Options, Highscores, GameOver).
 *   - Le retour au loader OTA via :
 *         • Combo RUN + MENU (global, à tout moment)
 *         • Option dédiée dans le menu Options
 *   - L’accès aux highscores depuis le menu Options.
 *
 * Notes :
 *   - Le fichier title_screen.cpp gère l’affichage de l’écran titre.
 *   - highscores.cpp gère la lecture/écriture du fichier AKAsseBricks.sco sur la SD.
 *   - Toute nouvelle fonctionnalité globale doit être intégrée dans cette machine à états.
 */

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

#include <esp_ota_ops.h>
#include <esp_partition.h>
#include "esp_timer.h"

// Librairies matérielles
#include "lib/expander.h"
#include "lib/LCD.h"
#include "lib/graphics_basic.h"
#include "lib/audio.h"
#include "lib/audio_basic.h"
#include "lib/sdcard.h"

// Core
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
#include "nvs_flash.h"

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
    int col_x[2] = {10, 120};
    int col = 0, row = 0;

    for (int i = 0; i < sizeof(colors)/sizeof(colors[0]); i++) {
        int x = col_x[col];
        int y = 10 + row * spacing_y;

        gfx.setColor(colors[i].value);
        gfx.fillRect(x, y, rect_w, rect_h);
        gfx_text(x + rect_w + 5, y + rect_h/2, colors[i].name, COLOR_WHITE);

        row++;
        if (row >= 9) {
            row = 0;
            col++;
        }
    }

    lcd_refresh();
}

// Timer combo retour loader
static uint32_t combo_start = 0;

// Retour au loader si RUN + MENU maintenus 500 ms
static void checkReturnToLoader()
{
    uint16_t raw = expander_read();

    bool run_held  = raw & EXPANDER_KEY_RUN;
    bool menu_held = raw & EXPANDER_KEY_MENU;

    if (run_held && menu_held) {
        uint32_t now = esp_timer_get_time() / 1000; // ms

        if (!combo_start) {
            combo_start = now;
        } else if (now - combo_start >= 500) {
            combo_start = 0;

            const esp_partition_t* loader = esp_partition_find_first(
                ESP_PARTITION_TYPE_APP,
                ESP_PARTITION_SUBTYPE_APP_OTA_1,
                nullptr
            );

            if (loader) {
                esp_ota_set_boot_partition(loader);
                esp_restart();
            }
        }
    } else {
        combo_start = 0;
    }
}

extern "C" void app_main() {

    // Initialisation matériel
    lcd_init_pwm();
    lcd_update_pwm(64);
    adc_init();
    expander_init();
    LCD_init();

	bool sd_ok = sd_init();
	printf("app_main: sd_init() returned %s\n", sd_ok ? "true" : "false");

	// Test : charge un WAV depuis la SD pour valider la chaine SD -> FIFO -> I2S.
	// Doit etre appele AVANT audio_init(), car audio_init() remplit le FIFO
	// (wav_pool_update) dès son tout premier appel.
	if (open_file("/sdcard/PAKAMAN/Sons/Bonus.wav") != ESP_OK) {
		printf("audio_test: impossible d'ouvrir le fichier WAV de test\n");
	}

    audio_init();
	audio_game_init();   // enregistre toutes les pistes
    audio_set_volume(volume);
	printf("app_main: before input_init\n");
    input_init();
	printf("app_main: before init_assets\n");
    init_assets();
	printf("app_main: before highscores_init\n");
	highscores_init(); // crée le fichier AKAsseBrick.sco si absent
	printf("app_main: before play_tone\n");
	snd_level_start.play_tone(440.0f, 250);
	printf("app_main: init sequence complete, entering main loop\n");
    GameState g;

    while (true) {
        Keys k;
        input_poll(k);
        player.pool();

        // Combo loader global
        checkReturnToLoader();

        if (debug)
            gfx_text(10, 10, ("State=" + std::to_string((int)g.state)).c_str(), color_white);

        switch (g.state) {

        case GameState::State::Title:
            title_screen_show();
            if (debug)
                gfx_text(10, 10, "DEBUG: Etat=Title", color_yellow);

            if (k.A) {
                snd_level_start.play_tone(523.0, 80);
                snd_level_start.play_tone(659.0, 80);
                snd_level_start.play_tone(784.0, 120);
                game_init(g);
                g.state = GameState::State::Playing;
                gfx_clear(color_black);
                gfx_flush();
            }

            if (k.MENU) {
                g.state = GameState::State::Options;
            }
            break;

        case GameState::State::Playing:
            game_update(g);
            game_draw(g);

            if (k.RUN) {
                g.state = GameState::State::Paused;
            }

            if (game_is_over(g)) {
                snd_gameover.play_tone(130.0, 400);
                highscores_submit(g.score);
                g.state = GameState::State::Highscores;
            }
            break;

        case GameState::State::Options:
            gfx_clear(color_black);
            gfx_text(20, 80,  "=== Options ===", color_yellow);
            gfx_text(20, 100, ("Volume: " + std::to_string(volume)).c_str(), color_white);
            gfx_text(20, 120, "LEFT/RIGHT pour regler", color_white);
            gfx_text(20, 140, "A pour consulter les scores", color_white);
            gfx_text(20, 160, "RUN+MENU: Retour loader", color_white);
            gfx_text(20, 180, "B pour retour titre", color_yellow);
            gfx_flush();

            if (k.left && volume > 0) volume -= 8;
            if (k.right && volume < 255) volume += 8;
            audio_set_volume(volume);

            // Afficher les scores
            if (k.A) {
                highscores_show();
                while (true) {
                    Keys ks;
                    input_poll(ks);
                    if (ks.B) break;
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
            }

            // Retour loader via menu (option explicite)
            if (k.RUN && k.MENU) {
                const esp_partition_t* loader = esp_partition_find_first(
                    ESP_PARTITION_TYPE_APP,
                    ESP_PARTITION_SUBTYPE_APP_OTA_1,
                    nullptr
                );
                if (loader) {
                    esp_ota_set_boot_partition(loader);
                    esp_restart();
                }
            }

            if (k.B) g.state = GameState::State::Title;
            break;

        case GameState::State::Paused:
            gfx_text(20, 160, "Pause - Appuyez sur A pour reprendre", color_white);
            gfx_flush();
            if (k.A) {
                g.state = GameState::State::Playing;
            }
            if (isLongPress(k, EXPANDER_KEY_RUN)) {
                g.state = GameState::State::GameOver;
            }
            lcd_refresh();
            break;

        case GameState::State::WaitingBall:
            gfx_text(50, 160, "Appuyez sur A pour lancer", color_yellow);
            gfx_flush();

            {
                Keys k2;
                input_poll(k2);
                if (k2.A) {
                    launch_ball(g);
                }
            }

            lcd_refresh();
            break;

        case GameState::State::Highscores:
            highscores_show();
            if (k.B) {
                g.state = GameState::State::Title;
            }
            break;

        case GameState::State::GameOver:
            gfx_text(20, 160, "Game Over - Appuyez sur A pour recommencer", color_red);
            snd_gameover.play_tone(130.0, 400);
            if (k.A || k.B) {
                g.state = GameState::State::Title;
            }
            lcd_refresh();
            break;

        default:
            g.state = GameState::State::Title;
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
