#define IVENGINESERVER_INTERFACE
#define IVENGINECLIENT_INTERFACE
#define ISERVER_INTERFACE

#include <gl_cnetchan.hpp>
#include <gl_inetchannelhandler.hpp>
#include <gl_subchannel_t.hpp>
#include <gl_datafragments_t.hpp>
#include <gl_bitbuf_write.hpp>
#include <gl_netadr_t.hpp>
#include <net.h>
#include <inetmessage.h>

struct CNetChan_userdata
{
	CNetChan *netchan;
	uint8_t type;
};

bool IsValid_CNetChan( CNetChan *netchan )
{

	if( netchan == nullptr )
		return false;

#if defined SOURCENET_SERVER

	for( int32_t i = 1; i <= g_pServer->GetClientCount( ); ++i )
		if( netchan == g_pEngineServer->GetPlayerNetInfo( i ) )
			return true;

#elif defined SOURCENET_CLIENT

	if( netchan == g_pEngineClient->GetNetChannelInfo( ) )
		return true;

#endif

	return false;

}

void Push_CNetChan( lua_State *state, CNetChan *netchan )
{
	if( netchan == nullptr )
	{
		LUA->PushNil( );
		return;
	}

	CNetChan_userdata *userdata = static_cast<CNetChan_userdata *>( LUA->NewUserdata( sizeof( CNetChan_userdata ) ) );
	userdata->type = GET_META_ID( CNetChan );
	userdata->netchan = netchan;

	LUA->CreateMetaTableType( GET_META_NAME( CNetChan ), GET_META_ID( CNetChan ) );
	LUA->SetMetaTable( -2 );
}

CNetChan *Get_CNetChan( lua_State *state, int32_t index )
{
	LUA->CheckType( index, GET_META_ID( CNetChan ) );

	CNetChan *netchan = static_cast<CNetChan_userdata *>( LUA->GetUserdata( index ) )->netchan;
	if( !IsValid_CNetChan( netchan ) )
		LUA->ThrowError( "invalid CNetChan" );

	return netchan;
}

inline int32_t VerifyStream( lua_State *state, int32_t stream )
{
	if( stream < 0 || stream >= MAX_STREAMS )
	{
		LUA->PushNil( );
		return 1;
	}

	return 0;
}

META_ID( CNetChan, 3 );

META_FUNCTION( CNetChan, __eq )
{
	CNetChan *netchan1 = Get_CNetChan( state, 1 );
	CNetChan *netchan2 = Get_CNetChan( state, 2 );

	LUA->PushBool( netchan1 == netchan2 );

	return 1;
}

META_FUNCTION( CNetChan, __tostring )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", GET_META_NAME( CNetChan ), netchan );

	return 1;
}

META_FUNCTION( CNetChan, IsValid )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	LUA->PushBool( IsValid_CNetChan( static_cast<CNetChan_userdata *>( LUA->GetUserdata( 1 ) )->netchan ) );

	return 1;
}

META_FUNCTION( CNetChan, DumpMessages )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	for( int32_t i = 0; i < netchan->netmessages.Count( ); ++i )
	{
		INetMessage *netmsg = netchan->netmessages.Element( i );
		Msg( "%d. %s (%d)\n", i + 1, netmsg->GetName( ), netmsg->GetType( ) );
	}

	return 0;
}

META_FUNCTION( CNetChan, Reset )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	netchan->Reset( );

	return 0;
}

META_FUNCTION( CNetChan, Clear )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	netchan->Clear( );

	return 0;
}

META_FUNCTION( CNetChan, Shutdown )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	netchan->Shutdown( LUA->GetString( 2 ) );

	return 0;
}

META_FUNCTION( CNetChan, Transmit )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	bool onlyReliable = false;

	if( LUA->IsType( 2, GarrysMod::Lua::Type::BOOL ) )
		onlyReliable = LUA->GetBool( 2 );

	LUA->PushBool( netchan->Transmit( onlyReliable ) );

	return 1;
}

