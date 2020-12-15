all: seq2wav

debug: seq2wav_d

static: libseq2wav.a

DLL = so
ifeq ($(OS),Windows_NT)
	DLL = dll
endif

INCLUDES = $(patsubst src/%.h, include/%.h, $(wildcard src/*.h src/*/*.h))

includes: $(INCLUDES)

build/Makefile.d: $(wildcard src/*.cpp src/*/*.cpp src/*.h src/*/*.h) Makefile src/Makefile
	$(MAKE) -C src ../build/Makefile.d

seq2wav seq2wav_d: src/Makefile build/Makefile.d
	$(MAKE) -C src ../$@

libseq2wav.a libseq2wav_d.$(DLL): src/Makefile build/Makefile.d $(INCLUDES)
	$(MAKE) -C src ../$@

include/%.h: src/%.h
	@mkdir -p $(dir $@)
	cp $< $@

clean: FORCE
	-rm -f build/*.o build/*.d build/*/*.o build/Makefile.d
	-rm -f seq2wav seq2wav_d libseq2wav.a libseq2wav_d.a
	-rm -f include/*.h include/*/*.h
	-rmdir include/*
	-rmdir include

FORCE:
