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
#include "gb_common.h"
#include "driver/gpio.h"

#include <esp_ota_ops.h>
#include <esp_partition.h>
#include "esp_timer.h"

// Materiel : composant standard gamebuino
#include "gb_core.h"
#include "core/sdcard.h"

// Core
#include "core/input.h"
#include "core/graphics.h"
#include "core/audio.h"
#include "core/i18n.h"
#include "core/settings.h"
#include "ui/menu.h"

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


gb_core g_core;   // instance unique du materiel (proprietaire I2C+ADC)

// Timer combo retour loader
static uint32_t combo_start = 0;

// Retour au loader si RUN + MENU maintenus 500 ms.
// On REUTILISE l'etat deja lu par input_poll() : rappeler expander_read() ici
// ferait deux lecteurs concurrents sur le bus I2C (source de timeouts).
static void checkReturnToLoader(const Keys& k)
{
    bool run_held  = k.RUN;
    bool menu_held = k.MENU;

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

    // Initialisation materielle : tout passe par le composant standard.
    // g_core.init() met en place ecran, bus I2C, ADC, audio et peripheriques.
    g_core.init();
    gfx_init();                 // retro-eclairage + cadence d'affichage

    // Calibration du joystick : g_core.init() lit deja l'ADC, on fige le centre
    // maintenant que le stick est au repos, sinon get_x()/get_y() seraient
    // decentres (derive de la raquette au demarrage).
    g_core.joystick.calibrate_center();

    bool sd_ok = sd_init();
    printf("app_main: sd_init() returned %s\n", sd_ok ? "true" : "false");

    // Reglages persistants (langue + volume) depuis la carte SD. Doit venir
    // APRES le montage SD (fait par g_core.init()) et AVANT audio_set_volume,
    // pour que le volume enregistre s'applique des le demarrage.
    settings_load();

    audio_game_init();          // enregistre la piste SFX aupres du player
    audio_set_volume(volume);
	printf("app_main: before input_init\n");
    input_init();
	printf("app_main: before init_assets\n");
    init_assets();
	printf("app_main: before highscores_init\n");
	highscores_init();
	printf("app_main: before play_tone\n");
	snd_level_start.play_tone(440.0f, 250);
	printf("app_main: init sequence complete, entering main loop\n");
	srand((unsigned)esp_timer_get_time());   // sinon meme suite de power-ups a chaque boot
    GameState g;

    while (true) {
        // Debut de frame : sert au limiteur de cadence en bas de boucle, pour
        // que chaque iteration dure une periode CONSTANTE (physique stable,
        // independante du temps de rendu).
        const uint32_t frame_start = esp_timer_get_time() / 1000;

        Keys k;
        input_poll(k);


        // Combo loader global
        checkReturnToLoader(k);

        // MENU (hors combo RUN+MENU) : court = ouvrir le menu, long = capture.
        // On distingue au RELACHEMENT : si MENU a ete tenu >= 500 ms, la capture
        // a deja ete faite, on n'ouvre pas le menu ; sinon appui court -> menu.
        static uint32_t menu_press_start = 0;
        static bool     menu_shot_done   = false;
        if (k.MENU && !k.RUN) {
            uint32_t now = esp_timer_get_time() / 1000;
            if (!menu_press_start) { menu_press_start = now; menu_shot_done = false; }
            else if (!menu_shot_done && now - menu_press_start >= 500) {
                menu_shot_done = true;
                char shot[64];
                if (gfx_save_screenshot_bmp(shot, sizeof shot)) {
                    printf("Screenshot: %s\n", shot);
                    // Confirmation a l'ecran (par-dessus l'image deja capturee,
                    // donc absente du BMP), maintenue brievement.
                    gfx_text(60, 115, i18n::T(i18n::STR_SHOT_SAVED), color_yellow);
                    gfx_flush();
                    vTaskDelay(pdMS_TO_TICKS(900));
                }
            }
        } else {
            if (menu_press_start && !menu_shot_done) {
                // Appui court relache -> menu modal.
                bool in_game = (g.state == GameState::State::Playing ||
                                g.state == GameState::State::Paused  ||
                                g.state == GameState::State::WaitingBall);
                MenuAction act = menu_open(in_game);
                if (act == MenuAction::StartGame) {
                    game_init(g);
                    g.state = GameState::State::WaitingBall;   // lancement unifie
                    gfx_clear(color_black); gfx_flush();
                } else if (act == MenuAction::ReturnTitle) {
                    g.state = GameState::State::Title;
                }
                // Resume : on ne touche pas a l'etat courant.
            }
            menu_press_start = 0;
            menu_shot_done   = false;
        }

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
                g.state = GameState::State::WaitingBall;   // lancement unifie via WaitingBall
                gfx_clear(color_black);
                gfx_flush();
            }
            // MENU est gere globalement (ouvre le menu moderne) : plus de
            // transition vers l'ancien etat Options ici.
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

        // (Etat Options retire : remplace par le menu moderne ouvert via MENU.)


        case GameState::State::Paused:
            gfx_text(20, 150, i18n::T(i18n::STR_PAUSE), color_yellow);
            gfx_text(20, 170, i18n::T(i18n::STR_PRESS_A_RESUME), color_white);
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
            game_draw(g, false);                   // dessine SANS presenter (evite le double refresh)
            gfx_text(50, 160, i18n::T(i18n::STR_PRESS_A_START), color_yellow);
            gfx_flush();                            // un SEUL refresh -> plus de dechirure (bande noire)
            // Lancement sur FRONT de A (via le k deja lu en haut de boucle).
            // L'appui qui a demarre la partie est deja "consomme" : il faut donc
            // relacher puis re-presser A -> plus de lancement immediat non voulu.
            if (k.pressed & EXPANDER_KEY_A) {
                launch_ball(g);
            }
            break;

        case GameState::State::Highscores:
            highscores_show();
            // Sortie sur FRONT B : un B encore enfonce (ex. apres saisie du nom)
            // ne doit pas faire quitter l'ecran avant meme de l'avoir vu.
            if (k.pressed & EXPANDER_KEY_B) {
                g.state = GameState::State::Title;
            }
            break;

        case GameState::State::GameOver:
            gfx_text(20, 150, i18n::T(i18n::STR_GAMEOVER), color_red);
            gfx_text(20, 170, i18n::T(i18n::STR_PRESS_A_RETRY), color_white);
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

        // Limiteur de cadence : periode FIXE de 30 ms (~33 Hz). L'ancienne boucle
        // durait "travail + 20 ms" (souvent ~30 ms) : on vise donc 30 ms pour
        // garder la MEME vitesse ressentie, mais desormais CONSTANTE quel que soit
        // le temps de rendu (fini la balle qui accelere quand le rendu est leger).
        // Si la balle te parait trop rapide/lente, ajuste FRAME_MS (plus grand =
        // plus lent) ou BALL_SPEED_INIT.
        constexpr uint32_t FRAME_MS = 30;
        const uint32_t work_ms = (esp_timer_get_time() / 1000) - frame_start;
        if (work_ms < FRAME_MS)
            vTaskDelay(pdMS_TO_TICKS(FRAME_MS - work_ms));
        else
            vTaskDelay(1);   // cede toujours la main au moins un tick
    }
}
