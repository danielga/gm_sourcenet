#include "gametags.hpp"

#include <GarrysMod/FunctionPointers.hpp>

#include <detouring/classproxy.hpp>

#include <strtools.h>
#include <networkstringtabledefs.h>
#include <steam/isteamgameserver.h>

#if defined( _WIN32 )
	#include <windows.h>
#else
	#include <dlfcn.h>
#endif

class CBaseServer;

namespace GameTags
{
	using SteamInternal_ContextInit_t = void *( * )( void * );
	using SteamInternal_FindOrCreateGameServerInterface_t = void *( * )( int hSteamUser, const char *pszVersion );
	using SteamGameServer_GetHSteamUser_t = int ( * )( );

	static SteamInternal_ContextInit_t s_pContextInit = nullptr;
	static SteamInternal_FindOrCreateGameServerInterface_t s_pFindOrCreateGS = nullptr;
	static SteamGameServer_GetHSteamUser_t s_pGetHSteamUser = nullptr;

	static void SteamInternal_Init_SteamGameServer( void *p )
	{
		*( void ** )p = s_pFindOrCreateGS( s_pGetHSteamUser( ), STEAMGAMESERVER_INTERFACE_VERSION );
	}

	static ISteamGameServer *GetSteamGameServer( )
	{
		static bool resolved = false;
		if( !resolved )
		{
			resolved = true;
#if defined( _WIN32 )
	#if defined( _WIN64 )
			HMODULE mod = GetModuleHandleA( "steam_api64.dll" );
	#else
			HMODULE mod = GetModuleHandleA( "steam_api.dll" );
	#endif
			if( mod != nullptr )
			{
				s_pContextInit = reinterpret_cast<SteamInternal_ContextInit_t>(
					GetProcAddress( mod, "SteamInternal_ContextInit" ) );
				s_pFindOrCreateGS = reinterpret_cast<SteamInternal_FindOrCreateGameServerInterface_t>(
					GetProcAddress( mod, "SteamInternal_FindOrCreateGameServerInterface" ) );
				s_pGetHSteamUser = reinterpret_cast<SteamGameServer_GetHSteamUser_t>(
					GetProcAddress( mod, "SteamGameServer_GetHSteamUser" ) );
			}
#else
			void *mod = dlopen( "libsteam_api.so", RTLD_NOLOAD | RTLD_LAZY );
			if( mod == nullptr )
				mod = dlopen( "libsteam_api.so", RTLD_LAZY );
			if( mod != nullptr )
			{
				s_pContextInit = reinterpret_cast<SteamInternal_ContextInit_t>(
					dlsym( mod, "SteamInternal_ContextInit" ) );
				s_pFindOrCreateGS = reinterpret_cast<SteamInternal_FindOrCreateGameServerInterface_t>(
					dlsym( mod, "SteamInternal_FindOrCreateGameServerInterface" ) );
				s_pGetHSteamUser = reinterpret_cast<SteamGameServer_GetHSteamUser_t>(
					dlsym( mod, "SteamGameServer_GetHSteamUser" ) );
			}
#endif
		}

		if( s_pContextInit == nullptr || s_pFindOrCreateGS == nullptr || s_pGetHSteamUser == nullptr )
			return nullptr;

		static void *s_ctx[3] = { reinterpret_cast<void *>( &SteamInternal_Init_SteamGameServer ), nullptr, nullptr };

		void *pSlot = s_pContextInit( s_ctx );
		if( pSlot == nullptr )
			return nullptr;
		return *reinterpret_cast<ISteamGameServer **>( pSlot );
	}

	class CBaseServerProxy : Detouring::ClassProxy<CBaseServer, CBaseServerProxy>
	{
	public:
		static void Initialize( GarrysMod::Lua::ILuaBase *LUA )
		{
			RecalculateTags_original = FunctionPointers::CBaseServer_RecalculateTags( );
			if( RecalculateTags_original == nullptr )
				LUA->ThrowError( "unable to find CBaseServer::RecalculateTags" );
		}

		void RecalculateTags( )
		{
			ISteamGameServer *gameserver = GetSteamGameServer( );
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
		static std::string gametags_substitute;
	};

	FunctionPointers::CBaseServer_RecalculateTags_t
		CBaseServerProxy::RecalculateTags_original = nullptr;
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
