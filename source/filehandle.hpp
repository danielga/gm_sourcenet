#pragma once

#include <main.hpp>

typedef void *FileHandle_t;

namespace FileHandle
{

extern const uint8_t metatype;
extern const char *metaname;

void Push( lua_State *state, FileHandle_t file );

FileHandle_t Get( lua_State *state, int32_t index );

void Initialize( lua_State *state );

void Deinitialize( lua_State *state );

}
