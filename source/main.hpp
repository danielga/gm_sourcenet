#pragma once

#include <GarrysMod/Lua/Interface.h>
#include <lua.hpp>
#include <cstdint>
#include <new>
#include <string>
#include <helpers.hpp>

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

static const std::string engine_lib = helpers::GetBinaryFileName(
	"engine",
	false,
	IS_SERVERSIDE,
	"bin/"
);

static const std::string client_lib = helpers::GetBinaryFileName(
	"client",
	false,
	IS_SERVERSIDE,
	"garrysmod/bin/"
);

static const std::string server_lib = helpers::GetBinaryFileName(
	"server",
	false,
	IS_SERVERSIDE,
	"garrysmod/bin/"
);

#if defined _WIN32

static const char *tostring_format = "%s: 0x%p";

#elif defined __linux || defined __APPLE__

static const char *tostring_format = "%s: %p";

#endif

static const uint8_t metabase = 100;

extern lua_State *lua_state;

extern void *( *engine_factory )( const char *pName, int *pReturnCode );

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