#include <netmessages.hpp>
#include <netmessage.hpp>
#include <sn4_bf_write.hpp>
#include <sn4_bf_read.hpp>

#undef STRING

#define CHECK_NETMESSAGE_TYPE( classname, classtype ) \
		classname *msg = static_cast<classname *>( NetMessage::Get( state, 1 ) ); \
		if( msg->GetType( ) != classtype ) \
			LUA->ThrowError( "tried to access netmessage member of wrong type" );

#define SETUP_COMMON_LUA_ACCESSORS( classname, classtype, luafunc, member ) \
	LUA_FUNCTION_STATIC( classname ## _Get ## member ) \
	{ \
		CHECK_NETMESSAGE_TYPE( classname, classtype ); \
		LUA->luafunc( msg->member ); \
		return 1; \
	}

#define SETUP_STRING_POINTER_LUA_ACCESSORS( classname, classtype, member ) \
	SETUP_COMMON_LUA_ACCESSORS( classname, classtype, PushString, member ); \
	LUA_FUNCTION_STATIC( classname ## _Set ## member ) \
	{ \
		CHECK_NETMESSAGE_TYPE( classname, classtype ); \
		LUA->CheckType( 2, GarrysMod::Lua::Type::STRING ); \
		msg->member = LUA->GetString( 2 ); \
		return 0; \
	}

#define SETUP_STRING_LUA_ACCESSORS( classname, classtype, member ) \
	SETUP_COMMON_LUA_ACCESSORS( classname, classtype, PushString, member ); \
	LUA_FUNCTION_STATIC( classname ## _Set ## member ) \
	{ \
		CHECK_NETMESSAGE_TYPE( classname, classtype ); \
		LUA->CheckType( 2, GarrysMod::Lua::Type::STRING ); \
		V_strncpy( msg->member, LUA->GetString( 2 ), sizeof( msg->member ) ); \
		return 0; \
	}

#define SETUP_ARRAY_LUA_ACCESSORS( classname, classtype, member ) \
	LUA_FUNCTION_STATIC( classname ## _Get ## member ) \
	{ \
		CHECK_NETMESSAGE_TYPE( classname, classtype ); \
		LUA->PushString( reinterpret_cast<const char *>( msg->member ), sizeof( msg->member ) ); \
		return 1; \
	} \
	LUA_FUNCTION_STATIC( classname ## _Set ## member ) \
	{ \
		CHECK_NETMESSAGE_TYPE( classname, classtype ); \
		LUA->CheckType( 2, GarrysMod::Lua::Type::STRING ); \
		size_t datalen = 0; \
		const char *data = LUA->GetString( 2, &datalen ); \
		V_memcpy( msg->member, data, sizeof( msg->member ) > datalen ? datalen : sizeof( msg->member ) ); \
		return 0; \
	}

#define SETUP_BOOL_LUA_ACCESSORS( classname, classtype, member ) \
	SETUP_COMMON_LUA_ACCESSORS( classname, classtype, PushBool, member ); \
	LUA_FUNCTION_STATIC( classname ## _Set ## member ) \
	{ \
		CHECK_NETMESSAGE_TYPE( classname, classtype ); \
		LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL ); \
		msg->member = LUA->GetBool( 2 ); \
		return 0; \
	}

#define SETUP_NUMBER_LUA_ACCESSORS( classname, classtype, membertype, member ) \
	SETUP_COMMON_LUA_ACCESSORS( classname, classtype, PushNumber, member ); \
	LUA_FUNCTION_STATIC( classname ## _Set ## member ) \
	{ \
		CHECK_NETMESSAGE_TYPE( classname, classtype ); \
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER ); \
		msg->member = static_cast<membertype>( LUA->GetNumber( 2 ) ); \
		return 0; \
	}

#define SETUP_ENUM_LUA_ACCESSORS( classname, classtype, membertype, member ) \
	SETUP_COMMON_LUA_ACCESSORS( classname, classtype, PushNumber, member ); \
	LUA_FUNCTION_STATIC( classname ## _Set ## member ) \
	{ \
		CHECK_NETMESSAGE_TYPE( classname, classtype ); \
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER ); \
		msg->member = static_cast<membertype>( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ); \
		return 0; \
	}

#define SETUP_ANGLE_LUA_ACCESSORS( classname, classtype, member ) \
	LUA_FUNCTION_STATIC( classname ## _Get ## member ) \
	{ \
		CHECK_NETMESSAGE_TYPE( classname, classtype ); \
		Global::PushAngle( state, msg->member ); \
		return 1; \
	} \
	LUA_FUNCTION_STATIC( classname ## _Set ## member ) \
	{ \
		CHECK_NETMESSAGE_TYPE( classname, classtype ); \
		LUA->CheckType( 2, GarrysMod::Lua::Type::ANGLE ); \
		msg->member = *static_cast<QAngle *>( \
			static_cast<GarrysMod::Lua::UserData *>( LUA->GetUserdata( 2 ) )->data \
		); \
		return 0; \
	}

#define SETUP_VECTOR_LUA_ACCESSORS( classname, classtype, member ) \
	LUA_FUNCTION_STATIC( classname ## _Get ## member ) \
	{ \
		CHECK_NETMESSAGE_TYPE( classname, classtype ); \
		Global::PushVector( state, msg->member ); \
		return 1; \
	} \
	LUA_FUNCTION_STATIC( classname ## _Set ## member ) \
	{ \
		CHECK_NETMESSAGE_TYPE( classname, classtype ); \
		LUA->CheckType( 2, GarrysMod::Lua::Type::VECTOR ); \
		msg->member = *static_cast<Vector *>( \
			static_cast<GarrysMod::Lua::UserData *>( LUA->GetUserdata( 2 ) )->data \
		); \
		return 0; \
	}

#define SETUP_READER_LUA_ACCESSORS( classname, classtype, member ) \
	LUA_FUNCTION_STATIC( classname ## _Get ## member ) \
	{ \
		CHECK_NETMESSAGE_TYPE( classname, classtype ); \
		sn4_bf_read::Push( state, &msg->member ); \
		return 1; \
	} \
	LUA_FUNCTION_STATIC( classname ## _Set ## member ) \
	{ \
		CHECK_NETMESSAGE_TYPE( classname, classtype ); \
		msg->member = *sn4_bf_read::Get( state, 2 ); \
		return 0; \
	}

#define SETUP_WRITER_LUA_ACCESSORS( classname, classtype, member ) \
	LUA_FUNCTION_STATIC( classname ## _Get ## member ) \
	{ \
		CHECK_NETMESSAGE_TYPE( classname, classtype ); \
		sn4_bf_write::Push( state, &msg->member ); \
		return 1; \
	} \
	LUA_FUNCTION_STATIC( classname ## _Set ## member ) \
	{ \
		CHECK_NETMESSAGE_TYPE( classname, classtype ); \
		msg->member = *sn4_bf_write::Get( state, 2 ); \
		return 0; \
	}

#define REGISTER_LUA_ACCESSORS( classname, member ) \
	LUA->PushCFunction( classname ## _Get ## member ); \
	LUA->SetField( -2, "Get" #member ); \
	LUA->PushCFunction( classname ## _Set ## member ); \
	LUA->SetField( -2, "Set" #member );

namespace NetMessages
{

CNetMessage::CNetMessage( ) :
	m_bReliable( true ), m_NetChannel( nullptr ), m_pMessageHandler( nullptr )
{ }

void CNetMessage::SetNetChannel( INetChannel *netchan )
{
	m_NetChannel = netchan;
}

void CNetMessage::SetReliable( bool state )
{
	m_bReliable = state;
}

bool CNetMessage::Process( )
{
	return false;
}

bool CNetMessage::ReadFromBuffer( bf_read &buffer )
{
	return false;
}

bool CNetMessage::WriteToBuffer( bf_write &buffer )
{
	return false;
}

bool CNetMessage::IsReliable( ) const
{
	return m_bReliable;
}

int32_t CNetMessage::GetType( ) const
{
	return -1;
}

int32_t CNetMessage::GetGroup( ) const
{
	return INetChannelInfo::GENERIC;
}

const char *CNetMessage::GetName( ) const
{
	return nullptr;
}

INetChannel *CNetMessage::GetNetChannel( ) const
{
	return m_NetChannel;
}

const char *CNetMessage::ToString( ) const
{
	return nullptr;
}

void CNetMessage::InstallVTable( void **vtable )
{
	*reinterpret_cast<void ***>( this ) = vtable;
}

void **CNetMessage::GetVTable( )
{
	return *reinterpret_cast<void ***>( this );
}

CNetMessage *NET_Tick::Create( )
{
	return new( std::nothrow ) NET_Tick;
}

SETUP_NUMBER_LUA_ACCESSORS( NET_Tick, net_Tick, int32_t, Tick );
SETUP_NUMBER_LUA_ACCESSORS( NET_Tick, net_Tick, float, HostFrameTime );
SETUP_NUMBER_LUA_ACCESSORS( NET_Tick, net_Tick, float, HostFrameTimeStdDeviation );

void NET_Tick::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( NET_Tick, Tick );
	REGISTER_LUA_ACCESSORS( NET_Tick, HostFrameTime );
	REGISTER_LUA_ACCESSORS( NET_Tick, HostFrameTimeStdDeviation );
}

NET_StringCmd::NET_StringCmd( ) :
	Command( nullptr )
{ }

CNetMessage *NET_StringCmd::Create( )
{
	return new( std::nothrow ) NET_StringCmd;
}

SETUP_STRING_POINTER_LUA_ACCESSORS( NET_StringCmd, net_StringCmd, Command );

void NET_StringCmd::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( NET_StringCmd, Command );
}

CNetMessage *NET_SetConVar::Create( )
{
	return new( std::nothrow ) NET_SetConVar;
}

LUA_FUNCTION_STATIC( NET_SetConVar_GetConVars )
{
	CHECK_NETMESSAGE_TYPE( NET_SetConVar, net_SetConVar );

	LUA->CreateTable( );

	auto &convars = msg->ConVars;
	for( int32_t k = 0; k < convars.Count( ); ++k )
	{
		LUA->PushString( convars[k].name );
		LUA->PushString( convars[k].value );
		LUA->SetTable( -3 );
	}

	return 1;
}

LUA_FUNCTION_STATIC( NET_SetConVar_SetConVars )
{
	CHECK_NETMESSAGE_TYPE( NET_SetConVar, net_SetConVar );
	LUA->CheckType( 2, GarrysMod::Lua::Type::TABLE );

	auto &convars = msg->ConVars;
	convars.RemoveAll( );

	NET_SetConVar::cvar_t cvar = { 0 };

	LUA->PushNil( );
	while( LUA->Next( -2 ) != 0 )
	{
		V_strncpy( cvar.name, LUA->GetString( -2 ), sizeof( cvar.name ) );

		V_strncpy( cvar.value, LUA->GetString( -1 ), sizeof( cvar.value ) );

		convars.AddToTail( cvar );

		LUA->Pop( 1 );
	}

	return 0;
}

void NET_SetConVar::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( NET_SetConVar, ConVars );
}

CNetMessage *NET_SignonState::Create( )
{
	return new( std::nothrow ) NET_SignonState;
}

SETUP_NUMBER_LUA_ACCESSORS( NET_SignonState, net_SignonState, int32_t, SignonState );
SETUP_NUMBER_LUA_ACCESSORS( NET_SignonState, net_SignonState, int32_t, SpawnCount );

void NET_SignonState::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( NET_SignonState, SignonState );
	REGISTER_LUA_ACCESSORS( NET_SignonState, SpawnCount );
}

SVC_Print::SVC_Print( ) :
	Text( nullptr )
{ }

CNetMessage *SVC_Print::Create( )
{
	return new( std::nothrow ) SVC_Print;
}

SETUP_STRING_POINTER_LUA_ACCESSORS( SVC_Print, svc_Print, Text );

void SVC_Print::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_Print, Text );
}

SVC_ServerInfo::SVC_ServerInfo( ) :
	GameDir( nullptr ), MapName( nullptr ), SkyName( nullptr ),
	HostName( nullptr ), LoadingURL( nullptr ), Gamemode( nullptr )
{ }

CNetMessage *SVC_ServerInfo::Create( )
{
	return new( std::nothrow ) SVC_ServerInfo;
}

SETUP_NUMBER_LUA_ACCESSORS( SVC_ServerInfo, svc_ServerInfo, int32_t, Protocol );
SETUP_NUMBER_LUA_ACCESSORS( SVC_ServerInfo, svc_ServerInfo, int32_t, ServerCount );
SETUP_BOOL_LUA_ACCESSORS( SVC_ServerInfo, svc_ServerInfo, Dedicated );
SETUP_BOOL_LUA_ACCESSORS( SVC_ServerInfo, svc_ServerInfo, HLTV );
SETUP_NUMBER_LUA_ACCESSORS( SVC_ServerInfo, svc_ServerInfo, char, OS );
SETUP_NUMBER_LUA_ACCESSORS( SVC_ServerInfo, svc_ServerInfo, CRC32_t, ClientCRC );
SETUP_ARRAY_LUA_ACCESSORS( SVC_ServerInfo, svc_ServerInfo, MapMD5 );
SETUP_NUMBER_LUA_ACCESSORS( SVC_ServerInfo, svc_ServerInfo, int32_t, MaxClients );
SETUP_NUMBER_LUA_ACCESSORS( SVC_ServerInfo, svc_ServerInfo, int32_t, MaxClasses );
SETUP_NUMBER_LUA_ACCESSORS( SVC_ServerInfo, svc_ServerInfo, int32_t, PlayerSlot );
SETUP_NUMBER_LUA_ACCESSORS( SVC_ServerInfo, svc_ServerInfo, float, TickInterval );
SETUP_STRING_POINTER_LUA_ACCESSORS( SVC_ServerInfo, svc_ServerInfo, GameDir );
SETUP_STRING_POINTER_LUA_ACCESSORS( SVC_ServerInfo, svc_ServerInfo, MapName );
SETUP_STRING_POINTER_LUA_ACCESSORS( SVC_ServerInfo, svc_ServerInfo, SkyName );
SETUP_STRING_POINTER_LUA_ACCESSORS( SVC_ServerInfo, svc_ServerInfo, HostName );
SETUP_STRING_POINTER_LUA_ACCESSORS( SVC_ServerInfo, svc_ServerInfo, LoadingURL );
SETUP_STRING_POINTER_LUA_ACCESSORS( SVC_ServerInfo, svc_ServerInfo, Gamemode );

void SVC_ServerInfo::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_ServerInfo, Protocol );
	REGISTER_LUA_ACCESSORS( SVC_ServerInfo, ServerCount );
	REGISTER_LUA_ACCESSORS( SVC_ServerInfo, Dedicated );
	REGISTER_LUA_ACCESSORS( SVC_ServerInfo, HLTV );
	REGISTER_LUA_ACCESSORS( SVC_ServerInfo, OS );
	REGISTER_LUA_ACCESSORS( SVC_ServerInfo, ClientCRC );
	REGISTER_LUA_ACCESSORS( SVC_ServerInfo, MapMD5 );
	REGISTER_LUA_ACCESSORS( SVC_ServerInfo, MaxClients );
	REGISTER_LUA_ACCESSORS( SVC_ServerInfo, MaxClasses );
	REGISTER_LUA_ACCESSORS( SVC_ServerInfo, PlayerSlot );
	REGISTER_LUA_ACCESSORS( SVC_ServerInfo, TickInterval );
	REGISTER_LUA_ACCESSORS( SVC_ServerInfo, GameDir );
	REGISTER_LUA_ACCESSORS( SVC_ServerInfo, MapName );
	REGISTER_LUA_ACCESSORS( SVC_ServerInfo, SkyName );
	REGISTER_LUA_ACCESSORS( SVC_ServerInfo, HostName );
	REGISTER_LUA_ACCESSORS( SVC_ServerInfo, LoadingURL );
	REGISTER_LUA_ACCESSORS( SVC_ServerInfo, Gamemode );
}

CNetMessage *SVC_SendTable::Create( )
{
	return new( std::nothrow ) SVC_SendTable;
}

SETUP_BOOL_LUA_ACCESSORS( SVC_SendTable, svc_SendTable, NeedsDecoder );
SETUP_NUMBER_LUA_ACCESSORS( SVC_SendTable, svc_SendTable, int32_t, Length );
SETUP_READER_LUA_ACCESSORS( SVC_SendTable, svc_SendTable, DataIn );
SETUP_WRITER_LUA_ACCESSORS( SVC_SendTable, svc_SendTable, DataOut );

void SVC_SendTable::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_SendTable, NeedsDecoder );
	REGISTER_LUA_ACCESSORS( SVC_SendTable, Length );
	REGISTER_LUA_ACCESSORS( SVC_SendTable, DataIn );
	REGISTER_LUA_ACCESSORS( SVC_SendTable, DataOut );
}

CNetMessage *SVC_ClassInfo::Create( )
{
	return new( std::nothrow ) SVC_ClassInfo;
}

SETUP_BOOL_LUA_ACCESSORS( SVC_ClassInfo, svc_ClassInfo, CreateOnClient );

LUA_FUNCTION_STATIC( SVC_ClassInfo_GetClasses )
{
	CHECK_NETMESSAGE_TYPE( SVC_ClassInfo, svc_ClassInfo );

	LUA->CreateTable( );

	auto &classes = msg->Classes;
	for( int32_t k = 0; k < classes.Count( ); ++k )
	{
		LUA->CreateTable( );

		LUA->PushNumber( classes[k].classID );
		LUA->SetField( -2, "ClassID" );

		LUA->PushString( classes[k].classname );
		LUA->SetField( -2, "ClassName" );

		LUA->PushString( classes[k].datatablename );
		LUA->SetField( -2, "DataTableName" );

		LUA->SetField( -2, classes[k].classname );
	}

	return 1;
}

LUA_FUNCTION_STATIC( SVC_ClassInfo_SetClasses )
{
	CHECK_NETMESSAGE_TYPE( SVC_ClassInfo, svc_ClassInfo );
	LUA->CheckType( 2, GarrysMod::Lua::Type::TABLE );

	auto &classes = msg->Classes;
	classes.RemoveAll( );

	SVC_ClassInfo::class_t cl = { 0 };

	LUA->PushNil( );
	while( LUA->Next( -2 ) != 0 )
	{
		LUA->GetField( -1, "ClassID" );
		cl.classID = LUA->GetNumber( -1 );

		LUA->GetField( -2, "ClassName" );
		V_strncpy( cl.classname, LUA->GetString( -1 ), sizeof( cl.classname ) );

		LUA->GetField( -3, "DataTableName" );
		V_strncpy( cl.datatablename, LUA->GetString( -1 ), sizeof( cl.datatablename ) );

		classes.AddToTail( cl );

		LUA->Pop( 4 );
	}

	return 0;
}

SETUP_NUMBER_LUA_ACCESSORS( SVC_ClassInfo, svc_ClassInfo, int32_t, NumServerClasses );

void SVC_ClassInfo::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_ClassInfo, CreateOnClient );
	REGISTER_LUA_ACCESSORS( SVC_ClassInfo, Classes );
	REGISTER_LUA_ACCESSORS( SVC_ClassInfo, NumServerClasses );
}

CNetMessage *SVC_SetPause::Create( )
{
	return new( std::nothrow ) SVC_SetPause;
}

SETUP_BOOL_LUA_ACCESSORS( SVC_SetPause, svc_SetPause, Paused );

void SVC_SetPause::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_SetPause, Paused );
}

SVC_CreateStringTable::SVC_CreateStringTable( ) :
	TableName( nullptr )
{ }

CNetMessage *SVC_CreateStringTable::Create( )
{
	return new( std::nothrow ) SVC_CreateStringTable;
}

SETUP_STRING_POINTER_LUA_ACCESSORS( SVC_CreateStringTable, svc_CreateStringTable, TableName );
SETUP_NUMBER_LUA_ACCESSORS( SVC_CreateStringTable, svc_CreateStringTable, int32_t, MaxEntries );
SETUP_NUMBER_LUA_ACCESSORS( SVC_CreateStringTable, svc_CreateStringTable, int32_t, NumEntries );
SETUP_BOOL_LUA_ACCESSORS( SVC_CreateStringTable, svc_CreateStringTable, UserDataFixedSize );
SETUP_NUMBER_LUA_ACCESSORS( SVC_CreateStringTable, svc_CreateStringTable, int32_t, UserDataSize );
SETUP_NUMBER_LUA_ACCESSORS( SVC_CreateStringTable, svc_CreateStringTable, int32_t, UserDataSizeBits );
SETUP_BOOL_LUA_ACCESSORS( SVC_CreateStringTable, svc_CreateStringTable, IsFilenames );
SETUP_NUMBER_LUA_ACCESSORS( SVC_CreateStringTable, svc_CreateStringTable, int32_t, Length );
SETUP_READER_LUA_ACCESSORS( SVC_CreateStringTable, svc_CreateStringTable, DataIn );
SETUP_WRITER_LUA_ACCESSORS( SVC_CreateStringTable, svc_CreateStringTable, DataOut );

void SVC_CreateStringTable::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_CreateStringTable, TableName );
	REGISTER_LUA_ACCESSORS( SVC_CreateStringTable, MaxEntries );
	REGISTER_LUA_ACCESSORS( SVC_CreateStringTable, NumEntries );
	REGISTER_LUA_ACCESSORS( SVC_CreateStringTable, UserDataFixedSize );
	REGISTER_LUA_ACCESSORS( SVC_CreateStringTable, UserDataSize );
	REGISTER_LUA_ACCESSORS( SVC_CreateStringTable, UserDataSizeBits );
	REGISTER_LUA_ACCESSORS( SVC_CreateStringTable, IsFilenames );
	REGISTER_LUA_ACCESSORS( SVC_CreateStringTable, Length );
	REGISTER_LUA_ACCESSORS( SVC_CreateStringTable, DataIn );
	REGISTER_LUA_ACCESSORS( SVC_CreateStringTable, DataOut );
}

CNetMessage *SVC_UpdateStringTable::Create( )
{
	return new( std::nothrow ) SVC_UpdateStringTable;
}

SETUP_NUMBER_LUA_ACCESSORS( SVC_UpdateStringTable, svc_UpdateStringTable, int32_t, TableID );
SETUP_NUMBER_LUA_ACCESSORS( SVC_UpdateStringTable, svc_UpdateStringTable, int32_t, ChangedEntries );
SETUP_NUMBER_LUA_ACCESSORS( SVC_UpdateStringTable, svc_UpdateStringTable, int32_t, Length );
SETUP_READER_LUA_ACCESSORS( SVC_UpdateStringTable, svc_UpdateStringTable, DataIn );
SETUP_WRITER_LUA_ACCESSORS( SVC_UpdateStringTable, svc_UpdateStringTable, DataOut );

void SVC_UpdateStringTable::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_UpdateStringTable, TableID );
	REGISTER_LUA_ACCESSORS( SVC_UpdateStringTable, ChangedEntries );
	REGISTER_LUA_ACCESSORS( SVC_UpdateStringTable, Length );
	REGISTER_LUA_ACCESSORS( SVC_UpdateStringTable, DataIn );
	REGISTER_LUA_ACCESSORS( SVC_UpdateStringTable, DataOut );
}

