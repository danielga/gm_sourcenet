#pragma once

#include <GarrysMod/Lua/Interface.h>
#include <lua.hpp>
#include <cstdint>
#include <string>
#include <GarrysMod/Interfaces.hpp>

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

extern bool is_dedicated;

LUA_FUNCTION( index );
LUA_FUNCTION( newindex );
LUA_FUNCTION( GetTable );

void CheckType( lua_State *state, int32_t index, int32_t type, const char *nametype );

void ThrowError( lua_State *state, const char *fmt, ... );

const QAngle &GetAngle( lua_State *state, int32_t index );
void PushAngle( lua_State *state, const QAngle &ang );

const Vector &GetVector( lua_State *state, int32_t index );
void PushVector( lua_State *state, const Vector &vec );

}
