#include <gl_ucharptr.hpp>

META_ID( UCHARPTR, 7 );

META_FUNCTION( UCHARPTR, Delete )
{
	LUA->CheckType( 1, GET_META_ID( UCHARPTR ) );

	uint8_t *ptr = GET_META( 1, uint8_t );

	delete[] ptr;

	return 0;
}

GLBL_FUNCTION( UCHARPTR )
{
	LUA->CheckType( 1, GarrysMod::Lua::Type::NUMBER );

	uint8_t *ptr = new uint8_t[static_cast<int>( LUA->GetNumber( 1 ) )];

	PUSH_META( ptr, UCHARPTR );

	return 1;
}