#pragma once

#include <main.hpp>

class INetMessage;
class CNetChan;

namespace NetMessage
{

void Push( lua_State *state, INetMessage *msg, CNetChan *netchan );

INetMessage *Get( lua_State *state, int32_t index, CNetChan **netchan = nullptr );

//void Destroy( lua_State *state, INetMessage *msg );

void Initialize( lua_State *state );

void Deinitialize( lua_State *state );

}