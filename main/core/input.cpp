/*
  core/input.cpp — Adaptateur entrees AKasseBricks au-dessus de gb_core.
  UNIQUE proprietaire du bus I2C + ADC : seul input_poll() appelle g_core.pool().
  (L'ancienne version lisait l'expander ici ET dans checkReturnToLoader ->
   deux lecteurs concurrents. Le combo loader reutilise desormais Keys.)
*/
#include "input.h"
#include "gb_core.h"
#include "gb_ll_common.h"   // EXPANDER_KEY_*, JOYX_MID

extern gb_core g_core;       // instance unique, definie dans app_main

void input_init() {
    // L'init materiel (ecran, bus, peripheriques) est faite par g_core.init().
}

void input_poll(Keys& k) {
    g_core.pool();                              // lit boutons + joystick (1 seule fois)

    const uint16_t raw = g_core.buttons.state();
    k.raw      = raw;
    k.pressed  = g_core.buttons.pressed();      // fronts fournis par le SDK
    k.released = g_core.buttons.released();

    k.up    = raw & EXPANDER_KEY_UP;
    k.down  = raw & EXPANDER_KEY_DOWN;
    k.left  = raw & EXPANDER_KEY_LEFT;
    k.right = raw & EXPANDER_KEY_RIGHT;
    k.A     = raw & EXPANDER_KEY_A;
    k.B     = raw & EXPANDER_KEY_B;
    k.C     = raw & EXPANDER_KEY_C;
    k.D     = raw & EXPANDER_KEY_D;
    k.RUN   = raw & EXPANDER_KEY_RUN;
    k.MENU  = raw & EXPANDER_KEY_MENU;
    k.R1    = raw & EXPANDER_KEY_R1;
    k.L1    = raw & EXPANDER_KEY_L1;

    // get_x()/get_y() renvoient -1000..1000 (centre calibre). paddle_move attend
    // une valeur centree sur JOYX_MID (echelle ADC historique) : on reconvertit,
    // ainsi le code du jeu reste inchange.
    k.joxx = JOYX_MID + (int)((long)g_core.joystick.get_x() * JOYX_MID / 1000);
    k.joxy = JOYX_MID + (int)((long)g_core.joystick.get_y() * JOYX_MID / 1000);
}

bool isLongPress(const Keys& k, int key) {
    static int pressDuration = 0;
    if (k.raw & key) {
        if (++pressDuration > 60) { pressDuration = 0; return true; }
    } else {
        pressDuration = 0;
    }
    return false;
}
