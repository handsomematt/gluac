-- this file is originally from https://code.google.com/p/lcdhost/

-- NOTE: This file is modified from the standard distribution.
-- It now only generates a .c file with the needed table in it.

-- Configuration part
local files = { "lua.h", "lauxlib.h", "lualib.h" }
local dstHfile = "lua_dyn.h"
local dstCfile = "lua_dyn.c"
local headerdef = "LUA_DYNAMIC_H"
local loaderfct_indll = false
local conf_defines = -- configuration #defines to avoid including all luaconf.h
{
	LUA_NUMBER = 'double',
	LUA_INTEGER = 'ptrdiff_t',
	LUA_IDSIZE = 60,
	LUAL_BUFFERSIZE = 'BUFSIZ',
 	LUA_COMPAT_OPENLIB = '',
-- 	LUA_COMPAT_GETN = '',
}
-- End of configuration

local outfile = assert(io.open(dstHfile, "wt"))
local function out(...)
	outfile:write(string.format(...))
end

local function suffix(name)
	if name:match("^lua_") then name = name:sub(5) 
	elseif name:match("^luaL_") then name = name:sub(6).."L" 
	else name = name:sub(4)
	end
	name = name:sub(1,1):upper() .. name:sub(2)
	return name
end

out('/* Lua language header file, designed for dynamic DLL loading. \n')
out('   This file is not part of the standard distribution, but was\n')
out('   generated from original files %s */\n\n', table.concat(files, ', '))
out('#ifndef %s\n#define %s\n\n#ifdef __cplusplus\nextern "C" {\n#endif\n', headerdef, headerdef)
out('#ifndef _WIN32\n#define __cdecl\n#define GetProcAddress dlsym\n#endif\n\n')

for k,v in pairs(conf_defines) do
	out('#define %s %s\n', k, v)
end
local fcts = {}
for _,file in ipairs(files) do
	local f = assert(io.open(file, "rt"))
	local data = f:read("*all")
	data = data:gsub("/%*%*%*.-%*%*%*/", function(a) out('\n%s\n',a) return "" end) -- exports copyright
	data = data:gsub("/%*.-%*/", "") -- remove comments to simplify parsing
	data = data:gsub('#include ".-\n', "") -- remove local includes
	data = data:gsub("^%s*#ifndef [%w_]+\n#define [%w_]+\n(.*)#endif%s*", "%1") -- removes header sentinel
	data = data:gsub("#if defined%(LUA_COMPAT_GETN%)\n(.-)#else\n(.-)#endif", 
		function (doif, doelse)
			if  conf_defines.LUA_COMPAT_GETN then return doif else return doelse end
		end)

	data = data:gsub("#if(.-)#endif", "")
	data = data:gsub("\n\n+", "\n\n") -- remove extra new lines 
	if conf_defines.LUA_COMPAT_OPENLIB then
		data = data:gsub("luaI_openlib", "luaL_openlib") -- remapped function: care!
	end
	-- Some function declarations lacks the extra parenthesis around their name
	data = data:gsub("(LUA_API%s[^%(%)%;]*)(lua_%w+)%s*(%([^%(%)]+%);)", "%1 (%2) %3")
	-- Detect external functions and transforms them into typedefs
	data = data:gsub("LUA%w*_API%s+([%w_%*%s]-)%s*%(([%w_]+)%)%s*%((.-)%);", 
		function (type,name,arg) 
			fcts[#fcts+1] = name
			return string.format("typedef %s (__cdecl *%s_t) (%s);", type, name, arg)
		end)
	out("%s", data)
	f:close()
end
table.sort(fcts)
for _,f in pairs(fcts) do
	out('#define %-23s LUA_PREFIX %s\n', f, suffix(f))
end
out '\ntypedef struct lua_All_functions\n{\n'
for _,f in pairs(fcts) do
	out('  %-23s %s;\n', f.."_t", suffix(f))
end
-- Write footer (verbatim string)
out '} lua_All_functions;\n\n'
if not loaderfct_indll then
out 'extern int luaL_loadfunctions(void* hModule, lua_All_functions* functions, size_t size_struct);\n\n'
else
out [[
typedef int (__cdecl *luaL_loadfunctions_t)(void* hModule, lua_All_functions* functions, size_t size_struct);

#define LUA_LOAD_FUNCTIONS(hmodule,lua_struct) do { luaL_loadfunctions_t fct; \
		if(!(hmodule && (fct = (luaL_loadfunctions_t) GetProcAddress(hmodule, "luaL_loadfunctions")) && \
			fct((void*)hmodule, (lua_struct), sizeof(lua_All_functions)))) \
			memset((lua_struct), 0, sizeof(lua_All_functions)); } while(0)
]]
end
out '#ifdef __cplusplus\n}\n#endif\n\n#endif\n'
outfile:close()

outfile = assert(io.open(dstCfile, "wt"))
-- Write C loader function (verbatim)
out ([[
static const char* const LuaFunctionNames[] = 
{
	"%s"
};
]], table.concat(fcts, '",\n\t"'), "")
outfile:close()