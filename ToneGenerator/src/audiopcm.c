/*
 * Copyright (c) 2011-2012 Research In Motion Limited.
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
#include <math.h>
#include <time.h>
#include <screen/screen.h>
#include <pthread.h>
#include <errno.h>

#include <audio/audio_manager_routing.h>

#include <sys/asoundlib.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>


// ulaw silence is FF
static const char SILENCE = 0xFF;

long *stage_buffer = NULL;
short *stage_samples = NULL;
short *record_buffer = NULL;
short *render_buffer = NULL;

extern short  *first_buffer;
extern _uint64 first_buffer_touch, first_buffer_gen, first_buffer_play;

extern _uint64 currentMillis();



typedef enum {
    STOPPED,
    RUNNING,
    DEAD
} State;

typedef enum {
	SineWaves,
	SquareWaves,
	TriangleWaves,
	SawtoothWaves,
    DTMF
} ToneType;

struct dtmf
{
    int f1;
    int f2;
} dtmf_table [] = {
    {697, 1209},
    {697, 1336},
    {697, 1477},
    {697, 1633},
    {770, 1209},
    {770, 1336},
    {770, 1477},
    {770, 1633},
    {852, 1209},
    {852, 1336},
    {852, 1477},
    {852, 1633},
    {941, 1209},
    {941, 1336},
    {941, 1477},
    {941, 1633}
};

typedef struct {
    ToneType type;
    union {
        struct dtmf dtmf;       // For DTMF tone
        int frequency;          // For FREQ_TONE
    };
} ToneData;


typedef struct Tone {
    struct Tone *prev;
    struct Tone *next;
    long start;
    long startFull;
    long endFull;
    long end;
    long position;
    bool active;
    bool killed;
    ToneData data;
} Tone;

ToneType selectedType = SineWaves;

static pthread_mutex_t toneMutex;
pthread_mutex_t fillMutex;

static pthread_cond_t condvar_newtone;
static State live = STOPPED;
static int nextid = 0;

static int tone_count;
static Tone *tones = NULL;

static pthread_t toneGeneratorThread;

// handle for sound library
static snd_pcm_t *playback_handle;
static snd_mixer_group_t group;

// File handle on which to write our current status
static int sample_handle;
static unsigned int audioman_handle = 0;

// set of handles to monitor for writes (audio  handle)
static fd_set write_handles;

// Audio channel properties
static int card;
static int device;
static int format;
static int sample_frequency;
//static void *frag_buffer;

int frag_samples;
int frame_size;
long tonepos = 0;
long wavepos = 0;



Tone *addTone(double frequency){
	Tone *tone = NULL;
	bool newTone = false;

	pthread_mutex_lock(&toneMutex);

	tone = malloc(sizeof(Tone));
	if (tone != NULL) {
		memset(tone, 0, sizeof(Tone));

		tone->active = true;
		tone->killed = false;
		tone->data.type = selectedType;
		tone->data.frequency = frequency;
		tone->position = tonepos;
		tone->start = tonepos;
		tone->startFull = tonepos + frag_samples;
		tone->endFull = tone->startFull + 100000;
		tone->end = tone->startFull + 150000;
		tone->next = NULL;

		tone_count++;

		newTone = true;

		if (tones == NULL) {
			tones = tone;
			tone->prev = NULL;
		} else {
			Tone *listTone;
			for(listTone = tones; listTone != NULL; listTone = listTone->next) {
				if (listTone->next == NULL) {
					tone->prev = listTone;
					listTone->next = tone;
					break;
				}
			}
 		}

		fprintf (stderr,"addTone tone %lx tones %lx start %ld count %d \n", tone, tones, tone->start, tone_count);

		Tone *listTone;
		for(listTone = tones; listTone != NULL; listTone = listTone->next) {
			fprintf (stderr,"addTone::tone %lx prev %lx next %lx \n", listTone, listTone->prev, listTone->next);
		}
	}

	pthread_mutex_unlock(&toneMutex);

	if (newTone) {
		pthread_cond_signal( &condvar_newtone );
	}

	return tone;
}

void endTone(Tone *tone){

	pthread_mutex_lock(&toneMutex);

	if (tone != NULL) {
		tone->endFull = tonepos;
		tone->end = tone->endFull + frag_samples * (tone->endFull - tone->startFull) / (sample_frequency / 10.0);
		fprintf (stderr,"endTone tone %lx start %ld end %ld  \n", tone, tone->start, tone->end);
	}

	pthread_mutex_unlock(&toneMutex);
}


// For now, this only supports 16-bit samples
static int writeTone(Tone *tone, bool superimpose)
{
    int index;
    int returnCode = EOK;
    double amplitude = 0;
    double currentAngle;
    double value;
    short maxValue = 0;

    // Update for only as much time as we have remaining
    for(index=0; index < frag_samples; index++ ) {
		if ((index + tone->position) >= tone->start && (index + tone->position) < tone->startFull) {
			amplitude = 32767.0 * (double)(index + tone->position - tone->start) / (double)(tone->startFull - tone->start);
		}
		if ((index + tone->position) >= tone->startFull && (index + tone->position) < tone->endFull) {
			amplitude = 32767.0;
		}
		if ((index + tone->position) >= tone->endFull && (index + tone->position) < tone->end) {
			amplitude = 32767.0 * (1.0 - (double)(index + tone->position - tone->endFull) / (double)(tone->end - tone->endFull));
		}
		if ((index + tone->position) >= tone->end) {
			amplitude = 0.0;
			tone->active = false;
			returnCode = EINVAL;
		}

    	switch(tone->data.type) {
			case SineWaves:
				currentAngle = fmod(((index+tone->position)*2.0*M_PI*tone->data.frequency/sample_frequency), 2.0*M_PI);

				value = sin(currentAngle)*amplitude;
				break;

			case SquareWaves:
				currentAngle = fmod(((index+tone->position)*2.0*M_PI*tone->data.frequency/sample_frequency), 2.0*M_PI);

				if (currentAngle > M_PI) {
					value = -amplitude;
				} else {
					value =  amplitude;
				}
				break;

			case TriangleWaves:
				currentAngle = fmod(((index+tone->position)*2.0*M_PI*tone->data.frequency/sample_frequency), 2.0*M_PI);

				if (currentAngle >= 0.0 && currentAngle < 0.5*M_PI) {
					value = amplitude * currentAngle / (0.5*M_PI);
				} else if (currentAngle >= 0.5*M_PI && currentAngle < M_PI) {
					value = amplitude * (M_PI - currentAngle) / (0.5*M_PI);
				} else if (currentAngle > M_PI && currentAngle < (1.5*M_PI)) {
					value = -amplitude * (currentAngle - M_PI) / (0.5*M_PI);
				} else if (currentAngle > (1.5*M_PI)) {
					value = -amplitude * ((2.0*M_PI) - currentAngle) / (0.5*M_PI);
				}
				break;

			case SawtoothWaves:
				currentAngle = fmod(((index+tone->position)*2.0*M_PI*tone->data.frequency/sample_frequency), 2.0*M_PI);

				if (currentAngle >= 0.0 && currentAngle < (2.0*M_PI)) {
					value = amplitude * (currentAngle - M_PI) / M_PI;
				}
				break;

			case DTMF:
				value = (sin((index+tone->position)*2.0*M_PI*tone->data.dtmf.f1/sample_frequency)+sin((index+tone->position)*2.0*M_PI*tone->data.dtmf.f2/sample_frequency))*32767.0/2.0;
				break;
		}

    	if (superimpose == true) {
			stage_buffer[index] += value;
			stage_buffer[index] += value;
			stage_samples[index]++;
    	} else {
			stage_buffer[index] = value;
			stage_buffer[index] = value;
			stage_samples[index] = 1;
    	}

    	if ((short)value > maxValue) {
    		maxValue = (short)value;
    	}
    }

	fprintf (stderr,"writeTone: pos %ld max value %d\n", tone->position, maxValue);

	tone->position += frag_samples;

    return returnCode;

}

// Initialize audio and report back a few parameters of the channel
int setupAudio( )
{
    int error;
    snd_pcm_channel_info_t pi;
    snd_pcm_channel_params_t pp;
    snd_pcm_channel_setup_t ps;

    //if ((error = snd_pcm_open_name (&playback_handle, "pcmPreferred", SND_PCM_OPEN_PLAYBACK)) < 0) {
    if ((error = snd_pcm_open_name (&playback_handle, "tones", SND_PCM_OPEN_PLAYBACK)) < 0) {
        slogf( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "Unable to open preferred audio driver\n");
        return -error;
    }

    sample_handle = snd_pcm_file_descriptor( playback_handle, SND_PCM_CHANNEL_PLAYBACK );

    if ((error = snd_pcm_plugin_set_disable (playback_handle, PLUGIN_DISABLE_MMAP)) < 0) {
        slogf( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "Unable to disable mmap\n");
        snd_pcm_close(playback_handle);
        return -error;
    }

    if ((error = snd_pcm_plugin_set_enable (playback_handle, PLUGIN_ROUTING)) < 0)
    {
        slogf ( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "snd_pcm_plugin_set_enable failed: %s\n", snd_strerror (error));
        snd_pcm_close(playback_handle);
        return -error;
    }

    // Find what sample rate and format to use
    memset( &pi, 0, sizeof(pi) );
    pi.channel = SND_PCM_CHANNEL_PLAYBACK;
    if ((error = snd_pcm_channel_info (playback_handle, &pi)) < 0) {
        slogf( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "Unable to get plugin info\n");
        snd_pcm_close(playback_handle);
        return -error;
    }

    // Initialize the playback channel
    memset(&pp, 0, sizeof(pp) );
    pp.mode = SND_PCM_MODE_BLOCK;
    pp.channel = SND_PCM_CHANNEL_PLAYBACK;
    pp.start_mode = SND_PCM_START_FULL;
    pp.stop_mode = SND_PCM_STOP_STOP;
    pp.buf.block.frag_size = pi.max_fragment_size;
    pp.buf.block.frags_max = 3;
    pp.buf.block.frags_min = 1;
    pp.format.interleave = 1;
    pp.format.format =  SND_PCM_SFMT_S16_LE;
    pp.format.rate = 48000;
    pp.format.voices = 2;

    memset( &ps, 0, sizeof( ps ) );
    memset( &group, 0, sizeof( group ) );
    ps.channel = SND_PCM_CHANNEL_PLAYBACK;
    ps.mixer_gid = &group.gid;
    //strcpy(pp.sw_mixer_subchn_name, "Wave playback channel");
    strcpy(pp.sw_mixer_subchn_name, "voice_ringtone");

    if ((error = snd_pcm_plugin_params( playback_handle, &pp)) < 0)
    {
        slogf( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "Unable to set plugin params\n");
        snd_pcm_close(playback_handle);
        return -error;
    }

    if ((error = snd_pcm_plugin_prepare (playback_handle, SND_PCM_CHANNEL_PLAYBACK)) < 0) {
        slogf( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "Unable to set prepare plugin\n");
        snd_pcm_close(playback_handle);
        return -error;
    }

    if ((error = snd_pcm_plugin_setup (playback_handle, &ps)) < 0) {
        slogf( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "Unable to setup plugin\n");
        snd_pcm_close(playback_handle);
        return -error;
    }

    card = ps.mixer_card;
    device = ps.mixer_device;

    if( audioman_handle ) {
        if ((error = snd_pcm_set_audioman_handle (playback_handle, audioman_handle)) < 0) {
            slogf( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "Unable to set audioman handle \n");
            snd_pcm_close(playback_handle);
            return -error;
        }
    }

    sample_frequency = ps.format.rate;
    frag_samples = ps.buf.block.frag_size / 4;
    frame_size = ps.buf.block.frag_size;
    format = pp.format.format;
/*
    // Only support 16-bit samples for now
    frag_buffer = malloc( ps.buf.block.frag_size * 2 );
    if( frag_buffer == NULL ) {
        slogf( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "Unable to alloc frag buffer\n");
        snd_pcm_close(playback_handle);
        return ENOMEM;
    }
*/
	fprintf (stderr,"Playback Format %s card = %d\n", snd_pcm_get_format_name (ps.format.format),card);
	fprintf (stderr,"Playback preferred frame_size %d \n", pi.max_fragment_size);
	fprintf (stderr,"Playback frame_size %d \n", ps.buf.block.frag_size);
	fprintf (stderr,"Playback frame_samples %d \n", frag_samples);
	fprintf (stderr,"Playback Rate %d \n", ps.format.rate);

	if (group.gid.name[0] == 0)
	{
		fprintf (stderr,"Playback Mixer Pcm Group [%s] Not Set \n", group.gid.name);
        return -1;
	}
	fprintf (stderr, "Playback Mixer Pcm Group [%s]\n", group.gid.name);

    return EOK;
}

