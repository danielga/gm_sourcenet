#pragma once

#include <main.hpp>

class INetChannelHandler;

void Push_INetChannelHandler( lua_State *state, INetChannelHandler *handler );

INetChannelHandler *Get_INetChannelHandler( lua_State *state, int32_t index );

EXT_META_ID( INetChannelHandler, 9 );

EXT_META_FUNCTION( INetChannelHandler, __eq );
EXT_META_FUNCTION( INetChannelHandler, __tostring );