#pragma once

#include <main.hpp>

class CNetChan;

namespace NetChannel
{
	bool IsValid( CNetChan *netchan );

	void Push( GarrysMod::Lua::ILuaBase *LUA, CNetChan *netchan );

	CNetChan *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index );

	void Destroy( GarrysMod::Lua::ILuaBase *LUA, CNetChan *netchan );

	void Initialize( GarrysMod::Lua::ILuaBase *LUA );

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );
}