void setVolume(int level)
{
    int volume;
    snd_mixer_t *mixer_handle;

    if (snd_mixer_open (&mixer_handle, card, device) < 0) {
        slogf( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "Unable to open mixer\n");
        return;
    }

    snd_mixer_group_read(mixer_handle, &group);

    volume = (float)(group.max - group.min) * ( level / 100.0) + group.min;

    if (group.channels & SND_MIXER_CHN_MASK_FRONT_LEFT)
        group.volume.names.front_left = volume;
    if (group.channels & SND_MIXER_CHN_MASK_REAR_LEFT)
        group.volume.names.rear_left = volume;
    if (group.channels & SND_MIXER_CHN_MASK_FRONT_CENTER)
        group.volume.names.front_center = volume;
    if (group.channels & SND_MIXER_CHN_MASK_FRONT_RIGHT)
        group.volume.names.front_right = volume;
    if (group.channels & SND_MIXER_CHN_MASK_REAR_RIGHT)
        group.volume.names.rear_right = volume;
    if (group.channels & SND_MIXER_CHN_MASK_WOOFER)
        group.volume.names.woofer = volume;
    snd_mixer_group_write(mixer_handle, &group);

    snd_mixer_close (mixer_handle);
}


static void *processTones(void *dummy)
{
    bool done;
    int error;
    int length;
    int index;
    Tone *tone, *nextTone;
    snd_pcm_channel_status_t status;


    //pthread_setname_np(pthread_self(), "tonegen");

    // Main loop. Wait for audio driver to need more data
    while( live ) {
        pthread_mutex_lock( &toneMutex );
        if( tone_count == 0 ) {
            // Wait for a tone command to come in
            snd_pcm_playback_flush( playback_handle );
            snd_pcm_plugin_prepare( playback_handle, SND_PCM_CHANNEL_PLAYBACK );

            pthread_cond_wait( &condvar_newtone, &toneMutex );
        }
        if( tone_count == 0 ) {
            pthread_mutex_unlock( &toneMutex );
            continue;
        }
        pthread_mutex_unlock( &toneMutex );

        // Send tone data
        FD_ZERO( &write_handles );
        FD_SET( sample_handle, &write_handles );
        error = select(sample_handle + 1, NULL, &write_handles, NULL, NULL);

        if( error < 0 ) {
            // Select failed.
            slogf( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "processTones select failed %d (%d)\n", error, errno);
            pthread_mutex_unlock( &toneMutex );
            live = DEAD;
            break;
        }

        length = 0;

        // This should always be true
        if( FD_ISSET( sample_handle, &write_handles ) ) {
        	int active_count = 0;
        	for(tone = tones; tone != NULL; tone = tone->next) {
                done = tone->killed || (tonepos > tone->end);
    			//fprintf (stderr,"processTone::writeTone (before active check) tone %ld tone->end %ld active %d done %d tonepos %ld \n", tone, tone->end, tone->active, done, tonepos);
                if( !done ) {
                    // Write the tone
                    error = writeTone(tone, (tone != tones));
                    //error = tone->generator(tone->position, length, sample_frequency, frag_buffer, &tone->data, terminating);
                    if( error != EOK ) {
                        done = true;
                    }
                    active_count++;
                }

                tone->active &= !done;
    			//fprintf (stderr,"processTone::writeTone tone %ld tone->end %ld active %d tonepos %ld \n", tone, tone->end, tone->active, tonepos);
        	}

        	for(index = 0; index < frag_samples; index++) {
        		record_buffer[index*2  ] = stage_buffer[index] / stage_samples[index];
        		record_buffer[index*2+1] = stage_buffer[index] / stage_samples[index];
        	}

        	pthread_mutex_lock(&fillMutex);

        	if (render_buffer != NULL) {
        		memcpy(render_buffer, record_buffer, frame_size);
        	}

        	pthread_mutex_unlock(&fillMutex);

			tonepos += frag_samples;

			if (active_count > 0) {
				fprintf (stderr,"processTone tonepos %ld \n", tonepos);

				error = snd_pcm_plugin_write (playback_handle, record_buffer, frame_size);
				if( error != frame_size ) {
				   memset (&status, 0, sizeof (status));
					status.channel = SND_PCM_CHANNEL_PLAYBACK;
					if ((error = snd_pcm_plugin_status (playback_handle, &status)) < 0)
					{
						slogf( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "retrieving audio interface status failed (%s)\n", snd_strerror (error));
					} else if (status.status == SND_PCM_STATUS_READY ||
							   status.status == SND_PCM_STATUS_UNDERRUN ||
							   status.status == SND_PCM_STATUS_CHANGE)
					{
						if ((error = snd_pcm_plugin_prepare (playback_handle, SND_PCM_CHANNEL_PLAYBACK)) < 0) {
							slogf( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "cannot prepare audio interface for use (%s)\n", snd_strerror (error));
						}
					} else {
						slogf( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "non-underrun write failure (%s)\n", snd_strerror (error));
					}
					// Retry now that we're prepared
					error = snd_pcm_plugin_write (playback_handle, record_buffer, frame_size);
				}
				if( error != frame_size ) {
					slogf( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "write to audio interface failed (%s) %d\n", snd_strerror (error), error);
				}
			}

        } else {
            slogf( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "Unknown file handle activated" );
        }

		pthread_mutex_lock( &toneMutex );


		//fprintf (stderr,"processTone::delete tones %lx count %d \n", tones, tone_count);

        int delete_count;
        do {
        	delete_count = 0;
			for(tone = tones; tone != NULL; tone = tone->next) {
				done = tone->killed || (tonepos > tone->end);
				if( done || !tone->active ) {
					// Remove the tone from the list
					if (tone->prev != NULL) {
						tone->prev->next = tone->next;
					} else {
						tones = tone->next;
					}
					if (tone->next != NULL) {
						tone->next->prev = tone->prev;
					}

					tone_count--;

					free(tone);

					delete_count++;

					fprintf (stderr,"processTone::delete tone %lx tones %lx count %d \n", tone, tones, tone_count);

					break;
				}
			}
        } while (delete_count > 0);

		pthread_mutex_unlock( &toneMutex );

        if (tone_count == 0) {
        	memset(record_buffer, 0, frame_size);
        }
    }

    return NULL;
}

