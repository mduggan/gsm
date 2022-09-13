/* Stuff for wav files */

#ifndef WAV_H
#define WAV_H

/* Data Types */
struct wavformatheader {
	char ckID[4];
	unsigned int nChunkSize;
	short wFormatTag;
	short nChannels;
	int nSamplesPerSec;
	int nAvgBytesPerSec;
	short nBlockAlign;
	short nBitsPerSample; /* only for PCM data */
};

struct wavheader {
	char riff[4];
	unsigned int size;
	char wave[4];
	struct wavformatheader fmt;
	char next_ck[4];
	unsigned int next_ck_size;
};


/* Prototypes */
/* Print the data contained in the wav header */
void print_wav_header(struct wavheader w);
/* check that the wav header contains the type we want */
int check_wav_header(struct wavheader w);

#endif
