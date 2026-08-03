#include "gametags.hpp"

#include <GarrysMod/FunctionPointers.hpp>
#include <GarrysMod/InterfacePointers.hpp>

#include <detouring/classproxy.hpp>

#include <strtools.h>
#include <networkstringtabledefs.h>
#include <steam/steam_gameserver.h>

class CBaseServer;
class CSteam3Server : public CSteamGameServerAPIContext { };

namespace GameTags
{
	class CBaseServerProxy : Detouring::ClassProxy<CBaseServer, CBaseServerProxy>
	{
	public:
		static void Initialize( GarrysMod::Lua::ILuaBase * )
		{
			// Non-fatal: if any signature scan misses, disable just GameTags
			// (SetGameTags returns false) instead of aborting the module load.
			RecalculateTags_original = FunctionPointers::CBaseServer_RecalculateTags( );
			if( RecalculateTags_original == nullptr )
				return;

			FunctionPointers::Steam3Server_t Steam3Server = FunctionPointers::Steam3Server( );
			if( Steam3Server == nullptr )
				return;

			gameserver_context = Steam3Server( );
			if( gameserver_context == nullptr )
				return;

			available = true;
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

		LUA_FUNCTION_STATIC_MEMBER( SetGameTags )
		{
			if( !available )
			{
				LUA->PushBool( false );
				return 1;
			}

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

	private:
		static FunctionPointers::CBaseServer_RecalculateTags_t RecalculateTags_original;
		static CSteam3Server *gameserver_context;
		static std::string gametags_substitute;
		static bool available;

		// A live instance keeps the ClassProxy SharedState alive so the static
		// Hook()/UnHook() entrypoints work.
		static CBaseServerProxy Singleton;
	};

	FunctionPointers::CBaseServer_RecalculateTags_t
		CBaseServerProxy::RecalculateTags_original = nullptr;
	CSteam3Server *CBaseServerProxy::gameserver_context = nullptr;
	std::string CBaseServerProxy::gametags_substitute;
	bool CBaseServerProxy::available = false;
	CBaseServerProxy CBaseServerProxy::Singleton;

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
