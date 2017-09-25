#include <netmessages.hpp>
#include <netmessage.hpp>
#include <sn_bf_write.hpp>
#include <sn_bf_read.hpp>
#include <inetchannelinfo.h>
#include <string>

namespace NetMessages
{

template<class ClassName> ClassName *CheckNetmessageType( GarrysMod::Lua::ILuaBase *LUA )
{
	ClassName *msg = static_cast<ClassName *>( NetMessage::Get( LUA, 1 ) );
	if( msg->GetType( ) != ClassName::Type )
		LUA->ThrowError( "tried to access netmessage member of wrong type" );

	return msg;
}

inline void SetupAccessors(
	GarrysMod::Lua::ILuaBase *LUA,
	const char *name,
	GarrysMod::Lua::CFunc Get,
	GarrysMod::Lua::CFunc Set
)
{
	std::string getter = "Get";
	getter += name;
	LUA->PushCFunction( Get );
	LUA->SetField( -2, getter.c_str( ) );

	std::string setter = "Set";
	setter += name;
	LUA->PushCFunction( Set );
	LUA->SetField( -2, setter.c_str( ) );
}

template<
	class ClassName,
	typename MemberType,
	MemberType ClassName::*M,
	typename FunctionType,
	void ( GarrysMod::Lua::ILuaBase::*Pusher )( FunctionType ),
	FunctionType ( GarrysMod::Lua::ILuaBase::*Getter )( int32_t )
>
struct Member
{
	LUA_FUNCTION_IMPLEMENT( Get )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		( LUA->*Pusher )( msg->*M );
		return 1;
	}

	LUA_FUNCTION_WRAP( Get );

	LUA_FUNCTION_IMPLEMENT( Set )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		msg->*M = ( LUA->*Getter )( 2 );
		return 0;
	}

	LUA_FUNCTION_WRAP( Set );

	Member( GarrysMod::Lua::ILuaBase *LUA, const char *name )
	{
		SetupAccessors( LUA, name, Get, Set );
	}
};

template<class ClassName, bool ClassName::*M>
struct BoolMember
{
	LUA_FUNCTION_IMPLEMENT( Get )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		LUA->PushBool( msg->*M );
		return 1;
	}

	LUA_FUNCTION_WRAP( Get );

	LUA_FUNCTION_IMPLEMENT( Set )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		msg->*M = LUA->GetBool( 2 );
		return 0;
	}

	LUA_FUNCTION_WRAP( Set );

	BoolMember( GarrysMod::Lua::ILuaBase *LUA, const char *name )
	{
		SetupAccessors( LUA, name, Get, Set );
	}
};

template<class ClassName, typename MemberType, MemberType ClassName::*M>
struct NumberMember
{
	LUA_FUNCTION_IMPLEMENT( Get )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		LUA->PushNumber( msg->*M );
		return 1;
	}

	LUA_FUNCTION_WRAP( Get );

	LUA_FUNCTION_IMPLEMENT( Set )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		msg->*M = LUA->GetNumber( 2 );
		return 0;
	}

	LUA_FUNCTION_WRAP( Set );

	NumberMember( GarrysMod::Lua::ILuaBase *LUA, const char *name )
	{
		SetupAccessors( LUA, name, Get, Set );
	}
};

template<class ClassName, typename MemberType, MemberType ClassName::*M>
struct EnumMember
{
	LUA_FUNCTION_IMPLEMENT( Get )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		LUA->PushNumber( msg->*M );
		return 1;
	}

	LUA_FUNCTION_WRAP( Get );

	LUA_FUNCTION_IMPLEMENT( Set )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		msg->*M = static_cast<MemberType>( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );
		return 0;
	}

	LUA_FUNCTION_WRAP( Set );

	EnumMember( GarrysMod::Lua::ILuaBase *LUA, const char *name )
	{
		SetupAccessors( LUA, name, Get, Set );
	}
};

template<class ClassName, QAngle ClassName::*M>
struct AngleMember
{
	LUA_FUNCTION_IMPLEMENT( Get )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		LUA->PushAngle( msg->*M );
		return 1;
	}

	LUA_FUNCTION_WRAP( Get );

	LUA_FUNCTION_IMPLEMENT( Set )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		msg->*M = LUA->GetAngle( 2 );
		return 0;
	}

	LUA_FUNCTION_WRAP( Set );

	AngleMember( GarrysMod::Lua::ILuaBase *LUA, const char *name )
	{
		SetupAccessors( LUA, name, Get, Set );
	}
};

template<class ClassName, Vector ClassName::*M>
struct VectorMember
{
	LUA_FUNCTION_IMPLEMENT( Get )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		LUA->PushVector( msg->*M );
		return 1;
	}

	LUA_FUNCTION_WRAP( Get );

	LUA_FUNCTION_IMPLEMENT( Set )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		msg->*M = LUA->GetVector( 2 );
		return 0;
	}

	LUA_FUNCTION_WRAP( Set );

	VectorMember( GarrysMod::Lua::ILuaBase *LUA, const char *name )
	{
		SetupAccessors( LUA, name, Get, Set );
	}
};

template<
	class ClassName,
	typename MemberType,
	size_t MaximumLength,
	MemberType ( ClassName::*M )[MaximumLength]
