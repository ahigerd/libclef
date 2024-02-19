first: test

include config.mak

test: clef_test$(EXE)

debug: clef_test_d$(EXE)

static: $(BUILDPATH)/libclef.a

$(BUILDPATH)/Makefile.d: $(wildcard src/*.cpp src/*/*.cpp src/*.h src/*/*.h) Makefile src/Makefile
	$(MAKE) -C src ../$(BUILDPATH)/Makefile.d

clef_test$(EXE) clef_test_d$(EXE): src/Makefile $(BUILDPATH)/Makefile.d
	$(MAKE) -C src ../$@

$(BUILDPATH)/libclef.a $(BUILDPATH)/libclef_d.a: src/Makefile $(BUILDPATH)/Makefile.d $(INCLUDES)
	$(MAKE) -C src ../$@

clean: FORCE
	-rm -f $(BUILDPATH)/*.o $(BUILDPATH)/*.d $(BUILDPATH)/*/*.o $(BUILDPATH)/Makefile.d
	-rm -f clef_test$(EXE) clef_test_d$(EXE) $(BUILDPATH)/libclef.a $(BUILDPATH)/libclef_d.a

FORCE:
