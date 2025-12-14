#include "audio.h"
#include "lib/audio_basic.h"

audio_player player; 

// Définition des pistes
audio_track_tone snd_paddle;
audio_track_tone snd_stick;
audio_track_tone snd_launch;
audio_track_tone snd_wall;
audio_track_tone snd_brick_hit;
audio_track_tone snd_brick_break;
audio_track_tone snd_level_start;
audio_track_tone snd_lost_life;
audio_track_tone snd_gameover;
audio_track_tone snd_keypress;
audio_track_tone snd_delete;

void audio_game_init() {
    player.add_track(&snd_paddle);
    player.add_track(&snd_stick);
    player.add_track(&snd_launch);
    player.add_track(&snd_wall);
    player.add_track(&snd_brick_hit);
    player.add_track(&snd_brick_break);
    player.add_track(&snd_level_start);
    player.add_track(&snd_lost_life);
    player.add_track(&snd_gameover);
    player.add_track(&snd_keypress);
    player.add_track(&snd_delete);
}

void audio_play_intro_async(){
    // audio_basic_play_melody_async("intro_theme");
}

void audio_play_effect(int id){
    // audio_basic_play_effect(id);
}
