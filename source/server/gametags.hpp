#pragma once

#include "main.hpp"

namespace GameTags
{
	void PreInitialize( GarrysMod::Lua::ILuaBase *LUA );

	void Initialize( GarrysMod::Lua::ILuaBase *LUA );

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );
}
