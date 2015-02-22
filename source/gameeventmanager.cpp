#include <main.hpp>
#include <gameevent.hpp>
#include <sn4_bf_read.hpp>
#include <sn4_bf_write.hpp>
#include <igameevents.h>

namespace GameEventManager
{

struct userdata
{
	IGameEventManager2 *manager;
	uint8_t type;
};

static const uint8_t metaid = Global::metabase + 12;
static const char *metaname = "IGameEventManager2";

static int32_t manager_ref = -1;

static IGameEventManager2 *Get( lua_State *state, int32_t index )
{
	Global::CheckType( state, index, metaid, metaname );
	return static_cast<userdata *>( LUA->GetUserdata( index ) )->manager;
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
	IGameEventManager2 *manager = Get( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", metaname, manager );

	return 1;
}

LUA_FUNCTION_STATIC( CreateEvent )
{
	IGameEventManager2 *manager = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	IGameEvent *event = manager->CreateEvent( LUA->GetString( 2 ), true );

	GameEvent::Push( state, event, manager );

	return 1;
}

LUA_FUNCTION_STATIC( SerializeEvent )
{
	IGameEventManager2 *manager = Get( state, 1 );
	IGameEvent *event = GameEvent::Get( state, 2 );
	bf_write *buf = sn4_bf_write::Get( state, 3 );

	LUA->PushBool( manager->SerializeEvent( event, buf ) );

	return 1;
}

LUA_FUNCTION_STATIC( UnserializeEvent )
{
	IGameEventManager2 *manager = Get( state, 1 );
	bf_read *buf = sn4_bf_read::Get( state, 2 );

	IGameEvent *event = manager->UnserializeEvent( buf );

	GameEvent::Push( state, event, manager );

	return 1;
}

LUA_FUNCTION_STATIC( Constructor )
{
	LUA->ReferencePush( manager_ref );

	return 1;
}

void Initialize( lua_State *state )
{
	IGameEventManager2 *manager = static_cast<IGameEventManager2 *>(
		Global::engine_factory( INTERFACEVERSION_GAMEEVENTSMANAGER2, nullptr )
	);

	userdata *udata = static_cast<userdata *>( LUA->NewUserdata( sizeof( userdata ) ) );
	udata->type = metaid;
	udata->manager = manager;

	LUA->CreateMetaTableType( metaname, metaid );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( state, -2 );

	manager_ref = LUA->ReferenceCreate( );

	LUA->CreateMetaTableType( metaname, metaid );

		LUA->PushCFunction( eq );
		LUA->SetField( -2, "__eq" );

		LUA->PushCFunction( tostring );
		LUA->SetField( -2, "__tostring" );

		LUA->PushCFunction( Global::index );
		LUA->SetField( -2, "__index" );

		LUA->PushCFunction( Global::newindex );
		LUA->SetField( -2, "__newindex" );

		LUA->PushCFunction( CreateEvent );
		LUA->SetField( -2, "CreateEvent" );

		LUA->PushCFunction( SerializeEvent );
		LUA->SetField( -2, "SerializeEvent" );

		LUA->PushCFunction( UnserializeEvent );
		LUA->SetField( -2, "UnserializeEvent" );

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

	LUA->ReferenceFree( manager_ref );
	manager_ref = -1;
}

}