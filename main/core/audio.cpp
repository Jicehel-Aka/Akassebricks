#include "audio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gb_ll_audio.h"

audio_player player;
SfxBus       g_sfx;

// Tous les effets historiques passent par le bus SFX 3 voix.
SfxBus& snd_paddle      = g_sfx;
SfxBus& snd_stick       = g_sfx;
SfxBus& snd_launch      = g_sfx;
SfxBus& snd_wall        = g_sfx;
SfxBus& snd_brick_hit   = g_sfx;
SfxBus& snd_brick_break = g_sfx;
SfxBus& snd_level_start = g_sfx;
SfxBus& snd_lost_life   = g_sfx;
SfxBus& snd_gameover    = g_sfx;
SfxBus& snd_keypress    = g_sfx;
SfxBus& snd_delete      = g_sfx;

// Le mixeur DOIT etre alimente tres souvent, sinon le FIFO I2S se vide et les
// sons deviennent quasi inaudibles. On lui dedie une tache (comme mAKArena) qui
// est le SEUL appelant de player.pool() -> pas d'acces concurrent au mixeur.
static void audio_task(void*) {
    while (true) {
        player.pool();
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

void audio_game_init() {
    gb_ll_audio_set_volume(200);   // volume MATERIEL du codec (manquait : sons faibles)
    for (int i = 0; i < SfxBus::VOICES; ++i) {
        player.add_track(&g_sfx.voices[i]);     // 4 voix SFX en round-robin
        g_sfx.voices[i].set_track_volume(1.0f);
    }
    xTaskCreatePinnedToCore(audio_task, "AudioTask", 4096, nullptr, 5, nullptr, 0);
}

void audio_set_volume(int volume) {
    if (volume < 0)   volume = 0;
    if (volume > 255) volume = 255;
    player.set_master_volume((uint8_t)volume);
}
