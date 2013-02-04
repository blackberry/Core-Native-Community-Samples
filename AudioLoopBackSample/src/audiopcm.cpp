/* Copyright (c) 2012 Research In Motion Limited.
 * Copyright (c) 2012 Truphone Limited.
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/keycodes.h>
#include <screen/screen.h>
#include <assert.h>
#include <bps/accelerometer.h>
#include <bps/navigator.h>
#include <bps/screen.h>
#include <bps/bps.h>
#include <bps/event.h>
#include <bps/orientation.h>
#include <math.h>
#include <time.h>
#include <screen/screen.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <sys/asoundlib.h>
#include <audio/audio_manager_routing.h>
#include <pthread.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <gulliver.h>

typedef struct circular_buffer {
	int length;
	int head;
	int tail;
	int size;
	char* buffer;
	pthread_mutex_t mutex;
} circular_buffer_t;

// Constants for clearer init code
// Jitter buffer size.
static const int JITTER_BUFFER_NUMBER_FRAMES = 20;
// Standard VoIP
static const int PREFERRED_FRAME_SIZE = 640;
static const int VOIP_SAMPLE_RATE = 16000;
// ulaw silence is FF
static const char SILENCE = 0xFF;
// Globals (g_xx)
static pthread_t g_capturethread;
static pthread_t g_playerthread;
static snd_pcm_t *g_pcm_handle_c;
static snd_pcm_t *g_pcm_handle_p;
static unsigned int g_audio_manager_handle_c;
static unsigned int g_audio_manager_handle_p;
static unsigned int g_audio_manager_handle_t;
static int g_frame_size_c;
static int g_frame_size_p;
// Flag to stop the record and capture threads
static bool g_execute_audio = true;
// Global used by both threads
static circular_buffer_t* g_circular_buffer;
static bool capture_ready = false;

// Forward declarations
static int capture(circular_buffer_t* circular_buffer);
static int play(circular_buffer_t* circular_buffer);
static void capturesetup();
static void playsetup();
void toggleSpeaker(bool enable);

/**
 * Encapsulation of a circular buffer
 * read from head write to tail
 * size is constant available space
 * length is number of available unread bytes
 * mutex protects a complete frame read and write
 * buffer dynamically allocated
 */

static circular_buffer_t* createCircularBuffer() {
	circular_buffer_t* circular_buffer = (circular_buffer_t*) malloc(
			sizeof(circular_buffer_t));
	memset(circular_buffer, 0, sizeof(circular_buffer_t));
	circular_buffer->buffer = (char*) malloc(
			JITTER_BUFFER_NUMBER_FRAMES * PREFERRED_FRAME_SIZE);
	circular_buffer->size = JITTER_BUFFER_NUMBER_FRAMES * PREFERRED_FRAME_SIZE;
	pthread_mutex_init(&circular_buffer->mutex, NULL);
	return circular_buffer;
}

static void destroyCircularBuffer(circular_buffer_t* circular_buffer) {
	pthread_mutex_destroy(&circular_buffer->mutex);
	free(circular_buffer->buffer);
	free(circular_buffer);
}

/**
 * Write to the tail index, increment it and do the modulo wrap-around
 */
static void writeByteToCircularBuffer(circular_buffer_t* circular_buffer,
		char val) {
	++circular_buffer->length;
	circular_buffer->buffer[circular_buffer->tail] = val;
	++circular_buffer->tail;
	// Modulo arithmetic
	circular_buffer->tail %= circular_buffer->size;
}

/**
 * Read from the head index, increment it and do the modulo wrap-around
 */
static char readByteFromCircularBuffer(circular_buffer_t* circular_buffer) {
	--circular_buffer->length;
	char val = circular_buffer->buffer[circular_buffer->head];
	++circular_buffer->head;
	// Modulo arithmetic
	circular_buffer->head %= circular_buffer->size;
	return val;
}

