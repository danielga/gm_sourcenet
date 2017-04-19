#include <vfnhook.h>
#include <netmessage.hpp>
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

struct Container
{
	INetMessage *msg;
	CNetChan *netchan;
};

static uint8_t metatype = GarrysMod::Lua::Type::NONE;
static const char *metaname = "INetMessage";
static const char *table_name = "sourcenet_INetMessage";

static std::unordered_map<std::string, void ( * )( GarrysMod::Lua::ILuaBase *LUA )> netmessages_setup;
static std::unordered_map<std::string, void **> netmessages_vtables;

static bool IsValid( INetMessage *msg, CNetChan *netchan )
{
	return msg != nullptr && NetChannel::IsValid( netchan );
}

void Push( GarrysMod::Lua::ILuaBase *LUA, INetMessage *msg, CNetChan *netchan )
{
	if( msg == nullptr )
	{
		LUA->PushNil( );
		return;
	}

	LUA->GetField( GarrysMod::Lua::INDEX_REGISTRY, table_name );
	LUA->PushUserdata( msg );
	LUA->GetTable( -2 );
	if( LUA->IsType( -1, metatype ) )
	{
		LUA->Remove( -2 );
		return;
	}

	LUA->Pop( 1 );

	Container *udata = LUA->NewUserType<Container>( metatype );
	udata->msg = msg;
	udata->netchan = netchan;

	LUA->PushMetaTable( metatype );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );

	auto it = netmessages_setup.find( msg->GetName( ) );
	if( it != netmessages_setup.end( ) )
		( *it ).second( LUA );

	lua_setfenv( LUA->state, -3 );

	if( netchan != nullptr )
	{
		LUA->PushUserdata( netchan );
		LUA->Push( -2 );
		LUA->SetTable( -4 );
		LUA->Remove( -2 );
	}
}

inline Container *GetUserData( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
{
	global::CheckType( LUA, index, metatype, metaname );
	return LUA->GetUserType<Container>( index, metatype );
}

INetMessage *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index, CNetChan **netchan )
{
	Container *udata = GetUserData( LUA, index );
	INetMessage *msg = udata->msg;
	CNetChan *netchannel = udata->netchan;
	if( !IsValid( msg, netchannel ) )
		global::ThrowError( LUA, "invalid %s", metaname );

	if( netchannel != nullptr )
		*netchan = netchannel;

	return msg;
}

void Destroy( GarrysMod::Lua::ILuaBase *LUA, CNetChan *netchan )
{
	LUA->GetField( GarrysMod::Lua::INDEX_REGISTRY, table_name );

	for( int32_t k = 0; k < netchan->m_NetMessages.Count( ); ++k )
	{
		LUA->PushUserdata( netchan->m_NetMessages[k] );
		LUA->PushNil( );
		LUA->SetTable( -2 );
	}

	LUA->Pop( 1 );
}

LUA_FUNCTION_STATIC( gc )
{
	Container *udata = GetUserData( LUA, 1 );
	INetMessage *netmsg = udata->msg;
	INetChannel *netchannel = udata->netchan;

	if( netchannel == nullptr )
		delete netmsg;

	LUA->SetUserType( 1, nullptr );

	return 0;
}

LUA_FUNCTION_STATIC( eq )
{
	LUA->PushBool( Get( LUA, 1 ) == Get( LUA, 2 ) );
	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	LUA->PushString( Get( LUA, 1 )->ToString( ) );
	return 1;
}

LUA_FUNCTION_STATIC( GetType )
{
	LUA->PushNumber( Get( LUA, 1 )->GetType( ) );
	return 1;
}

LUA_FUNCTION_STATIC( GetGroup )
{
	LUA->PushNumber( Get( LUA, 1 )->GetGroup( ) );
	return 1;
}

LUA_FUNCTION_STATIC( GetName )
{
	LUA->PushString( Get( LUA, 1 )->GetName( ) );
	return 1;
}

LUA_FUNCTION_STATIC( GetNetChannel )
{
	NetChannel::Push( LUA, static_cast<CNetChan *>( Get( LUA, 1 )->GetNetChannel( ) ) );
	return 1;
}

LUA_FUNCTION_STATIC( SetNetChannel )
{
	Get( LUA, 1 )->SetNetChannel( NetChannel::Get( LUA, 2 ) );
	return 0;
}

