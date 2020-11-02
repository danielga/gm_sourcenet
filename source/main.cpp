#include "main.hpp"
#include "net.hpp"
#include "hooks.hpp"
#include "sn_bf_write.hpp"
#include "sn_bf_read.hpp"
#include "netchannel.hpp"
#include "netchannelhandler.hpp"
#include "subchannel.hpp"
#include "datafragments.hpp"
#include "filehandle.hpp"
#include "ucharptr.hpp"
#include "netadr.hpp"
#include "networkstringtablecontainer.hpp"
#include "networkstringtable.hpp"
#include "gameeventmanager.hpp"
#include "gameevent.hpp"
#include "netmessage.hpp"

#if defined SOURCENET_SERVER

#include "server/datapack.hpp"
#include "server/gametags.hpp"

#endif

#include <GarrysMod/InterfacePointers.hpp>

#include <interface.h>
#include <eiface.h>
#include <cdll_int.h>
#include <iserver.h>

namespace global
{
	static bool loaded = false;

	const char *tostring_format = "%s: %p";

	GarrysMod::Lua::ILuaBase *lua = nullptr;

	const SourceSDK::FactoryLoader engine_loader( "engine" );
	const SourceSDK::ModuleLoader client_loader( "client" );
	const SourceSDK::ModuleLoader server_loader( "server" );

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

		LUA->PushString( "sourcenet 1.1.11" );
		LUA->SetField( -2, "Version" );

		// version num follows LuaJIT style, xxyyzz
		LUA->PushNumber( 10111 );
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

	global::engine_server = InterfacePointers::VEngineServer( );
	if( global::engine_server == nullptr )
		LUA->ThrowError( "failed to retrieve server engine interface" );

	global::is_dedicated = global::engine_server->IsDedicatedServer( );
	if( !global::is_dedicated )
	{
		global::engine_client = InterfacePointers::VEngineClient( );
		if( global::engine_client == nullptr )
			LUA->ThrowError( "failed to retrieve client engine interface" );
	}

	global::server = InterfacePointers::Server( );
	if( global::server == nullptr )
		LUA->ThrowError( "failed to locate IServer" );

#if defined SOURCENET_SERVER

	DataPack::PreInitialize( LUA );
	GameTags::PreInitialize( LUA );

#endif

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

#if defined SOURCENET_SERVER

	DataPack::Initialize( LUA );
	GameTags::Initialize( LUA );

#endif

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

#if defined SOURCENET_SERVER

	DataPack::Deinitialize( LUA );
	GameTags::Deinitialize( LUA );

#endif

	NetMessage::Deinitialize( LUA );
	Hooks::Deinitialize( LUA );
	global::Deinitialize( LUA );

	return 0;
}
