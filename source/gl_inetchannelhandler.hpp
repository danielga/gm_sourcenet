#pragma once

#include <main.hpp>

class INetChannelHandler;

namespace NetChannelHandler
{

void Push( lua_State *state, INetChannelHandler *handler );

INetChannelHandler *Get( lua_State *state, int32_t index );

void Initialize( lua_State *state );

void Deinitialize( lua_State *state );

}