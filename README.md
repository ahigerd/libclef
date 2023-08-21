seq2wav
=======

seq2wav is a generic library for building decoders and media player plugins for sequenced audio formats.

Currently, there is support for the following targets:

* Command-line (Windows, POSIX platforms)
* Audacious player plugin (Windows, macOS, X11 platforms)
* Winamp player plugin (Windows)
* Foobar2000 player plugin (Windows)
* CLAP instrument plugin (Windows, POSIX platforms)

Building
--------
seq2wav is meant to be statically linked as a component in another project. See the "Template
project" section below for more information.

On POSIX systems or when building under MinGW, seq2wav itself has no dependencies beyond a
C++17-compliant compiler and GNU Make. To build seq2wav using Microsoft Visual Studio, ATL support
and the Visual Studio command-line tools must be installed.

To build an Audacious plugin, the Audacious development headers must be present. These may be found in
the `audacious-dev` package on Debian-based distros.) The seq2wav build system uses `pkg-config` to
locate the required files. Audacious plugins cannot currently be built using Microsoft Visual C++.

To build a Foobar2000 plugin, the [Foobar2000 SDK](https://www.foobar2000.org/SDK) is required.
seq2wav has been tested with the 2020-07-28 version of the Foobar2000 SDK. The SDK should be
extracted into the `plugins/` folder of the project that uses seq2wav.

No additional dependencies are required for building a Winamp plugin.

Build scripts are included for cross-compiling Windows binaries on Linux using MinGW or using Wine
to run the Visual Studio command-line tools. (Installing the Visual Studio command-line tools on
Linux is left as an exercise to the reader.)

Further information for building seq2wav-based tools and plugins can be found in the [template
project documentation](template/README.md). If built standalone, seq2wav produces a very basic
command-line test program that creates a file named `sample.wav`. The test program does not
support building with Microsoft Visual C++.

Template project
----------------
A template project for creating tools or plugins based on seq2wav may be found in the `template/`
folder. Copy all of the files in `template/` to a new folder and place the seq2wav source tree
in a `seq2wav/` folder inside it. (If using `git`, you may use
`git submodule add https://bitbucket.org/ahigerd/seq2wav` to clone seq2wav into the appropriate
location and track it as a submodule.)

Modify `config.mak` in the new project to set `PLUGIN_NAME` appropriately, and change the filenames
in `buildvs.cmd` to refer to the same plugin name instead of `template`. Additionally, fill in
`plugins/s2wplugin.cpp` with the necessary metadata and code for the player plugins.

Avoid nesting subdirectories more than one level inside `src/`, as the build scripts will not find
source files in any deeper levels.

Further information can be found in the [template project documentation](template/README.md).

License
-------
seq2wav is copyright (c) 2020-2023 Adam Higerd and distributed
under the terms of the [MIT license](LICENSE.md).

[CLAP](https://cleveraudio.org/) is an open-source audio plugin format. The
[CLAP SDK](https://github.com/free-audio/clap) is copyright (c) 2021 Alexander BIQUE
and distributed under the terms of the [MIT license](LICENSE.CLAP).
