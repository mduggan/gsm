#include <math.h>
#include "gsm.h"
#include "gsm_consts.h"
#include "gsm_sta.h"

/* Note for our SIGN, we don't need the special case of 0 */
#define SIGN(x) (x>=0?1:-1)

/* Section 3.1.8 of standard */
void gsm_sta_decode(signed char lar[], float lar__[])
{
	/* Decode coded LARs */
	int i;
	for(i = 0; i < 8; i++) {
		lar__[i] = ( lar[i] - LAR_B[i] ) / LAR_A[i];
	}
}

/* Section 3.1.9 of standard
 * Calculate interpolated lars. */
static void sta_interp(float intlar[], float lar[], float oldlar[], int n)
{
	/* Calculate interpolated LAR values from old and new */
	int i;
	
	//printf("intlars:\n");
	for (i = 0; i < 8; i++)
	{
		intlar[i] = LAR_INTERP_COEFF1[n] * oldlar[i] + 
			LAR_INTERP_COEFF2[n] * lar[i];
		//printf("%f\t", intlar[i]);
	}
	//printf("\n");
}

/* Section 3.1.10 of standard */
static void sta_reflect(float lar[], float ref[])
{
	/* Calculate quantised reflection coefficients from interpolated LAR */
	int i; 
	float abslar;

	for(i = 0; i < 8; i++) {
		abslar = fabs(lar[i]);
		if (abslar >= 0.675) {
			if (abslar < 1.225) {
				ref[i] = (lar[i]*0.5) + 0.3375*SIGN(lar[i]);
			} else {
				ref[i] = (lar[i]*0.125) + 0.796875*SIGN(lar[i]);
			}
			if (ref[i] > 1) ref[i] = 1;
			if (ref[i] < -1) ref[i] = -1;
		} else {
			ref[i] = lar[i];
		}
	}
}

/* short term analysis (used in encoding) */
void gsm_sta(struct gsm_frame *g, struct gsm_signals *s, float lar__prev[],
		float u[], float sig[])
{
	float lar__[8];
	int i, k, n;
	float di, uk, tmp;
	float ref[8];
	float intlar[8];
	float d[160];

	gsm_sta_decode(g->lars, lar__);
	
	/* Section 3.1.11 of standard */
	/* calculate short-term residual signal (d) from s and ref */
	for (n = 0; n < 4; n++) {	
		sta_interp(intlar, lar__, lar__prev, n);
		sta_reflect(intlar, ref);
		
		for(k = LAR_INTERP_BOUND[n]; k < LAR_INTERP_BOUND[n+1]; k++) {
			uk = di = sig[k];
			for(i = 0; i < 8; i++) {
				tmp = u[i] + ref[i] * di;
				di += ref[i] * u[i];
				u[i] = uk;
				uk = tmp;
			}
			d[k] = di;
		}
	}

	/* Move lar__ to lar__prev (memcpy?) */
	for (i = 0; i < 8; i++) {
		lar__prev[i] = lar__[i];
	}
	
	/* split d into subframes */
	for (i = 0; i < 4; i++) {
		for (k = 0; k < 40; k++) {
			s->d[i][k] = d[(i * 40) + k];
		}
	}	
}

/* short term synthesis (used in decoding) */
void gsm_sts(float lar__[], float d_[], float lar__prev[], float v[],
		float sig[])
{
	float si;
	float ref[8];
	float intlar[8];
	int i, k, n;
	
	for (n = 0; n < 4; n++) {
		sta_interp(intlar, lar__, lar__prev, n);
		sta_reflect(intlar, ref);
		
		for(k = LAR_INTERP_BOUND[n]; k < LAR_INTERP_BOUND[n+1]; k++) {
			si = d_[k] - ref[7] * v[7];
			for (i = 6; i >=0; i--) {
				si = si - ref[i] * v[i];
				v[i+1] = v[i] + ref[i] * si ;
			}
			sig[k] = v[0] = si;
		}
	}

	/* Move lar__ to lar__prev (memcpy?) */
	for (i = 0; i < 8; i++) {
		lar__prev[i] = lar__[i];
	}
}

	
	
