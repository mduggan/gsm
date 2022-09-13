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

#define OUT_DSP 1
#define OUT_WAV 2
#define OUT_RAW 3

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
	char *outfile;
    unsigned char *buf;
	int inf, outf, s;
	struct wavheader w;
	char out_type = 0;

	short s1[160];
	struct gsm_frame *frame;
	struct gsm_decstate state;
	unsigned int i, finished;

	if (argc != 3) {
		printf("Usage:\n  %s input.gsm output.wav\n", argv[0]);
		printf("Operation:\n  Converts the input GSM encoded file to a WAV output file\n");
		printf("Notes:\n"
#ifdef HAVE_DEV_DSP
				"  * if output.wav is the value 'DSP', /dev/dsp "
				"will be used for streaming decoding.\n"
#endif
				"  * output.wav can be '-' for raw stdin input.\n"
				"  * output.wav can be '-' for output to stdout.\n");
		exit(1);
	}

#ifdef HAVE_DEV_DSP
	if (!(strcmp(argv[2], "DSP")))
	{
		out_type = OUT_DSP;
		/* if we're using DSP for output, we do special setup */
		fprintf(stderr, "Using /dev/dsp for output\n");
		outfile = "/dev/dsp";
		/* have to use RDWR to show that we can ioctl it */
		if ((outf = open(outfile, O_RDWR)) == -1) {
			perror("Error opening dsp");
			return 1;
		}

		if (set_dsp_ioctl(outf)) {
			close(outf);
			return 1;
		}

	} else {
#endif
		if (!(strcmp(argv[2], "-")))
		{
			out_type = OUT_RAW;
			/* input from stdin, raw mode */
			outf = STDOUT_FILENO;
		} else {

			out_type = OUT_WAV;
			/* if we're using a wav file, we open and 
			 * write the header */
			outfile = argv[2];
			if ((outf = open(outfile, O_WRONLY|O_CREAT,
							S_IRUSR | S_IWUSR |
							S_IRGRP | S_IWGRP
							)) == -1) {
				perror("Error opening output");
				return 1;
			}
			if ((s = write(outf, (void *)(&w), sizeof(struct wavheader))) == -1) {
				perror("Error writing wav header");
				return 1;
			}
			if (s != sizeof(struct wavheader))
				fprintf(stderr, "only wrote %d bytes (of %d)\n",
						s, sizeof(struct wavheader));

		}
#ifdef HAVE_DEV_DSP
	}
#endif

	if (!(strcmp(argv[1], "-"))) {
		/* input on stdin */
		inf = STDIN_FILENO;
	} else {
		/* input from file.gsm */
		if ((inf = open(argv[1], O_RDONLY)) == -1)
		{
			perror("Error opening input");
			return 1;
		}
	}


	/* do the real work here... */
	gsm_decode_init(&state);
	
	finished = 0;

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

	while(!finished) {

#ifdef USE_PACKED
		/* Read a packed GSM frame */
		i = read(inf, buf, 33);
		if (i < 33) {
			finished = 1;
			continue;
		}

		/* Unpack it */
		gsm_unpack(frame, buf);
#else 
		/* read unpacked gsm frame from the file */
		i = read(inf, frame, sizeof(struct gsm_frame));
		if (i < sizeof(struct gsm_frame)) {
			finished = 1;
			continue;
		}
#endif

		/* decode 20ms frame */
		gsm_decode(&state, frame, s1);

		/* write output */
		i = write(outf, s1, 160 * sizeof(short));
		
	}

	if (inf != STDIN_FILENO)
		close(inf);
	if (outf != STDOUT_FILENO)
		close(outf);

	return 0;
}

