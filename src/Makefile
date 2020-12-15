# Don't use -march=native for code intended for distribution
CXXFLAGS = -fPIC -std=gnu++14 -Wno-multichar -I. -march=native

SOURCES = $(filter-out main.cpp, $(wildcard *.cpp */*.cpp))

DLL = so
ifeq ($(OS),Windows_NT)
	DLL = dll
endif

../seq2wav: ../build/main.o ../libseq2wav.a
	$(CXX) -std=gnu++14 -o $@ $^

../seq2wav_d: ../build/main_d.o ../libseq2wav_d.a
	$(CXX) -std=gnu++14 -o $@ $^ -lasan

../libseq2wav.a: $(patsubst %.cpp, ../build/%.o, $(SOURCES)) ../build/Makefile.d
	rm -f $@
	gcc-ar -rc $@ $^

../libseq2wav_d.$(DLL): $(patsubst %.cpp, ../build/%_d.o, $(SOURCES)) ../build/Makefile.d
	$(CXX) -shared -o $@ $(CXXFLAGS) $(patsubst %.cpp, ../build/%_d.o, $(SOURCES))

../build/%.o: %.cpp Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -O3 -c -o $@ $<

../build/%_d.o: %.cpp Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -ggdb -fsanitize=address -c -o $@ $<

../build/Makefile.d: main.cpp $(SOURCES) $(wildcard *.h */*.h) Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MT ../build/main.o -MM -MF - main.cpp > $@
	$(CXX) $(CXXFLAGS) -MT ../build/main_d.o -MM -MF - main.cpp >> $@
	$(foreach src, $(SOURCES), $(CXX) $(CXXFLAGS) -MT $(patsubst %.cpp, ../build/%.o, $(src)) -MM -MF - $(src) >> $@;)
	$(foreach src, $(SOURCES), $(CXX) $(CXXFLAGS) -MT $(patsubst %.cpp, ../build/%_d.o, $(src)) -MM -MF - $(src) >> $@;)

include ../build/Makefile.d

FORCE:
