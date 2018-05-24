#include "lua_dyn.h"
#include "lua_jit.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <symbolfinder.hpp>

#define LUA_PREFIX LuaFunctions.
lua_All_functions LuaFunctions;

char* g_sFilename = nullptr;
bool g_bParseOnly = false;
bool g_bStripDebug = true;

typedef int(__cdecl *lj_bcwrite_t) (lua_State *L, void *gcproto, lua_Writer, void *data, int strip);
lj_bcwrite_t lj_bcwrite = NULL;

#ifdef _WIN32
static const char *LuaJIT_bcwrite_sym = "\x83\xEC\x24\x8B\x4C\x24\x2C\x8B\x54\x24\x30\x8B\x44\x24\x28\x89";
static const size_t LuaJIT_bcwrite_symlen = 16;
#else
static const char *LuaJIT_bcwrite_sym = "@lj_bcwrite";
static const size_t LuaJIT_bcwrite_symlen = 0;
#endif

static void print_usage()
{
	printf("USAGE: gluac [filename] [-o] [-p] [-s]\n");
	printf("-o: Output filename (default: stdout)\n");
	printf("-p: Parse only, doesn't dump bytecode\n");
	printf("-s: Strip debug information\n");

	exit(1);
}

typedef struct {
	size_t *len;
	char **data;
} wdata;

int write_dump(lua_State *L, const void* p, size_t sz, void* ud)
{
	wdata *wd = (wdata *)ud;

	char *newData = (char *)realloc(*(wd->data), (*(wd->len)) + sz);

	if (newData)
	{
		memcpy(newData + (*(wd->len)), p, sz);
		*(wd->data) = newData;
		*(wd->len) += sz;
	}
	else {
		free(newData);
		return 1;
	}

	return 0;
}

int lua_bcwrite(lua_State *L, lua_Writer writer, void *data, bool strip)
{
	cTValue *o = L->top - 1;
	return lj_bcwrite(L, (GCproto *)(mref((&gcval(o)->fn)->l.pc, char) - sizeof(GCproto)), writer, data, strip);
}

bool load_lua_shared()
{
	#ifdef _WIN32
	HMODULE module = LoadLibrary("lua_shared.dll");

	if (module == nullptr) {
		fprintf(stderr, "could not find lua_shared\n");
		return false;
	}
	#else
	void* module = dlopen("lua_shared_srv.so", RTLD_LAZY);

	if (module == nullptr) {
		fprintf(stderr, "%s\n", dlerror());
		return false;
	}

	#endif

	SymbolFinder symfinder;
	lj_bcwrite = reinterpret_cast<lj_bcwrite_t>(symfinder.Resolve(module, LuaJIT_bcwrite_sym, LuaJIT_bcwrite_symlen));
	
	if (lj_bcwrite == nullptr) {
		fprintf(stderr, "failed to resolve lj_bcwrite\n");
		return false;
	}

	return luaL_loadfunctions(module, &LuaFunctions, sizeof(LuaFunctions));
}

static int lua_main(lua_State* L)
{
	// load our Lua file as a chunk on the stack (if filename is NULL it loads from stdin)
	if (luaL_loadfile(L, g_sFilename) != 0) {
		fprintf(stderr, "luaL_loadfile: %s\n", lua_tostring(L, -1));
		return 0;
	}

	// return early if we only want parsing
	if (g_bParseOnly) {
		return 0;
	}

	char* bytecode = 0L;
	size_t len = 0;
	wdata wd = { &len, &bytecode };

	if (lua_bcwrite(L, write_dump, &wd, g_bStripDebug))
	{
		fprintf(stderr, "failed to dump bytecode\n");
		return 0;
	}

	// output bytecode to stdout
	fwrite(bytecode, len, 1, stdout);
	fflush(stdout);
	
	return 0;
}

int main(int argc, char* argv[])
{
	if (!load_lua_shared()) {
		fprintf(stderr, "error loading lua_shared\n");
		return 1;
	}

	lua_State* L = lua_open();
	luaL_openlibs(L);
	if (L == nullptr) {
		fprintf(stderr, "cannot create lua state: not enough memory.\n");
		return 1;
	}

	if (lua_cpcall(L, lua_main, nullptr) != 0) {
		fprintf(stderr, "lua_cpcall: %s\n", lua_tostring(L, -1));
		lua_close(L);
		return 1;
	}

	lua_close(L);
	return 0;
}