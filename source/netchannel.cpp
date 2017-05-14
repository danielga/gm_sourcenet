#include <netchannel.hpp>
#include <netchannelhandler.hpp>
#include <subchannel.hpp>
#include <datafragments.hpp>
#include <sn_bf_write.hpp>
#include <netadr.hpp>
#include <hooks.hpp>
#include <netmessage.hpp>
#include <net.hpp>
#include <inetmessage.h>
#include <eiface.h>
#include <cdll_int.h>
#include <sstream>

namespace NetChannel
{

static uint8_t metatype = GarrysMod::Lua::Type::NONE;
static const char *metaname = "CNetChan";
static const char *table_name = "sourcenet_CNetChan";

bool IsValid( CNetChan *netchan )
{
	return netchan != nullptr;
}

void Push( GarrysMod::Lua::ILuaBase *LUA, CNetChan *netchan )
{
	if( netchan == nullptr )
	{
		LUA->PushNil( );
		return;
	}

	LUA->GetField( GarrysMod::Lua::INDEX_REGISTRY, table_name );
	LUA->PushUserdata( netchan );
	LUA->GetTable( -2 );
	if( LUA->IsType( -1, metatype ) )
	{
		LUA->Remove( -2 );
		return;
	}

	LUA->Pop( 1 );

	LUA->PushUserType( netchan, metatype );

	LUA->PushMetaTable( metatype );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( LUA->GetState(), -2 );

	LUA->PushUserdata( netchan );
	LUA->Push( -2 );
	LUA->SetTable( -4 );
	LUA->Remove( -2 );

	Hooks::HookCNetChan( LUA );
}

inline CNetChan *GetUserData( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
{
	global::CheckType( LUA, index, metatype, metaname );
	return LUA->GetUserType<CNetChan>( index, metatype );
}

CNetChan *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
{
	CNetChan *netchan = GetUserData( LUA, 1 );
	if( !IsValid( netchan ) )
		global::ThrowError( LUA, "invalid %s", metaname );

	return netchan;
}

void Destroy( GarrysMod::Lua::ILuaBase *LUA, CNetChan *netchan )
{
	LUA->GetField( GarrysMod::Lua::INDEX_REGISTRY, table_name );
	LUA->PushUserdata( netchan );
	LUA->PushNil( );
	LUA->SetTable( -3 );
	LUA->Pop( 1 );
}

LUA_FUNCTION_STATIC( eq )
{
	LUA->PushBool( Get( LUA, 1 ) == Get( LUA, 2 ) );
	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	lua_pushfstring( LUA->GetState(), global::tostring_format, metaname, Get( LUA, 1 ) );
	return 1;
}

LUA_FUNCTION_STATIC( IsValid )
{
	LUA->PushBool( IsValid( GetUserData( LUA, 1 ) ) );
	return 1;
}

LUA_FUNCTION_STATIC( IsNull )
{
	LUA->PushBool( Get( LUA, 1 )->IsNull( ) );
	return 1;
}

LUA_FUNCTION_STATIC( DumpNetMessages )
{
	CNetChan *netchan = Get( LUA, 1 );

	std::ostringstream stream;
	for( int32_t i = 0; i < netchan->m_NetMessages.Count( ); ++i )
	{
		if( i != 0 )
			stream << '\n';

		INetMessage *netmsg = netchan->m_NetMessages.Element( i );
		stream << i + 1 << ". " << netmsg->GetName( ) << " (" << netmsg->GetType( ) << ')';
	}

	LUA->PushString( stream.str( ).c_str( ) );
	return 1;
}

LUA_FUNCTION_STATIC( GetNetMessageNum )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushNumber( netchan->m_NetMessages.Count( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetNetMessage )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t idx = static_cast<int32_t>( LUA->GetNumber( 2 ) ) - 1;
	if( idx < 0 || idx >= netchan->m_NetMessages.Count( ) )
		LUA->ThrowError( "invalid netmessage index" );

	NetMessage::Push( LUA, netchan->m_NetMessages.Element( idx ), netchan );

	return 1;
}

LUA_FUNCTION_STATIC( Reset )
{
	CNetChan *netchan = Get( LUA, 1 );

	netchan->Reset( );

	return 0;
}

LUA_FUNCTION_STATIC( Clear )
{
	CNetChan *netchan = Get( LUA, 1 );

	netchan->Clear( );

	return 0;
}

LUA_FUNCTION_STATIC( Shutdown )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	netchan->Shutdown( LUA->GetString( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( Transmit )
{
	CNetChan *netchan = Get( LUA, 1 );

	bool onlyReliable = false;

	if( LUA->IsType( 2, GarrysMod::Lua::Type::BOOL ) )
		onlyReliable = LUA->GetBool( 2 );

	LUA->PushBool( netchan->Transmit( onlyReliable ) );

	return 1;
}

LUA_FUNCTION_STATIC( SendNetMsg )
{
	CNetChan *netchan = Get( LUA, 1 );
	INetMessage *netmsg = NetMessage::Get( LUA, 2 );

	bool reliable = false;
	if( LUA->IsType( 3, GarrysMod::Lua::Type::BOOL ) )
		reliable = LUA->GetBool( 3 );

	bool voice = false;
	if( LUA->IsType( 4, GarrysMod::Lua::Type::BOOL ) )
		voice = LUA->GetBool( 4 );

	LUA->PushBool( netchan->SendNetMsg( *netmsg, reliable, voice ) );

	return 1;
}

LUA_FUNCTION_STATIC( SendData )
{
	CNetChan *netchan = Get( LUA, 1 );
	bf_write *netbuf = sn_bf_write::Get( LUA, 2 );

	bool reliable = true;
	if( LUA->IsType( 3, GarrysMod::Lua::Type::BOOL ) )
		reliable = LUA->GetBool( 3 );

	LUA->PushBool( netchan->SendData( *netbuf, reliable ) );

	return 1;
}

LUA_FUNCTION_STATIC( SendFile )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	LUA->PushBool( netchan->SendFile(
		LUA->GetString( 2 ),
		static_cast<uint32_t>( LUA->GetNumber( 3 ) )
	) );

	return 1;
}

LUA_FUNCTION_STATIC( DenyFile )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	netchan->DenyFile( LUA->GetString( 2 ), static_cast<uint32_t>( LUA->GetNumber( 3 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( RequestFile )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushNumber( netchan->RequestFile( LUA->GetString( 2 ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( CanPacket )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushBool( netchan->CanPacket( ) );

	return 1;
}

LUA_FUNCTION_STATIC( HasPendingReliableData )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushBool( netchan->HasPendingReliableData( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetOutgoingQueueSize )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );
	if( stream < 0 || stream >= MAX_STREAMS )
		return 0;

	LUA->PushNumber( netchan->m_WaitingList[stream].Count( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetOutgoingQueueFragments )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );
	if( stream < 0 || stream >= MAX_STREAMS )
		return 0;

	int32_t offset = static_cast<int32_t>( LUA->GetNumber( 3 ) );
	if( offset < 0 || offset >= netchan->m_WaitingList[stream].Count( ) )
		return 0;

	dataFragments::Push( LUA, netchan->m_WaitingList[stream].Element( offset ) );

	return 1;
}

LUA_FUNCTION_STATIC( QueueOutgoingFragments )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );
	if( stream < 0 || stream >= MAX_STREAMS )
		return 0;

	dataFragments_t *fragments = dataFragments::Get( LUA, 3 );

	netchan->m_WaitingList[stream].AddToTail( fragments );

	return 0;
}

LUA_FUNCTION_STATIC( GetIncomingFragments )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );
	if( stream < 0 || stream >= MAX_STREAMS )
		return 0;

	dataFragments::Push( LUA, &netchan->m_ReceiveList[stream] );

	return 1;
}

LUA_FUNCTION_STATIC( GetSubChannels )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->CreateTable( );

	for( int32_t i = 0; i < MAX_SUBCHANNELS; ++i )
	{
		subchannel::Push( LUA, &netchan->m_SubChannels[i], netchan );

		LUA->PushNumber( i + 1 );

		LUA->SetTable( -3 );
	}

	return 1;
}

LUA_FUNCTION_STATIC( GetReliableBuffer )
{
	CNetChan *netchan = Get( LUA, 1 );

	sn_bf_write::Push( LUA, &netchan->m_StreamReliable );

	return 1;
}

LUA_FUNCTION_STATIC( GetUnreliableBuffer )
{
	CNetChan *netchan = Get( LUA, 1 );

	sn_bf_write::Push( LUA, &netchan->m_StreamUnreliable );

	return 1;
}

LUA_FUNCTION_STATIC( GetVoiceBuffer )
{
	CNetChan *netchan = Get( LUA, 1 );

	sn_bf_write::Push( LUA, &netchan->m_StreamVoice );

	return 1;
}

LUA_FUNCTION_STATIC( GetNetChannelHandler )
{
	CNetChan *netchan = Get( LUA, 1 );

	NetChannelHandler::Push( LUA, netchan->GetMsgHandler( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetAddress )
{
	CNetChan *netchan = Get( LUA, 1 );

	netadr::Push( LUA, netchan->remote_address );
	
	return 1;
}

LUA_FUNCTION_STATIC( GetRemoteAddress )
{
	CNetChan *netchan = Get( LUA, 1 );

	netadr::Push( LUA, netchan->GetRemoteAddress( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetTime )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushNumber( netchan->GetTime( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetLatency )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t num = static_cast<int32_t>( LUA->GetNumber( 2 ) );
	if( num < 0 || num >= MAX_FLOWS )
		return 0;

	LUA->PushNumber( netchan->GetLatency( num ) );
	return 1;
}

LUA_FUNCTION_STATIC( GetAvgLatency )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetAvgLatency( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetAvgLoss )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetAvgLoss( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetAvgChoke )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetAvgChoke( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetAvgData )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetAvgData( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetAvgPackets )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetAvgPackets( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetTotalData )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetTotalData( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetSequenceNr )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetSequenceNr( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetTimeSinceLastReceived )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushNumber( netchan->GetTimeSinceLastReceived( ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsValidPacket )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	LUA->PushBool( netchan->IsValidPacket(
		static_cast<int32_t>( LUA->GetNumber( 2 ) ),
		static_cast<int32_t>( LUA->GetNumber( 3 ) )
	) );

	return 1;
}

LUA_FUNCTION_STATIC( GetPacketTime )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetPacketTime(
		static_cast<int32_t>( LUA->GetNumber( 2 ) ),
		static_cast<int32_t>( LUA->GetNumber( 3 ) )
	) );

	return 1;
}

LUA_FUNCTION_STATIC( GetPacketBytes )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 4, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetPacketBytes(
		static_cast<int32_t>( LUA->GetNumber( 2 ) ),
		static_cast<int32_t>( LUA->GetNumber( 3 ) ),
		static_cast<int32_t>( LUA->GetNumber( 4 ) )
	) );

	return 1;
}

LUA_FUNCTION_STATIC( GetStreamProgress )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t flow = static_cast<int32_t>( LUA->GetNumber( 2 ) ), received = 0, total = 0;
	if( netchan->GetStreamProgress( flow, &received, &total ) )
	{
		LUA->PushNumber( received );
		LUA->PushNumber( total );
	}
	else
	{
		LUA->PushNil( );
		LUA->PushNil( );
	}

	return 2;
}

LUA_FUNCTION_STATIC( GetCommandInterpolationAmount )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetCommandInterpolationAmount(
		static_cast<int32_t>( LUA->GetNumber( 2 ) ),
		static_cast<int32_t>( LUA->GetNumber( 3 ) )
	) );

	return 1;
}