>
struct ArrayMember
{
	LUA_FUNCTION_IMPLEMENT( Get )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		LUA->PushString( reinterpret_cast<const char *>( msg->*M ), MaximumLength );
		return 1;
	}

	LUA_FUNCTION_WRAP( Get );

	LUA_FUNCTION_IMPLEMENT( Set )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );

		uint32_t len = 0;
		const char *data = LUA->GetString( 2, &len );
		memcpy(
			reinterpret_cast<void *>( msg->*M ), data, MaximumLength < len ? MaximumLength : len
		);

		if( len < MaximumLength )
			memset( &( msg->*M )[len], 0, sizeof( MemberType ) * ( MaximumLength - len ) );

		return 0;
	}

	LUA_FUNCTION_WRAP( Set );

	ArrayMember( GarrysMod::Lua::ILuaBase *LUA, const char *name )
	{
		SetupAccessors( LUA, name, Get, Set );
	}
};

template<class ClassName, size_t MaximumLength, char ( ClassName::*M )[MaximumLength]>
struct StringArrayMember
{
	LUA_FUNCTION_IMPLEMENT( Get )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		LUA->PushString( msg->*M );
		return 1;
	}

	LUA_FUNCTION_WRAP( Get );

	LUA_FUNCTION_IMPLEMENT( Set )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		V_strncpy( msg->*M, LUA->GetString( 2 ), MaximumLength );
		return 0;
	}

	LUA_FUNCTION_WRAP( Set );

	StringArrayMember( GarrysMod::Lua::ILuaBase *LUA, const char *name )
	{
		SetupAccessors( LUA, name, Get, Set );
	}
};

template<class ClassName, const char *ClassName::*M> struct StringMember
{
	LUA_FUNCTION_IMPLEMENT( Get )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		LUA->PushString( msg->*M );
		return 1;
	}

	LUA_FUNCTION_WRAP( Get );

	LUA_FUNCTION_IMPLEMENT( Set )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		msg->*M = LUA->GetString( 2 );
		return 0;
	}

	LUA_FUNCTION_WRAP( Set );

	StringMember( GarrysMod::Lua::ILuaBase *LUA, const char *name )
	{
		SetupAccessors( LUA, name, Get, Set );
	}
};

template<class ClassName, bf_read ClassName::*M>
struct ReaderMember
{
	LUA_FUNCTION_IMPLEMENT( Get )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		sn_bf_read::Push( LUA, &( msg->*M ), -1 );
		return 1;
	}

	LUA_FUNCTION_WRAP( Get );

	LUA_FUNCTION_IMPLEMENT( Set )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		msg->*M = *sn_bf_read::Get( LUA, 2, nullptr );
		return 0;
	}

	LUA_FUNCTION_WRAP( Set );

	ReaderMember( GarrysMod::Lua::ILuaBase *LUA, const char *name )
	{
		SetupAccessors( LUA, name, Get, Set );
	}
};

template<class ClassName, bf_write ClassName::*M>
struct WriterMember
{
	LUA_FUNCTION_IMPLEMENT( Get )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		sn_bf_write::Push( LUA, &( msg->*M ), -1 );
		return 1;
	}

	LUA_FUNCTION_WRAP( Get );

	LUA_FUNCTION_IMPLEMENT( Set )
	{
		ClassName *msg = CheckNetmessageType<ClassName>( LUA );
		msg->*M = *sn_bf_write::Get( LUA, 2, nullptr );
		return 0;
	}

	LUA_FUNCTION_WRAP( Set );

	WriterMember( GarrysMod::Lua::ILuaBase *LUA, const char *name )
	{
		SetupAccessors( LUA, name, Get, Set );
	}
};

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

const char NET_Tick::Name[] = "net_Tick";
const char NET_Tick::LuaName[] = "NET_Tick";
const int32_t NET_Tick::Type = net_Tick;

void NET_Tick::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<NET_Tick, int32_t, &NET_Tick::Tick>( LUA, "Tick" );
	NumberMember<NET_Tick, float, &NET_Tick::HostFrameTime>( LUA, "HostFrameTime" );
	NumberMember<NET_Tick, float, &NET_Tick::HostFrameTimeStdDeviation>(
		LUA, "HostFrameTimeStdDeviation"
	);
}

const char NET_StringCmd::Name[] = "net_StringCmd";
const char NET_StringCmd::LuaName[] = "NET_StringCmd";
const int32_t NET_StringCmd::Type = net_StringCmd;

NET_StringCmd::NET_StringCmd( ) :
	Command( nullptr )
{ }

void NET_StringCmd::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	StringMember<NET_StringCmd, &NET_StringCmd::Command>( LUA, "Command" );
}

const char NET_SetConVar::Name[] = "net_SetConVar";
const char NET_SetConVar::LuaName[] = "NET_SetConVar";
const int32_t NET_SetConVar::Type = net_SetConVar;

