#include <gameevent.hpp>
#include <igameevents.h>
#include <gameeventmanager.hpp>

namespace GameEvent
{

struct Container
{
	IGameEvent *event;
	IGameEventManager2 *manager;
};

static uint8_t metatype = GarrysMod::Lua::Type::NONE;
static const char *metaname = "IGameEvent";

void Push( GarrysMod::Lua::ILuaBase *LUA, IGameEvent *event, IGameEventManager2 *manager )
{
	if( event == nullptr )
	{
		LUA->PushNil( );
		return;
	}

	Container *udata = LUA->NewUserType<Container>( metatype );
	udata->event = event;
	udata->manager = manager;

	LUA->PushMetaTable( metatype );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	LUA->SetFEnv( -2 );
}

inline Container *GetUserData( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
{
	global::CheckType( LUA, index, metatype, metaname );
	return LUA->GetUserType<Container>( index, metatype );
}

IGameEvent *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index, IGameEventManager2 **manager )
{
	Container *udata = GetUserData( LUA, index );
	if( udata == nullptr )
		LUA->FormattedError( "invalid %s", metaname );

	if( manager != nullptr )
		*manager = udata->manager;

	return udata->event;
}

LUA_FUNCTION_STATIC( gc )
{
	Container *udata = GetUserData( LUA, 1 );

	if( udata->manager != nullptr )
		udata->manager->FreeEvent( udata->event );

	LUA->SetUserType( 1, nullptr );

	return 0;
}

LUA_FUNCTION_STATIC( eq )
{
	IGameEvent *event1 = Get( LUA, 1 );
	IGameEvent *event2 = Get( LUA, 2 );

	LUA->PushBool( event1 == event2 );

	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	IGameEvent *event = Get( LUA, 1 );

	LUA->PushFormattedString( global::tostring_format, metaname, event );

	return 1;
}

LUA_FUNCTION_STATIC( GetName )
{
	IGameEvent *event = Get( LUA, 1 );

	LUA->PushString( event->GetName( ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsReliable )
{
	IGameEvent *event = Get( LUA, 1 );

	LUA->PushBool( event->IsReliable( ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsLocal )
{
	IGameEvent *event = Get( LUA, 1 );

	LUA->PushBool( event->IsLocal( ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsEmpty )
{
	IGameEvent *event = Get( LUA, 1 );

	LUA->PushBool( event->IsEmpty( LUA->GetString( 2 ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetBool )
{
	IGameEvent *event = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushBool( event->GetBool( LUA->GetString( 2 ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetInt )
{
	IGameEvent *event = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushNumber( event->GetInt( LUA->GetString( 2 ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetFloat )
{
	IGameEvent *event = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushNumber( event->GetFloat( LUA->GetString( 2 ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetString )
{
	IGameEvent *event = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushString( event->GetString( LUA->GetString( 2 ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( SetBool )
{
	IGameEvent *event = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::BOOL );

	event->SetBool( LUA->GetString( 2 ), LUA->GetBool( 3 ) );

	return 0;
}

LUA_FUNCTION_STATIC( SetInt )
{
	IGameEvent *event = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	event->SetInt( LUA->GetString( 2 ), static_cast<int32_t>( LUA->GetNumber( 3 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( SetFloat )
{
	IGameEvent *event = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	event->SetFloat( LUA->GetString( 2 ), LUA->GetNumber( 3 ) );

	return 0;
}

LUA_FUNCTION_STATIC( SetString )
{
	IGameEvent *event = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );
	LUA->CheckType( 3, GarrysMod::Lua::Type::STRING );

	event->SetString( LUA->GetString( 2 ), LUA->GetString( 3 ) );

	return 0;
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

		LUA->PushCFunction( GetName );
		LUA->SetField( -2, "GetName" );

		LUA->PushCFunction( IsReliable );
		LUA->SetField( -2, "IsReliable" );

		LUA->PushCFunction( IsLocal );
		LUA->SetField( -2, "IsLocal" );

		LUA->PushCFunction( IsEmpty );
		LUA->SetField( -2, "IsEmpty" );

		LUA->PushCFunction( GetBool );
		LUA->SetField( -2, "GetBool" );

		LUA->PushCFunction( GetInt );
		LUA->SetField( -2, "GetInt" );

		LUA->PushCFunction( GetFloat );
		LUA->SetField( -2, "GetFloat" );

		LUA->PushCFunction( GetString );
		LUA->SetField( -2, "GetString" );

		LUA->PushCFunction( SetBool );
		LUA->SetField( -2, "SetBool" );

		LUA->PushCFunction( SetInt );
		LUA->SetField( -2, "SetInt" );

		LUA->PushCFunction( SetFloat );
		LUA->SetField( -2, "SetFloat" );

		LUA->PushCFunction( SetString );
		LUA->SetField( -2, "SetString" );

	LUA->Pop( 1 );
}

void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
{
	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, metaname );
}

}
