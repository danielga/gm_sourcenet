#pragma once

#include <main.hpp>

typedef void *FileHandle_t;

void Push_FileHandle( lua_State *state, FileHandle_t file );

FileHandle_t Get_FileHandle( lua_State *state, int32_t index );

EXT_META_ID( FileHandle_t, 6 );

EXT_META_FUNCTION( FileHandle_t, __eq );
EXT_META_FUNCTION( FileHandle_t, __tostring );