/*
  core/audio.h — Adaptateur audio AKasseBricks au-dessus du composant gamebuino.
  On garde l'API historique du jeu (player, snd_*.play_tone(freq, ms)) mais le
  moteur est desormais gb_audio (composant standard). audio_track_tone est un
  simple sous-type de gb_audio_track_tone qui restaure la surcharge a 2 arguments
  (le SDK exige maintenant un volume).
*/
#pragma once
#include "gb_audio_player.h"
#include "gb_audio_track_tone.h"

// Compat : gb_audio_track_tone::play_tone attend (freq, volume, duree[, type]).
// Le jeu appelle play_tone(freq, duree). On restaure cette forme a volume plein.
struct audio_track_tone : public gb_audio_track_tone {
    using gb_audio_track_tone::play_tone;          // conserve les surcharges du SDK
    void play_tone(float f32_frequency, uint16_t u16_duration_ms) {
        gb_audio_track_tone::play_tone(f32_frequency, 1.0f, u16_duration_ms);
    }
};

using audio_player = gb_audio_player;

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

void audio_game_init();               // enregistre les pistes aupres du player
void audio_set_volume(int volume);    // 0..255 -> master volume
