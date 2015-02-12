#include <gl_ucharptr.hpp>

struct UCHARPTR_userdata
{
	UCHARPTR data;
	uint8_t type;
	int32_t bits;
};

void Push_UCHARPTR( lua_State *state, UCHARPTR data, int32_t bits )
{
	if( bits <= 0 )
		LUA->ThrowError( "invalid amount of bits for a buffer (less or equal to zero)" );

	UCHARPTR_userdata *userdata = static_cast<UCHARPTR_userdata *>( LUA->NewUserdata( sizeof( UCHARPTR_userdata ) ) );
	userdata->type = GET_META_ID( UCHARPTR );
	userdata->data = data;
	userdata->bits = bits;

	LUA->CreateMetaTableType( GET_META_NAME( UCHARPTR ), GET_META_ID( UCHARPTR ) );
	LUA->SetMetaTable( -2 );
}

UCHARPTR Get_UCHARPTR( lua_State *state, int32_t index, int32_t *bits, bool cleanup )
{
	LUA->CheckType( index, GET_META_ID( UCHARPTR ) );

	UCHARPTR_userdata *userdata = static_cast<UCHARPTR_userdata *>( LUA->GetUserdata( index ) );

	UCHARPTR ptr = userdata->data;
	if( ptr == nullptr )
		LUA->ThrowError( "invalid UCHARPTR" );

	if( bits != nullptr )
		*bits = userdata->bits;

	if( cleanup )
	{
		userdata->data = nullptr;
		userdata->bits = 0;
	}

	return ptr;
}

META_ID( UCHARPTR, 7 );

META_FUNCTION( UCHARPTR, __gc )
{
	delete[] Get_UCHARPTR( state, 1, nullptr, true );

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

	int32_t bytes = static_cast<int32_t>( LUA->GetNumber( 1 ) );
	if( bytes <= 0 )
		LUA->ThrowError( "invalid amount of bytes for a buffer (less or equal to zero)" );

	uint8_t *ptr = new( std::nothrow ) uint8_t[bytes];
	if( ptr == nullptr )
		LUA->ThrowError( "failed to allocate buffer" );

	Push_UCHARPTR( state, ptr, bytes * 8 );

	return 1;
}