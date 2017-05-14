#include <detours.h>
#include <vfnhook.h>
#include <hooks.hpp>
#include <netchannel.hpp>
#include <netchannelhandler.hpp>
#include <netmessage.hpp>
#include <sn_bf_read.hpp>
#include <sn_bf_write.hpp>
#include <net.hpp>
#include <inetmsghandler.h>
#include <cdll_int.h>
#include <symbolfinder.hpp>
#include <GarrysMod/Helpers.hpp>

namespace Hooks
{

#if defined _WIN32

typedef bool ( __thiscall *CNetChan_ProcessMessages_t )( CNetChan *netchan, bf_read &buf );

static const char *CNetChan_ProcessMessages_sig = "\x55\x8B\xEC\x83\xEC\x2C\x53\x89\x4D\xFC";
static const size_t CNetChan_ProcessMessages_siglen = 10;

#elif defined __linux

typedef bool ( *CNetChan_ProcessMessages_t )( CNetChan *netchan, bf_read &buf );

#if defined SOURCENET_SERVER

static const char *CNetChan_ProcessMessages_sig = "@_ZN8CNetChan15ProcessMessagesER7bf_read";
static const size_t CNetChan_ProcessMessages_siglen = 0;

#elif defined SOURCENET_CLIENT

static const char *CNetChan_ProcessMessages_sig =
	"\x55\x89\xE5\x57\x56\x53\x83\xEC\x6C\x8B\x35\x2A\x2A\x2A\x2A\x8B";
static const size_t CNetChan_ProcessMessages_siglen = 16;

#endif

#elif defined __APPLE__

typedef bool ( *CNetChan_ProcessMessages_t )( CNetChan *netchan, bf_read &buf );

static const char *CNetChan_ProcessMessages_sig = "@__ZN8CNetChan15ProcessMessagesER7bf_read";
static const size_t CNetChan_ProcessMessages_siglen = 0;

#endif

static CNetChan_ProcessMessages_t CNetChan_ProcessMessages_o = nullptr;

static uintptr_t CNetChan_vtable = 0;
static uintptr_t INetChannelHandler_vtable = 0;

#define HOOK_INIT( name ) \
do \
{ \
	GarrysMod::Lua::ILuaBase *LUA = global::lua; \
	if( !helpers::PushHookRun( LUA, name ) ) \
		break; \
	int32_t _argc = 0

#define HOOK_PUSH( code ) \
	code; \
	++_argc

#define HOOK_CALL( returns ) \
	helpers::CallHookRun( LUA, _argc, returns )

#define HOOK_END( ) \
} \
while( false )

#define MONITOR_HOOK( name ) \
	static bool attach_status__##name = false

#define IS_ATTACHED( name ) \
	attach_status__##name

#define REGISTER_ATTACH( name ) \
	attach_status__##name = true

#define REGISTER_DETACH( name ) \
	attach_status__##name = false

#define SIMPLE_VHOOK_BINDING( meta, namespc, name, offset ) \
	MONITOR_HOOK( name ); \
	LUA_FUNCTION_STATIC( Attach__##name ) \
	{ \
		if( !IS_ATTACHED( name ) ) \
		{ \
			meta *temp = namespc::Get( LUA, 1 ); \
			meta##_vtable = VTBL( temp ); \
			HOOKVFUNC( temp, offset, name##_o, name##_d ); \
			REGISTER_ATTACH( name ); \
		} \
		return 0; \
	} \
	LUA_FUNCTION_STATIC( Detach__##name ) \
	{ \
		if( IS_ATTACHED( name ) ) \
		{ \
			UNHOOKVFUNC( &meta##_vtable, offset, name##_o ); \
			REGISTER_DETACH( name ); \
		} \
		return 0; \
	}

DEFVFUNC_( CNetChan_SendDatagram_o, int32_t, ( CNetChan *netchan, bf_write *data ) );

int32_t VFUNC CNetChan_SendDatagram_d( CNetChan *netchan, bf_write *data )
{
	HOOK_INIT( "PreSendDatagram" );
		HOOK_PUSH( NetChannel::Push( LUA, netchan ) );

		if( !global::is_dedicated )
		{
			HOOK_PUSH( NetChannel::Push(
				LUA,
				static_cast<CNetChan *>( global::engine_client->GetNetChannelInfo( ) )
			) );
		}
		else
		{
			HOOK_PUSH( LUA->PushNil( ) );
		}

		HOOK_PUSH( bf_write **writer1 = sn_bf_write::Push( LUA, data ) );
		HOOK_PUSH( bf_write **writer2 = sn_bf_write::Push( LUA, &netchan->m_StreamReliable ) );
		HOOK_PUSH( bf_write **writer3 = sn_bf_write::Push( LUA, &netchan->m_StreamUnreliable ) );
		HOOK_PUSH( bf_write **writer4 = sn_bf_write::Push( LUA, &netchan->m_StreamVoice ) );
		HOOK_CALL( 0 );

		*writer1 = nullptr;
		*writer2 = nullptr;
		*writer3 = nullptr;
		*writer4 = nullptr;
	HOOK_END( );

	int32_t r = CNetChan_SendDatagram_o( netchan, data );

	HOOK_INIT( "PostSendDatagram" );
		HOOK_PUSH( NetChannel::Push( LUA, netchan ) );
		HOOK_CALL( 0 );
	HOOK_END( );

	return r;
}

#if defined _WIN32

SIMPLE_VHOOK_BINDING( CNetChan, NetChannel, CNetChan_SendDatagram, 46 );

#elif defined __linux || defined __APPLE__

SIMPLE_VHOOK_BINDING( CNetChan, NetChannel, CNetChan_SendDatagram, 47 );

#endif

DEFVFUNC_( CNetChan_ProcessPacket_o, void, (
	CNetChan *netchan,
	netpacket_t *packet,
	bool bHasHeader
) );

void VFUNC CNetChan_ProcessPacket_d( CNetChan *netchan, netpacket_t *packet, bool bHasHeader )
{
	HOOK_INIT( "PreProcessPacket" );
		HOOK_PUSH( NetChannel::Push( LUA, netchan ) );
		HOOK_CALL( 0 );
	HOOK_END( );

	CNetChan_ProcessPacket_o( netchan, packet, bHasHeader );

	HOOK_INIT( "PostProcessPacket" );
		HOOK_PUSH( NetChannel::Push( LUA, netchan ) );
		HOOK_CALL( 0 );
	HOOK_END( );
}

#if defined _WIN32

SIMPLE_VHOOK_BINDING( CNetChan, NetChannel, CNetChan_ProcessPacket, 39 );

#elif defined __linux || defined __APPLE__

SIMPLE_VHOOK_BINDING( CNetChan, NetChannel, CNetChan_ProcessPacket, 40 );

#endif

DEFVFUNC_( CNetChan_Shutdown_o, void, ( CNetChan *netchan, const char *reason ) );

void VFUNC CNetChan_Shutdown_d( CNetChan *netchan, const char *reason )
{
	HOOK_INIT( "PreNetChannelShutdown" );
		HOOK_PUSH( NetChannel::Push( LUA, netchan ) );

		// Fuck Linux
		if( reason != nullptr && reason != reinterpret_cast<const char *>( 0x02 ) )
		{
			HOOK_PUSH( LUA->PushString( reason ) );
		}
		else
		{
			HOOK_PUSH( LUA->PushNil( ) );
		}

		HOOK_CALL( 0 );
	HOOK_END( );

	NetMessage::Destroy( global::lua, netchan );

	NetChannel::Destroy( global::lua, netchan );

	CNetChan_Shutdown_o( netchan, reason );

	HOOK_INIT( "PostNetChannelShutdown" );
		HOOK_CALL( 0 );
	HOOK_END( );
}

SIMPLE_VHOOK_BINDING( CNetChan, NetChannel, CNetChan_Shutdown, 36 );

DEFVFUNC_( INetChannelHandler_ConnectionStart_o, void, (
	INetChannelHandler *handler,
	CNetChan *netchan
) );

void VFUNC INetChannelHandler_ConnectionStart_d( INetChannelHandler *handler, CNetChan *netchan )
{
	HOOK_INIT( "INetChannelHandler::ConnectionStart" );
		HOOK_PUSH( NetChannelHandler::Push( LUA, handler ) );
		HOOK_PUSH( NetChannel::Push( LUA, netchan ) );
		HOOK_CALL( 0 );
	HOOK_END( );

	INetChannelHandler_ConnectionStart_o( handler, netchan );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, NetChannelHandler, INetChannelHandler_ConnectionStart, 1 );

DEFVFUNC_( INetChannelHandler_ConnectionClosing_o, void, (
	INetChannelHandler *handler,
	const char *reason
) );

void VFUNC INetChannelHandler_ConnectionClosing_d(
	INetChannelHandler *handler,
	const char *reason
)
{
	HOOK_INIT( "INetChannelHandler::ConnectionClosing" );
		HOOK_PUSH( NetChannelHandler::Push( LUA, handler ) );
		HOOK_PUSH( LUA->PushString( reason ) );
		HOOK_CALL( 0 );
	HOOK_END( );

	NetChannelHandler::Destroy( global::lua, handler );

	INetChannelHandler_ConnectionClosing_o( handler, reason );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, NetChannelHandler, INetChannelHandler_ConnectionClosing, 2 );

DEFVFUNC_( INetChannelHandler_ConnectionCrashed_o, void, (
	INetChannelHandler *handler,
	const char *reason
) );

void VFUNC INetChannelHandler_ConnectionCrashed_d(
	INetChannelHandler *handler,
	const char *reason
)
{
	HOOK_INIT( "INetChannelHandler::ConnectionCrashed" );
		HOOK_PUSH( NetChannelHandler::Push( LUA, handler ) );
		HOOK_PUSH( LUA->PushString( reason ) );
		HOOK_CALL( 0 );
	HOOK_END( );

	NetChannelHandler::Destroy( global::lua, handler );

	INetChannelHandler_ConnectionCrashed_o( handler, reason );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, NetChannelHandler, INetChannelHandler_ConnectionCrashed, 3 );

DEFVFUNC_( INetChannelHandler_PacketStart_o, void, (
	INetChannelHandler *handler,
	int32_t incoming_sequence,
	int32_t outgoing_acknowledged
) );

void VFUNC INetChannelHandler_PacketStart_d(
	INetChannelHandler *handler,
	int32_t incoming_sequence,
	int32_t outgoing_acknowledged
)
{
	HOOK_INIT( "INetChannelHandler::PacketStart" );
		HOOK_PUSH( NetChannelHandler::Push( LUA, handler ) );
		HOOK_PUSH( LUA->PushNumber( incoming_sequence ) );
		HOOK_PUSH( LUA->PushNumber( incoming_sequence ) );
		HOOK_CALL( 0 );
	HOOK_END( );

	INetChannelHandler_PacketStart_o( handler, incoming_sequence, outgoing_acknowledged );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, NetChannelHandler, INetChannelHandler_PacketStart, 4 );

DEFVFUNC_( INetChannelHandler_PacketEnd_o, void, ( INetChannelHandler *handler ) );

void VFUNC INetChannelHandler_PacketEnd_d( INetChannelHandler *handler )
{
	HOOK_INIT( "INetChannelHandler::PacketEnd" );
		HOOK_PUSH( NetChannelHandler::Push( LUA, handler ) );
		HOOK_CALL( 0 );
	HOOK_END( );

	INetChannelHandler_PacketEnd_o( handler );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, NetChannelHandler, INetChannelHandler_PacketEnd, 5 );

DEFVFUNC_( INetChannelHandler_FileRequested_o, void, (
	INetChannelHandler *handler,
	const char *fileName,
	uint32_t transferID
) );

void VFUNC INetChannelHandler_FileRequested_d(
	INetChannelHandler *handler,
	const char *fileName,
	uint32_t transferID
)
{
	HOOK_INIT( "INetChannelHandler::FileRequested" );
		HOOK_PUSH( NetChannelHandler::Push( LUA, handler ) );
		HOOK_PUSH( LUA->PushString( fileName ) );
		HOOK_PUSH( LUA->PushNumber( transferID ) );
		HOOK_CALL( 0 );
	HOOK_END( );

	INetChannelHandler_FileRequested_o( handler, fileName, transferID );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, NetChannelHandler, INetChannelHandler_FileRequested, 6 );

DEFVFUNC_( INetChannelHandler_FileReceived_o, void, (
	INetChannelHandler *handler,
	const char *fileName,
	uint32_t transferID
) );

void VFUNC INetChannelHandler_FileReceived_d(
	INetChannelHandler *handler,
	const char *fileName,
	uint32_t transferID
)
{
	HOOK_INIT( "INetChannelHandler::FileReceived" );
		HOOK_PUSH( NetChannelHandler::Push( LUA, handler ) );
		HOOK_PUSH( LUA->PushString( fileName ) );
		HOOK_PUSH( LUA->PushNumber( transferID ) );
		HOOK_CALL( 0 );
	HOOK_END( );

	INetChannelHandler_FileReceived_o( handler, fileName, transferID );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, NetChannelHandler, INetChannelHandler_FileReceived, 7 );

DEFVFUNC_( INetChannelHandler_FileDenied_o, void, (
	INetChannelHandler *handler,
	const char *fileName,
	uint32_t transferID
) );

void VFUNC INetChannelHandler_FileDenied_d(
	INetChannelHandler *handler,
	const char *fileName,
	uint32_t transferID
)
{
	HOOK_INIT( "INetChannelHandler::FileDenied" );
		HOOK_PUSH( NetChannelHandler::Push( LUA, handler ) );
		HOOK_PUSH( LUA->PushString( fileName ) );
		HOOK_PUSH( LUA->PushNumber( transferID ) );
		HOOK_CALL( 0 );
	HOOK_END( );

	INetChannelHandler_FileDenied_o( handler, fileName, transferID );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, NetChannelHandler, INetChannelHandler_FileDenied, 8 );

static MologieDetours::Detour<CNetChan_ProcessMessages_t> *
	CNetChan_ProcessMessages_detour = nullptr;

#if defined _WIN32

static bool __fastcall CNetChan_ProcessMessages_d( CNetChan *netchan, void *, bf_read &buf )

#elif defined __linux || defined __APPLE__

static bool CNetChan_ProcessMessages_d( CNetChan *netchan, bf_read &buf )

#endif

{
	static uint8_t data[100000] = { 0 };
	if( !buf.IsOverflowed( ) )
	{
		memcpy( data, buf.m_pData, buf.GetNumBytesRead( ) );

		int32_t bitsread = buf.GetNumBitsRead( );
		bf_write write( data, sizeof( data ) );
		write.SeekToBit( bitsread );

		HOOK_INIT( "PreProcessMessages" );
			HOOK_PUSH( NetChannel::Push( LUA, netchan ) );
			HOOK_PUSH( bf_read **reader = sn_bf_read::Push( LUA, &buf ) );
			HOOK_PUSH( bf_write **writer = sn_bf_write::Push( LUA, &write ) );

			if( !global::is_dedicated )
			{
				HOOK_PUSH( NetChannel::Push(
					LUA,
					static_cast<CNetChan *>( global::engine_client->GetNetChannelInfo( ) )
				) );
			}

			HOOK_CALL( 0 );

			*reader = nullptr;
			*writer = nullptr;
		HOOK_END( );

		buf.StartReading(
			write.GetBasePointer( ),
			write.GetNumBytesWritten( ),
			bitsread,
			write.GetNumBitsWritten( )
		);
	}

	return CNetChan_ProcessMessages_detour->GetOriginalFunction( )( netchan, buf );
}

MONITOR_HOOK( CNetChan_ProcessMessages );

LUA_FUNCTION_STATIC( Attach__CNetChan_ProcessMessages )
{
	if( !IS_ATTACHED( CNetChan_ProcessMessages ) )
	{
		CNetChan_ProcessMessages_detour =
			new( std::nothrow ) MologieDetours::Detour<CNetChan_ProcessMessages_t>(
				CNetChan_ProcessMessages_o,
				reinterpret_cast<CNetChan_ProcessMessages_t>( CNetChan_ProcessMessages_d )
			);
		if( CNetChan_ProcessMessages_detour == nullptr )
			LUA->ThrowError( "failed to detour CNetChan::ProcessMessages" );

		REGISTER_ATTACH( CNetChan_ProcessMessages );
	}

	return 0;
}

LUA_FUNCTION_STATIC( Detach__CNetChan_ProcessMessages )
{
	if( IS_ATTACHED( CNetChan_ProcessMessages ) )
	{
		delete CNetChan_ProcessMessages_detour;
		CNetChan_ProcessMessages_detour = nullptr;

		REGISTER_DETACH( CNetChan_ProcessMessages );
	}

	return 0;
}

LUA_FUNCTION_STATIC( Empty )
{
	return 0;
}

void PreInitialize( GarrysMod::Lua::ILuaBase *LUA )
{
	SymbolFinder symfinder;

	CNetChan_ProcessMessages_o =
		reinterpret_cast<CNetChan_ProcessMessages_t>( symfinder.ResolveOnBinary(
			global::engine_lib.c_str( ),
			CNetChan_ProcessMessages_sig,
			CNetChan_ProcessMessages_siglen
		) );
	if( CNetChan_ProcessMessages_o == nullptr )
		LUA->ThrowError( "failed to locate CNetChan::ProcessMessages" );
}

void Initialize( GarrysMod::Lua::ILuaBase *LUA )
{
	LUA->PushCFunction( Attach__CNetChan_ProcessPacket );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__CNetChan_ProcessPacket" );

	LUA->PushCFunction( Detach__CNetChan_ProcessPacket );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__CNetChan_ProcessPacket" );

	LUA->PushCFunction( Attach__CNetChan_SendDatagram );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__CNetChan_SendDatagram" );

	LUA->PushCFunction( Detach__CNetChan_SendDatagram );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__CNetChan_SendDatagram" );

	LUA->PushCFunction( Attach__CNetChan_Shutdown );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__CNetChan_Shutdown" );

	//LUA->PushCFunction( Detach__CNetChan_Shutdown );
	LUA->PushCFunction( Empty );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__CNetChan_Shutdown" );

	LUA->PushCFunction( Attach__INetChannelHandler_ConnectionStart );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_ConnectionStart" );

	LUA->PushCFunction( Detach__INetChannelHandler_ConnectionStart );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_ConnectionStart" );

	LUA->PushCFunction( Attach__INetChannelHandler_ConnectionClosing );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_ConnectionClosing" );

	//LUA->PushCFunction( Detach__INetChannelHandler_ConnectionClosing );
	LUA->PushCFunction( Empty );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_ConnectionClosing" );

	LUA->PushCFunction( Attach__INetChannelHandler_ConnectionCrashed );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_ConnectionCrashed" );

	//LUA->PushCFunction( Detach__INetChannelHandler_ConnectionCrashed );
	LUA->PushCFunction( Empty );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_ConnectionCrashed" );

	LUA->PushCFunction( Attach__INetChannelHandler_PacketStart );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_PacketStart" );

	LUA->PushCFunction( Detach__INetChannelHandler_PacketStart );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_PacketStart" );

	LUA->PushCFunction( Attach__INetChannelHandler_PacketEnd );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_PacketEnd" );

	LUA->PushCFunction( Detach__INetChannelHandler_PacketEnd );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_PacketEnd" );

	LUA->PushCFunction( Attach__CNetChan_ProcessPacket );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__CNetChan_ProcessPacket" );

	LUA->PushCFunction( Detach__CNetChan_ProcessPacket );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__CNetChan_ProcessPacket" );

	LUA->PushCFunction( Attach__INetChannelHandler_FileReceived );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_FileReceived" );

	LUA->PushCFunction( Detach__INetChannelHandler_FileReceived );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_FileReceived" );

	LUA->PushCFunction( Attach__INetChannelHandler_FileDenied );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_FileDenied" );

	LUA->PushCFunction( Detach__INetChannelHandler_FileDenied );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_FileDenied" );

	LUA->PushCFunction( Attach__CNetChan_ProcessMessages );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__CNetChan_ProcessMessages" );

	LUA->PushCFunction( Detach__CNetChan_ProcessMessages );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__CNetChan_ProcessMessages" );
}

void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
{
	Detach__CNetChan_SendDatagram( LUA->GetState() );
	Detach__CNetChan_ProcessPacket( LUA->GetState() );
	Detach__CNetChan_Shutdown( LUA->GetState() );

	Detach__INetChannelHandler_ConnectionStart( LUA->GetState() );
	Detach__INetChannelHandler_ConnectionClosing( LUA->GetState() );
	Detach__INetChannelHandler_ConnectionCrashed( LUA->GetState() );
	Detach__INetChannelHandler_PacketStart( LUA->GetState() );
	Detach__INetChannelHandler_PacketEnd( LUA->GetState() );
	Detach__INetChannelHandler_FileRequested( LUA->GetState() );
	Detach__INetChannelHandler_FileReceived( LUA->GetState() );
	Detach__INetChannelHandler_FileDenied( LUA->GetState() );

	Detach__CNetChan_ProcessMessages( LUA->GetState() );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__CNetChan_ProcessPacket" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__CNetChan_ProcessPacket" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__CNetChan_SendDatagram" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__CNetChan_SendDatagram" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__CNetChan_Shutdown" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__CNetChan_Shutdown" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_ConnectionStart" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_ConnectionStart" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_ConnectionClosing" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_ConnectionClosing" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_ConnectionCrashed" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_ConnectionCrashed" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_PacketStart" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_PacketStart" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_PacketEnd" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_PacketEnd" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__CNetChan_ProcessPacket" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__CNetChan_ProcessPacket" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_FileReceived" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_FileReceived" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_FileDenied" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_FileDenied" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__CNetChan_ProcessMessages" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__CNetChan_ProcessMessages" );
}

void HookCNetChan( GarrysMod::Lua::ILuaBase *LUA )
{
	if( !IS_ATTACHED( CNetChan_Shutdown ) )
	{
		LUA->PushCFunction( Attach__CNetChan_Shutdown );
		LUA->Push( -2 );
		LUA->Call( 1, 0 );
	}
}

void HookINetChannelHandler( GarrysMod::Lua::ILuaBase *LUA )
{
	if( !IS_ATTACHED( INetChannelHandler_ConnectionClosing ) )
	{
		LUA->PushCFunction( Attach__INetChannelHandler_ConnectionClosing );
		LUA->Push( -2 );
		LUA->Call( 1, 0 );
	}

	if( !IS_ATTACHED( INetChannelHandler_ConnectionCrashed ) )
	{
		LUA->PushCFunction( Attach__INetChannelHandler_ConnectionCrashed );
		LUA->Push( -2 );
		LUA->Call( 1, 0 );
	}
}

}
