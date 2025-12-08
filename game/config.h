	#pragma once

	constexpr int SCREEN_W = 320;
	constexpr int SCREEN_H = 240;

	constexpr int PADDLE_W = 60;
	constexpr int PADDLE_H = 10;
	constexpr int PADDLE_WIDTHS[] = {30, 40, 60, 75, 90};
	constexpr int PADDLE_Y = SCREEN_H - 24;
	constexpr int PADDLE_SPEED = 4;

	// Dimension de la ball
	constexpr int BALL_SIZE = 6;
	constexpr float BALL_SPEED_INIT = 2.6f;
	constexpr float BALL_SPEED_MAX  = 6.0f;
	constexpr float BALL_SPEED_INC  = 0.08f;

	constexpr int BRICK_COLS = 9;
	constexpr int BRICK_ROWS = 6;

	// Dimensions des briques
	constexpr int BRICK_W = 28;
	constexpr int BRICK_H = 14;
	constexpr int BRICK_MARGIN_X = 6;
	constexpr int BRICK_MARGIN_Y = 6;
	constexpr int BRICK_TOP = 28;
	
	constexpr int BRICK_COLOR_COUNT = 5;

	// Dimensions des bonus
	constexpr int POWERUP_W = 12;   
	constexpr int POWERUP_H = 12; 
	constexpr int T_LASER = 5000;	
	

	// Mode debug 
	extern int debug;   // 0 = off, 1 = on