SVC_VoiceInit::SVC_VoiceInit( ) :
	VoiceCodec( nullptr )
{ }

CNetMessage *SVC_VoiceInit::Create( )
{
	return new( std::nothrow ) SVC_VoiceInit;
}

SETUP_STRING_POINTER_LUA_ACCESSORS( SVC_VoiceInit, svc_VoiceInit, VoiceCodec );
SETUP_NUMBER_LUA_ACCESSORS( SVC_VoiceInit, svc_VoiceInit, int32_t, Quality );

void SVC_VoiceInit::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_VoiceInit, VoiceCodec );
	REGISTER_LUA_ACCESSORS( SVC_VoiceInit, Quality );
}

SVC_VoiceData::SVC_VoiceData( ) :
	DataOut( nullptr )
{ }

CNetMessage *SVC_VoiceData::Create( )
{
	return new( std::nothrow ) SVC_VoiceData;
}

SETUP_NUMBER_LUA_ACCESSORS( SVC_VoiceData, svc_VoiceData, int32_t, Client );
SETUP_BOOL_LUA_ACCESSORS( SVC_VoiceData, svc_VoiceData, Proximity );
SETUP_NUMBER_LUA_ACCESSORS( SVC_VoiceData, svc_VoiceData, int32_t, Length );
SETUP_NUMBER_LUA_ACCESSORS( SVC_VoiceData, svc_VoiceData, uint64_t, XUID );
SETUP_READER_LUA_ACCESSORS( SVC_VoiceData, svc_VoiceData, DataIn );

