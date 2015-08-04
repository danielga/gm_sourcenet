#pragma once

#include <GarrysMod/Lua/Interface.h>
#include <lua.hpp>
#include <cstdint>
#include <new>
#include <string>
#include <interfaces.hpp>

#if defined __linux || defined __APPLE__

#undef min
#undef max

#endif

class IVEngineServer;
class IVEngineClient;
class IServer;
class QAngle;
class Vector;

namespace global
{

extern const std::string engine_lib;

extern const std::string client_lib;

extern const std::string server_lib;

extern const char *tostring_format;

extern const uint8_t metabase;

extern lua_State *lua_state;

extern SourceSDK::FactoryLoader engine_loader;

extern IVEngineServer *engine_server;

extern IVEngineClient *engine_client;

extern IServer *server;

LUA_FUNCTION( index );
LUA_FUNCTION( newindex );
LUA_FUNCTION( GetTable );

void CheckType( lua_State *state, int32_t index, int32_t type, const char *nametype );

void ThrowError( lua_State *state, const char *fmt, ... );

void PushAngle( lua_State *state, const QAngle &ang );

void PushVector( lua_State *state, const Vector &vec );

}
