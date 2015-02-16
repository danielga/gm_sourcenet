// Required interfaces
#define IVENGINESERVER_INTERFACE
#define IVENGINECLIENT_INTERFACE

#include <detours.h>
#include <gl_hooks.hpp>
#include <gl_cnetchan.hpp>
#include <gl_inetchannelhandler.hpp>
#include <gl_ucharptr.hpp>
#include <gl_bitbuf_read.hpp>
#include <gl_bitbuf_write.hpp>
#include <vfnhook.h>
#include <net.h>
#include <protocol.h>
#include <inetmessage.h>
#include <inetmsghandler.h>
#include <GarrysMod/Lua/LuaInterface.h>

static uintptr_t CNetChan_vtable = 0;
static uintptr_t INetChannelHandler_vtable = 0;

LUA_FUNCTION_STATIC( ErrorTraceback )
{
	std::string spaces( 2, ' ' );

	int strs = 1;
	lua_Debug dbg = { 0 };
	for(
		int level = 1;
		lua_getstack( state, level, &dbg ) == 1;
		++level, memset( &dbg, 0, sizeof( dbg ) )
	)
	{
		if( lua_getinfo( state, "Sln", &dbg ) == 0 )
			break;

		lua_pushfstring(
			state,
			"\n%s%d. %s - %s:%d",
			spaces.c_str( ),
			level,
			dbg.name == nullptr ? "unknown" : dbg.name,
			dbg.short_src,
			dbg.currentline
		);

		++strs;
		spaces += ' ';
	}

	lua_concat( state, strs );
	return 1;
}

