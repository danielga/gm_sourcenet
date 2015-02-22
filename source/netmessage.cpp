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

static std::unordered_map<int32_t, void **> netmessages_vtables;
static std::unordered_map< CNetChan *, std::unordered_map<INetMessage *, int32_t> > netmessages;
static std::unordered_map<int32_t, int32_t> static_netmessages;

static bool IsValid( INetMessage *msg, CNetChan *netchan )
{
	return msg != nullptr && ( netchan == nullptr || NetChannel::IsValid( netchan ) );
}

void Push( lua_State *state, INetMessage *msg, CNetChan *netchan )
{
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
	lua_setfenv( state, -2 );

	LUA->Push( -1 );
	netmessages[netchan][msg] = LUA->ReferenceCreate( );
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

LUA_FUNCTION_STATIC( Clear )
{
	CNetChan *netchan = nullptr;
	CNetMessage *msg = static_cast<CNetMessage *>( Get( state, 1, &netchan ) );

	if( netchan == nullptr )
		msg->Destroy( );

	return 0;
}

LUA_FUNCTION_STATIC( Process )
{
	INetMessage *msg = Get( state, 1 );

	LUA->PushBool( msg->Process( ) );

	return 1;
}

LUA_FUNCTION_STATIC( Constructor )
{
	LUA->CheckType( 1, GarrysMod::Lua::Type::NUMBER );

	auto it = static_netmessages.find( static_cast<int32_t>( LUA->GetNumber( 1 ) ) );
	if( it == static_netmessages.end( ) )
		return 0;

	LUA->ReferencePush( ( *it ).second );

	return 1;
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

			int32_t type = msg->GetType( );
			if( netmessages_vtables.find( type ) == netmessages_vtables.end( ) )
				netmessages_vtables[type] = vtable;
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
		SVC_CreateStringTable + 4 + *reinterpret_cast<intptr_t *>( SVC_CreateStringTable )
	) );

	uintptr_t SVC_CmdKeyValues = reinterpret_cast<uintptr_t>(
		CBaseClientState_ConnectionStart
	) + SVC_CmdKeyValues_offset;
	ResolveMessagesFromFunctionCode( state, reinterpret_cast<uint8_t *>(
		SVC_CmdKeyValues + 4 + *reinterpret_cast<intptr_t *>( SVC_CmdKeyValues )
	) );

	uintptr_t CLC_CmdKeyValues = reinterpret_cast<uintptr_t>(
		CBaseClient_ConnectionStart
	) + CLC_CmdKeyValues_offset;
	ResolveMessagesFromFunctionCode( state, reinterpret_cast<uint8_t *>(
		CLC_CmdKeyValues + 4 + *reinterpret_cast<intptr_t *>( CLC_CmdKeyValues )
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

		LUA->PushCFunction( Clear );
		LUA->SetField( -2, "Clear" );

		LUA->PushCFunction( Process );
		LUA->SetField( -2, "Process" );

	LUA->Pop( 1 );

	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );

		LUA->PushCFunction( Constructor );
		LUA->SetField( -2, metaname );

	LUA->Pop( 1 );

	for( auto pair : netmessages_vtables )
	{
		Push( state, CreateNetMessage( pair.first, pair.second ) );
		static_netmessages[pair.first] = LUA->ReferenceCreate( );
	}
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

	for( auto pair : static_netmessages )
		LUA->ReferenceFree( pair.second );

	for( auto pair : netmessages )
		for( auto pair2 : pair.second )
			LUA->ReferenceFree( pair2.second );
}

}