include ../config.mak

SOURCES = $(filter-out main.cpp, $(wildcard *.cpp */*.cpp))

../seq2wav_test$(EXE): ../$(BUILDPATH)/main.o ../$(BUILDPATH)/libseq2wav.a
	$(CXX) -o $@ $^

../seq2wav_test_d$(EXE): ../$(BUILDPATH)/main_d.o ../$(BUILDPATH)/libseq2wav_d.$(DLL)
	$(CXX) -o $@ $^

../$(BUILDPATH)/libseq2wav.a: $(patsubst %.cpp, ../$(BUILDPATH)/%.o, $(SOURCES)) ../$(BUILDPATH)/Makefile.d
	rm -f $@
	gcc-ar -rc $@ $^

../$(BUILDPATH)/libseq2wav_d.a: $(patsubst %.cpp, ../$(BUILDPATH)/%_d.o, $(SOURCES)) ../$(BUILDPATH)/Makefile.d
	rm -f $@
	gcc-ar -rc $@ $^
	#$(CXX) -o $@ $(CXXFLAGS) $(patsubst %.cpp, ../$(BUILDPATH)/%_d.o, $(SOURCES))

../$(BUILDPATH)/%.o: %.cpp Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS_R) -c -o $@ $<

../$(BUILDPATH)/%_d.o: %.cpp Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS_D) -c -o $@ $<

../$(BUILDPATH)/Makefile.d: main.cpp $(SOURCES) $(wildcard *.h */*.h) Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MT ../$(BUILDPATH)/main.o -MM -MF - main.cpp > $@
	$(CXX) $(CXXFLAGS) -MT ../$(BUILDPATH)/main_d.o -MM -MF - main.cpp >> $@
	$(foreach src, $(SOURCES), $(CXX) $(CXXFLAGS) -MT $(patsubst %.cpp, ../$(BUILDPATH)/%.o, $(src)) -MM -MF - $(src) >> $@;)
	$(foreach src, $(SOURCES), $(CXX) $(CXXFLAGS) -MT $(patsubst %.cpp, ../$(BUILDPATH)/%_d.o, $(src)) -MM -MF - $(src) >> $@;)

include ../$(BUILDPATH)/Makefile.d

FORCE:
