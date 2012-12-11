/*
* voiceTest.c
*
* Created on: 2012-10-17
* Author: Ashish Pradhan
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

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <gulliver.h>
#include <sys/asoundlib.h>
#include <audio/audio_manager_routing.h>

#define L_FRAME 160

#define DEFAULT_OUTPUT_FILEPATH "/accounts/1000/shared/downloads/output.wav"

//#define HANDSET 1  // un-comment this line to set output device to headset. (default is speaker)
#ifdef HANDSET
static audio_manager_device_t device_type = AUDIO_DEVICE_HANDSET;
#endif

static char inputfilepath[2048] = {0};
static char outputfilepath[2048] = DEFAULT_OUTPUT_FILEPATH;

static unsigned char* inputdata = NULL;
static unsigned char* outputdata = NULL;

static int outputdatasize = 0;
static int inputDataStart = 0;

static FILE* inputfile = NULL;
static FILE* outputfile = NULL;

static int samplerate = 0;
static int channels = 0;
static int fragsize = 0;


 // prototypes
static void GetOptions(int argc, char* const* argv);
static void PrintHelp(void);
static void cli_print(char *fmt, ...);
static void cleanupDataStreams(void);
static int prepareDataStreams(void);
static int process_read(snd_pcm_t* hRecord);
static int process_write(snd_pcm_t* hPlay);
static int voice_test(void);

static void cli_print(char *fmt, ...) {
    //Print the provided string(s) along with the line prefix
    char buf[10] = { 0 };
    va_list( args);
    va_start(args, fmt);
    vsnprintf(buf, (size_t)10, fmt, args);
    if(fmt) {
        vfprintf(stdout, fmt, args);
    }
    va_end(args);
    if ((buf[0] == '\0') || (buf[0] == '\n')) {
        //Do not print the extra newline if input is empty or just a new line
    } else {
        fprintf(stdout, "\n");
    }
    fflush(stdout);
}

// Start up options parsing
static void GetOptions(int argc, char* const* argv)
{
    int c;

    while ((c = getopt(argc, argv, "d:o:h")) != -1)
    {
        switch (c)
        {
            case 'd':
                strlcpy(inputfilepath,optarg,sizeof(inputfilepath));
                break;
            case 'o':
                strlcpy(outputfilepath,optarg,sizeof(outputfilepath));
                break;
            case 'h':
                PrintHelp();
                exit(1);
                break;
            default:
                break;
        }
    }
}

static void PrintHelp()
{
    cli_print("voiceTest DEMO Options:");
    cli_print(" ./voiceTest -d <inputfile> -o <outputfile>");
}

// wave file handling functions
static int FindTag(FILE* fp, const char* tag)
{
    typedef struct {
        char tag[4];
        int32_t length;
    } RiffTag;

    int retVal;
    RiffTag tagBfr = { "", 0 };

    retVal = 0;

    if((fp==NULL) || (tag==NULL)) {
        return -1;
    }
    // Keep reading until we find the tag or hit the EOF.
    while (fread ((unsigned char *) &tagBfr, sizeof (tagBfr), (size_t)1, fp))
    {
        // If this is our tag, set the length and break.
        if (strncmp (tag, tagBfr.tag, sizeof tagBfr.tag) == 0) {
            retVal = ENDIAN_LE32 (tagBfr.length);
            break;
        }
        // Skip ahead the specified number of bytes in the stream
        fseek (fp, (long)tagBfr.length, SEEK_CUR);
    }
    // Return the result of our operation
    return (retVal);
}

static int CheckHdr(FILE* fp)
{
    typedef struct {
        char Riff[4];
        int32_t Size;
        char Wave[4];
    } RiffHdr;

    const char *kRiffId = "RIFF";
    const char *kWaveId = "WAVE";

    RiffHdr riffHdr = { "", 0 };

    if(fp==NULL) {
        return -1;
    }
    // Read the header and, if successful, play the file
    // file or WAVE file.
    if ((fread ((unsigned char *) &riffHdr, (size_t)1, sizeof (RiffHdr), fp)) < sizeof (RiffHdr)) {
        return -1;
    }

    if (strncmp (riffHdr.Riff, kRiffId, strlen (kRiffId)) ||
        strncmp (riffHdr.Wave, kWaveId, strlen (kWaveId))) {
        return -1;
    }

    return 0;
}

static void WriteHdr(FILE* fp, int sampleRate, int nChannels, int dataSize)
{
    struct {
        char riff_id[4];
        int32_t wave_len;
        struct {
            char fmt_id[8];
            char fmt_len[4];
            struct {
                char format_tag[2];
                int16_t voices;
                int32_t rate;
                int32_t char_per_sec;
                int16_t block_align;
                int16_t bits_per_sample;
            } fmt;
            struct {
                char data_id[4];
                int32_t data_len;
            } data;
        } wave;
    } riff_hdr = {
        {'R', 'I', 'F', 'F'},
        0,
        {
            {'W', 'A', 'V', 'E', 'f', 'm', 't', ' ' },
            {sizeof (riff_hdr.wave.fmt), 0, 0, 0},
            {{1, 0 }, 0, 0, 0, 0, 0},
            {{'d', 'a', 't', 'a'}, 0},
        }
    };

    int a;
    if (fp) {
        fseek(fp, (long)0, SEEK_SET);
        riff_hdr.wave.fmt.voices = ENDIAN_LE16 (nChannels);
        riff_hdr.wave.fmt.rate = ENDIAN_LE32 (sampleRate);
        a = sampleRate * nChannels * 2;
        riff_hdr.wave.fmt.char_per_sec = ENDIAN_LE32 (a);
        a = nChannels * 2;
        riff_hdr.wave.fmt.block_align = ENDIAN_LE16 (a);
        riff_hdr.wave.fmt.bits_per_sample = ENDIAN_LE16 (16);
        a = dataSize;
        riff_hdr.wave.data.data_len = ENDIAN_LE32 (a);
        a = (((unsigned int)dataSize + sizeof(riff_hdr)) - (unsigned int)8);
        riff_hdr.wave_len = ENDIAN_LE32 (a);
        if (fwrite(&riff_hdr, (size_t)1, sizeof(riff_hdr), fp) != sizeof(riff_hdr)) {
            cli_print("Error writing output wave header");
        }
    } else {
        cli_print("Error! invalid fp");
    }

}

static int ReadHdr(FILE* fp, int* pSampleRate, int* pChannels, int* pDataStart)
{
    int rtn = 0;
    struct {
        int16_t FormatTag;
        int16_t Channels;
        int32_t SamplesPerSec;
        int32_t AvgBytesPerSec;
        int16_t BlockAlign;
        int16_t BitsPerSample;
    } wavHdr;

    if (fp && pSampleRate && pChannels && pDataStart) {
        int mSamples = FindTag(fp, "fmt ");
        int nread = fread(&wavHdr, sizeof(wavHdr), (size_t)1, fp);
        if (nread <= 0) {
            cli_print("ReadHdr: error in reading wavHdr");
            return -1;
        }

        *pSampleRate = ENDIAN_LE32 (wavHdr.SamplesPerSec);
        *pChannels = ENDIAN_LE16 (wavHdr.Channels);

        fseek(fp, (long)((unsigned int)mSamples - sizeof(wavHdr)), SEEK_CUR);
        rtn = FindTag(fp, "data");
        *pDataStart = ftell(fp);
    }
    return rtn;
}

static int process_write(snd_pcm_t* hPlay)
{
    // calculate read chunk size to ensure a entire sample point (all channels)
    // is read
    size_t iReadChunkSize = sizeof(short) * (size_t)channels;
    size_t iNumReadChunks = (unsigned int)fragsize / iReadChunkSize;

    int nread = fread(inputdata, iReadChunkSize, iNumReadChunks, inputfile);
    nread *= (int)iReadChunkSize;
    if (nread <= 0) {
        cli_print("%s: stopping, end of input", __func__);
        return 1;
    }
    if (nread > 0) {
        int nwritten;
        // make it a full frag
        if (nread < fragsize) {
            memset(inputdata + nread, 0, (size_t)(fragsize - nread));
        }
        nwritten = snd_pcm_plugin_write(hPlay, inputdata, (size_t)fragsize);
        if (nwritten != fragsize) {
            cli_print("snd_pcm_plugin_write: %s", snd_strerror(nwritten));
            return 1;
        }
    }
    return 0;
}

static int process_read(snd_pcm_t* hRecord)
{
    int nread = snd_pcm_plugin_read(hRecord, outputdata, (size_t)fragsize);
    if (nread != fragsize) {
         cli_print("snd_pcm_plugin_read: %s", snd_strerror(nread));
         return 1;
    } else {
        if (fwrite((void*)outputdata, (size_t)1, (size_t)nread, outputfile) != (size_t)nread) {
            cli_print("fwrite: %s", strerror(errno));
            return 1;
        } else {
            outputdatasize += nread;
        }
    }
    return 0;
}

static int prepareDataStreams() {

    int iErr = 0;

    inputdata = (unsigned char*)calloc((size_t)1, (size_t)fragsize);
    if (inputdata == NULL) {
        cli_print("error allocating buffers for input data %s", strerror(errno));
        iErr = ENOMEM;
    }
    outputdata = (unsigned char*)calloc((size_t)1, (size_t)fragsize);
    if (outputdata == NULL) {
        cli_print("error allocating buffers for output data %s", strerror(errno));
        iErr = ENOMEM;
    }

    if ((inputfile = fopen(inputfilepath, "r")) == 0) {
        cli_print("error opening %s!", inputfilepath);
        iErr = EBADF;
    } else {
        cli_print("Opened input file %s",inputfilepath);
        // seek to start of data
        fseek(inputfile, (long)inputDataStart, SEEK_SET);
    }

    if ((outputfile = fopen(outputfilepath, "w")) == 0) {
        cli_print("error opening %s!", outputfilepath);
        iErr = EBADF;
    } else {
        cli_print("Opened output file %s",outputfilepath);
        // Insert a dummy audio file header at the start of the output file
        outputdatasize = 0;
        WriteHdr(outputfile, samplerate, channels, outputdatasize);
    }

    if (iErr != 0) {
        cleanupDataStreams();
    }

    return iErr;
}

static void cleanupDataStreams(void) {

    if (inputfile) {
        fclose(inputfile);
        inputfile = NULL;
    }
    if (outputfile) {
       // seek to beginning of file and update header with correct size
       WriteHdr(outputfile, samplerate, channels, outputdatasize);
       fclose(outputfile);
       outputfile = NULL;
    }
    if (inputdata) {
        free(inputdata);
        inputdata = NULL;
    }
    if (outputdata) {
        free(outputdata);
        outputdata = NULL;
    }
}

typedef struct {
    snd_pcm_t* pDs; // pcm handle
    unsigned int hAudioman; // audioman handle
    int hFD; // pcm file descriptor handle
} Audio_t;

static int voice_test()
{
    int rtn = 0;
    char* name;
    fd_set rfds, wfds;
    Audio_t hPcm_Mic;
    Audio_t hPcm_Spk;
    snd_pcm_channel_params_t params;
    bool bQuit = false;

    /************************ CAPTURE **************************/

    //  configuring capture (mic)
    name = "voice";

    //  get audioman handle
    rtn = audio_manager_get_handle(AUDIO_TYPE_VIDEO_CHAT, getpid(), (bool)false, &hPcm_Mic.hAudioman);
    if(rtn < 0) {
        cli_print("audio_manager_get_handle (mic) failed %s", strerror(-rtn));
        return -1;
     }

    cli_print("Opening %s - for capture", name);
    rtn = snd_pcm_open_name(&hPcm_Mic.pDs, name, SND_PCM_OPEN_CAPTURE);
    if (rtn < 0) {
        (void)audio_manager_free_handle(hPcm_Mic.hAudioman);
        cli_print("snd_pcm_open_name (mic) failed %s", snd_strerror(rtn));
        return -1; // snd_pcm calls return negative values; make positive
    }

    rtn = snd_pcm_set_audioman_handle(hPcm_Mic.pDs, hPcm_Mic.hAudioman);
    if (rtn < 0) {
        (void)snd_pcm_close(hPcm_Mic.pDs);
        (void)audio_manager_free_handle(hPcm_Mic.hAudioman);
        cli_print("snd_pcm_set_audioman_handle (mic) failed %s", snd_strerror(rtn));
        (void)snd_pcm_close(hPcm_Mic.pDs);
        return -1;
    }

    // disable mmap
    (void)snd_pcm_plugin_set_disable(hPcm_Mic.pDs, (unsigned int)PLUGIN_DISABLE_MMAP);

    // set parameters
    memset(&params, 0, sizeof(params));
    params.mode = SND_PCM_MODE_BLOCK;
    params.channel = SND_PCM_CHANNEL_CAPTURE;
    params.start_mode = SND_PCM_START_GO;
    params.stop_mode = SND_PCM_STOP_ROLLOVER;
    params.buf.block.frag_size = fragsize;
    params.buf.block.frags_max = 1;
    params.buf.block.frags_min = 1;
    params.format.rate = 8000; //samplerate;
    params.format.interleave = 1;
    params.format.voices = channels;
    params.format.format = SND_PCM_SFMT_S16_LE;

    rtn = snd_pcm_plugin_params(hPcm_Mic.pDs, &params);
    if (rtn < 0) {
        cli_print("snd_pcm_plugin_params (mic) failed %s", snd_strerror(rtn));
        (void)snd_pcm_close(hPcm_Mic.pDs);
        (void)audio_manager_free_handle(hPcm_Mic.hAudioman);
        return -1;
    }

    // get file descriptor for use with the select() call
    hPcm_Mic.hFD = snd_pcm_file_descriptor(hPcm_Mic.pDs, SND_PCM_CHANNEL_CAPTURE);
    if (hPcm_Mic.hFD < 0) {
        cli_print("snd_pcm_file_descriptor (mic) failed %s", snd_strerror(hPcm_Mic.hFD));
        (void)snd_pcm_close(hPcm_Mic.pDs);
        (void)audio_manager_free_handle(hPcm_Mic.hAudioman);
        return -1;
    }
    // Signal the driver to ready the capture channel
    rtn = snd_pcm_plugin_prepare(hPcm_Mic.pDs, SND_PCM_CHANNEL_CAPTURE);
    if (rtn < 0) {
        cli_print("snd_pcm_plugin_prepare (mic) failed %s", snd_strerror(errno));
        (void)snd_pcm_close(hPcm_Mic.pDs);
        (void)audio_manager_free_handle(hPcm_Mic.hAudioman);
        return -1;
    }
    fragsize = params.buf.block.frag_size;


    /************************ PLAYBACK **************************/

    name = "voice";

    // get and set audioman handle
    rtn = audio_manager_get_handle(AUDIO_TYPE_VIDEO_CHAT, getpid(), (bool)false, &hPcm_Spk.hAudioman);
    if (rtn < 0) {
        cli_print("audioman audio_manager_get_handle (spk) failed %s", strerror(-rtn) );
        (void)snd_pcm_close(hPcm_Mic.pDs);
        (void)audio_manager_free_handle(hPcm_Mic.hAudioman);
        return -1;
    }
