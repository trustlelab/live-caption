/* audiocap.c
 * This file implements audio_thread using either the pipewire or pulse backend.
 * Currently only the pulse backend is used.
 *
 * Copyright 2022 abb128
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "audiocap.h"
#include "audiocap-internal.h"

#ifndef __APPLE__
#include <pulse/pulseaudio.h>
#endif
#include <april_api.h>

#ifdef __APPLE__
#define USE_COREAUDIO 1
#define USE_PULSEAUDIO 0
#else
#define USE_PULSEAUDIO 1
#define USE_COREAUDIO 0
#endif

struct audio_thread_i {
#ifdef LIVE_CAPTIONS_PIPEWIRE
    GThread * pw_thread_id;
#endif
#ifdef __APPLE__
    GThread * ca_thread_id;
#endif

    bool using_pulse;
    bool using_coreaudio;
    union {
        audio_thread_pa pulse;
#ifdef LIVE_CAPTIONS_PIPEWIRE
        audio_thread_pw pipewire;
#endif
#ifdef __APPLE__
        audio_thread_ca coreaudio;
#endif
    } thread;
};


static void *run_audio_thread(void *userdata) {
    audio_thread data = (audio_thread)userdata;

    return NULL;
}

audio_thread create_audio_thread(bool microphone, asr_thread asr){
    audio_thread data = calloc(1, sizeof(struct audio_thread_i));
    
    data->using_pulse = USE_PULSEAUDIO;
    data->using_coreaudio = USE_COREAUDIO;

    if(data->using_coreaudio) {
#ifdef __APPLE__
        data->thread.coreaudio = create_audio_thread_ca(microphone, asr);
        data->ca_thread_id = g_thread_new("lcap-audiothread-ca", run_audio_thread_ca, data->thread.coreaudio);
#endif
    }
    else if(data->using_pulse) {
#ifndef __APPLE__
        data->thread.pulse = create_audio_thread_pa(microphone, asr);
        run_audio_thread_pa(data->thread.pulse);
#endif
    }
#ifdef LIVE_CAPTIONS_PIPEWIRE
    else {
        data->thread.pipewire = create_audio_thread_pw(microphone, asr);
        data->pw_thread_id = g_thread_new("lcap-audiothread", run_audio_thread_pw, data->thread.pipewire);
    }
#endif

    return data;
}

void free_audio_thread(audio_thread thread) {
    if(thread->using_coreaudio) {
#ifdef __APPLE__
        free_audio_thread_ca(thread->thread.coreaudio);
        g_thread_join(thread->ca_thread_id);
        g_thread_unref(thread->ca_thread_id);
        free(thread->thread.coreaudio);
#endif
    }
    else if(thread->using_pulse){
#ifndef __APPLE__
        free_audio_thread_pa(thread->thread.pulse);
        free(thread->thread.pulse);
#endif
    }
#ifdef LIVE_CAPTIONS_PIPEWIRE
    else {
        free_audio_thread_pw(thread->thread.pipewire);
        g_thread_join(thread->pw_thread_id);
        g_thread_unref(thread->pw_thread_id); // ?
        free(thread->thread.pipewire);
    }
#endif

    free(thread);
}
