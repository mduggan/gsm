#include "gsm.h"
#include "gsm_consts.h"
#include "gsm_lta.h"

/* Section 3.1.16 of the spec. */
void gsm_lta(struct gsm_frame *g, struct gsm_signals *sig, int j, float d_[])
{
	int k;

	for (k = 0; k < 40; k++) {
		/* calculate new short term residual estimate */
		sig->d__[k] = QLB[(int)(g->b[j])] * d_[120 + k - g->N[j] ];
	}

}
