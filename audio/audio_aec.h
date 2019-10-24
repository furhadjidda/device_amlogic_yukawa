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
 * Definitions and interface related to HAL implementations of Acoustic Echo Canceller (AEC).
 *
 * AEC cleans the microphone signal by removing from it audio data corresponding to loudspeaker
 * playback. Note that this process can be nonlinear.
 *
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

/* 'bytes' are the number of bytes written to audio FIFO, for which 'timestamp' is valid.
 * 'available' is the number of frames available to read (for input) or yet to be played
 * (for output) frames in the PCM buffer.
 * timestamp and available are updated by pcm_get_htimestamp(), so they use the same
 * datatypes as the corresponding arguments to that function. */
struct ts_fifo_payload {
    struct timespec timestamp;
    unsigned int available;
    size_t bytes;
};

struct aec_thread_args {
    struct aec_t *aec;
    size_t bytes;
    int ret;
};

struct aec_io {
    size_t num_channels;
    int32_t *buf;
    size_t buf_size_bytes;
    size_t frame_size_bytes;
    uint32_t sampling_rate;
    struct ts_fifo_payload last_timestamp;
    void *audio_fifo;
    void *ts_fifo;
    struct pcm *pcm;
    ssize_t fifo_read_write_diff_bytes;
    bool running;
    bool prev_running;
};

struct aec_t {
    pthread_mutex_t lock;
    pthread_cond_t ready_to_run;
    bool running;
    pthread_t run_thread_id;
    struct aec_thread_args args;
    size_t num_reference_channels;
    struct aec_io mic;
    struct aec_io reference;
    struct aec_io out;
    int16_t *reference_playback_format;
    int16_t *reference_resampler_out;
    struct resampler_itfe *reference_resampler;
};

#ifdef AEC_HAL

/* Write audio samples to AEC reference FIFO for use in AEC.
 * Both audio samples and timestamps are added in FIFO fashion.
 * Must be called after every write to PCM. */
int write_to_reference_fifo (struct aec_t *aec, void *buffer, size_t bytes);

/* Processing function call for AEC.
 * AEC output is updated at location pointed to by 'buffer'.
 * This function does not run AEC when there is no playback -
 * as communicated to this AEC interface using aec_set_spk_running().*/
int process_aec (struct aec_t *aec, void* buffer, size_t bytes);

/* Initialize AEC object.
 * This must be called when the audio device is opened.
 * ALSA device mutex must be held before calling this API.*/
int init_aec (int sampling_rate, int num_reference_channels,
                int num_microphone_channels, struct aec_t **);

/* Release AEC object.
 * This must be called when the audio device is closed. */
void release_aec(struct aec_t *aec);

/* Initialize reference configuration for AEC.
 * Must be called when a new output stream is opened. */
int init_aec_reference_config (struct aec_t *aec, struct alsa_stream_out *out);

/* Initialize microphone configuration for AEC.
 * Must be called when a new input stream is opened. */
int init_aec_mic_config (struct aec_t *aec, struct alsa_stream_in *in);

/* Clear reference configuration for AEC.
 * Must be called when the output stream is closed. */
void destroy_aec_reference_config (struct aec_t *aec);

/* Clear microphone configuration for AEC.
 * Must be called when the input stream is closed. */
void destroy_aec_mic_config (struct aec_t *aec);

/* Used to communicate playback state (running or not) to AEC interface.
 * This is used by process_aec() to determine if AEC processing is to be run. */
void aec_set_spk_running (struct aec_t *aec, bool state);

#else /* #ifdef AEC_HAL */

#define write_to_reference_fifo(...) ((int)0)
#define process_aec(...) ((int)0)
#define init_aec(...) ((int)0)
#define release_aec(...) ((void)0)
#define init_aec_reference_config(...) ((int)0)
#define init_aec_mic_config(...) ((int)0)
#define destroy_aec_reference_config(...) ((void)0)
#define destroy_aec_mic_config(...) ((void)0)
#define aec_set_spk_running(...) ((void)0)

#endif /* #ifdef AEC_HAL */

#endif /* _AUDIO_AEC_H_ */
