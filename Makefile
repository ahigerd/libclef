all: seq2wav

debug: seq2wav_d

seq2wav_d: FORCE src/Makefile
	$(MAKE) -C src ../seq2wav_d

seq2wav: FORCE src/Makefile
	$(MAKE) -C src ../seq2wav

clean: FORCE
	$(MAKE) -C src clean

FORCE:
