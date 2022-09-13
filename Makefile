COMOBJECTS=gsm.o gsm_consts.o gsm_lpc.o gsm_sta.o gsm_ltp.o gsm_lta.o gsm_rpe.o gsm_pack.o wav.o
DECOBJECTS=gsmdec.o $(COMOBJECTS)
ENCOBJECTS=gsmenc.o $(COMOBJECTS)
VOCOBJECTS=gsmvoc.o $(COMOBJECTS)
#LIBS=-lm
CFLAGS=-W -Wall -O2

all: gsmenc gsmdec unpacktest

gsmdec: $(DECOBJECTS)
	gcc $(CFLAGS) -o $@ $(LIBS) $(DECOBJECTS)

gsmenc: $(ENCOBJECTS)
	gcc $(CFLAGS) -o $@ $(LIBS) $(ENCOBJECTS)

gsmvoc: $(VOCOBJECTS)
	gcc $(CFLAGS) -o $@ $(LIBS) $(VOCOBJECTS)

unpacktest: unpacktest.o gsm_pack.o
	gcc $(CFLAGS) -o $@ $(LIBS) unpacktest.o gsm_pack.o

gsm.o: gsm.c gsm.h gsm_consts.h gsm_sta.h gsm_lpc.h gsm_ltp.h gsm_rpe.h gsm_lta.h gsm_pack.h
	gcc $(CFLAGS) -c -o $@ $<

gsm_pack.o: gsm_pack.c gsm_pack.h
	gcc $(CFLAGS) -c -o $@ $<

gsmenc.o: gsmenc.c gsm.h wav.h
	gcc $(CFLAGS) -c -o $@ $<

gsm_consts.o: gsm_consts.c gsm_consts.h
	gcc $(CFLAGS) -c -o $@ $<

gsm_lpc.o: gsm_lpc.c gsm_lpc.h gsm.h gsm_consts.h
	gcc $(CFLAGS) -c -o $@ $<

gsm_sta.o: gsm_sta.c gsm_sta.h gsm.h gsm_consts.h
	gcc $(CFLAGS) -c -o $@ $<

gsm_ltp.o: gsm_ltp.c gsm_ltp.h gsm.h gsm_consts.h
	gcc $(CFLAGS) -c -o $@ $<

gsm_lta.o: gsm_lta.c gsm_lta.h gsm.h gsm_consts.h
	gcc $(CFLAGS) -c -o $@ $<

gsm_rpe.o: gsm_rpe.c gsm_rpe.h gsm.h gsm_consts.h
	gcc $(CFLAGS) -c -o $@ $<

wav.o: wav.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(ENCOBJECTS) $(DECOBJECTS) gsmenc gsmdec unpacktest unpacktest.o *~ core
