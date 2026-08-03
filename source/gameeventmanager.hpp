#pragma once

#include "main.hpp"

class IGameEventManager2;

namespace GameEventManager
{
	void Initialize( GarrysMod::Lua::ILuaBase *LUA );

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );

	// The engine game-event manager singleton, or nullptr. Lets GameEvent's __gc tell a
	// live wrapper from a userdata whose backing was recycled.
	IGameEventManager2 *GetManager( );
}
