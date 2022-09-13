#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <linux/soundcard.h>

#include "gsm.h"
#include "gsm_pack.h"
#include "wav.h"

/* Note: packing now works */
#define USE_PACKED

/* Setup /dev/dsp the way we want. returns 0 on scucces, non-zero on error */
#if 0
int set_dsp_ioctl(int f) {
	int data;

	data = 16;
	if (ioctl(f, SOUND_PCM_WRITE_BITS, &data) == -1) {
		perror("Error on ioctl (write bits)");
		return 1;
	}
	if (data != 16) {
		fprintf(stderr, "Fatal: Couldn't set 16 bit sampling\n");
		return 2;
	}
	data = 1;
	if (ioctl(f, SOUND_PCM_WRITE_CHANNELS, &data) == -1) {
		perror("Error on ioctl (write channels)");
		return 3;
	}
	if (data != 1) {
		fprintf(stderr, "Fatal: Couldn't set mono input\n");
		return 4;
	}
	data = 8000;
	if (ioctl(f, SOUND_PCM_WRITE_RATE, &data) == -1) {
		perror("Error on ioctl (write rate)");
		return 5;
	}
	if (data != 8000) {
		fprintf(stderr, "Note: real sampling rate: %d", data);
	}

	return 0;
}
#endif

int main(int argc, char *argv[]) 
{
	char *infile, *buf;
	int inf, outf, carf, s;
	struct wavheader w;

	short s1[160];
	short cs1[160];
	short sout[160];
	struct gsm_vocstate state;
	int i, finished;

	if (argc != 4) {
		printf("Usage:\n  %s input.raw carrier.raw output.raw\n", argv[0]);
		printf("Operation:\n  It's a vocoder.\n");
		printf("Notes:\n"
				"  * if input.wav is the value 'DSP', /dev/dsp "
				"will be used for streaming encoding.\n"
				"  * input.wav can be '-' for raw stdin input.\n"
				"  * output.gsm can be '-' for output to stdout.\n");
		exit(1);
	}

	if (!(strcmp(argv[1], "DSP")))
	{
		/* if we're using DSP for input, we do special setup */
		fprintf(stderr, "Using /dev/dsp\n");
#if 0
		infile = "/dev/dsp";
		/* have to use RDWR to show that we can ioctl it */
		if ((inf = open(infile, O_RDWR)) == -1) {
			perror("Error opening dsp");
			return 1;
		}

		if (set_dsp_ioctl(inf)) {
			close(inf);
			return 1;
		}*/
#endif
	} else {
		if (!(strcmp(argv[1], "-")))
		{
			/* input from stdin, raw mode */
			inf = STDIN_FILENO;
		} else {
			infile = argv[1];
			if ((inf = open(infile, O_RDONLY)) == -1) {
				perror("Error opening input");
				return 1;
			}
		}
	}

	if ((carf = open(argv[2], O_RDONLY)) == -1) {
		perror("Error opening carrier");
		return 1;
	}

	if (!(strcmp(argv[3], "-"))) {
		/* output to stdout */
		outf = STDOUT_FILENO;
	} else {
		/* output to file.raw */
		if ((outf = open(argv[3], O_WRONLY | O_CREAT, 
						S_IRUSR | S_IWUSR | 
						S_IRGRP | S_IWGRP)) == -1)
		{
			perror("Error opening output");
			return 1;
		}
	}


	/* do the real work here... */
	gsm_vocode_init(&state);

	finished = 0;
	while(!finished) {
		/* read in 160 samples from the file */
		i = read(inf, s1, 160 * sizeof(short));
		if (i < 160) {
			//printf("finished.\n");
			finished = 1;
			continue;
		}
		read(carf, cs1, 160 * sizeof(short));

		/* vocode 20ms frame */
		gsm_vocode(&state, sout, s1, cs1);
		
		/* Write an unpacked GSM frame */
		i = write(outf, sout, 160 * sizeof(short));

		if (i < 0) {
			perror("Error writing output");
		}
	}

	if (inf != STDIN_FILENO)
		close(inf);
	if (outf != STDOUT_FILENO)
		close(outf);

	return 0;
}

