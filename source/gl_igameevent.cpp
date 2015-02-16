#include <gl_igameevent.hpp>
#include <igameevents.h>

struct IGameEvent_userdata
{
	IGameEvent *event;
	uint8_t type;
	IGameEventManager2 *manager;
};

void Push_IGameEvent( lua_State *state, IGameEvent *event, IGameEventManager2 *manager )
{
	IGameEvent_userdata *userdata = static_cast<IGameEvent_userdata *>(
		LUA->NewUserdata( sizeof( IGameEvent_userdata ) )
	);
	userdata->type = GET_META_ID( IGameEvent );
	userdata->event = event;
	userdata->manager = manager;

	LUA->CreateMetaTableType( GET_META_NAME( IGameEvent ), GET_META_ID( IGameEvent ) );
	LUA->SetMetaTable( -2 );
}

IGameEvent *Get_IGameEvent(
	lua_State *state,
	int32_t index,
	IGameEventManager2 **manager,
	bool cleanup
)
{
	LUA->CheckType( index, GET_META_ID( IGameEvent ) );

	IGameEvent_userdata *userdata = static_cast<IGameEvent_userdata *>(
		LUA->GetUserdata( index )
	);
	IGameEvent *event = userdata->event;
	if( event == nullptr || userdata->manager == nullptr )
		LUA->ThrowError( "invalid IGameEvent" );

	if( manager != nullptr )
		*manager = userdata->manager;

	if( cleanup )
	{
		userdata->event = nullptr;
		userdata->manager = nullptr;
	}

	return event;
}

META_ID( IGameEvent, 13 );

META_FUNCTION( IGameEvent, __gc )
{
	IGameEventManager2 *manager = nullptr;
	IGameEvent *event = Get_IGameEvent( state, 1, &manager, true );

	manager->FreeEvent( event );

	return 0;
}

META_FUNCTION( IGameEvent, __eq )
{
	IGameEvent *event1 = Get_IGameEvent( state, 1 );
	IGameEvent *event2 = Get_IGameEvent( state, 2 );

	LUA->PushBool( event1 == event2 );

	return 1;
}

META_FUNCTION( IGameEvent, __tostring )
{
	IGameEvent *event = Get_IGameEvent( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", GET_META_NAME( IGameEvent ), event );

	return 1;
}

META_FUNCTION( IGameEvent, GetName )
{
	IGameEvent *event = Get_IGameEvent( state, 1 );

	LUA->PushString( event->GetName( ) );

	return 1;
}

META_FUNCTION( IGameEvent, IsReliable )
{
	IGameEvent *event = Get_IGameEvent( state, 1 );

	LUA->PushBool( event->IsReliable( ) );

	return 1;
}

META_FUNCTION( IGameEvent, IsLocal )
{
	IGameEvent *event = Get_IGameEvent( state, 1 );

	LUA->PushBool( event->IsLocal( ) );

	return 1;
}

META_FUNCTION( IGameEvent, IsEmpty )
{
	IGameEvent *event = Get_IGameEvent( state, 1 );

	LUA->PushBool( event->IsEmpty( LUA->GetString( 2 ) ) );

	return 1;
}

META_FUNCTION( IGameEvent, GetBool )
{
	IGameEvent *event = Get_IGameEvent( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushBool( event->GetBool( LUA->GetString( 2 ) ) );

	return 1;
}

META_FUNCTION( IGameEvent, GetInt )
{
	IGameEvent *event = Get_IGameEvent( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushNumber( event->GetInt( LUA->GetString( 2 ) ) );

	return 1;
}

META_FUNCTION( IGameEvent, GetFloat )
{
	IGameEvent *event = Get_IGameEvent( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushNumber( event->GetFloat( LUA->GetString( 2 ) ) );

	return 1;
}

META_FUNCTION( IGameEvent, GetString )
{
	IGameEvent *event = Get_IGameEvent( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushString( event->GetString( LUA->GetString( 2 ) ) );

	return 1;
}

META_FUNCTION( IGameEvent, SetBool )
{
	IGameEvent *event = Get_IGameEvent( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::BOOL );

	event->SetBool( LUA->GetString( 2 ), LUA->GetBool( 3 ) );

	return 0;
}

META_FUNCTION( IGameEvent, SetInt )
{
	IGameEvent *event = Get_IGameEvent( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	event->SetInt( LUA->GetString( 2 ), static_cast<int32_t>( LUA->GetNumber( 3 ) ) );

	return 0;
}

META_FUNCTION( IGameEvent, SetFloat )
{
	IGameEvent *event = Get_IGameEvent( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	event->SetFloat( LUA->GetString( 2 ), LUA->GetNumber( 3 ) );

	return 0;
}

META_FUNCTION( IGameEvent, SetString )
{
	IGameEvent *event = Get_IGameEvent( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::STRING );

	event->SetString( LUA->GetString( 2 ), LUA->GetString( 3 ) );

	return 0;
}