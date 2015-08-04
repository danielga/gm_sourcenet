#pragma once

#include <main.hpp>

class bf_write;

namespace sn_bf_write
{

bf_write **Push(
	lua_State *state,
	bf_write *writer = nullptr,
	int32_t bufref = -1
);

bf_write *Get(
	lua_State *state,
	int32_t index,
	int32_t *bufref = nullptr,
	bool cleanup = false
);

void Initialize( lua_State *state );

void Deinitialize( lua_State *state );

}
