libclef
=======

libclef is a generic library for building decoders and media player plugins for sequenced audio formats.
It was formerly known as "seq2wav".

Currently, there is support for the following targets:

* Command-line (Windows, POSIX platforms)
* Audacious player plugin (Windows, macOS, X11 platforms)
* Winamp player plugin (Windows)
* Foobar2000 player plugin (Windows)
* CLAP instrument plugin (Windows, POSIX platforms)

Building
--------
libclef is meant to be statically linked as a component in another project. See the "Template
project" section below for more information.

On POSIX systems or when building under MinGW, libclef itself has no dependencies beyond a
C++17-compliant compiler and GNU Make. To build libclef using Microsoft Visual Studio, ATL support
and the Visual Studio command-line tools must be installed.

To build an Audacious plugin, the Audacious development headers must be present. These may be found in
the `audacious-dev` package on Debian-based distros.) The libclef build system uses `pkg-config` to
locate the required files. Audacious plugins cannot currently be built using Microsoft Visual C++.

To build a Foobar2000 plugin, the [Foobar2000 SDK](https://www.foobar2000.org/SDK) is required.
libclef has been tested with the 2020-07-28 version of the Foobar2000 SDK. The SDK should be
extracted into the `plugins/` folder of the project that uses libclef.

No additional dependencies are required for building a Winamp plugin.

Build scripts are included for cross-compiling Windows binaries on Linux using MinGW or using Wine
to run the Visual Studio command-line tools. (Installing the Visual Studio command-line tools on
Linux is left as an exercise to the reader.)

Further information for building libclef-based tools and plugins can be found in the [template
project documentation](template/README.md). If built standalone, libclef produces a very basic
command-line test program that creates a file named `sample.wav`. The test program does not
support building with Microsoft Visual C++.

Template project
----------------
A template project for creating tools or plugins based on libclef may be found in the `template/`
folder. Copy all of the files in `template/` to a new folder and place the libclef source tree
in a `libclef/` folder inside it. (If using `git`, you may use
`git submodule add https://github.com/ahigerd/libclef` to clone libclef into the appropriate
location and track it as a submodule.)

Modify `config.mak` in the new project to set `PLUGIN_NAME` appropriately, and change the filenames
in `buildvs.cmd` to refer to the same plugin name instead of `template`. Additionally, fill in
`plugins/clefplugin.cpp` with the necessary metadata and code for the player plugins.

Avoid nesting subdirectories more than one level inside `src/`, as the build scripts will not find
source files in any deeper levels.

Further information can be found in the [template project documentation](template/README.md).

License
-------
libclef is copyright (c) 2020-2023 Adam Higerd and distributed under the terms of the
[MIT license](LICENSE.md).

[CLAP](https://cleveraudio.org/) is an open-source audio plugin format. The
[CLAP SDK](https://github.com/free-audio/clap) is copyright (c) 2021 Alexander BIQUE
and distributed under the terms of the [MIT license](LICENSE.CLAP).
