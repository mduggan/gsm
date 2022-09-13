#ifndef _GSM_PACK_H_
#define _GSM_PACK_H_

void gsm_pack(struct gsm_frame *, unsigned char *);
void gsm_unpack(struct gsm_frame *, unsigned char *);

#endif /* _GSM_PACK_H_ */
