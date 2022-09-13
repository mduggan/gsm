#include "gsm.h"
#include "gsm_consts.h"
#include "gsm_ltp.h"

/* Note for our SIGN, we don't need the special case of 0 */
#define SIGN(x) (x>=0?1:-1)
#define POW2(x) ((x)*(x))

void gsm_ltp(struct gsm_frame *f, struct gsm_signals *sig, int j, float d_[])
{
	int n, lambda, i;
	float rmax, r, s, b;

	for (lambda = 40; lambda < 121; lambda++) {
		r = 0.0;
		for (i = 0; i < 40; i++) {
			r += sig->d[j][i] * d_[120 + i - lambda];
		}
		if (lambda == 40 || r > rmax) {
			n = lambda;
			rmax = r;
		}
	}

	s = 0.0;
	for (i = 0; i < 40; i++) {
		s += POW2(d_[120 + i - n]);
		//printf("d_[%d] = %f\n", 120+i-n, state->d_[120+i-n]);
	}

	b = rmax / s;

	//printf("rmax = %f\t", rmax);
	//printf("s = %f\t", s);
	//printf("b = %f\n", b);
	
	i = 0;
	while (i < 3 && b > DLB[i])
		i++;

	f->N[j] = n;
	f->b[j] = i;
}

