#pragma once

#include <main.hpp>

class bf_read;

namespace sn_bf_read
{

bf_read **Push(
	lua_State *state,
	bf_read *reader = nullptr,
	int32_t bufref = -1
);

bf_read *Get(
	lua_State *state,
	int32_t index,
	int32_t *bufref = nullptr,
	bool cleanup = false
);

void Initialize( lua_State *state );

void Deinitialize( lua_State *state );

}
