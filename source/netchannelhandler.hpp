#pragma once

#include "main.hpp"

class INetChannelHandler;

namespace NetChannelHandler
{
	void Push( GarrysMod::Lua::ILuaBase *LUA, INetChannelHandler *handler );

	INetChannelHandler *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index );

	void Destroy( GarrysMod::Lua::ILuaBase *LUA, INetChannelHandler *handler );

	void Initialize( GarrysMod::Lua::ILuaBase *LUA );

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );
}
