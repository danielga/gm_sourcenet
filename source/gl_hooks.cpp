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

#define SIMPLE_VHOOK_BINDING( meta, name, offset ) \
	MONITOR_HOOK( name ); \
	GLBL_FUNCTION( Attach__##name ) \
	{ \
		LUA->CheckType( 1, GET_META_ID( meta ) ); \
		if( !IS_ATTACHED( name ) ) \
		{ \
			HOOKVFUNC( GET_META( 1, meta ), offset, name##_O, name##_D ); \
			REGISTER_ATTACH( name ); \
		} \
		return 0; \
	} \
	GLBL_FUNCTION( Detach__##name ) \
	{ \
		LUA->CheckType( 1, GET_META_ID( meta ) ); \
		if( IS_ATTACHED( name ) ) \
		{ \
			UNHOOKVFUNC( GET_META( 1, meta ), offset, name##_O ); \
			REGISTER_DETACH( name ); \
		} \
		return 0; \
	}

DEFVFUNC_( CNetChan_SendDatagram_O, int, ( CNetChan *netchan, sn4_bf_write *data ) );

int VFUNC CNetChan_SendDatagram_D( CNetChan *netchan, sn4_bf_write *data )
{
	INIT_HOOK( "PreSendDatagram" );
		DO_HOOK( PUSH_META( netchan, CNetChan ) );

		if( !g_pEngineServer->IsDedicatedServer( ) )
		{
			DO_HOOK( PUSH_META( g_pEngineClient->GetNetChannelInfo( ), CNetChan ) );
		}
		else
		{
			DO_HOOK( LUA->PushNil( ) );
		}

		DO_HOOK( PUSH_META( data, sn4_bf_write ) );
		DO_HOOK( PUSH_META( &netchan->reliabledata, sn4_bf_write ) );
		DO_HOOK( PUSH_META( &netchan->unreliabledata, sn4_bf_write ) );
		DO_HOOK( PUSH_META( &netchan->voicedata, sn4_bf_write ) );
	CALL_HOOK( 0 );

	int r = CNetChan_SendDatagram_O( netchan, data );

	INIT_HOOK( "PostSendDatagram" );
		DO_HOOK( PUSH_META( netchan, CNetChan ) );
	CALL_HOOK( 0 );

	return r;
}

#if defined _WIN32

SIMPLE_VHOOK_BINDING( CNetChan, CNetChan_SendDatagram, 46 );

#else

SIMPLE_VHOOK_BINDING( CNetChan, CNetChan_SendDatagram, 47 );

#endif

DEFVFUNC_( CNetChan_ProcessPacket_O, void, ( CNetChan *netchan, netpacket_t *packet, bool bHasHeader ) );

void VFUNC CNetChan_ProcessPacket_D( CNetChan *netchan, netpacket_t *packet, bool bHasHeader )
{
	INIT_HOOK( "PreProcessPacket" );
		DO_HOOK( PUSH_META( netchan, CNetChan ) );
	CALL_HOOK( 0 );

	CNetChan_ProcessPacket_O( netchan, packet, bHasHeader );

	INIT_HOOK( "PostProcessPacket" );
		DO_HOOK( PUSH_META( netchan, CNetChan ) );
	CALL_HOOK( 0 );
}

#if defined _WIN32

SIMPLE_VHOOK_BINDING( CNetChan, CNetChan_ProcessPacket, 39 );

#else

SIMPLE_VHOOK_BINDING( CNetChan, CNetChan_ProcessPacket, 40 ); // ?????

#endif

DEFVFUNC_( CNetChan_Shutdown_O, void, ( CNetChan *netchan, const char *reason ) );

void VFUNC CNetChan_Shutdown_D( CNetChan *netchan, const char *reason )
{
	INIT_HOOK( "PreNetChannelShutdown" );
		DO_HOOK( PUSH_META( netchan, CNetChan ) );
		
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

	CNetChan_Shutdown_O( netchan, reason );

	INIT_HOOK( "PostNetChannelShutdown" );
	CALL_HOOK( 0 );
}

SIMPLE_VHOOK_BINDING( CNetChan, CNetChan_Shutdown, 36 );

DEFVFUNC_( INetChannelHandler_ConnectionStart_O, void, ( INetChannelHandler *handler, CNetChan *netchan ) );

void VFUNC INetChannelHandler_ConnectionStart_D( INetChannelHandler *handler, CNetChan *netchan )
{
	INIT_HOOK( "INetChannelHandler::ConnectionStart" );
		DO_HOOK( PUSH_META( handler, INetChannelHandler ); );
		DO_HOOK( PUSH_META( netchan, CNetChan ) );
	CALL_HOOK( 0 );

	INetChannelHandler_ConnectionStart_O( handler, netchan );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_ConnectionStart, 1 );

DEFVFUNC_( INetChannelHandler_ConnectionClosing_O, void, ( INetChannelHandler *handler, const char *reason ) );

void VFUNC INetChannelHandler_ConnectionClosing_D( INetChannelHandler *handler, const char *reason )
{
	INIT_HOOK( "INetChannelHandler::ConnectionClosing" );
		DO_HOOK( PUSH_META( handler, INetChannelHandler ); );
		DO_HOOK( LUA->PushString( reason ) );
	CALL_HOOK( 0 );

	INetChannelHandler_ConnectionClosing_O( handler, reason );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_ConnectionClosing, 2 );

DEFVFUNC_( INetChannelHandler_ConnectionCrashed_O, void, ( INetChannelHandler *handler, const char *reason ) );

void VFUNC INetChannelHandler_ConnectionCrashed_D( INetChannelHandler *handler, const char *reason )
{
	INIT_HOOK( "INetChannelHandler::ConnectionCrashed" );
		DO_HOOK( PUSH_META( handler, INetChannelHandler ) );
		DO_HOOK( LUA->PushString( reason ) );
	CALL_HOOK( 0 );

	INetChannelHandler_ConnectionCrashed_O( handler, reason );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_ConnectionCrashed, 3 );

DEFVFUNC_( INetChannelHandler_PacketStart_O, void, ( INetChannelHandler *handler, int incoming_sequence, int outgoing_acknowledged ) );

void VFUNC INetChannelHandler_PacketStart_D( INetChannelHandler *handler, int incoming_sequence, int outgoing_acknowledged )
{
	INIT_HOOK( "INetChannelHandler::PacketStart" );
		DO_HOOK( PUSH_META( handler, INetChannelHandler ) );
		DO_HOOK( LUA->PushNumber( incoming_sequence ) );
		DO_HOOK( LUA->PushNumber( incoming_sequence ) );
	CALL_HOOK( 0 );

	INetChannelHandler_PacketStart_O( handler, incoming_sequence, outgoing_acknowledged );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_PacketStart, 4 );

DEFVFUNC_( INetChannelHandler_PacketEnd_O, void, ( INetChannelHandler *handler ) );

void VFUNC INetChannelHandler_PacketEnd_D( INetChannelHandler *handler )
{
	INIT_HOOK( "INetChannelHandler::PacketEnd" );
		DO_HOOK( PUSH_META( handler, INetChannelHandler ); );
	CALL_HOOK( 0 );

	INetChannelHandler_PacketEnd_O( handler );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_PacketEnd, 5 );

DEFVFUNC_( INetChannelHandler_FileRequested_O, void, ( INetChannelHandler *handler, const char *fileName, uint32_t transferID ) );

void VFUNC INetChannelHandler_FileRequested_D( INetChannelHandler *handler, const char *fileName, uint32_t transferID )
{
	INIT_HOOK( "INetChannelHandler::FileRequested" );
		DO_HOOK( PUSH_META( handler, INetChannelHandler ); );
		DO_HOOK( LUA->PushString( fileName ) );
		DO_HOOK( LUA->PushNumber( transferID ) );
	CALL_HOOK( 0 );

	INetChannelHandler_FileRequested_O( handler, fileName, transferID );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_FileRequested, 6 );

DEFVFUNC_( INetChannelHandler_FileReceived_O, void, ( INetChannelHandler *handler, const char *fileName, uint32_t transferID ) );

void VFUNC INetChannelHandler_FileReceived_D( INetChannelHandler *handler, const char *fileName, uint32_t transferID )
{
	INIT_HOOK( "INetChannelHandler::FileReceived" );
		DO_HOOK( PUSH_META( handler, INetChannelHandler ); );
		DO_HOOK( LUA->PushString( fileName ) );
		DO_HOOK( LUA->PushNumber( transferID ) );
	CALL_HOOK( 0 );

	INetChannelHandler_FileReceived_O( handler, fileName, transferID );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_FileReceived, 7 );

DEFVFUNC_( INetChannelHandler_FileDenied_O, void, ( INetChannelHandler *handler, const char *fileName, uint32_t transferID ) );

void VFUNC INetChannelHandler_FileDenied_D( INetChannelHandler *handler, const char *fileName, uint32_t transferID )
{
	INIT_HOOK( "INetChannelHandler::FileDenied" );
		DO_HOOK( PUSH_META( handler, INetChannelHandler ); );
		DO_HOOK( LUA->PushString( fileName ) );
		DO_HOOK( LUA->PushNumber( transferID ) );
	CALL_HOOK( 0 );

	INetChannelHandler_FileDenied_O( handler, fileName, transferID );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_FileDenied, 8 );

static MologieDetours::Detour<CNetChan_ProcessMessages_T> *CNetChan_ProcessMessages_detour = nullptr;

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

		int bitsread = buf.GetNumBitsRead( );
		bf_write write( data, sizeof( data ) );
		write.SeekToBit( bitsread );

		INIT_HOOK( "PreProcessMessages" );
			DO_HOOK( PUSH_META( netchan, CNetChan ) );
			DO_HOOK( PUSH_META( &buf, sn4_bf_read ) );
			DO_HOOK( PUSH_META( &write, sn4_bf_write ) );

			if( !g_pEngineServer->IsDedicatedServer( ) )
				DO_HOOK( PUSH_META( g_pEngineClient->GetNetChannelInfo( ), CNetChan ) );

		CALL_HOOK( 0 );

		buf.StartReading( write.GetBasePointer( ), write.GetNumBytesWritten( ), bitsread, write.GetNumBitsWritten( ) );
	}

	return CNetChan_ProcessMessages_detour->GetOriginalFunction( )( netchan, buf );
}

MONITOR_HOOK( CNetChan_ProcessMessages );

GLBL_FUNCTION( Attach__CNetChan_ProcessMessages )
{
	if( CNetChan_ProcessMessages_O == nullptr )
		LUA->ThrowError( "hooking to CNetChan::ProcessMessages is disabled as the signature scan failed" );

	if( !IS_ATTACHED( CNetChan_ProcessMessages ) )
	{
		CNetChan_ProcessMessages_detour = new MologieDetours::Detour<CNetChan_ProcessMessages_T>(
			CNetChan_ProcessMessages_O, reinterpret_cast<CNetChan_ProcessMessages_T>( CNetChan_ProcessMessages_D )
		);

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