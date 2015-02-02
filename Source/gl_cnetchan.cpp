#define IVENGINESERVER_INTERFACE
#define IVENGINECLIENT_INTERFACE

#include <gl_cnetchan.hpp>
#include <gl_inetchannelhandler.hpp>
#include <gl_subchannel_t.hpp>
#include <gl_datafragments_t.hpp>
#include <gl_bitbuf_write.hpp>
#include <gl_netadr_t.hpp>

#include <net.h>

#include <inetmessage.h>

META_ID( CNetChan, 3 );

META_FUNCTION( CNetChan, DumpMessages )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );
	for( int i = 0; i < netchan->netmessages.Count( ); ++i )
	{
		INetMessage *netmsg = netchan->netmessages.Element( i );
		Msg( "%d. %s (%d)\n", i + 1, netmsg->GetName( ), netmsg->GetType( ) );
	}

	return 0;
}

META_FUNCTION( CNetChan, Reset )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	netchan->Reset( );

	return 0;
}

META_FUNCTION( CNetChan, Clear )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	netchan->Clear( );

	return 0;
}

META_FUNCTION( CNetChan, Shutdown )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	CNetChan *netchan = GET_META( 1, CNetChan );

	netchan->Shutdown( LUA->GetString( 2 ) );

	return 0;
}

META_FUNCTION( CNetChan, Transmit )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	bool onlyReliable = false;

	if( LUA->IsType( 2, GarrysMod::Lua::Type::BOOL ) )
		onlyReliable = LUA->GetBool( 2 );

	LUA->PushBool( netchan->Transmit( onlyReliable ) );

	return 1;
}

META_FUNCTION( CNetChan, SendFile )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushBool( netchan->SendFile( LUA->GetString( 2 ), static_cast<uint32_t>( LUA->GetNumber( 3 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, DenyFile )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	netchan->DenyFile( LUA->GetString( 2 ), static_cast<uint32_t>( LUA->GetNumber( 3 ) ) );

	return 0;
}

META_FUNCTION( CNetChan, RequestFile )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushNumber( netchan->RequestFile( LUA->GetString( 2 ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetOutgoingQueueSize )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	int stream = static_cast<int>( LUA->GetNumber( 2 ) );

	VerifyStream( stream );

	LUA->PushNumber( netchan->waitlist[stream].Count( ) );

	return 1;
}

META_FUNCTION( CNetChan, GetOutgoingQueueFragments )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	int stream = static_cast<int>( LUA->GetNumber( 2 ) );

	VerifyStream( stream );

	int offset = static_cast<int>( LUA->GetNumber( 2 ) );

	VerifyOffset( netchan->waitlist[stream], offset );

	PUSH_META( netchan->waitlist[stream].Element( offset ), dataFragments_t );

	return 1;
}

META_FUNCTION( CNetChan, QueueOutgoingFragments )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GET_META_ID( dataFragments_t ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	int stream = static_cast<int>( LUA->GetNumber( 2 ) );

	VerifyStream( stream );

	dataFragments_t *fragments = GET_META( 3, dataFragments_t );

	netchan->waitlist[stream].AddToTail( fragments );

	return 0;
}

META_FUNCTION( CNetChan, GetIncomingFragments )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	int stream = static_cast<int>( LUA->GetNumber( 2 ) );

	VerifyStream( stream );

	PUSH_META( &netchan->recvlist[stream], dataFragments_t );

	return 1;
}

META_FUNCTION( CNetChan, GetSubChannels )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->CreateTable( );

	for( int i = 0; i < MAX_SUBCHANNELS; ++i )
	{
		subchannel_t *subchan = &netchan->subchannels[i];

		GarrysMod::Lua::UserData *userdata = static_cast<GarrysMod::Lua::UserData *>( LUA->NewUserdata( sizeof( GarrysMod::Lua::UserData ) ) );
		userdata->data = subchan;
		userdata->type = GET_META_ID( subchannel_t );

		LUA->CreateMetaTableType( GET_META_NAME( subchannel_t ), GET_META_ID( subchannel_t ) );
		LUA->SetMetaTable( -2 );

		LUA->PushNumber( i + 1 );

		LUA->SetTable( -3 );
	}

	return 1;
}