LUA_FUNCTION_STATIC( NET_SetConVar_GetConVars )
{
	NET_SetConVar *msg = CheckNetmessageType<NET_SetConVar>( LUA );

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
	NET_SetConVar *msg = CheckNetmessageType<NET_SetConVar>( LUA );
	LUA->CheckType( 2, GarrysMod::Lua::Type::TABLE );

	auto &convars = msg->ConVars;
	convars.RemoveAll( );

	NET_SetConVar::cvar_t cvar;

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

void NET_SetConVar::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	SetupAccessors( LUA, "ConVars", NET_SetConVar_GetConVars, NET_SetConVar_SetConVars );
}

const char NET_SignonState::Name[] = "net_SignonState";
const char NET_SignonState::LuaName[] = "NET_SignonState";
const int32_t NET_SignonState::Type = net_SignonState;

void NET_SignonState::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<NET_SignonState, int32_t, &NET_SignonState::SignonState>( LUA, "SignonState" );
	NumberMember<NET_SignonState, int32_t, &NET_SignonState::SpawnCount>( LUA, "SpawnCount" );
}

const char SVC_Print::Name[] = "svc_Print";
const char SVC_Print::LuaName[] = "SVC_Print";
const int32_t SVC_Print::Type = svc_Print;

SVC_Print::SVC_Print( ) :
	Text( nullptr )
{ }

void SVC_Print::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	StringMember<SVC_Print, &SVC_Print::Text>( LUA, "Text" );
}

const char SVC_ServerInfo::Name[] = "svc_ServerInfo";
const char SVC_ServerInfo::LuaName[] = "SVC_ServerInfo";
const int32_t SVC_ServerInfo::Type = svc_ServerInfo;

SVC_ServerInfo::SVC_ServerInfo( ) :
	GameDir( nullptr ), MapName( nullptr ), SkyName( nullptr ),
	HostName( nullptr ), LoadingURL( nullptr ), Gamemode( nullptr )
{ }

void SVC_ServerInfo::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<SVC_ServerInfo, int32_t, &SVC_ServerInfo::Protocol>( LUA, "Protocol" );
	NumberMember<SVC_ServerInfo, int32_t, &SVC_ServerInfo::ServerCount>( LUA, "ServerCount" );
	BoolMember<SVC_ServerInfo, &SVC_ServerInfo::Dedicated>( LUA, "Dedicated" );
	BoolMember<SVC_ServerInfo, &SVC_ServerInfo::HLTV>( LUA, "HLTV" );
	NumberMember<SVC_ServerInfo, char, &SVC_ServerInfo::OS>( LUA, "OS" );
	NumberMember<SVC_ServerInfo, CRC32_t, &SVC_ServerInfo::ClientCRC>( LUA, "ClientCRC" );
	ArrayMember<SVC_ServerInfo, uint8_t, 16, &SVC_ServerInfo::MapMD5>( LUA, "MapMD5" );
	NumberMember<SVC_ServerInfo, int32_t, &SVC_ServerInfo::MaxClients>( LUA, "MaxClients" );
	NumberMember<SVC_ServerInfo, int32_t, &SVC_ServerInfo::MaxClasses>( LUA, "MaxClasses" );
	NumberMember<SVC_ServerInfo, int32_t, &SVC_ServerInfo::PlayerSlot>( LUA, "PlayerSlot" );
	NumberMember<SVC_ServerInfo, float, &SVC_ServerInfo::TickInterval>( LUA, "TickInterval" );
	StringMember<SVC_ServerInfo, &SVC_ServerInfo::GameDir>( LUA, "GameDir" );
	StringMember<SVC_ServerInfo, &SVC_ServerInfo::MapName>( LUA, "MapName" );
	StringMember<SVC_ServerInfo, &SVC_ServerInfo::SkyName>( LUA, "SkyName" );
	StringMember<SVC_ServerInfo, &SVC_ServerInfo::HostName>( LUA, "HostName" );
	StringMember<SVC_ServerInfo, &SVC_ServerInfo::LoadingURL>( LUA, "LoadingURL" );
	StringMember<SVC_ServerInfo, &SVC_ServerInfo::Gamemode>( LUA, "Gamemode" );
}

const char SVC_SendTable::Name[] = "svc_SendTable";
const char SVC_SendTable::LuaName[] = "SVC_SendTable";
const int32_t SVC_SendTable::Type = svc_SendTable;

void SVC_SendTable::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	BoolMember<SVC_SendTable, &SVC_SendTable::NeedsDecoder>( LUA, "NeedsDecoder" );
	NumberMember<SVC_SendTable, int32_t, &SVC_SendTable::Length>( LUA, "Length" );
	ReaderMember<SVC_SendTable, &SVC_SendTable::DataIn>( LUA, "DataIn" );
	WriterMember<SVC_SendTable, &SVC_SendTable::DataOut>( LUA, "DataOut" );
}

const char SVC_ClassInfo::Name[] = "svc_ClassInfo";
const char SVC_ClassInfo::LuaName[] = "SVC_ClassInfo";
const int32_t SVC_ClassInfo::Type = svc_ClassInfo;

