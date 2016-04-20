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
#include <symbolfinder.hpp>
#include <vfnhook.h>
#include <GarrysMod/Interfaces.hpp>
#include <interface.h>
#include <eiface.h>
#include <cdll_int.h>
#include <iserver.h>

namespace global
{

#if defined _WIN32

static const char *IServer_sig = "\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\xD8\x6D\x24\x83\x4D\xEC\x10";
static const size_t IServer_siglen = 16;

static const char *netchunk_sig = "\x74\x2A\x85\xDB\x74\x2A\x8B\x43\x0C\x83\xC0\x07\xC1\xF8\x03\x85";
static const size_t netchunk_siglen = 16;

#elif defined __linux

#if defined SOURCENET_SERVER

static const char *IServer_sig = "@sv";
static const size_t IServer_siglen = 0;

#elif defined SOURCENET_CLIENT

static const char *IServer_sig = "\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x8D\x5D\x80\xC7";
static const size_t IServer_siglen = 13;

#endif

static const char *netchunk_sig = "\x74\x2A\x85\xFF\x74\x2A\x8B\x47\x0C\x83\xC0\x07\xC1\xF8\x03\x85";
static const size_t netchunk_siglen = 16;

#elif defined __APPLE__

static const char *IServer_sig = "\x2A\x2A\x2A\x2A\x8B\x08\x89\x04\x24\xFF\x51\x28\xD9\x9D\x9C\xFE";
static const size_t IServer_siglen = 16;

static const char *netchunk_sig = "\x74\x2A\x85\xD2\x74\x2A\x8B\x42\x0C\x83\xC0\x07\xC1\xf8\x03\x85";
static const size_t netchunk_siglen = 16;

#endif

static const size_t netpatch_len = 1;
static char netpatch_old[netpatch_len] = { 0 };
static const char *netpatch_new = "\x75";

static bool loaded = false;

const std::string engine_lib = helpers::GetBinaryFileName(
	"engine",
	false,
	IS_SERVERSIDE,
	"bin/"
);

const std::string client_lib = helpers::GetBinaryFileName(
	"client",
	false,
	IS_SERVERSIDE,
	"garrysmod/bin/"
);

const std::string server_lib = helpers::GetBinaryFileName(
	"server",
	false,
	IS_SERVERSIDE,
	"garrysmod/bin/"
);

#if defined _WIN32

const char *tostring_format = "%s: 0x%p";

#elif defined __linux || defined __APPLE__

const char *tostring_format = "%s: %p";

#endif

const uint8_t metabase = 100;

lua_State *lua_state = nullptr;

SourceSDK::FactoryLoader engine_loader( engine_lib, false, false );
static uint8_t *net_thread_chunk = nullptr;
CreateInterfaceFn engine_factory = nullptr;

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

	lua_getfenv( state, 1 );
	LUA->Push( 2 );
	LUA->RawGet( -2 );
	return 1;
}

LUA_FUNCTION( newindex )
{
	lua_getfenv( state, 1 );
	LUA->Push( 2 );
	LUA->Push( 3 );
	LUA->RawSet( -3 );
	return 0;
}

LUA_FUNCTION( GetTable )
{
	lua_getfenv( state, 1 );
	return 1;
}

void CheckType( lua_State *state, int32_t index, int32_t type, const char *nametype )
{
	if( !LUA->IsType( index, type ) )
		luaL_typerror( state, index, nametype );
}

void ThrowError( lua_State *state, const char *fmt, ... )
{
	va_list args;
	va_start( args, fmt );
	const char *error = lua_pushvfstring( state, fmt, args );
	va_end( args );
	LUA->ThrowError( error );
}

const QAngle &GetAngle( lua_State *state, int32_t index )
{
	LUA->CheckType( index, GarrysMod::Lua::Type::ANGLE );
	return *static_cast<const QAngle *>(
		static_cast<GarrysMod::Lua::UserData *>( LUA->GetUserdata( index ) )->data
	);
}

void PushAngle( lua_State *state, const QAngle &ang )
{
	LUA->GetField( GarrysMod::Lua::INDEX_GLOBAL, "Angle" );
	if( !LUA->IsType( -1, GarrysMod::Lua::Type::FUNCTION ) )
	{
		LUA->Pop( 1 );
		return;
	}

	LUA->PushNumber( ang.x );
	LUA->PushNumber( ang.y );
	LUA->PushNumber( ang.z );
	LUA->Call( 3, 1 );
}

const Vector &GetVector( lua_State *state, int32_t index )
{
	LUA->CheckType( index, GarrysMod::Lua::Type::VECTOR );
	return *static_cast<const Vector *>(
		static_cast<GarrysMod::Lua::UserData *>( LUA->GetUserdata( index ) )->data
	);
}

