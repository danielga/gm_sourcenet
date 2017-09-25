#include <networkstringtable.hpp>
#include <networkstringtabledefs.h>

namespace NetworkStringTable
{

static uint8_t metatype = 0;
static const char *metaname = "IGameEvent";

void Push( GarrysMod::Lua::ILuaBase *LUA, INetworkStringTable *table )
{
	if( table == nullptr )
	{
		LUA->PushNil( );
		return;
	}

	LUA->PushUserType( table, metatype );

	LUA->PushMetaTable( metatype );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	LUA->SetFEnv( -2 );
}

INetworkStringTable *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
{
	global::CheckType( LUA, index, metatype, metaname );
	return LUA->GetUserType<INetworkStringTable>( index, metatype );
}

LUA_FUNCTION_STATIC( eq )
{
	INetworkStringTable *table1 = Get( LUA, 1 );
	INetworkStringTable *table2 = Get( LUA, 2 );

	LUA->PushBool( table1 == table2 );

	return 1;
}
LUA_FUNCTION_STATIC( tostring )
{
	INetworkStringTable *table = Get( LUA, 1 );

	LUA->PushFormattedString( global::tostring_format, metaname, table );

	return 1;
}

LUA_FUNCTION_STATIC( GetName )
{
	INetworkStringTable *table = Get( LUA, 1 );

	LUA->PushString( table->GetTableName( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetID )
{
	INetworkStringTable *table = Get( LUA, 1 );

	LUA->PushNumber( table->GetTableId( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetNumStrings )
{
	INetworkStringTable *table = Get( LUA, 1 );

	LUA->PushNumber( table->GetNumStrings( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetMaxStrings )
{
	INetworkStringTable *table = Get( LUA, 1 );

	LUA->PushNumber( table->GetMaxStrings( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetEntryBits )
{
	INetworkStringTable *table = Get( LUA, 1 );

	LUA->PushNumber( table->GetEntryBits( ) );

	return 1;
}

LUA_FUNCTION_STATIC( SetTick )
{
	INetworkStringTable *table = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	table->SetTick( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( ChangedSinceTick )
{
	INetworkStringTable *table = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushBool( table->ChangedSinceTick( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( AddString )
{
	INetworkStringTable *table = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );
	LUA->CheckType( 3, GarrysMod::Lua::Type::STRING );

	uint32_t len = 0;
	const char *UserData = nullptr;
	if( LUA->IsType( 4, GarrysMod::Lua::Type::STRING ) )
		UserData = LUA->GetString( 4, &len );

	LUA->PushNumber( table->AddString( LUA->GetBool( 1 ), LUA->GetString( 2 ), len == 0 ? -1 : len, UserData ) );

	return 1;
}

LUA_FUNCTION_STATIC( SetStringUserData )
{
	INetworkStringTable *table = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	const char *str = table->GetString( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );
	if( str != nullptr )
		LUA->PushString( str );
	else
		LUA->PushNil( );

	return 1;
}

LUA_FUNCTION_STATIC( FindStringIndex )
{
	INetworkStringTable *table = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushNumber( table->FindStringIndex( LUA->GetString( 2 ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetString )
{
	INetworkStringTable *table = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	const char *str = table->GetString( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );
	if( str != nullptr )
		LUA->PushString( str );
	else
		LUA->PushNil( );

	return 1;
}

LUA_FUNCTION_STATIC( GetStringUserData )
{
	INetworkStringTable *table = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t len = 0;
	const char *UserData = static_cast<const char *>(
		table->GetStringUserData( static_cast<int32_t>( LUA->GetNumber( 2 ) ), &len )
	);
	if( UserData != nullptr )
		LUA->PushString( UserData, len );
	else
		LUA->PushNil( );

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

		LUA->PushCFunction( GetName );
		LUA->SetField( -2, "GetName" );

		LUA->PushCFunction( GetID );
		LUA->SetField( -2, "GetID" );

		LUA->PushCFunction( GetNumStrings );
		LUA->SetField( -2, "GetNumStrings" );

		LUA->PushCFunction( GetMaxStrings );
		LUA->SetField( -2, "GetMaxStrings" );

		LUA->PushCFunction( GetEntryBits );
		LUA->SetField( -2, "GetEntryBits" );

		LUA->PushCFunction( SetTick );
		LUA->SetField( -2, "SetTick" );

		LUA->PushCFunction( ChangedSinceTick );
		LUA->SetField( -2, "ChangedSinceTick" );

		LUA->PushCFunction( AddString );
		LUA->SetField( -2, "AddString" );

		LUA->PushCFunction( SetStringUserData );
		LUA->SetField( -2, "SetStringUserData" );

		LUA->PushCFunction( FindStringIndex );
		LUA->SetField( -2, "FindStringIndex" );

		LUA->PushCFunction( GetString );
		LUA->SetField( -2, "GetString" );

		LUA->PushCFunction( GetStringUserData );
		LUA->SetField( -2, "GetStringUserData" );

	LUA->Pop( 1 );
}

void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
{
	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, metaname );
}

}
