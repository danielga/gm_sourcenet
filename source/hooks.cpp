#include "hooks.hpp"
#include "netchannel.hpp"
#include "netchannelhandler.hpp"
#include "netmessage.hpp"
#include "sn_bf_read.hpp"
#include "sn_bf_write.hpp"
#include "net.hpp"

#include <GarrysMod/Lua/Helpers.hpp>
#include <GarrysMod/FunctionPointers.hpp>
#include <Platform.hpp>

#include <detouring/classproxy.hpp>

#include <inetmsghandler.h>
#include <cdll_int.h>

namespace Hooks
{

#define HOOK_INIT( name ) \
	do \
	{ \
		GarrysMod::Lua::ILuaBase *LUA = global::lua; \
		if( !LuaHelpers::PushHookRun( LUA, name ) ) \
			break; \
		int32_t _argc = 0

	#define HOOK_PUSH( code ) \
		code; \
		++_argc

	#define HOOK_CALL( returns ) \
		LuaHelpers::CallHookRun( LUA, _argc, returns )

	#define HOOK_END( ) \
	} \
	while( false )

#define VIRTUAL_FUNCTION_SETUP( ret, name, ... ) \
	static bool Attach##name( TargetClass *temp ) \
	{ \
		if( !Detouring::ClassProxy<TargetClass, SubstituteClass>::Initialize( temp, &Singleton ) ) \
			return false; \
		return Hook( &TargetClass::name, &SubstituteClass::name ); \
	} \
	LUA_FUNCTION_IMPLEMENT( LuaAttach##name ) \
	{ \
		TargetClass *temp = LuaGet( LUA, 1 ); \
		LUA->PushBool( Attach##name( temp ) ); \
		return 1; \
	} \
	LUA_FUNCTION_WRAP( LuaAttach##name ); \
	static bool Detach##name( ) \
	{ \
		return UnHook( &TargetClass::name ); \
	} \
	LUA_FUNCTION_IMPLEMENT( LuaDetach##name ) \
	{ \
		LUA->PushBool( Detach##name( ) ); \
		return 1; \
	} \
	LUA_FUNCTION_WRAP( LuaDetach##name ); \
	virtual ret name( __VA_ARGS__ )

	class CNetChanProxy : public Detouring::ClassProxy<CNetChan, CNetChanProxy>
	{
	private:
		typedef CNetChan TargetClass;
		typedef CNetChanProxy SubstituteClass;

		static CNetChan *LuaGet( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
		{
			return NetChannel::Get( LUA, index );
		}

		static FunctionPointers::CNetChan_ProcessMessages_t ProcessMessages_original;

	public:
		static void Initialize( GarrysMod::Lua::ILuaBase *LUA )
		{
			ProcessMessages_original = FunctionPointers::CNetChan_ProcessMessages( );
			if( ProcessMessages_original == nullptr )
				LUA->ThrowError( "failed to locate CNetChan::ProcessMessages" );
		}

		VIRTUAL_FUNCTION_SETUP( int32_t, SendDatagram, bf_write *data )
		{
			CNetChan *netchan = This( );

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

			int32_t r = Call( &CNetChan::SendDatagram, data );

			HOOK_INIT( "PostSendDatagram" );
			HOOK_PUSH( NetChannel::Push( LUA, netchan ) );
			HOOK_CALL( 0 );
			HOOK_END( );

			return r;
		}

		VIRTUAL_FUNCTION_SETUP( void, ProcessPacket, netpacket_t *packet, bool bHasHeader )
		{
			CNetChan *netchan = This( );

			HOOK_INIT( "PreProcessPacket" );
			HOOK_PUSH( NetChannel::Push( LUA, netchan ) );
			HOOK_CALL( 0 );
			HOOK_END( );

			Call( &CNetChan::ProcessPacket, packet, bHasHeader );

			HOOK_INIT( "PostProcessPacket" );
			HOOK_PUSH( NetChannel::Push( LUA, netchan ) );
			HOOK_CALL( 0 );
			HOOK_END( );
		}

		VIRTUAL_FUNCTION_SETUP( void, Shutdown, const char *reason )
		{
			CNetChan *netchan = This( );

			HOOK_INIT( "PreNetChannelShutdown" );
			HOOK_PUSH( NetChannel::Push( LUA, netchan ) );
			HOOK_PUSH( LUA->PushString( reason ) );
			HOOK_CALL( 0 );
			HOOK_END( );

			NetMessage::Destroy( global::lua, netchan );

			NetChannel::Destroy( global::lua, netchan );

			Call( &CNetChan::Shutdown, reason );

			HOOK_INIT( "PostNetChannelShutdown" );
			HOOK_CALL( 0 );
			HOOK_END( );
		}

		LUA_FUNCTION_IMPLEMENT( LuaAttachProcessMessages )
		{
			LUA->PushBool( Hook( ProcessMessages_original, &CNetChanProxy::ProcessMessages ) );
			return 1;
		}

		LUA_FUNCTION_WRAP( LuaAttachProcessMessages );

		static bool DetachProcessMessages( )
		{
			return UnHook( ProcessMessages_original );
		}

		LUA_FUNCTION_IMPLEMENT( LuaDetachProcessMessages )
		{
			LUA->PushBool( DetachProcessMessages( ) );
			return 1;
		}

		LUA_FUNCTION_WRAP( LuaDetachProcessMessages );

		bool ProcessMessages( bf_read &buf )
		{
			CNetChan *netchan = This( );

			if( !buf.IsOverflowed( ) )
			{
				static uint8_t data[100000] = { 0 };

				const int32_t bytesread = buf.GetNumBytesRead( );
				if( bytesread > 0 )
					std::memcpy( data, buf.GetBasePointer( ), bytesread );

				const int32_t bitsread = buf.GetNumBitsRead( );
				bf_write write( data, sizeof( data ) );
				write.SeekToBit( bitsread );

				bool handled = false;
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

				if( HOOK_CALL( 1 ) )
				{
					if( LUA->IsType( -1, GarrysMod::Lua::Type::BOOL ) )
						handled = LUA->GetBool( -1 );

					LUA->Pop( 1 );
				}

				*reader = nullptr;
				*writer = nullptr;
				HOOK_END( );

				if( handled )
					buf.StartReading(
						write.GetBasePointer( ),
						write.GetNumBytesWritten( ),
						bitsread,
						write.GetNumBitsWritten( )
					);
				else
					buf.Seek( bitsread );
			}

			return Call( ProcessMessages_original, buf );
		}

		bool HookShutdown( CNetChan *netchan )
		{
			if( !Detouring::ClassProxy<CNetChan, CNetChanProxy>::Initialize( netchan ) )
				return false;

			return Hook( &CNetChan::Shutdown, &CNetChanProxy::Shutdown );
		}

		static CNetChanProxy Singleton;
	};

	CNetChanProxy CNetChanProxy::Singleton;

	FunctionPointers::CNetChan_ProcessMessages_t CNetChanProxy::ProcessMessages_original = nullptr;

	class INetChannelHandlerProxy : public Detouring::ClassProxy<INetChannelHandler, INetChannelHandlerProxy>
	{
	private:
		typedef INetChannelHandler TargetClass;
		typedef INetChannelHandlerProxy SubstituteClass;

		static INetChannelHandler *LuaGet( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
		{
			return NetChannelHandler::Get( LUA, index );
		}

	public:
		VIRTUAL_FUNCTION_SETUP( void, ConnectionStart, INetChannel *netchannel )
		{
			INetChannelHandler *handler = This( );

			HOOK_INIT( "INetChannelHandler::ConnectionStart" );
			HOOK_PUSH( NetChannelHandler::Push( LUA, handler ) );
			HOOK_PUSH( NetChannel::Push( LUA, static_cast<CNetChan *>( netchannel ) ) );
			HOOK_CALL( 0 );
			HOOK_END( );

			Call( &INetChannelHandler::ConnectionStart, netchannel );
		}

		VIRTUAL_FUNCTION_SETUP( void, ConnectionClosing, const char *reason )
		{
			INetChannelHandler *handler = This( );

			HOOK_INIT( "INetChannelHandler::ConnectionClosing" );
			HOOK_PUSH( NetChannelHandler::Push( LUA, handler ) );
			HOOK_PUSH( LUA->PushString( reason ) );
			HOOK_CALL( 0 );
			HOOK_END( );

			NetChannelHandler::Destroy( global::lua, handler );

			Call( &INetChannelHandler::ConnectionClosing, reason );
		}

		VIRTUAL_FUNCTION_SETUP( void, ConnectionCrashed, const char *reason )
		{
			INetChannelHandler *handler = This( );

			HOOK_INIT( "INetChannelHandler::ConnectionCrashed" );
			HOOK_PUSH( NetChannelHandler::Push( LUA, handler ) );
			HOOK_PUSH( LUA->PushString( reason ) );
			HOOK_CALL( 0 );
			HOOK_END( );

			NetChannelHandler::Destroy( global::lua, handler );

			Call( &INetChannelHandler::ConnectionCrashed, reason );
		}

		VIRTUAL_FUNCTION_SETUP( void, PacketStart, int32_t incoming_sequence, int32_t outgoing_acknowledged )
		{
			INetChannelHandler *handler = This( );

			HOOK_INIT( "INetChannelHandler::PacketStart" );
			HOOK_PUSH( NetChannelHandler::Push( LUA, handler ) );
			HOOK_PUSH( LUA->PushNumber( incoming_sequence ) );
			HOOK_PUSH( LUA->PushNumber( outgoing_acknowledged ) );
			HOOK_CALL( 0 );
			HOOK_END( );

			Call( &INetChannelHandler::PacketStart, incoming_sequence, outgoing_acknowledged );
		}

		VIRTUAL_FUNCTION_SETUP( void, PacketEnd )
		{
			INetChannelHandler *handler = This( );

			HOOK_INIT( "INetChannelHandler::PacketEnd" );
			HOOK_PUSH( NetChannelHandler::Push( LUA, handler ) );
			HOOK_CALL( 0 );
			HOOK_END( );

			Call( &INetChannelHandler::PacketEnd );
		}

		VIRTUAL_FUNCTION_SETUP( void, FileRequested, const char *fileName, uint32_t transferID )
		{
			INetChannelHandler *handler = This( );

			HOOK_INIT( "INetChannelHandler::FileRequested" );
			HOOK_PUSH( NetChannelHandler::Push( LUA, handler ) );
			HOOK_PUSH( LUA->PushString( fileName ) );
			HOOK_PUSH( LUA->PushNumber( transferID ) );
			HOOK_CALL( 0 );
			HOOK_END( );

			Call( &INetChannelHandler::FileRequested, fileName, transferID );
		}

		VIRTUAL_FUNCTION_SETUP( void, FileReceived, const char *fileName, uint32_t transferID )
		{
			INetChannelHandler *handler = This( );

			HOOK_INIT( "INetChannelHandler::FileReceived" );
			HOOK_PUSH( NetChannelHandler::Push( LUA, handler ) );
			HOOK_PUSH( LUA->PushString( fileName ) );
			HOOK_PUSH( LUA->PushNumber( transferID ) );
			HOOK_CALL( 0 );
			HOOK_END( );

			Call( &INetChannelHandler::FileReceived, fileName, transferID );
		}

		VIRTUAL_FUNCTION_SETUP( void, FileDenied, const char *fileName, uint32_t transferID )
		{
			INetChannelHandler *handler = This( );

			HOOK_INIT( "INetChannelHandler::FileDenied" );
			HOOK_PUSH( NetChannelHandler::Push( LUA, handler ) );
			HOOK_PUSH( LUA->PushString( fileName ) );
			HOOK_PUSH( LUA->PushNumber( transferID ) );
			HOOK_CALL( 0 );
			HOOK_END( );

			Call( &INetChannelHandler::FileDenied, fileName, transferID );
		}

		bool HookConnectionClosing( INetChannelHandler *handler )
		{
			if( !Detouring::ClassProxy<INetChannelHandler, INetChannelHandlerProxy>::Initialize( handler ) )
				return false;

			return Hook(
				&INetChannelHandler::ConnectionClosing, &INetChannelHandlerProxy::ConnectionClosing
			);
		}

		bool HookConnectionCrashed( INetChannelHandler *handler )
		{
			if( !Detouring::ClassProxy<INetChannelHandler, INetChannelHandlerProxy>::Initialize( handler ) )
				return false;

			return Hook(
				&INetChannelHandler::ConnectionCrashed, &INetChannelHandlerProxy::ConnectionCrashed
			);
		}

		static INetChannelHandlerProxy Singleton;
	};

	INetChannelHandlerProxy INetChannelHandlerProxy::Singleton;

	LUA_FUNCTION_STATIC( Empty )
	{
		return 0;
	}

	void PreInitialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		CNetChanProxy::Initialize( LUA );
	}

	void Initialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		LUA->PushCFunction( CNetChanProxy::LuaAttachProcessPacket );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__CNetChan_ProcessPacket" );

		LUA->PushCFunction( CNetChanProxy::LuaDetachProcessPacket );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__CNetChan_ProcessPacket" );

		LUA->PushCFunction( CNetChanProxy::LuaAttachSendDatagram );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__CNetChan_SendDatagram" );

		LUA->PushCFunction( CNetChanProxy::LuaDetachSendDatagram );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__CNetChan_SendDatagram" );

		LUA->PushCFunction( CNetChanProxy::LuaAttachProcessPacket );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__CNetChan_ProcessPacket" );

		LUA->PushCFunction( CNetChanProxy::LuaDetachProcessPacket );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__CNetChan_ProcessPacket" );

		LUA->PushCFunction( CNetChanProxy::LuaAttachShutdown );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__CNetChan_Shutdown" );

		LUA->PushCFunction( Empty );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__CNetChan_Shutdown" );

		LUA->PushCFunction( CNetChanProxy::LuaAttachProcessMessages );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__CNetChan_ProcessMessages" );

		LUA->PushCFunction( CNetChanProxy::LuaDetachProcessMessages );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__CNetChan_ProcessMessages" );

		LUA->PushCFunction( INetChannelHandlerProxy::LuaAttachConnectionStart );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_ConnectionStart" );

		LUA->PushCFunction( INetChannelHandlerProxy::LuaDetachConnectionStart );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_ConnectionStart" );

		LUA->PushCFunction( INetChannelHandlerProxy::LuaAttachConnectionClosing );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_ConnectionClosing" );

		LUA->PushCFunction( Empty );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_ConnectionClosing" );

		LUA->PushCFunction( INetChannelHandlerProxy::LuaAttachConnectionCrashed );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_ConnectionCrashed" );

		LUA->PushCFunction( Empty );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_ConnectionCrashed" );

		LUA->PushCFunction( INetChannelHandlerProxy::LuaAttachPacketStart );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_PacketStart" );

		LUA->PushCFunction( INetChannelHandlerProxy::LuaDetachPacketStart );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_PacketStart" );

		LUA->PushCFunction( INetChannelHandlerProxy::LuaAttachPacketEnd );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_PacketEnd" );

		LUA->PushCFunction( INetChannelHandlerProxy::LuaDetachPacketEnd );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_PacketEnd" );

		LUA->PushCFunction( INetChannelHandlerProxy::LuaAttachFileRequested );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_FileRequested" );

		LUA->PushCFunction( INetChannelHandlerProxy::LuaDetachFileRequested );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_FileRequested" );

		LUA->PushCFunction( INetChannelHandlerProxy::LuaAttachFileReceived );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_FileReceived" );

		LUA->PushCFunction( INetChannelHandlerProxy::LuaDetachFileReceived );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_FileReceived" );

		LUA->PushCFunction( INetChannelHandlerProxy::LuaAttachFileDenied );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_FileDenied" );

		LUA->PushCFunction( INetChannelHandlerProxy::LuaDetachFileDenied );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_FileDenied" );
	}

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		CNetChanProxy::DetachSendDatagram( );
		CNetChanProxy::DetachProcessPacket( );
		CNetChanProxy::DetachShutdown( );
		CNetChanProxy::DetachProcessMessages( );

		INetChannelHandlerProxy::DetachConnectionStart( );
		INetChannelHandlerProxy::DetachConnectionClosing( );
		INetChannelHandlerProxy::DetachConnectionCrashed( );
		INetChannelHandlerProxy::DetachPacketStart( );
		INetChannelHandlerProxy::DetachPacketEnd( );
		INetChannelHandlerProxy::DetachFileRequested( );
		INetChannelHandlerProxy::DetachFileReceived( );
		INetChannelHandlerProxy::DetachFileDenied( );



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
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Attach__INetChannelHandler_FileRequested" );

		LUA->PushNil( );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "Detach__INetChannelHandler_FileRequested" );

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

	void HookCNetChan( CNetChan *netchan )
	{
		CNetChanProxy::Singleton.HookShutdown( netchan );
	}

	void HookINetChannelHandler( INetChannelHandler *handler )
	{
		INetChannelHandlerProxy::Singleton.HookConnectionClosing( handler );
		INetChannelHandlerProxy::Singleton.HookConnectionCrashed( handler );
	}
}
