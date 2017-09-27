#include <main.hpp>
#include <net.hpp>
#include <hooks.hpp>
#include <sn_bf_write.hpp>
#include <sn_bf_read.hpp>
#include <netchannel.hpp>
#include <netchannelhandler.hpp>
#include <subchannel.hpp>
#include <datafragments.hpp>
#include <filehandle.hpp>
#include <ucharptr.hpp>
#include <netadr.hpp>
#include <networkstringtablecontainer.hpp>
#include <networkstringtable.hpp>
#include <gameeventmanager.hpp>
#include <gameevent.hpp>
#include <netmessage.hpp>
#include <scanning/symbolfinder.hpp>
#include <GarrysMod/Interfaces.hpp>
#include <interface.h>
#include <eiface.h>
#include <cdll_int.h>
#include <iserver.h>
#include <Platform.hpp>

namespace global
{

#if defined SYSTEM_WINDOWS

	static const char IServer_sig[] = "\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\xD8\x6D\x24\x83\x4D\xEC\x10";
	static const size_t IServer_siglen = sizeof( IServer_sig ) - 1;

#elif defined SYSTEM_LINUX

#if defined SOURCENET_SERVER

	static const char IServer_sig[] = "@sv";
	static const size_t IServer_siglen = 0;

#elif defined SOURCENET_CLIENT

	static const char IServer_sig[] = "\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\xC7\x85\x68\xFF\xFF\xFF\x68";
	static const size_t IServer_siglen = sizeof( IServer_sig ) - 1;

#endif

#elif defined SYSTEM_MACOSX

	static const char IServer_sig[] = "\x2A\x2A\x2A\x2A\x8B\x08\x89\x04\x24\xFF\x51\x28\xF3\x0F\x10\x45";
	static const size_t IServer_siglen = sizeof( IServer_sig ) - 1;

#endif

	static bool loaded = false;

	const std::string engine_lib = Helpers::GetBinaryFileName(
		"engine",
		false,
		IS_SERVERSIDE,
		"bin/"
	);

	const std::string client_lib = Helpers::GetBinaryFileName(
		"client",
		false,
		IS_SERVERSIDE,
		"garrysmod/bin/"
	);

	const std::string server_lib = Helpers::GetBinaryFileName(
		"server",
		false,
		IS_SERVERSIDE,
		"garrysmod/bin/"
	);

	const char *tostring_format = "%s: %p";

	GarrysMod::Lua::ILuaBase *lua = nullptr;

	SourceSDK::FactoryLoader engine_loader( engine_lib, false, false );

	IVEngineServer *engine_server = nullptr;
	IVEngineClient *engine_client = nullptr;
	IServer *server = nullptr;

	bool is_dedicated = true;

	LUA_FUNCTION( index )
	{
		LUA->GetMetaTable( 1 );
		LUA->Push( 2 );
		LUA->RawGet( -2 );
		if( !LUA->IsType( -1, GarrysMod::Lua::Type::NIL ) )
			return 1;

		LUA->Pop( 2 );

		LUA->GetFEnv( 1 );
		LUA->Push( 2 );
		LUA->RawGet( -2 );
		return 1;
	}

	LUA_FUNCTION( newindex )
	{
		LUA->GetFEnv( 1 );
		LUA->Push( 2 );
		LUA->Push( 3 );
		LUA->RawSet( -3 );
		return 0;
	}

	LUA_FUNCTION( GetTable )
	{
		LUA->GetFEnv( 1 );
		return 1;
	}

	void CheckType( GarrysMod::Lua::ILuaBase *LUA, int32_t index, int32_t type, const char *nametype )
	{
		if( !LUA->IsType( index, type ) )
			LUA->TypeError( index, nametype );
	}

	static void Initialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		LUA->CreateTable( );

		LUA->PushString( "sourcenet 1.1.0" );
		LUA->SetField( -2, "Version" );

		// version num follows LuaJIT style, xxyyzz
		LUA->PushNumber( 10100 );
		LUA->SetField( -2, "VersionNum" );

		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "sourcenet" );

		loaded = true;
	}

	static void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		LUA->PushNil( );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "sourcenet" );

		loaded = false;
	}

}

GMOD_MODULE_OPEN( )
{
	global::lua = LUA;

	global::engine_server = global::engine_loader.GetInterface<IVEngineServer>( INTERFACEVERSION_VENGINESERVER );
	if( global::engine_server == nullptr )
		LUA->ThrowError( "failed to retrieve server engine interface" );

	global::is_dedicated = global::engine_server->IsDedicatedServer( );
	if( !global::is_dedicated )
	{
		global::engine_client = global::engine_loader.GetInterface<IVEngineClient>( VENGINE_CLIENT_INTERFACE_VERSION );
		if( global::engine_client == nullptr )
			LUA->ThrowError( "failed to retrieve client engine interface" );
	}

	IServer **pserver = nullptr;
	{
		SymbolFinder symfinder;

		pserver = reinterpret_cast<IServer **>( symfinder.ResolveOnBinary(
			global::engine_lib.c_str( ), global::IServer_sig, global::IServer_siglen
		) );
	}

	if( pserver == nullptr )
		LUA->ThrowError( "failed to locate IServer pointer" );

	global::server = *pserver;
	if( global::server == nullptr )
		LUA->ThrowError( "failed to locate IServer" );

	NetMessage::PreInitialize( LUA );
	Hooks::PreInitialize( LUA );

	sn_bf_write::Initialize( LUA );
	sn_bf_read::Initialize( LUA );
	NetChannel::Initialize( LUA );
	subchannel::Initialize( LUA );
	dataFragments::Initialize( LUA );
	FileHandle::Initialize( LUA );
	UCHARPTR::Initialize( LUA );
	netadr::Initialize( LUA );
	NetChannelHandler::Initialize( LUA );
	NetworkStringTableContainer::Initialize( LUA );
	NetworkStringTable::Initialize( LUA );
	GameEventManager::Initialize( LUA );
	GameEvent::Initialize( LUA );
	NetMessage::Initialize( LUA );
	Hooks::Initialize( LUA );
	global::Initialize( LUA );

	return 0;
}

GMOD_MODULE_CLOSE( )
{
	if( !global::loaded )
		return 0;

	sn_bf_write::Deinitialize( LUA );
	sn_bf_read::Deinitialize( LUA );
	NetChannel::Deinitialize( LUA );
	subchannel::Deinitialize( LUA );
	dataFragments::Deinitialize( LUA );
	FileHandle::Deinitialize( LUA );
	UCHARPTR::Deinitialize( LUA );
	netadr::Deinitialize( LUA );
	NetChannelHandler::Deinitialize( LUA );
	NetworkStringTableContainer::Deinitialize( LUA );
	NetworkStringTable::Deinitialize( LUA );
	GameEventManager::Deinitialize( LUA );
	GameEvent::Deinitialize( LUA );
	NetMessage::Deinitialize( LUA );
	Hooks::Deinitialize( LUA );
	global::Deinitialize( LUA );

	return 0;
}
