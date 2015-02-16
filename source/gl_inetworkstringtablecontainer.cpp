#include <gl_inetworkstringtablecontainer.hpp>
#include <gl_inetworkstringtable.hpp>
#include <networkstringtabledefs.h>

struct INetworkStringTableContainer_userdata
{
	INetworkStringTableContainer *container;
	uint8_t type;
};

static void Push_INetworkStringTableContainer(
	lua_State *state,
	INetworkStringTableContainer *container
)
{
	INetworkStringTableContainer_userdata *userdata =
		static_cast<INetworkStringTableContainer_userdata *>(
			LUA->NewUserdata( sizeof( INetworkStringTableContainer_userdata ) )
		);
	userdata->type = GET_META_ID( INetworkStringTableContainer );
	userdata->container = container;

	LUA->CreateMetaTableType(
		GET_META_NAME( INetworkStringTableContainer ),
		GET_META_ID( INetworkStringTableContainer )
	);
	LUA->SetMetaTable( -2 );
}

static INetworkStringTableContainer *Get_INetworkStringTableContainer(
	lua_State *state,
	int32_t index
)
{
	CheckType(
		state,
		index,
		GET_META_ID( INetworkStringTableContainer ),
		GET_META_NAME( INetworkStringTableContainer )
	);

	void *userdata = LUA->GetUserdata( index );
	return static_cast<INetworkStringTableContainer_userdata *>( userdata )->container;
}

META_ID( INetworkStringTableContainer, 10 );

META_FUNCTION( INetworkStringTableContainer, __eq )
{
	Get_INetworkStringTableContainer( state, 1 );
	Get_INetworkStringTableContainer( state, 2 );

	LUA->PushBool( true );

	return 1;
}

META_FUNCTION( INetworkStringTableContainer, __tostring )
{
	INetworkStringTableContainer *container = Get_INetworkStringTableContainer( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", GET_META_NAME( INetworkStringTableContainer ), container );

	return 1;
}

META_FUNCTION( INetworkStringTableContainer, FindTable )
{
	INetworkStringTableContainer *container = Get_INetworkStringTableContainer( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	Push_INetworkStringTable( state, container->FindTable( LUA->GetString( 2 ) ) );

	return 1;
}

META_FUNCTION( INetworkStringTableContainer, GetTable )
{
	INetworkStringTableContainer *container = Get_INetworkStringTableContainer( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	Push_INetworkStringTable(
		state,
		container->GetTable( static_cast<int32_t>( LUA->GetNumber( 2 ) ) )
	);

	return 1;
}

GLBL_FUNCTION( INetworkStringTableContainer )
{

#if defined SOURCENET_SERVER

	INetworkStringTableContainer *container = static_cast<INetworkStringTableContainer *>(
		fnEngineFactory( INTERFACENAME_NETWORKSTRINGTABLESERVER, nullptr )
	);

#elif defined SOURCENET_CLIENT

	INetworkStringTableContainer *container = static_cast<INetworkStringTableContainer *>(
		fnEngineFactory( INTERFACENAME_NETWORKSTRINGTABLECLIENT, nullptr )
	);

#endif

	Push_INetworkStringTableContainer( state, container );

	return 1;
}