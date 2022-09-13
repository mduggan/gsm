#ifndef _GSM_STA_H_
#define _GSM_STA_H_

void gsm_sta(struct gsm_frame *, struct gsm_signals *, float [], float [],
		float []);
void gsm_sts(float lar__[], float [], float [], float [], float []);

void gsm_sta_decode(signed char lar[], float lar__[]);

#endif /* GSM_STA_H */
