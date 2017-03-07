#pragma once

#include <stdint.h>
#include <stddef.h>

#if defined __linux || defined __APPLE__

#include <vector>
#include <string>

#if defined __APPLE__ && !defined MAC_OS_X_VERSION_10_7

#include <map>

#else

#include <unordered_map>

#endif

#endif

class SymbolFinder
{
public:
	SymbolFinder( );

	void *FindPattern( const void *handle, const uint8_t *pattern, size_t len );
	void *FindPatternFromBinary( const char *name, const uint8_t *pattern, size_t len );
	void *FindSymbol( const void *handle, const char *symbol );
	void *FindSymbolFromBinary( const char *name, const char *symbol );

	// data can be a symbol name (if appended by @) or a pattern
	void *Resolve( const void *handle, const char *data, size_t len = 0 );
	void *ResolveOnBinary( const char *name, const char *data, size_t len = 0 );

private:
	bool GetLibraryInfo( const void *handle, struct DynLibInfo &info );

#if defined __linux || defined __APPLE__

#if defined __APPLE__ && !defined MAC_OS_X_VERSION_10_7

    typedef std::map<std::string, void *> SymbolTable;

#else

    typedef std::unordered_map<std::string, void *> SymbolTable;

#endif

	struct LibSymbolTable
	{
		LibSymbolTable( uintptr_t base ) :
			table( ), lib_base( base ), last_pos( 0 )
		{ }

		SymbolTable table;
		uintptr_t lib_base;
		uint32_t last_pos;
	};

	std::vector<LibSymbolTable> symbolTables;
	struct dyld_all_image_infos *m_ImageList;
	long m_OSXMajor;
	long m_OSXMinor;

#endif

};
