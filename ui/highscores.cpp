/**
 * @file highscores.cpp
 * @brief Gestion des meilleurs scores pour le jeu AKAsseBricks.
 *
 * Ce module gère :
 *   - L’initialisation du fichier de scores (création si absent).
 *   - Le chargement des scores depuis la carte SD.
 *   - L’ajout d’un nouveau score (tri + sauvegarde).
 *   - L’affichage du tableau des scores.
 *   - La saisie interactive du nom du joueur.
 *
 * Le fichier est stocké sur la carte SD, dans :
 *      /sdcard/AKBricks/AKAsseBricks.sco
 *
 * Le support des noms longs (LFN) est assuré par le module sdcard.c
 * et nécessite les options FATFS activées dans sdkconfig.
 */

#include "highscores.h"
#include "core/graphics.h"
#include "core/input.h"
#include "lib/LCD.h"
#include "lib/graphics_basic.h"
#include "lib/sdcard.h"
#include "core/audio.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cinttypes>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// -----------------------------------------------------------------------------
//  Emplacement du fichier de scores
// -----------------------------------------------------------------------------
#define HIGHSCORE_DIR  "/sdcard/AKasseBricks"
#define HIGHSCORE_FILE "/sdcard/AKasseBricks/AKAsseBricks.sco"

// -----------------------------------------------------------------------------
//  highscores_init()
// -----------------------------------------------------------------------------
//  Initialise le système de scores :
//    - crée le dossier si nécessaire,
//    - crée un fichier vide si absent.
// -----------------------------------------------------------------------------
void highscores_init() {
    printf("highscores_init: start\n");
    ESP_LOGI("HIGHSCORES", "Initialisation du système de scores");

    // Crée le dossier si nécessaire
    printf("highscores_init: before sd_mkdir\n");
    sd_mkdir(HIGHSCORE_DIR);
    printf("highscores_init: after sd_mkdir\n");

    // Crée le fichier s'il n'existe pas
    printf("highscores_init: before sd_exists\n");
    if (!sd_exists(HIGHSCORE_FILE)) {
        printf("highscores_init: file missing, creating it\n");
        ESP_LOGI("HIGHSCORES", "Création du fichier %s", HIGHSCORE_FILE);
        FILE* f = fopen(HIGHSCORE_FILE, "wb");
        printf("highscores_init: fopen returned %p\n", (void*)f);
        if (f) fclose(f);
    }
    printf("highscores_init: done\n");
}

// -----------------------------------------------------------------------------
//  highscores_load()
// -----------------------------------------------------------------------------
//  Charge tous les scores depuis le fichier binaire.
//  Chaque entrée est un HighscoreEntry (nom + score).
//
//  Retour : vecteur contenant les scores chargés.
// -----------------------------------------------------------------------------
std::vector<HighscoreEntry> highscores_load() {
    ESP_LOGI("HIGHSCORES", "Chargement depuis %s", HIGHSCORE_FILE);

    std::vector<HighscoreEntry> scores;
    FILE* f = fopen(HIGHSCORE_FILE, "rb");
    if (!f) return scores;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    size_t count = (size >= 0) ? (size_t)(size / sizeof(HighscoreEntry)) : 0;
    scores.resize(count);

    if (count > 0) {
        fread(scores.data(), sizeof(HighscoreEntry), count, f);
    }

    fclose(f);
    return scores;
}

// -----------------------------------------------------------------------------
//  highscores_submit()
// -----------------------------------------------------------------------------
//  Ajoute un nouveau score :
//    - demande le nom du joueur,
//    - ajoute l’entrée,
//    - trie par score décroissant,
//    - limite à MAX_SCORES,
//    - sauvegarde dans le fichier.
// -----------------------------------------------------------------------------
void highscores_submit(int32_t score) {
    auto scores = highscores_load();
    std::string name = highscores_input_name();

    HighscoreEntry entry{};
    memset(entry.name, 0, sizeof(entry.name));
    strncpy(entry.name, name.c_str(), sizeof(entry.name)-1);
    entry.score = score;

    ESP_LOGI("HIGHSCORES", "Ajout score %d pour %s", score, entry.name);

    scores.push_back(entry);

    // Tri décroissant
    std::sort(scores.begin(), scores.end(),
              [](const HighscoreEntry& a, const HighscoreEntry& b){
                  return a.score > b.score;
              });

    // Limite à MAX_SCORES
    if (scores.size() > MAX_SCORES)
        scores.resize(MAX_SCORES);

    // Sauvegarde
    FILE* f = fopen(HIGHSCORE_FILE, "wb");
    if (f) {
        fwrite(scores.data(), sizeof(HighscoreEntry), scores.size(), f);
        fclose(f);
    }
}

// -----------------------------------------------------------------------------
//  highscores_show()
// -----------------------------------------------------------------------------
//  Affiche le tableau des scores à l’écran.
// -----------------------------------------------------------------------------
void highscores_show() {
    auto scores = highscores_load();

    gfx_clear(color_black);
    gfx_text(80, 10, "=== Highscores ===", color_yellow);

    if (scores.empty()) {
        gfx_text(20, 80, "Aucun score enregistre.", color_white);
    } else {
        int y = 50;
        int rank = 1;

        for (const auto& e : scores) {
            if (rank > MAX_SCORES) break;
            if (e.name[0] == '\0') continue;

            char buf[64];
            snprintf(buf, sizeof(buf), "%2d. %-8s %6" PRId32,
                     rank, e.name, e.score);

            gfx_text(20, y, buf, color_white);
            y += 20;
            rank++;
        }
    }

    gfx_text(20, 170, "Appuyez sur <B> pour revenir au menu.", color_yellow);
    gfx_flush();
}

// -----------------------------------------------------------------------------
//  highscores_input_name()
// -----------------------------------------------------------------------------
//  Saisie interactive du nom du joueur :
//    - navigation gauche/droite dans l’alphabet,
//    - validation d’un caractère avec A,
//    - suppression avec C,
//    - validation finale avec B (press → release).
//
//  Retour : nom choisi (max 8 caractères).
// -----------------------------------------------------------------------------
std::string highscores_input_name() {
    std::string name;
    char currentChar = 'A';
    const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    int index = 0;

    bool b_pressed = false;

    while (true) {
        Keys k;
        input_poll(k);

        // Navigation alphabet
        if (k.left) {
            if (--index < 0) index = alphabet.size()-1;
            currentChar = alphabet[index];
            snd_keypress.play_tone(880.0f, 50);
        }
        if (k.right) {
            if (++index >= alphabet.size()) index = 0;
            currentChar = alphabet[index];
            snd_keypress.play_tone(880.0f, 50);
        }

        // Ajout d’un caractère
        if (k.A && name.size() < 8) {
            name.push_back(currentChar);
            index = 0;
            currentChar = 'A';
            snd_keypress.play_tone(880.0f, 50);
        }

        // Suppression
        if (k.C && !name.empty()) {
            name.pop_back();
            snd_delete.play_tone(110.0f, 80);
        }

        // Validation par B (press → release)
        if (k.B && !name.empty()) {
            b_pressed = true;
        }
        if (!k.B && b_pressed) {
            snd_keypress.play_tone(660.0f, 120);
            break;
        }

        // Affichage
        gfx_clear(color_black);
        gfx_text(20, 100, ("Pseudo: " + name + currentChar).c_str(), color_white);
        gfx_text(20, 120, "A=Valider char, C=Effacer, B=OK", color_yellow);
        gfx_flush();

        player.pool();   // indispensable pour entendre les sons
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    return name;
}