META_FUNCTION( CNetChan, SendFile )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	LUA->PushBool( netchan->SendFile( LUA->GetString( 2 ), static_cast<uint32_t>( LUA->GetNumber( 3 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, DenyFile )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	netchan->DenyFile( LUA->GetString( 2 ), static_cast<uint32_t>( LUA->GetNumber( 3 ) ) );

	return 0;
}

META_FUNCTION( CNetChan, RequestFile )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushNumber( netchan->RequestFile( LUA->GetString( 2 ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetOutgoingQueueSize )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	VerifyStream( state, stream );

	LUA->PushNumber( netchan->waitlist[stream].Count( ) );

	return 1;
}

META_FUNCTION( CNetChan, GetOutgoingQueueFragments )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	int32_t ret = VerifyStream( state, stream );
	if( ret != 0 )
		return ret;

	int32_t offset = static_cast<int32_t>( LUA->GetNumber( 3 ) );
	if( offset < 0 || offset >= netchan->waitlist[stream].Count( ) )
	{
		LUA->PushNil( );
		return 1;
	}

	Push_dataFragments( state, netchan->waitlist[stream].Element( offset ), netchan );

	return 1;
}

META_FUNCTION( CNetChan, QueueOutgoingFragments )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GET_META_ID( dataFragments_t ) );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	int32_t ret = VerifyStream( state, stream );
	if( ret != 0 )
		return ret;

	dataFragments_t *fragments = Get_dataFragments( state, 3 );

	netchan->waitlist[stream].AddToTail( fragments );

	return 0;
}

META_FUNCTION( CNetChan, GetIncomingFragments )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	int32_t ret = VerifyStream( state, stream );
	if( ret != 0 )
		return ret;

	Push_dataFragments( state, &netchan->recvlist[stream], netchan );

	return 1;
}

META_FUNCTION( CNetChan, GetSubChannels )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	LUA->CreateTable( );

	for( int32_t i = 0; i < MAX_SUBCHANNELS; ++i )
	{
		Push_subchannel( state, &netchan->subchannels[i], netchan );

		LUA->PushNumber( i + 1 );

		LUA->SetTable( -3 );
	}

	return 1;
}

META_FUNCTION( CNetChan, GetReliableBuffer )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	Push_sn4_bf_write( state, &netchan->reliabledata );

	return 1;
}

META_FUNCTION( CNetChan, GetUnreliableBuffer )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	Push_sn4_bf_write( state, &netchan->unreliabledata );

	return 1;
}

META_FUNCTION( CNetChan, GetVoiceBuffer )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	Push_sn4_bf_write( state, &netchan->voicedata );

	return 1;
}

META_FUNCTION( CNetChan, GetNetChannelHandler )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	Push_INetChannelHandler( state, netchan->GetMsgHandler( ) );

	return 1;
}

META_FUNCTION( CNetChan, GetAddress )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	Push_netadr( state, netchan->remote_address );
	
	return 1;
}

META_FUNCTION( CNetChan, GetTime )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	LUA->PushNumber( netchan->GetTime( ) );

	return 1;
}

