#ifndef _GSM_RPE_H_
#define _GSM_RPE_H_

void gsm_rpe_enc(struct gsm_frame *, struct gsm_signals *, int);
void gsm_rpe_dec(struct gsm_frame *, struct gsm_signals *, int, float []);

#endif /* GSM_RPE_H */
