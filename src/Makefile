include ../config.mak

CXXFLAGS := $(patsubst -I../libclef/src,-I../src,$(CXXFLAGS))
CXXFLAGS_R := $(patsubst -I../libclef/src,-I../src,$(CXXFLAGS_R))
CXXFLAGS_D := $(patsubst -I../libclef/src,-I../src,$(CXXFLAGS_D))

SOURCES = $(filter-out main.cpp, $(wildcard *.cpp */*.cpp))

../clef_test$(EXE): ../$(BUILDPATH)/main.o ../$(BUILDPATH)/libclef.a
	$(CXX) -o $@ $^

../clef_test_d$(EXE): ../$(BUILDPATH)/main_d.o ../$(BUILDPATH)/libclef_d.$(DLL)
	$(CXX) -o $@ $^

../$(BUILDPATH)/libclef.a: $(patsubst %.cpp, ../$(BUILDPATH)/%.o, $(SOURCES)) | ../$(BUILDPATH)/Makefile.d
	rm -f $@
	gcc-ar -rc $@ $^

../$(BUILDPATH)/libclef_d.a: $(patsubst %.cpp, ../$(BUILDPATH)/%_d.o, $(SOURCES)) | ../$(BUILDPATH)/Makefile.d
	rm -f $@
	gcc-ar -rc $@ $^

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
