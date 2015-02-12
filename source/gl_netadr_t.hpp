#pragma once

#include <main.hpp>

typedef struct netadr_s netadr_t;

void Push_netadr( lua_State *state, const netadr_t &netadr );

EXT_META_ID( netadr_t, 8 );

EXT_META_FUNCTION( netadr_t, __eq );
EXT_META_FUNCTION( netadr_t, __tostring );

EXT_META_FUNCTION( netadr_t, IsLocalhost );
EXT_META_FUNCTION( netadr_t, IsLoopback );
EXT_META_FUNCTION( netadr_t, IsReservedAdr );
EXT_META_FUNCTION( netadr_t, IsValid );

EXT_META_FUNCTION( netadr_t, GetIP );
EXT_META_FUNCTION( netadr_t, GetPort );
EXT_META_FUNCTION( netadr_t, GetType );