LUA_FUNCTION_STATIC( IsReliable )
{
	LUA->PushBool( Get( LUA, 1 )->IsReliable( ) );
	return 1;
}

LUA_FUNCTION_STATIC( SetReliable )
{
	INetMessage *msg = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	msg->SetReliable( LUA->GetBool( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( ReadFromBuffer )
{
	INetMessage *msg = Get( LUA, 1 );
	bf_read *reader = sn_bf_read::Get( LUA, 2 );

	LUA->PushBool( msg->ReadFromBuffer( *reader ) );

	return 1;
}

LUA_FUNCTION_STATIC( WriteToBuffer )
{
	INetMessage *msg = Get( LUA, 1 );
	bf_write *writer = sn_bf_write::Get( LUA, 2 );

	LUA->PushBool( msg->WriteToBuffer( *writer ) );

	return 1;
}

LUA_FUNCTION_STATIC( Process )
{
	LUA->PushBool( Get( LUA, 1 )->Process( ) );
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

static void ResolveMessagesFromFunctionCode( GarrysMod::Lua::ILuaBase *LUA, uint8_t *funcCode )
{
	CNetMessage *msg = new( std::nothrow ) CNetMessage;
	if( msg == nullptr )
		global::ThrowError( LUA, "failed to create CNetMessage object for netmessages resolution" );

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

void PreInitialize( GarrysMod::Lua::ILuaBase *LUA )
{
	SymbolFinder symfinder;

	uint8_t *CBaseClientState_ConnectionStart = static_cast<uint8_t *>( symfinder.ResolveOnBinary(
		global::engine_lib.c_str( ),
		CBaseClientState_ConnectionStart_sig,
		CBaseClientState_ConnectionStart_siglen
	) );
	if( CBaseClientState_ConnectionStart == nullptr )
		LUA->ThrowError( "failed to locate CBaseClientState::ConnectionStart" );

	ResolveMessagesFromFunctionCode( LUA, CBaseClientState_ConnectionStart );

	uint8_t *CBaseClient_ConnectionStart = static_cast<uint8_t *>( symfinder.ResolveOnBinary(
		global::engine_lib.c_str( ),
		CBaseClient_ConnectionStart_sig,
		CBaseClient_ConnectionStart_siglen
	) );
	if( CBaseClient_ConnectionStart == nullptr )
		LUA->ThrowError( "failed to locate CBaseClient::ConnectionStart" );

	ResolveMessagesFromFunctionCode( LUA, CBaseClient_ConnectionStart );

	uintptr_t SVC_CreateStringTable = reinterpret_cast<uintptr_t>(
		CBaseClientState_ConnectionStart
	) + SVC_CreateStringTable_offset;
	ResolveMessagesFromFunctionCode( LUA, reinterpret_cast<uint8_t *>(
		SVC_CreateStringTable + sizeof( uintptr_t ) +
			*reinterpret_cast<intptr_t *>( SVC_CreateStringTable )
	) );

	uintptr_t SVC_CmdKeyValues = reinterpret_cast<uintptr_t>(
		CBaseClientState_ConnectionStart
	) + SVC_CmdKeyValues_offset;
	ResolveMessagesFromFunctionCode( LUA, reinterpret_cast<uint8_t *>(
		SVC_CmdKeyValues + sizeof( uintptr_t ) + *reinterpret_cast<intptr_t *>( SVC_CmdKeyValues )
	) );

	uintptr_t CLC_CmdKeyValues = reinterpret_cast<uintptr_t>(
		CBaseClient_ConnectionStart
	) + CLC_CmdKeyValues_offset;
	ResolveMessagesFromFunctionCode( LUA, reinterpret_cast<uint8_t *>(
		CLC_CmdKeyValues + sizeof( uintptr_t ) + *reinterpret_cast<intptr_t *>( CLC_CmdKeyValues )
	) );
}

template<class NetMessage> int Constructor( lua_State *L )
{
	GarrysMod::Lua::ILuaBase *LUA = L->luabase;
	LUA->SetState( L );
	Push( LUA, new( std::nothrow ) NetMessage );
	return 1;
}

template<class NetMessage> void Register( GarrysMod::Lua::ILuaBase *LUA )
{
	NetMessage *msg = new( std::nothrow ) NetMessage;
	if( msg == nullptr )
		global::ThrowError( LUA, "failed to create object for '%s'", NetMessage::Name );

	if( netmessages_vtables.find( NetMessage::Name ) == netmessages_vtables.end( ) )
	{
		delete msg;
		global::ThrowError( LUA, "failed to find vtable for '%s'", NetMessage::Name );
	}

	BuildVTable( netmessages_vtables[NetMessage::Name], msg->GetVTable( ) );
	delete msg;

	LUA->PushCFunction( Constructor<NetMessage> );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, NetMessage::LuaName );

	netmessages_setup[NetMessage::Name] = NetMessage::SetupLua;
}

template<class NetMessage> void UnRegister( GarrysMod::Lua::ILuaBase *LUA )
{
	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, NetMessage::Name );
}

void Initialize( GarrysMod::Lua::ILuaBase *LUA )
{
	LUA->CreateTable( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, table_name );

	metatype = LUA->CreateMetaTable( metaname );

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

		LUA->PushNumber( EnterPVS );
		LUA->SetField( -2, "EnterPVS" );

		LUA->PushNumber( LeavePVS );
		LUA->SetField( -2, "LeavePVS" );

		LUA->PushNumber( DeltaEnt );
		LUA->SetField( -2, "DeltaEnt" );

		LUA->PushNumber( PreserveEnt );
		LUA->SetField( -2, "PreserveEnt" );

		LUA->PushNumber( Finished );
		LUA->SetField( -2, "Finished" );

		LUA->PushNumber( Failed );
		LUA->SetField( -2, "Failed" );

	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "UpdateType" );



	LUA->PushNumber( NET_MESSAGE_BITS );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "NET_MESSAGE_BITS" );



	LUA->PushNumber( MAX_CUSTOM_FILES );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "MAX_CUSTOM_FILES" );

	LUA->PushNumber( MAX_FRAGMENT_SIZE );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "MAX_FRAGMENT_SIZE" );

	LUA->PushNumber( MAX_FILE_SIZE );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "MAX_FILE_SIZE" );



	LUA->PushNumber( RES_FATALIFMISSING );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "RES_FATALIFMISSING" );

	LUA->PushNumber( RES_PRELOAD );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "RES_PRELOAD" );



	LUA->PushNumber( FHDR_ZERO );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "FHDR_ZERO" );

	LUA->PushNumber( FHDR_LEAVEPVS );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "FHDR_LEAVEPVS" );

	LUA->PushNumber( FHDR_DELETE );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "FHDR_DELETE" );

	LUA->PushNumber( FHDR_ENTERPVS );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "FHDR_ENTERPVS" );



	LUA->PushNumber( SIGNONSTATE_NONE );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SIGNONSTATE_NONE" );

	LUA->PushNumber( SIGNONSTATE_CHALLENGE );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SIGNONSTATE_CHALLENGE" );

	LUA->PushNumber( SIGNONSTATE_CONNECTED );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SIGNONSTATE_CONNECTED" );

	LUA->PushNumber( SIGNONSTATE_NEW );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SIGNONSTATE_NEW" );

	LUA->PushNumber( SIGNONSTATE_PRESPAWN );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SIGNONSTATE_PRESPAWN" );

	LUA->PushNumber( SIGNONSTATE_SPAWN );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SIGNONSTATE_SPAWN" );

	LUA->PushNumber( SIGNONSTATE_FULL );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SIGNONSTATE_FULL" );

	LUA->PushNumber( SIGNONSTATE_CHANGELEVEL );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SIGNONSTATE_CHANGELEVEL" );



	LUA->PushNumber( net_NOP );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "net_NOP" );

	LUA->PushNumber( net_Disconnect );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "net_Disconnect" );

	LUA->PushNumber( net_File );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "net_File" );

	LUA->PushNumber( net_LastControlMessage );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "net_LastControlMessage" );


	LUA->PushNumber( net_Tick );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "net_Tick" );

	LUA->PushNumber( net_StringCmd );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "net_StringCmd" );

	LUA->PushNumber( net_SetConVar );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "net_SetConVar" );

	LUA->PushNumber( net_SignonState );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "net_SignonState" );


	LUA->PushNumber( svc_ServerInfo );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_ServerInfo" );

	LUA->PushNumber( svc_SendTable );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_SendTable" );

	LUA->PushNumber( svc_ClassInfo );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_ClassInfo" );

	LUA->PushNumber( svc_SetPause );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_SetPause" );

	LUA->PushNumber( svc_CreateStringTable );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_CreateStringTable" );

	LUA->PushNumber( svc_UpdateStringTable );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_UpdateStringTable" );

	LUA->PushNumber( svc_VoiceInit );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_VoiceInit" );

	LUA->PushNumber( svc_VoiceData );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_VoiceData" );

	LUA->PushNumber( svc_Print );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_Print" );

	LUA->PushNumber( svc_Sounds );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_Sounds" );

	LUA->PushNumber( svc_SetView );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_SetView" );

	LUA->PushNumber( svc_FixAngle );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_FixAngle" );

	LUA->PushNumber( svc_CrosshairAngle );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_CrosshairAngle" );

	LUA->PushNumber( svc_BSPDecal );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_BSPDecal" );

	LUA->PushNumber( svc_UserMessage );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_UserMessage" );

	LUA->PushNumber( svc_EntityMessage );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_EntityMessage" );

	LUA->PushNumber( svc_GameEvent );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_GameEvent" );

	LUA->PushNumber( svc_PacketEntities );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_PacketEntities" );

	LUA->PushNumber( svc_TempEntities );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_TempEntities" );

	LUA->PushNumber( svc_Prefetch );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_Prefetch" );

	LUA->PushNumber( svc_Menu );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_Menu" );

	LUA->PushNumber( svc_GameEventList );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_GameEventList" );

	LUA->PushNumber( svc_GetCvarValue );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_GetCvarValue" );

	LUA->PushNumber( svc_CmdKeyValues );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_CmdKeyValues" );

	LUA->PushNumber( svc_GMod_ServerToClient );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_GMod_ServerToClient" );

	LUA->PushNumber( SVC_LASTMSG );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SVC_LASTMSG" );


	LUA->PushNumber( clc_ClientInfo );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_ClientInfo" );

	LUA->PushNumber( clc_Move );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_Move" );

	LUA->PushNumber( clc_VoiceData );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_VoiceData" );

	LUA->PushNumber( clc_BaselineAck );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_BaselineAck" );

	LUA->PushNumber( clc_ListenEvents );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_ListenEvents" );

	LUA->PushNumber( clc_RespondCvarValue );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_RespondCvarValue" );

	LUA->PushNumber( clc_FileCRCCheck );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_FileCRCCheck" );

	LUA->PushNumber( clc_CmdKeyValues );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_CmdKeyValues" );

	LUA->PushNumber( clc_FileMD5Check );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_FileMD5Check" );

	LUA->PushNumber( clc_GMod_ClientToServer );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_GMod_ClientToServer" );

	LUA->PushNumber( CLC_LASTMSG );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "CLC_LASTMSG" );



	Register<NET_Tick>( LUA );
	Register<NET_StringCmd>( LUA );
	Register<NET_SetConVar>( LUA );
	Register<NET_SignonState>( LUA );

	Register<SVC_Print>( LUA );
	Register<SVC_ServerInfo>( LUA );
	Register<SVC_SendTable>( LUA );
	Register<SVC_ClassInfo>( LUA );
	Register<SVC_SetPause>( LUA );
	Register<SVC_CreateStringTable>( LUA );
	Register<SVC_UpdateStringTable>( LUA );
	Register<SVC_VoiceInit>( LUA );
	Register<SVC_VoiceData>( LUA );
	Register<SVC_Sounds>( LUA );
	Register<SVC_SetView>( LUA );
	Register<SVC_FixAngle>( LUA );
	Register<SVC_CrosshairAngle>( LUA );
	Register<SVC_BSPDecal>( LUA );
	Register<SVC_UserMessage>( LUA );
	Register<SVC_EntityMessage>( LUA );
	Register<SVC_GameEvent>( LUA );
	Register<SVC_PacketEntities>( LUA );
	Register<SVC_TempEntities>( LUA );
	Register<SVC_Prefetch>( LUA );
	Register<SVC_Menu>( LUA );
	Register<SVC_GameEventList>( LUA );
	Register<SVC_GetCvarValue>( LUA );
	Register<SVC_CmdKeyValues>( LUA );
	Register<SVC_GMod_ServerToClient>( LUA );

	Register<CLC_ClientInfo>( LUA );
	Register<CLC_Move>( LUA );
	Register<CLC_VoiceData>( LUA );
	Register<CLC_BaselineAck>( LUA );
	Register<CLC_ListenEvents>( LUA );
	Register<CLC_RespondCvarValue>( LUA );
	Register<CLC_FileCRCCheck>( LUA );
	Register<CLC_CmdKeyValues>( LUA );
	Register<CLC_FileMD5Check>( LUA );
	Register<CLC_GMod_ClientToServer>( LUA );
}