LUA_FUNCTION_STATIC( SVC_ClassInfo_GetClasses )
{
	SVC_ClassInfo *msg = CheckNetmessageType<SVC_ClassInfo>( LUA );

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
	SVC_ClassInfo *msg = CheckNetmessageType<SVC_ClassInfo>( LUA );
	LUA->CheckType( 2, GarrysMod::Lua::Type::TABLE );

	auto &classes = msg->Classes;
	classes.RemoveAll( );

	SVC_ClassInfo::class_t cl;

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

void SVC_ClassInfo::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	BoolMember<SVC_ClassInfo, &SVC_ClassInfo::CreateOnClient>( LUA, "CreateOnClient" );
	SetupAccessors( LUA, "Classes", SVC_ClassInfo_GetClasses, SVC_ClassInfo_SetClasses );
	NumberMember<SVC_ClassInfo, int32_t, &SVC_ClassInfo::NumServerClasses>(
		LUA, "NumServerClasses"
	);
}

const char SVC_SetPause::Name[] = "svc_SetPause";
const char SVC_SetPause::LuaName[] = "SVC_SetPause";
const int32_t SVC_SetPause::Type = svc_SetPause;

void SVC_SetPause::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	BoolMember<SVC_SetPause, &SVC_SetPause::Paused>( LUA, "Paused" );
}

const char SVC_CreateStringTable::Name[] = "svc_CreateStringTable";
const char SVC_CreateStringTable::LuaName[] = "SVC_CreateStringTable";
const int32_t SVC_CreateStringTable::Type = svc_CreateStringTable;

SVC_CreateStringTable::SVC_CreateStringTable( ) :
	TableName( nullptr )
{ }

void SVC_CreateStringTable::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	StringMember<SVC_CreateStringTable, &SVC_CreateStringTable::TableName>( LUA, "TableName" );
	NumberMember<SVC_CreateStringTable, int32_t, &SVC_CreateStringTable::MaxEntries>(
		LUA, "MaxEntries"
	);
	NumberMember<SVC_CreateStringTable, int32_t, &SVC_CreateStringTable::NumEntries>(
		LUA, "NumEntries"
	);
	BoolMember<SVC_CreateStringTable, &SVC_CreateStringTable::UserDataFixedSize>(
		LUA, "UserDataFixedSize"
	);
	NumberMember<SVC_CreateStringTable, int32_t, &SVC_CreateStringTable::UserDataSize>(
		LUA, "UserDataSize"
	);
	NumberMember<SVC_CreateStringTable, int32_t, &SVC_CreateStringTable::UserDataSizeBits>(
		LUA, "UserDataSizeBits"
	);
	BoolMember<SVC_CreateStringTable, &SVC_CreateStringTable::IsFilenames>( LUA, "IsFilenames" );
	NumberMember<SVC_CreateStringTable, int32_t, &SVC_CreateStringTable::Length>( LUA, "Length" );
	ReaderMember<SVC_CreateStringTable, &SVC_CreateStringTable::DataIn>( LUA, "DataIn" );
	WriterMember<SVC_CreateStringTable, &SVC_CreateStringTable::DataOut>( LUA, "DataOut" );
}

const char SVC_UpdateStringTable::Name[] = "svc_UpdateStringTable";
const char SVC_UpdateStringTable::LuaName[] = "SVC_UpdateStringTable";
const int32_t SVC_UpdateStringTable::Type = svc_UpdateStringTable;

void SVC_UpdateStringTable::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<SVC_UpdateStringTable, int32_t, &SVC_UpdateStringTable::TableID>(
		LUA, "TableID"
	);
	NumberMember<SVC_UpdateStringTable, int32_t, &SVC_UpdateStringTable::ChangedEntries>(
		LUA, "ChangedEntries"
	);
	NumberMember<SVC_UpdateStringTable, int32_t, &SVC_UpdateStringTable::Length>( LUA, "Length" );
	ReaderMember<SVC_UpdateStringTable, &SVC_UpdateStringTable::DataIn>( LUA, "DataIn" );
	WriterMember<SVC_UpdateStringTable, &SVC_UpdateStringTable::DataOut>( LUA, "DataOut" );
}

const char SVC_VoiceInit::Name[] = "svc_VoiceInit";
const char SVC_VoiceInit::LuaName[] = "SVC_VoiceInit";
const int32_t SVC_VoiceInit::Type = svc_VoiceInit;

SVC_VoiceInit::SVC_VoiceInit( ) :
	VoiceCodec( nullptr )
{ }

void SVC_VoiceInit::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	StringMember<SVC_VoiceInit, &SVC_VoiceInit::VoiceCodec>( LUA, "VoiceCodec" );
	NumberMember<SVC_VoiceInit, int32_t, &SVC_VoiceInit::Quality>( LUA, "Quality" );
}

const char SVC_VoiceData::Name[] = "svc_VoiceData";
const char SVC_VoiceData::LuaName[] = "SVC_VoiceData";
const int32_t SVC_VoiceData::Type = svc_VoiceData;

SVC_VoiceData::SVC_VoiceData( ) :
	DataOut( nullptr )
{ }

LUA_FUNCTION_STATIC( SVC_VoiceData_GetDataOut )
{
	SVC_VoiceData *msg = CheckNetmessageType<SVC_VoiceData>( LUA );

	bf_write *writer = *sn_bf_write::Push( LUA );

	writer->StartWriting( msg->DataOut, BitByte( msg->Length ), 0, msg->Length );

	return 1;
}

LUA_FUNCTION_STATIC( SVC_VoiceData_SetDataOut )
{
	SVC_VoiceData *msg = CheckNetmessageType<SVC_VoiceData>( LUA );
	bf_write *writer = sn_bf_write::Get( LUA, 2 );

	msg->DataOut = writer->GetBasePointer( );
	msg->Length = writer->GetNumBitsWritten( );

	return 0;
}

void SVC_VoiceData::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<SVC_VoiceData, int32_t, &SVC_VoiceData::Client>( LUA, "Client" );
	BoolMember<SVC_VoiceData, &SVC_VoiceData::Proximity>( LUA, "Proximity" );
	NumberMember<SVC_VoiceData, int32_t, &SVC_VoiceData::Length>( LUA, "Length" );
	NumberMember<SVC_VoiceData, uint64_t, &SVC_VoiceData::XUID>( LUA, "XUID" );
	ReaderMember<SVC_VoiceData, &SVC_VoiceData::DataIn>( LUA, "DataIn" );
	SetupAccessors( LUA, "DataOut", SVC_VoiceData_GetDataOut, SVC_VoiceData_SetDataOut );
}

const char SVC_Sounds::Name[] = "svc_Sounds";
const char SVC_Sounds::LuaName[] = "SVC_Sounds";
const int32_t SVC_Sounds::Type = svc_Sounds;

void SVC_Sounds::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	BoolMember<SVC_Sounds, &SVC_Sounds::ReliableSound>( LUA, "ReliableSound" );
	NumberMember<SVC_Sounds, int32_t, &SVC_Sounds::NumSounds>( LUA, "NumSounds" );
	NumberMember<SVC_Sounds, int32_t, &SVC_Sounds::Length>( LUA, "Length" );
	ReaderMember<SVC_Sounds, &SVC_Sounds::DataIn>( LUA, "DataIn" );
	WriterMember<SVC_Sounds, &SVC_Sounds::DataOut>( LUA, "DataOut" );
}

const char SVC_SetView::Name[] = "svc_SetView";
const char SVC_SetView::LuaName[] = "SVC_SetView";
const int32_t SVC_SetView::Type = svc_SetView;

void SVC_SetView::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<SVC_SetView, int32_t, &SVC_SetView::EntityIndex>( LUA, "EntityIndex" );
}

const char SVC_FixAngle::Name[] = "svc_FixAngle";
const char SVC_FixAngle::LuaName[] = "SVC_FixAngle";
const int32_t SVC_FixAngle::Type = svc_FixAngle;

void SVC_FixAngle::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	BoolMember<SVC_FixAngle, &SVC_FixAngle::Relative>( LUA, "Relative" );
	AngleMember<SVC_FixAngle, &SVC_FixAngle::Angle>( LUA, "Angle" );
}

const char SVC_CrosshairAngle::Name[] = "svc_CrosshairAngle";
const char SVC_CrosshairAngle::LuaName[] = "SVC_CrosshairAngle";
const int32_t SVC_CrosshairAngle::Type = svc_CrosshairAngle;

void SVC_CrosshairAngle::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	AngleMember<SVC_CrosshairAngle, &SVC_CrosshairAngle::Angle>( LUA, "Angle" );
}

const char SVC_BSPDecal::Name[] = "svc_BSPDecal";
const char SVC_BSPDecal::LuaName[] = "SVC_BSPDecal";
const int32_t SVC_BSPDecal::Type = svc_BSPDecal;

void SVC_BSPDecal::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	VectorMember<SVC_BSPDecal, &SVC_BSPDecal::Pos>( LUA, "Pos" );
	NumberMember<SVC_BSPDecal, int32_t, &SVC_BSPDecal::DecalTextureIndex>(
		LUA, "DecalTextureIndex"
	);
	NumberMember<SVC_BSPDecal, int32_t, &SVC_BSPDecal::EntityIndex>( LUA, "EntityIndex" );
	NumberMember<SVC_BSPDecal, int32_t, &SVC_BSPDecal::ModelIndex>( LUA, "ModelIndex" );
	BoolMember<SVC_BSPDecal, &SVC_BSPDecal::LowPriority>( LUA, "LowPriority" );
}

const char SVC_UserMessage::Name[] = "svc_UserMessage";
const char SVC_UserMessage::LuaName[] = "SVC_UserMessage";
const int32_t SVC_UserMessage::Type = svc_UserMessage;

void SVC_UserMessage::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<SVC_UserMessage, int32_t, &SVC_UserMessage::MsgType>( LUA, "MsgType" );
	NumberMember<SVC_UserMessage, int32_t, &SVC_UserMessage::Length>( LUA, "Length" );
	ReaderMember<SVC_UserMessage, &SVC_UserMessage::DataIn>( LUA, "DataIn" );
	WriterMember<SVC_UserMessage, &SVC_UserMessage::DataOut>( LUA, "DataOut" );
}

const char SVC_EntityMessage::Name[] = "svc_EntityMessage";
const char SVC_EntityMessage::LuaName[] = "SVC_EntityMessage";
const int32_t SVC_EntityMessage::Type = svc_EntityMessage;

