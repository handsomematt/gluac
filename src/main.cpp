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

struct MainUserdata {
	int fileCount;
	char** fileList;
};

static int lua_main(lua_State* L)
{
	struct MainUserdata* s = (struct MainUserdata*)lua_touserdata(L, 1);
	lua_pop(L, 1);
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

int main( int argc, char** argv )
{
	Debug::SuppressPopups( true );
	CommandLine::Set(argc, argv);

	if (!LoadLuaShared())
		Output::Error("Error loading lua_shared.");
	
	lua_State* L;

	L = lua_open();
	luaL_openlibs(L);
	if (L == NULL)
		Output::Error("Cannot create state: not enough memory.");

	// construct user data to pass to the Lua state
	struct MainUserdata userdata;

	if (lua_cpcall(L, lua_main, &userdata) != 0)
		Output::Warning((lua_tostring(L, -1)));

	lua_close(L);
	return 0;

}