int initToneGenerator(void)
{
    _int32 staticMemSize, scratchMemSize;
    int policy;
    struct sched_param param;
    int error;
    pthread_attr_t attr;

    if( live == RUNNING) {
        return 0;
    } else if( live == DEAD ) {
        return -EINVAL;
    }


   if( (error = pthread_mutex_init (&fillMutex, NULL) ) != 0 ) {
		return error;
	}

    tone_count = 0;
    if( (error = pthread_mutex_init( &toneMutex, NULL ) ) != 0 ) {
        return error;
    }

    if( (error = pthread_cond_init( &condvar_newtone, NULL ) ) != 0 ) {
        pthread_mutex_destroy( &toneMutex );
        return error;
    }

    audio_manager_get_handle( AUDIO_TYPE_VOICE_TONES, getpid(), true, &audioman_handle );

    if( (error = setupAudio( )) != 0 ) {
        slogf( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "Unable to initialize audio %s\n", snd_strerror(error));
        pthread_mutex_destroy( &toneMutex );
        pthread_cond_destroy( &condvar_newtone );
        if( audioman_handle ) {
            audio_manager_free_handle( audioman_handle );
        }
        return error;
    }

	// Re-usable buffer for generation
	stage_buffer = malloc (frame_size);
	memset(stage_buffer, 0, frame_size);
	stage_samples = malloc (frame_size);
	memset(stage_samples, 0, frame_size);
	record_buffer = malloc (frame_size);
	memset(record_buffer, 0, frame_size);
	render_buffer = malloc (frame_size);
	memset(render_buffer, 0, frame_size);

    // Default tonegen volume is 75% to achieve a 16dB loss ( SW mixer uses 10%=6dB. )
    setVolume(85);

    live = RUNNING;

    pthread_attr_init (&attr);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_getschedparam (pthread_self (), &policy, &param);
    param.sched_priority=12;
    pthread_attr_setschedparam (&attr, &param);
    pthread_attr_setschedpolicy (&attr, SCHED_RR);

    if( (error = pthread_create( &toneGeneratorThread, &attr, processTones, NULL ) ) != 0 ) {
        slogf( _SLOG_SETCODE(_SLOGC_AUDIO, 0), _SLOG_CRITICAL, "Unable to create tone thread %s\n", strerror(error));
        snd_pcm_close (playback_handle);
        //free( frag_buffer );
        pthread_mutex_destroy( &toneMutex );
        pthread_cond_destroy( &condvar_newtone );

        live = STOPPED;
        if( audioman_handle ) {
            audio_manager_free_handle( audioman_handle );
        }
        pthread_attr_destroy(&attr);
        return error;
    }

    pthread_attr_destroy(&attr);

    return 0;
}


void cleanupToneGenerator()
{
    int i;

    if( live == STOPPED ) {
        return;
    }
    pthread_mutex_lock( &toneMutex );

    live = STOPPED;

    // Terminate all playbacks
    Tone *tone = NULL;
    for(tone = tones; tone != NULL; tone = tone->next ) {
        tone->active = false;
    }

    pthread_mutex_unlock( &toneMutex );
    pthread_cond_signal( &condvar_newtone );

    pthread_join( toneGeneratorThread, NULL );
    snd_pcm_close (playback_handle);

    //free( frag_buffer );

    if( audioman_handle ) {
        audio_manager_free_handle( audioman_handle );
    }

    pthread_cond_destroy( &condvar_newtone );

    pthread_mutex_destroy( &toneMutex );

    pthread_mutex_destroy (&fillMutex);

	free(record_buffer);
	free(render_buffer);
	free(stage_samples);
	free(stage_buffer);
}