LUA_FUNCTION_STATIC( GetPacketResponseLatency )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	int32_t latencymsecs = 0, choke = 0;
	netchan->GetPacketResponseLatency(
		static_cast<int32_t>( LUA->GetNumber( 2 ) ),
		static_cast<int32_t>( LUA->GetNumber( 3 ) ),
		&latencymsecs,
		&choke
	);

	LUA->PushNumber( latencymsecs );
	LUA->PushNumber( choke );

	return 2;
}

LUA_FUNCTION_STATIC( GetRemoteFramerate )
{
	CNetChan *netchan = Get( LUA, 1 );

	float frametime = 0.0f, frametimedev = 0.0f;
	netchan->GetRemoteFramerate( &frametime, &frametimedev );

	LUA->PushNumber( frametime );
	LUA->PushNumber( frametimedev );

	return 2;
}

LUA_FUNCTION_STATIC( SetInterpolationAmount )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->SetInterpolationAmount( LUA->GetNumber( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( SetRemoteFramerate )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	netchan->SetRemoteFramerate( LUA->GetNumber( 2 ), LUA->GetNumber( 3 ) );

	return 0;
}

LUA_FUNCTION_STATIC( SetMaxBufferSize )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 4, GarrysMod::Lua::Type::BOOL );

	netchan->SetMaxBufferSize(
		LUA->GetBool( 2 ),
		static_cast<int32_t>( LUA->GetNumber( 3 ) ),
		LUA->GetBool( 4 )
	);

	return 0;
}

