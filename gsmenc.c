#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_DEV_DSP
#include <linux/soundcard.h>
#endif

#include "gsm.h"
#include "gsm_pack.h"
#include "wav.h"

/* Note: packing now works */
#define USE_PACKED

#ifdef HAVE_DEV_DSP
/* Setup /dev/dsp the way we want. returns 0 on scucces, non-zero on error */
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
	int inf, outf, s;
	struct wavheader w;

	short s1[160];
	struct gsm_frame *frame;
	struct gsm_encstate state;
	int i, finished;

	if (argc != 3) {
		printf("Usage:\n  %s input.wav output.gsm\n", argv[0]);
		printf("Operation:\n  Converts the input WAV file to a GSM encoded output file\n");
		printf("Notes:\n"
#ifdef HAVE_DEV_DSP
				"  * if input.wav is the value 'DSP', /dev/dsp "
				"will be used for streaming encoding.\n"
#endif
				"  * input.wav can be '-' for raw stdin input.\n"
				"  * output.gsm can be '-' for output to stdout.\n");
		exit(1);
	}

#ifdef HAVE_DEV_DSP
	if (!(strcmp(argv[1], "DSP")))
	{
		/* if we're using DSP for input, we do special setup */
		fprintf(stderr, "Using /dev/dsp\n");
		infile = "/dev/dsp";
		/* have to use RDWR to show that we can ioctl it */
		if ((inf = open(infile, O_RDWR)) == -1) {
			perror("Error opening dsp");
			return 1;
		}

		if (set_dsp_ioctl(inf)) {
			close(inf);
			return 1;
		}

	} else {
#endif
		if (!(strcmp(argv[1], "-")))
		{
			/* input from stdin, raw mode */
			inf = STDIN_FILENO;
		} else {

			/* if we're using a wav file, we read in the header */
			infile = argv[1];
			if ((inf = open(infile, O_RDONLY)) == -1) {
				perror("Error opening input");
				return 1;
			}

			if ((s = read(inf, (void *)(&w), sizeof(struct wavheader))) == -1) {
				perror("Error reading wav header");
				return 1;
			}
			if (s != sizeof(struct wavheader))
				fprintf(stderr, "only read %d bytes! (of %d)\n",
						s, sizeof(struct wavheader));

			if (check_wav_header(w))
				return 1;
		}
#ifdef HAVE_DEV_DSP
	}
#endif

	if (!(strcmp(argv[2], "-"))) {
		/* output to stdout */
		outf = STDOUT_FILENO;
	} else {
		/* output to file.gsm */
		if ((outf = open(argv[2], O_WRONLY | O_CREAT, 
						S_IRUSR | S_IWUSR | 
						S_IRGRP | S_IWGRP)) == -1)
		{
			perror("Error opening output");
			return 1;
		}
	}


	/* do the real work here... */
	gsm_encode_init(&state);

#ifdef USE_PACKED
	if((buf = malloc(33)) == NULL) {
		printf("Cannot malloc 33 bytes!\n");
		exit(255);
	}
#endif
	if((frame = malloc(sizeof(struct gsm_frame))) == NULL) {
		printf("Cannot malloc frame!\n");
		exit(255);
	}

	finished = 0;
	while(!finished) {
		/* read in 160 samples from the file */
		i = read(inf, s1, 160 * sizeof(short));
		if (i < 160) {
			//printf("finished.\n");
			finished = 1;
			continue;
		}

		/* encode 20ms frame */
		gsm_encode(&state, frame, s1);
		
#ifdef USE_PACKED
		/* Pack it and output it? */
		gsm_pack(frame, buf);
		/* Write a packed GSM frame */
		i = write(outf, buf, 33);
#else
		/* Write an unpacked GSM frame */
		i = write(outf, (char *)frame, sizeof(struct gsm_frame));

#endif

		if (i < 0) {
			perror("Error writing output");
		}

#if 0
		fprintf(stderr, "------------------------------------------------\n");
		for (i = 0; i < 7; i++)
			fprintf(stderr, "%d\t", frame->lars[i]);
		fprintf(stderr, "%d\n", frame->lars[7]);
		for (i = 0; i < 4; i++) {
			printf("%d\t%d\t\t", frame->N[i], frame->b[i]);
			printf("%d\t%d\n", frame->Mc[i], frame->xmaxc[i]);
			printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\n", frame->xMc[i][0],
				frame->xMc[i][1], frame->xMc[i][2], 
				frame->xMc[i][3], frame->xMc[i][4], 
				frame->xMc[i][5], frame->xMc[i][6]);
			printf("%d\t%d\t%d\t%d\t%d\t%d\n", frame->xMc[i][7],
				frame->xMc[i][8], frame->xMc[i][9], 
				frame->xMc[i][10], frame->xMc[i][11], 
				frame->xMc[i][12]);
		}
#endif
			
	}

	if (inf != STDIN_FILENO)
		close(inf);
	if (outf != STDOUT_FILENO)
		close(outf);

	return 0;
}

