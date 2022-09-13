#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "gsm.h"
#include "gsm_pack.h"

int main(int argc, char *argv[]) {

	char *outfile, *buf;
	int inf, outf;
	struct gsm_frame *frame;
	unsigned int i, finished;

	if (argc != 3) {
		printf("Usage:\n  %s input.gsm output.gsm\n", argv[0]);
		printf("Operation:\n  unpacks the GSM file given in input.gsm\n");
		exit(1);
	}


	outfile = argv[2];
	if ((outf = open(outfile, O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR 
					| S_IRGRP | S_IWGRP
					| S_IROTH | S_IWOTH)) == -1) {
		perror("Error opening output");
		return 1;
	}


	/* input from file.gsm */
	if ((inf = open(argv[1], O_RDONLY)) == -1)
	{
		perror("Error opening input");
		return 1;
	}

	finished = 0;

	if((buf = malloc(33)) == NULL) {
		printf("Cannot malloc 33 bytes!\n");
		exit(255);
	}
	if((frame = malloc(sizeof(struct gsm_frame))) == NULL) {
		printf("Cannot malloc frame!\n");
		exit(255);
	}

	while(!finished) {

		/* Read a packed GSM frame */
		i = read(inf, buf, 33);
		if (i < 33) {
			finished = 1;
			continue;
		}

		/* Unpack it */
		gsm_unpack(frame, buf);

		/* write output */
		i = write(outf, frame, sizeof(struct gsm_frame));
		
	}

	close(inf);
	close(outf);

	return 0;
}

