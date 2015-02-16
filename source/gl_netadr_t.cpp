#include <gl_netadr_t.hpp>
#include <netadr.h>

struct netadr_userdata
{
	netadr_t *pnetadr;
	uint8_t type;
	netadr_t netadr;
};

void Push_netadr( lua_State *state, const netadr_t &netadr )
{
	netadr_userdata *userdata = static_cast<netadr_userdata *>(
		LUA->NewUserdata( sizeof( netadr_userdata ) )
	);
	userdata->type = GET_META_ID( netadr_t );
	userdata->pnetadr = &userdata->netadr;
	new( &userdata->netadr ) netadr_t( netadr );

	LUA->CreateMetaTableType( GET_META_NAME( netadr_t ), GET_META_ID( netadr_t ) );
	LUA->SetMetaTable( -2 );
}

static netadr_t *Get_netadr( lua_State *state, int32_t index )
{
	CheckType( state, index, GET_META_ID( netadr_t ), GET_META_NAME( netadr_t ) );
	return static_cast<netadr_userdata *>( LUA->GetUserdata( index ) )->pnetadr;
}

META_ID( netadr_t, 8 );

META_FUNCTION( netadr_t, __eq )
{
	netadr_t *adr1 = Get_netadr( state, 1 );
	netadr_t *adr2 = Get_netadr( state, 2 );

	bool baseonly = false;
	if( LUA->IsType( 3, GarrysMod::Lua::Type::BOOL ) )
		baseonly = LUA->GetBool( 3 );

	LUA->PushBool( adr1->CompareAdr( *adr2, baseonly ) );

	return 1;
}

META_FUNCTION( netadr_t, __tostring )
{
	netadr_t *adr = Get_netadr( state, 1 );

	bool baseonly = false;
	if( LUA->IsType( 2, GarrysMod::Lua::Type::BOOL ) )
		baseonly = LUA->GetBool( 2 );

	LUA->PushString( adr->ToString( baseonly ) );

	return 1;
}

META_FUNCTION( netadr_t, IsLocalhost )
{
	netadr_t *adr = Get_netadr( state, 1 );

	LUA->PushBool( adr->IsLocalhost( ) );

	return 1;
}

META_FUNCTION( netadr_t, IsLoopback )
{
	netadr_t *adr = Get_netadr( state, 1 );

	LUA->PushBool( adr->IsLoopback( ) );

	return 1;
}

META_FUNCTION( netadr_t, IsReservedAdr )
{
	netadr_t *adr = Get_netadr( state, 1 );

	LUA->PushBool( adr->IsReservedAdr( ) );

	return 1;
}

META_FUNCTION( netadr_t, IsValid )
{
	netadr_t *adr = Get_netadr( state, 1 );

	LUA->PushBool( adr->IsValid( ) );

	return 1;
}

META_FUNCTION( netadr_t, GetIP )
{
	netadr_t *adr = Get_netadr( state, 1 );

	LUA->PushString( adr->ToString( true ) );
	LUA->PushNumber( adr->GetIPHostByteOrder( ) );

	return 2;
}

META_FUNCTION( netadr_t, GetPort )
{
	netadr_t *adr = Get_netadr( state, 1 );

	LUA->PushNumber( adr->GetPort( ) );

	return 1;
}

META_FUNCTION( netadr_t, GetType )
{
	netadr_t *adr = Get_netadr( state, 1 );

	LUA->PushNumber( adr->GetType( ) );

	return 1;
}