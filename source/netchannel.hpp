#pragma once

#include <main.hpp>

class CNetChan;

namespace NetChannel
{

bool IsValid( CNetChan *netchan );

void Push( lua_State *state, CNetChan *netchan );

CNetChan *Get( lua_State *state, int32_t index );

void Destroy( lua_State *state, CNetChan *netchan );

void Initialize( lua_State *state );

void Deinitialize( lua_State *state );

}