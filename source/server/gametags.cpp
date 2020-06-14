#include "gametags.hpp"

#include <GarrysMod/FunctionPointers.hpp>
#include <GarrysMod/InterfacePointers.hpp>

#include <detouring/classproxy.hpp>

#include <strtools.h>
#include <networkstringtabledefs.h>
#include <steam/steam_gameserver.h>

class CBaseServer;

namespace GameTags
{
	class CBaseServerProxy : Detouring::ClassProxy<CBaseServer, CBaseServerProxy>
	{
	public:
		static void Initialize( GarrysMod::Lua::ILuaBase *LUA )
		{
			RecalculateTags_original = FunctionPointers::CBaseServer_RecalculateTags( );
			if( RecalculateTags_original == nullptr )
				LUA->ThrowError( "unable to find CBaseServer::RecalculateTags" );

			gameserver_context = InterfacePointers::SteamGameServerAPIContext( );
			if( gameserver_context == nullptr )
				LUA->ThrowError( "Failed to load required CSteamGameServerAPIContext interface." );
		}

		void RecalculateTags( )
		{
			ISteamGameServer *gameserver = gameserver_context->SteamGameServer( );
			if( gameserver != nullptr )
				gameserver->SetGameTags( gametags_substitute.c_str( ) );
		}

		static bool HookRecalculateTags( )
		{
			return Hook( RecalculateTags_original, &CBaseServerProxy::RecalculateTags );
		}

		static bool UnHookRecalculateTags( )
		{
			return UnHook( RecalculateTags_original );
		}

		LUA_FUNCTION_IMPLEMENT( SetGameTags )
		{
			if( LUA->IsType( 1, GarrysMod::Lua::Type::STRING ) )
				gametags_substitute = LUA->GetString( 1 );
			else
				gametags_substitute.clear( );

			LUA->PushBool(
				!gametags_substitute.empty( ) ?
				HookRecalculateTags( ) :
				UnHookRecalculateTags( )
			);
			return 1;
		}

		LUA_FUNCTION_WRAP( SetGameTags );

	private:
		static FunctionPointers::CBaseServer_RecalculateTags_t RecalculateTags_original;
		static CSteamGameServerAPIContext *gameserver_context;
		static std::string gametags_substitute;
	};

	FunctionPointers::CBaseServer_RecalculateTags_t
		CBaseServerProxy::RecalculateTags_original = nullptr;
	CSteamGameServerAPIContext *CBaseServerProxy::gameserver_context = nullptr;
	std::string CBaseServerProxy::gametags_substitute;

	void PreInitialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		CBaseServerProxy::Initialize( LUA );
	}

	void Initialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		LUA->PushCFunction( CBaseServerProxy::SetGameTags );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SetGameTags" );
	}

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		CBaseServerProxy::UnHookRecalculateTags( );

		LUA->PushNil( );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SetGameTags" );
	}
}
