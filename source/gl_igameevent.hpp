#pragma once

#include <main.hpp>

class IGameEventManager2;
class IGameEvent;

void Push_IGameEvent( lua_State *state, IGameEvent *event, IGameEventManager2 *manager );

IGameEvent *Get_IGameEvent(
	lua_State *state,
	int32_t index,
	IGameEventManager2 **manager = nullptr,
	bool cleanup = false
);

EXT_META_ID( IGameEvent, 13 );

EXT_META_FUNCTION( IGameEvent, __gc );
EXT_META_FUNCTION( IGameEvent, __eq );
EXT_META_FUNCTION( IGameEvent, __tostring );

EXT_META_FUNCTION( IGameEvent, GetName );

EXT_META_FUNCTION( IGameEvent, IsReliable );
EXT_META_FUNCTION( IGameEvent, IsLocal );
EXT_META_FUNCTION( IGameEvent, IsEmpty );

EXT_META_FUNCTION( IGameEvent, GetBool );
EXT_META_FUNCTION( IGameEvent, GetInt );
EXT_META_FUNCTION( IGameEvent, GetFloat );
EXT_META_FUNCTION( IGameEvent, GetString );

EXT_META_FUNCTION( IGameEvent, SetBool );
EXT_META_FUNCTION( IGameEvent, SetInt );
EXT_META_FUNCTION( IGameEvent, SetFloat );
EXT_META_FUNCTION( IGameEvent, SetString );