LUA_FUNCTION_STATIC( IsLoopback )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushBool( netchan->IsLoopback( ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsTimingOut )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushBool( netchan->IsTimingOut( ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsTimedOut )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushBool( netchan->IsTimedOut( ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsPlayback )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushBool( netchan->IsPlayback( ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsOverflowed )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushBool( netchan->IsOverflowed( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetTimeoutSeconds )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushNumber( netchan->GetTimeoutSeconds(  ) );

	return 1;
}

LUA_FUNCTION_STATIC( SetTimeoutSeconds )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->SetTimeout( static_cast<float>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( GetTimeout )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushNumber( netchan->m_Timeout );

	return 1;
}

LUA_FUNCTION_STATIC( SetTimeout )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->m_Timeout = static_cast<float>( LUA->GetNumber( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( GetTimeConnected )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushNumber( netchan->GetTimeConnected( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetConnectTime )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushNumber( netchan->connect_time );

	return 1;
}

LUA_FUNCTION_STATIC( SetConnectTime )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->connect_time = LUA->GetNumber( 2 );

	return 1;
}

LUA_FUNCTION_STATIC( GetLastReceivedTime )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushNumber( netchan->last_received );

	return 1;
}

LUA_FUNCTION_STATIC( SetLastReceivedTime )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->last_received = static_cast<float>( LUA->GetNumber( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( GetName )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushString( netchan->m_Name );

	return 1;
}

LUA_FUNCTION_STATIC( SetName )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	strncpy( netchan->m_Name, LUA->GetString( 2 ), 32 );
	netchan->m_Name[sizeof( netchan->m_Name )] = '\0';

	return 0;
}

LUA_FUNCTION_STATIC( GetRate )
{
	CNetChan *netchan = Get( LUA, 1 );
	
	LUA->PushNumber( netchan->m_Rate );

	return 1;
}

LUA_FUNCTION_STATIC( SetRate )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->m_Rate = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( GetBackgroundMode )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushBool( netchan->m_bFileBackgroundTranmission );

	return 1;
}

LUA_FUNCTION_STATIC( SetBackgroundMode )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	netchan->m_bFileBackgroundTranmission = LUA->GetBool( 2 );

	return 0;
}

LUA_FUNCTION_STATIC( GetCompressionMode )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushBool( netchan->m_bUseCompression );

	return 1;
}

LUA_FUNCTION_STATIC( SetCompressionMode )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	netchan->m_bUseCompression = LUA->GetBool( 2 );

	return 0;
}

