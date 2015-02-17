#define IVENGINESERVER_INTERFACE
#define IVENGINECLIENT_INTERFACE

#include <main.hpp>
#include <net.h>
#include <protocol.h>
#include <gl_hooks.hpp>
#include <gl_bitbuf_write.hpp>
#include <gl_bitbuf_read.hpp>
#include <gl_cnetchan.hpp>
#include <gl_inetchannelhandler.hpp>
#include <gl_subchannel_t.hpp>
#include <gl_datafragments_t.hpp>
#include <gl_filehandle_t.hpp>
#include <gl_ucharptr.hpp>
#include <gl_netadr_t.hpp>
#include <gl_inetworkstringtablecontainer.hpp>
#include <gl_inetworkstringtable.hpp>
#include <gl_igameeventmanager2.hpp>
#include <gl_igameevent.hpp>
#include <symbolfinder.hpp>

extern "C"
{

#include <lauxlib.h>

}

#if defined _WIN32

#undef INVALID_HANDLE_VALUE

#include <windows.h>

#undef CreateEvent

namespace Global
{

#define BEGIN_MEMEDIT( addr, size ) \
{ \
	DWORD previous = 0; \
	VirtualProtect( addr, \
			size, \
			PAGE_EXECUTE_READWRITE, \
			&previous )

#define FINISH_MEMEDIT( addr, size ) \
	VirtualProtect( addr, \
			size, \
			previous, \
			nullptr ); \
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

inline uint8_t *PageAlign( uint8_t *addr, long page )
{
	return addr - ( reinterpret_cast<uintptr_t>( addr ) % page );
}

#define BEGIN_MEMEDIT( addr, size ) \
{ \
	long page = sysconf( _SC_PAGESIZE ); \
	mprotect( PageAlign( addr, page ), \
			page, PROT_EXEC | PROT_READ | PROT_WRITE )

#define FINISH_MEMEDIT( addr, size ) \
	mprotect( PageAlign( addr, page ), \
			page, PROT_EXEC | PROT_READ ); \
}

static const char *IServer_sig = "\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x8B\x7D\x88\xC7\x85\x78\xFF";
static const size_t IServer_siglen = 16;

static const size_t netpatch_len = 6;
static char netpatch_old[netpatch_len] = { 0 };
static const char *netpatch_new = "\xE9\x01\x00\x00\x00\90";

static const char *netchunk_sig = "\x74\x2A\x85\xFF\x74\x2A\x8B\x47\x0C\x83\xC0\x07\xC1\xF8\x03\x85";
static const size_t netchunk_siglen = 16;

#elif defined __APPLE__

#include <sys/mman.h>
#include <unistd.h>

namespace Global
{

inline uint8_t *PageAlign( uint8_t *addr, long page )
{
	return addr - ( reinterpret_cast<uintptr_t>( addr ) % page );
}

#define BEGIN_MEMEDIT( addr, size ) \
{ \
	long page = sysconf( _SC_PAGESIZE ); \
	mprotect( PageAlign( addr, page ), \
			page, PROT_EXEC | PROT_READ | PROT_WRITE )

#define FINISH_MEMEDIT( addr, size ) \
	mprotect( PageAlign( addr, page ), \
			page, PROT_EXEC | PROT_READ ); \
}

static const char *IServer_sig = "\x2A\x2A\x2A\x2A\x8B\x08\x89\x04\x24\xFF\x51\x28\xD9\x9D\x9C\xFE";
static const size_t IServer_siglen = 16;

static const size_t netpatch_len = 6;
static char netpatch_old[netpatch_len] = { 0 };
static const char *netpatch_new = "\xE9\x01\x00\x00\x00\90";

static const char *netchunk_sig = "\x74\x2A\x85\xD2\x74\x2A\x8B\x42\x0C\x83\xC0\x07\xC1\xf8\x03\x85";
static const size_t netchunk_siglen = 16;

#endif

lua_State *lua_state = nullptr;

static CDllDemandLoader engine_loader( engine_lib );
static void *net_thread_chunk = nullptr;
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

void CheckType( lua_State *state, int32_t index, int32_t type, const char *nametype )
{
	if( !LUA->IsType( index, type ) )
		luaL_typerror( state, index, nametype );
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

	IServer **pserver = reinterpret_cast<IServer **>(
		symfinder.ResolveOnBinary( Global::engine_lib, Global::IServer_sig, Global::IServer_siglen )
	);
	if( pserver == nullptr )
		LUA->ThrowError( "failed to locate IServer pointer" );

	Global::server = *pserver;
	if( Global::server == nullptr )
		LUA->ThrowError( "failed to locate IServer" );
	
#if defined SOURCENET_SERVER

	// Disables per-client threads (hacky fix for SendDatagram hooking)

	Global::net_thread_chunk = symfinder.ResolveOnBinary(
		Global::engine_lib, Global::netchunk_sig, Global::netchunk_siglen
	);
	if( Global::net_thread_chunk == nullptr )
		LUA->ThrowError( "failed to locate net thread chunk" );

	BEGIN_MEMEDIT( Global::net_thread_chunk, Global::netpatch_len );
		memcpy( Global::netpatch_old, Global::net_thread_chunk, Global::netpatch_len );
		memcpy( Global::net_thread_chunk, Global::netpatch_new, Global::netpatch_len );
	FINISH_MEMEDIT( Global::net_thread_chunk, Global::netpatch_len );

#endif

	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );

		LUA->CreateTable( );

			LUA->PushNumber( UpdateType::EnterPVS );
			LUA->SetField( -2, "EnterPVS" );

			LUA->PushNumber( UpdateType::LeavePVS );
			LUA->SetField( -2, "LeavePVS" );

			LUA->PushNumber( UpdateType::DeltaEnt );
			LUA->SetField( -2, "DeltaEnt" );

			LUA->PushNumber( UpdateType::PreserveEnt );
			LUA->SetField( -2, "PreserveEnt" );

			LUA->PushNumber( UpdateType::Finished );
			LUA->SetField( -2, "Finished" );

			LUA->PushNumber( UpdateType::Failed );
			LUA->SetField( -2, "Failed" );

		LUA->SetField( -2, "UpdateType" );



		LUA->PushNumber( FHDR_ZERO );
		LUA->SetField( -2, "FHDR_ZERO" );

		LUA->PushNumber( FHDR_LEAVEPVS );
		LUA->SetField( -2, "FHDR_LEAVEPVS" );

		LUA->PushNumber( FHDR_DELETE );
		LUA->SetField( -2, "FHDR_DELETE" );

		LUA->PushNumber( FHDR_ENTERPVS );
		LUA->SetField( -2, "FHDR_ENTERPVS" );



		LUA->PushString( INSTANCE_BASELINE_TABLENAME );
		LUA->SetField( -2, "INSTANCE_BASELINE_TABLENAME" );

		LUA->PushString( LIGHT_STYLES_TABLENAME );
		LUA->SetField( -2, "LIGHT_STYLES_TABLENAME" );

		LUA->PushString( USER_INFO_TABLENAME );
		LUA->SetField( -2, "USER_INFO_TABLENAME" );

		LUA->PushString( SERVER_STARTUP_DATA_TABLENAME );
		LUA->SetField( -2, "SERVER_STARTUP_DATA_TABLENAME" );

		LUA->PushNumber( NET_MESSAGE_BITS );
		LUA->SetField( -2, "NET_MESSAGE_BITS" );



		LUA->PushNumber( net_NOP );
		LUA->SetField( -2, "net_NOP" );

		LUA->PushNumber( net_Disconnect );
		LUA->SetField( -2, "net_Disconnect" );

		LUA->PushNumber( net_File );
		LUA->SetField( -2, "net_File" );

		LUA->PushNumber( net_LastControlMessage );
		LUA->SetField( -2, "net_LastControlMessage" );


		LUA->PushNumber( net_Tick );
		LUA->SetField( -2, "net_Tick" );

		LUA->PushNumber( net_StringCmd );
		LUA->SetField( -2, "net_StringCmd" );

		LUA->PushNumber( net_SetConVar );
		LUA->SetField( -2, "net_SetConVar" );

		LUA->PushNumber( net_SignonState );
		LUA->SetField( -2, "net_SignonState" );


		LUA->PushNumber( svc_ServerInfo );
		LUA->SetField( -2, "svc_ServerInfo" );

		LUA->PushNumber( svc_SendTable );
		LUA->SetField( -2, "svc_SendTable" );

		LUA->PushNumber( svc_ClassInfo );
		LUA->SetField( -2, "svc_ClassInfo" );

		LUA->PushNumber( svc_SetPause );
		LUA->SetField( -2, "svc_SetPause" );

		LUA->PushNumber( svc_CreateStringTable );
		LUA->SetField( -2, "svc_CreateStringTable" );

		LUA->PushNumber( svc_UpdateStringTable );
		LUA->SetField( -2, "svc_UpdateStringTable" );

		LUA->PushNumber( svc_VoiceInit );
		LUA->SetField( -2, "svc_VoiceInit" );

		LUA->PushNumber( svc_VoiceData );
		LUA->SetField( -2, "svc_VoiceData" );

		LUA->PushNumber( svc_Print );
		LUA->SetField( -2, "svc_Print" );

		LUA->PushNumber( svc_Sounds );
		LUA->SetField( -2, "svc_Sounds" );

		LUA->PushNumber( svc_SetView );
		LUA->SetField( -2, "svc_SetView" );

		LUA->PushNumber( svc_FixAngle );
		LUA->SetField( -2, "svc_FixAngle" );

		LUA->PushNumber( svc_CrosshairAngle );
		LUA->SetField( -2, "svc_CrosshairAngle" );

		LUA->PushNumber( svc_BSPDecal );
		LUA->SetField( -2, "svc_BSPDecal" );

		LUA->PushNumber( svc_UserMessage );
		LUA->SetField( -2, "svc_UserMessage" );

		LUA->PushNumber( svc_EntityMessage );
		LUA->SetField( -2, "svc_EntityMessage" );

		LUA->PushNumber( svc_GameEvent );
		LUA->SetField( -2, "svc_GameEvent" );

		LUA->PushNumber( svc_PacketEntities );
		LUA->SetField( -2, "svc_PacketEntities" );

		LUA->PushNumber( svc_TempEntities );
		LUA->SetField( -2, "svc_TempEntities" );

		LUA->PushNumber( svc_Prefetch );
		LUA->SetField( -2, "svc_Prefetch" );

		LUA->PushNumber( svc_Menu );
		LUA->SetField( -2, "svc_Menu" );

		LUA->PushNumber( svc_GameEventList );
		LUA->SetField( -2, "svc_GameEventList" );

		LUA->PushNumber( svc_GetCvarValue );
		LUA->SetField( -2, "svc_GetCvarValue" );

		LUA->PushNumber( svc_CmdKeyValues );
		LUA->SetField( -2, "svc_CmdKeyValues" );

		LUA->PushNumber( svc_GMod_ServerToClient );
		LUA->SetField( -2, "svc_GMod_ServerToClient" );

		LUA->PushNumber( SVC_LASTMSG );
		LUA->SetField( -2, "SVC_LASTMSG" );


		LUA->PushNumber( clc_ClientInfo );
		LUA->SetField( -2, "clc_ClientInfo" );

		LUA->PushNumber( clc_Move );
		LUA->SetField( -2, "clc_Move" );

		LUA->PushNumber( clc_VoiceData );
		LUA->SetField( -2, "clc_VoiceData" );

		LUA->PushNumber( clc_BaselineAck );
		LUA->SetField( -2, "clc_BaselineAck" );

		LUA->PushNumber( clc_ListenEvents );
		LUA->SetField( -2, "clc_ListenEvents" );

		LUA->PushNumber( clc_RespondCvarValue );
		LUA->SetField( -2, "clc_RespondCvarValue" );

		LUA->PushNumber( clc_FileCRCCheck );
		LUA->SetField( -2, "clc_FileCRCCheck" );

		LUA->PushNumber( clc_CmdKeyValues );
		LUA->SetField( -2, "clc_CmdKeyValues" );

		LUA->PushNumber( clc_FileMD5Check );
		LUA->SetField( -2, "clc_FileMD5Check" );

		LUA->PushNumber( clc_GMod_ClientToServer );
		LUA->SetField( -2, "clc_GMod_ClientToServer" );

		LUA->PushNumber( CLC_LASTMSG );
		LUA->SetField( -2, "CLC_LASTMSG" );



		LUA->PushNumber( RES_FATALIFMISSING );
		LUA->SetField( -2, "RES_FATALIFMISSING" );

		LUA->PushNumber( RES_PRELOAD );
		LUA->SetField( -2, "RES_PRELOAD" );



		LUA->PushNumber( SIGNONSTATE_NONE );
		LUA->SetField( -2, "SIGNONSTATE_NONE" );

		LUA->PushNumber( SIGNONSTATE_CHALLENGE );
		LUA->SetField( -2, "SIGNONSTATE_CHALLENGE" );

		LUA->PushNumber( SIGNONSTATE_CONNECTED );
		LUA->SetField( -2, "SIGNONSTATE_CONNECTED" );

		LUA->PushNumber( SIGNONSTATE_NEW );
		LUA->SetField( -2, "SIGNONSTATE_NEW" );

		LUA->PushNumber( SIGNONSTATE_PRESPAWN );
		LUA->SetField( -2, "SIGNONSTATE_PRESPAWN" );

		LUA->PushNumber( SIGNONSTATE_SPAWN );
		LUA->SetField( -2, "SIGNONSTATE_SPAWN" );

		LUA->PushNumber( SIGNONSTATE_FULL );
		LUA->SetField( -2, "SIGNONSTATE_FULL" );

		LUA->PushNumber( SIGNONSTATE_CHANGELEVEL );
		LUA->SetField( -2, "SIGNONSTATE_CHANGELEVEL" );



		LUA->PushNumber( MAX_STREAMS );
		LUA->SetField( -2, "MAX_STREAMS" );

		LUA->PushNumber( FRAG_NORMAL_STREAM );
		LUA->SetField( -2, "FRAG_NORMAL_STREAM" );

		LUA->PushNumber( FRAG_FILE_STREAM );
		LUA->SetField( -2, "FRAG_FILE_STREAM" );



		LUA->PushNumber( MAX_RATE );
		LUA->SetField( -2, "MAX_RATE" );

		LUA->PushNumber( MIN_RATE );
		LUA->SetField( -2, "MIN_RATE" );

		LUA->PushNumber( DEFAULT_RATE );
		LUA->SetField( -2, "DEFAULT_RATE" );



		LUA->PushNumber( MAX_FRAGMENT_SIZE );
		LUA->SetField( -2, "MAX_FRAGMENT_SIZE" );

		LUA->PushNumber( MAX_SUBCHANNELS );
		LUA->SetField( -2, "MAX_SUBCHANNELS" );

		LUA->PushNumber( MAX_FILE_SIZE );
		LUA->SetField( -2, "MAX_FILE_SIZE" );



		LUA->PushNumber( FLOW_OUTGOING );
		LUA->SetField( -2, "FLOW_OUTGOING" );

		LUA->PushNumber( FLOW_INCOMING );
		LUA->SetField( -2, "FLOW_INCOMING" );

		LUA->PushNumber( MAX_FLOWS );
		LUA->SetField( -2, "MAX_FLOWS" );



		LUA->PushNumber( MAX_CUSTOM_FILES );
		LUA->SetField( -2, "MAX_CUSTOM_FILES" );

	LUA->Pop( 1 );

	sn4_bf_write::Initialize( state );

	sn4_bf_read::Initialize( state );

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

	Hooks::Initialize( state );

	return 0;
}

GMOD_MODULE_CLOSE( )
{
	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );

		LUA->PushNil( );
		LUA->SetField( -2, "UpdateType" );



		LUA->PushNil( );
		LUA->SetField( -2, "FHDR_ZERO" );

		LUA->PushNil( );
		LUA->SetField( -2, "FHDR_LEAVEPVS" );

		LUA->PushNil( );
		LUA->SetField( -2, "FHDR_DELETE" );

		LUA->PushNil( );
		LUA->SetField( -2, "FHDR_ENTERPVS" );



		LUA->PushNil( );
		LUA->SetField( -2, "INSTANCE_BASELINE_TABLENAME" );

		LUA->PushNil( );
		LUA->SetField( -2, "LIGHT_STYLES_TABLENAME" );

		LUA->PushNil( );
		LUA->SetField( -2, "USER_INFO_TABLENAME" );

		LUA->PushNil( );
		LUA->SetField( -2, "SERVER_STARTUP_DATA_TABLENAME" );

		LUA->PushNil( );
		LUA->SetField( -2, "NET_MESSAGE_BITS" );



		LUA->PushNil( );
		LUA->SetField( -2, "net_NOP" );

		LUA->PushNil( );
		LUA->SetField( -2, "net_Disconnect" );

		LUA->PushNil( );
		LUA->SetField( -2, "net_File" );

		LUA->PushNil( );
		LUA->SetField( -2, "net_LastControlMessage" );


		LUA->PushNil( );
		LUA->SetField( -2, "net_Tick" );

		LUA->PushNil( );
		LUA->SetField( -2, "net_StringCmd" );

		LUA->PushNil( );
		LUA->SetField( -2, "net_SetConVar" );

		LUA->PushNil( );
		LUA->SetField( -2, "net_SignonState" );


		LUA->PushNil( );
		LUA->SetField( -2, "svc_ServerInfo" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_SendTable" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_ClassInfo" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_SetPause" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_CreateStringTable" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_UpdateStringTable" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_VoiceInit" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_VoiceData" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_Print" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_Sounds" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_SetView" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_FixAngle" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_CrosshairAngle" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_BSPDecal" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_UserMessage" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_EntityMessage" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_GameEvent" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_PacketEntities" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_TempEntities" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_Prefetch" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_Menu" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_GameEventList" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_GetCvarValue" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_CmdKeyValues" );

		LUA->PushNil( );
		LUA->SetField( -2, "svc_GMod_ServerToClient" );

		LUA->PushNil( );
		LUA->SetField( -2, "SVC_LASTMSG" );


		LUA->PushNil( );
		LUA->SetField( -2, "clc_ClientInfo" );

		LUA->PushNil( );
		LUA->SetField( -2, "clc_Move" );

		LUA->PushNil( );
		LUA->SetField( -2, "clc_VoiceData" );

		LUA->PushNil( );
		LUA->SetField( -2, "clc_BaselineAck" );

		LUA->PushNil( );
		LUA->SetField( -2, "clc_ListenEvents" );

		LUA->PushNil( );
		LUA->SetField( -2, "clc_RespondCvarValue" );

		LUA->PushNil( );
		LUA->SetField( -2, "clc_FileCRCCheck" );

		LUA->PushNil( );
		LUA->SetField( -2, "clc_CmdKeyValues" );

		LUA->PushNil( );
		LUA->SetField( -2, "clc_FileMD5Check" );

		LUA->PushNil( );
		LUA->SetField( -2, "clc_GMod_ClientToServer" );

		LUA->PushNil( );
		LUA->SetField( -2, "CLC_LASTMSG" );



		LUA->PushNil( );
		LUA->SetField( -2, "RES_FATALIFMISSING" );

		LUA->PushNil( );
		LUA->SetField( -2, "RES_PRELOAD" );



		LUA->PushNil( );
		LUA->SetField( -2, "SIGNONSTATE_NONE" );

		LUA->PushNil( );
		LUA->SetField( -2, "SIGNONSTATE_CHALLENGE" );

		LUA->PushNil( );
		LUA->SetField( -2, "SIGNONSTATE_CONNECTED" );

		LUA->PushNil( );
		LUA->SetField( -2, "SIGNONSTATE_NEW" );

		LUA->PushNil( );
		LUA->SetField( -2, "SIGNONSTATE_PRESPAWN" );

		LUA->PushNil( );
		LUA->SetField( -2, "SIGNONSTATE_SPAWN" );

		LUA->PushNil( );
		LUA->SetField( -2, "SIGNONSTATE_FULL" );

		LUA->PushNil( );
		LUA->SetField( -2, "SIGNONSTATE_CHANGELEVEL" );



		LUA->PushNil( );
		LUA->SetField( -2, "MAX_STREAMS" );

		LUA->PushNil( );
		LUA->SetField( -2, "FRAG_NORMAL_STREAM" );

		LUA->PushNil( );
		LUA->SetField( -2, "FRAG_FILE_STREAM" );



		LUA->PushNil( );
		LUA->SetField( -2, "MAX_RATE" );

		LUA->PushNil( );
		LUA->SetField( -2, "MIN_RATE" );

		LUA->PushNil( );
		LUA->SetField( -2, "DEFAULT_RATE" );



		LUA->PushNil( );
		LUA->SetField( -2, "MAX_FRAGMENT_SIZE" );

		LUA->PushNil( );
		LUA->SetField( -2, "MAX_SUBCHANNELS" );

		LUA->PushNil( );
		LUA->SetField( -2, "MAX_FILE_SIZE" );



		LUA->PushNil( );
		LUA->SetField( -2, "FLOW_OUTGOING" );

		LUA->PushNil( );
		LUA->SetField( -2, "FLOW_INCOMING" );

		LUA->PushNil( );
		LUA->SetField( -2, "MAX_FLOWS" );



		LUA->PushNil( );
		LUA->SetField( -2, "MAX_CUSTOM_FILES" );

	LUA->Pop( 1 );

	sn4_bf_write::Deinitialize( state );

	sn4_bf_read::Deinitialize( state );

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

	Hooks::Deinitialize( state );

#if defined SOURCENET_SERVER

	BEGIN_MEMEDIT( Global::net_thread_chunk, Global::netpatch_len );
		memcpy( Global::net_thread_chunk, Global::netpatch_old, Global::netpatch_len );
	FINISH_MEMEDIT( Global::net_thread_chunk, Global::netpatch_len );

#endif

	return 0;
}