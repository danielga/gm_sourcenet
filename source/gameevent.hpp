#pragma once

#include <main.hpp>

class IGameEventManager2;
class IGameEvent;

namespace GameEvent
{

void Push( lua_State *state, IGameEvent *event, IGameEventManager2 *manager );

IGameEvent *Get(
	lua_State *state,
	int32_t index,
	IGameEventManager2 **manager = nullptr,
	bool cleanup = false
);

void Initialize( lua_State *state );

void Deinitialize( lua_State *state );

}