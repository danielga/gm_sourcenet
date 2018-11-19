#pragma once

#include "main.hpp"

class IGameEventManager2;
class IGameEvent;

namespace GameEvent
{
	void Push( GarrysMod::Lua::ILuaBase *LUA, IGameEvent *event, IGameEventManager2 *manager );

	IGameEvent *Get(
		GarrysMod::Lua::ILuaBase *LUA,
		int32_t index,
		IGameEventManager2 **manager = nullptr
	);

	void Initialize( GarrysMod::Lua::ILuaBase *LUA );

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );
}
