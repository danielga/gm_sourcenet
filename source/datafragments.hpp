#pragma once

#include "main.hpp"
#include "net.hpp"

typedef CNetChan::dataFragments_t dataFragments_t;

namespace dataFragments
{
	void Push(
		GarrysMod::Lua::ILuaBase *LUA,
		dataFragments_t *datafrag = nullptr,
		CNetChan *netchan = nullptr
	);

	dataFragments_t *Get(
		GarrysMod::Lua::ILuaBase *LUA,
		int32_t index,
		CNetChan **netchan = nullptr
	);

	void Initialize( GarrysMod::Lua::ILuaBase *LUA );

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );
}
