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
#include <pthread.h>
#include <sys/timeb.h>

#include "png.h"
#include "bbutil.h"

static float width, height;
static GLuint background;

static GLfloat sine_wave_vertices[192];
static GLfloat square_wave_vertices[40];
static GLfloat triangle_wave_vertices[40];
static GLfloat sawtooth_wave_vertices[40];

static GLfloat square_vertices[8];

static GLfloat *wave_vertices;
static GLfloat *wave_colors;
static GLfloat *wave_vertices1;
static GLfloat *wave_colors1;

double frequency;
int lastX=-1, lastY=-1;
static bool sineWaves = true;
static bool squareWaves = false;
static bool triangleWaves = false;
static bool sawtoothWaves = false;

static screen_context_t screen_cxt;
static float pos_x, pos_y;

static font_t* font;

extern short *render_buffer;
extern pthread_mutex_t fillMutex;
extern int frag_samples;
extern int frame_size;

short *sample_buffer = NULL;


unsigned char   *first_buffer = NULL;
_uint64 first_buffer_touch = 0, first_buffer_gen = 0, first_buffer_play = 0;


typedef struct dtmf
{
    int f1;
    int f2;
} dtmf;

typedef enum {
	SineWaves,
	SquareWaves,
	TriangleWaves,
	SawtoothWaves,
    DTMF
} ToneType;


extern ToneType selectedType;

typedef struct {
    ToneType type;
    union {
        struct dtmf dtmf;       // For DTMF tone
        double frequency;          // For FREQ_TONE
    };
} ToneData;

typedef struct {
    struct Tone *prev;
    struct Tone *next;
    int id;
    long start;
    long startFull;
    long endFull;
    long end;
    double intensity;
    long position;
    bool active;
    bool killed;
    ToneData data;
} Tone;

extern int initToneGenerator();
extern int cleanupToneGenerator();
extern Tone *addTone(double frequency, double intensity);
extern void updateTone(Tone *tone, double intensity);
extern void endTone(Tone *tone);

Tone *lastTone[5] = { NULL, NULL, NULL, NULL, NULL };
float square_fade[5] = { 1.0, 1.0, 1.0, 1.0, 1.0 };


// render parameters
double deviceMainScopeY;
double deviceLargeScopeY;
double deviceFreqTextY;

double deviceSineWaveStartX;
double deviceSineWaveStartY;
double deviceSquareWaveStartX;
double deviceSquareWaveStartY;
double deviceTriangleWaveStartX;
double deviceTriangleWaveStartY;
double deviceSawtoothWaveStartX;
double deviceSawtoothWaveStartY;

int buttonTop;
double deviceTop;

// defaults for Z10
double z10MainScopeY = 400.0;
double z10LargeScopeY = 925.0;
double z10FreqTextY = 540.0;

double z10SineWaveStartX = 40.0;
double z10SineWaveStartY = 50.0;
double z10SquareWaveStartX = 232.0;
double z10SquareWaveStartY = 50.0;
double z10TriangleWaveStartX = 424.0;
double z10TriangleWaveStartY = 50.0;
double z10SawtoothWaveStartX = 606.0;
double z10SawtoothWaveStartY = 50.0;

int z10ButtonTop = 1024;
double z10Top = 1280.0;

// defaults for Q10
double q10MainScopeY = -1.0;
double q10LargeScopeY = 475.0;
double q10FreqTextY = 200.0;

double q10SineWaveStartX = 40.0;
double q10SineWaveStartY = 50.0;
double q10SquareWaveStartX = 232.0;
double q10SquareWaveStartY = 50.0;
double q10TriangleWaveStartX = 404.0;
double q10TriangleWaveStartY = 50.0;
double q10SawtoothWaveStartX = 586.0;
double q10SawtoothWaveStartY = 50.0;

int q10ButtonTop = 474;
double q10Top = 720.0;


