#include <main.hpp>
#include <GarrysMod/Lua/AutoReference.h>
#include <gameevent.hpp>
#include <sn_bf_read.hpp>
#include <sn_bf_write.hpp>
#include <igameevents.h>

namespace GameEventManager
{

struct UserData
{
	IGameEventManager2 *manager;
	uint8_t type;
};

static const uint8_t metatype = global::metabase + 12;
static const char *metaname = "IGameEventManager2";

static GarrysMod::Lua::AutoReference manager_ref;

static IGameEventManager2 *Get( lua_State *state, int32_t index )
{
	global::CheckType( state, index, metatype, metaname );
	return static_cast<UserData *>( LUA->GetUserdata( index ) )->manager;
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

	lua_pushfstring( state, global::tostring_format, metaname, manager );

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
	bf_write *buf = sn_bf_write::Get( state, 3 );

	LUA->PushBool( manager->SerializeEvent( event, buf ) );

	return 1;
}

LUA_FUNCTION_STATIC( UnserializeEvent )
{
	IGameEventManager2 *manager = Get( state, 1 );
	bf_read *buf = sn_bf_read::Get( state, 2 );

	IGameEvent *event = manager->UnserializeEvent( buf );

	GameEvent::Push( state, event, manager );

	return 1;
}

LUA_FUNCTION_STATIC( Constructor )
{
	manager_ref.Push( );
	return 1;
}

void Initialize( lua_State *state )
{
	IGameEventManager2 *manager = global::engine_loader.GetInterface<IGameEventManager2>(
		INTERFACEVERSION_GAMEEVENTSMANAGER2
	);
	if( manager == nullptr )
		LUA->ThrowError( "failed to obtain IGameEventManager2" );

	UserData *udata = static_cast<UserData *>( LUA->NewUserdata( sizeof( UserData ) ) );
	udata->manager = manager;
	udata->type = metatype;

	LUA->CreateMetaTableType( metaname, metatype );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( state, -2 );

	manager_ref.Setup( LUA );
	manager_ref.Create( );



	LUA->CreateMetaTableType( metaname, metatype );

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

		LUA->PushCFunction( CreateEvent );
		LUA->SetField( -2, "CreateEvent" );

		LUA->PushCFunction( SerializeEvent );
		LUA->SetField( -2, "SerializeEvent" );

		LUA->PushCFunction( UnserializeEvent );
		LUA->SetField( -2, "UnserializeEvent" );

	LUA->Pop( 1 );



	LUA->PushCFunction( Constructor );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, metaname );
}

void Deinitialize( lua_State *state )
{
	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, metaname );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, metaname );



	manager_ref.Free( );
}

}
