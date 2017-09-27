#pragma once

#include <main.hpp>

class bf_read;

namespace sn_bf_read
{
	bf_read **Push(
		GarrysMod::Lua::ILuaBase *LUA,
		bf_read *reader = nullptr,
		int32_t bufref = LUA_NOREF
	);

	bf_read *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index, int32_t *bufref = nullptr );

	void Initialize( GarrysMod::Lua::ILuaBase *LUA );

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );
}