META_FUNCTION( CNetChan, GetLatency )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetLatency( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

EXT_META_FUNCTION( CNetChan, GetAvgLatency )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetAvgLatency( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetAvgLoss )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetAvgLoss( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetAvgChoke )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetAvgChoke( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetAvgData )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetAvgData( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetAvgPackets )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetAvgPackets( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetTotalData )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetTotalData( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetSequenceNr )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetSequenceNr( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, IsValidPacket )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	LUA->PushBool( netchan->IsValidPacket( static_cast<int32_t>( LUA->GetNumber( 2 ) ), static_cast<int32_t>( LUA->GetNumber( 3 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetPacketTime )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetPacketTime( static_cast<int32_t>( LUA->GetNumber( 2 ) ), static_cast<int32_t>( LUA->GetNumber( 3 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetPacketBytes )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 4, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetPacketBytes( static_cast<int32_t>( LUA->GetNumber( 2 ) ), static_cast<int32_t>( LUA->GetNumber( 3 ) ), static_cast<int32_t>( LUA->GetNumber( 4 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetStreamProgress )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t received = 0, total = 0;

	if( netchan->GetStreamProgress( static_cast<int32_t>( LUA->GetNumber( 2 ) ), &received, &total ) )
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

META_FUNCTION( CNetChan, GetCommandInterpolationAmount )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetCommandInterpolationAmount( static_cast<int32_t>( LUA->GetNumber( 2 ) ), static_cast<int32_t>( LUA->GetNumber( 3 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetPacketResponseLatency )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	int32_t latencymsecs = 0, choke = 0;

	netchan->GetPacketResponseLatency( static_cast<int32_t>( LUA->GetNumber( 2 ) ), static_cast<int32_t>( LUA->GetNumber( 3 ) ), &latencymsecs, &choke );

	LUA->PushNumber( latencymsecs );
	LUA->PushNumber( choke );

	return 2;
}

META_FUNCTION( CNetChan, GetRemoteFramerate )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	float frametime = 0.0f, frametimedev = 0.0f;

	netchan->GetRemoteFramerate( &frametime, &frametimedev );

	LUA->PushNumber( frametime );
	LUA->PushNumber( frametimedev );

	return 2;
}

META_FUNCTION( CNetChan, SetInterpolationAmount )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->SetInterpolationAmount( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( CNetChan, SetRemoteFramerate )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	netchan->SetRemoteFramerate( LUA->GetNumber( 2 ), LUA->GetNumber( 3 ) );

	return 0;
}

META_FUNCTION( CNetChan, SetMaxBufferSize )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 4, GarrysMod::Lua::Type::BOOL );

	netchan->SetMaxBufferSize( LUA->GetBool( 2 ), static_cast<int32_t>( LUA->GetNumber( 3 ) ), LUA->GetBool( 4 ) );

	return 0;
}

META_FUNCTION( CNetChan, IsPlayback )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	LUA->PushBool( netchan->IsPlayback( ) );

	return 1;
}

META_FUNCTION( CNetChan, GetTimeoutSeconds )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	LUA->PushNumber( netchan->timeout_seconds );

	return 1;
}

META_FUNCTION( CNetChan, SetTimeoutSeconds )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->timeout_seconds = LUA->GetNumber( 2 );

	return 1;
}

META_FUNCTION( CNetChan, GetConnectTime )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	LUA->PushNumber( netchan->connect_time );

	return 1;
}

META_FUNCTION( CNetChan, SetConnectTime )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->connect_time = LUA->GetNumber( 2 );

	return 1;
}

META_FUNCTION( CNetChan, GetLastReceivedTime )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	LUA->PushNumber( netchan->last_received );

	return 1;
}

META_FUNCTION( CNetChan, SetLastReceivedTime )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->last_received = LUA->GetNumber( 2 );

	return 0;
}

META_FUNCTION( CNetChan, GetName )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	LUA->PushString( netchan->name );

	return 1;
}

META_FUNCTION( CNetChan, SetName )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	strncpy( netchan->name, LUA->GetString( 2 ), 32 );
	netchan->name[sizeof( netchan->name )] = '\0';

	return 0;
}

META_FUNCTION( CNetChan, GetRate )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	
	LUA->PushNumber( netchan->rate );

	return 1;
}

META_FUNCTION( CNetChan, SetRate )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->rate = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( CNetChan, GetBackgroundMode )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	LUA->PushBool( netchan->backgroundmode );

	return 1;
}

META_FUNCTION( CNetChan, SetBackgroundMode )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	netchan->backgroundmode = LUA->GetBool( 2 );

	return 0;
}

META_FUNCTION( CNetChan, GetCompressionMode )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	LUA->PushBool( netchan->usecompression );

	return 1;
}

META_FUNCTION( CNetChan, SetCompressionMode )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	netchan->usecompression = LUA->GetBool( 2 );

	return 0;
}

META_FUNCTION( CNetChan, GetMaxRoutablePayloadSize )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );

	LUA->PushNumber( netchan->splitsize );

	return 1;
}

META_FUNCTION( CNetChan, SetMaxRoutablePayloadSize )
{
	CNetChan *netchan = Get_CNetChan( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->SetMaxRoutablePayloadSize( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

GLBL_FUNCTION( CNetChan )
{

#if defined SOURCENET_SERVER

	LUA->CheckType( 1, GarrysMod::Lua::Type::NUMBER );

	Push_CNetChan( state, static_cast<CNetChan *>( g_pEngineServer->GetPlayerNetInfo( static_cast<int32_t>( LUA->GetNumber( 1 ) ) ) ) );	

#elif defined SOURCENET_CLIENT

	Push_CNetChan( state, static_cast<CNetChan *>( g_pEngineClient->GetNetChannelInfo( ) ) );

#endif

	return 1;
}