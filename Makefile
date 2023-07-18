include config.mak

test: seq2wav_test$(EXE)

debug: seq2wav_test_d$(EXE)

static: $(BUILDPATH)/libseq2wav.a

$(BUILDPATH)/Makefile.d: $(wildcard src/*.cpp src/*/*.cpp src/*.h src/*/*.h) Makefile src/Makefile
	$(MAKE) -C src ../$(BUILDPATH)/Makefile.d

seq2wav_test$(EXE) seq2wav_test_d$(EXE): src/Makefile $(BUILDPATH)/Makefile.d
	$(MAKE) -C src ../$@

$(BUILDPATH)/libseq2wav.a $(BUILDPATH)/libseq2wav_d.a: src/Makefile $(BUILDPATH)/Makefile.d $(INCLUDES)
	$(MAKE) -C src ../$@

include/%.h: src/%.h
	@mkdir -p $(dir $@)
	cp $< $@

clean: FORCE
	-rm -f $(BUILDPATH)/*.o $(BUILDPATH)/*.d $(BUILDPATH)/*/*.o $(BUILDPATH)/Makefile.d
	-rm -f seq2wav_test$(EXE) seq2wav_test_d$(EXE) $(BUILDPATH)/libseq2wav.a $(BUILDPATH)/libseq2wav_d.a
	-rm -f include/*.h include/*/*.h
	-rmdir include/*
	-rmdir include

FORCE:
