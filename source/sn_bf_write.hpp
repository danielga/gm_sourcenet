#pragma once

#include <main.hpp>

class bf_write;

namespace sn_bf_write
{
	bf_write **Push(
		GarrysMod::Lua::ILuaBase *LUA,
		bf_write *writer = nullptr,
		int32_t bufref = LUA_NOREF
	);

	bf_write *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index, int32_t *bufref = nullptr );

	void Initialize( GarrysMod::Lua::ILuaBase *LUA );

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );
}
