#include <netadr.hpp>
#include <netadr.h>

namespace netadr
{

static uint8_t metatype = GarrysMod::Lua::Type::NONE;
static const char *metaname = "netadr_t";

void Push( GarrysMod::Lua::ILuaBase *LUA, const netadr_t &netadr )
{
	netadr_t *adr = LUA->NewUserType<netadr_t>( metatype );
	*adr = netadr;

	LUA->PushMetaTable( metatype );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( LUA->GetState( ), -2 );
}

static netadr_t *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
{
	global::CheckType( LUA, index, metatype, metaname );
	return LUA->GetUserType<netadr_t>( index, metatype );
}

LUA_FUNCTION_STATIC( gc )
{
	LUA->SetUserType( 1, nullptr );
	return 0;
}

LUA_FUNCTION_STATIC( eq )
{
	netadr_t *adr1 = Get( LUA, 1 );
	netadr_t *adr2 = Get( LUA, 2 );

	bool baseonly = false;
	if( LUA->IsType( 3, GarrysMod::Lua::Type::BOOL ) )
		baseonly = LUA->GetBool( 3 );

	LUA->PushBool( adr1->CompareAdr( *adr2, baseonly ) );

	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	netadr_t *adr = Get( LUA, 1 );

	bool baseonly = false;
	if( LUA->IsType( 2, GarrysMod::Lua::Type::BOOL ) )
		baseonly = LUA->GetBool( 2 );

	LUA->PushString( adr->ToString( baseonly ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsLocalhost )
{
	netadr_t *adr = Get( LUA, 1 );

	LUA->PushBool( adr->IsLocalhost( ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsLoopback )
{
	netadr_t *adr = Get( LUA, 1 );

	LUA->PushBool( adr->IsLoopback( ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsReservedAdr )
{
	netadr_t *adr = Get( LUA, 1 );

	LUA->PushBool( adr->IsReservedAdr( ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsValid )
{
	netadr_t *adr = Get( LUA, 1 );

	LUA->PushBool( adr->IsValid( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetIP )
{
	netadr_t *adr = Get( LUA, 1 );

	LUA->PushString( adr->ToString( true ) );
	LUA->PushNumber( adr->GetIPHostByteOrder( ) );

	return 2;
}

LUA_FUNCTION_STATIC( GetPort )
{
	netadr_t *adr = Get( LUA, 1 );

	LUA->PushNumber( adr->GetPort( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetSteamID )
{
	netadr_t *adr = Get( LUA, 1 );

	const CSteamID &steamID = adr->GetSteamID( );
	EAccountType type = steamID.GetEAccountType( );
	if( type == k_EAccountTypeInvalid || type == k_EAccountTypeIndividual )
	{
		AccountID_t accountID = steamID.GetAccountID( );
		lua_pushfstring( LUA->GetState( ), "STEAM_0:%u:%u", accountID % 2, accountID / 2 );
	}
	else
		lua_pushfstring( LUA->GetState( ), "%llu", steamID.ConvertToUint64( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetType )
{
	netadr_t *adr = Get( LUA, 1 );

	LUA->PushNumber( adr->GetType( ) );

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

		LUA->PushCFunction( tostring );
		LUA->SetField( -2, "ToString" );

		LUA->PushCFunction( global::GetTable );
		LUA->SetField( -2, "GetTable" );

		LUA->PushCFunction( IsLocalhost );
		LUA->SetField( -2, "IsLocalhost" );

		LUA->PushCFunction( IsLoopback );
		LUA->SetField( -2, "IsLoopback" );

		LUA->PushCFunction( IsReservedAdr );
		LUA->SetField( -2, "IsReservedAdr" );

		LUA->PushCFunction( IsValid );
		LUA->SetField( -2, "IsValid" );

		LUA->PushCFunction( GetIP );
		LUA->SetField( -2, "GetIP" );

		LUA->PushCFunction( GetPort );
		LUA->SetField( -2, "GetPort" );

		LUA->PushCFunction( GetSteamID );
		LUA->SetField( -2, "GetSteamID" );

		LUA->PushCFunction( GetType );
		LUA->SetField( -2, "GetType" );

	LUA->Pop( 1 );
}

void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
{
	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, metaname );
}

}