int init() {
    EGLint surface_width, surface_height;

    int dpi = bbutil_calculate_dpi(screen_cxt);

    font = bbutil_load_font(
                "/usr/fonts/font_repository/monotype/SlatePro-Medium.ttf", 15, dpi);

    if (!font) {
        return EXIT_FAILURE;
    }

    //Query width and height of the window surface created by utility code
    eglQuerySurface(egl_disp, egl_surf, EGL_WIDTH, &surface_width);
    eglQuerySurface(egl_disp, egl_surf, EGL_HEIGHT, &surface_height);

    if (surface_height == 1280) {
    	// Z10
		deviceMainScopeY = z10MainScopeY;
		deviceLargeScopeY = z10LargeScopeY;
		deviceFreqTextY = z10FreqTextY;

		deviceSineWaveStartX = z10SineWaveStartX;
		deviceSineWaveStartY = z10SineWaveStartY;
		deviceSquareWaveStartX = z10SquareWaveStartX;
		deviceSquareWaveStartY = z10SquareWaveStartY;
		deviceTriangleWaveStartX = z10TriangleWaveStartX;
		deviceTriangleWaveStartY = z10TriangleWaveStartY;
		deviceSawtoothWaveStartX = z10SawtoothWaveStartX;
		deviceSawtoothWaveStartY = z10SawtoothWaveStartY;

		buttonTop = z10ButtonTop;
		deviceTop = z10Top;

    } else if (surface_height == 720) {
    	// Q5 / Q10
		deviceMainScopeY = q10MainScopeY;
		deviceLargeScopeY = q10LargeScopeY;
		deviceFreqTextY = q10FreqTextY;

		deviceSineWaveStartX = q10SineWaveStartX;
		deviceSineWaveStartY = q10SineWaveStartY;
		deviceSquareWaveStartX = q10SquareWaveStartX;
		deviceSquareWaveStartY = q10SquareWaveStartY;
		deviceTriangleWaveStartX = q10TriangleWaveStartX;
		deviceTriangleWaveStartY = q10TriangleWaveStartY;
		deviceSawtoothWaveStartX = q10SawtoothWaveStartX;
		deviceSawtoothWaveStartY = q10SawtoothWaveStartY;

		buttonTop = q10ButtonTop;
		deviceTop = q10Top;

    }


    EGLint err = eglGetError();
    if (err != 0x3000) {
        fprintf(stderr, "Unable to query EGL surface dimensions\n");
        return EXIT_FAILURE;
    }

    width = (float) surface_width;
    height = (float) surface_height;

    //Initialize GL for 2D rendering
    glViewport(0, 0, (int) width, (int) height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrthof(0.0f, width / height, 0.0f, 1.0f, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //Set world coordinates to coincide with screen pixels
    glScalef(1.0f / height, 1.0f / height, 1.0f);

    glShadeModel(GL_SMOOTH);

	// build wave vertices and normals
    int index = 0;
    int index1 = 0;

	// setup waves
	float x = 0.0;
	float x1 = 0.0;

	float y = 0.0;
	float y1 = 0.0;

	// setup sine wave icon vertices
	float startX = deviceSineWaveStartX;
	float startY = deviceSineWaveStartY;
	float thickX = 10.0;
	float thickY = 10.0;
	float stemX = 50.0;
	float stemY = 60.0;

	for (index = 0; index < 24; index++) {
		float angle = -180.0 + index * 15.0;
		float angle1 = -180.0 + (index+1) * 15.0;

		sine_wave_vertices[index*8 + 0] = startX + stemX * index / 12.0;
		sine_wave_vertices[index*8 + 1] = startY + stemY + sin(angle * M_PI / 180.0) * stemY;
		sine_wave_vertices[index*8 + 2] = startX + stemX * (index+1) / 12.0;
		sine_wave_vertices[index*8 + 3] = startY + stemY + sin(angle1 * M_PI / 180.0) * stemY;
		sine_wave_vertices[index*8 + 4] = startX + stemX * index / 12.0;
		sine_wave_vertices[index*8 + 5] = startY + stemY + sin(angle * M_PI / 180.0) * stemY - thickY;
		sine_wave_vertices[index*8 + 6] = startX + stemX * (index+1) / 12.0;
		sine_wave_vertices[index*8 + 7] = startY + stemY + sin(angle1 * M_PI / 180.0) * stemY - thickY;
	}


	// setup square wave icon vertices
	startX = deviceSquareWaveStartX;
	startY = deviceSquareWaveStartY;
	thickX = 5.0;
	thickY = 5.0;
	stemX = 50.0;
	stemY = 60.0;

	square_wave_vertices[ 0] = startX;
	square_wave_vertices[ 1] = startY + stemY;
	square_wave_vertices[ 2] = startX + thickX;
	square_wave_vertices[ 3] = startY + stemY;
	square_wave_vertices[ 4] = startX;
	square_wave_vertices[ 5] = startY;
	square_wave_vertices[ 6] = startX + thickX;
	square_wave_vertices[ 7] = startY;
	square_wave_vertices[ 8] = startX + thickX;
	square_wave_vertices[ 9] = startY + thickY;
	square_wave_vertices[10] = startX + thickX + stemX;
	square_wave_vertices[11] = startY + thickY;
	square_wave_vertices[12] = startX;
	square_wave_vertices[13] = startY;
	square_wave_vertices[14] = startX + thickX + stemX;
	square_wave_vertices[15] = startY;
	square_wave_vertices[16] = startX + thickX + stemX;
	square_wave_vertices[17] = startY + stemY*2;
	square_wave_vertices[18] = startX + thickX + stemX + thickX;
	square_wave_vertices[19] = startY + stemY*2;
	square_wave_vertices[20] = startX + thickX + stemX;
	square_wave_vertices[21] = startY;
	square_wave_vertices[22] = startX + thickX + stemX + thickX;
	square_wave_vertices[23] = startY;
	square_wave_vertices[24] = startX + thickX + stemX + thickX;
	square_wave_vertices[25] = startY + stemY*2;
	square_wave_vertices[26] = startX + (thickX + stemX)*2;
	square_wave_vertices[27] = startY + stemY*2;
	square_wave_vertices[28] = startX + thickX + stemX + thickX;
	square_wave_vertices[29] = startY + stemY*2 - thickY;
	square_wave_vertices[30] = startX + (thickX + stemX)*2;
	square_wave_vertices[31] = startY + stemY*2 - thickY;
	square_wave_vertices[32] = startX + (thickX + stemX)*2;
	square_wave_vertices[33] = startY + stemY*2;
	square_wave_vertices[34] = startX + (thickX + stemX)*2 + thickX;
	square_wave_vertices[35] = startY + stemY*2;
	square_wave_vertices[36] = startX + (thickX + stemX)*2;
	square_wave_vertices[37] = startY + stemY;
	square_wave_vertices[38] = startX + (thickX + stemX)*2 + thickX;
	square_wave_vertices[39] = startY + stemY;



	// setup triangle wave icon vertices
	startX = deviceTriangleWaveStartX;
	startY = deviceTriangleWaveStartY;
	thickX = 5.0;
	thickY = 5.0;
	stemX = 50.0;
	stemY = 60.0;

	triangle_wave_vertices[ 0] = startX;
	triangle_wave_vertices[ 1] = startY + stemY;
	triangle_wave_vertices[ 2] = startX + thickX;
	triangle_wave_vertices[ 3] = startY + stemY;
	triangle_wave_vertices[ 4] = startX + stemX*.75;
	triangle_wave_vertices[ 5] = startY;
	triangle_wave_vertices[ 6] = startX + stemX*.75 + thickX;
	triangle_wave_vertices[ 7] = startY;

	triangle_wave_vertices[ 8] = startX + stemX*.75 + thickX;
	triangle_wave_vertices[ 9] = startY + thickY;
	triangle_wave_vertices[10] = startX + stemX*2.25 + thickX;
	triangle_wave_vertices[11] = startY + stemY*2;
	triangle_wave_vertices[12] = startX + stemX*.75 + thickX;
	triangle_wave_vertices[13] = startY;
	triangle_wave_vertices[14] = startX + stemX*2.25 + thickX;
	triangle_wave_vertices[15] = startY + stemY*2 - thickY;

	triangle_wave_vertices[16] = startX + stemX*2.25 + thickX;
	triangle_wave_vertices[17] = startY + stemY*2;
	triangle_wave_vertices[18] = startX + stemX*3 + thickX;
	triangle_wave_vertices[19] = startY + stemY;
	triangle_wave_vertices[20] = startX + stemX*2.25 + thickX;
	triangle_wave_vertices[21] = startY + stemY*2 - thickY;
	triangle_wave_vertices[22] = startX + stemX*3 + thickX;
	triangle_wave_vertices[23] = startY + stemY - thickY;


	// setup sawtooth wave icon vertices
	startX = deviceSawtoothWaveStartX;
	startY = deviceSawtoothWaveStartY;
	thickX = 5.0;
	thickY = 5.0;
	stemX = 50.0;
	stemY = 60.0;

	sawtooth_wave_vertices[ 0] = startX;
	sawtooth_wave_vertices[ 1] = startY + stemY;
	sawtooth_wave_vertices[ 2] = startX + thickX;
	sawtooth_wave_vertices[ 3] = startY + stemY;
	sawtooth_wave_vertices[ 4] = startX;
	sawtooth_wave_vertices[ 5] = startY;
	sawtooth_wave_vertices[ 6] = startX + thickX;
	sawtooth_wave_vertices[ 7] = startY;
	sawtooth_wave_vertices[ 8] = startX + thickX;
	sawtooth_wave_vertices[ 9] = startY + thickY;
	sawtooth_wave_vertices[10] = startX + (thickX + stemX)*2;
	sawtooth_wave_vertices[11] = startY + stemY*2;
	sawtooth_wave_vertices[12] = startX + thickX;
	sawtooth_wave_vertices[13] = startY;
	sawtooth_wave_vertices[14] = startX + (thickX + stemX)*2;
	sawtooth_wave_vertices[15] = startY + stemY*2 - thickY;
	sawtooth_wave_vertices[16] = startX + (thickX + stemX)*2;
	sawtooth_wave_vertices[17] = startY + stemY*2;
	sawtooth_wave_vertices[18] = startX + (thickX + stemX)*2 + thickX;
	sawtooth_wave_vertices[19] = startY + stemY*2;
	sawtooth_wave_vertices[20] = startX + (thickX + stemX)*2;
	sawtooth_wave_vertices[21] = startY + stemY;
	sawtooth_wave_vertices[22] = startX + (thickX + stemX)*2 + thickX;
	sawtooth_wave_vertices[23] = startY + stemY;





    return EXIT_SUCCESS;
}

void render() {

	char freqText[20];

    int index = 0;
    int index1 = 0;

	// setup waves
	float x = 0.0;
	float x1 = 0.0;

	float y = 0.0;
	float y1 = 0.0;
	float y2 = 0.0;
	float y3 = 0.0;


	if (sample_buffer == NULL) {
		sample_buffer = malloc (frame_size);
		memset(sample_buffer, 0, frame_size);


		wave_vertices = calloc(8 * (frame_size), sizeof(GLfloat));
		wave_colors = calloc(16 * (frame_size), sizeof(GLfloat));
		wave_vertices1 = calloc(8 * (frame_size), sizeof(GLfloat));
		wave_colors1 = calloc(16 * (frame_size), sizeof(GLfloat));

		for (index = 0; index < (frame_size/4); index++) {
			x = (float)index;
			x1 = (float)(index+1);

			wave_vertices[index * 8 +  0] = x;
			wave_vertices[index * 8 +  1] = 0.0;
			wave_vertices[index * 8 +  2] = x1;
			wave_vertices[index * 8 +  3] = 0.0;
			wave_vertices[index * 8 +  4] = x;
			wave_vertices[index * 8 +  5] = 0.0;
			wave_vertices[index * 8 +  6] = x1;
			wave_vertices[index * 8 +  7] = 0.0;


			wave_colors[index * 16 +  0] = 1.0;
			wave_colors[index * 16 +  1] = 0.8;
			wave_colors[index * 16 +  2] = 0.0;
			wave_colors[index * 16 +  3] = 1.0;
			wave_colors[index * 16 +  4] = 1.0;
			wave_colors[index * 16 +  5] = 0.8;
			wave_colors[index * 16 +  6] = 0.0;
			wave_colors[index * 16 +  7] = 1.0;
			wave_colors[index * 16 +  8] = 0.0;
			wave_colors[index * 16 +  9] = 0.0;
			wave_colors[index * 16 + 10] = 0.0;
			wave_colors[index * 16 + 11] = 1.0;
			wave_colors[index * 16 + 12] = 0.0;
			wave_colors[index * 16 + 13] = 0.0;
			wave_colors[index * 16 + 14] = 0.0;
			wave_colors[index * 16 + 15] = 1.0;
		}

		for (index = 0; index < (frame_size/16); index++) {
			x = (float)(index) * 4.0;
			x1 = (float)(index+1) * 4.0 + 1.0;

			wave_vertices1[index * 8 +  0] = x;
			wave_vertices1[index * 8 +  1] = 0.0;
			wave_vertices1[index * 8 +  2] = x1;
			wave_vertices1[index * 8 +  3] = 0.0;
			wave_vertices1[index * 8 +  4] = x;
			wave_vertices1[index * 8 +  5] = 0.0;
			wave_vertices1[index * 8 +  6] = x1;
			wave_vertices1[index * 8 +  7] = 0.0;

			wave_colors1[index * 16 +  0] = 0.0;
			wave_colors1[index * 16 +  1] = 1.0;
			wave_colors1[index * 16 +  2] = 0.0;
			wave_colors1[index * 16 +  3] = 1.0;
			wave_colors1[index * 16 +  4] = 0.0;
			wave_colors1[index * 16 +  5] = 1.0;
			wave_colors1[index * 16 +  6] = 0.0;
			wave_colors1[index * 16 +  7] = 1.0;
			wave_colors1[index * 16 +  8] = 0.0;
			wave_colors1[index * 16 +  9] = 0.0;
			wave_colors1[index * 16 + 10] = 0.0;
			wave_colors1[index * 16 + 11] = 1.0;
			wave_colors1[index * 16 + 12] = 0.0;
			wave_colors1[index * 16 + 13] = 0.0;
			wave_colors1[index * 16 + 14] = 0.0;
			wave_colors1[index * 16 + 15] = 1.0;
		}
	}

	pthread_mutex_lock(&fillMutex);

	if (render_buffer != NULL) {
		memcpy(sample_buffer, render_buffer, frame_size);
	}

	pthread_mutex_unlock(&fillMutex);



	short sample = 0, sample1 = 0;

	// build wave vertices and normals for main scope
	if (deviceMainScopeY > 0) {
		for (index = 0; index < frag_samples; index++) {
			sample = sample_buffer[index*2];
			sample1 = sample_buffer[(index+1)*2];

			y = (float)(sample ) / 600.0 + deviceMainScopeY;
			y1 = (float)(sample1 ) / 600.0 + deviceMainScopeY;
			y2 = y  - 20.0;
			y3 = y1 - 20.0;

			wave_vertices[index * 8 +  1] = y;
			wave_vertices[index * 8 +  3] = y1;
			wave_vertices[index * 8 +  5] = y2;
			wave_vertices[index * 8 +  7] = y3;
		}
	}


	// build wave vertices and normals for zoomed scope
	for (index = 0; index < frag_samples / 2; index++) {
		sample = sample_buffer[index*2];
		sample1 = sample_buffer[(index+1)*2];

		y = (float)( sample ) / 100.0 + deviceLargeScopeY;
		y1 = (float)( sample1 ) / 100.0 + deviceLargeScopeY;
		y2 = y  - 40.0;
		y3 = y1 - 40.0;

		wave_vertices1[index * 8 +  1] = y;
		wave_vertices1[index * 8 +  3] = y1;
		wave_vertices1[index * 8 +  5] = y2;
		wave_vertices1[index * 8 +  7] = y3;
	}


    //Typical rendering pass
    glClear(GL_COLOR_BUFFER_BIT);


	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, wave_vertices1);
	glColorPointer(4, GL_FLOAT, 0, wave_colors1);

	for (index = 0; index < (frame_size/16); index++) {
		glDrawArrays(GL_TRIANGLE_STRIP, index * 8, 4);
	}

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);



	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, wave_vertices);
	glColorPointer(4, GL_FLOAT, 0, wave_colors);

	for (index = 0; index < (frame_size/4); index++) {
		glDrawArrays(GL_TRIANGLE_STRIP, index * 8, 4);

	}

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);


    //Setup touch location polygon
	for (index = 0; index < 5; index ++) {
		if (lastTone[index] != NULL) {
			// calculate position from frequency
			int h = ((int)lastTone[index]->data.frequency / 240) * 33;
			int w = ((int)fmod(lastTone[index]->data.frequency, 240.0) / 10) * 32;

			if (square_fade[index] == 1.0) {
				square_vertices[0] = (float)w;
				square_vertices[1] = deviceTop - (float)h;
				square_vertices[2] = (float)w+32.0;
				square_vertices[3] = deviceTop - (float)h;
				square_vertices[4] = (float)w;
				square_vertices[5] = deviceTop - (float)h-33.0;
				square_vertices[6] = (float)w+32.0;
				square_vertices[7] = deviceTop - (float)h-33.0;

				fprintf(stderr, "freq box: %f : %f %f %f %f %d %d\n", lastTone[index]->data.frequency, square_vertices[0], square_vertices[1], square_vertices[6], square_vertices[7], w, h);

				glColor4f(0.8f, 0.8f, 1.0f, 1.0f);
				glEnableClientState(GL_VERTEX_ARRAY);

				glVertexPointer(2, GL_FLOAT, 0, square_vertices);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				glDisableClientState(GL_VERTEX_ARRAY);

				square_fade[index] /= 1.25;
			} else
			if (square_fade[index] > 0.0) {
				square_vertices[0] -= 2.0;
				square_vertices[1] += 2.0;
				square_vertices[2] += 2.0;
				square_vertices[3] += 2.0;
				square_vertices[4] -= 2.0;
				square_vertices[5] -= 2.0;
				square_vertices[6] += 2.0;
				square_vertices[7] -= 2.0;

				glColor4f(0.8f, 0.8f, 1.0f, square_fade[index]);
				glEnableClientState(GL_VERTEX_ARRAY);

				glVertexPointer(2, GL_FLOAT, 0, square_vertices);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				glDisableClientState(GL_VERTEX_ARRAY);

				square_fade[index] /= 1.25;
				if (square_fade[index] < 0.1) {
					square_fade[index] = 0.0;
				}

			}
		}
	}

	//fprintf(stderr, "square_fade: %f\n", square_fade);

    //Use utility code to render welcome text onto the screen
	if (frequency < 1000.0) {
		sprintf(freqText, "%3.2f Hz", frequency);
	} else {
		sprintf(freqText, "%3.2f kHz", frequency / 1000.0);
	}

    float text_width, text_height;
    bbutil_measure_text(font, freqText, &text_width, &text_height);
    pos_x = (width - text_width) / 2;
    pos_y = deviceFreqTextY;

    bbutil_render_text(font, freqText, pos_x, pos_y, 0.8f, 0.8f, 0.8f, 1.0f);

    // draw sine wave button
	if (sineWaves) {
		glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
	} else {
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, sine_wave_vertices);

	for (index = 0; index < 24; index++) {
		glDrawArrays(GL_TRIANGLE_STRIP, index * 4, 4);
	}

	glDisableClientState(GL_VERTEX_ARRAY);

    // draw square wave button
	if (squareWaves) {
		glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
	} else {
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, square_wave_vertices);

	for (index = 0; index < 5; index++) {
		glDrawArrays(GL_TRIANGLE_STRIP, index * 4, 4);
	}

	glDisableClientState(GL_VERTEX_ARRAY);


    // draw triangle wave button
	if (triangleWaves) {
		glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
	} else {
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, triangle_wave_vertices);

	for (index = 0; index < 3; index++) {
		glDrawArrays(GL_TRIANGLE_STRIP, index * 4, 4);
	}

	glDisableClientState(GL_VERTEX_ARRAY);


    // draw sawtooth wave button
	if (sawtoothWaves) {
		glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
	} else {
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, sawtooth_wave_vertices);

	for (index = 0; index < 3; index++) {
		glDrawArrays(GL_TRIANGLE_STRIP, index * 4, 4);
	}

	glDisableClientState(GL_VERTEX_ARRAY);



    //Use utility code to update the screen
    bbutil_swap();
}

