#include <vfnhook.h>
#include <netmessage.hpp>
#include <GarrysMod/Lua/LuaInterface.h>
#include <GarrysMod/Lua/AutoReference.h>
#include <netmessages.hpp>
#include <sn_bf_write.hpp>
#include <sn_bf_read.hpp>
#include <netchannel.hpp>
#include <net.hpp>
#include <inetmessage.h>
#include <symbolfinder.hpp>
#include <hde.h>
#include <unordered_map>

namespace NetMessage
{

using namespace NetMessages;

#if defined _WIN32

static const char *CBaseClientState_ConnectionStart_sig = "\x55\x8B\xEC\x51\x53\x56\x57\x8B\xF9\x6A\x1C";
static const size_t CBaseClientState_ConnectionStart_siglen = 11;

static const char *CBaseClient_ConnectionStart_sig = "\x55\x8B\xEC\x51\x53\x56\x57\x8B\xF1\x6A\x1C";
static const size_t CBaseClient_ConnectionStart_siglen = 11;

static const uintptr_t SVC_CreateStringTable_offset = 611;

static const uintptr_t SVC_CmdKeyValues_offset = 1812;

static const uintptr_t CLC_CmdKeyValues_offset = 858;

#elif defined __linux

#if defined SOURCENET_SERVER

static const char *CBaseClientState_ConnectionStart_sig = "@_ZN16CBaseClientState15ConnectionStartEP11INetChannel";
static const size_t CBaseClientState_ConnectionStart_siglen = 0;

static const char *CBaseClient_ConnectionStart_sig = "@_ZN11CBaseClient15ConnectionStartEP11INetChannel";
static const size_t CBaseClient_ConnectionStart_siglen = 0;

#elif defined SOURCENET_CLIENT

static const char *CBaseClientState_ConnectionStart_sig = "\x55\x89\xE5\x57\x56\x53\x83\xEC\x2C\x8B\x5D";
static const size_t CBaseClientState_ConnectionStart_siglen = 11;

static const char *CBaseClient_ConnectionStart_sig = "\x55\x89\xE5\x57\x56\x53\x83\xEC\x2C\x8B\x5D";
static const size_t CBaseClient_ConnectionStart_siglen = 11;

#endif

static const uintptr_t SVC_CreateStringTable_offset = 576;

static const uintptr_t SVC_CmdKeyValues_offset = 1731;

static const uintptr_t CLC_CmdKeyValues_offset = 752;

#elif defined __APPLE__

static const char *CBaseClientState_ConnectionStart_sig = "@__ZN16CBaseClientState15ConnectionStartEP11INetChannel";
static const size_t CBaseClientState_ConnectionStart_siglen = 0;

static const char *CBaseClient_ConnectionStart_sig = "@__ZN11CBaseClient15ConnectionStartEP11INetChannel";
static const size_t CBaseClient_ConnectionStart_siglen = 0;

static const uintptr_t SVC_CreateStringTable_offset = 778;

static const uintptr_t SVC_CmdKeyValues_offset = 2765;

static const uintptr_t CLC_CmdKeyValues_offset = 1258;

#endif

struct userdata
{
	INetMessage *msg;
	uint8_t type;
	CNetChan *netchan;
};

const uint8_t metaid = global::metabase + 14;
const char *metaname = "INetMessage";

static std::unordered_map<
	CNetChan *,
	std::unordered_map<INetMessage *, GarrysMod::Lua::AutoReference>
> netmessages;
static std::unordered_map<std::string, void ( * )( lua_State *state )> netmessages_setup;
static std::unordered_map<std::string, void **> netmessages_vtables;

static bool IsValid( INetMessage *msg, CNetChan *netchan )
{
	return msg != nullptr && ( netchan == nullptr || NetChannel::IsValid( netchan ) );
}

void Push( lua_State *state, INetMessage *msg, CNetChan *netchan )
{
	if( msg == nullptr )
	{
		LUA->PushNil( );
		return;
	}

	if( netchan != nullptr )
	{
		auto it = netmessages.find( netchan );
		if( it != netmessages.end( ) )
		{
			auto &map = ( *it ).second;
			auto it2 = map.find( msg );
			if( it2 != map.end( ) )
			{
				( *it2 ).second.Push( );
				return;
			}
		}
	}

	userdata *udata = static_cast<userdata *>( LUA->NewUserdata( sizeof( userdata ) ) );
	udata->type = metaid;
	udata->msg = msg;
	udata->netchan = netchan;

	LUA->CreateMetaTableType( metaname, metaid );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	LUA->Push( -1 );
	lua_setfenv( state, -3 );

	auto it = netmessages_setup.find( msg->GetName( ) );
	if( it != netmessages_setup.end( ) )
		( *it ).second( state );

	LUA->Pop( 1 );

	if( netchan != nullptr )
	{
		GarrysMod::Lua::AutoReference &ref = netmessages[netchan][msg];
		ref.Setup( LUA );
		ref.Create( -1 );
	}
}

INetMessage *Get( lua_State *state, int32_t index, CNetChan **netchan, bool cleanup )
{
	global::CheckType( state, index, metaid, metaname );

	userdata *udata = static_cast<userdata *>( LUA->GetUserdata( index ) );
	INetMessage *msg = udata->msg;
	if( !IsValid( msg, udata->netchan ) && !cleanup )
		global::ThrowError( state, "invalid %s", metaname );

	if( netchan != nullptr )
		*netchan = udata->netchan;

	if( cleanup )
	{
		udata->msg = nullptr;
		udata->netchan = nullptr;
	}

	return msg;
}

void Destroy( lua_State *state, CNetChan *netchan )
{
	auto it = netmessages.find( netchan );
	if( it != netmessages.end( ) )
		netmessages.erase( it );
}

LUA_FUNCTION_STATIC( gc )
{
	CNetChan *netchan = nullptr;
	INetMessage *msg = Get( state, 1, &netchan, true );

	if( netchan == nullptr )
		delete msg;

	return 0;
}

LUA_FUNCTION_STATIC( eq )
{
	INetMessage *msg1 = Get( state, 1 );
	INetMessage *msg2 = Get( state, 2 );

	LUA->PushBool( msg1 == msg2 );

	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	INetMessage *msg = Get( state, 1 );

	LUA->PushString( msg->ToString( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetType )
{
	INetMessage *msg = Get( state, 1 );

	LUA->PushNumber( msg->GetType( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetGroup )
{
	INetMessage *msg = Get( state, 1 );

	LUA->PushNumber( msg->GetGroup( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetName )
{
	INetMessage *msg = Get( state, 1 );

	LUA->PushString( msg->GetName( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetNetChannel )
{
	INetMessage *msg = Get( state, 1 );

	NetChannel::Push( state, static_cast<CNetChan *>( msg->GetNetChannel( ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( SetNetChannel )
{
	INetMessage *msg = Get( state, 1 );
	CNetChan *netchan = NetChannel::Get( state, 2 );

	msg->SetNetChannel( netchan );

	return 0;
}

LUA_FUNCTION_STATIC( IsReliable )
{
	INetMessage *msg = Get( state, 1 );

	LUA->PushBool( msg->IsReliable( ) );

	return 1;
}

LUA_FUNCTION_STATIC( SetReliable )
{
	INetMessage *msg = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	msg->SetReliable( LUA->GetBool( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( ReadFromBuffer )
{
	INetMessage *msg = Get( state, 1 );
	bf_read *reader = sn_bf_read::Get( state, 2 );

	LUA->PushBool( msg->ReadFromBuffer( *reader ) );

	return 1;
}

LUA_FUNCTION_STATIC( WriteToBuffer )
{
	INetMessage *msg = Get( state, 1 );
	bf_write *writer = sn_bf_write::Get( state, 2 );

	LUA->PushBool( msg->WriteToBuffer( *writer ) );

	return 1;
}

LUA_FUNCTION_STATIC( Process )
{
	INetMessage *msg = Get( state, 1 );

	LUA->PushBool( msg->Process( ) );

	return 1;
}

inline void BuildVTable( void **source, void **destination )
{

#if defined _WIN32

	static size_t offset = 0;

#elif defined __linux || defined __APPLE__

	static size_t offset = 1;

#endif

	uintptr_t *src = reinterpret_cast<uintptr_t *>( source ), *dst = reinterpret_cast<uintptr_t *>( destination );

	ProtectMemory( dst, ( 12 + offset ) * sizeof( uintptr_t ), false );

	/*for( size_t k = 0; k < 12 + offset; ++k )
		dst[k] = src[k];*/

	dst[3 + offset] = src[3 + offset]; // Process
	dst[4 + offset] = src[4 + offset]; // ReadFromBuffer
	dst[5 + offset] = src[5 + offset]; // WriteToBuffer

	dst[7 + offset] = src[7 + offset]; // GetType
	dst[8 + offset] = src[8 + offset]; // GetGroup
	dst[9 + offset] = src[9 + offset]; // GetName

	dst[11 + offset] = src[11 + offset]; // ToString

	ProtectMemory( dst, ( 12 + offset ) * sizeof( uintptr_t ), true );
}

inline bool IsEndOfFunction( const hde32s &hs )
{

#if defined _WIN32

	return hs.opcode == 0xC3 || hs.opcode == 0xC2;

#elif defined __linux

	return hs.opcode == 0xC9;

#elif defined __APPLE__

	return hs.opcode == 0xC3;

#endif

}

inline bool PossibleVTable( const hde32s &hs )
{
	if( hs.len != 6 )
		return false;

	uint8_t opcode = hs.opcode, modrm = hs.modrm;

#if defined _WIN32

	return opcode == 0xC7 && ( modrm == 0x00 || modrm == 0x06 || modrm == 0x07 );

#elif defined __linux

	return opcode == 0xC7 && ( modrm == 0x00 || modrm == 0x02 || modrm == 0x03 || modrm == 0x06 );

#elif defined __APPLE__

	if( opcode == 0x8D && ( modrm == 0x80 ) )
		return true;

	return opcode == 0x8B && ( modrm == 0x80 || modrm == 0x83 || modrm == 0x87 || modrm == 0x8A ||
		modrm == 0x8B || modrm == 0x8E || modrm == 0x8F || modrm == 0x97 );

#endif

}

static void ResolveMessagesFromFunctionCode( lua_State *state, uint8_t *funcCode )
{
	CNetMessage *msg = new CNetMessage;
	void **msgvtable = msg->GetVTable( );

	hde32s hs;
	for(
		uint32_t len = hde32_disasm( funcCode, &hs );
		!IsEndOfFunction( hs );
		funcCode += len, len = hde32_disasm( funcCode, &hs )
	)
		if( PossibleVTable( hs ) )
		{
			void **vtable = reinterpret_cast<void **>( hs.imm.imm32 );
			msg->InstallVTable( vtable );

			const char *name = msg->GetName( );
			if( netmessages_vtables.find( name ) == netmessages_vtables.end( ) )
				netmessages_vtables[name] = vtable;
		}

	msg->InstallVTable( msgvtable );
	delete msg;
}

void PreInitialize( lua_State *state )
{
	SymbolFinder symfinder;

	uint8_t *CBaseClientState_ConnectionStart = static_cast<uint8_t *>( symfinder.ResolveOnBinary(
		global::engine_lib.c_str( ),
		CBaseClientState_ConnectionStart_sig,
		CBaseClientState_ConnectionStart_siglen
	) );
	if( CBaseClientState_ConnectionStart == nullptr )
		LUA->ThrowError( "failed to locate CBaseClientState::ConnectionStart" );

	ResolveMessagesFromFunctionCode( state, CBaseClientState_ConnectionStart );

	uint8_t *CBaseClient_ConnectionStart = static_cast<uint8_t *>( symfinder.ResolveOnBinary(
		global::engine_lib.c_str( ),
		CBaseClient_ConnectionStart_sig,
		CBaseClient_ConnectionStart_siglen
	) );
	if( CBaseClient_ConnectionStart == nullptr )
		LUA->ThrowError( "failed to locate CBaseClient::ConnectionStart" );

	ResolveMessagesFromFunctionCode( state, CBaseClient_ConnectionStart );

	uintptr_t SVC_CreateStringTable = reinterpret_cast<uintptr_t>(
		CBaseClientState_ConnectionStart
	) + SVC_CreateStringTable_offset;
	ResolveMessagesFromFunctionCode( state, reinterpret_cast<uint8_t *>(
		SVC_CreateStringTable + sizeof( uintptr_t ) +
			*reinterpret_cast<intptr_t *>( SVC_CreateStringTable )
	) );

	uintptr_t SVC_CmdKeyValues = reinterpret_cast<uintptr_t>(
		CBaseClientState_ConnectionStart
	) + SVC_CmdKeyValues_offset;
	ResolveMessagesFromFunctionCode( state, reinterpret_cast<uint8_t *>(
		SVC_CmdKeyValues + sizeof( uintptr_t ) + *reinterpret_cast<intptr_t *>( SVC_CmdKeyValues )
	) );

	uintptr_t CLC_CmdKeyValues = reinterpret_cast<uintptr_t>(
		CBaseClient_ConnectionStart
	) + CLC_CmdKeyValues_offset;
	ResolveMessagesFromFunctionCode( state, reinterpret_cast<uint8_t *>(
		CLC_CmdKeyValues + sizeof( uintptr_t ) + *reinterpret_cast<intptr_t *>( CLC_CmdKeyValues )
	) );
}

template<class NetMessage> int Constructor( lua_State *state )
{
	Push( state, new( std::nothrow ) NetMessage );
	return 1;
}

template<class NetMessage> bool Register( lua_State *state )
{
	NetMessage *msg = new( std::nothrow ) NetMessage;
	if( msg == nullptr )
	{
		static_cast<GarrysMod::Lua::ILuaInterface *>( LUA )->Msg( "Failed to create NetMessage object for \"%s\"!\n", NetMessage::Name );
		return false;
	}

	BuildVTable( netmessages_vtables[NetMessage::Name], msg->GetVTable( ) );
	delete msg;

	LUA->PushCFunction( Constructor<NetMessage> );
	LUA->SetField( -2, NetMessage::LuaName );

	netmessages_setup[NetMessage::Name] = NetMessage::SetupLua;

	return true;
}

template<class NetMessage> void UnRegister( lua_State *state )
{
	LUA->PushNil( );
	LUA->SetField( -2, NetMessage::Name );
}

void Initialize( lua_State *state )
{
	LUA->CreateMetaTableType( metaname, metaid );

		LUA->PushCFunction( gc );
		LUA->SetField( -2, "__gc" );

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

		LUA->PushCFunction( GetType );
		LUA->SetField( -2, "GetType" );

		LUA->PushCFunction( GetGroup );
		LUA->SetField( -2, "GetGroup" );

		LUA->PushCFunction( GetName );
		LUA->SetField( -2, "GetName" );

		LUA->PushCFunction( GetNetChannel );
		LUA->SetField( -2, "GetNetChannel" );

		LUA->PushCFunction( SetNetChannel );
		LUA->SetField( -2, "SetNetChannel" );

		LUA->PushCFunction( IsReliable );
		LUA->SetField( -2, "IsReliable" );

		LUA->PushCFunction( SetReliable );
		LUA->SetField( -2, "SetReliable" );

		LUA->PushCFunction( ReadFromBuffer );
		LUA->SetField( -2, "ReadFromBuffer" );

		LUA->PushCFunction( WriteToBuffer );
		LUA->SetField( -2, "WriteToBuffer" );

		LUA->PushCFunction( Process );
		LUA->SetField( -2, "Process" );

	LUA->Pop( 1 );

	LUA->CreateTable( );

		LUA->PushNumber( UpdateType::EnterPVS );
		LUA->SetField( -2, "EnterPVS" );

		LUA->PushNumber( UpdateType::LeavePVS );
		LUA->SetField( -2, "LeavePVS" );

		LUA->PushNumber( UpdateType::DeltaEnt );
		LUA->SetField( -2, "DeltaEnt" );

		LUA->PushNumber( UpdateType::PreserveEnt );
		LUA->SetField( -2, "PreserveEnt" );

		LUA->PushNumber( UpdateType::Finished );
		LUA->SetField( -2, "Finished" );

		LUA->PushNumber( UpdateType::Failed );
		LUA->SetField( -2, "Failed" );

	LUA->SetField( -2, "UpdateType" );



	LUA->PushNumber( NET_MESSAGE_BITS );
	LUA->SetField( -2, "NET_MESSAGE_BITS" );



	LUA->PushNumber( MAX_CUSTOM_FILES );
	LUA->SetField( -2, "MAX_CUSTOM_FILES" );

	LUA->PushNumber( MAX_FRAGMENT_SIZE );
	LUA->SetField( -2, "MAX_FRAGMENT_SIZE" );

	LUA->PushNumber( MAX_FILE_SIZE );
	LUA->SetField( -2, "MAX_FILE_SIZE" );



	LUA->PushNumber( RES_FATALIFMISSING );
	LUA->SetField( -2, "RES_FATALIFMISSING" );

	LUA->PushNumber( RES_PRELOAD );
	LUA->SetField( -2, "RES_PRELOAD" );



	LUA->PushNumber( FHDR_ZERO );
	LUA->SetField( -2, "FHDR_ZERO" );

	LUA->PushNumber( FHDR_LEAVEPVS );
	LUA->SetField( -2, "FHDR_LEAVEPVS" );

	LUA->PushNumber( FHDR_DELETE );
	LUA->SetField( -2, "FHDR_DELETE" );

	LUA->PushNumber( FHDR_ENTERPVS );
	LUA->SetField( -2, "FHDR_ENTERPVS" );



	LUA->PushNumber( SIGNONSTATE_NONE );
	LUA->SetField( -2, "SIGNONSTATE_NONE" );

	LUA->PushNumber( SIGNONSTATE_CHALLENGE );
	LUA->SetField( -2, "SIGNONSTATE_CHALLENGE" );

	LUA->PushNumber( SIGNONSTATE_CONNECTED );
	LUA->SetField( -2, "SIGNONSTATE_CONNECTED" );

	LUA->PushNumber( SIGNONSTATE_NEW );
	LUA->SetField( -2, "SIGNONSTATE_NEW" );

	LUA->PushNumber( SIGNONSTATE_PRESPAWN );
	LUA->SetField( -2, "SIGNONSTATE_PRESPAWN" );

	LUA->PushNumber( SIGNONSTATE_SPAWN );
	LUA->SetField( -2, "SIGNONSTATE_SPAWN" );

	LUA->PushNumber( SIGNONSTATE_FULL );
	LUA->SetField( -2, "SIGNONSTATE_FULL" );

	LUA->PushNumber( SIGNONSTATE_CHANGELEVEL );
	LUA->SetField( -2, "SIGNONSTATE_CHANGELEVEL" );



	LUA->PushNumber( net_NOP );
	LUA->SetField( -2, "net_NOP" );

	LUA->PushNumber( net_Disconnect );
	LUA->SetField( -2, "net_Disconnect" );

	LUA->PushNumber( net_File );
	LUA->SetField( -2, "net_File" );

	LUA->PushNumber( net_LastControlMessage );
	LUA->SetField( -2, "net_LastControlMessage" );


	LUA->PushNumber( net_Tick );
	LUA->SetField( -2, "net_Tick" );

	LUA->PushNumber( net_StringCmd );
	LUA->SetField( -2, "net_StringCmd" );

	LUA->PushNumber( net_SetConVar );
	LUA->SetField( -2, "net_SetConVar" );

	LUA->PushNumber( net_SignonState );
	LUA->SetField( -2, "net_SignonState" );


	LUA->PushNumber( svc_ServerInfo );
	LUA->SetField( -2, "svc_ServerInfo" );

	LUA->PushNumber( svc_SendTable );
	LUA->SetField( -2, "svc_SendTable" );

	LUA->PushNumber( svc_ClassInfo );
	LUA->SetField( -2, "svc_ClassInfo" );

	LUA->PushNumber( svc_SetPause );
	LUA->SetField( -2, "svc_SetPause" );

	LUA->PushNumber( svc_CreateStringTable );
	LUA->SetField( -2, "svc_CreateStringTable" );

	LUA->PushNumber( svc_UpdateStringTable );
	LUA->SetField( -2, "svc_UpdateStringTable" );

	LUA->PushNumber( svc_VoiceInit );
	LUA->SetField( -2, "svc_VoiceInit" );

	LUA->PushNumber( svc_VoiceData );
	LUA->SetField( -2, "svc_VoiceData" );

	LUA->PushNumber( svc_Print );
	LUA->SetField( -2, "svc_Print" );

	LUA->PushNumber( svc_Sounds );
	LUA->SetField( -2, "svc_Sounds" );

	LUA->PushNumber( svc_SetView );
	LUA->SetField( -2, "svc_SetView" );

	LUA->PushNumber( svc_FixAngle );
	LUA->SetField( -2, "svc_FixAngle" );

	LUA->PushNumber( svc_CrosshairAngle );
	LUA->SetField( -2, "svc_CrosshairAngle" );

	LUA->PushNumber( svc_BSPDecal );
	LUA->SetField( -2, "svc_BSPDecal" );

	LUA->PushNumber( svc_UserMessage );
	LUA->SetField( -2, "svc_UserMessage" );

	LUA->PushNumber( svc_EntityMessage );
	LUA->SetField( -2, "svc_EntityMessage" );

	LUA->PushNumber( svc_GameEvent );
	LUA->SetField( -2, "svc_GameEvent" );

	LUA->PushNumber( svc_PacketEntities );
	LUA->SetField( -2, "svc_PacketEntities" );

	LUA->PushNumber( svc_TempEntities );
	LUA->SetField( -2, "svc_TempEntities" );

	LUA->PushNumber( svc_Prefetch );
	LUA->SetField( -2, "svc_Prefetch" );

	LUA->PushNumber( svc_Menu );
	LUA->SetField( -2, "svc_Menu" );

	LUA->PushNumber( svc_GameEventList );
	LUA->SetField( -2, "svc_GameEventList" );

	LUA->PushNumber( svc_GetCvarValue );
	LUA->SetField( -2, "svc_GetCvarValue" );

	LUA->PushNumber( svc_CmdKeyValues );
	LUA->SetField( -2, "svc_CmdKeyValues" );

	LUA->PushNumber( svc_GMod_ServerToClient );
	LUA->SetField( -2, "svc_GMod_ServerToClient" );

	LUA->PushNumber( SVC_LASTMSG );
	LUA->SetField( -2, "SVC_LASTMSG" );


	LUA->PushNumber( clc_ClientInfo );
	LUA->SetField( -2, "clc_ClientInfo" );

	LUA->PushNumber( clc_Move );
	LUA->SetField( -2, "clc_Move" );

	LUA->PushNumber( clc_VoiceData );
	LUA->SetField( -2, "clc_VoiceData" );

	LUA->PushNumber( clc_BaselineAck );
	LUA->SetField( -2, "clc_BaselineAck" );

	LUA->PushNumber( clc_ListenEvents );
	LUA->SetField( -2, "clc_ListenEvents" );

	LUA->PushNumber( clc_RespondCvarValue );
	LUA->SetField( -2, "clc_RespondCvarValue" );

	LUA->PushNumber( clc_FileCRCCheck );
	LUA->SetField( -2, "clc_FileCRCCheck" );

	LUA->PushNumber( clc_CmdKeyValues );
	LUA->SetField( -2, "clc_CmdKeyValues" );

	LUA->PushNumber( clc_FileMD5Check );
	LUA->SetField( -2, "clc_FileMD5Check" );

	LUA->PushNumber( clc_GMod_ClientToServer );
	LUA->SetField( -2, "clc_GMod_ClientToServer" );

	LUA->PushNumber( CLC_LASTMSG );
	LUA->SetField( -2, "CLC_LASTMSG" );



	Register<NET_Tick>( state );
	Register<NET_StringCmd>( state );
	Register<NET_SetConVar>( state );
	Register<NET_SignonState>( state );

	Register<SVC_Print>( state );
	Register<SVC_ServerInfo>( state );
	Register<SVC_SendTable>( state );
	Register<SVC_ClassInfo>( state );
	Register<SVC_SetPause>( state );
	Register<SVC_CreateStringTable>( state );
	Register<SVC_UpdateStringTable>( state );
	Register<SVC_VoiceInit>( state );
	Register<SVC_VoiceData>( state );
	Register<SVC_Sounds>( state );
	Register<SVC_SetView>( state );
	Register<SVC_FixAngle>( state );
	Register<SVC_CrosshairAngle>( state );
	Register<SVC_BSPDecal>( state );
	Register<SVC_UserMessage>( state );
	Register<SVC_EntityMessage>( state );
	Register<SVC_GameEvent>( state );
	Register<SVC_PacketEntities>( state );
	Register<SVC_TempEntities>( state );
	Register<SVC_Prefetch>( state );
	Register<SVC_Menu>( state );
	Register<SVC_GameEventList>( state );
	Register<SVC_GetCvarValue>( state );
	Register<SVC_CmdKeyValues>( state );
	Register<SVC_GMod_ServerToClient>( state );

	Register<CLC_ClientInfo>( state );
	Register<CLC_Move>( state );
	Register<CLC_VoiceData>( state );
	Register<CLC_BaselineAck>( state );
	Register<CLC_ListenEvents>( state );
	Register<CLC_RespondCvarValue>( state );
	Register<CLC_FileCRCCheck>( state );
	Register<CLC_CmdKeyValues>( state );
	Register<CLC_FileMD5Check>( state );
	Register<CLC_GMod_ClientToServer>( state );
}

void Deinitialize( lua_State *state )
{
	LUA->PushNil( );
	LUA->SetField( -3, metaname );



	LUA->PushNil( );
	LUA->SetField( -2, "UpdateType" );



	LUA->PushNil( );
	LUA->SetField( -2, "NET_MESSAGE_BITS" );



	LUA->PushNil( );
	LUA->SetField( -2, "MAX_CUSTOM_FILES" );

	LUA->PushNil( );
	LUA->SetField( -2, "MAX_FRAGMENT_SIZE" );

	LUA->PushNil( );
	LUA->SetField( -2, "MAX_FILE_SIZE" );



	LUA->PushNil( );
	LUA->SetField( -2, "RES_FATALIFMISSING" );

	LUA->PushNil( );
	LUA->SetField( -2, "RES_PRELOAD" );



	LUA->PushNil( );
	LUA->SetField( -2, "FHDR_ZERO" );

	LUA->PushNil( );
	LUA->SetField( -2, "FHDR_LEAVEPVS" );

	LUA->PushNil( );
	LUA->SetField( -2, "FHDR_DELETE" );

	LUA->PushNil( );
	LUA->SetField( -2, "FHDR_ENTERPVS" );



	LUA->PushNil( );
	LUA->SetField( -2, "SIGNONSTATE_NONE" );

	LUA->PushNil( );
	LUA->SetField( -2, "SIGNONSTATE_CHALLENGE" );

	LUA->PushNil( );
	LUA->SetField( -2, "SIGNONSTATE_CONNECTED" );

	LUA->PushNil( );
	LUA->SetField( -2, "SIGNONSTATE_NEW" );

	LUA->PushNil( );
	LUA->SetField( -2, "SIGNONSTATE_PRESPAWN" );

	LUA->PushNil( );
	LUA->SetField( -2, "SIGNONSTATE_SPAWN" );

	LUA->PushNil( );
	LUA->SetField( -2, "SIGNONSTATE_FULL" );

	LUA->PushNil( );
	LUA->SetField( -2, "SIGNONSTATE_CHANGELEVEL" );



	LUA->PushNil( );
	LUA->SetField( -2, "net_NOP" );

	LUA->PushNil( );
	LUA->SetField( -2, "net_Disconnect" );

	LUA->PushNil( );
	LUA->SetField( -2, "net_File" );

	LUA->PushNil( );
	LUA->SetField( -2, "net_LastControlMessage" );


	LUA->PushNil( );
	LUA->SetField( -2, "net_Tick" );

	LUA->PushNil( );
	LUA->SetField( -2, "net_StringCmd" );

	LUA->PushNil( );
	LUA->SetField( -2, "net_SetConVar" );

	LUA->PushNil( );
	LUA->SetField( -2, "net_SignonState" );


	LUA->PushNil( );
	LUA->SetField( -2, "svc_ServerInfo" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_SendTable" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_ClassInfo" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_SetPause" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_CreateStringTable" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_UpdateStringTable" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_VoiceInit" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_VoiceData" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_Print" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_Sounds" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_SetView" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_FixAngle" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_CrosshairAngle" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_BSPDecal" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_UserMessage" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_EntityMessage" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_GameEvent" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_PacketEntities" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_TempEntities" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_Prefetch" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_Menu" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_GameEventList" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_GetCvarValue" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_CmdKeyValues" );

	LUA->PushNil( );
	LUA->SetField( -2, "svc_GMod_ServerToClient" );

	LUA->PushNil( );
	LUA->SetField( -2, "SVC_LASTMSG" );


	LUA->PushNil( );
	LUA->SetField( -2, "clc_ClientInfo" );

	LUA->PushNil( );
	LUA->SetField( -2, "clc_Move" );

	LUA->PushNil( );
	LUA->SetField( -2, "clc_VoiceData" );

	LUA->PushNil( );
	LUA->SetField( -2, "clc_BaselineAck" );

	LUA->PushNil( );
	LUA->SetField( -2, "clc_ListenEvents" );

	LUA->PushNil( );
	LUA->SetField( -2, "clc_RespondCvarValue" );

	LUA->PushNil( );
	LUA->SetField( -2, "clc_FileCRCCheck" );

	LUA->PushNil( );
	LUA->SetField( -2, "clc_CmdKeyValues" );

	LUA->PushNil( );
	LUA->SetField( -2, "clc_FileMD5Check" );

	LUA->PushNil( );
	LUA->SetField( -2, "clc_GMod_ClientToServer" );

	LUA->PushNil( );
	LUA->SetField( -2, "CLC_LASTMSG" );



	UnRegister<NET_Tick>( state );
	UnRegister<NET_StringCmd>( state );
	UnRegister<NET_SetConVar>( state );
	UnRegister<NET_SignonState>( state );

	UnRegister<SVC_Print>( state );
	UnRegister<SVC_ServerInfo>( state );
	UnRegister<SVC_SendTable>( state );
	UnRegister<SVC_ClassInfo>( state );
	UnRegister<SVC_SetPause>( state );
	UnRegister<SVC_CreateStringTable>( state );
	UnRegister<SVC_UpdateStringTable>( state );
	UnRegister<SVC_VoiceInit>( state );
	UnRegister<SVC_VoiceData>( state );
	UnRegister<SVC_Sounds>( state );
	UnRegister<SVC_SetView>( state );
	UnRegister<SVC_FixAngle>( state );
	UnRegister<SVC_CrosshairAngle>( state );
	UnRegister<SVC_BSPDecal>( state );
	UnRegister<SVC_UserMessage>( state );
	UnRegister<SVC_EntityMessage>( state );
	UnRegister<SVC_GameEvent>( state );
	UnRegister<SVC_PacketEntities>( state );
	UnRegister<SVC_TempEntities>( state );
	UnRegister<SVC_Prefetch>( state );
	UnRegister<SVC_Menu>( state );
	UnRegister<SVC_GameEventList>( state );
	UnRegister<SVC_GetCvarValue>( state );
	UnRegister<SVC_CmdKeyValues>( state );
	UnRegister<SVC_GMod_ServerToClient>( state );

	UnRegister<CLC_ClientInfo>( state );
	UnRegister<CLC_Move>( state );
	UnRegister<CLC_VoiceData>( state );
	UnRegister<CLC_BaselineAck>( state );
	UnRegister<CLC_ListenEvents>( state );
	UnRegister<CLC_RespondCvarValue>( state );
	UnRegister<CLC_FileCRCCheck>( state );
	UnRegister<CLC_CmdKeyValues>( state );
	UnRegister<CLC_FileMD5Check>( state );
	UnRegister<CLC_GMod_ClientToServer>( state );



	netmessages.clear( );
}

}