void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
{
	UnRegister<NET_Tick>( LUA );
	UnRegister<NET_StringCmd>( LUA );
	UnRegister<NET_SetConVar>( LUA );
	UnRegister<NET_SignonState>( LUA );

	UnRegister<SVC_Print>( LUA );
	UnRegister<SVC_ServerInfo>( LUA );
	UnRegister<SVC_SendTable>( LUA );
	UnRegister<SVC_ClassInfo>( LUA );
	UnRegister<SVC_SetPause>( LUA );
	UnRegister<SVC_CreateStringTable>( LUA );
	UnRegister<SVC_UpdateStringTable>( LUA );
	UnRegister<SVC_VoiceInit>( LUA );
	UnRegister<SVC_VoiceData>( LUA );
	UnRegister<SVC_Sounds>( LUA );
	UnRegister<SVC_SetView>( LUA );
	UnRegister<SVC_FixAngle>( LUA );
	UnRegister<SVC_CrosshairAngle>( LUA );
	UnRegister<SVC_BSPDecal>( LUA );
	UnRegister<SVC_UserMessage>( LUA );
	UnRegister<SVC_EntityMessage>( LUA );
	UnRegister<SVC_GameEvent>( LUA );
	UnRegister<SVC_PacketEntities>( LUA );
	UnRegister<SVC_TempEntities>( LUA );
	UnRegister<SVC_Prefetch>( LUA );
	UnRegister<SVC_Menu>( LUA );
	UnRegister<SVC_GameEventList>( LUA );
	UnRegister<SVC_GetCvarValue>( LUA );
	UnRegister<SVC_CmdKeyValues>( LUA );
	UnRegister<SVC_GMod_ServerToClient>( LUA );

	UnRegister<CLC_ClientInfo>( LUA );
	UnRegister<CLC_Move>( LUA );
	UnRegister<CLC_VoiceData>( LUA );
	UnRegister<CLC_BaselineAck>( LUA );
	UnRegister<CLC_ListenEvents>( LUA );
	UnRegister<CLC_RespondCvarValue>( LUA );
	UnRegister<CLC_FileCRCCheck>( LUA );
	UnRegister<CLC_CmdKeyValues>( LUA );
	UnRegister<CLC_FileMD5Check>( LUA );
	UnRegister<CLC_GMod_ClientToServer>( LUA );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "UpdateType" );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "NET_MESSAGE_BITS" );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "MAX_CUSTOM_FILES" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "MAX_FRAGMENT_SIZE" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "MAX_FILE_SIZE" );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "RES_FATALIFMISSING" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "RES_PRELOAD" );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "FHDR_ZERO" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "FHDR_LEAVEPVS" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "FHDR_DELETE" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "FHDR_ENTERPVS" );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SIGNONSTATE_NONE" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SIGNONSTATE_CHALLENGE" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SIGNONSTATE_CONNECTED" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SIGNONSTATE_NEW" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SIGNONSTATE_PRESPAWN" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SIGNONSTATE_SPAWN" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SIGNONSTATE_FULL" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SIGNONSTATE_CHANGELEVEL" );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "net_NOP" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "net_Disconnect" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "net_File" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "net_LastControlMessage" );


	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "net_Tick" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "net_StringCmd" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "net_SetConVar" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "net_SignonState" );


	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_ServerInfo" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_SendTable" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_ClassInfo" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_SetPause" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_CreateStringTable" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_UpdateStringTable" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_VoiceInit" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_VoiceData" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_Print" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_Sounds" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_SetView" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_FixAngle" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_CrosshairAngle" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_BSPDecal" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_UserMessage" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_EntityMessage" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_GameEvent" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_PacketEntities" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_TempEntities" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_Prefetch" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_Menu" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_GameEventList" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_GetCvarValue" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_CmdKeyValues" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "svc_GMod_ServerToClient" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SVC_LASTMSG" );


	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_ClientInfo" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_Move" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_VoiceData" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_BaselineAck" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_ListenEvents" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_RespondCvarValue" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_FileCRCCheck" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_CmdKeyValues" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_FileMD5Check" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "clc_GMod_ClientToServer" );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "CLC_LASTMSG" );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, metaname );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, table_name );
}

}
