#include <detours.h>
#include <vfnhook.h>
#include <hooks.hpp>
#include <netchannel.hpp>
#include <netchannelhandler.hpp>
#include <netmessage.hpp>
#include <ucharptr.hpp>
#include <sn4_bf_read.hpp>
#include <sn4_bf_write.hpp>
#include <net.hpp>
#include <protocol.hpp>
#include <inetmessage.h>
#include <inetmsghandler.h>
#include <sstream>
#include <symbolfinder.hpp>
#include <GarrysMod/Lua/LuaInterface.h>

namespace Hooks
{

#if defined _WIN32

typedef bool ( __thiscall *CNetChan_ProcessMessages_T )( CNetChan *netchan, bf_read &buf );

static const char *CNetChan_ProcessMessages_sig = "\x55\x8B\xEC\x83\xEC\x2C\x53\x89\x4D\xFC";
static const size_t CNetChan_ProcessMessages_siglen = 10;

#elif defined __linux

typedef bool ( *CNetChan_ProcessMessages_T )( CNetChan *netchan, bf_read &buf );

static const char *CNetChan_ProcessMessages_sig = "@_ZN8CNetChan15ProcessMessagesER7bf_read";
static const size_t CNetChan_ProcessMessages_siglen = 0;

#elif defined __APPLE__

typedef bool ( *CNetChan_ProcessMessages_T )( CNetChan *netchan, bf_read &buf );

static const char *CNetChan_ProcessMessages_sig = "@__ZN8CNetChan15ProcessMessagesER7bf_read";
static const size_t CNetChan_ProcessMessages_siglen = 0;

#endif

static CNetChan_ProcessMessages_T CNetChan_ProcessMessages_O = nullptr;

static uintptr_t CNetChan_vtable = 0;
static uintptr_t INetChannelHandler_vtable = 0;

LUA_FUNCTION_STATIC( ErrorTraceback )
{
	GarrysMod::Lua::ILuaInterface *lua = static_cast<GarrysMod::Lua::ILuaInterface *>( LUA );

	std::string spaces( "\n  " );
	std::ostringstream stream;
	stream << LUA->GetString( 1 );

	lua_Debug dbg = { 0 };
	for( int lvl = 1; lua->GetStack( lvl, &dbg ) == 1; ++lvl, memset( &dbg, 0, sizeof( dbg ) ) )
	{
		if( lua->GetInfo( "Sln", &dbg ) == 0 )
			break;

		stream
			<< spaces
			<< lvl
			<< ". "
			<< ( dbg.name == nullptr ? "unknown" : dbg.name )
			<< " - "
			<< dbg.short_src
			<< ':'
			<< dbg.currentline;
		spaces += ' ';
	}

	LUA->PushString( stream.str( ).c_str( ) );
	return 1;
}

#define INIT_HOOK( name ) \
{ \
	lua_State *state = Global::lua_state; \
	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB ); \
	LUA->GetField( -1, "hook" ); \
	LUA->PushCFunction( ErrorTraceback ); \
	LUA->GetField( -2, "Call" ); \
	LUA->PushString( name ); \
	LUA->PushNil( ); \
	int32_t __argc = 2

#define DO_HOOK( code ) \
	code; \
	++__argc

#define CALL_HOOK( returns ) \
	if( LUA->PCall( __argc, returns, -2 - __argc ) != 0 ) \
	{ \
		static_cast<GarrysMod::Lua::ILuaInterface *>( LUA )->ErrorNoHalt( \
			"\n%s\n\n", LUA->GetString( -1 ) \
		); \
		LUA->Pop( 1 ); \
	} \
	LUA->Pop( 3 );

#define END_HOOK( ) \
}

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
			meta *temp = namespc::Get( state, 1 ); \
			meta##_vtable = VTBL( temp ); \
			HOOKVFUNC( temp, offset, name##_O, name##_D ); \
			REGISTER_ATTACH( name ); \
		} \
		return 0; \
	} \
	LUA_FUNCTION_STATIC( Detach__##name ) \
	{ \
		if( IS_ATTACHED( name ) ) \
		{ \
			UNHOOKVFUNC( &meta##_vtable, offset, name##_O ); \
			REGISTER_DETACH( name ); \
		} \
		return 0; \
	}

DEFVFUNC_( CNetChan_SendDatagram_O, int32_t, ( CNetChan *netchan, bf_write *data ) );

int32_t VFUNC CNetChan_SendDatagram_D( CNetChan *netchan, bf_write *data )
{
	INIT_HOOK( "PreSendDatagram" );
		DO_HOOK( NetChannel::Push( state, netchan ) );

		if( !Global::engine_server->IsDedicatedServer( ) )
		{
			DO_HOOK( NetChannel::Push(
				state,
				static_cast<CNetChan *>( Global::engine_client->GetNetChannelInfo( ) )
			) );
		}
		else
		{
			DO_HOOK( LUA->PushNil( ) );
		}

		DO_HOOK( bf_write **writer1 = sn4_bf_write::Push( state, data ) );
		DO_HOOK( bf_write **writer2 = sn4_bf_write::Push( state, &netchan->reliabledata ) );
		DO_HOOK( bf_write **writer3 = sn4_bf_write::Push( state, &netchan->unreliabledata ) );
		DO_HOOK( bf_write **writer4 = sn4_bf_write::Push( state, &netchan->voicedata ) );
		CALL_HOOK( 0 );

		*writer1 = nullptr;
		*writer2 = nullptr;
		*writer3 = nullptr;
		*writer4 = nullptr;
	END_HOOK( );

	int32_t r = CNetChan_SendDatagram_O( netchan, data );

	INIT_HOOK( "PostSendDatagram" );
		DO_HOOK( NetChannel::Push( state, netchan ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	return r;
}

#if defined _WIN32

SIMPLE_VHOOK_BINDING( CNetChan, NetChannel, CNetChan_SendDatagram, 46 );

#elif defined __linux || defined __APPLE__

SIMPLE_VHOOK_BINDING( CNetChan, NetChannel, CNetChan_SendDatagram, 47 );

#endif

DEFVFUNC_( CNetChan_ProcessPacket_O, void, (
	CNetChan *netchan,
	netpacket_t *packet,
	bool bHasHeader
) );

void VFUNC CNetChan_ProcessPacket_D( CNetChan *netchan, netpacket_t *packet, bool bHasHeader )
{
	INIT_HOOK( "PreProcessPacket" );
		DO_HOOK( NetChannel::Push( state, netchan ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	CNetChan_ProcessPacket_O( netchan, packet, bHasHeader );

	INIT_HOOK( "PostProcessPacket" );
		DO_HOOK( NetChannel::Push( state, netchan ) );
		CALL_HOOK( 0 );
	END_HOOK( );
}

#if defined _WIN32

SIMPLE_VHOOK_BINDING( CNetChan, NetChannel, CNetChan_ProcessPacket, 39 );

#elif defined __linux || defined __APPLE__

SIMPLE_VHOOK_BINDING( CNetChan, NetChannel, CNetChan_ProcessPacket, 40 );

#endif

DEFVFUNC_( CNetChan_Shutdown_O, void, ( CNetChan *netchan, const char *reason ) );

void VFUNC CNetChan_Shutdown_D( CNetChan *netchan, const char *reason )
{
	INIT_HOOK( "PreNetChannelShutdown" );
		DO_HOOK( NetChannel::Push( state, netchan ) );

		// Fuck Linux
		if( reason != nullptr && reason != reinterpret_cast<const char *>( 0x02 ) )
		{
			DO_HOOK( LUA->PushString( reason ) );
		}
		else
		{
			DO_HOOK( LUA->PushNil( ) );
		}

		CALL_HOOK( 0 );
	END_HOOK( );

	NetMessage::Destroy( Global::lua_state, netchan );

	NetChannel::Destroy( Global::lua_state, netchan );

	CNetChan_Shutdown_O( netchan, reason );

	INIT_HOOK( "PostNetChannelShutdown" );
		CALL_HOOK( 0 );
	END_HOOK( );
}

SIMPLE_VHOOK_BINDING( CNetChan, NetChannel, CNetChan_Shutdown, 36 );

DEFVFUNC_( INetChannelHandler_ConnectionStart_O, void, (
	INetChannelHandler *handler,
	CNetChan *netchan
) );

void VFUNC INetChannelHandler_ConnectionStart_D( INetChannelHandler *handler, CNetChan *netchan )
{
	INIT_HOOK( "INetChannelHandler::ConnectionStart" );
		DO_HOOK( NetChannelHandler::Push( state, handler ) );
		DO_HOOK( NetChannel::Push( state, netchan ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	INetChannelHandler_ConnectionStart_O( handler, netchan );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, NetChannelHandler, INetChannelHandler_ConnectionStart, 1 );

DEFVFUNC_( INetChannelHandler_ConnectionClosing_O, void, (
	INetChannelHandler *handler,
	const char *reason
) );

void VFUNC INetChannelHandler_ConnectionClosing_D(
	INetChannelHandler *handler,
	const char *reason
)
{
	INIT_HOOK( "INetChannelHandler::ConnectionClosing" );
		DO_HOOK( NetChannelHandler::Push( state, handler ) );
		DO_HOOK( LUA->PushString( reason ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	NetChannelHandler::Destroy( Global::lua_state, handler );

	INetChannelHandler_ConnectionClosing_O( handler, reason );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, NetChannelHandler, INetChannelHandler_ConnectionClosing, 2 );

DEFVFUNC_( INetChannelHandler_ConnectionCrashed_O, void, (
	INetChannelHandler *handler,
	const char *reason
) );

void VFUNC INetChannelHandler_ConnectionCrashed_D(
	INetChannelHandler *handler,
	const char *reason
)
{
	INIT_HOOK( "INetChannelHandler::ConnectionCrashed" );
		DO_HOOK( NetChannelHandler::Push( state, handler ) );
		DO_HOOK( LUA->PushString( reason ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	NetChannelHandler::Destroy( Global::lua_state, handler );

	INetChannelHandler_ConnectionCrashed_O( handler, reason );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, NetChannelHandler, INetChannelHandler_ConnectionCrashed, 3 );

DEFVFUNC_( INetChannelHandler_PacketStart_O, void, (
	INetChannelHandler *handler,
	int32_t incoming_sequence,
	int32_t outgoing_acknowledged
) );

void VFUNC INetChannelHandler_PacketStart_D(
	INetChannelHandler *handler,
	int32_t incoming_sequence,
	int32_t outgoing_acknowledged
)
{
	INIT_HOOK( "INetChannelHandler::PacketStart" );
		DO_HOOK( NetChannelHandler::Push( state, handler ) );
		DO_HOOK( LUA->PushNumber( incoming_sequence ) );
		DO_HOOK( LUA->PushNumber( incoming_sequence ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	INetChannelHandler_PacketStart_O( handler, incoming_sequence, outgoing_acknowledged );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, NetChannelHandler, INetChannelHandler_PacketStart, 4 );

DEFVFUNC_( INetChannelHandler_PacketEnd_O, void, ( INetChannelHandler *handler ) );

void VFUNC INetChannelHandler_PacketEnd_D( INetChannelHandler *handler )
{
	INIT_HOOK( "INetChannelHandler::PacketEnd" );
		DO_HOOK( NetChannelHandler::Push( state, handler ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	INetChannelHandler_PacketEnd_O( handler );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, NetChannelHandler, INetChannelHandler_PacketEnd, 5 );

DEFVFUNC_( INetChannelHandler_FileRequested_O, void, (
	INetChannelHandler *handler,
	const char *fileName,
	uint32_t transferID
) );

void VFUNC INetChannelHandler_FileRequested_D(
	INetChannelHandler *handler,
	const char *fileName,
	uint32_t transferID
)
{
	INIT_HOOK( "INetChannelHandler::FileRequested" );
		DO_HOOK( NetChannelHandler::Push( state, handler ) );
		DO_HOOK( LUA->PushString( fileName ) );
		DO_HOOK( LUA->PushNumber( transferID ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	INetChannelHandler_FileRequested_O( handler, fileName, transferID );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, NetChannelHandler, INetChannelHandler_FileRequested, 6 );

DEFVFUNC_( INetChannelHandler_FileReceived_O, void, (
	INetChannelHandler *handler,
	const char *fileName,
	uint32_t transferID
) );

void VFUNC INetChannelHandler_FileReceived_D(
	INetChannelHandler *handler,
	const char *fileName,
	uint32_t transferID
)
{
	INIT_HOOK( "INetChannelHandler::FileReceived" );
		DO_HOOK( NetChannelHandler::Push( state, handler ) );
		DO_HOOK( LUA->PushString( fileName ) );
		DO_HOOK( LUA->PushNumber( transferID ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	INetChannelHandler_FileReceived_O( handler, fileName, transferID );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, NetChannelHandler, INetChannelHandler_FileReceived, 7 );

DEFVFUNC_( INetChannelHandler_FileDenied_O, void, (
	INetChannelHandler *handler,
	const char *fileName,
	uint32_t transferID
) );

void VFUNC INetChannelHandler_FileDenied_D(
	INetChannelHandler *handler,
	const char *fileName,
	uint32_t transferID
)
{
	INIT_HOOK( "INetChannelHandler::FileDenied" );
		DO_HOOK( NetChannelHandler::Push( state, handler ) );
		DO_HOOK( LUA->PushString( fileName ) );
		DO_HOOK( LUA->PushNumber( transferID ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	INetChannelHandler_FileDenied_O( handler, fileName, transferID );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, NetChannelHandler, INetChannelHandler_FileDenied, 8 );

static MologieDetours::Detour<CNetChan_ProcessMessages_T> *
	CNetChan_ProcessMessages_detour = nullptr;

#if defined _WIN32

static bool __fastcall CNetChan_ProcessMessages_D( CNetChan *netchan, void *, bf_read &buf )

#elif defined __linux || defined __APPLE__

static bool CNetChan_ProcessMessages_D( CNetChan *netchan, bf_read &buf )

#endif

{
	static uint8_t data[100000] = { 0 };
	if( !buf.IsOverflowed( ) )
	{
		memcpy( data, buf.m_pData, buf.GetNumBytesRead( ) );

		int32_t bitsread = buf.GetNumBitsRead( );
		bf_write write( data, sizeof( data ) );
		write.SeekToBit( bitsread );

		INIT_HOOK( "PreProcessMessages" );
			DO_HOOK( NetChannel::Push( state, netchan ) );
			DO_HOOK( bf_read **reader = sn4_bf_read::Push( state, &buf ) );
			DO_HOOK( bf_write **writer = sn4_bf_write::Push( state, &write ) );

			if( !Global::engine_server->IsDedicatedServer( ) )
				DO_HOOK( NetChannel::Push(
					state,
					static_cast<CNetChan *>( Global::engine_client->GetNetChannelInfo( ) )
				) );

			CALL_HOOK( 0 );

			*reader = nullptr;
			*writer = nullptr;
		END_HOOK( );

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
			new( std::nothrow ) MologieDetours::Detour<CNetChan_ProcessMessages_T>(
				CNetChan_ProcessMessages_O,
				reinterpret_cast<CNetChan_ProcessMessages_T>( CNetChan_ProcessMessages_D )
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

void Initialize( lua_State *state )
{
	SymbolFinder symfinder;

	CNetChan_ProcessMessages_O =
		reinterpret_cast<CNetChan_ProcessMessages_T>( symfinder.ResolveOnBinary(
			Global::engine_lib,
			CNetChan_ProcessMessages_sig,
			CNetChan_ProcessMessages_siglen
		) );
	if( CNetChan_ProcessMessages_O == nullptr )
		LUA->ThrowError( "failed to locate CNetChan::ProcessMessages" );

	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );

		LUA->PushCFunction( Attach__CNetChan_ProcessPacket );
		LUA->SetField( -2, "Attach__CNetChan_ProcessPacket" );

		LUA->PushCFunction( Detach__CNetChan_ProcessPacket );
		LUA->SetField( -2, "Detach__CNetChan_ProcessPacket" );

		LUA->PushCFunction( Attach__CNetChan_SendDatagram );
		LUA->SetField( -2, "Attach__CNetChan_SendDatagram" );

		LUA->PushCFunction( Detach__CNetChan_SendDatagram );
		LUA->SetField( -2, "Detach__CNetChan_SendDatagram" );

		LUA->PushCFunction( Attach__CNetChan_Shutdown );
		LUA->SetField( -2, "Attach__CNetChan_Shutdown" );

		//LUA->PushCFunction( Detach__CNetChan_Shutdown );
		LUA->PushCFunction( Empty );
		LUA->SetField( -2, "Detach__CNetChan_Shutdown" );

		LUA->PushCFunction( Attach__INetChannelHandler_ConnectionStart );
		LUA->SetField( -2, "Attach__INetChannelHandler_ConnectionStart" );

		LUA->PushCFunction( Detach__INetChannelHandler_ConnectionStart );
		LUA->SetField( -2, "Detach__INetChannelHandler_ConnectionStart" );

		LUA->PushCFunction( Attach__INetChannelHandler_ConnectionClosing );
		LUA->SetField( -2, "Attach__INetChannelHandler_ConnectionClosing" );

		//LUA->PushCFunction( Detach__INetChannelHandler_ConnectionClosing );
		LUA->PushCFunction( Empty );
		LUA->SetField( -2, "Detach__INetChannelHandler_ConnectionClosing" );

		LUA->PushCFunction( Attach__INetChannelHandler_ConnectionCrashed );
		LUA->SetField( -2, "Attach__INetChannelHandler_ConnectionCrashed" );

		//LUA->PushCFunction( Detach__INetChannelHandler_ConnectionCrashed );
		LUA->PushCFunction( Empty );
		LUA->SetField( -2, "Detach__INetChannelHandler_ConnectionCrashed" );

		LUA->PushCFunction( Attach__INetChannelHandler_PacketStart );
		LUA->SetField( -2, "Attach__INetChannelHandler_PacketStart" );

		LUA->PushCFunction( Detach__INetChannelHandler_PacketStart );
		LUA->SetField( -2, "Detach__INetChannelHandler_PacketStart" );

		LUA->PushCFunction( Attach__INetChannelHandler_PacketEnd );
		LUA->SetField( -2, "Attach__INetChannelHandler_PacketEnd" );

		LUA->PushCFunction( Detach__INetChannelHandler_PacketEnd );
		LUA->SetField( -2, "Detach__INetChannelHandler_PacketEnd" );

		LUA->PushCFunction( Attach__CNetChan_ProcessPacket );
		LUA->SetField( -2, "Attach__CNetChan_ProcessPacket" );

		LUA->PushCFunction( Detach__CNetChan_ProcessPacket );
		LUA->SetField( -2, "Detach__CNetChan_ProcessPacket" );

		LUA->PushCFunction( Attach__INetChannelHandler_FileReceived );
		LUA->SetField( -2, "Attach__INetChannelHandler_FileReceived" );

		LUA->PushCFunction( Detach__INetChannelHandler_FileReceived );
		LUA->SetField( -2, "Detach__INetChannelHandler_FileReceived" );

		LUA->PushCFunction( Attach__INetChannelHandler_FileDenied );
		LUA->SetField( -2, "Attach__INetChannelHandler_FileDenied" );

		LUA->PushCFunction( Detach__INetChannelHandler_FileDenied );
		LUA->SetField( -2, "Detach__INetChannelHandler_FileDenied" );

		LUA->PushCFunction( Attach__CNetChan_ProcessMessages );
		LUA->SetField( -2, "Attach__CNetChan_ProcessMessages" );

		LUA->PushCFunction( Detach__CNetChan_ProcessMessages );
		LUA->SetField( -2, "Detach__CNetChan_ProcessMessages" );

	LUA->Pop( 1 );
}

void Deinitialize( lua_State *state )
{
	Detach__CNetChan_SendDatagram( state );
	Detach__CNetChan_ProcessPacket( state );
	Detach__CNetChan_Shutdown( state );

	Detach__INetChannelHandler_ConnectionStart( state );
	Detach__INetChannelHandler_ConnectionClosing( state );
	Detach__INetChannelHandler_ConnectionCrashed( state );
	Detach__INetChannelHandler_PacketStart( state );
	Detach__INetChannelHandler_PacketEnd( state );
	Detach__INetChannelHandler_FileRequested( state );
	Detach__INetChannelHandler_FileReceived( state );
	Detach__INetChannelHandler_FileDenied( state );

	Detach__CNetChan_ProcessMessages( state );

	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );

		LUA->PushNil( );
		LUA->SetField( -2, "Attach__CNetChan_ProcessPacket" );

		LUA->PushNil( );
		LUA->SetField( -2, "Detach__CNetChan_ProcessPacket" );

		LUA->PushNil( );
		LUA->SetField( -2, "Attach__CNetChan_SendDatagram" );

		LUA->PushNil( );
		LUA->SetField( -2, "Detach__CNetChan_SendDatagram" );

		LUA->PushNil( );
		LUA->SetField( -2, "Attach__CNetChan_Shutdown" );

		LUA->PushNil( );
		LUA->SetField( -2, "Detach__CNetChan_Shutdown" );

		LUA->PushNil( );
		LUA->SetField( -2, "Attach__INetChannelHandler_ConnectionStart" );

		LUA->PushNil( );
		LUA->SetField( -2, "Detach__INetChannelHandler_ConnectionStart" );

		LUA->PushNil( );
		LUA->SetField( -2, "Attach__INetChannelHandler_ConnectionClosing" );

		LUA->PushNil( );
		LUA->SetField( -2, "Detach__INetChannelHandler_ConnectionClosing" );

		LUA->PushNil( );
		LUA->SetField( -2, "Attach__INetChannelHandler_ConnectionCrashed" );

		LUA->PushNil( );
		LUA->SetField( -2, "Detach__INetChannelHandler_ConnectionCrashed" );

		LUA->PushNil( );
		LUA->SetField( -2, "Attach__INetChannelHandler_PacketStart" );

		LUA->PushNil( );
		LUA->SetField( -2, "Detach__INetChannelHandler_PacketStart" );

		LUA->PushNil( );
		LUA->SetField( -2, "Attach__INetChannelHandler_PacketEnd" );

		LUA->PushNil( );
		LUA->SetField( -2, "Detach__INetChannelHandler_PacketEnd" );

		LUA->PushNil( );
		LUA->SetField( -2, "Attach__CNetChan_ProcessPacket" );

		LUA->PushNil( );
		LUA->SetField( -2, "Detach__CNetChan_ProcessPacket" );

		LUA->PushNil( );
		LUA->SetField( -2, "Attach__INetChannelHandler_FileReceived" );

		LUA->PushNil( );
		LUA->SetField( -2, "Detach__INetChannelHandler_FileReceived" );

		LUA->PushNil( );
		LUA->SetField( -2, "Attach__INetChannelHandler_FileDenied" );

		LUA->PushNil( );
		LUA->SetField( -2, "Detach__INetChannelHandler_FileDenied" );

		LUA->PushNil( );
		LUA->SetField( -2, "Attach__CNetChan_ProcessMessages" );

		LUA->PushNil( );
		LUA->SetField( -2, "Detach__CNetChan_ProcessMessages" );

	LUA->Pop( 1 );
}

void HookCNetChan( lua_State *state, CNetChan *netchan )
{
	LUA->PushCFunction( Attach__CNetChan_Shutdown );
	LUA->Push( -2 );
	LUA->Call( 1, 0 );
}

void HookINetChannelHandler( lua_State *state, INetChannelHandler *handler )
{
	LUA->PushCFunction( Attach__INetChannelHandler_ConnectionClosing );
	LUA->Push( -2 );
	LUA->Call( 1, 0 );

	LUA->PushCFunction( Attach__INetChannelHandler_ConnectionCrashed );
	LUA->Push( -2 );
	LUA->Call( 1, 0 );
}

}