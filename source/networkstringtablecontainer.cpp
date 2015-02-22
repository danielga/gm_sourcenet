#include <networkstringtablecontainer.hpp>
#include <networkstringtable.hpp>
#include <networkstringtabledefs.h>

namespace NetworkStringTableContainer
{

struct userdata
{
	INetworkStringTableContainer *container;
	uint8_t type;
};

static const uint8_t metaid = Global::metabase + 10;
static const char *metaname = "INetworkStringTableContainer";

static int32_t container_ref = -1;

static INetworkStringTableContainer *Get( lua_State *state, int32_t index )
{
	Global::CheckType( state, index, metaid, metaname );
	return static_cast<userdata *>( LUA->GetUserdata( index ) )->container;
}

LUA_FUNCTION_STATIC( eq )
{
	Get( state, 1 );
	Get( state, 2 );

	LUA->PushBool( true );

	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	INetworkStringTableContainer *container = Get( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", metaname, container );

	return 1;
}

LUA_FUNCTION_STATIC( GetNumTables )
{
	INetworkStringTableContainer *container = Get( state, 1 );

	LUA->PushNumber( container->GetNumTables( ) );

	return 1;
}

LUA_FUNCTION_STATIC( FindTable )
{
	INetworkStringTableContainer *container = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	NetworkStringTable::Push( state, container->FindTable( LUA->GetString( 2 ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetTable )
{
	INetworkStringTableContainer *container = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	NetworkStringTable::Push(
		state,
		container->GetTable( static_cast<int32_t>( LUA->GetNumber( 2 ) ) )
	);

	return 1;
}

LUA_FUNCTION_STATIC( Constructor )
{
	LUA->ReferencePush( container_ref );

	return 1;
}

void Initialize( lua_State *state )
{
#if defined SOURCENET_SERVER

	INetworkStringTableContainer *container = static_cast<INetworkStringTableContainer *>(
		Global::engine_factory( INTERFACENAME_NETWORKSTRINGTABLESERVER, nullptr )
	);

#elif defined SOURCENET_CLIENT

	INetworkStringTableContainer *container = static_cast<INetworkStringTableContainer *>(
		Global::engine_factory( INTERFACENAME_NETWORKSTRINGTABLECLIENT, nullptr )
	);

#endif

	userdata *udata = static_cast<userdata *>( LUA->NewUserdata( sizeof( userdata ) ) );
	udata->type = metaid;
	udata->container = container;

	LUA->CreateMetaTableType( metaname, metaid );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( state, -2 );

	container_ref = LUA->ReferenceCreate( );

	LUA->CreateMetaTableType( metaname, metaid );

		LUA->PushCFunction( eq );
		LUA->SetField( -2, "__eq" );

		LUA->PushCFunction( tostring );
		LUA->SetField( -2, "__tostring" );

		LUA->PushCFunction( Global::index );
		LUA->SetField( -2, "__index" );

		LUA->PushCFunction( Global::newindex );
		LUA->SetField( -2, "__newindex" );

		LUA->PushCFunction( GetNumTables );
		LUA->SetField( -2, "GetNumTables" );

		LUA->PushCFunction( FindTable );
		LUA->SetField( -2, "FindTable" );

		LUA->PushCFunction( GetTable );
		LUA->SetField( -2, "GetTable" );

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

	LUA->ReferenceFree( container_ref );
	container_ref = -1;
}

}