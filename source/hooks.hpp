#pragma once

#include <main.hpp>

namespace Hooks
{

void Initialize( lua_State *state );

void Deinitialize( lua_State *state );

}