/**
 * Returns True/False - Either there is enough room or the call fails (false)<br>
 * Mutex protect the buffer<br>
 */
static bool writeToCircularBuffer(circular_buffer_t* circular_buffer,
		char* buffer, int number_bytes) {
	// Check there's room
	// Not inside the mutex as it's read only
	// Mutex protect a number_bytes write to the circular buffer

	if (circular_buffer->length + number_bytes > circular_buffer->size) {
		return false;
	}
	pthread_mutex_lock(&circular_buffer->mutex);
	for (int i = 0; i < number_bytes; ++i) {
		writeByteToCircularBuffer(circular_buffer, buffer[i]);
	}
	pthread_mutex_unlock(&circular_buffer->mutex);
	return true;
}

/**
 * Returns True/False - Either there is data in the buffer to satisfy the request or it fails (false)<br>
 * Mutex protect the buffer<br>
 */
static bool readFromCircularBuffer(circular_buffer_t* circular_buffer,
		char buffer[], int number_bytes) {
	// Check there's sufficient data
	// Not inside the mutex as it's read only
	// Mutex protect a number_bytes read from the circular buffer

	if (circular_buffer->length < number_bytes) {
		return false;
	}
	int i;
	pthread_mutex_lock(&circular_buffer->mutex);
	for (i = 0; i < number_bytes; ++i) {
		buffer[i] = readByteFromCircularBuffer(circular_buffer);
	}
	pthread_mutex_unlock(&circular_buffer->mutex);
	return true;
}

/**
 * Thread func just calls the player
 * arg is the circular buffer
 */
static void* playerThread(void* arg) {
	fprintf(stderr,"Player thread is %d\n", pthread_self() );
	play((circular_buffer_t*)arg);
	return( 0 );
}

	/**
	 * Thread function just calls the capture<br>
	 * arg is the circular buffer<br>
	 */
static void* captureThread(void* arg) {
	fprintf(stderr, "Recorder thread is %d\n", pthread_self() );
	capture((circular_buffer_t*)arg);
	return( 0 );
}
	/**
	 * Create the threads passing the circular buffer as the arg<br>
	 * The startup not synchronized but not important as the jitter buffer is ample size<br>
	 */

void startPcmAudio() {
	fprintf(stderr,"StartPCMAudio ****************: Enter \n");
	g_execute_audio = true;
	g_circular_buffer = createCircularBuffer();
	/* get a new audioman handle.
	 This handle will be used to toggle device between speaker and headset
	 */
	audio_manager_get_handle(AUDIO_TYPE_VIDEO_CHAT, 0, false, &g_audio_manager_handle_t);

    capturesetup();
    playsetup();
    toggleSpeaker(false); // set playback device = headset
    pthread_create(&g_capturethread,NULL, &captureThread, g_circular_buffer);
    pthread_create(&g_playerthread,NULL,&playerThread,g_circular_buffer);
    fprintf(stderr,"StartPCMAudio ****************: EXIT \n");
}

	/**
	 * Set the thread exit flag and cleanup<br>
	 */
void stopPcmAudio() {
	// Threads will see this flag every 20ms in their loop
	fprintf(stderr,"\nStopPCMAudio ****************: ENTER \n");
	g_execute_audio = false;
	fprintf(stderr,"CAPTURE JOIN = %d\n",pthread_join(g_capturethread, NULL));
	fprintf(stderr,"PLAY JOIN = %d\n",pthread_join(g_playerthread, NULL));
	audio_manager_free_handle(g_audio_manager_handle_t);
	destroyCircularBuffer(g_circular_buffer);
	fprintf(stderr,"StopPCMAudio ****************: EXIT\n");
}

