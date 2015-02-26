#include <netchannel.hpp>
#include <netchannelhandler.hpp>
#include <subchannel.hpp>
#include <datafragments.hpp>
#include <sn4_bf_write.hpp>
#include <netadr.hpp>
#include <hooks.hpp>
#include <netmessage.hpp>
#include <net.hpp>
#include <inetmessage.h>
#include <unordered_map>
#include <sstream>

namespace NetChannel
{

struct userdata
{
	CNetChan *netchan;
	uint8_t type;
};

static const uint8_t metaid = Global::metabase + 3;
static const char *metaname = "CNetChan";

static std::unordered_map<CNetChan *, int32_t> netchannels;

bool IsValid( CNetChan *netchan )
{
	return netchan != nullptr && netchannels.find( netchan ) != netchannels.end( );
}

void Push( lua_State *state, CNetChan *netchan )
{
	if( netchan == nullptr )
	{
		LUA->PushNil( );
		return;
	}

	auto it = netchannels.find( netchan );
	if( it != netchannels.end( ) )
	{
		LUA->ReferencePush( ( *it ).second );
		return;
	}

	userdata *udata = static_cast<userdata *>( LUA->NewUserdata( sizeof( userdata ) ) );
	udata->type = metaid;
	udata->netchan = netchan;

	LUA->CreateMetaTableType( metaname, metaid );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( state, -2 );

	LUA->Push( -1 );
	netchannels[netchan] = LUA->ReferenceCreate( );

	Hooks::HookCNetChan( state );
}

CNetChan *Get( lua_State *state, int32_t index )
{
	Global::CheckType( state, index, metaid, metaname );

	CNetChan *netchan = static_cast<userdata *>( LUA->GetUserdata( index ) )->netchan;
	if( !IsValid( netchan ) )
		Global::ThrowError( state, "invalid %s", metaname );

	return netchan;
}

void Destroy( lua_State *state, CNetChan *netchan )
{
	auto it = netchannels.find( netchan );
	if( it != netchannels.end( ) )
	{
		LUA->ReferenceFree( ( *it ).second );
		netchannels.erase( it );
	}
}

LUA_FUNCTION_STATIC( eq )
{
	CNetChan *netchan1 = Get( state, 1 );
	CNetChan *netchan2 = Get( state, 2 );

	LUA->PushBool( netchan1 == netchan2 );

	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	CNetChan *netchan = Get( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", metaname, netchan );

	return 1;
}

LUA_FUNCTION_STATIC( IsValid )
{
	Global::CheckType( state, 1, metaid, metaname );

	LUA->PushBool( IsValid( static_cast<userdata *>( LUA->GetUserdata( 1 ) )->netchan ) );

	return 1;
}

LUA_FUNCTION_STATIC( DumpNetMessages )
{
	CNetChan *netchan = Get( state, 1 );

	std::ostringstream stream;
	for( int32_t i = 0; i < netchan->netmessages.Count( ); ++i )
	{
		if( i != 0 )
			stream << '\n';

		INetMessage *netmsg = netchan->netmessages.Element( i );
		stream << i + 1 << ". " << netmsg->GetName( ) << " (" << netmsg->GetType( ) << ')';
	}

	LUA->PushString( stream.str( ).c_str( ) );
	return 1;
}

LUA_FUNCTION_STATIC( GetNetMessageNum )
{
	CNetChan *netchan = Get( state, 1 );

	LUA->PushNumber( netchan->netmessages.Count( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetNetMessage )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t idx = static_cast<int32_t>( LUA->GetNumber( 2 ) ) - 1;
	if( idx < 0 || idx >= netchan->netmessages.Count( ) )
		LUA->ThrowError( "invalid netmessage index" );

	NetMessage::Push( state, netchan->netmessages.Element( idx ), netchan );

	return 1;
}

LUA_FUNCTION_STATIC( Reset )
{
	CNetChan *netchan = Get( state, 1 );

	netchan->Reset( );

	return 0;
}

LUA_FUNCTION_STATIC( Clear )
{
	CNetChan *netchan = Get( state, 1 );

	netchan->Clear( );

	return 0;
}

LUA_FUNCTION_STATIC( Shutdown )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	netchan->Shutdown( LUA->GetString( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( Transmit )
{
	CNetChan *netchan = Get( state, 1 );

	bool onlyReliable = false;

	if( LUA->IsType( 2, GarrysMod::Lua::Type::BOOL ) )
		onlyReliable = LUA->GetBool( 2 );

	LUA->PushBool( netchan->Transmit( onlyReliable ) );

	return 1;
}

LUA_FUNCTION_STATIC( SendFile )
{
	CNetChan *netchan = Get( state, 1 );
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
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	netchan->DenyFile( LUA->GetString( 2 ), static_cast<uint32_t>( LUA->GetNumber( 3 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( RequestFile )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushNumber( netchan->RequestFile( LUA->GetString( 2 ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetOutgoingQueueSize )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );
	if( stream < 0 || stream >= MAX_STREAMS )
		return 0;

	LUA->PushNumber( netchan->waitlist[stream].Count( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetOutgoingQueueFragments )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );
	if( stream < 0 || stream >= MAX_STREAMS )
		return 0;

	int32_t offset = static_cast<int32_t>( LUA->GetNumber( 3 ) );
	if( offset < 0 || offset >= netchan->waitlist[stream].Count( ) )
		return 0;

	dataFragments::Push( state, netchan->waitlist[stream].Element( offset ), netchan );

	return 1;
}

LUA_FUNCTION_STATIC( QueueOutgoingFragments )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );
	if( stream < 0 || stream >= MAX_STREAMS )
		return 0;

	dataFragments_t *fragments = dataFragments::Get( state, 3 );

	netchan->waitlist[stream].AddToTail( fragments );

	return 0;
}

LUA_FUNCTION_STATIC( GetIncomingFragments )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );
	if( stream < 0 || stream >= MAX_STREAMS )
		return 0;

	dataFragments::Push( state, &netchan->recvlist[stream], netchan );

	return 1;
}

LUA_FUNCTION_STATIC( GetSubChannels )
{
	CNetChan *netchan = Get( state, 1 );

	LUA->CreateTable( );

	for( int32_t i = 0; i < MAX_SUBCHANNELS; ++i )
	{
		subchannel::Push( state, &netchan->subchannels[i], netchan );

		LUA->PushNumber( i + 1 );

		LUA->SetTable( -3 );
	}

	return 1;
}

LUA_FUNCTION_STATIC( GetReliableBuffer )
{
	CNetChan *netchan = Get( state, 1 );

	sn4_bf_write::Push( state, &netchan->reliabledata );

	return 1;
}

LUA_FUNCTION_STATIC( GetUnreliableBuffer )
{
	CNetChan *netchan = Get( state, 1 );

	sn4_bf_write::Push( state, &netchan->unreliabledata );

	return 1;
}

LUA_FUNCTION_STATIC( GetVoiceBuffer )
{
	CNetChan *netchan = Get( state, 1 );

	sn4_bf_write::Push( state, &netchan->voicedata );

	return 1;
}

LUA_FUNCTION_STATIC( GetNetChannelHandler )
{
	CNetChan *netchan = Get( state, 1 );

	NetChannelHandler::Push( state, netchan->GetMsgHandler( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetAddress )
{
	CNetChan *netchan = Get( state, 1 );

	netadr::Push( state, netchan->remote_address );
	
	return 1;
}

LUA_FUNCTION_STATIC( GetTime )
{
	CNetChan *netchan = Get( state, 1 );

	LUA->PushNumber( netchan->GetTime( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetLatency )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetLatency( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetAvgLatency )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetAvgLatency( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetAvgLoss )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetAvgLoss( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetAvgChoke )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetAvgChoke( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetAvgData )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetAvgData( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetAvgPackets )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetAvgPackets( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetTotalData )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetTotalData( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetSequenceNr )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( netchan->GetSequenceNr( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsValidPacket )
{
	CNetChan *netchan = Get( state, 1 );
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
	CNetChan *netchan = Get( state, 1 );
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
	CNetChan *netchan = Get( state, 1 );
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
	CNetChan *netchan = Get( state, 1 );
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
	CNetChan *netchan = Get( state, 1 );
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
	CNetChan *netchan = Get( state, 1 );
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
	CNetChan *netchan = Get( state, 1 );

	float frametime = 0.0f, frametimedev = 0.0f;
	netchan->GetRemoteFramerate( &frametime, &frametimedev );

	LUA->PushNumber( frametime );
	LUA->PushNumber( frametimedev );

	return 2;
}

LUA_FUNCTION_STATIC( SetInterpolationAmount )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->SetInterpolationAmount( LUA->GetNumber( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( SetRemoteFramerate )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	netchan->SetRemoteFramerate( LUA->GetNumber( 2 ), LUA->GetNumber( 3 ) );

	return 0;
}

LUA_FUNCTION_STATIC( SetMaxBufferSize )
{
	CNetChan *netchan = Get( state, 1 );
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

LUA_FUNCTION_STATIC( IsPlayback )
{
	CNetChan *netchan = Get( state, 1 );

	LUA->PushBool( netchan->IsPlayback( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetTimeoutSeconds )
{
	CNetChan *netchan = Get( state, 1 );

	LUA->PushNumber( netchan->timeout_seconds );

	return 1;
}

LUA_FUNCTION_STATIC( SetTimeoutSeconds )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->timeout_seconds = LUA->GetNumber( 2 );

	return 1;
}

LUA_FUNCTION_STATIC( GetConnectTime )
{
	CNetChan *netchan = Get( state, 1 );

	LUA->PushNumber( netchan->connect_time );

	return 1;
}

LUA_FUNCTION_STATIC( SetConnectTime )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->connect_time = LUA->GetNumber( 2 );

	return 1;
}

LUA_FUNCTION_STATIC( GetLastReceivedTime )
{
	CNetChan *netchan = Get( state, 1 );

	LUA->PushNumber( netchan->last_received );

	return 1;
}

LUA_FUNCTION_STATIC( SetLastReceivedTime )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->last_received = LUA->GetNumber( 2 );

	return 0;
}

LUA_FUNCTION_STATIC( GetName )
{
	CNetChan *netchan = Get( state, 1 );

	LUA->PushString( netchan->name );

	return 1;
}

LUA_FUNCTION_STATIC( SetName )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	strncpy( netchan->name, LUA->GetString( 2 ), 32 );
	netchan->name[sizeof( netchan->name )] = '\0';

	return 0;
}

LUA_FUNCTION_STATIC( GetRate )
{
	CNetChan *netchan = Get( state, 1 );
	
	LUA->PushNumber( netchan->rate );

	return 1;
}

LUA_FUNCTION_STATIC( SetRate )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->rate = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( GetBackgroundMode )
{
	CNetChan *netchan = Get( state, 1 );

	LUA->PushBool( netchan->backgroundmode );

	return 1;
}

LUA_FUNCTION_STATIC( SetBackgroundMode )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	netchan->backgroundmode = LUA->GetBool( 2 );

	return 0;
}

LUA_FUNCTION_STATIC( GetCompressionMode )
{
	CNetChan *netchan = Get( state, 1 );

	LUA->PushBool( netchan->usecompression );

	return 1;
}

LUA_FUNCTION_STATIC( SetCompressionMode )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	netchan->usecompression = LUA->GetBool( 2 );

	return 0;
}

LUA_FUNCTION_STATIC( GetMaxRoutablePayloadSize )
{
	CNetChan *netchan = Get( state, 1 );

	LUA->PushNumber( netchan->splitsize );

	return 1;
}

LUA_FUNCTION_STATIC( SetMaxRoutablePayloadSize )
{
	CNetChan *netchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	netchan->SetMaxRoutablePayloadSize( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( Constructor )
{

#if defined SOURCENET_SERVER

	LUA->CheckType( 1, GarrysMod::Lua::Type::NUMBER );

	int32_t index = static_cast<int32_t>( LUA->GetNumber( 1 ) );
	Push(
		state,
		static_cast<CNetChan *>( Global::engine_server->GetPlayerNetInfo( index ) )
	);	

#elif defined SOURCENET_CLIENT

	Push( state, static_cast<CNetChan *>( Global::engine_client->GetNetChannelInfo( ) ) );

#endif

	return 1;
}

void Initialize( lua_State *state )
{
	LUA->CreateMetaTableType( metaname, metaid );

		LUA->PushCFunction( eq );
		LUA->SetField( -2, "__eq" );

		LUA->PushCFunction( tostring );
		LUA->SetField( -2, "__tostring" );

		LUA->PushCFunction( Global::index );
		LUA->SetField( -2, "__index" );

		LUA->PushCFunction( Global::newindex );
		LUA->SetField( -2, "__newindex" );

		LUA->PushCFunction( IsValid );
		LUA->SetField( -2, "IsValid" );

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

		LUA->PushCFunction( SendFile );
		LUA->SetField( -2, "SendFile" );

		LUA->PushCFunction( DenyFile );
		LUA->SetField( -2, "DenyFile" );

		LUA->PushCFunction( RequestFile );
		LUA->SetField( -2, "RequestFile" );

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

		LUA->PushCFunction( IsPlayback );
		LUA->SetField( -2, "IsPlayback" );

		LUA->PushCFunction( GetTimeoutSeconds );
		LUA->SetField( -2, "GetTimeoutSeconds" );

		LUA->PushCFunction( SetTimeoutSeconds );
		LUA->SetField( -2, "SetTimeoutSeconds" );

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

	LUA->Pop( 1 );

	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );

		LUA->PushCFunction( Constructor );
		LUA->SetField( -2, metaname );

	LUA->Pop( 1 );
}

void Deinitialize( lua_State *state )
{
	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_REG );

		LUA->PushNil( );
		LUA->SetField( -2, metaname );

	LUA->Pop( 1 );

	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );

		LUA->PushNil( );
		LUA->SetField( -2, metaname );

	LUA->Pop( 1 );

	for( auto pair : netchannels )
		LUA->ReferenceFree( pair.second );
}

}