LUA_FUNCTION_STATIC( SVC_VoiceData_GetDataOut )
{
	CHECK_NETMESSAGE_TYPE( SVC_VoiceData, svc_VoiceData );

	bf_write *writer = *sn4_bf_write::Push( state );

	writer->StartWriting( msg->DataOut, BitByte( msg->Length ), 0, msg->Length );

	return 1;
}

LUA_FUNCTION_STATIC( SVC_VoiceData_SetDataOut )
{
	CHECK_NETMESSAGE_TYPE( SVC_VoiceData, svc_VoiceData );
	bf_write *writer = sn4_bf_write::Get( state, 2 );

	msg->DataOut = writer->GetBasePointer( );
	msg->Length = writer->GetNumBitsWritten( );

	return 0;
}

void SVC_VoiceData::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_VoiceData, Client );
	REGISTER_LUA_ACCESSORS( SVC_VoiceData, Proximity );
	REGISTER_LUA_ACCESSORS( SVC_VoiceData, Length );
	REGISTER_LUA_ACCESSORS( SVC_VoiceData, XUID );
	REGISTER_LUA_ACCESSORS( SVC_VoiceData, DataIn );
	REGISTER_LUA_ACCESSORS( SVC_VoiceData, DataOut );
}

CNetMessage *SVC_Sounds::Create( )
{
	return new( std::nothrow ) SVC_Sounds;
}

SETUP_BOOL_LUA_ACCESSORS( SVC_Sounds, svc_Sounds, ReliableSound );
SETUP_NUMBER_LUA_ACCESSORS( SVC_Sounds, svc_Sounds, int32_t, NumSounds );
SETUP_NUMBER_LUA_ACCESSORS( SVC_Sounds, svc_Sounds, int32_t, Length );
SETUP_READER_LUA_ACCESSORS( SVC_Sounds, svc_Sounds, DataIn );
SETUP_WRITER_LUA_ACCESSORS( SVC_Sounds, svc_Sounds, DataOut );

void SVC_Sounds::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_Sounds, ReliableSound );
	REGISTER_LUA_ACCESSORS( SVC_Sounds, NumSounds );
	REGISTER_LUA_ACCESSORS( SVC_Sounds, Length );
	REGISTER_LUA_ACCESSORS( SVC_Sounds, DataIn );
	REGISTER_LUA_ACCESSORS( SVC_Sounds, DataOut );
}

CNetMessage *SVC_SetView::Create( )
{
	return new( std::nothrow ) SVC_SetView;
}

SETUP_NUMBER_LUA_ACCESSORS( SVC_SetView, svc_SetView, int32_t, EntityIndex );

void SVC_SetView::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_SetView, EntityIndex );
}

CNetMessage *SVC_FixAngle::Create( )
{
	return new( std::nothrow ) SVC_FixAngle;
}

SETUP_BOOL_LUA_ACCESSORS( SVC_FixAngle, svc_FixAngle, Relative );
SETUP_ANGLE_LUA_ACCESSORS( SVC_FixAngle, svc_FixAngle, Angle );

void SVC_FixAngle::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_FixAngle, Relative );
	REGISTER_LUA_ACCESSORS( SVC_FixAngle, Angle );
}

CNetMessage *SVC_CrosshairAngle::Create( )
{
	return new( std::nothrow ) SVC_CrosshairAngle;
}

SETUP_ANGLE_LUA_ACCESSORS( SVC_CrosshairAngle, svc_CrosshairAngle, Angle );

void SVC_CrosshairAngle::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_CrosshairAngle, Angle );
}

CNetMessage *SVC_BSPDecal::Create( )
{
	return new( std::nothrow ) SVC_BSPDecal;
}

SETUP_VECTOR_LUA_ACCESSORS( SVC_BSPDecal, svc_BSPDecal, Pos );
SETUP_NUMBER_LUA_ACCESSORS( SVC_BSPDecal, svc_BSPDecal, int32_t, DecalTextureIndex );
SETUP_NUMBER_LUA_ACCESSORS( SVC_BSPDecal, svc_BSPDecal, int32_t, EntityIndex );
SETUP_NUMBER_LUA_ACCESSORS( SVC_BSPDecal, svc_BSPDecal, int32_t, ModelIndex );
SETUP_BOOL_LUA_ACCESSORS( SVC_BSPDecal, svc_BSPDecal, LowPriority );

void SVC_BSPDecal::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_BSPDecal, Pos );
	REGISTER_LUA_ACCESSORS( SVC_BSPDecal, DecalTextureIndex );
	REGISTER_LUA_ACCESSORS( SVC_BSPDecal, EntityIndex );
	REGISTER_LUA_ACCESSORS( SVC_BSPDecal, ModelIndex );
	REGISTER_LUA_ACCESSORS( SVC_BSPDecal, LowPriority );
}

CNetMessage *SVC_UserMessage::Create( )
{
	return new( std::nothrow ) SVC_UserMessage;
}

SETUP_NUMBER_LUA_ACCESSORS( SVC_UserMessage, svc_UserMessage, int32_t, MsgType );
SETUP_NUMBER_LUA_ACCESSORS( SVC_UserMessage, svc_UserMessage, int32_t, Length );
SETUP_READER_LUA_ACCESSORS( SVC_UserMessage, svc_UserMessage, DataIn );
SETUP_WRITER_LUA_ACCESSORS( SVC_UserMessage, svc_UserMessage, DataOut );

void SVC_UserMessage::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_UserMessage, MsgType );
	REGISTER_LUA_ACCESSORS( SVC_UserMessage, Length );
	REGISTER_LUA_ACCESSORS( SVC_UserMessage, DataIn );
	REGISTER_LUA_ACCESSORS( SVC_UserMessage, DataOut );
}

CNetMessage *SVC_EntityMessage::Create( )
{
	return new( std::nothrow ) SVC_EntityMessage;
}

SETUP_NUMBER_LUA_ACCESSORS( SVC_EntityMessage, svc_EntityMessage, int32_t, EntityIndex );
SETUP_NUMBER_LUA_ACCESSORS( SVC_EntityMessage, svc_EntityMessage, int32_t, ClassID );
SETUP_NUMBER_LUA_ACCESSORS( SVC_EntityMessage, svc_EntityMessage, int32_t, Length );
SETUP_READER_LUA_ACCESSORS( SVC_EntityMessage, svc_EntityMessage, DataIn );
SETUP_WRITER_LUA_ACCESSORS( SVC_EntityMessage, svc_EntityMessage, DataOut );

void SVC_EntityMessage::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_EntityMessage, EntityIndex );
	REGISTER_LUA_ACCESSORS( SVC_EntityMessage, ClassID );
	REGISTER_LUA_ACCESSORS( SVC_EntityMessage, Length );
	REGISTER_LUA_ACCESSORS( SVC_EntityMessage, DataIn );
	REGISTER_LUA_ACCESSORS( SVC_EntityMessage, DataOut );
}

CNetMessage *SVC_GameEvent::Create( )
{
	return new( std::nothrow ) SVC_GameEvent;
}

SETUP_NUMBER_LUA_ACCESSORS( SVC_GameEvent, svc_GameEvent, int32_t, Length );
SETUP_READER_LUA_ACCESSORS( SVC_GameEvent, svc_GameEvent, DataIn );
SETUP_WRITER_LUA_ACCESSORS( SVC_GameEvent, svc_GameEvent, DataOut );

void SVC_GameEvent::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_GameEvent, Length );
	REGISTER_LUA_ACCESSORS( SVC_GameEvent, DataIn );
	REGISTER_LUA_ACCESSORS( SVC_GameEvent, DataOut );
}

CNetMessage *SVC_PacketEntities::Create( )
{
	return new( std::nothrow ) SVC_PacketEntities;
}

SETUP_NUMBER_LUA_ACCESSORS( SVC_PacketEntities, svc_PacketEntities, int32_t, MaxEntries );
SETUP_NUMBER_LUA_ACCESSORS( SVC_PacketEntities, svc_PacketEntities, int32_t, UpdatedEntries );
SETUP_BOOL_LUA_ACCESSORS( SVC_PacketEntities, svc_PacketEntities, Delta );
SETUP_BOOL_LUA_ACCESSORS( SVC_PacketEntities, svc_PacketEntities, UpdateBaseline );
SETUP_NUMBER_LUA_ACCESSORS( SVC_PacketEntities, svc_PacketEntities, int32_t, Baseline );
SETUP_NUMBER_LUA_ACCESSORS( SVC_PacketEntities, svc_PacketEntities, int32_t, DeltaFrom );
SETUP_NUMBER_LUA_ACCESSORS( SVC_PacketEntities, svc_PacketEntities, int32_t, Length );
SETUP_READER_LUA_ACCESSORS( SVC_PacketEntities, svc_PacketEntities, DataIn );
SETUP_WRITER_LUA_ACCESSORS( SVC_PacketEntities, svc_PacketEntities, DataOut );

void SVC_PacketEntities::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_PacketEntities, MaxEntries );
	REGISTER_LUA_ACCESSORS( SVC_PacketEntities, UpdatedEntries );
	REGISTER_LUA_ACCESSORS( SVC_PacketEntities, Delta );
	REGISTER_LUA_ACCESSORS( SVC_PacketEntities, UpdateBaseline );
	REGISTER_LUA_ACCESSORS( SVC_PacketEntities, Baseline );
	REGISTER_LUA_ACCESSORS( SVC_PacketEntities, DeltaFrom );
	REGISTER_LUA_ACCESSORS( SVC_PacketEntities, Length );
	REGISTER_LUA_ACCESSORS( SVC_PacketEntities, DataIn );
	REGISTER_LUA_ACCESSORS( SVC_PacketEntities, DataOut );
}

CNetMessage *SVC_TempEntities::Create( )
{
	return new( std::nothrow ) SVC_TempEntities;
}

SETUP_NUMBER_LUA_ACCESSORS( SVC_TempEntities, svc_TempEntities, int32_t, NumEntries );
SETUP_NUMBER_LUA_ACCESSORS( SVC_TempEntities, svc_TempEntities, int32_t, Length );
SETUP_READER_LUA_ACCESSORS( SVC_TempEntities, svc_TempEntities, DataIn );
SETUP_WRITER_LUA_ACCESSORS( SVC_TempEntities, svc_TempEntities, DataOut );

void SVC_TempEntities::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_TempEntities, NumEntries );
	REGISTER_LUA_ACCESSORS( SVC_TempEntities, Length );
	REGISTER_LUA_ACCESSORS( SVC_TempEntities, DataIn );
	REGISTER_LUA_ACCESSORS( SVC_TempEntities, DataOut );
}

CNetMessage *SVC_Prefetch::Create( )
{
	return new( std::nothrow ) SVC_Prefetch;
}

SETUP_NUMBER_LUA_ACCESSORS( SVC_Prefetch, svc_Prefetch, uint16_t, Type );
SETUP_NUMBER_LUA_ACCESSORS( SVC_Prefetch, svc_Prefetch, uint16_t, SoundIndex );

void SVC_Prefetch::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_Prefetch, Type );
	REGISTER_LUA_ACCESSORS( SVC_Prefetch, SoundIndex );
}

SVC_Menu::SVC_Menu( ) :
	MenuKeyValues( nullptr )
{ }

SVC_Menu::~SVC_Menu( )
{
	if( MenuKeyValues != nullptr )
		MenuKeyValues->deleteThis( );
}

CNetMessage *SVC_Menu::Create( )
{
	return new( std::nothrow ) SVC_Menu;
}

// KeyValues *MenuKeyValues;
SETUP_ENUM_LUA_ACCESSORS( SVC_Menu, svc_Menu, DIALOG_TYPE, Type );
SETUP_NUMBER_LUA_ACCESSORS( SVC_Menu, svc_Menu, int32_t, Length );

void SVC_Menu::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_Menu, Type );
	REGISTER_LUA_ACCESSORS( SVC_Menu, Length );
}

CNetMessage *SVC_GameEventList::Create( )
{
	return new( std::nothrow ) SVC_GameEventList;
}

SETUP_NUMBER_LUA_ACCESSORS( SVC_GameEventList, svc_GameEventList, int32_t, NumEvents );
SETUP_NUMBER_LUA_ACCESSORS( SVC_GameEventList, svc_GameEventList, int32_t, Length );
SETUP_READER_LUA_ACCESSORS( SVC_GameEventList, svc_GameEventList, DataIn );
SETUP_WRITER_LUA_ACCESSORS( SVC_GameEventList, svc_GameEventList, DataOut );

void SVC_GameEventList::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_GameEventList, NumEvents );
	REGISTER_LUA_ACCESSORS( SVC_GameEventList, Length );
	REGISTER_LUA_ACCESSORS( SVC_GameEventList, DataIn );
	REGISTER_LUA_ACCESSORS( SVC_GameEventList, DataOut );
}

SVC_GetCvarValue::SVC_GetCvarValue( ) :
	CvarName( nullptr )
{ }

CNetMessage *SVC_GetCvarValue::Create( )
{
	return new( std::nothrow ) SVC_GetCvarValue;
}

SETUP_NUMBER_LUA_ACCESSORS( SVC_GetCvarValue, svc_GetCvarValue, QueryCvarCookie_t, Cookie );
SETUP_STRING_POINTER_LUA_ACCESSORS( SVC_GetCvarValue, svc_GetCvarValue, CvarName );

void SVC_GetCvarValue::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_GetCvarValue, Cookie );
	REGISTER_LUA_ACCESSORS( SVC_GetCvarValue, CvarName );
}

SVC_CmdKeyValues::SVC_CmdKeyValues( ) :
	CmdKeyValues( nullptr )
{ }

SVC_CmdKeyValues::~SVC_CmdKeyValues( )
{
	if( CmdKeyValues != nullptr )
		CmdKeyValues->deleteThis( );
}

CNetMessage *SVC_CmdKeyValues::Create( )
{
	return new( std::nothrow ) SVC_CmdKeyValues;
}

// KeyValues *CmdKeyValues;

void SVC_CmdKeyValues::SetupLua( lua_State *state )
{

}

CNetMessage *SVC_GMod_ServerToClient::Create( )
{
	return new( std::nothrow ) SVC_GMod_ServerToClient;
}

SETUP_READER_LUA_ACCESSORS( SVC_GMod_ServerToClient, svc_GMod_ServerToClient, Data );

void SVC_GMod_ServerToClient::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( SVC_GMod_ServerToClient, Data );
}

CNetMessage *CLC_ClientInfo::Create( )
{
	return new( std::nothrow ) CLC_ClientInfo;
}

SETUP_NUMBER_LUA_ACCESSORS( CLC_ClientInfo, clc_ClientInfo, CRC32_t, SendTableCRC );
SETUP_NUMBER_LUA_ACCESSORS( CLC_ClientInfo, clc_ClientInfo, int32_t, ServerCount );
SETUP_BOOL_LUA_ACCESSORS( CLC_ClientInfo, clc_ClientInfo, HLTV );
SETUP_NUMBER_LUA_ACCESSORS( CLC_ClientInfo, clc_ClientInfo, uint32_t, FriendsID );
SETUP_STRING_LUA_ACCESSORS( CLC_ClientInfo, clc_ClientInfo, FriendsName );

