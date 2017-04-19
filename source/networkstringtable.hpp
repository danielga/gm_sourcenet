#pragma once

#include <main.hpp>

class INetworkStringTable;

namespace NetworkStringTable
{

void Push( GarrysMod::Lua::ILuaBase *LUA, INetworkStringTable *table );

void Initialize( GarrysMod::Lua::ILuaBase *LUA );

void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );

}
