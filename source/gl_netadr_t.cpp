#include <gl_netadr_t.hpp>
#include <netadr.h>

META_ID( netadr_t, 8 );

META_FUNCTION( netadr_t, IsLocalhost )
{
	LUA->CheckType( 1, GET_META_ID( netadr_t ) );

	netadr_t *adr = GET_META( 1, netadr_t );

	LUA->PushBool( adr->IsLocalhost( ) );

	return 1;
}

META_FUNCTION( netadr_t, IsLoopback )
{
	LUA->CheckType( 1, GET_META_ID( netadr_t ) );

	netadr_t *adr = GET_META( 1, netadr_t );

	LUA->PushBool( adr->IsLoopback( ) );

	return 1;
}

META_FUNCTION( netadr_t, IsReservedAdr )
{
	LUA->CheckType( 1, GET_META_ID( netadr_t ) );

	netadr_t *adr = GET_META( 1, netadr_t );

	LUA->PushBool( adr->IsReservedAdr( ) );

	return 1;
}

META_FUNCTION( netadr_t, IsValid )
{
	LUA->CheckType( 1, GET_META_ID( netadr_t ) );

	netadr_t *adr = GET_META( 1, netadr_t );

	LUA->PushBool( adr->IsValid( ) );

	return 1;
}

META_FUNCTION( netadr_t, GetIP )
{
	LUA->CheckType( 1, GET_META_ID( netadr_t ) );

	netadr_t *adr = GET_META( 1, netadr_t );

	LUA->PushNumber( adr->GetIPHostByteOrder( ) );

	return 1;
}

META_FUNCTION( netadr_t, GetPort )
{
	LUA->CheckType( 1, GET_META_ID( netadr_t ) );

	netadr_t *adr = GET_META( 1, netadr_t );

	LUA->PushNumber( adr->GetPort( ) );

	return 1;
}

META_FUNCTION( netadr_t, GetType )
{
	LUA->CheckType( 1, GET_META_ID( netadr_t ) );

	netadr_t *adr = GET_META( 1, netadr_t );

	LUA->PushNumber( adr->GetType( ) );

	return 1;
}

META_FUNCTION( netadr_t, ToString )
{
	LUA->CheckType( 1, GET_META_ID( netadr_t ) );

	netadr_t *adr = GET_META( 1, netadr_t );

	LUA->PushString( adr->ToString( LUA->GetBool( 2 ) ) );

	return 1;
}