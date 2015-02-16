#include <gl_ucharptr.hpp>
#include <GarrysMod/Lua/LuaInterface.h>

struct UCHARPTR_userdata
{
	UCHARPTR data;
	uint8_t type;
	int32_t bits;
	bool own;
};

UCHARPTR Push_UCHARPTR( lua_State *state, int32_t bits, UCHARPTR data )
{
	if( bits < 0 )
		static_cast<GarrysMod::Lua::ILuaInterface *>( LUA )->ErrorFromLua(
			"invalid amount of bits for a buffer (requested %d, less than zero)", bits
		);

	bool own = false;
	if( data == nullptr )
	{
		own = true;
		data = new( std::nothrow ) uint8_t[bits == 0 ? 1 : ( bits + 7 ) >> 3];
		if( data == nullptr )
			LUA->ThrowError( "failed to allocate buffer" );
	}

	UCHARPTR_userdata *userdata = static_cast<UCHARPTR_userdata *>(
		LUA->NewUserdata( sizeof( UCHARPTR_userdata ) )
	);
	userdata->type = GET_META_ID( UCHARPTR );
	userdata->data = data;
	userdata->bits = bits;
	userdata->own = own;

	LUA->CreateMetaTableType( GET_META_NAME( UCHARPTR ), GET_META_ID( UCHARPTR ) );
	LUA->SetMetaTable( -2 );

	return data;
}

UCHARPTR Get_UCHARPTR( lua_State *state, int32_t index, int32_t *bits, bool cleanup, bool *own )
{
	LUA->CheckType( index, GET_META_ID( UCHARPTR ) );

	UCHARPTR_userdata *userdata = static_cast<UCHARPTR_userdata *>( LUA->GetUserdata( index ) );

	UCHARPTR ptr = userdata->data;
	if( ptr == nullptr )
		LUA->ThrowError( "invalid UCHARPTR" );

	if( bits != nullptr )
		*bits = userdata->bits;

	if( own != nullptr )
		*own = userdata->own;

	if( cleanup )
	{
		userdata->data = nullptr;
		userdata->bits = 0;
		userdata->own = false;
	}

	return ptr;
}

META_ID( UCHARPTR, 7 );

META_FUNCTION( UCHARPTR, __gc )
{
	bool own = false;
	UCHARPTR data = Get_UCHARPTR( state, 1, nullptr, true, &own );
	if( !own )
		return 0;

	delete[] data;

	return 0;
}

META_FUNCTION( UCHARPTR, __eq )
{
	UCHARPTR ptr1 = Get_UCHARPTR( state, 1 );
	UCHARPTR ptr2 = Get_UCHARPTR( state, 2 );

	LUA->PushBool( ptr1 == ptr2 );

	return 1;
}

META_FUNCTION( UCHARPTR, __tostring )
{
	UCHARPTR ptr = Get_UCHARPTR( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", GET_META_NAME( UCHARPTR ), ptr );

	return 1;
}

META_FUNCTION( UCHARPTR, IsValid )
{
	LUA->CheckType( 1, GET_META_ID( UCHARPTR ) );

	LUA->PushBool( static_cast<UCHARPTR_userdata *>( LUA->GetUserdata( 1 ) )->data != nullptr );

	return 1;
}

META_FUNCTION( UCHARPTR, Size )
{
	int32_t bits = 0;
	Get_UCHARPTR( state, 1, &bits );

	LUA->PushNumber( bits );

	return 1;
}

GLBL_FUNCTION( UCHARPTR )
{
	LUA->CheckType( 1, GarrysMod::Lua::Type::NUMBER );

	Push_UCHARPTR( state, static_cast<int32_t>( LUA->GetNumber( 1 ) ) * 8 );

	return 1;
}