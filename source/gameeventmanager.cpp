#include <main.hpp>
#include <gameeventmanager.hpp>
#include <GarrysMod/Lua/AutoReference.h>
#include <gameevent.hpp>
#include <sn_bf_read.hpp>
#include <sn_bf_write.hpp>
#include <igameevents.h>

namespace GameEventManager
{

static uint8_t metatype = GarrysMod::Lua::Type::NONE;
static const char *metaname = "IGameEventManager2";

static GarrysMod::Lua::AutoReference manager_ref;

static IGameEventManager2 *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
{
	global::CheckType( LUA, index, metatype, metaname );
	return LUA->GetUserType<IGameEventManager2>( index, metatype );
}

LUA_FUNCTION_STATIC( eq )
{
	Get( LUA, 1 );
	Get( LUA, 2 );

	LUA->PushBool( true );

	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	IGameEventManager2 *manager = Get( LUA, 1 );

	lua_pushfstring( LUA->state, global::tostring_format, metaname, manager );

	return 1;
}

LUA_FUNCTION_STATIC( CreateEvent )
{
	IGameEventManager2 *manager = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	IGameEvent *event = manager->CreateEvent( LUA->GetString( 2 ), true );

	GameEvent::Push( LUA, event, manager );

	return 1;
}

LUA_FUNCTION_STATIC( SerializeEvent )
{
	IGameEventManager2 *manager = Get( LUA, 1 );
	IGameEvent *event = GameEvent::Get( LUA, 2 );
	bf_write *buf = sn_bf_write::Get( LUA, 3 );

	LUA->PushBool( manager->SerializeEvent( event, buf ) );

	return 1;
}

LUA_FUNCTION_STATIC( UnserializeEvent )
{
	IGameEventManager2 *manager = Get( LUA, 1 );
	bf_read *buf = sn_bf_read::Get( LUA, 2 );

	IGameEvent *event = manager->UnserializeEvent( buf );

	GameEvent::Push( LUA, event, manager );

	return 1;
}

LUA_FUNCTION_STATIC( Constructor )
{
	manager_ref.Push( );
	return 1;
}

void Initialize( GarrysMod::Lua::ILuaBase *LUA )
{
	IGameEventManager2 *manager = global::engine_loader.GetInterface<IGameEventManager2>(
		INTERFACEVERSION_GAMEEVENTSMANAGER2
	);
	if( manager == nullptr )
		LUA->ThrowError( "failed to obtain IGameEventManager2" );



	metatype = LUA->CreateMetaTable( metaname );

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



	LUA->PushUserType( manager, metatype );

	LUA->PushMetaTable( metatype );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( LUA->state, -2 );

	manager_ref.Setup( LUA );
	manager_ref.Create( );



	LUA->PushCFunction( Constructor );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, metaname );
}

void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
{
	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, metaname );



	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, metaname );



	manager_ref.Free( );
}

}
