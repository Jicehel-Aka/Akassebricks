/**
 * @file highscores.cpp
 * @brief Meilleurs scores AKasseBricks — stockage SD (8.3), saisie sur fronts.
 *
 * Fichier : /sdcard/AKBRICKS/SCORES.DAT  (noms 8.3 stricts : pas de dependance
 * au LFN, contrairement a l'ancien "/sdcard/AKasseBricks/AKAsseBricks.sco" dont
 * le dossier en nom long ne se creait pas toujours -> rien n'etait persiste).
 */
#include "highscores.h"
#include "core/graphics.h"
#include "core/input.h"
#include "core/audio.h"
#include "core/i18n.h"
#include "sdcard.h"
#include "gb_ll_common.h"          // EXPANDER_KEY_*

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cinttypes>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Chemin d'origine : le fichier existant sur la carte (D:\AKAsseBricks) contient
// deja des scores valides -> la sauvegarde fonctionne, on ne change pas le chemin
// (le LFN est actif : CONFIG_FATFS_LFN_HEAP=y).
#define HIGHSCORE_DIR  "/sdcard/AKAsseBricks"
#define HIGHSCORE_FILE "/sdcard/AKAsseBricks/AKAsseBricks.sco"

using i18n::T;

void highscores_init() {
    sd_mkdir(HIGHSCORE_DIR);
    if (!sd_exists(HIGHSCORE_FILE)) {
        FILE* f = fopen(HIGHSCORE_FILE, "wb");
        if (f) fclose(f);
    }
}

std::vector<HighscoreEntry> highscores_load() {
    std::vector<HighscoreEntry> scores;
    FILE* f = fopen(HIGHSCORE_FILE, "rb");
    if (!f) return scores;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    size_t count = (size >= 0) ? (size_t)(size / sizeof(HighscoreEntry)) : 0;
    scores.resize(count);
    if (count > 0) {
        size_t got = fread(scores.data(), sizeof(HighscoreEntry), count, f);
        scores.resize(got);
    }
    fclose(f);
    return scores;
}

void highscores_submit(int32_t score) {
    auto scores = highscores_load();
    std::string name = highscores_input_name();
    if (name.empty()) name = "---";     // securite : jamais d'entree vide

    HighscoreEntry entry{};
    memset(entry.name, 0, sizeof(entry.name));
    strncpy(entry.name, name.c_str(), sizeof(entry.name) - 1);
    entry.score = score;

    scores.push_back(entry);
    std::sort(scores.begin(), scores.end(),
              [](const HighscoreEntry& a, const HighscoreEntry& b){ return a.score > b.score; });
    if (scores.size() > (size_t)MAX_SCORES) scores.resize(MAX_SCORES);

    // Sauvegarde (on s'assure que le dossier existe, au cas ou).
    sd_mkdir(HIGHSCORE_DIR);
    FILE* f = fopen(HIGHSCORE_FILE, "wb");
    if (f) {
        fwrite(scores.data(), sizeof(HighscoreEntry), scores.size(), f);
        fflush(f);
        fclose(f);
        printf("highscores: %u entree(s) sauvegardee(s) pour %s\n",
               (unsigned)scores.size(), entry.name);
    } else {
        printf("highscores: ECHEC ouverture %s en ecriture\n", HIGHSCORE_FILE);
    }
}

void highscores_show() {
    auto scores = highscores_load();
    printf("highscores_show: %u entree(s) chargee(s)\n", (unsigned)scores.size());

    gfx_clear(color_black);
    gfx_text(90, 10, T(i18n::STR_HIGHSCORES), color_yellow);

    if (scores.empty()) {
        gfx_text(20, 90, T(i18n::STR_NO_SCORE), color_white);
    } else {
        int y = 50, rank = 1;
        for (const auto& e : scores) {
            if (rank > MAX_SCORES) break;
            const char* nm = (e.name[0] == '\0') ? "---" : e.name;
            char buf[64];
            snprintf(buf, sizeof(buf), "%2d. %-8s %6" PRId32, rank, nm, e.score);
            gfx_text(30, y, buf, color_white);
            y += 20; rank++;
        }
    }
    gfx_text(20, 175, T(i18n::STR_BACK), color_yellow);
    gfx_flush();
}

std::string highscores_input_name() {
    std::string name;
    const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";
    int index = 0;
    bool b_armed = false;

    for (;;) {
        Keys k;
        input_poll(k);                       // k.pressed = fronts (fournis par gb_core)

        // Navigation dans l'alphabet : sur FRONT, pas en continu.
        if (k.pressed & EXPANDER_KEY_LEFT) {
            if (--index < 0) index = (int)alphabet.size() - 1;
            snd_keypress.play_tone(880.0f, 40, 0.5f);
        }
        if (k.pressed & EXPANDER_KEY_RIGHT) {
            if (++index >= (int)alphabet.size()) index = 0;
            snd_keypress.play_tone(880.0f, 40, 0.5f);
        }
        char currentChar = alphabet[index];

        // Ajout d'une lettre (A), suppression (C) : sur FRONT.
        if ((k.pressed & EXPANDER_KEY_A) && name.size() < 8) {
            name.push_back(currentChar);
            index = 0;
            snd_keypress.play_tone(1046.0f, 45, 0.5f);
        }
        if ((k.pressed & EXPANDER_KEY_C) && !name.empty()) {
            name.pop_back();
            snd_delete.play_tone(180.0f, 70, 0.5f);
        }

        // Validation (B) : arme au front, valide au relachement, si non vide.
        if ((k.pressed & EXPANDER_KEY_B) && !name.empty()) b_armed = true;
        if ((k.released & EXPANDER_KEY_B) && b_armed) {
            snd_keypress.play_tone(660.0f, 120);
            break;
        }

        // Affichage
        gfx_clear(color_black);
        char line[32];
        snprintf(line, sizeof(line), "%s: %s%c", T(i18n::STR_NAME), name.c_str(), currentChar);
        gfx_text(20, 95, line, color_white);
        gfx_text(20, 120, T(i18n::STR_NAME_HELP), color_yellow);
        gfx_flush();

                       // indispensable pour entendre les sons
        vTaskDelay(pdMS_TO_TICKS(60));
    }
    return name;
}