void toggleSpeaker(bool enable) {
	fprintf(stderr,"toggleSpeaker ****************: ENTER = %d\n", enable);
	int ret;
	if (enable == true) {
		ret = audio_manager_set_handle_type(g_audio_manager_handle_t, AUDIO_TYPE_VIDEO_CHAT, AUDIO_DEVICE_SPEAKER, AUDIO_DEVICE_DEFAULT);
	} else {
		ret = audio_manager_set_handle_type(g_audio_manager_handle_t, AUDIO_TYPE_VIDEO_CHAT, AUDIO_DEVICE_HANDSET, AUDIO_DEVICE_DEFAULT);
	}
	if(ret == 0) {
		ret = audio_manager_set_handle_routing_conditions(g_audio_manager_handle_t, SETTINGS_RESET_ON_DEVICE_CONNECTION);
		if(ret != 0) {
			fprintf(stderr,"toggleSpeaker() ERROR audio_manager_set_handle_routing_conditions = %d\n",ret);
		}
	} else {
		fprintf(stderr,"toggleSpeaker() ERROR audio_manager_set_handle_type = %d\n",ret);
	}
	fprintf(stderr,"toggleSpeaker ****************: EXIT\n");
}

static void capturesetup() {

	snd_pcm_channel_setup_t setup;
	int ret;
	snd_pcm_channel_info_t pi;
	snd_mixer_group_t group;
	snd_pcm_channel_params_t pp;
	int card = setup.mixer_card;

	audio_manager_snd_pcm_open_name(AUDIO_TYPE_VIDEO_CHAT, &g_pcm_handle_c,
			&g_audio_manager_handle_c, (char*) "voice", SND_PCM_OPEN_CAPTURE);

	if ((ret = snd_pcm_plugin_set_disable(g_pcm_handle_c, PLUGIN_DISABLE_MMAP))
			< 0) {
		fprintf(stderr, "snd_pcm_plugin_set_disable failed: %s\n", snd_strerror (ret));
		return;
	}

	if ((ret = snd_pcm_plugin_set_enable(g_pcm_handle_c, PLUGIN_ROUTING)) < 0) {
		fprintf(stderr, "snd_pcm_plugin_set_enable: %s\n", snd_strerror (ret));
		return;
	}

	// sample reads the capabilities of the capture
	memset(&pi, 0, sizeof(pi));
	pi.channel = SND_PCM_CHANNEL_CAPTURE;
	if ((ret = snd_pcm_plugin_info(g_pcm_handle_c, &pi)) < 0) {
		fprintf(stderr, "snd_pcm_plugin_info failed: %s\n", snd_strerror (ret));
		return;
	}

	fprintf(stderr,"CAPTURE Minimum Rate = %d\n",pi.min_rate);

	// Request the VoIP parameters
	// These parameters are different to waverec sample
	memset(&pp, 0, sizeof(pp));
	fprintf(stderr,"CAPTURE Minimum fragment size = %d\n",pi.min_fragment_size);
	// Blocking read
	pp.mode = SND_PCM_MODE_BLOCK;
	pp.channel = SND_PCM_CHANNEL_CAPTURE;
	pp.start_mode = SND_PCM_START_DATA;
	// Auto-recover from errors
	pp.stop_mode = SND_PCM_STOP_ROLLOVER;
	pp.buf.block.frag_size = PREFERRED_FRAME_SIZE;
	pp.buf.block.frags_max = 1;
	pp.buf.block.frags_min = 1;
	pp.format.interleave = 1;
	pp.format.rate = VOIP_SAMPLE_RATE;
	pp.format.voices = 1;
	pp.format.format = SND_PCM_SFMT_S16_LE;
	// make the request
	if ((ret = snd_pcm_plugin_params(g_pcm_handle_c, &pp)) < 0) {
		fprintf(stderr, "snd_pcm_plugin_params failed: %s\n", snd_strerror (ret));
		return;
	}

	// Again based on the sample
	memset(&setup, 0, sizeof(setup));
	memset(&group, 0, sizeof(group));
	setup.channel = SND_PCM_CHANNEL_CAPTURE;
	setup.mixer_gid = &group.gid;
	if ((ret = snd_pcm_plugin_setup(g_pcm_handle_c, &setup)) < 0) {
		fprintf(stderr, "snd_pcm_plugin_setup failed: %s\n", snd_strerror (ret));
		return;
	}
	// On the simulator at least, our requested capabilities are accepted.
	fprintf(stderr,"CAPTURE Format %s card = %d\n", snd_pcm_get_format_name (setup.format.format),card);
	fprintf(stderr,"CAPTURE Rate %d \n", setup.format.rate);
	g_frame_size_c = setup.buf.block.frag_size;

	if (group.gid.name[0] == 0) {
		printf("Mixer Pcm Group [%s] Not Set \n", group.gid.name);
		printf("***>>>> Input Gain Controls Disabled <<<<*** \n");
	} else {
		printf("Mixer Pcm Group [%s]\n", group.gid.name);
	}

	// frag_size should be 160
	g_frame_size_c = setup.buf.block.frag_size;
	fprintf(stderr, "CAPTURE frame_size = %d\n", g_frame_size_c);

	// Sample calls prepare()
	if ((ret = snd_pcm_plugin_prepare(g_pcm_handle_c, SND_PCM_CHANNEL_CAPTURE))
			< 0) {
		fprintf(stderr, "snd_pcm_plugin_prepare failed: %s\n", snd_strerror (ret));
	}
}

