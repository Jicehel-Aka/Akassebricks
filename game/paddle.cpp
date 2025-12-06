#include "paddle.h"
#include "config.h"

void paddle_move(Paddle& p, bool left, bool right){
    if(left) p.x -= PADDLE_SPEED;
    if(right) p.x += PADDLE_SPEED;
    if(p.x<0) p.x=0;
    if(p.x>SCREEN_W-p.w) p.x=SCREEN_W-p.w;
}
