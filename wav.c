#include <stdio.h>
#include <string.h>


#include "wav.h"

#define MAKEFOURCC(x,c1,c2,c3,c4) \
(x)[0] = c1; (x)[1] = c2; (x)[2] = c3; (x)[3] = c4; 

	
void print_wav_header(struct wavheader w) 
{

	printf("riff: %.4s, size: %d, wave: %.4s\n",
			w.riff, w.size, w.wave);
	printf(	"ckID: %.4s, nChunkSize: %d, wFormatTag: %d, nChannels: %d, \n"
			"nSamplesPerSec: %d, nAvgBytesPerSec: %d, nBlockAlign: %d, \n"
			"nBitsPerSample: %d\n", w.fmt.ckID, w.fmt.nChunkSize, w.fmt.wFormatTag,
			w.fmt.nChannels, w.fmt.nSamplesPerSec, w.fmt.nAvgBytesPerSec, w.fmt.nBlockAlign,
			w.fmt.nBitsPerSample);
	printf(	"next_chunk: %.4s, next_chunk_size: %d\n", w.next_ck, w.next_ck_size);
}

void set_wav_header (struct wavheader *w) 
{
	MAKEFOURCC(w->riff, 'R', 'I', 'F', 'F');
	MAKEFOURCC(w->wave, 'W', 'A', 'V', 'E');
	MAKEFOURCC(w->fmt.ckID, 'f', 'm', 't', ' ');
	w->fmt.wFormatTag = 1;
	w->fmt.nChannels = 1;
	w->fmt.nSamplesPerSec = 8000;
	w->fmt.nBitsPerSample = 16;
	w->size = 0;
	MAKEFOURCC(w->next_ck, 'd', 'a', 't', 'a');
	w->fmt.nAvgBytesPerSec = w->fmt.nSamplesPerSec*w->fmt.nBitsPerSample/8;
	/*nBlockAlign?*/
}

/* checks the wav header to make sure it's the type we want.. returns 0 if it is, non-zero if it's not. */
int check_wav_header(struct wavheader w)
{
	if (strncmp((w.riff), "RIFF", 4)) {
		fprintf(stderr, "RIFF header incorrect!\n");
		return 1;
	}
	if (strncmp((w.wave), "WAVE", 4)) {
		fprintf(stderr, "WAVE header incorrect!\n");
		return 2;
	}
	if (strncmp((w.fmt.ckID), "fmt ", 4)) {
		fprintf(stderr, "fmt header incorrect!\n");
		return 3;
	}
	if (w.fmt.wFormatTag != 1) {
		fprintf(stderr, "Data is not PCM!\n");
		return 4;
	}
	if (w.fmt.nChannels != 1) {
		fprintf(stderr, "Sound is not MONO\n");
		return 5;
	}
	if (w.fmt.nSamplesPerSec != 8000) {
		fprintf (stderr, "Warning: Sample Rate is not 8000\n");
		//return 6;
	}
	if (w.fmt.nBitsPerSample != 16) {
		fprintf(stderr, "Bit Depth is not 16\n");
		return 7;
	}
	if (w.size < 320) { /* 160 samples (1 packet) * 2 bytes per sample */
		fprintf(stderr, "Warning: Probably not enough data for one packet!\n");
		//return 8;
	}
	if(strncmp(w.next_ck, "data", 4)) {
		fprintf(stderr, "Next chunk is not data, I don't understand this file :(\n");
		return 9;
	}

	return 0;
}
