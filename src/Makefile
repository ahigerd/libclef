LIB = lib
DLL = so
ifeq ($(OS),Windows_NT)
	DLL = dll
	LIB =
endif

CXXFLAGS = -std=gnu++14 -Wno-multichar

SOURCES = $(filter-out main.cpp, $(wildcard *.cpp */*.cpp))

../seq2wav: ../build/main.o ../libseq2wav.a
	$(CXX) -std=gnu++14 -o $@ $^

../seq2wav_d: ../build/main_d.o $(patsubst %.cpp, ../build/%_d.o, $(SOURCES))
	$(CXX) -std=gnu++14 -o $@ $^ -lasan

../libseq2wav.a: $(patsubst %.cpp, ../build/%.o, $(SOURCES))
	@rm -f $@
	gcc-ar -rc $@ $^

../$(LIB)seq2wav.$(DLL): $(patsubst %.cpp, ../build/%.o, $(SOURCES))
	$(CXX) -shared -o $@ $^

../build/%.o: %.cpp Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -O3 -flto -ffat-lto-objects -c -o $@ $<

../build/%_d.o: %.cpp Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -ggdb -fsanitize=address -c -o $@ $<

../build/Makefile.d: main.cpp $(SOURCES)
	@mkdir -p $(dir $@)
	-rm -f $@
	$(foreach src, $^, $(CXX) $(CXXFLAGS) -MT $@ -MM -MP -MF - $(src) >> $@;)

include ../build/Makefile.d

FORCE:
