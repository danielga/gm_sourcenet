#pragma once

#include <main.hpp>

namespace UCHARPTR
{

extern uint8_t metatype;
extern const char *metaname;

uint8_t *Push( GarrysMod::Lua::ILuaBase *LUA, int32_t bits, uint8_t *data = nullptr );

uint8_t *Get(
	GarrysMod::Lua::ILuaBase *LUA,
	int32_t index,
	int32_t *bits = nullptr,
	bool *own = nullptr
);

uint8_t *Release( GarrysMod::Lua::ILuaBase *LUA, int32_t index, int32_t *bits );

void Initialize( GarrysMod::Lua::ILuaBase *LUA );

void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );

}
