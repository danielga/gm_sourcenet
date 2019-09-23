#pragma once

#include <GarrysMod/Lua/Interface.h>
#include <lua.hpp>
#include <stdint.h>
#include <string>
#include <GarrysMod/FactoryLoader.hpp>
#include <Platform.hpp>

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
	extern const char *tostring_format;

	extern GarrysMod::Lua::ILuaBase *lua;

	extern const SourceSDK::FactoryLoader engine_loader;
	extern const SourceSDK::ModuleLoader client_loader;
	extern const SourceSDK::ModuleLoader server_loader;

	extern IVEngineServer *engine_server;

	extern IVEngineClient *engine_client;

	extern IServer *server;

	extern bool is_dedicated;

	LUA_FUNCTION_DECLARE( index );
	LUA_FUNCTION_DECLARE( newindex );
	LUA_FUNCTION_DECLARE( GetTable );

	void CheckType( GarrysMod::Lua::ILuaBase *LUA, int32_t index, int32_t type, const char *nametype );
}
