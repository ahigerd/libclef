# Note: Don't use -march=native for code intended for distribution. -msse2 is probably good.
CXXFLAGS := -fPIC -std=gnu++17 -Wno-multichar -I../src -I../libclef/src -msse2 -I. -Isrc/ $(PLUGIN_CXXFLAGS)
QMAKE = qmake

DLL := so
EXE :=
BUILDPATH := build
EXPECT_XSPEC :=
CLAP_LDFLAGS :=
ifeq ($(CROSS),msvc)
	DLL := dll
	EXE := .exe
ifneq ($(OS),Windows_NT)
	WINE = wine
endif
	EXPECT_XSPEC := win32-msvc
plugins: winamp foobar
else ifeq ($(CROSS),mingw-posix)
	CC := i686-w64-mingw32-gcc-posix
	CXX := i686-w64-mingw32-g++-posix
	OS := Windows_NT
	EXPECT_XSPEC := win32-g++
else ifeq ($(CROSS),mingw64-posix)
	CC := x86_64-w64-mingw32-gcc-posix
	CXX := x86_64-w64-mingw32-g++-posix
	OS := Windows_NT
	EXPECT_XSPEC := win32-g++
else ifeq ($(CROSS),mingw)
	CC := i686-w64-mingw32-gcc
	CXX := i686-w64-mingw32-g++
	OS := Windows_NT
	EXPECT_XSPEC := win32-g++
else ifeq ($(CROSS),mingw64)
	CC := x86_64-w64-mingw32-gcc
	CXX := x86_64-w64-mingw32-g++
	OS := Windows_NT
	EXPECT_XSPEC := win32-g++
endif
ifeq ($(OS),Windows_NT)
	DLL := dll
	EXE := .exe
	CXXFLAGS := $(CXXFLAGS) -static-libgcc -static-libstdc++
	LDFLAGS := $(LDFLAGS) -static-libgcc -static-libstdc++ -static -lpthread
	BUILDPATH := build_win
	CLAP_LDFLAGS := -luuid
ifneq ($(CROSS),msvc)
	EXPECT_XSPEC := win32-g++
plugins: winamp
else
	EXPECT_XSPEC := win32-msvc
endif
else ifneq ($(CROSS),)
$(error Invalid CROSS. Supported values: mingw mingw64 mingw-posix mingw64-posix msvc)
endif

CXXFLAGS_R := $(CXXFLAGS) -O3 -ffast-math
ifeq ($(OS),Windows_NT)
CXXFLAGS_D := $(CXXFLAGS) -Og -ffast-math -gstabs
else
CXXFLAGS_D := $(CXXFLAGS) -Og -ffast-math -ggdb3
endif
LDFLAGS := $(LDFLAGS) -L../libclef/$(BUILDPATH) -L../$(BUILDPATH) $(PLUGIN_LDFLAGS)
LDFLAGS_R := $(LDFLAGS) -lclef
LDFLAGS_D := $(LDFLAGS) -lclef_d

validate: FORCE
ifeq ($(PLUGIN_NAME),sample)
	@echo Please update PLUGIN_NAME in config.mak.
	@exit 1
endif

libclef/$(BUILDPATH)/libclef.a libclef/$(BUILDPATH)/libclef_d.a: validate

validategui: FORCE
	@QMAKE=$(QMAKE) MAKE=$(MAKE) sh libclef/validate-qmake.sh $(EXPECT_XSPEC)

gui/Makefile gui/Makefile.debug: validategui

FORCE:
