## Avalanche Formats Library
A C++ library to parse Avalanche's proprietary data formats. This library works with raw buffers and provides an easy to use API with which you can write your own custom tools for Avalanche titles.

The goal is to support as many Avalanche games as possible, however only Just Cause 3 and 4 have been thoroughly tested. If you are having issues with a game that is not listed above, feel free to create an issue or PR.

### How to Use
Download the [latest version](https://github.com/aaronkirkham/ava-format-lib/releases/latest), link AvaFormatLib.lib, and include AvaFormatLib.h in your project.

### Contributions
Contributions are welcome. Clang Format is used to keep code consistent so try to make sure any PR is formatted correctly.

### Build Instructions
To compile the code you will need **Visual Studio 2017 or later** (or a C++17 compiler)
- Clone this repository
- Run `git submodule update --init` to clone dependencies
- Run `configure.ps1` with PowerShell or run premake manually
- Build `projects/AvaFormatLib.sln` in Visual Studio
