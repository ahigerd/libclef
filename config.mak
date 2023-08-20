# Note: Don't use -march=native for code intended for distribution. -msse2 is probably good.
CXXFLAGS := -fPIC -std=gnu++17 -Wno-multichar -I../src -I../seq2wav/src -msse2 -I. -Isrc/ $(PLUGIN_CXXFLAGS)
QMAKE = qmake

DLL := so
EXE :=
BUILDPATH := build
EXPECT_XSPEC :=
ifeq ($(CROSS),msvc)
	DLL := dll
	EXE := .exe
ifneq ($(OS),Windows_NT)
	WINE = wine
endif
	EXPECT_XSPEC := win32-msvc
plugins: winamp foobar
else ifeq ($(CROSS),mingw)
	CC := i686-w64-mingw32-gcc-posix
	CXX := i686-w64-mingw32-g++-posix
	OS := Windows_NT
	EXPECT_XSPEC := win32-g++
else ifeq ($(CROSS),mingw64)
	CC := x86_64-w64-mingw32-gcc-posix
	CXX := x86_64-w64-mingw32-g++-posix
	OS := Windows_NT
	EXPECT_XSPEC := win32-g++
endif
ifeq ($(OS),Windows_NT)
	DLL := dll
	EXE := .exe
	CXXFLAGS := $(CXXFLAGS) -static-libgcc -static-libstdc++
	LDFLAGS := $(LDFLAGS) -static-libgcc -static-libstdc++ -static -lpthread
	BUILDPATH := build_win
ifneq ($(CROSS),msvc)
	EXPECT_XSPEC := win32-g++
plugins: winamp
else
	EXPECT_XSPEC := win32-msvc
endif
endif

CXXFLAGS_R := $(CXXFLAGS) -O3 -ffast-math
ifeq ($(OS),Windows_NT)
CXXFLAGS_D := $(CXXFLAGS) -Og -ffast-math -gstabs
else
CXXFLAGS_D := $(CXXFLAGS) -Og -ffast-math -ggdb3
endif
LDFLAGS := $(LDFLAGS) -L../seq2wav/$(BUILDPATH) -L../$(BUILDPATH) $(PLUGIN_LDFLAGS)
LDFLAGS_R := $(LDFLAGS) -lseq2wav
LDFLAGS_D := $(LDFLAGS) -lseq2wav_d

validate: FORCE
ifeq ($(PLUGIN_NAME),sample)
	@echo Please update PLUGIN_NAME in config.mak.
	@exit 1
endif

seq2wav/$(BUILDPATH)/libseq2wav.a seq2wav/$(BUILDPATH)/libseq2wav_d.a: validate

define validate_xspec =
ifeq ($(XSPEC),)
	@echo Unable to execute qmake binary: $(QMAKE)
	@exit 1
endif
endef

define validate_spec =
ifneq ("$(EXPECT_XSPEC)","$(XSPEC)")
	@echo
	@echo Incorrect qmake binary detected: $(QMAKE)
	@echo - qmake target platform: "'$(XSPEC)'"
	@echo - expected: "'$(EXPECT_XSPEC)'"
	@echo
	@echo Please specify the path to the correct qmake binary:
	@echo "  $(MAKE) gui QMAKE=/path/to/qmake"
	@echo
	@exit 1
endif
endef

validategui: FORCE
	$(eval XSPEC := $(shell $(QMAKE) -query QMAKE_XSPEC))
	$(eval $(call validate_xspec))
ifeq ($(EXPECT_XSPEC),)
	$(eval EXPECT_XSPEC := $(shell $(QMAKE) -query QMAKE_SPEC))
endif
	$(eval $(call validate_spec))

gui/Makefile gui/Makefile.debug: validategui

FORCE:
