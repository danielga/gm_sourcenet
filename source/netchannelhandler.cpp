#include <netchannelhandler.hpp>
#include <GarrysMod/Lua/AutoReference.h>
#include <hooks.hpp>
#include <unordered_map>

#include <dbg.h>

namespace NetChannelHandler
{

struct userdata
{
	INetChannelHandler *handler;
	uint8_t type;
};

const uint8_t metaid = global::metabase + 9;
const char *metaname = "INetChannelHandler";

static std::unordered_map<INetChannelHandler *, GarrysMod::Lua::AutoReference> handlers;

void Push( lua_State *state, INetChannelHandler *handler )
{
	auto it = handlers.find( handler );
	if( it != handlers.end( ) )
	{
		( *it ).second.Push( );
		return;
	}

	userdata *udata = static_cast<userdata *>( LUA->NewUserdata( sizeof( userdata ) ) );
	udata->type = metaid;
	udata->handler = handler;

	LUA->CreateMetaTableType( metaname, metaid );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( state, -2 );

	GarrysMod::Lua::AutoReference &ref = handlers[handler];
	ref.Setup( LUA );
	ref.Create( -1 );

	Hooks::HookINetChannelHandler( state );
}

INetChannelHandler *Get( lua_State *state, int32_t index )
{
	global::CheckType( state, index, metaid, metaname );
	return static_cast<userdata *>( LUA->GetUserdata( index ) )->handler;
}

void Destroy( lua_State *state, INetChannelHandler *handler )
{
	auto it = handlers.find( handler );
	if( it != handlers.end( ) )
		handlers.erase( it );
}

LUA_FUNCTION_STATIC( eq )
{
	INetChannelHandler *handler1 = Get( state, 1 );
	INetChannelHandler *handler2 = Get( state, 2 );

	LUA->PushBool( handler1 == handler2 );

	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	INetChannelHandler *handler = Get( state, 1 );

	lua_pushfstring( state, global::tostring_format, metaname, handler );

	return 1;
}

void Initialize( lua_State *state )
{
	LUA->CreateMetaTableType( metaname, metaid );

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

	LUA->Pop( 1 );
}

void Deinitialize( lua_State *state )
{
	LUA->PushNil( );
	LUA->SetField( -2, metaname );

	handlers.clear( );
}

}