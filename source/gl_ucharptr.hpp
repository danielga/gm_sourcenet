#pragma once

#include <main.hpp>

typedef uint8_t *UCHARPTR;

void Push_UCHARPTR( lua_State *state, UCHARPTR data, int32_t bits );

UCHARPTR Get_UCHARPTR( lua_State *state, int32_t index, int32_t *bits = nullptr, bool cleanup = false );

EXT_META_ID( UCHARPTR, 7 );

EXT_META_FUNCTION( UCHARPTR, __gc );
EXT_META_FUNCTION( UCHARPTR, __eq );
EXT_META_FUNCTION( UCHARPTR, __tostring );

EXT_META_FUNCTION( UCHARPTR, IsValid );

EXT_META_FUNCTION( UCHARPTR, Size );

EXT_GLBL_FUNCTION( UCHARPTR );