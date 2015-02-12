#include <gl_inetchannelhandler.hpp>

struct INetChannelHandler_userdata
{
	INetChannelHandler *handler;
	uint8_t type;
};

void Push_INetChannelHandler( lua_State *state, INetChannelHandler *handler )
{
	INetChannelHandler_userdata *userdata = static_cast<INetChannelHandler_userdata *>( LUA->NewUserdata( sizeof( INetChannelHandler_userdata ) ) );
	userdata->type = GET_META_ID( INetChannelHandler );
	userdata->handler = handler;

	LUA->CreateMetaTableType( GET_META_NAME( INetChannelHandler ), GET_META_ID( INetChannelHandler ) );
	LUA->SetMetaTable( -2 );
}

INetChannelHandler *Get_INetChannelHandler( lua_State *state, int32_t index )
{
	LUA->CheckType( index, GET_META_ID( INetChannelHandler ) );
	return static_cast<INetChannelHandler_userdata *>( LUA->GetUserdata( index ) )->handler;
}

META_ID( INetChannelHandler, 9 );

META_FUNCTION( INetChannelHandler, __eq )
{
	INetChannelHandler *handler1 = Get_INetChannelHandler( state, 1 );
	INetChannelHandler *handler2 = Get_INetChannelHandler( state, 2 );

	LUA->PushBool( handler1 == handler2 );

	return 1;
}

META_FUNCTION( INetChannelHandler, __tostring )
{
	INetChannelHandler *handler = Get_INetChannelHandler( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", GET_META_NAME( INetChannelHandler ), handler );

	return 1;
}