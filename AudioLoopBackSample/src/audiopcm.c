/* Copyright (c) 2012 Research In Motion Limited.
 * Copyright (c) 2012 Truphone Limited.
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
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


// Constants for clearer init code
// Jitter buffer size.
static const int JITTER_BUFFER_NUMBER_FRAMES = 20;
// Standard VoIP
static const int PREFERRED_FRAME_SIZE = 640;
static const int VOIP_SAMPLE_RATE = 16000;
// ulaw silence is FF
static const char SILENCE = 0xFF;
pthread_t capturethread;
pthread_t playerthread;

/**
 * Encapsulation of a circular buffer
 * read from head write to tail
 * size is constant available space
 * length is number of available unread bytes
 * mutex protects a complete frame read and write
 * buffer dynamically allocated
*/
typedef struct circular_buffer {
	int length;
	int head;
	int tail;
	int size;
	char* buffer;
	pthread_mutex_t mutex;
}circular_buffer_t;

static circular_buffer_t* createCircularBuffer(){
	circular_buffer_t* circular_buffer = malloc(sizeof(circular_buffer_t));
	memset(circular_buffer,0,sizeof(circular_buffer_t));
	circular_buffer->buffer = malloc(JITTER_BUFFER_NUMBER_FRAMES*PREFERRED_FRAME_SIZE);
	circular_buffer->size = JITTER_BUFFER_NUMBER_FRAMES*PREFERRED_FRAME_SIZE;
	pthread_mutex_init (&circular_buffer->mutex, NULL);
	return circular_buffer;
}

static void destroyCircularBuffer(circular_buffer_t* circular_buffer){
	pthread_mutex_destroy(&circular_buffer->mutex);
	free(circular_buffer->buffer);
	free(circular_buffer);
}

/**
 * Write to the tail index, increment it and do the modulo wrap-around
 */
static void writeByteToCircularBuffer(circular_buffer_t* circular_buffer,char val){
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
static bool writeToCircularBuffer(circular_buffer_t* circular_buffer,char* buffer,int number_bytes){
	// Check there's room
	// Not inside the mutex as it's read only
	if(circular_buffer->length + number_bytes > circular_buffer->size){
		return false;
	}
	// Mutex protect a number_bytes write to the circular buffer
	pthread_mutex_lock(&circular_buffer->mutex);
	int i;
	for(i=0;i<number_bytes;++i){
		writeByteToCircularBuffer(circular_buffer,buffer[i]);
	}
	pthread_mutex_unlock(&circular_buffer->mutex);
	return true;
}

/**
 * Returns True/False - Either there is data in the buffer to satisfy the request or it fails (false)<br>
 * Mutex protect the buffer<br>
 */
static bool readFromCircularBuffer(circular_buffer_t* circular_buffer,char buffer[],int number_bytes){
	// Check there's sufficient data
	// Not inside the mutex as it's read only
	if(circular_buffer->length < number_bytes){
		return false;
	}
	// Mutex protect a number_bytes read from the circular buffer
	pthread_mutex_lock(&circular_buffer->mutex);
	int i;
	for(i=0;i<number_bytes;++i){
		buffer[i] = readByteFromCircularBuffer(circular_buffer);
	}
	pthread_mutex_unlock(&circular_buffer->mutex);
	return true;
}

// Forward declarations
static int capture(circular_buffer_t* circular_buffer);
static int play(circular_buffer_t* circular_buffer);

// Flag to stop the recorder and capture
static bool g_execute_audio = true;
// Global used by both threads
static circular_buffer_t* g_circular_buffer;
static bool capture_ready = false;

/**
 * Thread func just calls the player
 * arg is the circular buffer
 */
static void*  playerThread( void*  arg ){
	fprintf(stderr,"Player thread is %d\n", pthread_self() );
	play(arg);
	return( 0 );
}

/**
 * Thread func just calls the capture<br>
 * arg is the circular buffer<br>
 */
static void* captureThread( void*  arg ){
	fprintf(stderr, "Recorder thread is %d\n", pthread_self() );
	capture(arg);
	return( 0 );
}
/**
 * Create the threads passing the circular buffer as the arg<br>
 * The startup not synchronized but not important as the jitter buffer is ample size<br>
 */
void startPcmAudio(){
	g_circular_buffer = createCircularBuffer();
	pthread_attr_t attr;
	pthread_attr_init( &attr );
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED );
	pthread_create(&capturethread, &attr, &captureThread, g_circular_buffer);
	pthread_create(&playerthread,&attr,&playerThread,g_circular_buffer);
}

/**
 * Set the thread exit flag and cleanup<br>
 */
void stopPcmAudio(){
	// Threads will see this flag every 20ms in their loop
	g_execute_audio = false;
	pthread_join(capturethread, NULL);
	pthread_join(playerthread, NULL);
	destroyCircularBuffer(g_circular_buffer);
}

/**
 * Set up capture based on the audio sample waverec except we use VoIP parameters
 */
static int capture(circular_buffer_t* circular_buffer){

	snd_pcm_channel_setup_t setup;
	snd_pcm_t *pcm_handle;
	int dev = 0;
	unsigned int handle;
	audio_manager_audio_type_t *type;


	//enables echo cancellation and routes audio to device earpiece
    //snd_pcm_open_name(&pcm_handle,"/dev/snd/voicec", SND_PCM_OPEN_CAPTURE);
	//snd_pcm_open_preferred(&pcm_handle, &card, &dev, SND_PCM_OPEN_CAPTURE);
	audio_manager_snd_pcm_open_name(AUDIO_TYPE_VOICE, &pcm_handle, &handle, "/dev/snd/voicec", SND_PCM_OPEN_CAPTURE);

	snd_pcm_channel_info_t pi;
	snd_mixer_group_t group;
	snd_pcm_channel_params_t pp;

	// sample reads the capabilities of the capture
	memset (&pi, 0, sizeof (pi));
	pi.channel = SND_PCM_CHANNEL_CAPTURE;
	int rtn = -1;
	if ((rtn = snd_pcm_plugin_info (pcm_handle, &pi)) < 0)
	{
		fprintf (stderr, "snd_pcm_plugin_info failed: %s\n", snd_strerror (rtn));
		return -1;
	}

	fprintf(stderr,"CAPTURE Minimum Rate = %d\n",pi.min_rate);

	// Request the VoIP parameters
	// These parameters are different to waverec sample
	memset (&pp, 0, sizeof (pp));
	fprintf(stderr,"CAPTURE Minimum fragment size = %d\n",pi.min_fragment_size);
	// Blocking read
	pp.mode = SND_PCM_MODE_BLOCK;
	pp.channel = SND_PCM_CHANNEL_CAPTURE;
	pp.start_mode = SND_PCM_START_DATA;
	// Auto-recover from errors
	pp.stop_mode = SND_PCM_STOP_ROLLOVER;
	pp.buf.block.frag_size = PREFERRED_FRAME_SIZE;
	// Not applicable for capture hence -1
	pp.buf.block.frags_max = -1;
	pp.buf.block.frags_min = 1;
	pp.format.interleave = 1;
	pp.format.rate = VOIP_SAMPLE_RATE;
	pp.format.voices = 1;
	pp.format.format = SND_PCM_SFMT_S16_LE;
	// make the request
	if ((rtn = snd_pcm_plugin_params (pcm_handle, &pp)) < 0){
		fprintf (stderr, "snd_pcm_plugin_params failed: %s\n", snd_strerror (rtn));
		return -1;
	}

	// Again based on the sample
	memset (&setup, 0, sizeof (setup));
	memset (&group, 0, sizeof (group));
	setup.channel = SND_PCM_CHANNEL_CAPTURE;
	setup.mixer_gid = &group.gid;
	if ((rtn = snd_pcm_plugin_setup (pcm_handle, &setup)) < 0){
		fprintf (stderr, "snd_pcm_plugin_setup failed: %s\n", snd_strerror (rtn));
		return -1;
	}
	int  card = setup.mixer_card;
	// On the simulator at least, our requested capabilities are accepted.
	fprintf (stderr,"CAPTURE Format %s card = %d\n", snd_pcm_get_format_name (setup.format.format),card);
	fprintf (stderr,"CAPTURE Rate %d \n", setup.format.rate);
	int frame_size = setup.buf.block.frag_size;

	if (group.gid.name[0] == 0){
		printf ("Mixer Pcm Group [%s] Not Set \n", group.gid.name);
		printf ("***>>>> Input Gain Controls Disabled <<<<*** \n");
	} else{
		printf ("Mixer Pcm Group [%s]\n", group.gid.name);
	}

	// frag_size should be 160
	frame_size = setup.buf.block.frag_size;
	fprintf (stderr, "CAPTURE frame_size = %d\n", frame_size);

	// Sample calls prepare()
	if ((rtn = snd_pcm_plugin_prepare (pcm_handle, SND_PCM_CHANNEL_CAPTURE)) < 0){
		fprintf (stderr, "snd_pcm_plugin_prepare failed: %s\n", snd_strerror (rtn));
	}

	// Re-usable buffer for capture
	char   *record_buffer;
	record_buffer = malloc (frame_size);

	// Some diagnostic variables
	int failed = 0;
	int totalRead = 0;
	snd_pcm_channel_status_t status;

	// Loop until stopAudio() flags us
	while(g_execute_audio){
		// This blocking read appears to take much longer than 20ms on the simulator
		// but it never fails and always returns 160 bytes
		int read = snd_pcm_plugin_read(pcm_handle, record_buffer, frame_size);
		if(read <0 || read != frame_size){
			failed++;

			if (snd_pcm_plugin_status (pcm_handle, &status) < 0){
			                    fprintf (stderr, "overrun: capture channel status error\n");
			                    exit (1);
			   }

			if (status.status == SND_PCM_STATUS_READY || status.status == SND_PCM_STATUS_OVERRUN) {
			      if (snd_pcm_plugin_prepare (pcm_handle, SND_PCM_CHANNEL_CAPTURE) < 0)
			          {
			              fprintf (stderr, "overrun: capture channel prepare error\n");
			              exit (1);
			          }
			  }




		}else{
			totalRead += read;
		}
		capture_ready = true;
		// On simulator always room in the circular buffer
		if(!writeToCircularBuffer(circular_buffer,record_buffer,frame_size)){
			failed++;
		}
	}

	(void)snd_pcm_plugin_flush (pcm_handle, SND_PCM_CHANNEL_CAPTURE);
	//(void)snd_mixer_close (mixer_handle);
	(void)snd_pcm_close (pcm_handle);

	// IMPORTANT NB: You only get failed on capture if the play loop has exited hence the circular buffer fills. This is with the simulator
	fprintf (stderr, "CAPTURE TOTAL Bytes read = %d failed = %d\n\n\n",totalRead,failed);
	free(record_buffer);

	return 0;
}

/**
 * setup play based on the audio sample wave
 */
static int play(circular_buffer_t* circular_buffer){

	snd_pcm_channel_setup_t setup;
	snd_pcm_t *pcm_handle;



	//Enabling Echo cancellor
	int     dev = 0;

	int rtn;
	snd_pcm_channel_info_t pi;
	snd_mixer_group_t group;
	snd_pcm_channel_params_t pp;
	unsigned int handle;
	audio_manager_audio_type_t *type;

	// No AEC
	/*if ((rtn = snd_pcm_open_preferred (&pcm_handle, &card, &dev, SND_PCM_OPEN_PLAYBACK)) < 0){
		return rtn;
	}*/

	//Enabling AEC
	/*if ((rtn = snd_pcm_open_name (&pcm_handle,"/dev/snd/voicep", SND_PCM_OPEN_PLAYBACK)) < 0){
		return rtn;
	} */

	//audio routing enabled with AEC
	if ((rtn = audio_manager_snd_pcm_open_name(AUDIO_TYPE_VOICE, &pcm_handle, &handle, "/dev/snd/voicep", SND_PCM_OPEN_PLAYBACK)) < 0){
			return rtn;
		}

	memset (&pi, 0, sizeof (pi));
	pi.channel = SND_PCM_CHANNEL_PLAYBACK;
	if ((rtn = snd_pcm_plugin_info (pcm_handle, &pi)) < 0)
	{
		fprintf (stderr, "snd_pcm_plugin_info failed: %s\n", snd_strerror (rtn));
		return -1;
	}

	fprintf(stderr,"PLAY Minimum Rate = %d\n",pi.min_rate);
	// Interestingly on the simulator this returns 4096 but in reality always 170 is the result
	fprintf(stderr,"PLAY Minimum fragment size = %d\n",pi.min_fragment_size);

	memset (&pp, 0, sizeof (pp));

	// Request VoIP compatible capabilities
	// On simulator frag_size is always negotiated to 170
	pp.mode = SND_PCM_MODE_BLOCK;
	pp.channel = SND_PCM_CHANNEL_PLAYBACK;
	pp.start_mode = SND_PCM_START_DATA;
	pp.stop_mode = SND_PCM_STOP_ROLLOVER;
	pp.buf.block.frag_size = PREFERRED_FRAME_SIZE;
	// Increasing this internal buffer count delays write failure in the loop
	pp.buf.block.frags_max = 4;
	pp.buf.block.frags_min = 1;
	pp.format.interleave = 1;
	pp.format.rate = VOIP_SAMPLE_RATE;
	pp.format.voices = 1;
	pp.format.format = SND_PCM_SFMT_S16_LE;

	// Make the calls as per the wave sample
	if ((rtn = snd_pcm_plugin_params (pcm_handle, &pp)) < 0){
		fprintf (stderr, "snd_pcm_plugin_params failed: %s\n", snd_strerror (rtn));
		return -1;
	}

	memset (&setup, 0, sizeof (setup));
	memset (&group, 0, sizeof (group));
	setup.channel = SND_PCM_CHANNEL_PLAYBACK;
	setup.mixer_gid = &group.gid;
	if ((rtn = snd_pcm_plugin_setup (pcm_handle, &setup)) < 0)
	{
		fprintf (stderr, "snd_pcm_plugin_setup failed: %s\n", snd_strerror (rtn));
		return -1;
	}
	int card = setup.mixer_card;

	fprintf (stderr,"PLAY Format %s card = %d\n", snd_pcm_get_format_name (setup.format.format),card);
	fprintf (stderr,"PLAY frame_size %d \n", setup.buf.block.frag_size);
	fprintf (stderr,"PLAY Rate %d \n", setup.format.rate);
	int frame_size = setup.buf.block.frag_size;

	if (group.gid.name[0] == 0)
	{
		fprintf (stderr,"Mixer Pcm Group [%s] Not Set \n", group.gid.name);
		exit (-1);
	}
	printf ("Mixer Pcm Group [%s]\n", group.gid.name);


	if ((rtn = snd_pcm_plugin_prepare (pcm_handle, SND_PCM_CHANNEL_PLAYBACK)) < 0)
		fprintf (stderr, "snd_pcm_plugin_prepare failed: %s\n", snd_strerror (rtn));




	// If there's nothing in the circular buffer play the sound of silence
	// In the real world you may well play the last frame you received. There are algorithms DSP experts know about apparently
	char* silence_buffer = malloc(frame_size);
	memset(silence_buffer,SILENCE,frame_size);

	// Data read from the circular buffer
	char* play_buffer = malloc(frame_size);
	memset(play_buffer,0,frame_size);

	// Diagnostics
	int total_written = 0;
	int failed = 0;
	int capture_not_ready = 0;
	snd_pcm_channel_status_t status;
	// Loop till stopAudio() flags us
	while(g_execute_audio){
		int written;
		// Read the circular buffer
		// returns true only if there is data to satisfy frame_size
		if(!capture_ready){
			++capture_not_ready;
			written = snd_pcm_plugin_write(pcm_handle,play_buffer,PREFERRED_FRAME_SIZE);
			if(written < 0 || written != PREFERRED_FRAME_SIZE){
				fprintf(stderr,"Player exit 1 written = %d size = %d\n",written,frame_size);
				if (snd_pcm_plugin_status (pcm_handle, &status) < 0) {
									fprintf (stderr, "underrun: playback channel status error\n");
									exit (1);
				}

				if (status.status == SND_PCM_STATUS_READY || status.status == SND_PCM_STATUS_UNDERRUN){
									if (snd_pcm_plugin_prepare (pcm_handle, SND_PCM_CHANNEL_PLAYBACK) < 0)
									{
										fprintf (stderr, "underrun: playback channel prepare error\n");
										exit (1);
									}
				}
				break;
			}
			total_written += written;

		}else if(readFromCircularBuffer(circular_buffer,play_buffer,PREFERRED_FRAME_SIZE)){
			written = snd_pcm_plugin_write(pcm_handle,play_buffer,PREFERRED_FRAME_SIZE);
			if(written < 0 || written != PREFERRED_FRAME_SIZE){
				fprintf(stderr,"Player exit 2 written = %d size = %d\n",written,frame_size);
				if (snd_pcm_plugin_status (pcm_handle, &status) < 0) {
									fprintf (stderr, "underrun: playback channel status error\n");
									exit (1);
				}

				if (status.status == SND_PCM_STATUS_READY || status.status == SND_PCM_STATUS_UNDERRUN){
									if (snd_pcm_plugin_prepare (pcm_handle, SND_PCM_CHANNEL_PLAYBACK) < 0)
									{
										fprintf (stderr, "underrun: playback channel prepare error\n");
										exit (1);
									}
				}

				break;
			}
			total_written += written;

		}else{
				// You would expect the jitter buffer to be possibly empty at startup because threads are not synchronized
				// increasing the frags_max reduces the occurrences of failure here
				// On the simulator it always fails written = 0 presumably meaning an overrun
				written = snd_pcm_plugin_write(pcm_handle,silence_buffer,PREFERRED_FRAME_SIZE);
				if(written < 0 || written != PREFERRED_FRAME_SIZE){
					fprintf(stderr,"Player exit 3 written = %d size = %d\n",written,frame_size);
					if (snd_pcm_plugin_status (pcm_handle, &status) < 0) {
										fprintf (stderr, "underrun: playback channel status error\n");
										exit (1);
					}

					if (status.status == SND_PCM_STATUS_READY || status.status == SND_PCM_STATUS_UNDERRUN){
										if (snd_pcm_plugin_prepare (pcm_handle, SND_PCM_CHANNEL_PLAYBACK) < 0)
										{
											fprintf (stderr, "underrun: playback channel prepare error\n");
											exit (1);
										}
					}
					break;
				}
				failed++;

		}
	}

	(void)snd_pcm_plugin_flush (pcm_handle, SND_PCM_CHANNEL_PLAYBACK);
	//(void)snd_mixer_close (mixer_handle);
	(void)snd_pcm_close (pcm_handle);

	free(play_buffer);
	free(silence_buffer);
	fprintf (stderr, "PLAY Total Bytes written = %d failed = %d capture_not_ready %d\n",total_written,failed,capture_not_ready);

	return 0;

}
