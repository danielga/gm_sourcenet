#include <networkstringtablecontainer.hpp>
#include <GarrysMod/Lua/AutoReference.h>
#include <networkstringtable.hpp>
#include <protocol.hpp>
#include <networkstringtabledefs.h>

namespace NetworkStringTableContainer
{
	static int32_t metatype = 0;
	static const char *metaname = "INetworkStringTableContainer";

	static GarrysMod::Lua::AutoReference container_ref;

	static INetworkStringTableContainer *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
	{
		global::CheckType( LUA, index, metatype, metaname );
		return LUA->GetUserType<INetworkStringTableContainer>( index, metatype );
	}

	LUA_FUNCTION_STATIC( eq )
	{
		global::CheckType( LUA, 1, metatype, metaname );
		global::CheckType( LUA, 2, metatype, metaname );

		LUA->PushBool( true );
		return 1;
	}

	LUA_FUNCTION_STATIC( tostring )
	{
		LUA->PushFormattedString( global::tostring_format, metaname, Get( LUA, 1 ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( GetNumTables )
	{
		LUA->PushNumber( Get( LUA, 1 )->GetNumTables( ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( FindTable )
	{
		NetworkStringTable::Push( LUA, Get( LUA, 1 )->FindTable( LUA->CheckString( 2 ) ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( GetTable )
	{
		NetworkStringTable::Push(
			LUA,
			Get( LUA, 1 )->GetTable( static_cast<int32_t>( LUA->CheckNumber( 2 ) ) )
		);
		return 1;
	}

	LUA_FUNCTION_STATIC( Constructor )
	{
		(void)LUA;
		container_ref.Push( );
		return 1;
	}

	void Initialize( GarrysMod::Lua::ILuaBase *LUA )
	{

#if defined SOURCENET_SERVER

		INetworkStringTableContainer *container = global::engine_loader.GetInterface<INetworkStringTableContainer>(
			INTERFACENAME_NETWORKSTRINGTABLESERVER
			);
		if( container == nullptr )
			LUA->ThrowError( "failed to obtain INetworkStringTableContainer" );

#elif defined SOURCENET_CLIENT

		INetworkStringTableContainer *container = global::engine_loader.GetInterface<INetworkStringTableContainer>(
			INTERFACENAME_NETWORKSTRINGTABLECLIENT
			);
		if( container == nullptr )
			LUA->ThrowError( "failed to obtain INetworkStringTableContainer" );

#endif



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

		LUA->PushCFunction( GetNumTables );
		LUA->SetField( -2, "GetNumTables" );

		LUA->PushCFunction( FindTable );
		LUA->SetField( -2, "FindTable" );

		LUA->PushCFunction( GetTable );
		LUA->SetField( -2, "GetTable" );

		LUA->Pop( 1 );



		LUA->PushUserType( container, metatype );

		LUA->PushMetaTable( metatype );
		LUA->SetMetaTable( -2 );

		LUA->CreateTable( );
		LUA->SetFEnv( -2 );

		container_ref.Setup( LUA );
		container_ref.Create( );



		LUA->PushString( INSTANCE_BASELINE_TABLENAME );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "INSTANCE_BASELINE_TABLENAME" );

		LUA->PushString( LIGHT_STYLES_TABLENAME );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "LIGHT_STYLES_TABLENAME" );

		LUA->PushString( USER_INFO_TABLENAME );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "USER_INFO_TABLENAME" );

		LUA->PushString( SERVER_STARTUP_DATA_TABLENAME );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SERVER_STARTUP_DATA_TABLENAME" );



		LUA->PushCFunction( Constructor );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, metaname );
	}

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		LUA->PushNil( );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, metaname );



		LUA->PushNil( );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "INSTANCE_BASELINE_TABLENAME" );

		LUA->PushNil( );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "LIGHT_STYLES_TABLENAME" );

		LUA->PushNil( );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "USER_INFO_TABLENAME" );

		LUA->PushNil( );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SERVER_STARTUP_DATA_TABLENAME" );



		LUA->PushNil( );
		LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, metaname );



		container_ref.Free( );
	}
}
