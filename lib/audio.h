#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include "common.h"

#define GB_AUDIO_BUFFER_FIFO_COUNT      4   // count of audio buffer fifo
#define GB_AUDIO_BUFFER_SAMPLE_COUNT    512 // 16b audio buffers (size in bytes = 2*GB_AUDIO_BUFFER_SAMPLE_COUNT )
#define GB_AUDIO_SAMPLE_RATE            44100

int audio_init();
void wav_pool_update();
void audio_push_buffer( const int16_t* pi16_audio_buffer );

    //! return total count of buffer 
uint32_t audio_fifo_buffer_count();
    //! return count of buffers used
uint32_t audio_fifo_buffer_used();
    //! return count of free buffers
uint32_t audio_fifo_buffer_free();


#ifdef __cplusplus
}
#endif