#define INIT_HOOK( name ) \
{ \
	lua_State *state = global_state; \
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
	bool attach_status__##name = false

#define IS_ATTACHED( name ) \
	attach_status__##name

#define REGISTER_ATTACH( name ) \
	Msg( "Attach: %s\n", #name ); \
	attach_status__##name = true

#define REGISTER_DETACH( name ) \
	Msg( "Detach: %s\n", #name ); \
	attach_status__##name = false

#define SIMPLE_VHOOK_BINDING( meta, name, offset ) \
	MONITOR_HOOK( name ); \
	GLBL_FUNCTION( Attach__##name ) \
	{ \
		LUA->CheckType( 1, GET_META_ID( meta ) ); \
		if( !IS_ATTACHED( name ) ) \
		{ \
			meta *temp = Get_##meta( state, 1 ); \
			meta##_vtable = VTBL( temp ); \
			HOOKVFUNC( temp, offset, name##_O, name##_D ); \
			REGISTER_ATTACH( name ); \
		} \
		return 0; \
	} \
	GLBL_FUNCTION( Detach__##name ) \
	{ \
		if( IS_ATTACHED( name ) ) \
		{ \
			UNHOOKVFUNC( &meta##_vtable, offset, name##_O ); \
			REGISTER_DETACH( name ); \
		} \
		return 0; \
	}

DEFVFUNC_( CNetChan_SendDatagram_O, int32_t, ( CNetChan *netchan, sn4_bf_write *data ) );

int32_t VFUNC CNetChan_SendDatagram_D( CNetChan *netchan, sn4_bf_write *data )
{
	INIT_HOOK( "PreSendDatagram" );
		DO_HOOK( Push_CNetChan( state, netchan ) );

		if( !g_pEngineServer->IsDedicatedServer( ) )
		{
			DO_HOOK( Push_CNetChan(
				state,
				static_cast<CNetChan *>( g_pEngineClient->GetNetChannelInfo( ) )
			) );
		}
		else
		{
			DO_HOOK( LUA->PushNil( ) );
		}

		DO_HOOK( sn4_bf_write **writer1 = Push_sn4_bf_write( state, data ) );
		DO_HOOK( sn4_bf_write **writer2 = Push_sn4_bf_write( state, &netchan->reliabledata ) );
		DO_HOOK( sn4_bf_write **writer3 = Push_sn4_bf_write( state, &netchan->unreliabledata ) );
		DO_HOOK( sn4_bf_write **writer4 = Push_sn4_bf_write( state, &netchan->voicedata ) );
		CALL_HOOK( 0 );

		*writer1 = nullptr;
		*writer2 = nullptr;
		*writer3 = nullptr;
		*writer4 = nullptr;
	END_HOOK( );

	int32_t r = CNetChan_SendDatagram_O( netchan, data );

	INIT_HOOK( "PostSendDatagram" );
		DO_HOOK( Push_CNetChan( state, netchan ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	return r;
}

#if defined _WIN32

SIMPLE_VHOOK_BINDING( CNetChan, CNetChan_SendDatagram, 46 );

#else

SIMPLE_VHOOK_BINDING( CNetChan, CNetChan_SendDatagram, 47 );

#endif

DEFVFUNC_( CNetChan_ProcessPacket_O, void, (
	CNetChan *netchan,
	netpacket_t *packet,
	bool bHasHeader
) );

void VFUNC CNetChan_ProcessPacket_D( CNetChan *netchan, netpacket_t *packet, bool bHasHeader )
{
	INIT_HOOK( "PreProcessPacket" );
		DO_HOOK( Push_CNetChan( state, netchan ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	CNetChan_ProcessPacket_O( netchan, packet, bHasHeader );

	INIT_HOOK( "PostProcessPacket" );
		DO_HOOK( Push_CNetChan( state, netchan ) );
		CALL_HOOK( 0 );
	END_HOOK( );
}

#if defined _WIN32

SIMPLE_VHOOK_BINDING( CNetChan, CNetChan_ProcessPacket, 39 );

#else

SIMPLE_VHOOK_BINDING( CNetChan, CNetChan_ProcessPacket, 40 );

#endif

DEFVFUNC_( CNetChan_Shutdown_O, void, ( CNetChan *netchan, const char *reason ) );

void VFUNC CNetChan_Shutdown_D( CNetChan *netchan, const char *reason )
{
	INIT_HOOK( "PreNetChannelShutdown" );
		DO_HOOK( Push_CNetChan( state, netchan ) );

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

	CNetChan_Shutdown_O( netchan, reason );

	INIT_HOOK( "PostNetChannelShutdown" );
		CALL_HOOK( 0 );
	END_HOOK( );
}

SIMPLE_VHOOK_BINDING( CNetChan, CNetChan_Shutdown, 36 );

DEFVFUNC_( INetChannelHandler_ConnectionStart_O, void, (
	INetChannelHandler *handler,
	CNetChan *netchan
) );

void VFUNC INetChannelHandler_ConnectionStart_D( INetChannelHandler *handler, CNetChan *netchan )
{
	INIT_HOOK( "INetChannelHandler::ConnectionStart" );
		DO_HOOK( Push_INetChannelHandler( state, handler ) );
		DO_HOOK( Push_CNetChan( state, netchan ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	INetChannelHandler_ConnectionStart_O( handler, netchan );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_ConnectionStart, 1 );

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
		DO_HOOK( Push_INetChannelHandler( state, handler ) );
		DO_HOOK( LUA->PushString( reason ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	INetChannelHandler_ConnectionClosing_O( handler, reason );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_ConnectionClosing, 2 );

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
		DO_HOOK( Push_INetChannelHandler( state, handler ) );
		DO_HOOK( LUA->PushString( reason ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	INetChannelHandler_ConnectionCrashed_O( handler, reason );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_ConnectionCrashed, 3 );

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
		DO_HOOK( Push_INetChannelHandler( state, handler ) );
		DO_HOOK( LUA->PushNumber( incoming_sequence ) );
		DO_HOOK( LUA->PushNumber( incoming_sequence ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	INetChannelHandler_PacketStart_O( handler, incoming_sequence, outgoing_acknowledged );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_PacketStart, 4 );

DEFVFUNC_( INetChannelHandler_PacketEnd_O, void, ( INetChannelHandler *handler ) );

void VFUNC INetChannelHandler_PacketEnd_D( INetChannelHandler *handler )
{
	INIT_HOOK( "INetChannelHandler::PacketEnd" );
		DO_HOOK( Push_INetChannelHandler( state, handler ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	INetChannelHandler_PacketEnd_O( handler );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_PacketEnd, 5 );

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
		DO_HOOK( Push_INetChannelHandler( state, handler ) );
		DO_HOOK( LUA->PushString( fileName ) );
		DO_HOOK( LUA->PushNumber( transferID ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	INetChannelHandler_FileRequested_O( handler, fileName, transferID );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_FileRequested, 6 );

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
		DO_HOOK( Push_INetChannelHandler( state, handler ) );
		DO_HOOK( LUA->PushString( fileName ) );
		DO_HOOK( LUA->PushNumber( transferID ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	INetChannelHandler_FileReceived_O( handler, fileName, transferID );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_FileReceived, 7 );

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
		DO_HOOK( Push_INetChannelHandler( state, handler ) );
		DO_HOOK( LUA->PushString( fileName ) );
		DO_HOOK( LUA->PushNumber( transferID ) );
		CALL_HOOK( 0 );
	END_HOOK( );

	INetChannelHandler_FileDenied_O( handler, fileName, transferID );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_FileDenied, 8 );

static MologieDetours::Detour<CNetChan_ProcessMessages_T> *
	CNetChan_ProcessMessages_detour = nullptr;

#if defined _WIN32

static bool __fastcall CNetChan_ProcessMessages_D( CNetChan *netchan, void *, sn4_bf_read &buf )

#elif defined __linux || defined __APPLE__

static bool CNetChan_ProcessMessages_D( CNetChan *netchan, sn4_bf_read &buf )

#endif

{
	static uint8_t data[96000] = { 0 };
	if( !buf.IsOverflowed( ) )
	{
		memcpy( data, buf.m_pData, buf.GetNumBytesRead( ) );

		int32_t bitsread = buf.GetNumBitsRead( );
		sn4_bf_write write( data, sizeof( data ) );
		write.SeekToBit( bitsread );

		INIT_HOOK( "PreProcessMessages" );
			DO_HOOK( Push_CNetChan( state, netchan ) );
			DO_HOOK( sn4_bf_read **reader = Push_sn4_bf_read( state, &buf ) );
			DO_HOOK( sn4_bf_write **writer = Push_sn4_bf_write( state, &write ) );

			if( !g_pEngineServer->IsDedicatedServer( ) )
				DO_HOOK( Push_CNetChan(
					state,
					static_cast<CNetChan *>( g_pEngineClient->GetNetChannelInfo( ) )
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

GLBL_FUNCTION( Attach__CNetChan_ProcessMessages )
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

GLBL_FUNCTION( Detach__CNetChan_ProcessMessages )
{
	if( IS_ATTACHED( CNetChan_ProcessMessages ) )
	{
		delete CNetChan_ProcessMessages_detour;
		CNetChan_ProcessMessages_detour = nullptr;

		REGISTER_DETACH( CNetChan_ProcessMessages );
	}

	return 0;
}

void UnloadHooks( lua_State *state )
{
	_G__Detach__CNetChan_SendDatagram( state );
	_G__Detach__CNetChan_ProcessPacket( state );
	_G__Detach__CNetChan_Shutdown( state );

	_G__Detach__INetChannelHandler_ConnectionStart( state );
	_G__Detach__INetChannelHandler_ConnectionClosing( state );
	_G__Detach__INetChannelHandler_ConnectionCrashed( state );
	_G__Detach__INetChannelHandler_PacketStart( state );
	_G__Detach__INetChannelHandler_PacketEnd( state );
	_G__Detach__INetChannelHandler_FileRequested( state );
	_G__Detach__INetChannelHandler_FileReceived( state );
	_G__Detach__INetChannelHandler_FileDenied( state );

	_G__Detach__CNetChan_ProcessMessages( state );
}