LUA_FUNCTION_STATIC( CLC_ClientInfo_GetCustomFiles )
{
	CHECK_NETMESSAGE_TYPE( CLC_ClientInfo, clc_ClientInfo );

	LUA->CreateTable( );

	for( int32_t k = 0; k < MAX_CUSTOM_FILES; ++k )
	{
		LUA->PushNumber( k + 1 );
		LUA->PushNumber( msg->CustomFiles[k] );
		LUA->SetTable( -3 );
	}

	return 1;
}

LUA_FUNCTION_STATIC( CLC_ClientInfo_SetCustomFiles )
{
	CHECK_NETMESSAGE_TYPE( CLC_ClientInfo, clc_ClientInfo );
	LUA->CheckType( 2, GarrysMod::Lua::Type::TABLE );

	memset( msg->CustomFiles, 0, sizeof( msg->CustomFiles ) );

	for( size_t k = 0; k < MAX_CUSTOM_FILES; ++k )
	{
		LUA->PushNumber( k + 1 );
		LUA->GetTable( 2 );

		msg->CustomFiles[k] = static_cast<CRC32_t>( LUA->GetNumber( -1 ) );
	}

	return 0;
}

void CLC_ClientInfo::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( CLC_ClientInfo, SendTableCRC );
	REGISTER_LUA_ACCESSORS( CLC_ClientInfo, ServerCount );
	REGISTER_LUA_ACCESSORS( CLC_ClientInfo, HLTV );
	REGISTER_LUA_ACCESSORS( CLC_ClientInfo, FriendsID );
	REGISTER_LUA_ACCESSORS( CLC_ClientInfo, FriendsName );
	REGISTER_LUA_ACCESSORS( CLC_ClientInfo, CustomFiles );
}

CNetMessage *CLC_Move::Create( )
{
	return new( std::nothrow ) CLC_Move;
}

SETUP_NUMBER_LUA_ACCESSORS( CLC_Move, clc_Move, int32_t, BackupCommands );
SETUP_NUMBER_LUA_ACCESSORS( CLC_Move, clc_Move, int32_t, NewCommands );
SETUP_NUMBER_LUA_ACCESSORS( CLC_Move, clc_Move, int32_t, Length );
SETUP_READER_LUA_ACCESSORS( CLC_Move, clc_Move, DataIn );
SETUP_WRITER_LUA_ACCESSORS( CLC_Move, clc_Move, DataOut );

void CLC_Move::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( CLC_Move, BackupCommands );
	REGISTER_LUA_ACCESSORS( CLC_Move, NewCommands );
	REGISTER_LUA_ACCESSORS( CLC_Move, Length );
	REGISTER_LUA_ACCESSORS( CLC_Move, DataIn );
	REGISTER_LUA_ACCESSORS( CLC_Move, DataOut );
}

CNetMessage *CLC_VoiceData::Create( )
{
	return new( std::nothrow ) CLC_VoiceData;
}

SETUP_NUMBER_LUA_ACCESSORS( CLC_VoiceData, clc_VoiceData, int32_t, Length );
SETUP_READER_LUA_ACCESSORS( CLC_VoiceData, clc_VoiceData, DataIn );
SETUP_WRITER_LUA_ACCESSORS( CLC_VoiceData, clc_VoiceData, DataOut );
SETUP_NUMBER_LUA_ACCESSORS( CLC_VoiceData, clc_VoiceData, uint64_t, XUID );

void CLC_VoiceData::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( CLC_VoiceData, Length );
	REGISTER_LUA_ACCESSORS( CLC_VoiceData, DataIn );
	REGISTER_LUA_ACCESSORS( CLC_VoiceData, DataOut );
	REGISTER_LUA_ACCESSORS( CLC_VoiceData, XUID );
}

CNetMessage *CLC_BaselineAck::Create( )
{
	return new( std::nothrow ) CLC_BaselineAck;
}

SETUP_NUMBER_LUA_ACCESSORS( CLC_BaselineAck, clc_BaselineAck, int32_t, BaselineTick );
SETUP_NUMBER_LUA_ACCESSORS( CLC_BaselineAck, clc_BaselineAck, int32_t, BaselineNr );

void CLC_BaselineAck::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( CLC_BaselineAck, BaselineTick );
	REGISTER_LUA_ACCESSORS( CLC_BaselineAck, BaselineNr );
}

CNetMessage *CLC_ListenEvents::Create( )
{
	return new( std::nothrow ) CLC_ListenEvents;
}

// CBitVec<MAX_EVENT_NUMBER> EventArray;

void CLC_ListenEvents::SetupLua( lua_State *state )
{

}

CLC_RespondCvarValue::CLC_RespondCvarValue( ) :
	CvarName( nullptr ), CvarValue( nullptr )
{ }

CNetMessage *CLC_RespondCvarValue::Create( )
{
	return new( std::nothrow ) CLC_RespondCvarValue;
}

SETUP_NUMBER_LUA_ACCESSORS( CLC_RespondCvarValue, clc_RespondCvarValue, QueryCvarCookie_t, Cookie );
SETUP_STRING_POINTER_LUA_ACCESSORS( CLC_RespondCvarValue, clc_RespondCvarValue, CvarName );
SETUP_STRING_POINTER_LUA_ACCESSORS( CLC_RespondCvarValue, clc_RespondCvarValue, CvarValue );
SETUP_ENUM_LUA_ACCESSORS( CLC_RespondCvarValue, clc_RespondCvarValue, EQueryCvarValueStatus, StatusCode );

void CLC_RespondCvarValue::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( CLC_RespondCvarValue, Cookie );
	REGISTER_LUA_ACCESSORS( CLC_RespondCvarValue, CvarName );
	REGISTER_LUA_ACCESSORS( CLC_RespondCvarValue, CvarValue );
	REGISTER_LUA_ACCESSORS( CLC_RespondCvarValue, StatusCode );
}

CNetMessage *CLC_FileCRCCheck::Create( )
{
	return new( std::nothrow ) CLC_FileCRCCheck;
}

SETUP_STRING_LUA_ACCESSORS( CLC_FileCRCCheck, clc_FileCRCCheck, PathID );
SETUP_STRING_LUA_ACCESSORS( CLC_FileCRCCheck, clc_FileCRCCheck, Filename );
SETUP_NUMBER_LUA_ACCESSORS( CLC_FileCRCCheck, clc_FileCRCCheck, CRC32_t, CRC );

void CLC_FileCRCCheck::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( CLC_FileCRCCheck, PathID );
	REGISTER_LUA_ACCESSORS( CLC_FileCRCCheck, Filename );
	REGISTER_LUA_ACCESSORS( CLC_FileCRCCheck, CRC );
}

CLC_CmdKeyValues::CLC_CmdKeyValues( ) :
	CmdKeyValues( nullptr )
{ }

CLC_CmdKeyValues::~CLC_CmdKeyValues( )
{
	if( CmdKeyValues != nullptr )
		CmdKeyValues->deleteThis( );
}