META_FUNCTION( CNetChan, GetReliableBuffer )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	PUSH_META( &netchan->reliabledata, sn4_bf_write );

	return 1;
}

META_FUNCTION( CNetChan, GetUnreliableBuffer )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	PUSH_META( &netchan->unreliabledata, sn4_bf_write );

	return 1;
}

META_FUNCTION( CNetChan, GetVoiceBuffer )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	PUSH_META( &netchan->voicedata, sn4_bf_write );

	return 1;
}

META_FUNCTION( CNetChan, GetNetChannelHandler )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	PUSH_META( netchan->GetMsgHandler( ), INetChannelHandler );

	return 1;
}

META_FUNCTION( CNetChan, GetAddress )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	PUSH_META( &netchan->remote_address, netadr_t );
	
	return 1;
}

META_FUNCTION( CNetChan, GetTime )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushNumber( netchan->GetTime( ) );

	return 1;
}

META_FUNCTION( CNetChan, GetLatency )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushNumber( netchan->GetLatency( static_cast<int>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

EXT_META_FUNCTION( CNetChan, GetAvgLatency )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushNumber( netchan->GetAvgLatency( static_cast<int>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetAvgLoss )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushNumber( netchan->GetAvgLoss( static_cast<int>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetAvgChoke )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushNumber( netchan->GetAvgChoke( static_cast<int>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetAvgData )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushNumber( netchan->GetAvgData( static_cast<int>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetAvgPackets )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushNumber( netchan->GetAvgPackets( static_cast<int>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetTotalData )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushNumber( netchan->GetTotalData( static_cast<int>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetSequenceNr )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushNumber( netchan->GetSequenceNr( static_cast<int>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, IsValidPacket )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushBool( netchan->IsValidPacket( static_cast<int>( LUA->GetNumber( 2 ) ), static_cast<int>( LUA->GetNumber( 3 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetPacketTime )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushNumber( netchan->GetPacketTime( static_cast<int>( LUA->GetNumber( 2 ) ), static_cast<int>( LUA->GetNumber( 3 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetPacketBytes )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 4, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushNumber( netchan->GetPacketBytes( static_cast<int>( LUA->GetNumber( 2 ) ), static_cast<int>( LUA->GetNumber( 3 ) ), static_cast<int>( LUA->GetNumber( 4 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetStreamProgress )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	int received = 0, total = 0;

	if( netchan->GetStreamProgress( static_cast<int>( LUA->GetNumber( 2 ) ), &received, &total ) )
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
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushNumber( netchan->GetCommandInterpolationAmount( static_cast<int>( LUA->GetNumber( 2 ) ), static_cast<int>( LUA->GetNumber( 3 ) ) ) );

	return 1;
}

META_FUNCTION( CNetChan, GetPacketResponseLatency )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	int latencymsecs = 0, choke = 0;

	netchan->GetPacketResponseLatency( static_cast<int>( LUA->GetNumber( 2 ) ), static_cast<int>( LUA->GetNumber( 3 ) ), &latencymsecs, &choke );

	LUA->PushNumber( latencymsecs );
	LUA->PushNumber( choke );

	return 2;
}

META_FUNCTION( CNetChan, GetRemoteFramerate )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	float frametime = 0.0f, frametimedev = 0.0f;

	netchan->GetRemoteFramerate( &frametime, &frametimedev );

	LUA->PushNumber( frametime );
	LUA->PushNumber( frametimedev );

	return 2;
}

META_FUNCTION( CNetChan, SetInterpolationAmount )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	netchan->SetInterpolationAmount( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( CNetChan, SetRemoteFramerate )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	netchan->SetRemoteFramerate( LUA->GetNumber( 2 ), LUA->GetNumber( 3 ) );

	return 0;
}

META_FUNCTION( CNetChan, SetMaxBufferSize )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 4, GarrysMod::Lua::Type::BOOL );
	
	CNetChan *netchan = GET_META( 1, CNetChan );

	netchan->SetMaxBufferSize( LUA->GetBool( 2 ), static_cast<int>( LUA->GetNumber( 3 ) ), LUA->GetBool( 4 ) );

	return 0;
}

META_FUNCTION( CNetChan, IsPlayback )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushBool( netchan->IsPlayback( ) );

	return 1;
}

META_FUNCTION( CNetChan, GetTimeoutSeconds )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushNumber( netchan->timeout_seconds );

	return 1;
}

META_FUNCTION( CNetChan, SetTimeoutSeconds )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	netchan->timeout_seconds = LUA->GetNumber( 2 );

	return 1;
}

META_FUNCTION( CNetChan, GetConnectTime )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushNumber( netchan->connect_time );

	return 1;
}

META_FUNCTION( CNetChan, SetConnectTime )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	netchan->connect_time = LUA->GetNumber( 2 );

	return 1;
}

META_FUNCTION( CNetChan, GetLastReceivedTime )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushNumber( netchan->last_received );

	return 1;
}

META_FUNCTION( CNetChan, SetLastReceivedTime )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	netchan->last_received = LUA->GetNumber( 2 );

	return 0;
}

META_FUNCTION( CNetChan, GetName )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushString( netchan->name );

	return 1;
}

META_FUNCTION( CNetChan, SetName )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	CNetChan *netchan = GET_META( 1, CNetChan );

	strcpy_s( netchan->name, 32, LUA->GetString( 2 ) );

	return 0;
}

META_FUNCTION( CNetChan, GetRate )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );
	
	LUA->PushNumber( netchan->rate );

	return 1;
}

META_FUNCTION( CNetChan, SetRate )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	netchan->rate = static_cast<int>( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( CNetChan, GetBackgroundMode )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushBool( netchan->backgroundmode );

	return 1;
}

META_FUNCTION( CNetChan, SetBackgroundMode )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	CNetChan *netchan = GET_META( 1, CNetChan );

	netchan->backgroundmode = LUA->GetBool( 2 );

	return 0;
}

META_FUNCTION( CNetChan, GetCompressionMode )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushBool( netchan->usecompression );

	return 1;
}

