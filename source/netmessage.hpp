#pragma once

#include <main.hpp>

class INetMessage;
class CNetChan;

namespace NetMessage
{

void Push( GarrysMod::Lua::ILuaBase *LUA, INetMessage *msg, CNetChan *netchan = nullptr );

INetMessage *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index, CNetChan **netchan = nullptr );

void Destroy( GarrysMod::Lua::ILuaBase *LUA, CNetChan *netchan );

void PreInitialize( GarrysMod::Lua::ILuaBase *LUA );

void Initialize( GarrysMod::Lua::ILuaBase *LUA );

void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );

}
