#include "ball.h"
#include "config.h"

void ball_update(Ball& b){
    // déplacement
    b.x += b.vx;
    b.y += b.vy;

    // rebond gauche/droite
    if (b.x < 0) {
        b.vx = -b.vx;
        b.x = 0;
    }
    if (b.x + BALL_SIZE > SCREEN_W) {
        b.vx = -b.vx;
        b.x = SCREEN_W - BALL_SIZE;
    }

    // rebond haut
    if (b.y < 0) {
        b.vy = -b.vy;
        b.y = 0;
    }

    // sortie bas → marquée inactive
    if (b.y > SCREEN_H) {
        b.active = false;
    }
}