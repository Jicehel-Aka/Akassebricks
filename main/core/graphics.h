/*
  core/graphics.h — Adaptateur graphique AKasseBricks au-dessus du composant
  gamebuino. L'API historique du jeu est conservee (gfx_*, lcd_draw_bitmap,
  lcd_draw_partial_bitmap, constantes color_*), mais tout passe desormais par
  gb_graphics / gb_ll_lcd. lcd_clear / lcd_refresh / lcd_putpixel / lcd_getpixel
  viennent directement du composant (gb_ll_lcd.h).
*/
#pragma once
#include <stdint.h>
#include "gb_ll_lcd.h"   // lcd_clear/refresh/putpixel/getpixel + enum gamebuino_color (color_white...)

// Les constantes color_white / color_black / color_red / color_yellow /
// color_green / color_lightgreen / color_darkgray viennent de gb_ll_lcd.h
// (enum gamebuino_color, valeurs identiques a l'ancien LCD.h).

extern uint16_t current_text_color;

void gfx_init();
void gfx_clear(uint16_t color);
void gfx_flush();
void gfx_set_text_color(uint16_t color);
void gfx_text(int x, int y, const char* txt, uint16_t color);
int  gfx_text_width(const char* text);
void gfx_text_center(int y, const char* text, uint16_t color);
int  gfx_char_width(char c);

// Blit d'image complete (opaque) et de sprite (sous-rect d'une sprite sheet).
// Comportement identique a l'ancienne lib : copie pixel par pixel, sans cle de
// transparence (AKasseBricks n'en utilise pas).
void lcd_draw_bitmap(const uint16_t* pixels, int w, int h, int dx, int dy);
void lcd_draw_partial_bitmap(const uint16_t* pixels,
                             int sheetW, int sheetH,
                             int sx, int sy,
                             int spriteW, int spriteH,
                             int dx, int dy);
void lcd_draw_text(uint16_t x, uint16_t y, const char* pc);

// Capture d'ecran BMP 24 bits -> /sdcard/AKBRICKS/SHOTxxxx.BMP. Renvoie le
// chemin ecrit dans out_path si fourni.
bool gfx_save_screenshot_bmp(char* out_path = nullptr, int out_path_size = 0);

// Anciennes constantes COLOR_* (conservees pour compat).
#define COLOR_BLACK      0x0000
#define COLOR_WHITE      0xFFFF
#define COLOR_DARKGRAY   0x426A
#define COLOR_YELLOW     0x073E