#ifdef HANDSET
    // set audio manager handle type
    rtn = audio_manager_set_handle_type(hPcm_Spk.hAudioman, AUDIO_TYPE_VIDEO_CHAT, device_type, device_type);
    if (rtn < 0) {
        (void)snd_pcm_close(hPcm_Mic.pDs);
        (void)audio_manager_free_handle(hPcm_Mic.hAudioman);
        (void)audio_manager_free_handle(hPcm_Spk.hAudioman);
        cli_print("audio_manager_set_handle_type (spk) failed %s", strerror(-rtn));
        return -1;
    }
#endif
    // Create a handle and open a connection to an audio interface specified by name
    cli_print("Opening %s - for playback", name);
    rtn = snd_pcm_open_name(&hPcm_Spk.pDs, name, SND_PCM_OPEN_PLAYBACK);
    if (rtn < 0) {
        cli_print("snd_pcm_open_name (spk) failed %s", snd_strerror(rtn));
        (void)snd_pcm_close(hPcm_Mic.pDs);
        (void)audio_manager_free_handle(hPcm_Mic.hAudioman);
        (void)audio_manager_free_handle(hPcm_Spk.hAudioman);
        return -1;
    }
    rtn = snd_pcm_set_audioman_handle(hPcm_Spk.pDs, hPcm_Spk.hAudioman);
    if (rtn < 0) {
        cli_print("snd_pcm_set_audioman_handle (spk) failed %s", snd_strerror(rtn));
        (void)snd_pcm_close(hPcm_Mic.pDs);
        (void)snd_pcm_close(hPcm_Spk.pDs);
        (void)audio_manager_free_handle(hPcm_Mic.hAudioman);
        (void)audio_manager_free_handle(hPcm_Spk.hAudioman);
        return -1;
    }

    // disable mmap
    (void)snd_pcm_plugin_set_disable(hPcm_Spk.pDs, (unsigned int)PLUGIN_DISABLE_MMAP);

    // set parameters
    memset(&params, 0, sizeof(params));
    params.mode = SND_PCM_MODE_BLOCK;
    params.channel = SND_PCM_CHANNEL_PLAYBACK;
    params.start_mode = SND_PCM_START_GO;
    params.stop_mode = SND_PCM_STOP_ROLLOVER;
    params.buf.block.frag_size = fragsize;
    params.buf.block.frags_max = 1;
    params.buf.block.frags_min = 1;
    params.format.rate = samplerate;
    params.format.interleave = 1;
    params.format.voices = channels;
    params.format.format = SND_PCM_SFMT_S16_LE;

    // Set the configurable parameters for a PCM channel
    rtn = snd_pcm_plugin_params(hPcm_Spk.pDs, &params);
    if (rtn < 0) {
        cli_print("snd_pcm_plugin_params  (spk) failed %s", snd_strerror(rtn));
        (void)snd_pcm_close(hPcm_Mic.pDs);
        (void)snd_pcm_close(hPcm_Spk.pDs);
        (void)audio_manager_free_handle(hPcm_Mic.hAudioman);
        (void)audio_manager_free_handle(hPcm_Spk.hAudioman);
        return -1;
    }

    // get file descriptor for use with the select() call
    hPcm_Spk.hFD = snd_pcm_file_descriptor(hPcm_Spk.pDs, SND_PCM_CHANNEL_PLAYBACK);
    if (hPcm_Spk.hFD < 0) {
        cli_print("snd_pcm_file_descriptor (spk) failed %s", snd_strerror(hPcm_Spk.hFD));
        (void)snd_pcm_close(hPcm_Mic.pDs);
        (void)snd_pcm_close(hPcm_Spk.pDs);
        (void)audio_manager_free_handle(hPcm_Mic.hAudioman);
        (void)audio_manager_free_handle(hPcm_Spk.hAudioman);
        return -1;
    }
    // Signal the driver to ready the playback channel
    rtn = snd_pcm_plugin_prepare(hPcm_Spk.pDs, SND_PCM_CHANNEL_PLAYBACK);
    if (rtn < 0) {
        cli_print("snd_pcm_plugin_prepare  (spk) failed %s", snd_strerror(rtn));
        (void)snd_pcm_close(hPcm_Mic.pDs);
        (void)snd_pcm_close(hPcm_Spk.pDs);
        (void)audio_manager_free_handle(hPcm_Mic.hAudioman);
        (void)audio_manager_free_handle(hPcm_Spk.hAudioman);
        return -1;
    }
    fragsize = params.buf.block.frag_size;

    rtn = snd_pcm_capture_go(hPcm_Mic.pDs);
    if( rtn < 0) {
        cli_print("snd_pcm_capture_go (mic) failed %s", snd_strerror(rtn));
        (void)snd_pcm_close(hPcm_Mic.pDs);
        (void)snd_pcm_close(hPcm_Spk.pDs);
        (void)audio_manager_free_handle(hPcm_Mic.hAudioman);
        (void)audio_manager_free_handle(hPcm_Spk.hAudioman);
        return -1;
    }
    rtn = snd_pcm_playback_go(hPcm_Spk.pDs);
    if (rtn < 0) {
        cli_print("snd_pcm_playback_go (spk) failed %s", snd_strerror(rtn));
        (void)snd_pcm_close(hPcm_Mic.pDs);
        (void)snd_pcm_close(hPcm_Spk.pDs);
        (void)audio_manager_free_handle(hPcm_Mic.hAudioman);
        (void)audio_manager_free_handle(hPcm_Spk.hAudioman);
        return -1;
    }

    /******************* PLAYBACK/RECORD LOOP **************************/
    while(!bQuit)
    {
        int width;
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 350000; // 350 ms

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_SET(hPcm_Mic.hFD, &rfds);
        FD_SET(hPcm_Spk.hFD, &wfds);

        width = ((hPcm_Spk.hFD > hPcm_Mic.hFD) ? hPcm_Spk.hFD : hPcm_Mic.hFD) + 1;
        rtn = select(width, &rfds, &wfds, NULL, &timeout);
        if (rtn > 0) {
            if (FD_ISSET(hPcm_Spk.hFD, &wfds)) {
                bQuit = process_write(hPcm_Spk.pDs);
            }
            if (FD_ISSET(hPcm_Mic.hFD, &rfds)) {
                bQuit = process_read(hPcm_Mic.pDs);
            }
        }
        else if (rtn == 0){
            cli_print("select: timed out");
            bQuit = true;
        }
        else { // (rtn < 0)
            cli_print("select: %s", strerror(errno));
            bQuit = true;
        }
    }

    // Ensure audio processing is stopped
    if ((rtn = snd_pcm_plugin_playback_drain(hPcm_Spk.pDs)) < 0) {
        cli_print("snd_pcm_plugin_playback_drain (spk) failed %s", snd_strerror(rtn));
    }
    if ((rtn = snd_pcm_plugin_flush(hPcm_Mic.pDs, SND_PCM_CHANNEL_CAPTURE)) < 0) {
        cli_print("snd_pcm_plugin_flush (mic) failed %s", snd_strerror(rtn));
    }
    if ((rtn = snd_pcm_close(hPcm_Spk.pDs)) < 0) {
        cli_print("snd_pcm_close (spk) failed %s", snd_strerror(rtn));
    }
    if ((rtn = snd_pcm_close(hPcm_Mic.pDs)) < 0) {
        cli_print("snd_pcm_close (mic) failed %s", snd_strerror(rtn));
    }
    if (hPcm_Spk.hAudioman) {
        (void)audio_manager_free_handle(hPcm_Spk.hAudioman);
    }
    if (hPcm_Mic.hAudioman) {
        (void)audio_manager_free_handle(hPcm_Mic.hAudioman);
    }
    return 0;
}


int main(int argc, char *argv[])
{
    int rtn = 0;
    // Handle command line options
    GetOptions(argc, argv);

    if (inputfilepath[0]) {
        if ((inputfile = fopen(inputfilepath, "r")) == 0) {
            cli_print("Failed opening input file %s",strerror(errno));
            return -1;
        }
        if (CheckHdr(inputfile) == -1) {
            cli_print("Input file not a wave file, exiting");
            fclose(inputfile);
            return -1;
        }
        rtn = ReadHdr(inputfile, &samplerate, &channels, &inputDataStart);
        fclose(inputfile);
        inputfile = NULL;

        // could not read header .. abort!
        if(rtn < 0) {
            cli_print("error in reading file header. Aborting!");
            return -1;
        }
    }
    else
    {
        cli_print("No input file specified. Please provide an input WAV file with the -i option.");
        return -1;
    }
    cli_print("rate: %d channels: %d", samplerate, channels);

    fragsize = (256 * channels) * (samplerate / 8000); //256;
    if (prepareDataStreams() != EOK) {
        cli_print("error initialising i/o streams; Aborting");
        return -1;
    }

    (void)voice_test();
    cli_print("Voice test over");

    cleanupDataStreams();

    return 0;
}
