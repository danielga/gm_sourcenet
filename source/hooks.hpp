#pragma once

#include <main.hpp>

class CNetChan;
class INetChannelHandler;

namespace Hooks
{

void PreInitialize( lua_State *state );

void Initialize( lua_State *state );

void Deinitialize( lua_State *state );

void HookCNetChan( lua_State *state );

void HookINetChannelHandler( lua_State *state );

}