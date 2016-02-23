#include <ucharptr.hpp>
#include <cstring>

namespace UCHARPTR
{

struct UserData
{
	uint8_t *data;
	uint8_t type;
	int32_t bits;
	bool own;
};

const uint8_t metatype = global::metabase + 7;
const char *metaname = "UCHARPTR";

uint8_t *Push( lua_State *state, int32_t bits, uint8_t *data )
{
	if( bits < 0 )
		global::ThrowError(
			state,
			"invalid amount of bits for a buffer (%d is less than zero)",
			bits
		);

	bool own = false;
	if( data == nullptr )
	{
		own = true;
		data = new( std::nothrow ) uint8_t[bits == 0 ? 1 : ( bits + 7 ) >> 3];
		if( data == nullptr )
			LUA->ThrowError( "failed to allocate buffer" );
	}

	UserData *udata = static_cast<UserData *>( LUA->NewUserdata( sizeof( UserData ) ) );
	udata->type = metatype;
	udata->data = data;
	udata->bits = bits;
	udata->own = own;

	LUA->CreateMetaTableType( metaname, metatype );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( state, -2 );

	return data;
}

inline UserData *GetUserData( lua_State *state, int32_t index )
{
	global::CheckType( state, index, metatype, metaname );
	return static_cast<UserData *>( LUA->GetUserdata( index ) );
}

uint8_t *Get( lua_State *state, int32_t index, int32_t *bits, bool *own )
{
	UserData *udata = GetUserData( state, index );
	uint8_t *ptr = udata->data;
	if( ptr == nullptr )
		global::ThrowError( state, "invalid %s", metaname );

	if( bits != nullptr )
		*bits = udata->bits;

	if( own != nullptr )
		*own = udata->own;

	return ptr;
}

uint8_t *Release( lua_State *state, int32_t index, int32_t *bits )
{
	UserData *udata = GetUserData( state, index );
	uint8_t *ptr = udata->data;
	if( ptr == nullptr )
		global::ThrowError( state, "invalid %s", metaname );

	if( bits != nullptr )
		*bits = udata->bits;

	udata->data = nullptr;
	udata->bits = 0;
	udata->own = false;

	return ptr;
}

LUA_FUNCTION_STATIC( gc )
{
	UserData *udata = GetUserData( state, 1 );

	if( udata->own )
		delete[] udata->data;

	udata->data = nullptr;
	udata->bits = 0;
	udata->own = false;

	return 0;
}

LUA_FUNCTION_STATIC( eq )
{
	uint8_t *ptr1 = Get( state, 1 );
	uint8_t *ptr2 = Get( state, 2 );

	LUA->PushBool( ptr1 == ptr2 );

	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	uint8_t *ptr = Get( state, 1 );

	lua_pushfstring( state, global::tostring_format, metaname, ptr );

	return 1;
}

LUA_FUNCTION_STATIC( IsValid )
{
	global::CheckType( state, 1, metatype, metaname );

	LUA->PushBool( GetUserData( state, 1 )->data != nullptr );

	return 1;
}

LUA_FUNCTION_STATIC( Size )
{
	int32_t bits = 0;
	Get( state, 1, &bits );

	LUA->PushNumber( bits );

	return 1;
}

LUA_FUNCTION_STATIC( Copy )
{
	int32_t bits = 0;
	uint8_t *src = Get( state, 1, &bits );

	uint8_t *data = Push( state, bits );
	memcpy( data, src, ( bits + 7 ) >> 3 );

	return 1;
}

LUA_FUNCTION_STATIC( Constructor )
{
	LUA->CheckType( 1, GarrysMod::Lua::Type::NUMBER );

	Push( state, static_cast<int32_t>( LUA->GetNumber( 1 ) ) * 8 );

	return 1;
}

void Initialize( lua_State *state )
{
	LUA->CreateMetaTableType( metaname, metatype );

		LUA->PushCFunction( gc );
		LUA->SetField( -2, "__gc" );

		LUA->PushCFunction( eq );
		LUA->SetField( -2, "__eq" );

		LUA->PushCFunction( tostring );
		LUA->SetField( -2, "__tostring" );

		LUA->PushCFunction( global::index );
		LUA->SetField( -2, "__index" );

		LUA->PushCFunction( global::newindex );
		LUA->SetField( -2, "__newindex" );

		LUA->PushCFunction( global::GetTable );
		LUA->SetField( -2, "GetTable" );

		LUA->PushCFunction( IsValid );
		LUA->SetField( -2, "IsValid" );

		LUA->PushCFunction( Size );
		LUA->SetField( -2, "Size" );

		LUA->PushCFunction( Copy );
		LUA->SetField( -2, "Copy" );

	LUA->Pop( 1 );

	LUA->PushCFunction( Constructor );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, metaname );
}

void Deinitialize( lua_State *state )
{
	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, metaname );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, metaname );
}

}
