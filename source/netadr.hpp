#pragma once

#include <main.hpp>

typedef struct netadr_s netadr_t;

namespace netadr
{

void Push( lua_State *state, const netadr_t &netadr );

void Initialize( lua_State *state );

void Deinitialize( lua_State *state );

}
