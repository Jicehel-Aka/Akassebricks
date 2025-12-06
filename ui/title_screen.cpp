#include "title_screen.h"
#include "assets/title_image.h"
#include "graphics.h"
#include "input.h"
#include "expander.h"          // ✅ pour EXPANDER_KEY_A
#include "freertos/FreeRTOS.h" // ✅ pour pdMS_TO_TICKS
#include "freertos/task.h"     // ✅ pour vTaskDelay

void title_screen_show() {
    gfx_clear(COLOR_BLACK);
    // Utiliser les bons noms de constantes
    lcd_draw_bitmap(title_image_pixels, TITLE_IMAGE_WIDTH, TITLE_IMAGE_HEIGHT, 0, 0);
    gfx_text(50, 190, "Appuyez sur A pour lancer", COLOR_WHITE);
    gfx_flush();

}
