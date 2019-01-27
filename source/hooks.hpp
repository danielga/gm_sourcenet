#pragma once

#include "main.hpp"

class CNetChan;
class INetChannelHandler;

namespace Hooks
{
	void PreInitialize( GarrysMod::Lua::ILuaBase *LUA );

	void Initialize( GarrysMod::Lua::ILuaBase *LUA );

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );

	void HookCNetChan( CNetChan *netchan );

	void HookINetChannelHandler( INetChannelHandler *handler );
}
