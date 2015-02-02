#include <gl_inetworkstringtablecontainer.hpp>
#include <gl_inetworkstringtable.hpp>

#include <networkstringtabledefs.h>

META_ID( INetworkStringTableContainer, 10 );

META_FUNCTION( INetworkStringTableContainer, FindTable )
{
	LUA->CheckType( 1, GET_META_ID( INetworkStringTableContainer ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	INetworkStringTableContainer *pContainer = GET_META( 1, INetworkStringTableContainer );

	PUSH_META( pContainer->FindTable( LUA->GetString( 2 ) ), INetworkStringTable );

	return 1;
}

META_FUNCTION( INetworkStringTableContainer, GetTable )
{
	LUA->CheckType( 1, GET_META_ID( INetworkStringTableContainer ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	INetworkStringTableContainer *pContainer = GET_META( 1, INetworkStringTableContainer );

	PUSH_META( pContainer->GetTable( static_cast<int>( LUA->GetNumber( 2 ) ) ), INetworkStringTable );

	return 1;
}

GLBL_FUNCTION( INetworkStringTableContainer )
{
	INetworkStringTableContainer *pContainer = static_cast<INetworkStringTableContainer *>( fnEngineFactory( LUA->IsClient( ) ? INTERFACENAME_NETWORKSTRINGTABLECLIENT : INTERFACENAME_NETWORKSTRINGTABLESERVER, nullptr ) );

	PUSH_META( pContainer, INetworkStringTableContainer );

	return 1;
}