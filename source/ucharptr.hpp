#pragma once

#include <main.hpp>

namespace UCHARPTR
{

extern const uint8_t metaid;
extern const char *metaname;

uint8_t *Push( lua_State *state, int32_t bits, uint8_t *data = nullptr );

uint8_t *Get(
	lua_State *state,
	int32_t index,
	int32_t *bits = nullptr,
	bool cleanup = false,
	bool *own = nullptr
);

void Initialize( lua_State *state );

void Deinitialize( lua_State *state );

}