../seq2wav: $(patsubst %.cpp, ../build/%.o, $(wildcard *.cpp */*.cpp))
	g++ -std=gnu++14 -o $@ $^

../seq2wav_d: $(patsubst %.cpp, ../build/%_d.o, $(wildcard *.cpp */*.cpp))
	g++ -std=gnu++14 -o $@ $^ -lasan

../build/%.o: %.cpp $(wildcard *.h */*.h) Makefile
	@mkdir -p $(dir $@)
	g++ -std=gnu++14 -O3 -flto -c -o $@ -Wno-multichar $<

../build/%_d.o: %.cpp $(wildcard *.h */*.h) Makefile
	@mkdir -p $(dir $@)
	g++ -std=gnu++14 -ggdb -fsanitize=address -c -o $@ -Wno-multichar $<

clean: FORCE
	rm -f ../build/*.o ../build/*/*.o
	rm -f ../seq2wav ../seq2wav_d

FORCE:
