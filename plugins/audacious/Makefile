DLL = .so
ifeq ($(OS),Windows_NT)
	DLL = .dll
endif

PLUGIN = TODO

CXXFLAGS = -std=gnu++14 -fPIC -Wno-multichar -I../../src -I../../seq2wav/include -march=native $(shell pkg-config --cflags audacious) -DPLUGIN_NAME=\"$(PLUGIN)\"
CXXFLAGS_R = $(CXXFLAGS) -O3
CXXFLAGS_D = $(CXXFLAGS) -ggdb -fsanitize=address
LDFLAGS = -L../../seq2wav $(shell pkg-config --libs audacious)
LDFLAGS_R = $(LDFLAGS) -lseq2wav
LDFLAGS_D = $(LDFLAGS) -lseq2wav_d -lasan

SOURCES = $(wildcard *.cpp */*.cpp)
BASE_OBJS = $(filter-out %_d.o,$(wildcard ../../build/*.o ../../build/*/*.o))
OBJS_R = $(patsubst %.cpp, build/%.o, $(SOURCES)) $(BASE_OBJS)
OBJS_D = $(patsubst %.cpp, build/%_d.o, $(SOURCES)) $(BASE_OBJS)

../../aud_$(PLUGIN)$(DLL): ../../seq2wav/libseq2wav.a $(OBJS_R)
	$(CXX) -shared -o $@ $(OBJS_R) $(LDFLAGS_R)

../../aud_$(PLUGIN)_d$(DLL): ../../seq2wav/libseq2wav_d.a $(OBJS_D)
	$(CXX) -shared -o $@ $(OBJS_D) $(LDFLAGS_D)

build/%.o: %.cpp Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS_R) -c -o $@ $<

build/%_d.o: %.cpp Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS_D) -c -o $@ $<

build/Makefile.d: $(SOURCES) $(wildcard *.h */*.h) Makefile
	@mkdir -p $(dir $@)
	-rm -f $@
	$(foreach src, $(SOURCES), $(CXX) $(CXXFLAGS) -MT $(patsubst %.cpp, build/%.o, $(src)) -MM -MF - $(src) >> $@;)
	$(foreach src, $(SOURCES), $(CXX) $(CXXFLAGS) -MT $(patsubst %.cpp, build/%_d.o, $(src)) -MM -MF - $(src) >> $@;)

include build/Makefile.d

FORCE:
