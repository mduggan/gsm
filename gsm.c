#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "gsm.h"
#include "gsm_consts.h"
#include "gsm_pack.h"
#include "gsm_lpc.h"
#include "gsm_ltp.h"
#include "gsm_lta.h"
#include "gsm_sta.h"
#include "gsm_rpe.h"

#define FUDGE_FACTOR 4.0

/* Shifts ints to be 13 bit, then converts to float */
static void int_to_float_shifted(short in[], float out[], int N) 
{
	int i;
	for(i=0; i < N; i++)
	{
		/* Note: more precision than GSM */
#ifdef HIGH_QUAL
		out[i] = (float)in[i] / 8.0;
#else
		out[i] = (float)(in[i] >> 3);
#endif
		out[i] = out[i] * FUDGE_FACTOR;
	}
}

/* Shifts ints to be 13 bit, then converts to float */
static void float_to_shifted_int(float in[], short out[], int N) 
{
	int i;
	for(i=0; i < N; i++)
	{
		/* Fudge factor */
		in[i] = in[i] / FUDGE_FACTOR;
		/* Note: more precision than GSM */
		if (in[i] > 4095.0) in[i] = 4095.0;
		if (in[i] < -4096.0) in[i] = -4096.0; 
#ifdef HIGH_QUAL
		out[i] = (short)(in[i] * 8.0);
#else
		out[i] = (short)(in[i]) << 3;
#endif
	}
}

static void gsm_shift(float d_[], int off, int size) {
	int i;
	for(i=0; i < size; i++) {
		d_[i] = d_[i+off];
	}
}

static void preproc(float in[], float out[], float *pre_so, float *pre_sof, int N) 
{
	int i;
	float sof;
	/* PRE: length(in) == length(out) == N */
	/*      pre_so, pre_sof != NULL */

	/* first iteration uses special "previous" values */
	sof = in[0] - *pre_so + (alpha * *pre_sof);
	out[0] = sof - beta * *pre_sof;
	*pre_sof = sof;

	for (i = 1; i < N; i++) {
		sof = in[i] - in[i-1] + (alpha * *pre_sof);
		out[i] = sof - beta * *pre_sof;
		*pre_sof = sof;
	}

	*pre_so = out[N-1];
}

int gsm_encode_init(struct gsm_encstate *state)
{
	int i;
	
	state->so = 0.0;
	state->sof = 0.0;
	for (i = 0; i < 8; i++)
		state->lar__prev[i] = 0.0;
	for (i = 0; i < 120; i++)
		state->d_[i] = 0.0;
	for (i = 0; i < 8; i++)
		state->u[i] = 0.0;

	return 0;
}

int gsm_encode(struct gsm_encstate *state, struct gsm_frame *frame, short sin[])
{
	float s1[160];
	float s[160];
	struct gsm_signals sig;
	int j, k;
	
	/* convert to 13 bit values */
	int_to_float_shifted(sin, s1, 160);
	/* Preprocessing */
	preproc(s1, s, &(state->so), &(state->sof), 160);
	/* Linear Predictive Coding */
	gsm_lpc(frame, s);
	/* Short Term Analysis */
	gsm_sta(frame, &sig, state->lar__prev, state->u, s);

	/* Run the same procedure on each sub-frame */
	for (j = 0; j < 4; j++) {
		/* Long Term Predictor */
		gsm_ltp(frame, &sig, j, state->d_);
		/* Long Term Analysis */
		gsm_lta(frame, &sig, j, state->d_);
		/* Calculate long term residual e[k] Eq 3.17*/
		for (k = 0; k < 40; k++)
			sig.e[k] = sig.d[j][k] - sig.d__[k];
		/* Move the 2 subframes of d' back */
		gsm_shift(state->d_, 40, 80);
		/* Residual Pulse Encoding (also LTS) */
		gsm_rpe_enc(frame, &sig, j);
		gsm_rpe_dec(frame, &sig, j, (state->d_) + 80);
	}

	return 0;
}

int gsm_decode_init(struct gsm_decstate *state)
{
	int i;
	state->nr_ = 40;
	for(i = 0; i < 160; i++) {
		state->d_[i] = 0;
	}
	for(i = 0; i < 8; i++) {
		state->lar__prev[i] = 0;
	}
	for(i = 0; i < 8; i++) {
		state->v[i] = 0;
	}
	state->sro = 0;

	return 0;
}

static void postproc(float in[], float out[], float *pre_sro, int N) 
{
	int i;
	/* PRE: length(in) == length(out) == N */
	/*      pre_sro != NULL */

	/* first iteration uses special "previous" values */
	out[0] = in[0] + (beta * *pre_sro);

	for (i = 1; i < N; i++) {
		out[i] = in[i] + (beta * out[i - 1]);
	}

	*pre_sro = out[N - 1];
}

int gsm_decode(struct gsm_decstate *state, struct gsm_frame *frame, 
		short sout[])
{
	float s1[160], sin[160], lar__[8];
	struct gsm_signals sig;
	int j;
	
	for (j = 0; j < 4; j++) {
		/* Long Term Analysis */
		gsm_lta(frame, &sig, j, (state->d_)+40);
		/* Move the last 3 subframes of dr' back */
		gsm_shift(state->d_, 40, 120);
		/* RPE decoder (Also LTS) */
		gsm_rpe_dec(frame, &sig, j, (state->d_) + 120); 
	}

	/* short term sythesis */
	gsm_sta_decode(frame->lars, lar__);
	gsm_sts(lar__, state->d_, state->lar__prev, state->v, sin);

	postproc(sin, s1, &(state->sro), 160);
	float_to_shifted_int(s1, sout, 160);

	return 0;
}

int gsm_vocode_init(struct gsm_vocstate *state)
{
	int i;

	state->so = 0.0;
	state->sof = 0.0;
	state->cso = 0.0;
	state->csof = 0.0;
	
	for(i = 0; i < 8; i++) {
		state->lar__prev[i] = 0;
	}
	for(i = 0; i < 8; i++) {
		state->v[i] = 0;
	}
	state->sro = 0;

	return 0;
}

int gsm_vocode(struct gsm_vocstate *state, short sout[], short sin[], short csin[])
{
	float s1[160];
	float s[160];
	float cs1[160];
	float cs[160];
	float lars[8];
	int i;
	
	/* convert to 13 bit values */
	int_to_float_shifted(sin, s1, 160);
	int_to_float_shifted(csin, cs1, 160);
	/* Preprocessing */
	preproc(s1, s, &(state->so), &(state->sof), 160);
	preproc(cs1, cs, &(state->cso), &(state->csof), 160);
	
	/* Get filter coefficients */
	gsm_get_lars(lars, s);

	/* Apply filter to carrier signal */
	gsm_sts(lars, cs, state->lar__prev, state->v, s);

	/* Postprocess */
	for (i=0; i<160; i++)
		s[i] = s[i] / 3;
	postproc(s, s1, &(state->sro), 160);
	float_to_shifted_int(s1, sout, 160);

	return 0;
}


