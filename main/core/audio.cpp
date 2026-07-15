#include "audio.h"

audio_player player;

audio_track_tone snd_paddle, snd_stick, snd_launch, snd_wall;
audio_track_tone snd_brick_hit, snd_brick_break, snd_level_start;
audio_track_tone snd_lost_life, snd_gameover, snd_keypress, snd_delete;

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

void audio_set_volume(int volume) {
    if (volume < 0)   volume = 0;
    if (volume > 255) volume = 255;
    player.set_master_volume((uint8_t)volume);
}
