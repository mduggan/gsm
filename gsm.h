#ifndef _GSM_H_
#define _GSM_H_

struct gsm_frame {
	/* LPC Parameters */
	signed char lars[8]; /* LAR values -> OUTPUT, STA */

	/* LTP Parameters */
	unsigned char N[4]; /* 7 bits -> OUTPUT */
	unsigned char b[4]; /* 2 bits -> OUTPUT */

	/* RPE Parameters */
	unsigned char Mc[4]; /* 2 bits -> OUTPUT */
	unsigned char xMc[4][13]; /* 3 bits each -> OUTPUT */
	unsigned char xmaxc[4]; /* 6 bits each -> OUTPUT */
};

struct gsm_decstate {
	int nr_; /* LTP lag from previous frame */
	float d_[160]; /* LTP delay line */
	float lar__prev[8]; /* LARs from previous frame */
	float v[8]; /* Short term synthesis filter memory */
	float sro; /* De-emphasis filter memory */
};

struct gsm_encstate {
	/* This is required for inter-frame state memory. */
	float so;
	float sof;
	float lar__prev[8];	/* previous quantised LARs */
	/* These are required for inter-subframe state memory. */
	float d_[120];		/* LTP delay line */
	float u[8];		/* short term analysis filter memory */
};

struct gsm_vocstate {
	float so;
	float sof;
	float cso;
	float csof;
	
	float lar__prev[8];
	float v[8];
	float sro;
};

struct gsm_signals {
	/* These are only used during a single subframe, to pass intra-subframe
	 * parameters between funcional blocks. */
	float d[4][40];		/* short term residual */
	float d__[40];		/* short term residual estimate */
	float e[40];		/* long term residual */
};

int gsm_encode_init(struct gsm_encstate *);
int gsm_encode(struct gsm_encstate *, struct gsm_frame *, short []);
int gsm_decode_init(struct gsm_decstate *);
int gsm_decode(struct gsm_decstate *, struct gsm_frame *, short []);
int gsm_vocode_init(struct gsm_vocstate *state);
int gsm_vocode(struct gsm_vocstate *, short sout[], short sin[], short csin[]);

#endif /* GSM_H */

