#pragma once
#include "common.h"

//! base object for audio
class audio_track_base
{
public:
    audio_track_base() {};
    ~audio_track_base() {};
    virtual int play_callback(int16_t *pi16_buffer, uint16_t u16_sample_count) = 0;
    //! generate fade out effect on buffer ( volume down ramp )
    void fade_out_buffer(int16_t *pi16_buffer);
    //! generate fade in effect on buffer ( volume up ramp )
    void fade_in_buffer(int16_t *pi16_buffer);

protected:
    int32_t i32_cb_loop_count{0};
};

//! base object for audio
class audio_track_tone : public audio_track_base
{
public:
    //! start playing sound for duration and frequency
    void play_tone(float f32_frequency, uint16_t u16_duration_ms);
    //! buffer fill callback
    int play_callback(int16_t *pi16_buffer, uint16_t u16_sample_count);

private:
    uint32_t u32_sin_inc{0};
    uint32_t u32_sin_sample_remain{0};
    uint32_t u32_sin_read_index{0};
};

#define AUDIO_PLAYER_TRACK_COUNT 4
class audio_player
{
public:
    audio_player();
    ~audio_player() {};
    //! add a track type to player
    int add_track(audio_track_base *track);
    //! remove a track from player
    int del_track(audio_track_base *track);
    //! update track objects ( generate samples )
    void pool();

private:
    audio_track_base *tracks[AUDIO_PLAYER_TRACK_COUNT];
};
