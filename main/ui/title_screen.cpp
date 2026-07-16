#include "title_screen.h"
#include "assets/title_image.h"
#include "graphics.h"
#include "input.h"
#include "core/i18n.h"
#include "config.h"
#include "graphics_basic.h"
#include "gb_ll_common.h"          // ✅ pour EXPANDER_KEY_A
#include "freertos/FreeRTOS.h" // ✅ pour pdMS_TO_TICKS
#include "freertos/task.h"     // ✅ pour vTaskDelay

void title_screen_show() {
    /* gfx_clear(COLOR_BLACK); */
    lcd_draw_bitmap(title_image_pixels, SCREEN_W ,SCREEN_H , 0, 0);
    gfx_text(50, 190, i18n::T(i18n::STR_PRESS_A_START), color_white); 
	/* graphics_basic gfx;
	gfx.setColor( COLOR_WHITE);
	gfx.fillRect(0, 0, SCREEN_W, SCREEN_H); */
    gfx_flush();

}
