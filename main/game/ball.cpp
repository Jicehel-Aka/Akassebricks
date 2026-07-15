#include "ball.h"
#include "config.h"
#include "audio.h"

void ball_update(Ball& b){
    // déplacement
    b.x += b.vx;
    b.y += b.vy;

    // rebond gauche/droite
    if (b.x < 0) {
        b.vx = -b.vx;
        b.x = 0;
		snd_wall.play_tone(784.0, 60); // son rebond mur
    }
    if (b.x + BALL_SIZE > SCREEN_W) {
        b.vx = -b.vx;
        b.x = SCREEN_W - BALL_SIZE;
		snd_wall.play_tone(784.0, 60); // son rebond mur
    }

    // rebond haut
    if (b.y < 0) {
        b.vy = -b.vy;
        b.y = 0;
		snd_wall.play_tone(784.0, 60); // son rebond plafond
    }

    // sortie bas → marquée inactive
    if (b.y > SCREEN_H) {
        b.active = false;
    }
}