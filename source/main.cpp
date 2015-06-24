#include <main.hpp>
#include <net.hpp>
#include <protocol.hpp>
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
#include <interface.h>
#include <eiface.h>
#include <cdll_int.h>
#include <iserver.h>
#include <symbolfinder.hpp>

#if defined _WIN32

#undef INVALID_HANDLE_VALUE

#include <windows.h>

namespace Global
{

inline void ProtectMemory( void *addr, size_t size, bool protect )
{
	static DWORD previous = 0;
	VirtualProtect( addr, size, protect ? previous : PAGE_EXECUTE_READWRITE, &previous );
}

static const char *IServer_sig = "\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\xD8\x6D\x24\x83\x4D\xEC\x10";
static const size_t IServer_siglen = 16;

static const size_t netpatch_len = 6;
static char netpatch_old[netpatch_len] = { 0 };
static const char *netpatch_new = "\xE9\x2B\x00\x00\x00\x90";

static const char *netchunk_sig = "\x74\x2A\x85\xDB\x74\x2A\x8B\x43\x0C\x83\xC0\x07\xC1\xF8\x03\x85";
static const size_t netchunk_siglen = 16;

#elif defined __linux

#include <sys/mman.h>
#include <unistd.h>

namespace Global
{

inline void *PageAlign( void *addr, long page )
{
	uintptr_t uaddr = reinterpret_cast<uintptr_t>( addr );
	return reinterpret_cast<void *>( uaddr - uaddr % page );
}

inline void ProtectMemory( void *addr, size_t size, bool protect )
{
	long page = sysconf( _SC_PAGESIZE );
	mprotect( Global::PageAlign( addr, page ), page, ( protect ? 0 : PROT_WRITE ) | PROT_EXEC | PROT_READ );
}

#if defined SOURCENET_SERVER

static const char *IServer_sig = "@sv";
static const size_t IServer_siglen = 0;

#elif defined SOURCENET_CLIENT

static const char *IServer_sig = "\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x8D\x5D\x80\xC7";
static const size_t IServer_siglen = 13;

#endif

static const size_t netpatch_len = 6;
static char netpatch_old[netpatch_len] = { 0 };
static const char *netpatch_new = "\xE9\x01\x00\x00\x00\x90";

static const char *netchunk_sig = "\x74\x2A\x85\xFF\x74\x2A\x8B\x47\x0C\x83\xC0\x07\xC1\xF8\x03\x85";
static const size_t netchunk_siglen = 16;

#elif defined __APPLE__

#include <sys/mman.h>
#include <unistd.h>

namespace Global
{

inline void *PageAlign( void *addr, long page )
{
	uintptr_t uaddr = reinterpret_cast<uintptr_t>( addr );
	return reinterpret_cast<void *>( uaddr - uaddr % page );
}

inline void ProtectMemory( void *addr, size_t size, bool protect )
{
	long page = sysconf( _SC_PAGESIZE );
	mprotect( Global::PageAlign( addr, page ), page, ( protect ? 0 : PROT_WRITE ) | PROT_EXEC | PROT_READ );
}

static const char *IServer_sig = "\x2A\x2A\x2A\x2A\x8B\x08\x89\x04\x24\xFF\x51\x28\xD9\x9D\x9C\xFE";
static const size_t IServer_siglen = 16;

static const size_t netpatch_len = 6;
static char netpatch_old[netpatch_len] = { 0 };
static const char *netpatch_new = "\xE9\x01\x00\x00\x00\x90";

static const char *netchunk_sig = "\x74\x2A\x85\xD2\x74\x2A\x8B\x42\x0C\x83\xC0\x07\xC1\xf8\x03\x85";
static const size_t netchunk_siglen = 16;

#endif

lua_State *lua_state = nullptr;

static CDllDemandLoader engine_loader( engine_lib );
static uint8_t *net_thread_chunk = nullptr;
CreateInterfaceFn engine_factory = nullptr;

IVEngineServer *engine_server = nullptr;
IVEngineClient *engine_client = nullptr;
IServer *server = nullptr;

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

void PushAngle( lua_State *state, const QAngle &ang )
{
	QAngle *angle = new( std::nothrow ) QAngle( ang );
	if( angle == nullptr )
		LUA->ThrowError( "failed to allocate Angle" );

	GarrysMod::Lua::UserData *userdata = static_cast<GarrysMod::Lua::UserData *>(
		LUA->NewUserdata( sizeof( GarrysMod::Lua::UserData ) )
	);
	userdata->type = GarrysMod::Lua::Type::VECTOR;
	userdata->data = angle;

	LUA->CreateMetaTableType( "Angle", GarrysMod::Lua::Type::ANGLE );
	LUA->SetMetaTable( -2 );
}

void PushVector( lua_State *state, const Vector &vec )
{
	Vector *vector = new( std::nothrow ) Vector( vec );
	if( vector == nullptr )
		LUA->ThrowError( "failed to allocate Vector" );

	GarrysMod::Lua::UserData *userdata = static_cast<GarrysMod::Lua::UserData *>(
		LUA->NewUserdata( sizeof( GarrysMod::Lua::UserData ) )
	);
	userdata->type = GarrysMod::Lua::Type::VECTOR;
	userdata->data = vector;

	LUA->CreateMetaTableType( "Vector", GarrysMod::Lua::Type::VECTOR );
	LUA->SetMetaTable( -2 );
}

}

GMOD_MODULE_OPEN( )
{
	Global::lua_state = state;

	Global::engine_factory = Global::engine_loader.GetFactory( );
	if( Global::engine_factory == nullptr )
		LUA->ThrowError( "failed to retrieve engine factory function" );

	Global::engine_server = static_cast<IVEngineServer *>(
		Global::engine_factory( "VEngineServer021", nullptr )
	);
	if( Global::engine_server == nullptr )
		LUA->ThrowError( "failed to retrieve server engine interface" );

	if( !Global::engine_server->IsDedicatedServer( ) )
	{
		Global::engine_client = static_cast<IVEngineClient *>(
			Global::engine_factory( "VEngineClient015", nullptr )
		);
		if( Global::engine_client == nullptr )
			LUA->ThrowError( "failed to retrieve client engine interface" );
	}

	SymbolFinder symfinder;

	IServer **pserver = reinterpret_cast<IServer **>( symfinder.ResolveOnBinary(
		Global::engine_lib, Global::IServer_sig, Global::IServer_siglen
	) );
	if( pserver == nullptr )
		LUA->ThrowError( "failed to locate IServer pointer" );

	Global::server = *pserver;
	if( Global::server == nullptr )
		LUA->ThrowError( "failed to locate IServer" );

#if defined SOURCENET_SERVER

	// Disables per-client threads (hacky fix for SendDatagram hooking)

	Global::net_thread_chunk = static_cast<uint8_t *>( symfinder.ResolveOnBinary(
		Global::engine_lib, Global::netchunk_sig, Global::netchunk_siglen
	) );
	if( Global::net_thread_chunk == nullptr )
		LUA->ThrowError( "failed to locate net thread chunk" );

	Global::ProtectMemory( Global::net_thread_chunk, Global::netpatch_len, false );
		memcpy( Global::netpatch_old, Global::net_thread_chunk, Global::netpatch_len );
		memcpy( Global::net_thread_chunk, Global::netpatch_new, Global::netpatch_len );
	Global::ProtectMemory( Global::net_thread_chunk, Global::netpatch_len, true );

#endif

	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );

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

	return 0;
}

GMOD_MODULE_CLOSE( )
{
	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_REG );

	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );

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

#if defined SOURCENET_SERVER

	Global::ProtectMemory( Global::net_thread_chunk, Global::netpatch_len, false );
		memcpy( Global::net_thread_chunk, Global::netpatch_old, Global::netpatch_len );
	Global::ProtectMemory( Global::net_thread_chunk, Global::netpatch_len, true );

#endif

	return 0;
}