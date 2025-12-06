#include "highscores.h"
#include "core/graphics.h"
#include "core/input.h"
#include "lib/LCD.h"
#include <algorithm>

static std::vector<HighScore> scores;

	void highscores_show(){
		persist_load(scores);
		gfx_clear(COLOR_BLACK);
		gfx_text(100,40,"Meilleurs Scores",COLOR_WHITE);
		int y=60;
		for(size_t i=0;i<scores.size() && i<10;i++){
			char buf[64];
			std::snprintf(buf,sizeof(buf),"%s : %d",scores[i].name.c_str(),scores[i].score);
			gfx_text(80,y,buf,COLOR_WHITE);
			y+=16;
		}

		gfx_text(10, 180, "Appuyez sur B pour retourner au menu", COLOR_RED);
		gfx_flush();
		Keys k;
		for(;;){
			input_poll(k);	
			if(k.B) break;
		}
	}

	void highscores_submit(int score){
		persist_load(scores);
		HighScore hs={"AAA",score};
		scores.push_back(hs);
		std::sort(scores.begin(),scores.end(),[](auto&a,auto&b){return a.score>b.score;});
		if(scores.size()>10) scores.resize(10);
		persist_save(scores);
	}
