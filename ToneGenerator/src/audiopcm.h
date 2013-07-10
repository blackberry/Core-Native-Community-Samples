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

#ifndef _AUDIOPCM_H_INCLUDED
#define _AUDIOPCM_H_INCLUDED


typedef struct _dtmf
{
    int f1;
    int f2;
} dtmf;

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


extern ToneType selectedType;

typedef struct _ToneData {
    ToneType type;
    union {
        dtmf dtmf;       // For DTMF tone
        double frequency;          // For FREQ_TONE
    };
} ToneData;

typedef struct _Tone {
    struct _Tone *prev;
    struct _Tone *next;
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
extern void cleanupToneGenerator();
extern Tone *addTone(double frequency, double intensity);
extern void updateTone(Tone *tone, double intensity);
extern void endTone(Tone *tone);


#endif
