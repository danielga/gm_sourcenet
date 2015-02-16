#include <gl_subchannel_t.hpp>
#include <gl_cnetchan.hpp>
#include <net.h>

struct subchannel_userdata
{
	subchannel_t *subchan;
	uint8_t type;
	CNetChan *netchan;
};

static bool IsValid_subchannel( subchannel_t *subchan, CNetChan *netchan )
{
	return subchan != nullptr && ( netchan == nullptr || IsValid_CNetChan( netchan ) );
}

void Push_subchannel( lua_State *state, subchannel_t *subchan, CNetChan *netchan )
{
	subchannel_userdata *userdata = static_cast<subchannel_userdata *>(
		LUA->NewUserdata( sizeof( subchannel_userdata ) )
	);
	userdata->type = GET_META_ID( subchannel_t );
	userdata->subchan = subchan;

	LUA->CreateMetaTableType( GET_META_NAME( subchannel_t ), GET_META_ID( subchannel_t ) );
	LUA->SetMetaTable( -2 );
}

static subchannel_t *Get_subchannel( lua_State *state, int32_t index )
{
	CheckType( state, index, GET_META_ID( subchannel_t ), GET_META_NAME( subchannel_t ) );

	subchannel_userdata *userdata = static_cast<subchannel_userdata *>(
		LUA->GetUserdata( index )
	);
	if( !IsValid_subchannel( userdata->subchan, userdata->netchan ) )
		LUA->ThrowError( "invalid subchannel_t" );

	return userdata->subchan;
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

META_ID( subchannel_t, 4 );

META_FUNCTION( subchannel_t, __eq )
{
	subchannel_t *subchan1 = Get_subchannel( state, 1 );
	subchannel_t *subchan2 = Get_subchannel( state, 2 );

	LUA->PushBool( subchan1 == subchan2 );

	return 1;
}

META_FUNCTION( subchannel_t, __tostring )
{
	subchannel_t *subchan = Get_subchannel( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", GET_META_NAME( subchannel_t ), subchan );

	return 1;
}

META_FUNCTION( subchannel_t, IsValid )
{
	CheckType( state, 1, GET_META_ID( subchannel_t ), GET_META_NAME( subchannel_t ) );

	subchannel_userdata *userdata = static_cast<subchannel_userdata *>( LUA->GetUserdata( 1 ) );
	LUA->PushBool( IsValid_subchannel( userdata->subchan, userdata->netchan ) );

	return 1;
}

META_FUNCTION( subchannel_t, GetFragmentOffset )
{
	subchannel_t *subchan = Get_subchannel( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	int32_t ret = VerifyStream( state, stream );
	if( ret != 0 )
		return ret;

	LUA->PushNumber( subchan->frag_ofs[stream] );

	return 1;
}

META_FUNCTION( subchannel_t, SetFragmentOffset )
{
	subchannel_t *subchan = Get_subchannel( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	int32_t ret = VerifyStream( state, stream );
	if( ret != 0 )
		return ret;

	subchan->frag_ofs[stream] = static_cast<int32_t>( LUA->GetNumber( 3 ) );

	return 0;
}

META_FUNCTION( subchannel_t, GetFragmentNumber )
{
	subchannel_t *subchan = Get_subchannel( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	int32_t ret = VerifyStream( state, stream );
	if( ret != 0 )
		return ret;

	LUA->PushNumber( subchan->frag_num[stream] );

	return 1;
}

META_FUNCTION( subchannel_t, SetFragmentNumber )
{
	subchannel_t *subchan = Get_subchannel( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	int32_t ret = VerifyStream( state, stream );
	if( ret != 0 )
		return ret;

	subchan->frag_num[stream] = static_cast<int32_t>( LUA->GetNumber( 3 ) );

	return 0;
}

META_FUNCTION( subchannel_t, GetSequence )
{
	subchannel_t *subchan = Get_subchannel( state, 1 );

	LUA->PushNumber( subchan->sequence );

	return 1;
}

META_FUNCTION( subchannel_t, SetSequence )
{
	subchannel_t *subchan = Get_subchannel( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	subchan->sequence = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	return 1;
}

META_FUNCTION( subchannel_t, GetState )
{
	subchannel_t *subchan = Get_subchannel( state, 1 );

	LUA->PushNumber( subchan->state );

	return 1;
}

META_FUNCTION( subchannel_t, SetState )
{
	subchannel_t *subchan = Get_subchannel( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	subchan->state = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	return 1;
}

META_FUNCTION( subchannel_t, GetIndex )
{
	subchannel_t *subchan = Get_subchannel( state, 1 );

	LUA->PushNumber( subchan->index );

	return 1;
}

META_FUNCTION( subchannel_t, SetIndex )
{
	subchannel_t *subchan = Get_subchannel( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	subchan->index = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	return 1;
}