#include <gl_igameeventmanager2.hpp>
#include <gl_igameevent.hpp>
#include <gl_bitbuf_read.hpp>
#include <gl_bitbuf_write.hpp>

#include <igameevents.h>

META_ID( IGameEventManager2, 12 );

META_FUNCTION( IGameEventManager2, CreateEvent )
{
	LUA->CheckType( 1, GET_META_ID( IGameEventManager2 ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	IGameEventManager2 *pGameEventListener = GET_META( 1, IGameEventManager2 );

	IGameEvent *event = pGameEventListener->CreateEvent( LUA->GetString( 2 ), true );

	PUSH_META( event, IGameEvent );

	return 1;
}

META_FUNCTION( IGameEventManager2, SerializeEvent )
{
	LUA->CheckType( 1, GET_META_ID( IGameEventManager2 ) );
	LUA->CheckType( 2, GET_META_ID( IGameEvent ) );
	LUA->CheckType( 3, GET_META_ID( sn4_bf_write ) );

	IGameEventManager2 *pGameEventListener = GET_META( 1, IGameEventManager2 );

	IGameEvent *event = GET_META( 2, IGameEvent );

	sn4_bf_write *buf = GET_META( 3, sn4_bf_write );

	LUA->PushBool( pGameEventListener->SerializeEvent( event, buf ) );

	return 1;
}

META_FUNCTION( IGameEventManager2, UnserializeEvent )
{
	LUA->CheckType( 1, GET_META_ID( IGameEventManager2 ) );
	LUA->CheckType( 2, GET_META_ID( sn4_bf_read ) );

	IGameEventManager2 *pGameEventListener = GET_META( 1, IGameEventManager2 );

	sn4_bf_read *buf = GET_META( 2, sn4_bf_read );

	IGameEvent *event = pGameEventListener->UnserializeEvent( buf );

	PUSH_META( event, IGameEvent );

	return 1;
}

GLBL_FUNCTION( IGameEventManager2 )
{
	IGameEventManager2 *pGameEventListener = static_cast<IGameEventManager2 *>( fnEngineFactory( INTERFACEVERSION_GAMEEVENTSMANAGER2, nullptr ) );

	PUSH_META( pGameEventListener, IGameEventManager2 );

	return 1;
}