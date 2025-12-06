// hud.cpp
#include "hud.h"
#include "game/config.h"       // pour GameState et debug
#include "core/graphics.h"     // pour gfx_text, gfx_flush
#include "lib/LCD.h"           // pour les constantes de couleur (color_white, color_yellow)
#include "freertos/FreeRTOS.h" // pour portTICK_PERIOD_MS
#include "freertos/task.h"     // pour vTaskDelay
#include <cstdio>              // pour std::snprintf

void hud_draw(const GameState& g) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "Score: %d", g.score);
	// Forcer l’affichage immédiat
    gfx_flush();

    // Petite attente pour avoir le temps de voir le message
    vTaskDelay(500 / portTICK_PERIOD_MS); // 0,5 seconde

    // Affichage du score en haut à gauche
    gfx_text(4, 4, buf, color_white);

    // Debug optionnel : afficher aussi les vies et le nombre de balles
    if (debug) {
        char dbg[64];
        std::snprintf(dbg, sizeof(dbg), "Vies=%d  Balles=%zu", g.lives, g.balls.size());
        gfx_text(4, 20, dbg, color_yellow);

        // Forcer l’affichage immédiat
        gfx_flush();

        // Petite attente pour avoir le temps de voir le message
        vTaskDelay(500 / portTICK_PERIOD_MS); // 0,5 seconde
    }
}