static void playsetup() {

	snd_pcm_channel_setup_t setup;
	//Enabling Echo canceller
	int ret;
	snd_pcm_channel_info_t pi;
	snd_mixer_group_t group;
	snd_pcm_channel_params_t pp;

	//audio routing enabled with AEC
	if ((ret = audio_manager_snd_pcm_open_name(AUDIO_TYPE_VIDEO_CHAT,
			&g_pcm_handle_p, &g_audio_manager_handle_p, (char*) "voice",
			SND_PCM_OPEN_PLAYBACK)) < 0) {
		return;
	}

	if ((ret = snd_pcm_plugin_set_disable(g_pcm_handle_p, PLUGIN_DISABLE_MMAP))
			< 0) {
		fprintf(stderr, "snd_pcm_plugin_set_disable failed: %s\n", snd_strerror (ret));
		return;
	}

	if ((ret = snd_pcm_plugin_set_enable(g_pcm_handle_p, PLUGIN_ROUTING)) < 0) {
		fprintf(stderr, "snd_pcm_plugin_set_enable: %s\n", snd_strerror (ret));
		return;
	}
	memset(&pi, 0, sizeof(pi));
	pi.channel = SND_PCM_CHANNEL_PLAYBACK;
	if ((ret = snd_pcm_plugin_info(g_pcm_handle_p, &pi)) < 0) {
		fprintf(stderr, "snd_pcm_plugin_info failed: %s\n", snd_strerror (ret));
		return;
	}

	fprintf(stderr,"PLAY Minimum Rate = %d\n",pi.min_rate);
	// Interestingly on the simulator this returns 4096 but in reality always 170 is the result
	fprintf(stderr,"PLAY Minimum fragment size = %d\n",pi.min_fragment_size);

	memset(&pp, 0, sizeof(pp));

	// Request VoIP compatible capabilities
	// On simulator frag_size is always negotiated to 170
	pp.mode = SND_PCM_MODE_BLOCK;
	pp.channel = SND_PCM_CHANNEL_PLAYBACK;
	pp.start_mode = SND_PCM_START_DATA;
	pp.stop_mode = SND_PCM_STOP_ROLLOVER;
	pp.buf.block.frag_size = PREFERRED_FRAME_SIZE;
	// Increasing this internal buffer count delays write failure in the loop
	pp.buf.block.frags_max = 3;
	pp.buf.block.frags_min = 1;
	pp.format.interleave = 1;
	pp.format.rate = VOIP_SAMPLE_RATE;
	pp.format.voices = 1;
	pp.format.format = SND_PCM_SFMT_S16_LE;

	// Make the calls as per the wave sample
	if ((ret = snd_pcm_plugin_params(g_pcm_handle_p, &pp)) < 0) {
		fprintf(stderr, "snd_pcm_plugin_params failed: %s\n", snd_strerror (ret));
		return;
	}

	memset(&setup, 0, sizeof(setup));
	memset(&group, 0, sizeof(group));
	setup.channel = SND_PCM_CHANNEL_PLAYBACK;
	setup.mixer_gid = &group.gid;
	if ((ret = snd_pcm_plugin_setup(g_pcm_handle_p, &setup)) < 0) {
		fprintf(stderr, "snd_pcm_plugin_setup failed: %s\n", snd_strerror (ret));
		return;
	}

	fprintf(stderr,"PLAY frame_size %d \n", setup.buf.block.frag_size);
	fprintf(stderr,"PLAY Rate %d \n", setup.format.rate);
	g_frame_size_p = setup.buf.block.frag_size;

	if (group.gid.name[0] == 0) {
		fprintf(stderr,"FATAL Mixer Pcm Group [%s] Not Set \n", group.gid.name);
		exit (-1);
	}
	printf("Mixer Pcm Group [%s]\n", group.gid.name);
	if ((ret = snd_pcm_plugin_prepare(g_pcm_handle_p, SND_PCM_CHANNEL_PLAYBACK))
			< 0) {
		fprintf(stderr, "snd_pcm_plugin_prepare failed: %s\n", snd_strerror (ret));
	}
}

