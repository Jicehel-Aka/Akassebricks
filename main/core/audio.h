/*
  core/audio.h — Adaptateur audio AKasseBricks sur le composant gamebuino.

  gb_audio_player n'a que 4 voix (AUDIO_PLAYER_TRACK_COUNT). On en reserve 3 pour
  les effets, gerees en ROUND-ROBIN par SfxBus : chaque appel play_tone part sur
  la voix suivante, donc un son n'en coupe plus un autre (ex. rebond + casse de
  brique). Il reste 1 voix libre. Tous les snd_* pointent sur ce bus unique.
*/
#pragma once
#include "gb_audio_player.h"
#include "gb_audio_track_tone.h"
#include <cstdint>

struct SfxBus {
    static const int VOICES = 4;   // pas de musique ici : les 4 voix pour les SFX
    gb_audio_track_tone voices[VOICES];
    int next = 0;
    // Compat : le jeu appelle play_tone(freq, duree[, gain]). Le gain (0..1)
    // permet d'equilibrer les effets entre eux (ex. la brique, plus grave donc
    // percue plus faible, reste a 1.0 tandis que les petits bruits sont baisses).
    void play_tone(float f32_frequency, uint16_t u16_duration_ms, float gain = 1.0f) {
        if (gain < 0.0f) gain = 0.0f;
        if (gain > 1.0f) gain = 1.0f;
        voices[next].play_tone(f32_frequency, gain, u16_duration_ms);
        next = (next + 1) % VOICES;
    }
};

using audio_player = gb_audio_player;

extern audio_player player;
extern SfxBus       g_sfx;

extern SfxBus& snd_paddle;
extern SfxBus& snd_stick;
extern SfxBus& snd_launch;
extern SfxBus& snd_wall;
extern SfxBus& snd_brick_hit;
extern SfxBus& snd_brick_break;
extern SfxBus& snd_level_start;
extern SfxBus& snd_lost_life;
extern SfxBus& snd_gameover;
extern SfxBus& snd_keypress;
extern SfxBus& snd_delete;

void audio_game_init();
void audio_set_volume(int volume);
