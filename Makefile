all: seq2wav

debug: seq2wav_d

static: libseq2wav.a

LIB = lib
DLL = so
ifeq ($(OS),Windows_NT)
	DLL = dll
	LIB =
endif

INCLUDES = $(patsubst src/%.h, include/%.h, $(wildcard src/*.h src/*/*.h))

shared: libseq2wav.$(DLL)

build/Makefile.d: $(wildcard src/*.cpp src/*/*.cpp)
	@mkdir -p build
	-rm -f $@
	cd src; $(foreach src, $^, $(CXX) $(CXXFLAGS) -MT ../$@ -MM -MP -MF - $(patsubst src/%, %, $(src)) >> ../$@;)

seq2wav seq2wav_d: FORCE src/Makefile build/Makefile.d
	$(MAKE) -C src ../$@

libseq2wav.a libseq2wav_d.a $(LIB)seq2wav.$(DLL) $(LIB)seq2wav_d.$(DLL): FORCE src/Makefile build/Makefile.d $(INCLUDES)
	$(MAKE) -C src ../$@

include/%.h: src/%.h
	@mkdir -p $(dir $@)
	cp $< $@

clean: FORCE
	$(MAKE) -C src clean
	-rm -f build/Makefile.d
	-rm -f include/*.h include/*/*.h
	-rmdir include/*
	-rmdir include

FORCE:
