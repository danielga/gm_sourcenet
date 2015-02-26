#include <vfnhook.h>
#include <netmessage.hpp>
#include <netmessages.hpp>
#include <sn4_bf_write.hpp>
#include <sn4_bf_read.hpp>
#include <netchannel.hpp>
#include <hooks.hpp>
#include <net.hpp>
#include <inetmessage.h>
#include <symbolfinder.hpp>
#include <hde.h>

#define SETUP_LUA_CONSTRUCTOR( classname ) \
	LUA_FUNCTION_STATIC( classname ) \
	{ \
		NetMessages::CNetMessage *msg = NetMessages::classname::Create( ); \
		Push( state, msg ); \
		return 1; \
	}

#define REGISTER_LUA_CONSTRUCTOR( classname ) \
	LUA->PushCFunction( classname ); \
	LUA->SetField( -2, #classname );

#define UNREGISTER_LUA_CONSTRUCTOR( classname ) \
	LUA->PushNil( ); \
	LUA->SetField( -2, #classname );

namespace NetMessage
{

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

const uint8_t metaid = Global::metabase + 14;
const char *metaname = "INetMessage";

static std::unordered_map< CNetChan *, std::unordered_map<INetMessage *, int32_t> > netmessages;

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
				LUA->ReferencePush( ( *it2 ).second );
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
	lua_setfenv( state, -2 );

	NetMessages::SetupLua( state, msg->GetName( ) );

	LUA->Pop( 1 );

	if( netchan != nullptr )
	{
		LUA->Push( -1 );
		netmessages[netchan][msg] = LUA->ReferenceCreate( );
	}
}

INetMessage *Get( lua_State *state, int32_t index, CNetChan **netchan, bool cleanup )
{
	Global::CheckType( state, index, metaid, metaname );

	userdata *udata = static_cast<userdata *>( LUA->GetUserdata( index ) );
	INetMessage *msg = udata->msg;
	if( !IsValid( msg, udata->netchan ) && !cleanup )
		Global::ThrowError( state, "invalid %s", metaname );

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
	{
		auto &map = ( *it ).second;
		for( auto pair : map )
			LUA->ReferenceFree( pair.second );

		netmessages.erase( it );
	}
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
	bf_read *reader = sn4_bf_read::Get( state, 2 );

	LUA->PushBool( msg->ReadFromBuffer( *reader ) );

	return 1;
}

LUA_FUNCTION_STATIC( WriteToBuffer )
{
	INetMessage *msg = Get( state, 1 );
	bf_write *writer = sn4_bf_write::Get( state, 2 );

	LUA->PushBool( msg->WriteToBuffer( *writer ) );

	return 1;
}

LUA_FUNCTION_STATIC( Process )
{
	INetMessage *msg = Get( state, 1 );

	LUA->PushBool( msg->Process( ) );

	return 1;
}

SETUP_LUA_CONSTRUCTOR( NET_Tick );
SETUP_LUA_CONSTRUCTOR( NET_StringCmd );
SETUP_LUA_CONSTRUCTOR( NET_SetConVar );
SETUP_LUA_CONSTRUCTOR( NET_SignonState );

SETUP_LUA_CONSTRUCTOR( SVC_Print );
SETUP_LUA_CONSTRUCTOR( SVC_ServerInfo );
SETUP_LUA_CONSTRUCTOR( SVC_SendTable );
SETUP_LUA_CONSTRUCTOR( SVC_ClassInfo );
SETUP_LUA_CONSTRUCTOR( SVC_SetPause );
SETUP_LUA_CONSTRUCTOR( SVC_CreateStringTable );
SETUP_LUA_CONSTRUCTOR( SVC_UpdateStringTable );
SETUP_LUA_CONSTRUCTOR( SVC_VoiceInit );
SETUP_LUA_CONSTRUCTOR( SVC_VoiceData );
SETUP_LUA_CONSTRUCTOR( SVC_Sounds );
SETUP_LUA_CONSTRUCTOR( SVC_SetView );
SETUP_LUA_CONSTRUCTOR( SVC_FixAngle );
SETUP_LUA_CONSTRUCTOR( SVC_CrosshairAngle );
SETUP_LUA_CONSTRUCTOR( SVC_BSPDecal );
SETUP_LUA_CONSTRUCTOR( SVC_UserMessage );
SETUP_LUA_CONSTRUCTOR( SVC_EntityMessage );
SETUP_LUA_CONSTRUCTOR( SVC_GameEvent );
SETUP_LUA_CONSTRUCTOR( SVC_PacketEntities );
SETUP_LUA_CONSTRUCTOR( SVC_TempEntities );
SETUP_LUA_CONSTRUCTOR( SVC_Prefetch );
SETUP_LUA_CONSTRUCTOR( SVC_Menu );
SETUP_LUA_CONSTRUCTOR( SVC_GameEventList );
SETUP_LUA_CONSTRUCTOR( SVC_GetCvarValue );
SETUP_LUA_CONSTRUCTOR( SVC_CmdKeyValues );
SETUP_LUA_CONSTRUCTOR( SVC_GMod_ServerToClient );

SETUP_LUA_CONSTRUCTOR( CLC_ClientInfo );
SETUP_LUA_CONSTRUCTOR( CLC_Move );
SETUP_LUA_CONSTRUCTOR( CLC_VoiceData );
SETUP_LUA_CONSTRUCTOR( CLC_BaselineAck );
SETUP_LUA_CONSTRUCTOR( CLC_ListenEvents );
SETUP_LUA_CONSTRUCTOR( CLC_RespondCvarValue );
SETUP_LUA_CONSTRUCTOR( CLC_FileCRCCheck );
SETUP_LUA_CONSTRUCTOR( CLC_CmdKeyValues );
SETUP_LUA_CONSTRUCTOR( CLC_FileMD5Check );
SETUP_LUA_CONSTRUCTOR( CLC_GMod_ClientToServer );

inline void BuildVTable( void **source, void **destination )
{

#if defined _WIN32

	static size_t offset = 0;

#elif defined __linux || defined __APPLE__

	static size_t offset = 1;

#endif

	uintptr_t *src = reinterpret_cast<uintptr_t *>( source ), *dst = reinterpret_cast<uintptr_t *>( destination );

	Protection( dst, ( 12 + offset ) * sizeof( uintptr_t ), false );

	for( size_t k = 0; k < 12 + offset; ++k )
		dst[k] = src[k];

/*
	dst[3 + offset] = src[3 + offset]; // Process
	dst[4 + offset] = src[4 + offset]; // ReadFromBuffer
	dst[5 + offset] = src[5 + offset]; // WriteToBuffer

	dst[7 + offset] = src[7 + offset]; // GetType
	dst[8 + offset] = src[8 + offset]; // GetGroup
	dst[9 + offset] = src[9 + offset]; // GetName

	dst[11 + offset] = src[11 + offset]; // ToString
*/

	Protection( dst, ( 12 + offset ) * sizeof( uintptr_t ), true );
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
	NetMessages::CNetMessage *msg = new NetMessages::CNetMessage;
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

			NetMessages::CNetMessage *temp = NetMessages::Create( msg->GetName( ) );
			if( temp != nullptr )
			{
				BuildVTable( vtable, temp->GetVTable( ) );
				delete temp;
			}
		}

	msg->InstallVTable( msgvtable );
	delete msg;
}

void PreInitialize( lua_State *state )
{
	SymbolFinder symfinder;

	uint8_t *CBaseClientState_ConnectionStart = static_cast<uint8_t *>( symfinder.ResolveOnBinary(
		Global::engine_lib,
		CBaseClientState_ConnectionStart_sig,
		CBaseClientState_ConnectionStart_siglen
	) );
	if( CBaseClientState_ConnectionStart == nullptr )
		LUA->ThrowError( "failed to locate CBaseClientState::ConnectionStart" );

	ResolveMessagesFromFunctionCode( state, CBaseClientState_ConnectionStart );

	uint8_t *CBaseClient_ConnectionStart = static_cast<uint8_t *>( symfinder.ResolveOnBinary(
		Global::engine_lib,
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

void Initialize( lua_State *state )
{
	LUA->CreateMetaTableType( metaname, metaid );

		LUA->PushCFunction( gc );
		LUA->SetField( -2, "__gc" );

		LUA->PushCFunction( eq );
		LUA->SetField( -2, "__eq" );

		LUA->PushCFunction( tostring );
		LUA->SetField( -2, "__tostring" );

		LUA->PushCFunction( Global::index );
		LUA->SetField( -2, "__index" );

		LUA->PushCFunction( Global::newindex );
		LUA->SetField( -2, "__newindex" );

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

	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );

		REGISTER_LUA_CONSTRUCTOR( NET_Tick );
		REGISTER_LUA_CONSTRUCTOR( NET_StringCmd );
		REGISTER_LUA_CONSTRUCTOR( NET_SetConVar );
		REGISTER_LUA_CONSTRUCTOR( NET_SignonState );

		REGISTER_LUA_CONSTRUCTOR( SVC_Print );
		REGISTER_LUA_CONSTRUCTOR( SVC_ServerInfo );
		REGISTER_LUA_CONSTRUCTOR( SVC_SendTable );
		REGISTER_LUA_CONSTRUCTOR( SVC_ClassInfo );
		REGISTER_LUA_CONSTRUCTOR( SVC_SetPause );
		REGISTER_LUA_CONSTRUCTOR( SVC_CreateStringTable );
		REGISTER_LUA_CONSTRUCTOR( SVC_UpdateStringTable );
		REGISTER_LUA_CONSTRUCTOR( SVC_VoiceInit );
		REGISTER_LUA_CONSTRUCTOR( SVC_VoiceData );
		REGISTER_LUA_CONSTRUCTOR( SVC_Sounds );
		REGISTER_LUA_CONSTRUCTOR( SVC_SetView );
		REGISTER_LUA_CONSTRUCTOR( SVC_FixAngle );
		REGISTER_LUA_CONSTRUCTOR( SVC_CrosshairAngle );
		REGISTER_LUA_CONSTRUCTOR( SVC_BSPDecal );
		REGISTER_LUA_CONSTRUCTOR( SVC_UserMessage );
		REGISTER_LUA_CONSTRUCTOR( SVC_EntityMessage );
		REGISTER_LUA_CONSTRUCTOR( SVC_GameEvent );
		REGISTER_LUA_CONSTRUCTOR( SVC_PacketEntities );
		REGISTER_LUA_CONSTRUCTOR( SVC_TempEntities );
		REGISTER_LUA_CONSTRUCTOR( SVC_Prefetch );
		REGISTER_LUA_CONSTRUCTOR( SVC_Menu );
		REGISTER_LUA_CONSTRUCTOR( SVC_GameEventList );
		REGISTER_LUA_CONSTRUCTOR( SVC_GetCvarValue );
		REGISTER_LUA_CONSTRUCTOR( SVC_CmdKeyValues );
		REGISTER_LUA_CONSTRUCTOR( SVC_GMod_ServerToClient );

		REGISTER_LUA_CONSTRUCTOR( CLC_ClientInfo );
		REGISTER_LUA_CONSTRUCTOR( CLC_Move );
		REGISTER_LUA_CONSTRUCTOR( CLC_VoiceData );
		REGISTER_LUA_CONSTRUCTOR( CLC_BaselineAck );
		REGISTER_LUA_CONSTRUCTOR( CLC_ListenEvents );
		REGISTER_LUA_CONSTRUCTOR( CLC_RespondCvarValue );
		REGISTER_LUA_CONSTRUCTOR( CLC_FileCRCCheck );
		REGISTER_LUA_CONSTRUCTOR( CLC_CmdKeyValues );
		REGISTER_LUA_CONSTRUCTOR( CLC_FileMD5Check );
		REGISTER_LUA_CONSTRUCTOR( CLC_GMod_ClientToServer );

	LUA->Pop( 1 );
}

void Deinitialize( lua_State *state )
{
	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_REG );

		LUA->PushNil( );
		LUA->SetField( -2, metaname );

	LUA->Pop( 1 );

	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );

		UNREGISTER_LUA_CONSTRUCTOR( NET_Tick );
		UNREGISTER_LUA_CONSTRUCTOR( NET_StringCmd );
		UNREGISTER_LUA_CONSTRUCTOR( NET_SetConVar );
		UNREGISTER_LUA_CONSTRUCTOR( NET_SignonState );

		UNREGISTER_LUA_CONSTRUCTOR( SVC_Print );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_ServerInfo );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_SendTable );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_ClassInfo );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_SetPause );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_CreateStringTable );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_UpdateStringTable );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_VoiceInit );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_VoiceData );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_Sounds );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_SetView );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_FixAngle );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_CrosshairAngle );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_BSPDecal );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_UserMessage );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_EntityMessage );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_GameEvent );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_PacketEntities );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_TempEntities );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_Prefetch );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_Menu );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_GameEventList );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_GetCvarValue );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_CmdKeyValues );
		UNREGISTER_LUA_CONSTRUCTOR( SVC_GMod_ServerToClient );

		UNREGISTER_LUA_CONSTRUCTOR( CLC_ClientInfo );
		UNREGISTER_LUA_CONSTRUCTOR( CLC_Move );
		UNREGISTER_LUA_CONSTRUCTOR( CLC_VoiceData );
		UNREGISTER_LUA_CONSTRUCTOR( CLC_BaselineAck );
		UNREGISTER_LUA_CONSTRUCTOR( CLC_ListenEvents );
		UNREGISTER_LUA_CONSTRUCTOR( CLC_RespondCvarValue );
		UNREGISTER_LUA_CONSTRUCTOR( CLC_FileCRCCheck );
		UNREGISTER_LUA_CONSTRUCTOR( CLC_CmdKeyValues );
		UNREGISTER_LUA_CONSTRUCTOR( CLC_FileMD5Check );
		UNREGISTER_LUA_CONSTRUCTOR( CLC_GMod_ClientToServer );

	LUA->Pop( 1 );

	for( auto pair : netmessages )
		for( auto pair2 : pair.second )
			LUA->ReferenceFree( pair2.second );
}

}