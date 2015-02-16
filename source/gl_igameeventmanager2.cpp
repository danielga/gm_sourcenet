#include <gl_igameeventmanager2.hpp>
#include <gl_igameevent.hpp>
#include <gl_bitbuf_read.hpp>
#include <gl_bitbuf_write.hpp>
#include <igameevents.h>

struct IGameEventManager2_userdata
{
	IGameEventManager2 *manager;
	uint8_t type;
};

static void Push_IGameEventManager2( lua_State *state, IGameEventManager2 *manager )
{
	IGameEventManager2_userdata *userdata = static_cast<IGameEventManager2_userdata *>(
		LUA->NewUserdata( sizeof( IGameEventManager2_userdata ) )
	);
	userdata->type = GET_META_ID( IGameEventManager2 );
	userdata->manager = manager;

	LUA->CreateMetaTableType(
		GET_META_NAME( IGameEventManager2 ),
		GET_META_ID( IGameEventManager2 )
	);
	LUA->SetMetaTable( -2 );
}

static IGameEventManager2 *Get_IGameEventManager2( lua_State *state, int32_t index )
{
	LUA->CheckType( index, GET_META_ID( IGameEventManager2 ) );
	return static_cast<IGameEventManager2_userdata *>( LUA->GetUserdata( index ) )->manager;
}

META_ID( IGameEventManager2, 12 );

META_FUNCTION( IGameEventManager2, __eq )
{
	LUA->CheckType( 1, GET_META_ID( IGameEventManager2 ) );
	LUA->CheckType( 2, GET_META_ID( IGameEventManager2 ) );

	LUA->PushBool( true );

	return 1;
}

META_FUNCTION( IGameEventManager2, __tostring )
{
	IGameEventManager2 *manager = Get_IGameEventManager2( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", GET_META_NAME( IGameEventManager2 ), manager );

	return 1;
}

META_FUNCTION( IGameEventManager2, CreateEvent )
{
	IGameEventManager2 *manager = Get_IGameEventManager2( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	IGameEvent *event = manager->CreateEvent( LUA->GetString( 2 ), true );

	Push_IGameEvent( state, event, manager );

	return 1;
}

META_FUNCTION( IGameEventManager2, SerializeEvent )
{
	IGameEventManager2 *manager = Get_IGameEventManager2( state, 1 );
	IGameEvent *event = Get_IGameEvent( state, 2 );
	sn4_bf_write *buf = Get_sn4_bf_write( state, 3 );

	LUA->PushBool( manager->SerializeEvent( event, buf ) );

	return 1;
}

META_FUNCTION( IGameEventManager2, UnserializeEvent )
{
	IGameEventManager2 *manager = Get_IGameEventManager2( state, 1 );
	sn4_bf_read *buf = Get_sn4_bf_read( state, 2 );

	IGameEvent *event = manager->UnserializeEvent( buf );

	Push_IGameEvent( state, event, manager );

	return 1;
}

GLBL_FUNCTION( IGameEventManager2 )
{
	IGameEventManager2 *manager = static_cast<IGameEventManager2 *>(
		fnEngineFactory( INTERFACEVERSION_GAMEEVENTSMANAGER2, nullptr )
	);

	Push_IGameEventManager2( state, manager );

	return 1;
}