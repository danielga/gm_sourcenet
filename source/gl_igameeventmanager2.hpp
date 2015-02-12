#pragma once

#include <main.hpp>

EXT_META_ID( IGameEventManager2, 12 );

EXT_META_FUNCTION( IGameEventManager2, __eq );
EXT_META_FUNCTION( IGameEventManager2, __tostring );

EXT_META_FUNCTION( IGameEventManager2, CreateEvent );
EXT_META_FUNCTION( IGameEventManager2, SerializeEvent );
EXT_META_FUNCTION( IGameEventManager2, UnserializeEvent );

EXT_GLBL_FUNCTION( IGameEventManager2 );