META_FUNCTION( CNetChan, SetCompressionMode )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	CNetChan *netchan = GET_META( 1, CNetChan );

	netchan->usecompression = LUA->GetBool( 2 );

	return 0;
}

META_FUNCTION( CNetChan, GetMaxRoutablePayloadSize )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );

	CNetChan *netchan = GET_META( 1, CNetChan );

	LUA->PushNumber( netchan->splitsize );

	return 1;
}

META_FUNCTION( CNetChan, SetMaxRoutablePayloadSize )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	CNetChan *netchan = GET_META( 1, CNetChan );

	netchan->SetMaxRoutablePayloadSize( static_cast<int>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( CNetChan, __eq )
{
	LUA->CheckType( 1, GET_META_ID( CNetChan ) );
	LUA->CheckType( 2, GET_META_ID( CNetChan ) );

	CNetChan *netchan1 = GET_META( 1, CNetChan );
	CNetChan *netchan2 = GET_META( 2, CNetChan );

	LUA->PushBool( netchan1 == netchan2 );

	return 1;
}

GLBL_FUNCTION( CNetChan )
{
	CNetChan *netchan = nullptr;
	if( !LUA->IsClient( ) )
	{
		LUA->CheckType( 1, GarrysMod::Lua::Type::NUMBER );

		netchan = static_cast<CNetChan *>( g_pEngineServer->GetPlayerNetInfo( static_cast<int>( LUA->GetNumber( 1 ) ) ) );	
	}
	else
	{
		netchan = static_cast<CNetChan *>( g_pEngineClient->GetNetChannelInfo( ) );
	}

	if( netchan != nullptr )
	{
		PUSH_META( netchan, CNetChan );
	}
	else
	{
		LUA->PushNil( );
	}

	return 1;
}