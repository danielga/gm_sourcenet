#include <ucharptr.hpp>
#include <cstring>

namespace UCHARPTR
{

struct Container
{
	uint8_t *data;
	int32_t bits;
	bool own;
};

uint8_t metatype = GarrysMod::Lua::Type::NONE;
const char *metaname = "UCHARPTR";

uint8_t *Push( GarrysMod::Lua::ILuaBase *LUA, int32_t bits, uint8_t *data )
{
	if( bits <= 0 )
		global::ThrowError(
			LUA,
			"invalid amount of bits for a buffer (%d is less than or equal to zero)",
			bits
		);

	bool own = false;
	if( data == nullptr )
	{
		own = true;
		data = new( std::nothrow ) uint8_t[( bits + 7 ) >> 3];
		if( data == nullptr )
			LUA->ThrowError( "failed to allocate buffer" );
	}

	Container *container = LUA->NewUserType<Container>( metatype );
	container->data = data;
	container->bits = bits;
	container->own = own;

	LUA->PushMetaTable( metatype );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( LUA->state, -2 );

	return data;
}

inline Container *GetUserData( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
{
	global::CheckType( LUA, index, metatype, metaname );
	return LUA->GetUserType<Container>( index, metatype );
}

uint8_t *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index, int32_t *bits, bool *own )
{
	Container *container = GetUserData( LUA, index );
	uint8_t *ptr = container->data;
	if( ptr == nullptr )
		global::ThrowError( LUA, "invalid %s", metaname );

	if( bits != nullptr )
		*bits = container->bits;

	if( own != nullptr )
		*own = container->own;

	return ptr;
}

uint8_t *Release( GarrysMod::Lua::ILuaBase *LUA, int32_t index, int32_t *bits )
{
	Container *container = GetUserData( LUA, index );
	uint8_t *ptr = container->data;
	if( ptr == nullptr )
		global::ThrowError( LUA, "invalid %s", metaname );

	if( bits != nullptr )
		*bits = container->bits;

	LUA->SetUserType( index, nullptr );

	return ptr;
}

LUA_FUNCTION_STATIC( gc )
{
	Container *container = GetUserData( LUA, 1 );

	if( container->own )
		delete[] container->data;

	LUA->SetUserType( 1, nullptr );

	return 0;
}

LUA_FUNCTION_STATIC( eq )
{
	uint8_t *ptr1 = Get( LUA, 1 );
	uint8_t *ptr2 = Get( LUA, 2 );

	LUA->PushBool( ptr1 == ptr2 );

	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	uint8_t *ptr = Get( LUA, 1 );

	lua_pushfstring( LUA->state, global::tostring_format, metaname, ptr );

	return 1;
}

LUA_FUNCTION_STATIC( IsValid )
{
	global::CheckType( LUA, 1, metatype, metaname );

	LUA->PushBool( GetUserData( LUA, 1 )->data != nullptr );

	return 1;
}

LUA_FUNCTION_STATIC( Size )
{
	int32_t bits = 0;
	Get( LUA, 1, &bits );

	LUA->PushNumber( bits );

	return 1;
}

LUA_FUNCTION_STATIC( Copy )
{
	int32_t bits = 0;
	uint8_t *src = Get( LUA, 1, &bits );

	uint8_t *data = Push( LUA, bits );
	memcpy( data, src, ( bits + 7 ) >> 3 );

	return 1;
}

LUA_FUNCTION_STATIC( Constructor )
{
	LUA->CheckType( 1, GarrysMod::Lua::Type::NUMBER );

	Push( LUA, static_cast<int32_t>( LUA->GetNumber( 1 ) ) * 8 );

	return 1;
}

void Initialize( GarrysMod::Lua::ILuaBase *LUA )
{
	metatype = LUA->CreateMetaTable( metaname );

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

void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
{
	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, metaname );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, metaname );
}

}