_uint64 currentMillis()
{
	int result = 0;

	_uint64 millis = 0;
	struct timeb time;

	if (!ftime(&time)) {
		millis = (_uint64)time.time * (_uint64)1000 + (_uint64)time.millitm;
	}
   	//fprintf(stderr, "millis: time %lu  millitm %u  millis %llu\n", (long)time.time, time.millitm, millis);

	return millis;
}

void handleScreenEvent(bps_event_t *event) {
    int screenEvent;
    int buttons;
    int position[2];
    int realPosition[2];
    int touchID;
    int touchPressure;

    //static bool mouse_pressed = false;

    screen_event_t screen_event = screen_event_get_event(event);

    //Query type of screen event and its location on the screen
    screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_TYPE,
            &screenEvent);
    screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_SOURCE_POSITION,
    		position);
    screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_POSITION,
    		realPosition);
    screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_TOUCH_ID,
    		&touchID);
    screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_TOUCH_PRESSURE,
    		&touchPressure);


    int x = 0;
    int y = 0;
    int index = 0;
	unsigned short presample;
	short sample = 0, sample1 = 0;

	switch (screenEvent) {
		case SCREEN_EVENT_MTOUCH_TOUCH:
			x = realPosition[0];
			y = realPosition[1];

			if (y < buttonTop) {
				x /= 32;
				x *= 10;

				y /= 33;

				frequency = (float)x + (float)y * 240.0;
				if (frequency < 0) {
					frequency = 0;
				}

				fprintf(stderr, "last tone: %x\n", lastTone[touchID] );
				if (lastTone[touchID] != NULL) {
					endTone(lastTone[touchID]);
				}

				lastTone[touchID] = addTone(frequency, (float)touchPressure / 80.0);
				fprintf(stderr, "tone: %x\n", lastTone[touchID] );

				square_fade[touchID] = 1.0;

				if (first_buffer_touch == 0 && first_buffer == NULL) {
					first_buffer_touch = currentMillis();
				}

				fprintf(stderr, "touch: %d %d,%d %d %d,%d %f\n", touchID, realPosition[0], realPosition[1], touchPressure, x, y, frequency );

			} else {
				if (x >= 0 && x < 192) {
					selectedType = SineWaves;
					sineWaves = true;
					squareWaves = false;
					triangleWaves = false;
					sawtoothWaves = false;

					fprintf(stderr, "switch to sine waves\n");
				}
				if (x >= 192 && x < 384) {
					selectedType = SquareWaves;
					sineWaves = false;
					squareWaves = true;
					triangleWaves = false;
					sawtoothWaves = false;

					fprintf(stderr, "switch to square waves\n");
				}
				if (x >= 384 && x < 576) {
					selectedType = TriangleWaves;
					sineWaves = false;
					squareWaves = false;
					triangleWaves = true;
					sawtoothWaves = false;

					fprintf(stderr, "switch to triangle waves\n");
				}
				if (x >= 576 && x < 768) {
					selectedType = SawtoothWaves;
					sineWaves = false;
					squareWaves = false;
					triangleWaves = false;
					sawtoothWaves = true;

					fprintf(stderr, "switch to sawtooth waves\n");
				}
			}
			break;

		case SCREEN_EVENT_MTOUCH_MOVE:

			x = realPosition[0];
			y = realPosition[1];

			if (y < buttonTop) {
				x /= 32;
				x *= 10;

				y /= 33;

				frequency = (float)x + (float)y * 240.0;
				if (frequency < 0) {
					frequency = 0;
				}

				if (lastTone[touchID] == NULL || (lastTone[touchID] != NULL && lastTone[touchID]->data.frequency != frequency)) {
					fprintf(stderr, "last tone: %lx\n", lastTone[touchID] );
					if (lastTone[touchID] != NULL) {
						endTone(lastTone[touchID]);
					}

					lastTone[touchID] = addTone(frequency, (float)touchPressure / 80.0);
					fprintf(stderr, "tone: %lx\n", lastTone[touchID] );
				}

				square_fade[touchID] = 1.0;

				fprintf(stderr, "touch move: %d %d,%d %d %d,%d %f\n", touchID, realPosition[0], realPosition[1], touchPressure, x, y, frequency );
			} else {
				if (x >= 0 && x < 192) {
					selectedType = SineWaves;
					sineWaves = true;
					squareWaves = false;
					triangleWaves = false;
					sawtoothWaves = false;

					fprintf(stderr, "switch to sine waves\n");
				}
				if (x >= 192 && x < 384) {
					selectedType = SquareWaves;
					sineWaves = false;
					squareWaves = true;
					triangleWaves = false;
					sawtoothWaves = false;

					fprintf(stderr, "switch to square waves\n");
				}
				if (x >= 384 && x < 576) {
					selectedType = TriangleWaves;
					sineWaves = false;
					squareWaves = false;
					triangleWaves = true;
					sawtoothWaves = false;

					fprintf(stderr, "switch to triangle waves\n");
				}
				if (x >= 576 && x < 768) {
					selectedType = SawtoothWaves;
					sineWaves = false;
					squareWaves = false;
					triangleWaves = false;
					sawtoothWaves = true;

					fprintf(stderr, "switch to sawtooth waves\n");
				}
			}
			break;

		case SCREEN_EVENT_MTOUCH_RELEASE:
			x = realPosition[0];
			y = realPosition[1];

			if (y < buttonTop) {
				x /= 32;
				x *= 10;

				y /= 33;

				frequency = (float)x + (float)y * 240.0;
				if (frequency < 0) {
					frequency = 0;
				}

				fprintf(stderr, "tone: %lx\n", lastTone[touchID] );
				if (lastTone[touchID] != NULL) {
					endTone(lastTone[touchID]);
					lastTone[touchID] = NULL;
				}

				square_fade[touchID] = 1.0;

				fprintf(stderr, "touch release: %d %d,%d %d %d,%d %f\n", touchID, realPosition[0], realPosition[1], touchPressure, x, y, frequency );
			} else {
				if (x >= 0 && x < 192) {
					selectedType = SineWaves;
					sineWaves = true;
					squareWaves = false;
					triangleWaves = false;
					sawtoothWaves = false;

					fprintf(stderr, "switch to sine waves\n");
				}
				if (x >= 192 && x < 384) {
					selectedType = SquareWaves;
					sineWaves = false;
					squareWaves = true;
					triangleWaves = false;
					sawtoothWaves = false;

					fprintf(stderr, "switch to square waves\n");
				}
				if (x >= 384 && x < 576) {
					selectedType = TriangleWaves;
					sineWaves = false;
					squareWaves = false;
					triangleWaves = true;
					sawtoothWaves = false;

					fprintf(stderr, "switch to triangle waves\n");
				}
				if (x >= 576 && x < 768) {
					selectedType = SawtoothWaves;
					sineWaves = false;
					squareWaves = false;
					triangleWaves = false;
					sawtoothWaves = true;

					fprintf(stderr, "switch to sawtooth waves\n");
				}
			}
			break;

		case SCREEN_EVENT_POINTER:
	        screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_BUTTONS,
				&buttons);

	    	switch (buttons) {
	    		case SCREEN_LEFT_MOUSE_BUTTON:
	    		case SCREEN_RIGHT_MOUSE_BUTTON:
	                //handleClick(position[0], position[1]);
	    			break;
	    	}

			break;
	}
}

