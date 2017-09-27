#include <filehandle.hpp>

namespace FileHandle
{
	int32_t metatype = 0;
	const char *metaname = "FileHandle_t";

	void Push( GarrysMod::Lua::ILuaBase *LUA, FileHandle_t file )
	{
		if( file == nullptr )
		{
			LUA->PushNil( );
			return;
		}

		LUA->PushUserType( file, metatype );

		LUA->PushMetaTable( metatype );
		LUA->SetMetaTable( -2 );

		LUA->CreateTable( );
		LUA->SetFEnv( -2 );
	}

	FileHandle_t Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
	{
		global::CheckType( LUA, index, metatype, metaname );
		return LUA->GetUserType<FileHandle_t>( index, metatype );
	}

	LUA_FUNCTION_STATIC( eq )
	{
		LUA->PushBool( Get( LUA, 1 ) == Get( LUA, 2 ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( tostring )
	{
		LUA->PushFormattedString( global::tostring_format, metaname, Get( LUA, 1 ) );
		return 1;
	}

	void Initialize( GarrysMod::Lua::ILuaBase *LUA )
	{
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
	}
}
