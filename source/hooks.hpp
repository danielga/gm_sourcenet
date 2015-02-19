#pragma once

#include <main.hpp>

class CNetChan;
class INetChannelHandler;

namespace Hooks
{

void Initialize( lua_State *state );

void Deinitialize( lua_State *state );

void HookCNetChan( lua_State *state, CNetChan *netchan );

void HookINetChannelHandler( lua_State *state, INetChannelHandler *handler );

}