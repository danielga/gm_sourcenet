#pragma once

#include "main.hpp"

typedef struct subchannel_s subchannel_t;
class CNetChan;

namespace subchannel
{
	void Push( GarrysMod::Lua::ILuaBase *LUA, subchannel_t *subchan, CNetChan *netchan );

	void Initialize( GarrysMod::Lua::ILuaBase *LUA );

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );
}
