#pragma once

#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/FactoryLoader.hpp>
#include <lua.hpp>
#include <Platform.hpp>

#include <cstdint>
#include <string>
#include <limits>

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

	template<typename NumericType>
	inline NumericType GetNumber( GarrysMod::Lua::ILuaBase *LUA, int32_t idx,
		NumericType min = std::numeric_limits<NumericType>::min( ),
		NumericType max = std::numeric_limits<NumericType>::max( ) )
	{
		double number = LUA->CheckNumber( idx );
		if( number < static_cast<double>( min ) )
		{
			auto extra = LUA->PushFormattedString( "number %f is less than minimum limit of %f",
				number, static_cast<double>( min ) );
			LUA->ArgError( idx, extra );
		}
		else if( number > static_cast<double>( max ) )
		{
			auto extra = LUA->PushFormattedString( "number %f is greater than maximum limit of %f",
				number, static_cast<double>( min ) );
			LUA->ArgError( idx, extra );
		}

		return static_cast<NumericType>( number );
	}
}
