#include <netmessage.hpp>
#include <sn4_bf_write.hpp>
#include <sn4_bf_read.hpp>
#include <netchannel.hpp>
#include <net.hpp>
#include <inetmessage.h>

namespace NetMessage
{

struct userdata
{
	INetMessage *msg;
	uint8_t type;
	CNetChan *netchan;
};

const uint8_t metaid = Global::metabase + 14;
const char *metaname = "INetMessage";

static bool IsValid( INetMessage *msg, CNetChan *netchan )
{
	return msg != nullptr && NetChannel::IsValid( netchan );
}

void Push( lua_State *state, INetMessage *msg, CNetChan *netchan )
{
	userdata *udata = static_cast<userdata *>( LUA->NewUserdata( sizeof( userdata ) ) );
	udata->type = metaid;
	udata->msg = msg;
	udata->netchan = netchan;

	LUA->CreateMetaTableType( metaname, metaid );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( state, -2 );
}

INetMessage *Get( lua_State *state, int32_t index, CNetChan **netchan )
{
	Global::CheckType( state, index, metaid, metaname );

	userdata *udata = static_cast<userdata *>( LUA->GetUserdata( index ) );
	INetMessage *msg = udata->msg;
	if( !IsValid( msg, udata->netchan ) )
		Global::ThrowError( state, "invalid %s", metaname );

	if( netchan != nullptr )
		*netchan = udata->netchan;

	return msg;
}

//void Destroy( lua_State *state, INetMessage *msg ) { }

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
}

void Deinitialize( lua_State *state )
{
	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_REG );

		LUA->PushNil( );
		LUA->SetField( -2, metaname );

	LUA->Pop( 1 );
}

}