void PushVector( lua_State *state, const Vector &vec )
{
	LUA->GetField( GarrysMod::Lua::INDEX_GLOBAL, "Vector" );
	if( !LUA->IsType( -1, GarrysMod::Lua::Type::FUNCTION ) )
	{
		LUA->Pop( 1 );
		return;
	}

	LUA->PushNumber( vec.x );
	LUA->PushNumber( vec.y );
	LUA->PushNumber( vec.z );
	LUA->Call( 3, 1 );
}

static void Initialize( lua_State *state )
{
	LUA->CreateTable( );

	LUA->PushString( "sourcenet 1.0.0" );
	LUA->SetField( -2, "Version" );

	// version num follows LuaJIT style, xxyyzz
	LUA->PushNumber( 10000 );
	LUA->SetField( -2, "VersionNum" );

	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "sourcenet" );

	loaded = true;
}

static void Deinitialize( lua_State *state )
{
	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "sourcenet" );

	loaded = false;
}

}

GMOD_MODULE_OPEN( )
{
	global::lua_state = state;

	global::engine_factory = global::engine_loader.GetFactory( );
	if( global::engine_factory == nullptr )
		LUA->ThrowError( "failed to retrieve engine factory function" );

	global::engine_server = static_cast<IVEngineServer *>(
		global::engine_factory( INTERFACEVERSION_VENGINESERVER_VERSION_21, nullptr )
	);
	if( global::engine_server == nullptr )
		LUA->ThrowError( "failed to retrieve server engine interface" );

	global::is_dedicated = global::engine_server->IsDedicatedServer( );
	if( !global::is_dedicated )
	{
		global::engine_client = static_cast<IVEngineClient *>(
			global::engine_factory( "VEngineClient015", nullptr )
		);
		if( global::engine_client == nullptr )
			LUA->ThrowError( "failed to retrieve client engine interface" );
	}

	SymbolFinder symfinder;

	IServer **pserver = reinterpret_cast<IServer **>( symfinder.ResolveOnBinary(
		global::engine_lib.c_str( ), global::IServer_sig, global::IServer_siglen
	) );
	if( pserver == nullptr )
		LUA->ThrowError( "failed to locate IServer pointer" );

	global::server = *pserver;
	if( global::server == nullptr )
		LUA->ThrowError( "failed to locate IServer" );

#if defined SOURCENET_SERVER

	// Disables per-client threads (hacky fix for SendDatagram hooking)

	global::net_thread_chunk = static_cast<uint8_t *>( symfinder.ResolveOnBinary(
		global::engine_lib.c_str( ), global::netchunk_sig, global::netchunk_siglen
	) );
	if( global::net_thread_chunk == nullptr )
		LUA->ThrowError( "failed to locate net thread chunk" );

	ProtectMemory( global::net_thread_chunk, global::netpatch_len, false );
		memcpy( global::netpatch_old, global::net_thread_chunk, global::netpatch_len );
		memcpy( global::net_thread_chunk, global::netpatch_new, global::netpatch_len );
	ProtectMemory( global::net_thread_chunk, global::netpatch_len, true );

#endif

	NetMessage::PreInitialize( state );
	Hooks::PreInitialize( state );

	sn_bf_write::Initialize( state );
	sn_bf_read::Initialize( state );
	NetChannel::Initialize( state );
	subchannel::Initialize( state );
	dataFragments::Initialize( state );
	FileHandle::Initialize( state );
	UCHARPTR::Initialize( state );
	netadr::Initialize( state );
	NetChannelHandler::Initialize( state );
	NetworkStringTableContainer::Initialize( state );
	NetworkStringTable::Initialize( state );
	GameEventManager::Initialize( state );
	GameEvent::Initialize( state );
	NetMessage::Initialize( state );
	Hooks::Initialize( state );
	global::Initialize( state );

	return 0;
}

GMOD_MODULE_CLOSE( )
{
	if( !global::loaded )
		return 0;

#if defined SOURCENET_SERVER

	ProtectMemory( global::net_thread_chunk, global::netpatch_len, false );
		memcpy( global::net_thread_chunk, global::netpatch_old, global::netpatch_len );
	ProtectMemory( global::net_thread_chunk, global::netpatch_len, true );

#endif

	sn_bf_write::Deinitialize( state );
	sn_bf_read::Deinitialize( state );
	NetChannel::Deinitialize( state );
	subchannel::Deinitialize( state );
	dataFragments::Deinitialize( state );
	FileHandle::Deinitialize( state );
	UCHARPTR::Deinitialize( state );
	netadr::Deinitialize( state );
	NetChannelHandler::Deinitialize( state );
	NetworkStringTableContainer::Deinitialize( state );
	NetworkStringTable::Deinitialize( state );
	GameEventManager::Deinitialize( state );
	GameEvent::Deinitialize( state );
	NetMessage::Deinitialize( state );
	Hooks::Deinitialize( state );
	global::Deinitialize( state );

	return 0;
}
