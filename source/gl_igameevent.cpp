#include <gl_igameevent.hpp>
#include <igameevents.h>

namespace GameEvent
{

struct userdata
{
	IGameEvent *event;
	uint8_t type;
	IGameEventManager2 *manager;
};

static const uint8_t metaid = Global::metabase + 13;
static const char *metaname = "IGameEvent";

void Push( lua_State *state, IGameEvent *event, IGameEventManager2 *manager )
{
	userdata *udata = static_cast<userdata *>( LUA->NewUserdata( sizeof( userdata ) ) );
	udata->type = metaid;
	udata->event = event;
	udata->manager = manager;

	LUA->CreateMetaTableType( metaname, metaid );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( state, -2 );
}

IGameEvent *Get( lua_State *state, int32_t index, IGameEventManager2 **manager, bool cleanup )
{
	Global::CheckType( state, index, metaid, metaname );

	userdata *udata = static_cast<userdata *>( LUA->GetUserdata( index ) );
	IGameEvent *event = udata->event;
	if( ( event == nullptr || udata->manager == nullptr ) && !cleanup )
		static_cast<GarrysMod::Lua::ILuaInterface *>( LUA )->ErrorFromLua( "invalid %s", metaname );

	if( manager != nullptr )
		*manager = udata->manager;

	if( cleanup )
	{
		udata->event = nullptr;
		udata->manager = nullptr;
	}

	return event;
}

LUA_FUNCTION_STATIC( gc )
{
	IGameEventManager2 *manager = nullptr;
	IGameEvent *event = Get( state, 1, &manager, true );

	if( manager != nullptr )
		manager->FreeEvent( event );

	return 0;
}

LUA_FUNCTION_STATIC( eq )
{
	IGameEvent *event1 = Get( state, 1 );
	IGameEvent *event2 = Get( state, 2 );

	LUA->PushBool( event1 == event2 );

	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	IGameEvent *event = Get( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", metaname, event );

	return 1;
}

LUA_FUNCTION_STATIC( GetName )
{
	IGameEvent *event = Get( state, 1 );

	LUA->PushString( event->GetName( ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsReliable )
{
	IGameEvent *event = Get( state, 1 );

	LUA->PushBool( event->IsReliable( ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsLocal )
{
	IGameEvent *event = Get( state, 1 );

	LUA->PushBool( event->IsLocal( ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsEmpty )
{
	IGameEvent *event = Get( state, 1 );

	LUA->PushBool( event->IsEmpty( LUA->GetString( 2 ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetBool )
{
	IGameEvent *event = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushBool( event->GetBool( LUA->GetString( 2 ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetInt )
{
	IGameEvent *event = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushNumber( event->GetInt( LUA->GetString( 2 ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetFloat )
{
	IGameEvent *event = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushNumber( event->GetFloat( LUA->GetString( 2 ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetString )
{
	IGameEvent *event = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushString( event->GetString( LUA->GetString( 2 ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( SetBool )
{
	IGameEvent *event = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::BOOL );

	event->SetBool( LUA->GetString( 2 ), LUA->GetBool( 3 ) );

	return 0;
}

LUA_FUNCTION_STATIC( SetInt )
{
	IGameEvent *event = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	event->SetInt( LUA->GetString( 2 ), static_cast<int32_t>( LUA->GetNumber( 3 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( SetFloat )
{
	IGameEvent *event = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	event->SetFloat( LUA->GetString( 2 ), LUA->GetNumber( 3 ) );

	return 0;
}

LUA_FUNCTION_STATIC( SetString )
{
	IGameEvent *event = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::STRING );

	event->SetString( LUA->GetString( 2 ), LUA->GetString( 3 ) );

	return 0;
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

		LUA->PushCFunction( GetName );
		LUA->SetField( -2, "GetName" );

		LUA->PushCFunction( IsReliable );
		LUA->SetField( -2, "IsReliable" );

		LUA->PushCFunction( IsLocal );
		LUA->SetField( -2, "IsLocal" );

		LUA->PushCFunction( IsEmpty );
		LUA->SetField( -2, "IsEmpty" );

		LUA->PushCFunction( GetBool );
		LUA->SetField( -2, "GetBool" );

		LUA->PushCFunction( GetInt );
		LUA->SetField( -2, "GetInt" );

		LUA->PushCFunction( GetFloat );
		LUA->SetField( -2, "GetFloat" );

		LUA->PushCFunction( GetString );
		LUA->SetField( -2, "GetString" );

		LUA->PushCFunction( SetBool );
		LUA->SetField( -2, "SetBool" );

		LUA->PushCFunction( SetInt );
		LUA->SetField( -2, "SetInt" );

		LUA->PushCFunction( SetFloat );
		LUA->SetField( -2, "SetFloat" );

		LUA->PushCFunction( SetString );
		LUA->SetField( -2, "SetString" );

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