#include "spritesheet_paddle.h"

Frame paddle_frames[5][4]; // définition globale

void initPaddleFrames() {
    for (int s = VERYSMALL; s <= VERYBIG; ++s) {
        const int w = PADDLE_WIDTHS[s];
        const int h = paddle_rows[s].spriteH;
        const int y = 0;

        paddle_frames[s][0] = { 0 * w, y, w, h }; // NONE
        paddle_frames[s][1] = { 1 * w, y, w, h }; // GLUE
        paddle_frames[s][2] = { 2 * w, y, w, h }; // LASER
        paddle_frames[s][3] = { 3 * w, y, w, h }; // GLUE+LASER
    }
}
