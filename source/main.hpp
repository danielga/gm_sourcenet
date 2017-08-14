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

namespace GarrysMod
{
	namespace Lua
	{
		class ILuaBase;
	}
}

namespace global
{

extern const std::string engine_lib;

extern const std::string client_lib;

extern const std::string server_lib;

extern const char *tostring_format;

extern GarrysMod::Lua::ILuaBase *lua;

extern SourceSDK::FactoryLoader engine_loader;

extern IVEngineServer *engine_server;

extern IVEngineClient *engine_client;

extern IServer *server;

extern bool is_dedicated;

LUA_FUNCTION_DECLARE( index );
LUA_FUNCTION_DECLARE( newindex );
LUA_FUNCTION_DECLARE( GetTable );

void CheckType( GarrysMod::Lua::ILuaBase *LUA, int32_t index, int32_t type, const char *nametype );

}
