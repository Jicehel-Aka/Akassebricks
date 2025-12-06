#include "audio_basic.h"
#include "audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

audio_player::audio_player()
{
    for (int i = 0; i < count_of(tracks); i++)
        tracks[i] = 0;
}

//! generate fade out effect on buffer ( volume down ramp )
void audio_track_base::fade_out_buffer(int16_t *pi16_buffer)
{
    for (int16_t i = 0; i < GB_AUDIO_BUFFER_SAMPLE_COUNT; i++)
        pi16_buffer[i] = pi16_buffer[i] * (GB_AUDIO_BUFFER_SAMPLE_COUNT - 1 - i) / GB_AUDIO_BUFFER_SAMPLE_COUNT;
}
//! generate fade in effect on buffer ( volume up ramp )
void audio_track_base::fade_in_buffer(int16_t *pi16_buffer)
{
    for (int16_t i = 0; i < GB_AUDIO_BUFFER_SAMPLE_COUNT; i++)
        pi16_buffer[i] = pi16_buffer[i] * i / GB_AUDIO_BUFFER_SAMPLE_COUNT;
}

int audio_player::add_track(audio_track_base *track)
{
    if (!track)
        return -1; // fail
    for (int i = 0; i < count_of(tracks); i++)
    {
        if (tracks[i] == 0) // not yet populated
        {
            tracks[i] = track;
            return 0; // success
        }
    }
    return -1; // fail, no more space
}

int audio_player::del_track(audio_track_base *track)
{
    if (!track)
        return -1; // fail
    for (int i = 0; i < count_of(tracks); i++)
    {
        if (tracks[i] == track) // populated with this track
        {
            tracks[i] = 0; // delete
            return 0;      // success
        }
    }
    return -1; // fail, no more space
}

void audio_player::pool()
{
    /*static int iPool = 0;
        if ( 0 == (iPool % 100) )
        {
            // caution : log on usbSerial cause small glitch on sound..
            printf("%ld count\n", audio_fifo_buffer_count() );
            printf("%ld used\n", audio_fifo_buffer_used() );
            printf("%ld free\n", audio_fifo_buffer_free() );
        }
        iPool++;
        */

    while (audio_fifo_buffer_free()) // at least 1 buffer
    {
        int8_t i8_buffer_ready = 0;
        static int16_t i16_audio_buffer_out[GB_AUDIO_BUFFER_SAMPLE_COUNT];
        static int16_t i16_audio_buffer_track[GB_AUDIO_BUFFER_SAMPLE_COUNT];
        for (int smp = 0; smp < GB_AUDIO_BUFFER_SAMPLE_COUNT; smp++)
            i16_audio_buffer_out[smp] = 0;
        for (int i = 0; i < count_of(tracks); i++)
        {
            if (tracks[i]) // populated
            {
                if (0 == tracks[i]->play_callback(i16_audio_buffer_track, GB_AUDIO_BUFFER_SAMPLE_COUNT)) // track generate successfully
                {
                    i8_buffer_ready++;
                    for (int smp = 0; smp < GB_AUDIO_BUFFER_SAMPLE_COUNT; smp++)
                        i16_audio_buffer_out[smp] += i16_audio_buffer_track[smp] / 4;
                }
            }
        }
        if (i8_buffer_ready)
            audio_push_buffer(i16_audio_buffer_out);
        if (!i8_buffer_ready)
            return;
    }
}

// sin wav table
const int16_t i16_sin_tab[256] = {0, 804, 1608, 2410, 3212, 4011, 4808, 5602, 6393, 7179, 7962, 8739, 9512, 10278, 11039, 11793, 12539, 13279, 14010, 14732, 15446, 16151, 16846, 17530, 18204, 18868, 19519, 20159, 20787, 21403, 22005, 22594, 23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790, 27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956, 30273, 30571, 30852, 31113, 31356, 31580, 31785, 31971, 32137, 32285, 32412, 32521, 32609, 32678, 32728, 32757, 32767, 32757, 32728, 32678, 32609, 32521, 32412, 32285, 32137, 31971, 31785, 31580, 31356, 31113, 30852, 30571, 30273, 29956, 29621, 29268, 28898, 28510, 28105, 27683, 27245, 26790, 26319, 25832, 25329, 24811, 24279, 23731, 23170, 22594, 22005, 21403, 20787, 20159, 19519, 18868, 18204, 17530, 16846, 16151, 15446, 14732, 14010, 13279, 12539, 11793, 11039, 10278, 9512, 8739, 7962, 7179, 6393, 5602, 4808, 4011, 3212, 2410, 1608, 804, 0, -804, -1608, -2410, -3212, -4011, -4808, -5602, -6393, -7179, -7962, -8739, -9512, -10278, -11039, -11793, -12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530, -18204, -18868, -19519, -20159, -20787, -21403, -22005, -22594, -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790, -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956, -30273, -30571, -30852, -31113, -31356, -31580, -31785, -31971, -32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757, -32767, -32757, -32728, -32678, -32609, -32521, -32412, -32285, -32137, -31971, -31785, -31580, -31356, -31113, -30852, -30571, -30273, -29956, -29621, -29268, -28898, -28510, -28105, -27683, -27245, -26790, -26319, -25832, -25329, -24811, -24279, -23731, -23170, -22594, -22005, -21403, -20787, -20159, -19519, -18868, -18204, -17530, -16846, -16151, -15446, -14732, -14010, -13279, -12539, -11793, -11039, -10278, -9512, -8739, -7962, -7179, -6393, -5602, -4808, -4011, -3212, -2410, -1608, -1608};

//! start playing sound for duration and frequency
void audio_track_tone::play_tone(float f32_frequency, uint16_t u16_duration_ms)
{
    // convert 1 sinus period for 256 samples to user freq
    u32_sin_inc = (uint32_t)(65536.0f * f32_frequency * 256.0f / GB_AUDIO_SAMPLE_RATE);
    if (!u32_sin_sample_remain)
        i32_cb_loop_count = 0;
    // convert duration ms to sample count
    u32_sin_sample_remain = (uint32_t)u16_duration_ms * GB_AUDIO_SAMPLE_RATE / 1000;
}

//! buffer fill callback
int audio_track_tone::play_callback(int16_t *pi16_buffer, uint16_t u16_sample_count)
{
    if (u32_sin_sample_remain)
    {
        for (int i = 0; i < u16_sample_count; i++)
            pi16_buffer[i] = i16_sin_tab[((u32_sin_read_index += u32_sin_inc) / 65536) & 0xFF];
        if (i32_cb_loop_count == 0)
            fade_in_buffer(pi16_buffer);
        if (u32_sin_sample_remain > u16_sample_count)
            u32_sin_sample_remain -= u16_sample_count;
        else
        {
            u32_sin_sample_remain = 0;
            u32_sin_read_index = 0;
            fade_out_buffer(pi16_buffer);
        }
        i32_cb_loop_count++;
        return 0; // success
    }
    return -1; // not playing
}
