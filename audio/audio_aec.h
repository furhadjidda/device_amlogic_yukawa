/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Definitions and interface related to HAL implementations of Acoustic Echo Cancellation (AEC).
 *
 * AEC cleans the microphone signal by removing from it audio data corresponding to loudspeaker
 * playback. Note that this process can be nonlinear.
 */

#ifndef _AUDIO_AEC_H_
#define _AUDIO_AEC_H_

#include <stdint.h>
#include <pthread.h>
#include <sys/time.h>
#include <hardware/audio.h>
#include <audio_utils/resampler.h>
#include "audio_hw.h"
#include "fifo_wrapper.h"

struct ts_fifo_payload {
    struct timespec timestamp;
    unsigned int available;
    ssize_t bytes;
};

struct aec_itfe {
    pthread_mutex_t lock;
    int32_t *mic_buf;
    ssize_t mic_buf_size_bytes;
    ssize_t mic_frame_size_bytes;
    uint32_t mic_sampling_rate;
    struct ts_fifo_payload last_mic_ts;
    int32_t *spk_buf;
    ssize_t spk_buf_size_bytes;
    ssize_t spk_frame_size_bytes;
    uint32_t spk_sampling_rate;
    struct ts_fifo_payload last_spk_ts;
    int16_t *spk_buf_playback_format;
    int16_t *spk_buf_resampler_out;
    void *spk_fifo;
    void *ts_fifo;
    ssize_t read_write_diff_bytes;
    struct resampler_itfe *spk_resampler;
    bool spk_running;
    bool prev_spk_running;
};

/* Write speaker audio samples to FIFO for use in AEC.
 * Both audio samples and timestamps are added in FIFO fashion.
 * Must be called after every write to PCM. */
int write_to_spk_fifo (struct alsa_stream_out *out, void *buffer, size_t bytes);

/* Processing function call for AEC.
 * AEC output is updated at location pointed to by 'buffer'.
 * This function does not run AEC when there is no playback -
 * as communicated to this AEC interface using aec_set_spk_running().*/
int process_aec (struct alsa_stream_in *stream, void* buffer, size_t bytes);

/* Initialize speaker configuration for AEC.
 * Must be called when a new output stream is opened. */
int init_aec_spk_config (struct alsa_stream_out *out);

/* Initialize microphone configuration for AEC.
 * Must be called when a new input stream is opened. */
int init_aec_mic_config (struct alsa_stream_in *in);

/* Clear speaker configuration for AEC.
 * Must be called when the output stream is closed. */
void destroy_aec_spk_config (struct alsa_stream_out *out);

/* Clear microphone configuration for AEC.
 * Must be called when the input stream is closed. */
void destroy_aec_mic_config (struct alsa_stream_in *in);

/* Used to communicate playback state (running or not) to AEC interface.
 * This is used by process_aec() to determine if AEC processing is to be run. */
void aec_set_spk_running (struct alsa_stream_out *out, bool state);

#ifndef AEC_HAL

#define write_to_spk_fifo(...) ((int)0)
#define process_aec(...) ((int)0)
#define init_aec_spk_config(...) ((int)0)
#define init_aec_mic_config(...) ((int)0)
#define destroy_aec_spk_config(...) ((void)0)
#define destroy_aec_mic_config(...) ((void)0)
#define aec_set_spk_running(...) ((void)0)

#endif


#endif /* _AUDIO_AEC_H_ */
