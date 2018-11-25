#include "hooks.hpp"
#include "netchannel.hpp"
#include "netchannelhandler.hpp"
#include "netmessage.hpp"
#include "sn_bf_read.hpp"
#include "sn_bf_write.hpp"
#include "net.hpp"

#include <inetmsghandler.h>
#include <cdll_int.h>
#include <scanning/symbolfinder.hpp>
#include <GarrysMod/Lua/Helpers.hpp>
#include <detouring/classproxy.hpp>
#include <Platform.hpp>

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
		Detouring::ClassProxy<TargetClass, SubstituteClass>::Initialize( temp, &Singleton ); \
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

	class CNetChanProxy : Detouring::ClassProxy<CNetChan, CNetChanProxy>
	{
	private:

#if defined SYSTEM_WINDOWS

		typedef bool( __thiscall *ProcessMessages_t )( CNetChan *netchan, bf_read &buf );

#else

		typedef bool( *ProcessMessages_t )( CNetChan *netchan, bf_read &buf );

#endif

		typedef CNetChan TargetClass;
		typedef CNetChanProxy SubstituteClass;

		static CNetChan *LuaGet( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
		{
			return NetChannel::Get( LUA, index );
		}

		static const char ProcessMessages_sig[];
		static const size_t ProcessMessages_siglen;

		static ProcessMessages_t ProcessMessages_original;

	public:
		static void Initialize( GarrysMod::Lua::ILuaBase *LUA )
		{
			{
				SymbolFinder symfinder;

				ProcessMessages_original =
					reinterpret_cast<ProcessMessages_t>( symfinder.ResolveOnBinary(
						global::engine_lib.c_str( ),
						ProcessMessages_sig,
						ProcessMessages_siglen
					) );
			}

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
			if( !Hook( ProcessMessages_original, &CNetChanProxy::ProcessMessages ) )
					LUA->ThrowError( "failed to hook CNetChan::ProcessMessages" );

			return 0;
		}

		LUA_FUNCTION_WRAP( LuaAttachProcessMessages );

		static bool DetachProcessMessages( )
		{
			return UnHook( ProcessMessages_original );
		}

		LUA_FUNCTION_IMPLEMENT( LuaDetachProcessMessages )
		{
			(void)LUA;

			if( !DetachProcessMessages( ) )
				LUA->ThrowError( "failed to unhook CNetChan::ProcessMessages" );

			return 0;
		}

		LUA_FUNCTION_WRAP( LuaDetachProcessMessages );

		bool ProcessMessages( bf_read &buf )
		{
			CNetChan *netchan = This( );

			static uint8_t data[100000] = { 0 };
			if( !buf.IsOverflowed( ) )
			{
				memcpy( data, buf.GetBasePointer( ), buf.GetNumBytesRead( ) );

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

			return Call<bool, bf_read &>( ProcessMessages_original, buf );
		}

		static bool HookShutdown( )
		{
			return Hook( &CNetChan::Shutdown, &CNetChanProxy::Shutdown );
		}

		static CNetChanProxy Singleton;
	};

	CNetChanProxy CNetChanProxy::Singleton;

#if defined SYSTEM_WINDOWS

	const char CNetChanProxy::ProcessMessages_sig[] = "\x55\x8B\xEC\x83\xEC\x2C\xF7\x05";
	const size_t CNetChanProxy::ProcessMessages_siglen =
		sizeof( CNetChanProxy::ProcessMessages_sig ) - 1;

#elif ( defined SYSTEM_LINUX && defined SOURCENET_SERVER ) || defined SYSTEM_MACOSX

	const char CNetChanProxy::ProcessMessages_sig[] =
		"@_ZN8CNetChan15ProcessMessagesER7bf_read";
	const size_t CNetChanProxy::ProcessMessages_siglen = 0;

#elif defined SYSTEM_LINUX && defined SOURCENET_CLIENT

	const char CNetChanProxy::ProcessMessages_sig[] =
		"\x55\x89\xE5\x57\x56\x53\x83\xEC\x6C\x8B\x3D\x2A\x2A\x2A\x2A\x8B";
	const size_t CNetChanProxy::ProcessMessages_siglen =
		sizeof( CNetChanProxy::ProcessMessages_sig ) - 1;

#endif

	CNetChanProxy::ProcessMessages_t CNetChanProxy::ProcessMessages_original = nullptr;

	class INetChannelHandlerProxy : Detouring::ClassProxy<INetChannelHandler, INetChannelHandlerProxy>
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

		static bool HookConnectionClosing( )
		{
			return Hook(
				&INetChannelHandler::ConnectionClosing, &INetChannelHandlerProxy::ConnectionClosing
			);
		}

		static bool HookConnectionCrashed( )
		{
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

	void HookCNetChan( GarrysMod::Lua::ILuaBase *LUA )
	{
		CNetChanProxy::HookShutdown( );
	}

	void HookINetChannelHandler( GarrysMod::Lua::ILuaBase *LUA )
	{
		INetChannelHandlerProxy::HookConnectionClosing( );
		INetChannelHandlerProxy::HookConnectionCrashed( );
	}
}
