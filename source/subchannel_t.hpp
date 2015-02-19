#pragma once

#include <main.hpp>

typedef struct subchannel_s subchannel_t;
class CNetChan;

namespace subchannel
{

void Push( lua_State *state, subchannel_t *subchan, CNetChan *netchan );

void Initialize( lua_State *state );

void Deinitialize( lua_State *state );

}