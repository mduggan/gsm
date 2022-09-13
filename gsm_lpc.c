#include <math.h>
#include <stdio.h>
#include "gsm.h"
#include "gsm_consts.h"
#include "gsm_lpc.h"

#define sgn(x) ((x)<0?-1:1)

#define NINT(x) ((int)((x)+sgn(x)*0.5))

/* 0 <= k <= 8 */
static float acf(float s[], int k)
{
	int i;
	float sum = 0;

	for (i = k; i < 160; i++) {
		sum += s[i] * s[i-k];
	}

	return sum;
}

/* Schur recursion ! */
static void schur(float s[], float r[])
{
	int i;
	float p[9];
	float k[9];
	int n, m;
	
	for (i = 0; i < 8; i++)
		r[i] = 0;
	
	p[0] = acf(s, 0);
	if (p[0] == 0)
		return;	
	for (i = 1; i < 8; i++) {
		k[9-i] = p[i] = acf(s, i);
	}	
	p[8] = acf(s, 8);
	
	for (n = 0; n < 8; n++)
	{
		if (p[0] < fabs(p[1])) {
			break;
		}
		
		r[n] = - p[1] / p[0];
		
		if (n == 7)
			break;
		
		p[0] += p[1] * r[n];
		
		for (m = 1; m < 8-n; m++) {
			p[m] = p[1+m] + r[n] * k[9-m];
			k[9-m] = k[9-m] + r[n] * p[1+m];
		}
	}
}

void gsm_get_lars(float lars[], float s[])
{
	int i;
	float abs_ri;
	float r[8];

	/* Perform Schur recursion to get reflection coefficients */
	schur(s, r);

	for (i = 0; i < 8; i++) {
		/* printf("rc: %f\t", r[i]); */
		/* Convert to log-area-ratios (eq 3.4 of 3GPP TS 06.10) */
		abs_ri = fabs(r[i]);
		if (abs_ri < 0.675)
			lars[i] = r[i];
		else if (abs_ri < 0.950)
			lars[i] = 2 * r[i] - sgn(r[i]) * 0.675;
		else
			lars[i] = 8 * r[i] - sgn(r[i]) * 6.375;
		
		/* printf("lar: %f\t", lars[i]); */
	}
}

/* Perform the linear predictive coding step.
 * Requires: s[160]
 * Changes: frame->lars
 */
void gsm_lpc(struct gsm_frame *frame, float s[])
{
	int i;
	float lars[8];

	gsm_get_lars(lars, s);
	
	for (i = 0; i < 8; i++) {
		/* Quantise */
		frame->lars[i] = NINT(LAR_A[i] * lars[i] + LAR_B[i]);
		if (frame->lars[i] < LAR_MIN[i])
			frame->lars[i] = LAR_MIN[i];
		if (frame->lars[i] > LAR_MAX[i])
			frame->lars[i] = LAR_MAX[i];

		/* printf("larc: %d\n", frame->lars[i]); */
	}
}