void SVC_EntityMessage::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<SVC_EntityMessage, int32_t, &SVC_EntityMessage::EntityIndex>(
		LUA, "EntityIndex"
	);
	NumberMember<SVC_EntityMessage, int32_t, &SVC_EntityMessage::ClassID>( LUA, "ClassID" );
	NumberMember<SVC_EntityMessage, int32_t, &SVC_EntityMessage::Length>( LUA, "Length" );
	ReaderMember<SVC_EntityMessage, &SVC_EntityMessage::DataIn>( LUA, "DataIn" );
	WriterMember<SVC_EntityMessage, &SVC_EntityMessage::DataOut>( LUA, "DataOut" );
}

const char SVC_GameEvent::Name[] = "svc_GameEvent";
const char SVC_GameEvent::LuaName[] = "SVC_GameEvent";
const int32_t SVC_GameEvent::Type = svc_GameEvent;

void SVC_GameEvent::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<SVC_GameEvent, int32_t, &SVC_GameEvent::Length>( LUA, "Length" );
	ReaderMember<SVC_GameEvent, &SVC_GameEvent::DataIn>( LUA, "DataIn" );
	WriterMember<SVC_GameEvent, &SVC_GameEvent::DataOut>( LUA, "DataOut" );
}

const char SVC_PacketEntities::Name[] = "svc_PacketEntities";
const char SVC_PacketEntities::LuaName[] = "SVC_PacketEntities";
const int32_t SVC_PacketEntities::Type = svc_PacketEntities;

void SVC_PacketEntities::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<SVC_PacketEntities, int32_t, &SVC_PacketEntities::MaxEntries>(
		LUA, "MaxEntries"
	);
	NumberMember<SVC_PacketEntities, int32_t, &SVC_PacketEntities::UpdatedEntries>(
		LUA, "UpdatedEntries"
	);
	BoolMember<SVC_PacketEntities, &SVC_PacketEntities::Delta>( LUA, "Delta" );
	BoolMember<SVC_PacketEntities, &SVC_PacketEntities::UpdateBaseline>( LUA, "UpdateBaseline" );
	NumberMember<SVC_PacketEntities, int32_t, &SVC_PacketEntities::Baseline>( LUA, "Baseline" );
	NumberMember<SVC_PacketEntities, int32_t, &SVC_PacketEntities::DeltaFrom>( LUA, "DeltaFrom" );
	NumberMember<SVC_PacketEntities, int32_t, &SVC_PacketEntities::Length>( LUA, "Length" );
	ReaderMember<SVC_PacketEntities, &SVC_PacketEntities::DataIn>( LUA, "DataIn" );
	WriterMember<SVC_PacketEntities, &SVC_PacketEntities::DataOut>( LUA, "DataOut" );
}

const char SVC_TempEntities::Name[] = "svc_TempEntities";
const char SVC_TempEntities::LuaName[] = "SVC_TempEntities";
const int32_t SVC_TempEntities::Type = svc_TempEntities;

void SVC_TempEntities::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<SVC_TempEntities, int32_t, &SVC_TempEntities::NumEntries>( LUA, "NumEntries" );
	NumberMember<SVC_TempEntities, int32_t, &SVC_TempEntities::Length>( LUA, "Length" );
	ReaderMember<SVC_TempEntities, &SVC_TempEntities::DataIn>( LUA, "DataIn" );
	WriterMember<SVC_TempEntities, &SVC_TempEntities::DataOut>( LUA, "DataOut" );
}

const char SVC_Prefetch::Name[] = "svc_Prefetch";
const char SVC_Prefetch::LuaName[] = "SVC_Prefetch";
const int32_t SVC_Prefetch::Type = svc_Prefetch;

void SVC_Prefetch::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<SVC_Prefetch, uint16_t, &SVC_Prefetch::SoundType>( LUA, "SoundType" );
	NumberMember<SVC_Prefetch, uint16_t, &SVC_Prefetch::SoundIndex>( LUA, "SoundIndex" );
}

const char SVC_Menu::Name[] = "svc_Menu";
const char SVC_Menu::LuaName[] = "SVC_Menu";
const int32_t SVC_Menu::Type = svc_Menu;

SVC_Menu::SVC_Menu( ) :
	MenuKeyValues( nullptr )
{ }

SVC_Menu::~SVC_Menu( )
{
	if( MenuKeyValues != nullptr )
		MenuKeyValues->deleteThis( );
}

void SVC_Menu::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	EnumMember<SVC_Menu, DIALOG_TYPE, &SVC_Menu::DialogType>( LUA, "DialogType" );
	NumberMember<SVC_Menu, int32_t, &SVC_Menu::Length>( LUA, "Length" );
}

const char SVC_GameEventList::Name[] = "svc_GameEventList";
const char SVC_GameEventList::LuaName[] = "SVC_GameEventList";
const int32_t SVC_GameEventList::Type = svc_GameEventList;

void SVC_GameEventList::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<SVC_GameEventList, int32_t, &SVC_GameEventList::NumEvents>( LUA, "NumEvents" );
	NumberMember<SVC_GameEventList, int32_t, &SVC_GameEventList::Length>( LUA, "Length" );
	ReaderMember<SVC_GameEventList, &SVC_GameEventList::DataIn>( LUA, "DataIn" );
	WriterMember<SVC_GameEventList, &SVC_GameEventList::DataOut>( LUA, "DataOut" );
}

