#pragma once

#include <main.hpp>

typedef void *FileHandle_t;

namespace FileHandle
{
	extern int32_t metatype;
	extern const char *metaname;

	void Push( GarrysMod::Lua::ILuaBase *LUA, FileHandle_t file );

	FileHandle_t Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index );

	void Initialize( GarrysMod::Lua::ILuaBase *LUA );

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );
}