CNetMessage *CLC_CmdKeyValues::Create( )
{
	return new( std::nothrow ) CLC_CmdKeyValues;
}

// KeyValues *CmdKeyValues;

void CLC_CmdKeyValues::SetupLua( lua_State *state )
{

}

CNetMessage *CLC_FileMD5Check::Create( )
{
	return new( std::nothrow ) CLC_FileMD5Check;
}

SETUP_STRING_LUA_ACCESSORS( CLC_FileMD5Check, clc_FileMD5Check, PathID );
SETUP_STRING_LUA_ACCESSORS( CLC_FileMD5Check, clc_FileMD5Check, Filename );
SETUP_ARRAY_LUA_ACCESSORS( CLC_FileMD5Check, clc_FileMD5Check, MD5 );

void CLC_FileMD5Check::SetupLua( lua_State *state )
{
	
	REGISTER_LUA_ACCESSORS( CLC_FileMD5Check, PathID );
	REGISTER_LUA_ACCESSORS( CLC_FileMD5Check, Filename );
	REGISTER_LUA_ACCESSORS( CLC_FileMD5Check, MD5 );
}

CNetMessage *CLC_GMod_ClientToServer::Create( )
{
	return new( std::nothrow ) CLC_GMod_ClientToServer;
}

SETUP_READER_LUA_ACCESSORS( CLC_GMod_ClientToServer, clc_GMod_ClientToServer, Data );

void CLC_GMod_ClientToServer::SetupLua( lua_State *state )
{
	REGISTER_LUA_ACCESSORS( CLC_GMod_ClientToServer, Data );
}

struct Functions
{
	CNetMessage *( *creator )( );
	void ( *luafuncs )( lua_State *state );
};

static std::unordered_map<std::string, Functions> netmessages_creators = {
	{ "net_Tick", { NET_Tick::Create, NET_Tick::SetupLua } },
	{ "net_StringCmd", { NET_StringCmd::Create, NET_StringCmd::SetupLua } },
	{ "net_SetConVar", { NET_SetConVar::Create, NET_SetConVar::SetupLua } },
	{ "net_SignonState", { NET_SignonState::Create, NET_SignonState::SetupLua } },
	{ "svc_Print", { SVC_Print::Create, SVC_Print::SetupLua } },
	{ "svc_ServerInfo", { SVC_ServerInfo::Create, SVC_ServerInfo::SetupLua } },
	{ "svc_SendTable", { SVC_SendTable::Create, SVC_SendTable::SetupLua } },
	{ "svc_ClassInfo", { SVC_ClassInfo::Create, SVC_ClassInfo::SetupLua } },
	{ "svc_SetPause", { SVC_SetPause::Create, SVC_SetPause::SetupLua } },
	{ "svc_CreateStringTable", { SVC_CreateStringTable::Create, SVC_CreateStringTable::SetupLua } },
	{ "svc_UpdateStringTable", { SVC_UpdateStringTable::Create, SVC_UpdateStringTable::SetupLua } },
	{ "svc_VoiceInit", { SVC_VoiceInit::Create, SVC_VoiceInit::SetupLua } },
	{ "svc_VoiceData", { SVC_VoiceData::Create, SVC_VoiceData::SetupLua } },
	{ "svc_Sounds", { SVC_Sounds::Create, SVC_Sounds::SetupLua } },
	{ "svc_SetView", { SVC_SetView::Create, SVC_SetView::SetupLua } },
	{ "svc_FixAngle", { SVC_FixAngle::Create, SVC_FixAngle::SetupLua } },
	{ "svc_CrosshairAngle", { SVC_CrosshairAngle::Create, SVC_CrosshairAngle::SetupLua } },
	{ "svc_BSPDecal", { SVC_BSPDecal::Create, SVC_BSPDecal::SetupLua } },
	{ "svc_UserMessage", { SVC_UserMessage::Create, SVC_UserMessage::SetupLua } },
	{ "svc_EntityMessage", { SVC_EntityMessage::Create, SVC_EntityMessage::SetupLua } },
	{ "svc_GameEvent", { SVC_GameEvent::Create, SVC_GameEvent::SetupLua } },
	{ "svc_PacketEntities", { SVC_PacketEntities::Create, SVC_PacketEntities::SetupLua } },
	{ "svc_TempEntities", { SVC_TempEntities::Create, SVC_TempEntities::SetupLua } },
	{ "svc_Prefetch", { SVC_Prefetch::Create, SVC_Prefetch::SetupLua } },
	{ "svc_Menu", { SVC_Menu::Create, SVC_Menu::SetupLua } },
	{ "svc_GameEventList", { SVC_GameEventList::Create, SVC_GameEventList::SetupLua } },
	{ "svc_GetCvarValue", { SVC_GetCvarValue::Create, SVC_GetCvarValue::SetupLua } },
	{ "svc_CmdKeyValues", { SVC_CmdKeyValues::Create, SVC_CmdKeyValues::SetupLua } },
	{ "svc_GMod_ServerToClient", { SVC_GMod_ServerToClient::Create, SVC_GMod_ServerToClient::SetupLua } },
	{ "clc_ClientInfo", { CLC_ClientInfo::Create, CLC_ClientInfo::SetupLua } },
	{ "clc_Move", { CLC_Move::Create, CLC_Move::SetupLua } },
	{ "clc_VoiceData", { CLC_VoiceData::Create, CLC_VoiceData::SetupLua } },
	{ "clc_BaselineAck", { CLC_BaselineAck::Create, CLC_BaselineAck::SetupLua } },
	{ "clc_ListenEvents", { CLC_ListenEvents::Create, CLC_ListenEvents::SetupLua } },
	{ "clc_RespondCvarValue", { CLC_RespondCvarValue::Create, CLC_RespondCvarValue::SetupLua } },
	{ "clc_FileCRCCheck", { CLC_FileCRCCheck::Create, CLC_FileCRCCheck::SetupLua } },
	{ "clc_CmdKeyValues", { CLC_CmdKeyValues::Create, CLC_CmdKeyValues::SetupLua } },
	{ "clc_FileMD5Check", { CLC_FileMD5Check::Create, CLC_FileMD5Check::SetupLua } },
	{ "clc_GMod_ClientToServer", { CLC_GMod_ClientToServer::Create, CLC_GMod_ClientToServer::SetupLua } }
};

CNetMessage *Create( const char *name )
{
	auto it = netmessages_creators.find( name );
	if( it != netmessages_creators.end( ) )
		return ( *it ).second.creator( );

	return nullptr;
}

void SetupLua( lua_State *state, const char *name )
{
	auto it = netmessages_creators.find( name );
	if( it != netmessages_creators.end( ) )
		( *it ).second.luafuncs( state );
}

}