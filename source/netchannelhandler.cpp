#include <netchannelhandler.hpp>
#include <hooks.hpp>

namespace NetChannelHandler
{

static uint8_t metatype = GarrysMod::Lua::Type::NONE;
static const char *metaname = "INetChannelHandler";
static const char *table_name = "sourcenet_INetChannelHandler";

void Push( GarrysMod::Lua::ILuaBase *LUA, INetChannelHandler *handler )
{
	LUA->GetField( GarrysMod::Lua::INDEX_REGISTRY, table_name );
	LUA->PushUserdata( handler );
	LUA->GetTable( -2 );
	if( LUA->IsType( -1, metatype ) )
	{
		LUA->Remove( -2 );
		return;
	}

	LUA->Pop( 1 );

	LUA->PushUserType( handler, metatype );

	LUA->PushMetaTable( metatype );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( LUA->state, -2 );

	LUA->PushUserdata( handler );
	LUA->Push( -2 );
	LUA->SetTable( -4 );
	LUA->Remove( -2 );

	Hooks::HookINetChannelHandler( LUA );
}

INetChannelHandler *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
{
	global::CheckType( LUA, index, metatype, metaname );
	return LUA->GetUserType<INetChannelHandler>( index, index );
}

void Destroy( GarrysMod::Lua::ILuaBase *LUA, INetChannelHandler *handler )
{
	LUA->GetField( GarrysMod::Lua::INDEX_REGISTRY, table_name );
	LUA->PushUserdata( handler );
	LUA->PushNil( );
	LUA->SetTable( -3 );
	LUA->Pop( 1 );
}

LUA_FUNCTION_STATIC( eq )
{
	LUA->PushBool( Get( LUA, 1 ) == Get( LUA, 2 ) );
	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	lua_pushfstring( LUA->state, global::tostring_format, metaname, Get( LUA, 1 ) );
	return 1;
}

void Initialize( GarrysMod::Lua::ILuaBase *LUA )
{
	LUA->CreateTable( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, table_name );

	metatype = LUA->CreateMetaTable( metaname );

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

	LUA->Pop( 1 );
}

void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
{
	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, metaname );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, table_name );
}

}
