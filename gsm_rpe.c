#include <math.h>

#include "gsm.h"
#include "gsm_consts.h"
#include "gsm_rpe.h"

/* Part 3.1.18 of the standard */
static void rpe_weight(float x[], float e[]) {
	/* PRE: x and e are at least 40 floats long */
	int i, k;
	float temp[50];

	/* make an array that's basically e with 0s either side */
	for(i=0; i < 5; i++) {
		temp[i] = 0;
	}
	for(i=5; i < 45; i++) {
		temp[i] = e[i-5];
	}
	for(i=45; i < 50; i++) {
		temp[i] = 0;
	}

	/* perform the convolution */
	for(k=0; k < 40; k++) {
		x[k] = 0;
		for(i = 0; i < 10; i++) {
			/* the standard is a bit crazy here.. 
			 * access to temp has to be offset 
			 * by -10 to make it right - but that's the same
			 * as not offsetting it at all because weighting_h is 
			 * symmetrical! */
			x[k] += (weighting_h[i]/8192.0) * temp[k + i];
		}
	}


}

/* Part 3.1.19 of the standard */
static void rpe_decimate(float xm[], float x[]) {
	int i, m;

	/* interleave values */
	for (m = 0; m < 4; m++) {
		for (i = 0; i < 13; i++) {
			xm[(m * 13) + i] = x[3*i + m];
		}
	}
}

/* Part 3.1.19 of the standard */
static void rpe_optimum(unsigned char *Mc, float xm[]) {
	int i,m;
	float temp[4];

	*Mc = 0;
	/* Find the one with the maximum energy */
	for (m = 0; m < 4; m++) {
		for (i = 0; i < 13; i++) {
			temp[m] += (xm[(m*13)+i] * xm[(m*13)+i]);
		}
		if (temp[m] > temp[(int)(*Mc)]) *Mc = m;
	}
}

/* Part 3.1.20 of the standard */
static void rpe_quant(unsigned char Mc, float xm[], unsigned char *xmaxc, unsigned char xMc[]) {
	/* PRE: 0 <= Mc <= 3 
	 *      xm is at least 40 floats long
	 *      xmaxc points to a valid char
	 *      xMc is at least 13 chars long 
	 */
	int i, temp, x_i;
	float xmax = 0;
	float x_[13];

	/* find the maximum of the absolute values */
	for (i = 0; i < 40; i++) {
		if (fabs(xm[i]) > xmax) xmax = fabs(xm[i]);
	}

	/* Logarithmically Quantise it (table 3.5) */
	*xmaxc = 0;
	while ((xmax > xmax_lookup[(int)*xmaxc]) && (*xmaxc < 63))
		(*xmaxc)++;

	/* Now xmaxc is in the range 0-63 */
	/* Normalise the optimum by dividing by the decoded version */
	for (i = 0; i < 13; i++) {
		x_[i] = xm[i + 13*Mc]/xmax_lookup[(int)*xmaxc];

		/* uniformly quantise according to table 3.6 */
		x_i = x_[i] * 32768; /* Convert to integer for this bit */

		xMc[i] = 0;

		temp = -24577; /* top of first range (32768-24577) */
		while ((x_i > temp) && (xMc[i] < 7))
		{
			xMc[i]++;
			temp += 8192;
		}

	}

}

/* Section 3.1.21 of the specification */
static void rpe_invq(unsigned char xMc[], unsigned char xmaxc, float x_M[]) {
	int i;

	/* De-normalise */
	for(i = 0; i < 13; i++) {
		/* decode to find x(bar)M * 2^15 according to table 3.6 */
		x_M[i] = -28672 + (xMc[i] * 8192);
		/* denormalise (invert equation 3.23) */
		x_M[i] *= xmax_lookup[(int)xmaxc];
		/* bring back down to [-32768 <= x'M <= 32768] sensible length */
		x_M[i] /= 32768;

	}
}

/* Section 3.1.22 of the specification */
static void rpe_gridpos(unsigned char Mc, float x_M[], float e_[]) {
	int i;

	/* clear the e' array */
	for(i = 0; i < 40; i++) {
		e_[i] = 0;
	}
	/* insert the values from x'M offset by Mc */
	for(i = 0; i < 13; i++) {
		e_[(3*i)+Mc] = x_M[i];
		//printf("x_M=%f\n", x_M[i]);
	}

}

/* Implements RPE grid selection and coding */
void gsm_rpe_enc(struct gsm_frame *g, struct gsm_signals *sig, int sf) {
	float xm[52], x[40];

	rpe_weight(x, sig->e);
	rpe_decimate(xm, x);
	rpe_optimum(&(g->Mc[sf]), xm);
	rpe_quant(g->Mc[sf], xm, &(g->xmaxc[sf]), g->xMc[sf]);
}

/* Implement RPE grid decoding and positioning */
void gsm_rpe_dec(struct gsm_frame *g, struct gsm_signals *sig, int sf, 
		float d_[])
{
	float x_M[13], e_[40];
	int i;

	rpe_invq(g->xMc[sf], g->xmaxc[sf], x_M) ;
	rpe_gridpos(g->Mc[sf], x_M, e_);

	/* 
	 * Section 3.1.17: "Long Term Synthesis" 
	 * 
	 * (equation 3.19) 
	 */
	for(i = 0; i < 40; i++) {
		d_[i] = e_[i] + sig->d__[i];
		//printf ("d_[%d] = e_[%d] + d__[%d] = %f + %f = %f\n", i, i, i, e_[i], sig->d__[i], d_[i]);
	}
}
