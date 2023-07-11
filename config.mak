# Note: Don't use -march=native for code intended for distribution. -msse2 is probably good.
CXXFLAGS := -fPIC -std=gnu++17 -Wno-multichar -I../src -I../seq2wav/include -msse2 -I. -Isrc/ $(PLUGIN_CXXFLAGS)

DLL := so
EXE :=
BUILDPATH := build
ifeq ($(CROSS),msvc)
	DLL := dll
	EXE := .exe
ifneq ($(OS),Windows_NT)
ifndef WINE
	WINE := wine
endif
endif
plugins: winamp foobar
else ifeq ($(CROSS),mingw)
	CC := i686-w64-mingw32-gcc-posix
	CXX := i686-w64-mingw32-g++-posix
	OS := Windows_NT
endif
ifeq ($(OS),Windows_NT)
	DLL := dll
	EXE := .exe
	CXXFLAGS := $(CXXFLAGS) -static-libgcc -static-libstdc++
	LDFLAGS := $(LDFLAGS) -static-libgcc -static-libstdc++ -static -lpthread
	BUILDPATH := build_win
ifneq ($(CROSS),msvc)
plugins: winamp
endif
endif

CXXFLAGS_R := $(CXXFLAGS) -O3
CXXFLAGS_D := $(CXXFLAGS) -Og -ggdb3
LDFLAGS := $(LDFLAGS) -L../seq2wav/$(BUILDPATH) -L../$(BUILDPATH) $(PLUGIN_LDFLAGS)
LDFLAGS_R := $(LDFLAGS) -lseq2wav
LDFLAGS_D := $(LDFLAGS) -lseq2wav_d
