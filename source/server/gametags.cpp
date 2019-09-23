#include "gametags.hpp"

#include <strtools.h>
#include <networkstringtabledefs.h>
#include <steam/steam_gameserver.h>
#include <scanning/symbolfinder.hpp>
#include <detouring/classproxy.hpp>

class CBaseServer;

namespace GameTags
{
	class CSteamGameServerAPIContext
	{
	public:
		ISteamClient *m_pSteamClient;
		ISteamGameServer *m_pSteamGameServer;
		ISteamUtils *m_pSteamGameServerUtils;
		ISteamNetworking *m_pSteamGameServerNetworking;
		ISteamGameServerStats *m_pSteamGameServerStats;
		ISteamHTTP *m_pSteamHTTP;
		ISteamInventory *m_pSteamInventory;
		ISteamUGC *m_pSteamUGC;
		ISteamApps *m_pSteamApps;
	};

	class CBaseServerProxy : Detouring::ClassProxy<CBaseServer, CBaseServerProxy>
	{
	public:
		static void Initialize( GarrysMod::Lua::ILuaBase *LUA )
		{
			{
				SymbolFinder symfinder;

				RecalculateTags_original =
					reinterpret_cast<RecalculateTags_t>( symfinder.Resolve(
						global::engine_loader.GetModuleLoader( ).GetModule( ),
						RecalculateTags_sig,
						RecalculateTags_siglen
					) );

#if defined SYSTEM_WINDOWS

				CSteamGameServerAPIContext **gameserver_context_pointer =
					reinterpret_cast<CSteamGameServerAPIContext **>( symfinder.Resolve(
						global::server_loader.GetModule( ),
						SteamGameServerAPIContext_sig,
						SteamGameServerAPIContext_siglen
					) );
				if( gameserver_context_pointer == nullptr )
					LUA->ThrowError(
						"Failed to load required CSteamGameServerAPIContext interface pointer."
					);

				gameserver_context = *gameserver_context_pointer;

#else

				gameserver_context =
					reinterpret_cast<CSteamGameServerAPIContext *>( symfinder.ResolveOnBinary(
						global::server_lib.c_str( ),
						SteamGameServerAPIContext_sig,
						SteamGameServerAPIContext_siglen
					) );

#endif

				if( gameserver_context == nullptr )
					LUA->ThrowError( "Failed to load required CSteamGameServerAPIContext interface." );
			}

			if( RecalculateTags_original == nullptr )
				LUA->ThrowError( "unable to find CBaseServer::RecalculateTags" );
		}

		void RecalculateTags( )
		{
			ISteamGameServer *gameserver = gameserver_context->m_pSteamGameServer;
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
		typedef void( *RecalculateTags_t )( CBaseServer * );

		static const char RecalculateTags_sig[];
		static const size_t RecalculateTags_siglen;

		static const char SteamGameServerAPIContext_sig[];
		static const size_t SteamGameServerAPIContext_siglen;

		static RecalculateTags_t RecalculateTags_original;
		static CSteamGameServerAPIContext *gameserver_context;
		static std::string gametags_substitute;
	};

#if defined SYSTEM_WINDOWS

	const char CBaseServerProxy::RecalculateTags_sig[] =
		"\x55\x8B\xEC\x83\xEC\x60\x56\x6A\x00\x68\x2A\x2A\x2A\x2A\x8D\x4D\xE8";
	const size_t CBaseServerProxy::RecalculateTags_siglen =
		sizeof( CBaseServerProxy::RecalculateTags_sig ) - 1;

	const char CBaseServerProxy::SteamGameServerAPIContext_sig[] =
		"\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x6A\x00\x68\x2A\x2A\x2A\x2A\xFF\x55\x08\x83\xC4\x08\xA3";
	const size_t CBaseServerProxy::SteamGameServerAPIContext_siglen =
		sizeof( CBaseServerProxy::SteamGameServerAPIContext_sig ) - 1;

#elif defined SYSTEM_POSIX

	const char CBaseServerProxy::RecalculateTags_sig[] = "@_ZN11CBaseServer15RecalculateTagsEv";
	const size_t CBaseServerProxy::RecalculateTags_siglen = 0;

	const char CBaseServerProxy::SteamGameServerAPIContext_sig[] = "@_ZL27s_SteamGameServerAPIContext";
	const size_t CBaseServerProxy::SteamGameServerAPIContext_siglen = 0;

#endif

	CBaseServerProxy::RecalculateTags_t CBaseServerProxy::RecalculateTags_original = nullptr;
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
