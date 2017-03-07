#include "Bootil/Bootil.h"

using namespace Bootil;

#include "lua_dyn.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#define LUA_PREFIX LuaFunctions.
lua_All_functions LuaFunctions;

String::List g_FileList;

static void PrintUsage()
{
	Output::Msg("USAGE: gluac [filenames] [-?] [-o] [-p]\n");
	Output::Msg("-?: Prints this message\n");
	Output::Msg("-o: Output filename (default is lua.luac)\n");
	Output::Msg("-p: Parse only, doesn't dump bytecode\n");

	exit(1);
}

static int lua_main(lua_State* L)
{
	BOOTIL_FOREACH_CONST(file, g_FileList, String::List)
	{
		const char* filename = (*file).c_str();

		if (luaL_loadfile(L, filename) != 0)
		{
			Output::Warning("%s\n", lua_tostring(L, -1));
			continue;
		}

		if (CommandLine::HasSwitch("-p"))
			continue;
	}

	return 0;
}

bool LoadLuaShared()
{
	#ifdef _WIN32
		HMODULE module = LoadLibrary("lua_shared.dll");
	#else
		void* module = dlopen("lua_shared.so", RTLD_LAZY);
	#endif

	return luaL_loadfunctions(module, &LuaFunctions, sizeof(LuaFunctions));
}

int main(int argc, char* argv[])
{
	Debug::SuppressPopups( true );
	CommandLine::Set(argc, argv);

	if (CommandLine::GetArgCount() <= 0 || CommandLine::HasSwitch("-?"))
		PrintUsage();

	for (int i = 0; i < CommandLine::GetArgCount(); i++)
		if (String::Test::StartsWith(CommandLine::GetArg(i), "-"))
			break;
		else
			g_FileList.push_back(CommandLine::GetArg(i));

	if (g_FileList.empty())
		PrintUsage();
	
	if (!LoadLuaShared())
	{
		Output::Warning("Error loading lua_shared.\n");
		exit(1);
	}
	
	lua_State* L;

	L = lua_open();
	luaL_openlibs(L);
	if (L == NULL)
	{
		Output::Warning("Cannot create state: not enough memory.\n");
		exit(1);
	}

	if (lua_cpcall(L, lua_main, NULL) != 0)
		Output::Warning("%s\n", lua_tostring(L, -1));

	lua_close(L);
	return 0;

}