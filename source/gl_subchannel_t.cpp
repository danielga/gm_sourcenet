#include <gl_subchannel_t.hpp>
#include <gl_cnetchan.hpp>
#include <net.h>

namespace subchannel
{

struct userdata
{
	subchannel_t *subchan;
	uint8_t type;
	CNetChan *netchan;
};

static const uint8_t metaid = Global::metabase + 4;
static const char *metaname = "subchannel_t";

static bool IsValid( subchannel_t *subchan, CNetChan *netchan )
{
	return subchan != nullptr && ( netchan == nullptr || NetChannel::IsValid( netchan ) );
}

void Push( lua_State *state, subchannel_t *subchan, CNetChan *netchan )
{
	userdata *udata = static_cast<userdata *>( LUA->NewUserdata( sizeof( userdata ) ) );
	udata->type = metaid;
	udata->subchan = subchan;

	LUA->CreateMetaTableType( metaname, metaid );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( state, -2 );
}

static subchannel_t *Get( lua_State *state, int32_t index )
{
	Global::CheckType( state, index, metaid, metaname );

	userdata *udata = static_cast<userdata *>( LUA->GetUserdata( index ) );
	if( !IsValid( udata->subchan, udata->netchan ) )
		static_cast<GarrysMod::Lua::ILuaInterface *>( LUA )->ErrorFromLua( "invalid %s", metaname );

	return udata->subchan;
}

inline int32_t VerifyStream( lua_State *state, int32_t stream )
{
	if( stream < 0 || stream >= MAX_STREAMS )
	{
		LUA->PushNil( );
		return 1;
	}

	return 0;
}

LUA_FUNCTION_STATIC( eq )
{
	subchannel_t *subchan1 = Get( state, 1 );
	subchannel_t *subchan2 = Get( state, 2 );

	LUA->PushBool( subchan1 == subchan2 );

	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	subchannel_t *subchan = Get( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", metaname, subchan );

	return 1;
}

LUA_FUNCTION_STATIC( IsValid )
{
	Global::CheckType( state, 1, metaid, metaname );

	userdata *udata = static_cast<userdata *>( LUA->GetUserdata( 1 ) );
	LUA->PushBool( IsValid( udata->subchan, udata->netchan ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetFragmentOffset )
{
	subchannel_t *subchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	int32_t ret = VerifyStream( state, stream );
	if( ret != 0 )
		return ret;

	LUA->PushNumber( subchan->frag_ofs[stream] );

	return 1;
}

LUA_FUNCTION_STATIC( SetFragmentOffset )
{
	subchannel_t *subchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	int32_t ret = VerifyStream( state, stream );
	if( ret != 0 )
		return ret;

	subchan->frag_ofs[stream] = static_cast<int32_t>( LUA->GetNumber( 3 ) );

	return 0;
}

LUA_FUNCTION_STATIC( GetFragmentNumber )
{
	subchannel_t *subchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	int32_t ret = VerifyStream( state, stream );
	if( ret != 0 )
		return ret;

	LUA->PushNumber( subchan->frag_num[stream] );

	return 1;
}

LUA_FUNCTION_STATIC( SetFragmentNumber )
{
	subchannel_t *subchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	int32_t ret = VerifyStream( state, stream );
	if( ret != 0 )
		return ret;

	subchan->frag_num[stream] = static_cast<int32_t>( LUA->GetNumber( 3 ) );

	return 0;
}

LUA_FUNCTION_STATIC( GetSequence )
{
	subchannel_t *subchan = Get( state, 1 );

	LUA->PushNumber( subchan->sequence );

	return 1;
}

LUA_FUNCTION_STATIC( SetSequence )
{
	subchannel_t *subchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	subchan->sequence = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetState )
{
	subchannel_t *subchan = Get( state, 1 );

	LUA->PushNumber( subchan->state );

	return 1;
}

LUA_FUNCTION_STATIC( SetState )
{
	subchannel_t *subchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	subchan->state = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetIndex )
{
	subchannel_t *subchan = Get( state, 1 );

	LUA->PushNumber( subchan->index );

	return 1;
}

LUA_FUNCTION_STATIC( SetIndex )
{
	subchannel_t *subchan = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	subchan->index = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	return 1;
}

void Initialize( lua_State *state )
{
	LUA->CreateMetaTableType( metaname, metaid );

		LUA->PushCFunction( eq );
		LUA->SetField( -2, "__eq" );

		LUA->PushCFunction( tostring );
		LUA->SetField( -2, "__tostring" );

		LUA->PushCFunction( Global::index );
		LUA->SetField( -2, "__index" );

		LUA->PushCFunction( Global::newindex );
		LUA->SetField( -2, "__newindex" );

		LUA->PushCFunction( IsValid );
		LUA->SetField( -2, "IsValid" );

		LUA->PushCFunction( GetFragmentOffset );
		LUA->SetField( -2, "GetFragmentOffset" );

		LUA->PushCFunction( SetFragmentOffset );
		LUA->SetField( -2, "SetFragmentOffset" );

		LUA->PushCFunction( GetFragmentNumber );
		LUA->SetField( -2, "GetFragmentNumber" );

		LUA->PushCFunction( SetFragmentNumber );
		LUA->SetField( -2, "SetFragmentNumber" );

		LUA->PushCFunction( GetSequence );
		LUA->SetField( -2, "GetSequence" );

		LUA->PushCFunction( SetSequence );
		LUA->SetField( -2, "SetSequence" );

		LUA->PushCFunction( GetState );
		LUA->SetField( -2, "GetState" );

		LUA->PushCFunction( SetState );
		LUA->SetField( -2, "SetState" );

		LUA->PushCFunction( GetIndex );
		LUA->SetField( -2, "GetIndex" );

		LUA->PushCFunction( SetIndex );
		LUA->SetField( -2, "SetIndex" );

	LUA->Pop( 1 );
}

void Deinitialize( lua_State *state )
{
	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_REG );

		LUA->PushNil( );
		LUA->SetField( -2, metaname );

	LUA->Pop( 1 );
}

}