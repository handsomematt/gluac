[![Build Status](https://travis-ci.org/gmodstore/gluac.svg?branch=master)](https://travis-ci.org/gmodstore/gluac)

## Synopsis

> Tool for compiling Lua into bytecode using Garry's Mod's `lua_shared` dynamic library.
> Useful for obfuscation of source code you don't want readable.

Based on `https://github.com/lua/luac/blob/master/luac.c` and `https://code.google.com/p/lcdhost/`

## Prerequisites

Depends on [Bootil](https://github.com/garrynewman/bootil) and [danielga/scanning](https://github.com/danielga/scanning)
which are both included as git submodules.
And [premake5](https://premake.github.io/) is used to create project files.

`lua_dyn.c` & `lua_dyn.h` are generated using `tools/lua_dyn_export_h.lua` alongside the
source code for the target version of LuaJIT used in GMod (currently `luajit-2.0.3`)

Running the program requires the libraries from GMod to be in the directory with them:

- Windows: `lua_shared.dll` and `tier0.dll`
- Linux: `lua_shared.so`, `libsteam.so` and `libtier0.so`

## Building From Source

First run: `git submodule update --init --recursive` to grab `Bootil` and `danielga/scanning`.

* **Windows**: Generate your project files using `premake5 vs2015` and build using `project/gluac.sln`
* **Linux**: Run `premake5 gmake && make`

## License

[The MIT License (MIT) - Copyright (c) 2017-2018 Matt Stevens](LICENSE)