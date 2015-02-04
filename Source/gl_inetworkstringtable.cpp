#include <gl_inetworkstringtable.hpp>
#include <networkstringtabledefs.h>

META_ID( INetworkStringTable, 11 );

META_FUNCTION( INetworkStringTable, GetString )
{
	LUA->CheckType( 1, GET_META_ID( INetworkStringTable ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	INetworkStringTable *pTable = GET_META( 1, INetworkStringTable );

	const char *str = pTable->GetString( static_cast<int>( LUA->GetNumber( 2 ) ) );
	if( str != nullptr )
		LUA->PushString( str );
	else
		LUA->PushNil( );

	return 1;
}