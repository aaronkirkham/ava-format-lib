## Avalanche Formats Library
A C++ library to parse avalanche's proprietary data formats. This library works with raw file buffers and provides an easy to use API with which you can write your own custom tools for Avalanche titles.

### How to Use
Link AvaFormatLib.lib, and include AvaFormatLib.h.

### Build Instructions
To compile the code you will need **Visual Studio 2017 or later** (or a C++17 compiler)
- Clone this repository
- Run `git submodule update --init` to clone dependencies
- Run `configure.sh` inside a terminal
- Build `projects/AvaFormatLib.sln` in Visual Studio