LUA_FUNCTION_STATIC( GetMaxRoutablePayloadSize )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushNumber( netchan->m_nMaxRoutablePayloadSize );

	return 1;
}

LUA_FUNCTION_STATIC( SetMaxRoutablePayloadSize )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->SetMaxRoutablePayloadSize( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( GetDataRate )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushNumber( netchan->GetDataRate( ) );

	return 1;
}

LUA_FUNCTION_STATIC( SetDataRate )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->SetDataRate( static_cast<float>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( GetBufferSize )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushNumber( netchan->GetBufferSize( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetNumBitsWritten )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	LUA->PushNumber( netchan->GetNumBitsWritten( LUA->GetBool( 2 ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetDropNumber )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushNumber( netchan->GetDropNumber( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetChallengeNr )
{
	CNetChan *netchan = Get( LUA, 1 );

	LUA->PushNumber( netchan->GetChallengeNr( ) );

	return 1;
}

LUA_FUNCTION_STATIC( SetChallengeNr )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->SetChallengeNr( static_cast<uint32_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( GetSequenceData )
{
	CNetChan *netchan = Get( LUA, 1 );

	int32_t out = 0, in = 0, outack = 0;
	netchan->GetSequenceData( out, in, outack );
	LUA->PushNumber( out );
	LUA->PushNumber( in );
	LUA->PushNumber( outack );

	return 3;
}

LUA_FUNCTION_STATIC( SetSequenceData )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 4, GarrysMod::Lua::Type::NUMBER );

	netchan->SetSequenceData(
		static_cast<int32_t>( LUA->GetNumber( 2 ) ),
		static_cast<int32_t>( LUA->GetNumber( 3 ) ),
		static_cast<int32_t>( LUA->GetNumber( 4 ) )
	);

	return 0;
}

LUA_FUNCTION_STATIC( SetChoked )
{
	CNetChan *netchan = Get( LUA, 1 );

	netchan->SetChoked( );

	return 0;
}

LUA_FUNCTION_STATIC( SetFileTransmissionMode )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	netchan->SetFileTransmissionMode( LUA->GetBool( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( UpdateMessageStats )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	netchan->UpdateMessageStats(
		static_cast<int32_t>( LUA->GetNumber( 2 ) ),
		static_cast<int32_t>( LUA->GetNumber( 3 ) )
	);

	return 0;
}

LUA_FUNCTION_STATIC( StartStreaming )
{
	CNetChan *netchan = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->StartStreaming( static_cast<uint32_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( ResetStreaming )
{
	CNetChan *netchan = Get( LUA, 1 );

	netchan->ResetStreaming( );

	return 0;
}

LUA_FUNCTION_STATIC( Constructor )
{

#if defined SOURCENET_SERVER

	int32_t index = -1;
	if( LUA->IsType( 1, GarrysMod::Lua::Type::ENTITY ) )
	{
		LUA->PushMetaTable( GarrysMod::Lua::Type::ENTITY );
		LUA->GetField( -1, "EntIndex" );
		LUA->Push( 1 );
		LUA->Call( 1, 1 );

		index = static_cast<int32_t>( LUA->CheckNumber( -1 ) );
	}
	else
	{
		index = static_cast<int32_t>( LUA->CheckNumber( 1 ) );
	}

	Push(
		LUA,
		static_cast<CNetChan *>( global::engine_server->GetPlayerNetInfo( index ) )
	);

#elif defined SOURCENET_CLIENT

	Push( LUA, static_cast<CNetChan *>( global::engine_client->GetNetChannelInfo( ) ) );

#endif

	return 1;
}

void Initialize( GarrysMod::Lua::ILuaBase *LUA )
{
	LUA->CreateTable( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, table_name );



	metatype = LUA->CreateMetaTable( metaname );

		LUA->PushCFunction( eq );
		LUA->SetField( -2, "__eq" );

		LUA->PushCFunction( tostring );
		LUA->SetField( -2, "__tostring" );

		LUA->PushCFunction( global::index );
		LUA->SetField( -2, "__index" );

		LUA->PushCFunction( global::newindex );
		LUA->SetField( -2, "__newindex" );

		LUA->PushCFunction( global::GetTable );
		LUA->SetField( -2, "GetTable" );

		LUA->PushCFunction( IsValid );
		LUA->SetField( -2, "IsValid" );

		LUA->PushCFunction( IsNull );
		LUA->SetField( -2, "IsNull" );

		LUA->PushCFunction( DumpNetMessages );
		LUA->SetField( -2, "DumpNetMessages" );

		LUA->PushCFunction( GetNetMessageNum );
		LUA->SetField( -2, "GetNetMessageNum" );

		LUA->PushCFunction( GetNetMessage );
		LUA->SetField( -2, "GetNetMessage" );

		LUA->PushCFunction( Reset );
		LUA->SetField( -2, "Reset" );

		LUA->PushCFunction( Clear );
		LUA->SetField( -2, "Clear" );

		LUA->PushCFunction( Shutdown );
		LUA->SetField( -2, "Shutdown" );

		LUA->PushCFunction( Transmit );
		LUA->SetField( -2, "Transmit" );

		LUA->PushCFunction( SendNetMsg );
		LUA->SetField( -2, "SendNetMsg" );

		LUA->PushCFunction( SendData );
		LUA->SetField( -2, "SendData" );

		LUA->PushCFunction( SendFile );
		LUA->SetField( -2, "SendFile" );

		LUA->PushCFunction( DenyFile );
		LUA->SetField( -2, "DenyFile" );

		LUA->PushCFunction( RequestFile );
		LUA->SetField( -2, "RequestFile" );

		LUA->PushCFunction( CanPacket );
		LUA->SetField( -2, "CanPacket" );

		LUA->PushCFunction( HasPendingReliableData );
		LUA->SetField( -2, "HasPendingReliableData" );

		LUA->PushCFunction( GetOutgoingQueueSize );
		LUA->SetField( -2, "GetOutgoingQueueSize" );

		LUA->PushCFunction( GetOutgoingQueueFragments );
		LUA->SetField( -2, "GetOutgoingQueueFragments" );

		LUA->PushCFunction( QueueOutgoingFragments );
		LUA->SetField( -2, "QueueOutgoingFragments" );

		LUA->PushCFunction( GetIncomingFragments );
		LUA->SetField( -2, "GetIncomingFragments" );

		LUA->PushCFunction( GetSubChannels );
		LUA->SetField( -2, "GetSubChannels" );

		LUA->PushCFunction( GetReliableBuffer );
		LUA->SetField( -2, "GetReliableBuffer" );

		LUA->PushCFunction( GetUnreliableBuffer );
		LUA->SetField( -2, "GetUnreliableBuffer" );

		LUA->PushCFunction( GetVoiceBuffer );
		LUA->SetField( -2, "GetVoiceBuffer" );

		LUA->PushCFunction( GetNetChannelHandler );
		LUA->SetField( -2, "GetNetChannelHandler" );

		LUA->PushCFunction( GetAddress );
		LUA->SetField( -2, "GetAddress" );

		LUA->PushCFunction( GetRemoteAddress );
		LUA->SetField( -2, "GetRemoteAddress" );

		LUA->PushCFunction( GetTime );
		LUA->SetField( -2, "GetTime" );

		LUA->PushCFunction( GetLatency );
		LUA->SetField( -2, "GetLatency" );

		LUA->PushCFunction( GetAvgLatency );
		LUA->SetField( -2, "GetAvgLatency" );

		LUA->PushCFunction( GetAvgLoss );
		LUA->SetField( -2, "GetAvgLoss" );

		LUA->PushCFunction( GetAvgChoke );
		LUA->SetField( -2, "GetAvgChoke" );

		LUA->PushCFunction( GetAvgData );
		LUA->SetField( -2, "GetAvgData" );

		LUA->PushCFunction( GetAvgPackets );
		LUA->SetField( -2, "GetAvgPackets" );

		LUA->PushCFunction( GetTotalData );
		LUA->SetField( -2, "GetTotalData" );

		LUA->PushCFunction( GetSequenceNr );
		LUA->SetField( -2, "GetSequenceNr" );

		LUA->PushCFunction( GetTimeSinceLastReceived );
		LUA->SetField( -2, "GetTimeSinceLastReceived" );

		LUA->PushCFunction( IsValidPacket );
		LUA->SetField( -2, "IsValidPacket" );

		LUA->PushCFunction( GetPacketTime );
		LUA->SetField( -2, "GetPacketTime" );

		LUA->PushCFunction( GetPacketBytes );
		LUA->SetField( -2, "GetPacketBytes" );

		LUA->PushCFunction( GetStreamProgress );
		LUA->SetField( -2, "GetStreamProgress" );

		LUA->PushCFunction( GetCommandInterpolationAmount );
		LUA->SetField( -2, "GetCommandInterpolationAmount" );

		LUA->PushCFunction( GetPacketResponseLatency );
		LUA->SetField( -2, "GetPacketResponseLatency" );

		LUA->PushCFunction( GetRemoteFramerate );
		LUA->SetField( -2, "GetRemoteFramerate" );

		LUA->PushCFunction( SetInterpolationAmount );
		LUA->SetField( -2, "SetInterpolationAmount" );

		LUA->PushCFunction( SetRemoteFramerate );
		LUA->SetField( -2, "SetRemoteFramerate" );

		LUA->PushCFunction( SetMaxBufferSize );
		LUA->SetField( -2, "SetMaxBufferSize" );

		LUA->PushCFunction( IsLoopback );
		LUA->SetField( -2, "IsLoopback" );

		LUA->PushCFunction( IsTimingOut );
		LUA->SetField( -2, "IsTimingOut" );

		LUA->PushCFunction( IsTimedOut );
		LUA->SetField( -2, "IsTimedOut" );

		LUA->PushCFunction( IsPlayback );
		LUA->SetField( -2, "IsPlayback" );

		LUA->PushCFunction( IsOverflowed );
		LUA->SetField( -2, "IsOverflowed" );

		LUA->PushCFunction( GetTimeoutSeconds );
		LUA->SetField( -2, "GetTimeoutSeconds" );

		LUA->PushCFunction( SetTimeoutSeconds );
		LUA->SetField( -2, "SetTimeoutSeconds" );

		LUA->PushCFunction( GetTimeout );
		LUA->SetField( -2, "GetTimeout" );

		LUA->PushCFunction( SetTimeout );
		LUA->SetField( -2, "SetTimeout" );

		LUA->PushCFunction( GetTimeConnected );
		LUA->SetField( -2, "GetTimeConnected" );

		LUA->PushCFunction( GetConnectTime );
		LUA->SetField( -2, "GetConnectTime" );

		LUA->PushCFunction( SetConnectTime );
		LUA->SetField( -2, "SetConnectTime" );

		LUA->PushCFunction( GetLastReceivedTime );
		LUA->SetField( -2, "GetLastReceivedTime" );

		LUA->PushCFunction( SetLastReceivedTime );
		LUA->SetField( -2, "SetLastReceivedTime" );

		LUA->PushCFunction( GetName );
		LUA->SetField( -2, "GetName" );

		LUA->PushCFunction( SetName );
		LUA->SetField( -2, "SetName" );

		LUA->PushCFunction( GetRate );
		LUA->SetField( -2, "GetRate" );

		LUA->PushCFunction( SetRate );
		LUA->SetField( -2, "SetRate" );

		LUA->PushCFunction( GetBackgroundMode );
		LUA->SetField( -2, "GetBackgroundMode" );

		LUA->PushCFunction( SetBackgroundMode );
		LUA->SetField( -2, "SetBackgroundMode" );

		LUA->PushCFunction( GetCompressionMode );
		LUA->SetField( -2, "GetCompressionMode" );

		LUA->PushCFunction( SetCompressionMode );
		LUA->SetField( -2, "SetCompressionMode" );

		LUA->PushCFunction( GetMaxRoutablePayloadSize );
		LUA->SetField( -2, "GetMaxRoutablePayloadSize" );

		LUA->PushCFunction( SetMaxRoutablePayloadSize );
		LUA->SetField( -2, "SetMaxRoutablePayloadSize" );

		LUA->PushCFunction( GetDataRate );
		LUA->SetField( -2, "GetDataRate" );

		LUA->PushCFunction( SetDataRate );
		LUA->SetField( -2, "SetDataRate" );

		LUA->PushCFunction( GetBufferSize );
		LUA->SetField( -2, "GetBufferSize" );

		LUA->PushCFunction( GetNumBitsWritten );
		LUA->SetField( -2, "GetNumBitsWritten" );

		LUA->PushCFunction( GetDropNumber );
		LUA->SetField( -2, "GetDropNumber" );

		LUA->PushCFunction( GetChallengeNr );
		LUA->SetField( -2, "GetChallengeNr" );

		LUA->PushCFunction( SetChallengeNr );
		LUA->SetField( -2, "SetChallengeNr" );

		LUA->PushCFunction( GetSequenceData );
		LUA->SetField( -2, "GetSequenceData" );

		LUA->PushCFunction( SetSequenceData );
		LUA->SetField( -2, "SetSequenceData" );

		LUA->PushCFunction( SetChoked );
		LUA->SetField( -2, "SetChoked" );

		LUA->PushCFunction( SetFileTransmissionMode );
		LUA->SetField( -2, "SetFileTransmissionMode" );

		LUA->PushCFunction( UpdateMessageStats );
		LUA->SetField( -2, "UpdateMessageStats" );

		LUA->PushCFunction( StartStreaming );
		LUA->SetField( -2, "StartStreaming" );

		LUA->PushCFunction( ResetStreaming );
		LUA->SetField( -2, "ResetStreaming" );

	LUA->Pop( 1 );



	LUA->PushNumber( MAX_RATE );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "RATE_MAX" );

	LUA->PushNumber( MIN_RATE );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "RATE_MIN" );

	LUA->PushNumber( DEFAULT_RATE );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "RATE_DEFAULT" );



	LUA->PushNumber( FLOW_OUTGOING );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "FLOW_OUTGOING" );

	LUA->PushNumber( FLOW_INCOMING );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "FLOW_INCOMING" );

	LUA->PushNumber( MAX_FLOWS );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "FLOW_MAX" );



	LUA->PushNumber( MAX_SUBCHANNELS );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SUBCHANNEL_MAX" );



	LUA->PushNumber( FRAG_NORMAL_STREAM );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "STREAM_FRAG_NORMAL" );

	LUA->PushNumber( FRAG_FILE_STREAM );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "STREAM_FRAG_FILE" );

	LUA->PushNumber( MAX_STREAMS );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "STREAM_MAX" );



	LUA->PushCFunction( Constructor );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, metaname );
}

void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
{
	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, metaname );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "RATE_MAX" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "RATE_MIN" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "RATE_DEFAULT" );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "FLOW_OUTGOING" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "FLOW_INCOMING" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "FLOW_MAX" );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SUBCHANNEL_MAX" );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "STREAM_FRAG_NORMAL" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "STREAM_FRAG_FILE" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "STREAM_MAX" );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, metaname );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, table_name );
}

}
