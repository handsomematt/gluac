[![Build Status](https://travis-ci.org/HandsomeMatt/gluac.svg?branch=master)](https://travis-ci.org/HandsomeMatt/gluac)

## Synopsis

A simple utility for compiling Lua into bytecode using Garry's Mod Lua compiler.

Based on `https://github.com/lua/luac/blob/master/luac.c` and `https://code.google.com/p/lcdhost/`

## Prerequisites

Depends on `Bootil` which is included as a git submodule and `premake5` to build project files.

`lua_dyn.c` & `lua_dyn.h` are generated using `tools/lua_dyn_export_h.lua` with `luajit-2.0.3` (the current luajit shipped with gmod)

Running the compiler requires the libraries from GMod to be in the directory with them:

- Windows: `lua_shared.dll` and `tier0.dll`
- Linux: `lua_shared.so`, `libsteam.so` and `libtier0.so`

## Building From Source

First run: `git submodule update --init --recursive` to grab Bootil.

* **Windows**: Generate your project files using `premake5 vs2015` and build using `project/gluac.sln`
* **Linux**: Run `premake5 gmake && make`

## License

[The MIT License (MIT) - Copyright (c) 2017 Matt Stevens](LICENSE)