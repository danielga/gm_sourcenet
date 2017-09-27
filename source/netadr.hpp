#pragma once

#include <main.hpp>

typedef struct netadr_s netadr_t;

namespace netadr
{
	void Push( GarrysMod::Lua::ILuaBase *LUA, const netadr_t &netadr );

	void Initialize( GarrysMod::Lua::ILuaBase *LUA );

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );
}