int main(int argc, char **argv) {
    int rc;

    //Create a screen context that will be used to create an EGL surface to to receive libscreen events
    screen_create_context(&screen_cxt, 0);

    //Initialize BPS library
    bps_initialize();

    //Use utility code to initialize EGL for rendering with GL ES 1.1
    if (EXIT_SUCCESS != bbutil_init_egl(screen_cxt)) {
        fprintf(stderr, "Unable to initialize EGL\n");
        screen_destroy_context(screen_cxt);
        return 0;
    }

    //Initialize app data
    if (EXIT_SUCCESS != init()) {
        fprintf(stderr, "Unable to initialize app logic\n");
        bbutil_terminate();
        screen_destroy_context(screen_cxt);
        return 0;
    }

    //Signal BPS library that navigator orientation is to be locked
    if (BPS_SUCCESS != navigator_rotation_lock(true)) {
        fprintf(stderr, "navigator_rotation_lock failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_cxt);
        return 0;
    }

    //Signal BPS library that navigator and screen events will be requested
    if (BPS_SUCCESS != screen_request_events(screen_cxt)) {
        fprintf(stderr, "screen_request_events failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_cxt);
        return 0;
    }

    if (BPS_SUCCESS != navigator_request_events(0)) {
        fprintf(stderr, "navigator_request_events failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_cxt);
        return 0;
    }

    initToneGenerator();

    //startPcmAudio();

    bps_event_t *event = NULL;
    bool exitApp = false;

    _uint64 lastMillis = 0, millis, diffMillis, minLoopMillis = 0x7fffffff, maxLoopMillis = 0, sumMillis = 0, millisCount = 0;
    double avgLoopMillis;

   	do {
    	do {
			//Request and process BPS next available event
			event = NULL;
			rc = bps_get_event(&event, 0);
			assert(rc == BPS_SUCCESS);

			if (event) {
				int domain = bps_event_get_domain(event);

				if (domain == screen_get_domain()) {
					handleScreenEvent(event);
				} else if (domain == navigator_get_domain()) {
					//handleNavigatorEvent(event);
					if (NAVIGATOR_EXIT == bps_event_get_code(event)) {
						event = NULL;
						exitApp = true;
					}
				}
			}
    	} while (event);

		render();

		millis = currentMillis();

		if (lastMillis > 0) {
			diffMillis = millis - lastMillis;

			millisCount++;

			sumMillis += diffMillis;
			avgLoopMillis = (double)sumMillis / (double)millisCount;

			if (diffMillis < minLoopMillis) {
				minLoopMillis = diffMillis;
			}

			if (diffMillis > maxLoopMillis) {
				maxLoopMillis = diffMillis;
			}

		   	//fprintf(stderr, "loop : millis %llu  lastmillis %llu  diff %llu  count %llu  min %llu  max %llu  avg %F\n", millis, lastMillis, diffMillis, millisCount, minLoopMillis, maxLoopMillis, avgLoopMillis);

		}

		lastMillis = millis;

	} while (!exitApp);

	if (first_buffer_play > 0 && first_buffer_gen > 0 && first_buffer_touch > 0) {
	   	fprintf(stderr, "latency: touch %llu  gen %llu  play %llu  total %llu\n",
	   		first_buffer_touch, (first_buffer_gen - first_buffer_touch), (first_buffer_play - first_buffer_gen), (first_buffer_play + 32 - first_buffer_touch));
	}
   	fprintf(stderr, "loop stats: lastdiff %llu  count %llu  min %llu  max %llu  avg %F\n", diffMillis, millisCount, minLoopMillis, maxLoopMillis, avgLoopMillis);

	//stopPcmAudio();
   	cleanupToneGenerator();

    //Stop requesting events from libscreen
    screen_stop_events(screen_cxt);

    //Shut down BPS library for this process
    bps_shutdown();

    //Destroy the font
    bbutil_destroy_font(font);

    //Use utility code to terminate EGL setup
    bbutil_terminate();

    //Destroy libscreen context
    screen_destroy_context(screen_cxt);
    return 0;
}
