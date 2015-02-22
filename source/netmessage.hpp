#pragma once

#include <main.hpp>

class INetMessage;
class CNetChan;

namespace NetMessage
{

void Push( lua_State *state, INetMessage *msg, CNetChan *netchan = nullptr );

INetMessage *Get(
	lua_State *state,
	int32_t index,
	CNetChan **netchan = nullptr,
	bool cleanup = false
);

void Destroy( lua_State *state, CNetChan *netchan );

void Initialize( lua_State *state );

void Deinitialize( lua_State *state );

}