/**
 * Set up capture based on the audio sample waverec except we use VoIP parameters
 */
static int capture(circular_buffer_t* circular_buffer) {
	// Re-usable buffer for capture
	char *record_buffer;
	record_buffer = (char*) malloc(g_frame_size_c);

	// Some diagnostic variables
	int failed = 0;
	int totalRead = 0;
	snd_pcm_channel_status_t status;
	status.channel = SND_PCM_CHANNEL_CAPTURE;

	// Loop until stopAudio() flags us
	while (g_execute_audio) {
		// This blocking read appears to take much longer than 20ms on the simulator
		// but it never fails and always returns 160 bytes
		int read = snd_pcm_plugin_read(g_pcm_handle_c, record_buffer,
				g_frame_size_c);
		if (read < 0 || read != g_frame_size_c) {
			failed++;
			fprintf(stderr,"CAPTURE FAILURE: snd_pcm_plugin_read: %d requested = %d\n",read,g_frame_size_c);
			if (snd_pcm_plugin_status(g_pcm_handle_c, &status) < 0) {
				fprintf(stderr, "Capture channel status error: %d\n",status.status);
			} else {
				if (status.status == SND_PCM_STATUS_READY
				|| status.status == SND_PCM_STATUS_OVERRUN
				|| status.status == SND_PCM_STATUS_ERROR) {
					fprintf(stderr, "CAPTURE FAILURE:snd_pcm_plugin_status: = %d \n",status.status);
					if (snd_pcm_plugin_prepare (g_pcm_handle_c, SND_PCM_CHANNEL_CAPTURE) < 0) {
						fprintf (stderr, "Capture channel prepare error %d\n",status.status);
						exit (1);
					}
				}
			}
		} else {
			totalRead += read;
		}
		capture_ready = true;
		// On simulator always room in the circular buffer
		if (!writeToCircularBuffer(circular_buffer, record_buffer,
				g_frame_size_c)) {
			failed++;
		}
	}
	fprintf(stderr,"CAPTURE EXIT BEGIN\n");
	(void) snd_pcm_plugin_flush(g_pcm_handle_c, SND_PCM_CHANNEL_CAPTURE);
	//(void)snd_mixer_close (mixer_handle);
	(void) snd_pcm_close(g_pcm_handle_c);
	audio_manager_free_handle(g_audio_manager_handle_c);
	// IMPORTANT NB: You only get failed on capture if the play loop has exited hence the circular buffer fills. This is with the simulator
	fprintf(stderr, "CAPTURE EXIT Total Bytes read = %d failed = %d\n",totalRead,failed);
	free(record_buffer);
	return 0;
}