const char SVC_GetCvarValue::Name[] = "svc_GetCvarValue";
const char SVC_GetCvarValue::LuaName[] = "SVC_GetCvarValue";
const int32_t SVC_GetCvarValue::Type = svc_GetCvarValue;

SVC_GetCvarValue::SVC_GetCvarValue( ) :
	CvarName( nullptr )
{ }

void SVC_GetCvarValue::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<SVC_GetCvarValue, QueryCvarCookie_t, &SVC_GetCvarValue::Cookie>( LUA, "Cookie" );
	StringMember<SVC_GetCvarValue, &SVC_GetCvarValue::CvarName>( LUA, "CvarName" );
}

const char SVC_CmdKeyValues::Name[] = "svc_CmdKeyValues";
const char SVC_CmdKeyValues::LuaName[] = "SVC_CmdKeyValues";
const int32_t SVC_CmdKeyValues::Type = svc_CmdKeyValues;

SVC_CmdKeyValues::SVC_CmdKeyValues( ) :
	CmdKeyValues( nullptr )
{ }

SVC_CmdKeyValues::~SVC_CmdKeyValues( )
{
	if( CmdKeyValues != nullptr )
		CmdKeyValues->deleteThis( );
}

void SVC_CmdKeyValues::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{

}

const char SVC_GMod_ServerToClient::Name[] = "svc_GMod_ServerToClient";
const char SVC_GMod_ServerToClient::LuaName[] = "SVC_GMod_ServerToClient";
const int32_t SVC_GMod_ServerToClient::Type = svc_GMod_ServerToClient;

void SVC_GMod_ServerToClient::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	ReaderMember<SVC_GMod_ServerToClient, &SVC_GMod_ServerToClient::Data>( LUA, "Data" );
}

const char CLC_ClientInfo::Name[] = "clc_ClientInfo";
const char CLC_ClientInfo::LuaName[] = "CLC_ClientInfo";
const int32_t CLC_ClientInfo::Type = clc_ClientInfo;

LUA_FUNCTION_STATIC( CLC_ClientInfo_GetCustomFiles )
{
	CLC_ClientInfo *msg = CheckNetmessageType<CLC_ClientInfo>( LUA );

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
	CLC_ClientInfo *msg = CheckNetmessageType<CLC_ClientInfo>( LUA );
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

void CLC_ClientInfo::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<CLC_ClientInfo, CRC32_t, &CLC_ClientInfo::SendTableCRC>( LUA, "SendTableCRC" );
	NumberMember<CLC_ClientInfo, int32_t, &CLC_ClientInfo::ServerCount>( LUA, "ServerCount" );
	BoolMember<CLC_ClientInfo, &CLC_ClientInfo::HLTV>( LUA, "HLTV" );
	NumberMember<CLC_ClientInfo, uint32_t, &CLC_ClientInfo::FriendsID>( LUA, "FriendsID" );
	StringArrayMember<CLC_ClientInfo, 32, &CLC_ClientInfo::FriendsName>( LUA, "FriendsName" );
	SetupAccessors(
		LUA, "CustomFiles", CLC_ClientInfo_GetCustomFiles, CLC_ClientInfo_SetCustomFiles
	);
}

const char CLC_Move::Name[] = "clc_Move";
const char CLC_Move::LuaName[] = "CLC_Move";
const int32_t CLC_Move::Type = clc_Move;

void CLC_Move::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<CLC_Move, int32_t, &CLC_Move::BackupCommands>( LUA, "BackupCommands" );
	NumberMember<CLC_Move, int32_t, &CLC_Move::NewCommands>( LUA, "NewCommands" );
	NumberMember<CLC_Move, int32_t, &CLC_Move::Length>( LUA, "Length" );
	ReaderMember<CLC_Move, &CLC_Move::DataIn>( LUA, "DataIn" );
	WriterMember<CLC_Move, &CLC_Move::DataOut>( LUA, "DataOut" );
}

const char CLC_VoiceData::Name[] = "clc_VoiceData";
const char CLC_VoiceData::LuaName[] = "CLC_VoiceData";
const int32_t CLC_VoiceData::Type = clc_VoiceData;

void CLC_VoiceData::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<CLC_VoiceData, int32_t, &CLC_VoiceData::Length>( LUA, "Length" );
	ReaderMember<CLC_VoiceData, &CLC_VoiceData::DataIn>( LUA, "DataIn" );
	WriterMember<CLC_VoiceData, &CLC_VoiceData::DataOut>( LUA, "DataOut" );
	NumberMember<CLC_VoiceData, uint64_t, &CLC_VoiceData::XUID>( LUA, "XUID" );
}

const char CLC_BaselineAck::Name[] = "clc_BaselineAck";
const char CLC_BaselineAck::LuaName[] = "CLC_BaselineAck";
const int32_t CLC_BaselineAck::Type = clc_BaselineAck;

void CLC_BaselineAck::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<CLC_BaselineAck, int32_t, &CLC_BaselineAck::BaselineTick>( LUA, "BaselineTick" );
	NumberMember<CLC_BaselineAck, int32_t, &CLC_BaselineAck::BaselineNr>( LUA, "BaselineNr" );
}

