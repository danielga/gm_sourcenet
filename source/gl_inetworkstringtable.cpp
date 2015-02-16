#include <gl_inetworkstringtable.hpp>
#include <networkstringtabledefs.h>

struct INetworkStringTable_userdata
{
	INetworkStringTable *table;
	uint8_t type;
};

void Push_INetworkStringTable( lua_State *state, INetworkStringTable *table )
{
	INetworkStringTable_userdata *userdata = static_cast<INetworkStringTable_userdata *>(
		LUA->NewUserdata( sizeof( INetworkStringTable_userdata ) )
	);
	userdata->type = GET_META_ID( INetworkStringTable );
	userdata->table = table;

	LUA->CreateMetaTableType(
		GET_META_NAME( INetworkStringTable ),
		GET_META_ID( INetworkStringTable )
	);
	LUA->SetMetaTable( -2 );
}

INetworkStringTable *Get_INetworkStringTable( lua_State *state, int32_t index )
{
	CheckType(
		state,
		index,
		GET_META_ID( INetworkStringTable ),
		GET_META_NAME( INetworkStringTable )
	);
	return static_cast<INetworkStringTable_userdata *>( LUA->GetUserdata( index ) )->table;
}

META_ID( INetworkStringTable, 11 );

META_FUNCTION( INetworkStringTable, __eq )
{
	INetworkStringTable *table1 = Get_INetworkStringTable( state, 1 );
	INetworkStringTable *table2 = Get_INetworkStringTable( state, 2 );

	LUA->PushBool( table1 == table2 );

	return 1;
}

META_FUNCTION( INetworkStringTable, __tostring )
{
	INetworkStringTable *table = Get_INetworkStringTable( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", GET_META_NAME( INetworkStringTable ), table );

	return 1;
}

META_FUNCTION( INetworkStringTable, FindStringIndex )
{
	INetworkStringTable *table = Get_INetworkStringTable( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushNumber( table->FindStringIndex( LUA->GetString( 2 ) ) );

	return 1;
}

META_FUNCTION( INetworkStringTable, GetString )
{
	INetworkStringTable *table = Get_INetworkStringTable( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	const char *str = table->GetString( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );
	if( str != nullptr )
		LUA->PushString( str );
	else
		LUA->PushNil( );

	return 1;
}