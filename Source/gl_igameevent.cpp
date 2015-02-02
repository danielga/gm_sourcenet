#include <gl_igameevent.hpp>

#include <igameevents.h>

META_ID( IGameEvent, 13 );

META_FUNCTION( IGameEvent, GetName )
{
	LUA->CheckType( 1, GET_META_ID( IGameEvent ) );

	IGameEvent *event = GET_META( 1, IGameEvent );

	LUA->PushString( event->GetName( ) );

	return 1;
}

META_FUNCTION( IGameEvent, IsReliable )
{
	LUA->CheckType( 1, GET_META_ID( IGameEvent ) );

	IGameEvent *event = GET_META( 1, IGameEvent );

	LUA->PushBool( event->IsReliable( ) );

	return 1;
}

META_FUNCTION( IGameEvent, IsLocal )
{
	LUA->CheckType( 1, GET_META_ID( IGameEvent ) );

	IGameEvent *event = GET_META( 1, IGameEvent );

	LUA->PushBool( event->IsLocal( ) );

	return 1;
}

META_FUNCTION( IGameEvent, IsEmpty )
{
	LUA->CheckType( 1, GET_META_ID( IGameEvent ) );

	IGameEvent *event = GET_META( 1, IGameEvent );

	LUA->PushBool( event->IsEmpty( LUA->GetString( 2 ) ) );

	return 1;
}

META_FUNCTION( IGameEvent, GetBool )
{
	LUA->CheckType( 1, GET_META_ID( IGameEvent ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	IGameEvent *event = GET_META( 1, IGameEvent );

	LUA->PushBool( event->GetBool( LUA->GetString( 2 ) ) );

	return 1;
}

META_FUNCTION( IGameEvent, GetInt )
{
	LUA->CheckType( 1, GET_META_ID( IGameEvent ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	IGameEvent *event = GET_META( 1, IGameEvent );

	LUA->PushNumber( event->GetInt( LUA->GetString( 2 ) ) );

	return 1;
}

META_FUNCTION( IGameEvent, GetFloat )
{
	LUA->CheckType( 1, GET_META_ID( IGameEvent ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	IGameEvent *event = GET_META( 1, IGameEvent );

	LUA->PushNumber( event->GetFloat( LUA->GetString( 2 ) ) );

	return 1;
}

META_FUNCTION( IGameEvent, GetString )
{
	LUA->CheckType( 1, GET_META_ID( IGameEvent ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	IGameEvent *event = GET_META( 1, IGameEvent );

	LUA->PushString( event->GetString( LUA->GetString( 2 ) ) );

	return 1;
}

META_FUNCTION( IGameEvent, SetBool )
{
	LUA->CheckType( 1, GET_META_ID( IGameEvent ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::BOOL );

	IGameEvent *event = GET_META( 1, IGameEvent );

	event->SetBool( LUA->GetString( 2 ), LUA->GetBool( 3 ) );

	return 0;
}

META_FUNCTION( IGameEvent, SetInt )
{
	LUA->CheckType( 1, GET_META_ID( IGameEvent ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	IGameEvent *event = GET_META( 1, IGameEvent );

	event->SetInt( LUA->GetString( 2 ), static_cast<int>( LUA->GetNumber( 3 ) ) );

	return 0;
}

META_FUNCTION( IGameEvent, SetFloat )
{
	LUA->CheckType( 1, GET_META_ID( IGameEvent ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	IGameEvent *event = GET_META( 1, IGameEvent );

	event->SetFloat( LUA->GetString( 2 ), LUA->GetNumber( 3 ) );

	return 0;
}

META_FUNCTION( IGameEvent, SetString )
{
	LUA->CheckType( 1, GET_META_ID( IGameEvent ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::STRING );

	IGameEvent *event = GET_META( 1, IGameEvent );

	event->SetString( LUA->GetString( 2 ), LUA->GetString( 3 ) );

	return 0;
}

META_FUNCTION( IGameEvent, Delete )
{
	LUA->CheckType( 1, GET_META_ID( IGameEvent ) );

	IGameEvent *event = GET_META( 1, IGameEvent );

	event->~IGameEvent( );

	return 0;
}