const char CLC_ListenEvents::Name[] = "clc_ListenEvents";
const char CLC_ListenEvents::LuaName[] = "CLC_ListenEvents";
const int32_t CLC_ListenEvents::Type = clc_ListenEvents;

LUA_FUNCTION_STATIC( CLC_ListenEvents_GetEventArray )
{
	CLC_ListenEvents *msg = CheckNetmessageType<CLC_ListenEvents>( LUA );

	LUA->CreateTable( );

	size_t idx = 0;
	auto &events = msg->EventArray;
	for( int32_t k = 0; k < events.GetNumBits( ); ++k )
	{
		LUA->PushNumber( ++idx );
		LUA->PushBool( events.IsBitSet( k ) );
		LUA->SetTable( -3 );
	}

	return 1;
}

LUA_FUNCTION_STATIC( CLC_ListenEvents_SetEventArray )
{
	CLC_ListenEvents *msg = CheckNetmessageType<CLC_ListenEvents>( LUA );
	LUA->CheckType( 2, GarrysMod::Lua::Type::TABLE );

	size_t idx = 0;
	auto &events = msg->EventArray;
	for( int32_t k = 0; k < events.GetNumBits( ); ++k )
	{
		LUA->PushNumber( ++idx );
		LUA->GetTable( 2 );

		if( LUA->GetBool( -1 ) )
			events.Set( k );

		LUA->Pop( 1 );
	}

	return 0;
}

void CLC_ListenEvents::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	SetupAccessors( LUA, "EventArray", CLC_ListenEvents_GetEventArray, CLC_ListenEvents_SetEventArray );
}

const char CLC_RespondCvarValue::Name[] = "clc_RespondCvarValue";
const char CLC_RespondCvarValue::LuaName[] = "CLC_RespondCvarValue";
const int32_t CLC_RespondCvarValue::Type = clc_RespondCvarValue;

CLC_RespondCvarValue::CLC_RespondCvarValue( ) :
	CvarName( nullptr ), CvarValue( nullptr )
{ }

void CLC_RespondCvarValue::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	NumberMember<CLC_RespondCvarValue, QueryCvarCookie_t, &CLC_RespondCvarValue::Cookie>( LUA, "Cookie" );
	StringMember<CLC_RespondCvarValue, &CLC_RespondCvarValue::CvarName>( LUA, "CvarName" );
	StringMember<CLC_RespondCvarValue, &CLC_RespondCvarValue::CvarValue>( LUA, "CvarValue" );
	EnumMember<CLC_RespondCvarValue, EQueryCvarValueStatus, &CLC_RespondCvarValue::StatusCode>( LUA, "StatusCode" );
}

const char CLC_FileCRCCheck::Name[] = "clc_FileCRCCheck";
const char CLC_FileCRCCheck::LuaName[] = "CLC_FileCRCCheck";
const int32_t CLC_FileCRCCheck::Type = clc_FileCRCCheck;

void CLC_FileCRCCheck::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	StringArrayMember<CLC_FileCRCCheck, 260, &CLC_FileCRCCheck::PathID>( LUA, "PathID" );
	StringArrayMember<CLC_FileCRCCheck, 260, &CLC_FileCRCCheck::Filename>( LUA, "Filename" );
	NumberMember<CLC_FileCRCCheck, CRC32_t, &CLC_FileCRCCheck::CRC>( LUA, "CRC" );
}

const char CLC_CmdKeyValues::Name[] = "clc_CmdKeyValues";
const char CLC_CmdKeyValues::LuaName[] = "CLC_CmdKeyValues";
const int32_t CLC_CmdKeyValues::Type = clc_CmdKeyValues;

CLC_CmdKeyValues::CLC_CmdKeyValues( ) :
	CmdKeyValues( nullptr )
{ }

CLC_CmdKeyValues::~CLC_CmdKeyValues( )
{
	if( CmdKeyValues != nullptr )
		CmdKeyValues->deleteThis( );
}

void CLC_CmdKeyValues::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{

}

const char CLC_FileMD5Check::Name[] = "clc_FileMD5Check";
const char CLC_FileMD5Check::LuaName[] = "CLC_FileMD5Check";
const int32_t CLC_FileMD5Check::Type = clc_FileMD5Check;

void CLC_FileMD5Check::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	StringArrayMember<CLC_FileMD5Check, 260, &CLC_FileMD5Check::PathID>( LUA, "PathID" );
	StringArrayMember<CLC_FileMD5Check, 260, &CLC_FileMD5Check::Filename>( LUA, "Filename" );
	ArrayMember<CLC_FileMD5Check, uint8_t, 16, &CLC_FileMD5Check::MD5>( LUA, "MD5" );
}

const char CLC_GMod_ClientToServer::Name[] = "clc_GMod_ClientToServer";
const char CLC_GMod_ClientToServer::LuaName[] = "CLC_GMod_ClientToServer";
const int32_t CLC_GMod_ClientToServer::Type = clc_GMod_ClientToServer;

void CLC_GMod_ClientToServer::SetupLua( GarrysMod::Lua::ILuaBase *LUA )
{
	ReaderMember<CLC_GMod_ClientToServer, &CLC_GMod_ClientToServer::Data>( LUA, "Data" );
}

}
