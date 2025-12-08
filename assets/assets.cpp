#include "assets.h"
#include "spritesheet_bricks.h"
#include "spritesheet_powerups.h"
#include "sprite_atlas.h"
#include "assets_paddle.h"
#include "spritesheet_paddle.h"
#include "config.h"   // pour BALL_SIZE, PADDLE_W, PADDLE_H
#include "lib/LCD.h"  // pour color_white, color_lightgreen

// Ball : carré BALL_SIZE x BALL_SIZE rempli de blanc
const uint16_t ball_pixels[BALL_SIZE * BALL_SIZE] = {
    color_black, color_darkgray, color_white, color_white, color_darkgray, color_black,
    color_darkgray, color_white, color_white, color_white, color_white, color_darkgray,
    color_white, color_white, color_white, color_white, color_white, color_white,
    color_white, color_white, color_white, color_white, color_white, color_white,
    color_darkgray, color_white, color_white, color_white, color_white, color_darkgray,
    color_black, color_darkgray, color_white, color_white, color_darkgray, color_black
};

#include "assets_paddle.h"
#include "spritesheet_paddle.h"
#include "config.h"

SpriteAtlas atlas_bricks;
SpriteAtlas atlas_powerups;

void init_assets(){
    atlas_bricks.load(bricks_pixels, bricks_sheet_w, bricks_sheet_h, BRICK_W, BRICK_H);    
    atlas_powerups.load(powerups_pixels, powerups_sheet_w, powerups_sheet_h, powerup_w, powerup_h);
	init_paddle_rows();   // charge les 5 lignes
    initPaddleFrames();   // construit les frames (NONE/GLUE/LASER/GLUE+LASER)
}