static void resetPlay() {
	snd_pcm_channel_status_t status;
	status.channel = SND_PCM_CHANNEL_PLAYBACK;
	if (snd_pcm_plugin_status(g_pcm_handle_p, &status) < 0) {
		fprintf(stderr, "FATAL Playback channel status error %d\n", status.status);
		exit (1);
	}
	if (status.status == SND_PCM_STATUS_READY
			|| status.status == SND_PCM_STATUS_UNDERRUN
			|| status.status == SND_PCM_STATUS_ERROR) {
		fprintf(stderr, "PLAYBACK FAILURE:snd_pcm_plugin_status: = %d \n",status.status);
		if (snd_pcm_plugin_prepare (g_pcm_handle_p, SND_PCM_CHANNEL_PLAYBACK) < 0) {
			fprintf (stderr, "FATAL Playback channel prepare error %d\n", status.status);
			exit (1);
		}
	}
}

/**
 * setup play based on the audio sample wave
 */
static int play(circular_buffer_t* circular_buffer) {

	// If there's nothing in the circular buffer play the sound of silence
	// In the real world you may well play the last frame you received. There are algorithms DSP experts know about apparently
	char* silence_buffer = (char*) malloc(g_frame_size_p);
	memset(silence_buffer, SILENCE, g_frame_size_p);

	// Data read from the circular buffer
	char* play_buffer = (char*) malloc(g_frame_size_p);
	memset(play_buffer, 0, g_frame_size_p);

	// Diagnostics
	int total_written = 0;
	int failed = 0;
	int capture_not_ready = 0;
	// Loop till stopAudio() flags us
	while (g_execute_audio) {
		int written;
		// Read the circular buffer
		// returns true only if there is data to satisfy frame_size
		if (!capture_ready) {
			++capture_not_ready;
			written = snd_pcm_plugin_write(g_pcm_handle_p, play_buffer,
					PREFERRED_FRAME_SIZE);
			if (written < 0 || written != PREFERRED_FRAME_SIZE) {
				fprintf(stderr,"PLAY RESET 1\n");
				resetPlay();
			}
			total_written += written;
		} else if (readFromCircularBuffer(circular_buffer, play_buffer,
				PREFERRED_FRAME_SIZE)) {
			written = snd_pcm_plugin_write(g_pcm_handle_p, play_buffer,
					PREFERRED_FRAME_SIZE);
			if (written < 0 || written != PREFERRED_FRAME_SIZE) {
				fprintf(stderr,"PLAY RESET 2\n");
				resetPlay();
			}
			total_written += written;
		} else {
			// You would expect the jitter buffer to be possibly empty at startup because threads are not synchronized
			// increasing the frags_max reduces the occurrences of failure here
			// On the simulator it always fails written = 0 presumably meaning an overrun
			written = snd_pcm_plugin_write(g_pcm_handle_p, silence_buffer,
					PREFERRED_FRAME_SIZE);
			if (written < 0 || written != PREFERRED_FRAME_SIZE) {
				fprintf(stderr,"PLAY RESET 3\n");
				resetPlay();
			}
			failed++;
		}
	}
	fprintf(stderr,"PLAY EXIT BEGIN\n");
	(void) snd_pcm_plugin_flush(g_pcm_handle_p, SND_PCM_CHANNEL_PLAYBACK);
	(void) snd_pcm_close(g_pcm_handle_p);
	audio_manager_free_handle(g_audio_manager_handle_p);
	free(play_buffer);
	free(silence_buffer);
	fprintf(stderr,"PLAY EXIT Total Bytes written = %d failed = %d capture_not_ready %d\n",total_written,failed,capture_not_ready);
	return 0;
}
