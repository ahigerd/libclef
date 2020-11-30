DLL = so
ifeq ($(OS),Windows_NT)
	DLL = dll
endif

../seq2wav: ../build/main.o ../libseq2wav.a
	$(CXX) -std=gnu++14 -o $@ $^

../seq2wav_d: $(patsubst %.cpp, ../build/%_d.o, $(wildcard *.cpp */*.cpp))
	$(CXX) -std=gnu++14 -o $@ $^ -lasan

../libseq2wav.a: $(patsubst %.cpp, ../build/%.o, $(filter-out main.cpp, $(wildcard *.cpp */*.cpp)))
	@rm -f $@
	gcc-ar -rc $@ $^

../libseq2wav.$(DLL): $(patsubst %.cpp, ../build/%.o, $(filter-out main.cpp, $(wildcard *.cpp */*.cpp)))
	$(CXX) -shared -o $@ $^

../build/%.o: %.cpp $(wildcard *.h */*.h) Makefile
	@mkdir -p $(dir $@)
	$(CXX) -std=gnu++14 -O3 -flto -ffat-lto-objects -c -o $@ -Wno-multichar $<

../build/%_d.o: %.cpp $(wildcard *.h */*.h) Makefile
	@mkdir -p $(dir $@)
	$(CXX) -std=gnu++14 -ggdb -fsanitize=address -c -o $@ -Wno-multichar $<

clean: FORCE
	rm -f ../build/*.o ../build/*/*.o
	rm -f ../seq2wav ../seq2wav_d

FORCE:
