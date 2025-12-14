#pragma once
#include "lib/audio.h"
#include "lib/audio_basic.h"

extern audio_player player;   // instance globale

extern audio_track_tone snd_paddle;
extern audio_track_tone snd_stick;
extern audio_track_tone snd_launch;
extern audio_track_tone snd_wall;
extern audio_track_tone snd_brick_hit;
extern audio_track_tone snd_brick_break;
extern audio_track_tone snd_level_start;
extern audio_track_tone snd_lost_life;
extern audio_track_tone snd_gameover;
extern audio_track_tone snd_keypress;
extern audio_track_tone snd_delete;

void audio_play_intro_async();
void audio_play_effect(int id);

// Fonction d’initialisation
void audio_game_init();
