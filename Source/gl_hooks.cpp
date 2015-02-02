// Required interfaces
#define IVENGINESERVER_INTERFACE
#define IVENGINECLIENT_INTERFACE
#define ICVAR_INTERFACE

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
			HOOKVFUNC( GET_META( 1, meta ), offset, name##_T, name##_H ); \
			REGISTER_ATTACH( name ); \
		} \
		return 0; \
	} \
	GLBL_FUNCTION( Detach__##name ) \
	{ \
		LUA->CheckType( 1, GET_META_ID( meta ) ); \
		if( IS_ATTACHED( name ) ) \
		{ \
			UNHOOKVFUNC( GET_META( 1, meta ), offset, name##_T ); \
			REGISTER_DETACH( name ); \
		} \
		return 0; \
	}

DEFVFUNC_( CNetChan_SendDatagram_T, int, ( CNetChan *netchan, sn4_bf_write *data ) );

int VFUNC CNetChan_SendDatagram_H( CNetChan *netchan, sn4_bf_write *data )
{
	BEGIN_MULTISTATE_HOOK( "PreSendDatagram" );
		
	DO_MULTISTATE_HOOK( PUSH_META( netchan, CNetChan ) );

	if( !g_pEngineServer->IsDedicatedServer( ) )
	{
		DO_MULTISTATE_HOOK( PUSH_META( g_pEngineClient->GetNetChannelInfo( ), CNetChan ) );
	}
	else
	{
		DO_MULTISTATE_HOOK( LUA->PushNil( ) );
	}

	DO_MULTISTATE_HOOK( PUSH_META( data, sn4_bf_write ) );
	DO_MULTISTATE_HOOK( PUSH_META( &netchan->reliabledata, sn4_bf_write ) );
	DO_MULTISTATE_HOOK( PUSH_META( &netchan->unreliabledata, sn4_bf_write ) );
	DO_MULTISTATE_HOOK( PUSH_META( &netchan->voicedata, sn4_bf_write ) );

	CALL_MULTISTATE_HOOK( 0 );
	END_MULTISTATE_HOOK( );

	int r = CNetChan_SendDatagram_T( netchan, data );

	BEGIN_MULTISTATE_HOOK( "PostSendDatagram" );
	
	DO_MULTISTATE_HOOK( PUSH_META( netchan, CNetChan ) );

	CALL_MULTISTATE_HOOK( 0 );
	
	END_MULTISTATE_HOOK( );

	return r;
}

#ifdef _WIN32
SIMPLE_VHOOK_BINDING( CNetChan, CNetChan_SendDatagram, 46 );
#else
SIMPLE_VHOOK_BINDING( CNetChan, CNetChan_SendDatagram, 47 );
#endif

DEFVFUNC_( CNetChan_ProcessPacket_T, void, ( CNetChan *netchan, netpacket_t *packet, bool bHasHeader ) );

void VFUNC CNetChan_ProcessPacket_H( CNetChan *netchan, netpacket_t *packet, bool bHasHeader )
{
	BEGIN_MULTISTATE_HOOK( "PreProcessPacket" );
		DO_MULTISTATE_HOOK( PUSH_META( netchan, CNetChan ) );

		CALL_MULTISTATE_HOOK( 0 );
	END_MULTISTATE_HOOK();

	CNetChan_ProcessPacket_T( netchan, packet, bHasHeader );

	BEGIN_MULTISTATE_HOOK( "PostProcessPacket" );
		DO_MULTISTATE_HOOK( PUSH_META( netchan, CNetChan ) );

		CALL_MULTISTATE_HOOK( 0 );
	END_MULTISTATE_HOOK( );
}

#ifdef _WIN32
SIMPLE_VHOOK_BINDING( CNetChan, CNetChan_ProcessPacket, 39 );
#else
SIMPLE_VHOOK_BINDING( CNetChan, CNetChan_ProcessPacket, 40 ); // ?????
#endif

DEFVFUNC_( CNetChan_Shutdown_T, void, ( CNetChan *netchan, const char *reason ) );

void VFUNC CNetChan_Shutdown_H( CNetChan *netchan, const char *reason )
{
	BEGIN_MULTISTATE_HOOK( "PreNetChannelShutdown" );
		DO_MULTISTATE_HOOK( PUSH_META( netchan, CNetChan ) );
		
		// Fuck Linux
		if( reason && reason != reinterpret_cast<const char *>( 0x02 ) )
		{
			DO_MULTISTATE_HOOK( LUA->PushString( reason ) );
		}
		else
		{
			DO_MULTISTATE_HOOK( LUA->PushNil( ) );
		}
		
		CALL_MULTISTATE_HOOK( 0 );
	END_MULTISTATE_HOOK( );

	CNetChan_Shutdown_T( netchan, reason );

	BEGIN_MULTISTATE_HOOK( "PostNetChannelShutdown" );
		CALL_MULTISTATE_HOOK( 0 );
	END_MULTISTATE_HOOK( );
}

SIMPLE_VHOOK_BINDING( CNetChan, CNetChan_Shutdown, 36 );

DEFVFUNC_( INetChannelHandler_ConnectionStart_T, void, ( INetChannelHandler *handler, CNetChan *netchan ) );

void VFUNC INetChannelHandler_ConnectionStart_H( INetChannelHandler *handler, CNetChan *netchan )
{
	BEGIN_MULTISTATE_HOOK( "INetChannelHandler::ConnectionStart" );
		DO_MULTISTATE_HOOK( PUSH_META( handler, INetChannelHandler ); );
		DO_MULTISTATE_HOOK( PUSH_META( netchan, CNetChan ) );
		
		CALL_MULTISTATE_HOOK( 0 );
	END_MULTISTATE_HOOK( );

	INetChannelHandler_ConnectionStart_T( handler, netchan );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_ConnectionStart, 1 );

DEFVFUNC_( INetChannelHandler_ConnectionClosing_T, void, ( INetChannelHandler *handler, const char *reason ) );

void VFUNC INetChannelHandler_ConnectionClosing_H( INetChannelHandler *handler, const char *reason )
{
	BEGIN_MULTISTATE_HOOK( "INetChannelHandler::ConnectionClosing" );
		DO_MULTISTATE_HOOK( PUSH_META( handler, INetChannelHandler ); );
		DO_MULTISTATE_HOOK( LUA->PushString( reason ) );
		
		CALL_MULTISTATE_HOOK( 0 );
	END_MULTISTATE_HOOK( );

	INetChannelHandler_ConnectionClosing_T( handler, reason );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_ConnectionClosing, 2 );

DEFVFUNC_( INetChannelHandler_ConnectionCrashed_T, void, ( INetChannelHandler *handler, const char *reason ) );

void VFUNC INetChannelHandler_ConnectionCrashed_H( INetChannelHandler *handler, const char *reason )
{
	BEGIN_MULTISTATE_HOOK( "INetChannelHandler::ConnectionCrashed" );
		DO_MULTISTATE_HOOK( PUSH_META( handler, INetChannelHandler ) );
		DO_MULTISTATE_HOOK( LUA->PushString( reason ) );
		
		CALL_MULTISTATE_HOOK( 0 );
	END_MULTISTATE_HOOK( );

	INetChannelHandler_ConnectionCrashed_T( handler, reason );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_ConnectionCrashed, 3 );

DEFVFUNC_( INetChannelHandler_PacketStart_T, void, ( INetChannelHandler *handler, int incoming_sequence, int outgoing_acknowledged ) );

void VFUNC INetChannelHandler_PacketStart_H( INetChannelHandler *handler, int incoming_sequence, int outgoing_acknowledged )
{
	BEGIN_MULTISTATE_HOOK( "INetChannelHandler::PacketStart" );
		DO_MULTISTATE_HOOK( PUSH_META( handler, INetChannelHandler ) );
		DO_MULTISTATE_HOOK( LUA->PushNumber( incoming_sequence ) );
		DO_MULTISTATE_HOOK( LUA->PushNumber( incoming_sequence ) );
		
		CALL_MULTISTATE_HOOK( 0 );
	END_MULTISTATE_HOOK( );

	INetChannelHandler_PacketStart_T( handler, incoming_sequence, outgoing_acknowledged );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_PacketStart, 4 );

DEFVFUNC_( INetChannelHandler_PacketEnd_T, void, ( INetChannelHandler *handler ) );

void VFUNC INetChannelHandler_PacketEnd_H( INetChannelHandler *handler )
{
	BEGIN_MULTISTATE_HOOK( "INetChannelHandler::PacketEnd" );
		DO_MULTISTATE_HOOK( PUSH_META( handler, INetChannelHandler ); );
		
		CALL_MULTISTATE_HOOK( 0 );
	END_MULTISTATE_HOOK( );

	INetChannelHandler_PacketEnd_T( handler );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_PacketEnd, 5 );

DEFVFUNC_( INetChannelHandler_FileRequested_T, void, ( INetChannelHandler *handler, const char *fileName, unsigned int transferID ) );

void VFUNC INetChannelHandler_FileRequested_H( INetChannelHandler *handler, const char *fileName, unsigned int transferID )
{
	BEGIN_MULTISTATE_HOOK( "INetChannelHandler::FileRequested" );
		DO_MULTISTATE_HOOK( PUSH_META( handler, INetChannelHandler ); );
		DO_MULTISTATE_HOOK( LUA->PushString( fileName ) );
		DO_MULTISTATE_HOOK( LUA->PushNumber( transferID ) );

		CALL_MULTISTATE_HOOK( 0 );
	END_MULTISTATE_HOOK( );

	INetChannelHandler_FileRequested_T( handler, fileName, transferID );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_FileRequested, 6 );

DEFVFUNC_( INetChannelHandler_FileReceived_T, void, ( INetChannelHandler *handler, const char *fileName, unsigned int transferID ) );

void VFUNC INetChannelHandler_FileReceived_H( INetChannelHandler *handler, const char *fileName, unsigned int transferID )
{
	BEGIN_MULTISTATE_HOOK( "INetChannelHandler::FileReceived" );
		DO_MULTISTATE_HOOK( PUSH_META( handler, INetChannelHandler ); );
		DO_MULTISTATE_HOOK( LUA->PushString( fileName ) );
		DO_MULTISTATE_HOOK( LUA->PushNumber( transferID ) );

		CALL_MULTISTATE_HOOK( 0 );
	END_MULTISTATE_HOOK();

	INetChannelHandler_FileReceived_T( handler, fileName, transferID );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_FileReceived, 7 );

DEFVFUNC_( INetChannelHandler_FileDenied_T, void, ( INetChannelHandler *handler, const char *fileName, unsigned int transferID ) );

void VFUNC INetChannelHandler_FileDenied_H( INetChannelHandler *handler, const char *fileName, unsigned int transferID )
{
	BEGIN_MULTISTATE_HOOK( "INetChannelHandler::FileDenied" );
		DO_MULTISTATE_HOOK( PUSH_META( handler, INetChannelHandler ); );
		DO_MULTISTATE_HOOK( LUA->PushString( fileName ) );
		DO_MULTISTATE_HOOK( LUA->PushNumber( transferID ) );

		CALL_MULTISTATE_HOOK( 0 );
	END_MULTISTATE_HOOK( );

	INetChannelHandler_FileDenied_T( handler, fileName, transferID );
}

SIMPLE_VHOOK_BINDING( INetChannelHandler, INetChannelHandler_FileDenied, 8 );

// Non-virtual

#include <icvar.h>
#include <convar.h>

ConVar *net_showmsg = nullptr, *net_blockmsg = nullptr, *net_showpeaks = nullptr;

#if defined _WIN32

bool __stdcall CNetChan_ProcessMessages_H( sn4_bf_read &buf )

#else

bool CNetChan_ProcessMessages_H( CNetChan *netchan, sn4_bf_read &buf )

#endif

{
#if defined _WIN32
	CNetChan *netchan = nullptr;

	__asm MOV netchan, ECX
#endif
	if( !buf.IsOverflowed( ) )
	{
		int bitsread = buf.GetNumBitsRead( );

		uint8_t data[96000];
		memcpy( data, buf.m_pData, BitByte( bitsread ) );

		bf_write write( data, sizeof( data ) );

		BEGIN_MULTISTATE_HOOK( "PreProcessMessages" );
			write.SeekToBit( bitsread );

			DO_MULTISTATE_HOOK( PUSH_META( netchan, CNetChan ) );
			DO_MULTISTATE_HOOK( PUSH_META( &buf, sn4_bf_read ) );
			DO_MULTISTATE_HOOK( PUSH_META( &write, sn4_bf_write ) );

			if( !g_pEngineServer->IsDedicatedServer( ) )
				DO_MULTISTATE_HOOK( PUSH_META( g_pEngineClient->GetNetChannelInfo( ), CNetChan ) );

			CALL_MULTISTATE_HOOK( 0 );

			buf.StartReading( write.GetBasePointer( ), write.GetNumBytesWritten( ), bitsread );
		END_MULTISTATE_HOOK( );
	}

	const char *showmsg = net_showmsg->GetString( ); // ESP+14
	const char *blockmsg = net_blockmsg->GetString( ); // ESP+18
	
	if( *showmsg == '0' )
		showmsg = NULL;

	if( *blockmsg == '0' )
		blockmsg = NULL;
	
	if( net_showpeaks->GetInt( ) && buf.GetNumBytesLeft( ) < net_showpeaks->GetInt( ) )
		showmsg = "1";

	while( true )
	{
		if( buf.IsOverflowed( ) )
		{
			netchan->msghandler->ConnectionCrashed( "Buffer overflow in net message" );
			return false;
		}

		if( buf.GetNumBitsLeft( ) < NET_MESSAGE_BITS )
			break;

		unsigned char msg = buf.ReadUBitLong( NET_MESSAGE_BITS );

		if ( msg <= net_LastControlMessage )
		{
			if ( !netchan->ProcessControlMessage( msg, buf ) )
				return false;

			continue;
		}

		INetMessage *netmsg = netchan->FindMessage( msg );
		if( netmsg == nullptr )
		{
			ConMsg( "Netchannel: unknown net message (%i) from %s.\n", msg, netchan->remote_address.ToString( ) );
			return false;
		}

		int oldpos = buf.GetNumBitsRead( );
		if( !netmsg->ReadFromBuffer( buf ) )
		{
			ConMsg( "Netchannel: failed reading message %s from %s.\n", netmsg->GetName( ), netchan->remote_address.ToString( ) );
			return false;
		}

		netchan->UpdateMessageStats( netmsg->GetGroup( ), buf.GetNumBitsRead( ) - oldpos );

		if( showmsg && ( *showmsg == '1' || !V_strcasecmp( showmsg, netmsg->GetName( ) ) ) )
			ConMsg( "Msg from %s: %s\n", netchan->remote_address.ToString( ), netmsg->ToString( ) );

		if( blockmsg && ( *blockmsg == '1' || !V_strcasecmp( blockmsg, netmsg->GetName( ) ) ) )
		{
			ConMsg( "Blocking message: %s\n", netmsg->ToString( ) );
		}
		else
		{
			netchan->process_state = true;

			bool processed = netmsg->Process( );

			netchan->process_state = false;

			if( netchan->fatal_error )
			{
				delete netchan;
				return false;
			}

			if( !processed )
			{
				ConDMsg( "Netchannel: failed processing message %s.\n", netmsg->GetName( ) );
				return false;
			}

			if( netchan->IsOverflowed( ) )
				return false;
		}
	}

	return true;
}

unsigned char oldbytes[1 + sizeof( DWORD )];
unsigned char newbytes[1 + sizeof( DWORD )] = { 0xE9, 0x00, 0x00, 0x00, 0x00 };

MONITOR_HOOK( CNetChan_ProcessMessages );

GLBL_FUNCTION( Attach__CNetChan_ProcessMessages )
{
	if( CNetChan_ProcessMessages_T == nullptr )
		LUA->ThrowError( "hooking to CNetChan::ProcessMessages is disabled as the signature scan failed" );

	ICvar *g_pCVar = nullptr;

	if ( LUA->IsClient( ) )
		g_pCVar = g_pCVarClient;
	else
		g_pCVar = g_pCVarServer;

	if( !IS_ATTACHED( CNetChan_ProcessMessages ) )
	{
		net_showmsg = g_pCVar->FindVar( "net_showmsg" );
		if( net_showmsg == nullptr )
		{
			Msg( "[gm_sourcenet3] net_showmsg not found\n" );
			return 0;
		}

		net_blockmsg = g_pCVar->FindVar( "net_blockmsg" );
		if( net_blockmsg == nullptr )
		{
			Msg( "[gm_sourcenet3] net_blockmsg not found\n" );
			return 0;
		}

		net_showpeaks = g_pCVar->FindVar( "net_showpeaks" );
		if( net_showpeaks == nullptr )
		{
			Msg( "[gm_sourcenet3] net_showpeaks not found\n" );
			return 0;
		}

		uintptr_t jmppos = reinterpret_cast<uintptr_t>( CNetChan_ProcessMessages_H ) - reinterpret_cast<uintptr_t>( CNetChan_ProcessMessages_T ) - 5;

		memcpy( newbytes + 1, &jmppos, sizeof( uintptr_t ) );

		BEGIN_MEMEDIT( CNetChan_ProcessMessages_T, sizeof( newbytes ) );
			memcpy( oldbytes, CNetChan_ProcessMessages_T, sizeof( oldbytes ) );
			memcpy( CNetChan_ProcessMessages_T, newbytes, sizeof( newbytes ) );
		FINISH_MEMEDIT( CNetChan_ProcessMessages_T, sizeof( newbytes ) );

		REGISTER_ATTACH( CNetChan_ProcessMessages );
	}

	return 0;
}

GLBL_FUNCTION( Detach__CNetChan_ProcessMessages )
{
	if( IS_ATTACHED( CNetChan_ProcessMessages ) )
	{
		BEGIN_MEMEDIT( CNetChan_ProcessMessages_T, sizeof( oldbytes ) );
			memcpy( CNetChan_ProcessMessages_T, oldbytes, sizeof( oldbytes ) );	
		FINISH_MEMEDIT( CNetChan_ProcessMessages_T, sizeof( oldbytes ) );

		REGISTER_DETACH( CNetChan_ProcessMessages );
	}

	return 0;
}
