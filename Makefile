include config.mak

all: seq2wav_test$(EXE)

debug: seq2wav_test_d$(EXE)

static: $(BUILDPATH)/libseq2wav.a

INCLUDES = $(patsubst src/%.h, include/%.h, $(wildcard src/*.h src/*/*.h))

includes: $(INCLUDES)

build/Makefile.d: $(wildcard src/*.cpp src/*/*.cpp src/*.h src/*/*.h) Makefile src/Makefile
	$(MAKE) -C src ../build/Makefile.d

seq2wav_test$(EXE) seq2wav_test_d$(EXE): src/Makefile build/Makefile.d
	$(MAKE) -C src ../$@

$(BUILDPATH)/libseq2wav.a $(BUILDPATH)/libseq2wav_d.a: src/Makefile build/Makefile.d $(INCLUDES)
	$(MAKE) -C src ../$@

include/%.h: src/%.h
	@mkdir -p $(dir $@)
	cp $< $@

clean: FORCE
	-rm -f build/*.o build/*.d build/*/*.o build/Makefile.d
	-rm -f seq2wav seq2wav_d $(BUILDPATH)/libseq2wav.a $(BUILDPATH)/libseq2wav_d.a
	-rm -f include/*.h include/*/*.h
	-rmdir include/*
	-rmdir include

FORCE:
