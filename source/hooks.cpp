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
#include <eiface.h>
#include <iserver.h>

namespace Hooks
{
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
		typedef Detouring::ClassProxy<TargetClass, SubstituteClass> ClassProxy;

		static void Initialize( GarrysMod::Lua::ILuaBase *LUA )
		{
			ProcessMessages_original = FunctionPointers::CNetChan_ProcessMessages( );
			if( ProcessMessages_original == nullptr )
				LUA->ThrowError( "failed to locate CNetChan::ProcessMessages" );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaAttachSendDatagram )
		{
			LUA->PushBool(
				Hook( &TargetClass::SendDatagram, &SubstituteClass::SendDatagram )
			);
			return 1;
		}

		static bool DetachSendDatagram( )
		{
			return UnHook( &TargetClass::SendDatagram );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaDetachSendDatagram )
		{
			LUA->PushBool( DetachSendDatagram( ) );
			return 1;
		}

		virtual int32_t SendDatagram( bf_write *data )
		{
			CNetChan *netchan = This( );

			{
				GarrysMod::Lua::ILuaBase *LUA = global::lua;
				if( LuaHelpers::PushHookRun( LUA, "PreSendDatagram" ) != 0 )
				{
					NetChannel::Push( LUA, netchan );

					if( !global::is_dedicated )
					{
						NetChannel::Push(
							LUA,
							static_cast<CNetChan *>( global::engine_client->GetNetChannelInfo( ) )
						);
					}
					else
					{
						LUA->PushNil( );
					}

					bf_write **writer1 = sn_bf_write::Push( LUA, data );
					bf_write **writer2 = sn_bf_write::Push( LUA, &netchan->m_StreamReliable );
					bf_write **writer3 = sn_bf_write::Push( LUA, &netchan->m_StreamUnreliable );
					bf_write **writer4 = sn_bf_write::Push( LUA, &netchan->m_StreamVoice );

					LuaHelpers::CallHookRun( LUA, 6, 0 );

					*writer1 = nullptr;
					*writer2 = nullptr;
					*writer3 = nullptr;
					*writer4 = nullptr;
				}
			}

			int32_t r = Call( &CNetChan::SendDatagram, data );

			{
				GarrysMod::Lua::ILuaBase *LUA = global::lua;
				if( LuaHelpers::PushHookRun( LUA, "PostSendDatagram" ) != 0 )
				{
					NetChannel::Push( LUA, netchan );

					LuaHelpers::CallHookRun( LUA, 1, 0 );
				}
			}

			return r;
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaAttachProcessPacket )
		{
			LUA->PushBool(
				Hook( &TargetClass::ProcessPacket, &SubstituteClass::ProcessPacket )
			);
			return 1;
		}

		static bool DetachProcessPacket( )
		{
			return UnHook( &TargetClass::ProcessPacket );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaDetachProcessPacket )
		{
			LUA->PushBool( DetachProcessPacket( ) );
			return 1;
		}

		virtual void ProcessPacket( netpacket_t *packet, bool bHasHeader )
		{
			CNetChan *netchan = This( );

			{
				GarrysMod::Lua::ILuaBase *LUA = global::lua;
				if( LuaHelpers::PushHookRun( LUA, "PreProcessPacket" ) != 0 )
				{
					NetChannel::Push( LUA, netchan );

					LuaHelpers::CallHookRun( LUA, 1, 0 );
				}
			}

			Call( &CNetChan::ProcessPacket, packet, bHasHeader );

			{
				GarrysMod::Lua::ILuaBase *LUA = global::lua;
				if( LuaHelpers::PushHookRun( LUA, "PostProcessPacket" ) != 0 )
				{
					NetChannel::Push( LUA, netchan );

					LuaHelpers::CallHookRun( LUA, 1, 0 );
				}
			}
		}

		static bool AttachShutdown( )
		{
			return Hook( &TargetClass::Shutdown, &SubstituteClass::Shutdown );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaAttachShutdown )
		{
			LUA->PushBool( AttachShutdown( ) );
			return 1;
		}

		static bool DetachShutdown( )
		{
			return UnHook( &TargetClass::Shutdown );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaDetachShutdown )
		{
			LUA->PushBool( DetachShutdown( ) );
			return 1;
		}

		virtual void Shutdown( const char *reason )
		{
			CNetChan *netchan = This( );

			{
				GarrysMod::Lua::ILuaBase *LUA = global::lua;
				if( LuaHelpers::PushHookRun( LUA, "PreNetChannelShutdown" ) != 0 )
				{
					NetChannel::Push( LUA, netchan );
					LUA->PushString( reason );

					LuaHelpers::CallHookRun( LUA, 2, 0 );
				}
			}

			NetMessage::Destroy( global::lua, netchan );

			NetChannel::Destroy( global::lua, netchan );

			Call( &CNetChan::Shutdown, reason );

			{
				GarrysMod::Lua::ILuaBase *LUA = global::lua;
				if( LuaHelpers::PushHookRun( LUA, "PostNetChannelShutdown" ) != 0 )
					LuaHelpers::CallHookRun( LUA, 0, 0 );
			}
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaAttachProcessMessages )
		{
			LUA->PushBool( Hook( ProcessMessages_original, &CNetChanProxy::ProcessMessages ) );
			return 1;
		}

		static bool DetachProcessMessages( )
		{
			return UnHook( ProcessMessages_original );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaDetachProcessMessages )
		{
			LUA->PushBool( DetachProcessMessages( ) );
			return 1;
		}

		bool ProcessMessages( bf_read &buf )
		{
			CNetChan *netchan = This( );

			if( buf.IsOverflowed( ) )
				return Call( ProcessMessages_original, buf );

			const uint8_t *original_data = buf.GetBasePointer( );
			static uint8_t data[100000] = { 0 };

			const int32_t bytesread = buf.GetNumBytesRead( );
			if( bytesread > sizeof( data ) )
				throw "FUCK";

			if( bytesread > 0 )
				std::memcpy( data, buf.GetBasePointer( ), bytesread );

			const int32_t bitsread = buf.GetNumBitsRead( );
			bf_write write( data, sizeof( data ) );
			write.SeekToBit( bitsread );

			bool handled = false;
			{
				GarrysMod::Lua::ILuaBase *LUA = global::lua;
				if( LuaHelpers::PushHookRun( LUA, "PreProcessMessages" ) != 0 )
				{
					NetChannel::Push( LUA, netchan );
					bf_read **reader = sn_bf_read::Push( LUA, &buf );
					bf_write **writer = sn_bf_write::Push( LUA, &write );

					if( !global::is_dedicated )
					{
						NetChannel::Push(
							LUA,
							static_cast<CNetChan *>( global::engine_client->GetNetChannelInfo( ) )
						);
					}

					if( LuaHelpers::CallHookRun( LUA, global::is_dedicated ? 3 : 4, 1 ) )
					{
						if( LUA->IsType( -1, GarrysMod::Lua::Type::BOOL ) )
							handled = LUA->GetBool( -1 );

						LUA->Pop( 1 );
					}

					*reader = nullptr;
					*writer = nullptr;
				}
			}

			if( handled )
			{
				std::memcpy( const_cast<uint8_t *>( original_data ), data, write.GetNumBytesWritten( ) );
				buf.StartReading(
					original_data,
					write.GetNumBytesWritten( ),
					bitsread,
					write.GetNumBitsWritten( )
				);
			}
			else
				buf.Seek( bitsread );

			return Call( ProcessMessages_original, buf );
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
		typedef Detouring::ClassProxy<TargetClass, SubstituteClass> ClassProxy;

		static bool AttachConnectionStart( )
		{
			return Hook( &TargetClass::ConnectionStart, &SubstituteClass::ConnectionStart );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaAttachConnectionStart )
		{
			LUA->PushBool( AttachConnectionStart( ) );
			return 1;
		}

		static bool DetachConnectionStart( )
		{
			return UnHook( &TargetClass::ConnectionStart );
		}
		
		LUA_FUNCTION_STATIC_MEMBER( LuaDetachConnectionStart )
		{
			LUA->PushBool( DetachConnectionStart( ) );
			return 1;
		}

		virtual void ConnectionStart( INetChannel *netchannel )
		{
			INetChannelHandler *handler = This( );

			{
				GarrysMod::Lua::ILuaBase *LUA = global::lua;
				if( LuaHelpers::PushHookRun( LUA, "INetChannelHandler::ConnectionStart" ) != 0 )
				{
					NetChannelHandler::Push( LUA, handler );
					NetChannel::Push( LUA, static_cast<CNetChan *>( netchannel ) );

					LuaHelpers::CallHookRun( LUA, 2, 0 );
				}
			}

			Call( &INetChannelHandler::ConnectionStart, netchannel );
		}

		static bool AttachConnectionClosing( )
		{
			return Hook( &TargetClass::ConnectionClosing, &SubstituteClass::ConnectionClosing );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaAttachConnectionClosing )
		{
			LUA->PushBool( AttachConnectionClosing( ) );
			return 1;
		}

		static bool DetachConnectionClosing( )
		{
			return UnHook( &TargetClass::ConnectionClosing );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaDetachConnectionClosing )
		{
			LUA->PushBool( DetachConnectionClosing( ) );
			return 1;
		}

		virtual void ConnectionClosing( const char *reason )
		{
			INetChannelHandler *handler = This( );

			{
				GarrysMod::Lua::ILuaBase *LUA = global::lua;
				if( LuaHelpers::PushHookRun( LUA, "INetChannelHandler::ConnectionClosing" ) != 0 )
				{
					NetChannelHandler::Push( LUA, handler );
					LUA->PushString( reason );

					LuaHelpers::CallHookRun( LUA, 2, 0 );
				}
			}

			NetChannelHandler::Destroy( global::lua, handler );

			Call( &INetChannelHandler::ConnectionClosing, reason );
		}

		static bool AttachConnectionCrashed( )
		{
			return Hook( &TargetClass::ConnectionCrashed, &SubstituteClass::ConnectionCrashed );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaAttachConnectionCrashed )
		{
			LUA->PushBool( AttachConnectionCrashed( ) );
			return 1;
		}

		static bool DetachConnectionCrashed( )
		{
			return UnHook( &TargetClass::ConnectionCrashed );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaDetachConnectionCrashed )
		{
			LUA->PushBool( DetachConnectionCrashed( ) );
			return 1;
		}

		virtual void ConnectionCrashed( const char *reason )
		{
			INetChannelHandler *handler = This( );

			{
				GarrysMod::Lua::ILuaBase *LUA = global::lua;
				if( LuaHelpers::PushHookRun( LUA, "INetChannelHandler::ConnectionCrashed" ) != 0 )
				{
					NetChannelHandler::Push( LUA, handler );
					LUA->PushString( reason );

					LuaHelpers::CallHookRun( LUA, 2, 0 );
				}
			}

			NetChannelHandler::Destroy( global::lua, handler );

			Call( &INetChannelHandler::ConnectionCrashed, reason );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaAttachPacketStart )
		{
			LUA->PushBool(
				Hook( &TargetClass::PacketStart, &SubstituteClass::PacketStart )
			);
			return 1;
		}

		static bool DetachPacketStart( )
		{
			return UnHook( &TargetClass::PacketStart );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaDetachPacketStart )
		{
			LUA->PushBool( DetachPacketStart( ) );
			return 1;
		}

		virtual void PacketStart( int32_t incoming_sequence, int32_t outgoing_acknowledged )
		{
			INetChannelHandler *handler = This( );

			{
				GarrysMod::Lua::ILuaBase *LUA = global::lua;
				if( LuaHelpers::PushHookRun( LUA, "INetChannelHandler::PacketStart" ) != 0 )
				{
					NetChannelHandler::Push( LUA, handler );
					LUA->PushNumber( incoming_sequence );
					LUA->PushNumber( outgoing_acknowledged );

					LuaHelpers::CallHookRun( LUA, 3, 0 );
				}
			}

			Call( &INetChannelHandler::PacketStart, incoming_sequence, outgoing_acknowledged );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaAttachPacketEnd )
		{
			LUA->PushBool(
				Hook( &TargetClass::PacketEnd, &SubstituteClass::PacketEnd )
			);
			return 1;
		}

		static bool DetachPacketEnd( )
		{
			return UnHook( &TargetClass::PacketEnd );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaDetachPacketEnd )
		{
			LUA->PushBool( DetachPacketEnd( ) );
			return 1;
		}

		virtual void PacketEnd( )
		{
			INetChannelHandler *handler = This( );

			{
				GarrysMod::Lua::ILuaBase *LUA = global::lua;
				if( LuaHelpers::PushHookRun( LUA, "INetChannelHandler::PacketEnd" ) != 0 )
				{
					NetChannelHandler::Push( LUA, handler );

					LuaHelpers::CallHookRun( LUA, 1, 0 );
				}
			}

			Call( &INetChannelHandler::PacketEnd );
		}
		
		LUA_FUNCTION_STATIC_MEMBER( LuaAttachFileRequested )
		{
			LUA->PushBool(
				Hook( &TargetClass::FileRequested, &SubstituteClass::FileRequested )
			);
			return 1;
		}

		static bool DetachFileRequested( )
		{
			return UnHook( &TargetClass::FileRequested );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaDetachFileRequested )
		{
			LUA->PushBool( DetachFileRequested( ) );
			return 1;
		}

		virtual void FileRequested( const char *fileName, uint32_t transferID )
		{
			INetChannelHandler *handler = This( );

			{
				GarrysMod::Lua::ILuaBase *LUA = global::lua;
				if( LuaHelpers::PushHookRun( LUA, "INetChannelHandler::FileRequested" ) != 0 )
				{
					NetChannelHandler::Push( LUA, handler );
					LUA->PushString( fileName );
					LUA->PushNumber( transferID );

					LuaHelpers::CallHookRun( LUA, 3, 0 );
				}
			}

			Call( &INetChannelHandler::FileRequested, fileName, transferID );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaAttachFileReceived )
		{
			LUA->PushBool(
				Hook( &TargetClass::FileReceived, &SubstituteClass::FileReceived )
			);
			return 1;
		}

		static bool DetachFileReceived( )
		{
			return UnHook( &TargetClass::FileReceived );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaDetachFileReceived )
		{
			LUA->PushBool( DetachFileReceived( ) );
			return 1;
		}

		virtual void FileReceived( const char *fileName, uint32_t transferID )
		{
			INetChannelHandler *handler = This( );

			{
				GarrysMod::Lua::ILuaBase *LUA = global::lua;
				if( LuaHelpers::PushHookRun( LUA, "INetChannelHandler::FileReceived" ) != 0 )
				{
					NetChannelHandler::Push( LUA, handler );
					LUA->PushString( fileName );
					LUA->PushNumber( transferID );

					LuaHelpers::CallHookRun( LUA, 3, 0 );
				}
			}

			Call( &INetChannelHandler::FileReceived, fileName, transferID );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaAttachFileDenied )
		{
			LUA->PushBool(
				Hook( &TargetClass::FileDenied, &SubstituteClass::FileDenied )
			);
			return 1;
		}

		static bool DetachFileDenied( )
		{
			return UnHook( &TargetClass::FileDenied );
		}

		LUA_FUNCTION_STATIC_MEMBER( LuaDetachFileDenied )
		{
			LUA->PushBool( DetachFileDenied( ) );
			return 1;
		}

		virtual void FileDenied( const char *fileName, uint32_t transferID )
		{
			INetChannelHandler *handler = This( );

			{
				GarrysMod::Lua::ILuaBase *LUA = global::lua;
				if( LuaHelpers::PushHookRun( LUA, "INetChannelHandler::FileDenied" ) != 0 )
				{
					NetChannelHandler::Push( LUA, handler );
					LUA->PushString( fileName );
					LUA->PushNumber( transferID );

					LuaHelpers::CallHookRun( LUA, 3, 0 );
				}
			}

			Call( &INetChannelHandler::FileDenied, fileName, transferID );
		}

		static INetChannelHandlerProxy Singleton;
	};

	INetChannelHandlerProxy INetChannelHandlerProxy::Singleton;

	LUA_FUNCTION_STATIC( Empty )
	{
		return 0;
	}

#if defined SOURCENET_SERVER

	static Detouring::Hook NET_CreateNetChannel_hook;

	static INetChannel *NET_CreateNetChannel_detour(
		int socket,
		const ns_address *adr,
		const char *name,
		INetChannelHandler *handler,
		const unsigned char *pbEncryptionKey,
		bool bForceNewChannel
	) {
		INetChannel *netchan = NET_CreateNetChannel_hook.GetTrampoline<FunctionPointers::NET_CreateNetChannel_t>( )(
			socket,
			adr,
			name,
			handler,
			pbEncryptionKey,
			bForceNewChannel
		);
		if( netchan == nullptr )
			return netchan;

		HookCNetChan( static_cast<CNetChan *>( netchan ) );
		HookINetChannelHandler( handler );
		NET_CreateNetChannel_hook.Destroy( );
		return netchan;
	}

	static void DetourCreateNetChannel( GarrysMod::Lua::ILuaBase *LUA )
	{
		auto NET_CreateNetChannel_original = FunctionPointers::NET_CreateNetChannel( );
		if( NET_CreateNetChannel_original == nullptr )
			LUA->ThrowError( "failed to locate NET_CreateNetChannel" );

		if( !NET_CreateNetChannel_hook.Create(
			reinterpret_cast<void *>( NET_CreateNetChannel_original ),
			reinterpret_cast<void *>( NET_CreateNetChannel_detour )
		) )
			LUA->ThrowError( "failed to create hook for NET_CreateNetChannel" );

		if( !NET_CreateNetChannel_hook.Enable( ) )
			LUA->ThrowError( "failed to hook NET_CreateNetChannel" );
	}

#endif

