/* audiocap-ca.c
 * This file contains the Core Audio implementation of audio_thread for macOS
 *
 * Copyright 2025
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

#ifdef __APPLE__

#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <glib.h>

#include <april_api.h>

#include "audiocap-internal.h"
#include "audiocap.h"

struct audio_thread_ca_i {
    asr_thread asr;
    bool microphone;
    size_t sample_rate;
    
    AudioQueueRef queue;
    AudioQueueBufferRef buffers[3];
    
    pthread_mutex_t mutex;
    bool running;
    bool initialized;
};

#define NUM_BUFFERS 3
#define BUFFER_SIZE 4096

// Helper function to find BlackHole audio device UID
static CFStringRef find_blackhole_device_uid(void) {
    CFStringRef deviceUID = NULL;
    UInt32 propertySize;
    
    // Get number of audio devices
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };
    
    OSStatus status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject,
                                                      &propertyAddress,
                                                      0,
                                                      NULL,
                                                      &propertySize);
    if (status != noErr) return NULL;
    
    int deviceCount = propertySize / sizeof(AudioDeviceID);
    AudioDeviceID *devices = (AudioDeviceID *)malloc(propertySize);
    
    status = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                       &propertyAddress,
                                       0,
                                       NULL,
                                       &propertySize,
                                       devices);
    
    if (status == noErr) {
        // Search for BlackHole device by name
        for (int i = 0; i < deviceCount; i++) {
            CFStringRef deviceName = NULL;
            propertySize = sizeof(deviceName);
            
            AudioObjectPropertyAddress nameAddress = {
                kAudioDevicePropertyDeviceNameCFString,
                kAudioObjectPropertyScopeGlobal,
                kAudioObjectPropertyElementMain
            };
            
            status = AudioObjectGetPropertyData(devices[i],
                                               &nameAddress,
                                               0,
                                               NULL,
                                               &propertySize,
                                               &deviceName);
            
            if (status == noErr && deviceName != NULL) {
                char name[256];
                CFStringGetCString(deviceName, name, sizeof(name), kCFStringEncodingUTF8);
                
                // Check if this is BlackHole (case-insensitive)
                if (strstr(name, "BlackHole") != NULL || strstr(name, "blackhole") != NULL) {
                    printf("Found BlackHole device: %s\n", name);
                    
                    // Get the device UID
                    AudioObjectPropertyAddress uidAddress = {
                        kAudioDevicePropertyDeviceUID,
                        kAudioObjectPropertyScopeGlobal,
                        kAudioObjectPropertyElementMain
                    };
                    
                    CFStringRef uid = NULL;
                    propertySize = sizeof(uid);
                    status = AudioObjectGetPropertyData(devices[i],
                                                       &uidAddress,
                                                       0,
                                                       NULL,
                                                       &propertySize,
                                                       &uid);
                    
                    if (status == noErr && uid != NULL) {
                        deviceUID = CFStringCreateCopy(NULL, uid);
                        CFRelease(uid);
                    }
                    
                    CFRelease(deviceName);
                    break;
                }
                
                CFRelease(deviceName);
            }
        }
    }
    
    free(devices);
    return deviceUID;
}

static void audioInputCallback(void *inUserData,
                               AudioQueueRef inAQ,
                               AudioQueueBufferRef inBuffer,
                               const AudioTimeStamp *inStartTime,
                               UInt32 inNumberPacketDescriptions,
                               const AudioStreamPacketDescription *inPacketDescs) {
    audio_thread_ca data = (audio_thread_ca)inUserData;
    
    if (!data->running) return;
    
    // Convert audio data to 16-bit PCM and send to ASR
    int16_t *audio_data = (int16_t *)inBuffer->mAudioData;
    size_t num_samples = inBuffer->mAudioDataByteSize / sizeof(int16_t);
    
    if (data->asr != NULL) {
        asr_thread_enqueue_audio(data->asr, audio_data, num_samples);
    }
    
    // Re-enqueue the buffer
    if (data->running) {
        AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, NULL);
    }
}

void *run_audio_thread_ca(void *userdata) {
    audio_thread_ca data = (audio_thread_ca)userdata;
    
    OSStatus status;
    
    // Configure audio format (16-bit PCM, mono)
    AudioStreamBasicDescription format;
    memset(&format, 0, sizeof(format));
    format.mSampleRate = data->sample_rate;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    format.mBitsPerChannel = 16;
    format.mChannelsPerFrame = 1;
    format.mBytesPerFrame = 2;
    format.mFramesPerPacket = 1;
    format.mBytesPerPacket = 2;
    
    // Create audio queue
    status = AudioQueueNewInput(&format,
                               audioInputCallback,
                               data,
                               NULL,
                               kCFRunLoopCommonModes,
                               0,
                               &data->queue);
    
    if (status != noErr) {
        fprintf(stderr, "Failed to create audio queue: %d\n", status);
        return NULL;
    }
    
    // Set up input device based on microphone toggle
    if (!data->microphone) {
        // Desktop audio mode: Try to use BlackHole
        CFStringRef blackholeUID = find_blackhole_device_uid();
        
        if (blackholeUID != NULL) {
            // Set the audio queue to use BlackHole by UID
            status = AudioQueueSetProperty(data->queue,
                                          kAudioQueueProperty_CurrentDevice,
                                          &blackholeUID,
                                          sizeof(blackholeUID));
            
            if (status == noErr) {
                printf("✓ Desktop audio mode: Using BlackHole for system audio capture\n");
                printf("  Make sure your output is set to Multi-Output Device (Headphones + BlackHole)\n");
            } else {
                fprintf(stderr, "⚠ Could not set BlackHole device (error: %d)\n", status);
                fprintf(stderr, "  Using default input device instead\n");
            }
            
            CFRelease(blackholeUID);
        } else {
            fprintf(stderr, "⚠ Desktop audio mode: BlackHole not found!\n");
            fprintf(stderr, "  Please install BlackHole from: https://github.com/ExistentialAudio/BlackHole\n");
            fprintf(stderr, "  Using default input device (likely your microphone)\n");
        }
    } else {
        // Microphone mode: Use default input (your headset mic)
        printf("✓ Microphone mode: Using default input device (your headset microphone)\n");
    }
    
    // Allocate and enqueue buffers
    for (int i = 0; i < NUM_BUFFERS; i++) {
        status = AudioQueueAllocateBuffer(data->queue, BUFFER_SIZE * sizeof(int16_t), &data->buffers[i]);
        if (status != noErr) {
            fprintf(stderr, "Failed to allocate audio buffer: %d\n", status);
            AudioQueueDispose(data->queue, true);
            return NULL;
        }
        AudioQueueEnqueueBuffer(data->queue, data->buffers[i], 0, NULL);
    }
    
    // Start recording
    data->running = true;
    data->initialized = true;
    status = AudioQueueStart(data->queue, NULL);
    if (status != noErr) {
        fprintf(stderr, "Failed to start audio queue: %d\n", status);
        data->running = false;
        AudioQueueDispose(data->queue, true);
        return NULL;
    }
    
    printf("Core Audio capture started (sample rate: %zu Hz)\n", data->sample_rate);
    
    // Keep thread alive while running
    while (data->running) {
        usleep(100000); // 100ms
    }
    
    return NULL;
}

audio_thread_ca create_audio_thread_ca(bool microphone, asr_thread asr) {
    audio_thread_ca data = calloc(1, sizeof(struct audio_thread_ca_i));
    
    data->microphone = microphone;
    data->asr = asr;
    data->sample_rate = asr_thread_samplerate(asr);
    data->running = false;
    data->initialized = false;
    
    pthread_mutex_init(&data->mutex, NULL);
    
    return data;
}

void free_audio_thread_ca(audio_thread_ca thread) {
    if (!thread) return;
    
    if (thread->initialized) {
        thread->running = false;
        
        // Stop and dispose of the audio queue
        AudioQueueStop(thread->queue, true);
        
        // Free buffers
        for (int i = 0; i < NUM_BUFFERS; i++) {
            if (thread->buffers[i]) {
                AudioQueueFreeBuffer(thread->queue, thread->buffers[i]);
            }
        }
        
        AudioQueueDispose(thread->queue, true);
    }
    
    pthread_mutex_destroy(&thread->mutex);
}

#endif // __APPLE__
