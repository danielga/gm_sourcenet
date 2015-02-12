#pragma once

#include <main.hpp>

class INetworkStringTable;

void Push_INetworkStringTable( lua_State *state, INetworkStringTable *table );

EXT_META_ID( INetworkStringTable, 11 );

EXT_META_FUNCTION( INetworkStringTable, __eq );
EXT_META_FUNCTION( INetworkStringTable, __tostring );

EXT_META_FUNCTION( INetworkStringTable, FindStringIndex );
EXT_META_FUNCTION( INetworkStringTable, GetString );