	void PreInitialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		CNetChanProxy::Initialize( LUA );

#if defined SOURCENET_SERVER

		const int client_count = global::server->GetClientCount( );
		CNetChan *netchan = nullptr;
		for( int k = 1; netchan == nullptr && k < client_count; ++k )
			netchan = static_cast<CNetChan *>( global::engine_server->GetPlayerNetInfo( k ) );

		if( netchan == nullptr )
		{
			DetourCreateNetChannel( LUA );
			return;
		}

		auto handler = netchan->GetMsgHandler( );
		if( handler == nullptr )
		{
			DetourCreateNetChannel( LUA );
			return;
		}

#elif defined SOURCENET_CLIENT

		CNetChan *netchan = static_cast<CNetChan *>( global::engine_client->GetNetChannelInfo( ) );
		if( netchan == nullptr )
			LUA->ThrowError( "failed to retrieve client CNetChan" );

		auto handler = netchan->GetMsgHandler( );
		if( handler == nullptr )
			LUA->ThrowError( "failed to retrieve client INetChannelHandler" );

#endif

		HookCNetChan( netchan );
		HookINetChannelHandler( handler );
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

#if defined SOURCENET_SERVER

		NET_CreateNetChannel_hook.Destroy( );

#endif

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
		CNetChanProxy::ClassProxy::Initialize( netchan, &CNetChanProxy::Singleton );
		CNetChanProxy::Singleton.AttachShutdown( );
	}

	void HookINetChannelHandler( INetChannelHandler *handler )
	{
		INetChannelHandlerProxy::ClassProxy::Initialize( handler, &INetChannelHandlerProxy::Singleton );
		INetChannelHandlerProxy::Singleton.AttachConnectionClosing( );
		INetChannelHandlerProxy::Singleton.AttachConnectionCrashed( );
	}
}
