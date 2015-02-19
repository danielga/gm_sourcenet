#pragma once

#include <main.hpp>
#include <net.h>

typedef CNetChan::dataFragments_t dataFragments_t;

namespace dataFragments
{

void Push(
	lua_State *state,
	dataFragments_t *datafrag,
	CNetChan *netchan = nullptr
);

dataFragments_t *Get(
	lua_State *state,
	int32_t index,
	CNetChan **netchan = nullptr,
	bool cleanup = false
);

void Initialize( lua_State *state );

void Deinitialize( lua_State *state );

}