#pragma once

#include <main.hpp>

class INetworkStringTable;

namespace NetworkStringTable
{

void Push( lua_State *state, INetworkStringTable *table );

void Initialize( lua_State *state );

void Deinitialize( lua_State *state );

}
