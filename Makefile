all: seq2wav

debug: seq2wav_d

static: libseq2wav.a

DLL = so
ifeq ($(OS),Windows_NT)
	DLL = dll
endif

shared: libseq2wav.$(DLL)

seq2wav_d: FORCE src/Makefile
	$(MAKE) -C src ../seq2wav_d

seq2wav: FORCE src/Makefile
	$(MAKE) -C src ../seq2wav

libseq2wav.a: FORCE src/Makefile $(patsubst src/%.h, include/%.h, $(wildcard src/*.h src/*/*.h))
	$(MAKE) -C src ../libseq2wav.a

libseq2wav.$(DLL): FORCE src/Makefile $(patsubst src/%.h, include/%.h, $(wildcard src/*.h src/*/*.h))
	$(MAKE) -C src ../libseq2wav.$(DLL)

include/%.h: src/%.h
	@mkdir -p $(dir $@)
	cp $< $@

clean: FORCE
	$(MAKE) -C src clean
	rm -f include/*.h include/*/*.h
	rmdir include/*
	rmdir include

FORCE:
