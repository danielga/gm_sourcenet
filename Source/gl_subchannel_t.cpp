#include <gl_subchannel_t.hpp>

#include <net.h>

META_ID( subchannel_t, 4 );

META_FUNCTION( subchannel_t, GetFragmentOffset )
{
	LUA->CheckType( 1, GET_META_ID( subchannel_t ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	subchannel_t *subchan = GET_META( 1, subchannel_t );

	int stream = static_cast<int>( LUA->GetNumber( 2 ) );

	VerifyStream( stream );

	LUA->PushNumber( subchan->frag_ofs[stream] );

	return 1;
}

META_FUNCTION( subchannel_t, SetFragmentOffset )
{
	LUA->CheckType( 1, GET_META_ID( subchannel_t ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	subchannel_t *subchan = GET_META( 1, subchannel_t );

	int stream = static_cast<int>( LUA->GetNumber( 2 ) );

	VerifyStream( stream );

	subchan->frag_ofs[stream] = static_cast<int>( LUA->GetNumber( 3 ) );

	return 0;
}

META_FUNCTION( subchannel_t, GetFragmentNumber )
{
	LUA->CheckType( 1, GET_META_ID( subchannel_t ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	subchannel_t *subchan = GET_META( 1, subchannel_t );

	int stream = static_cast<int>( LUA->GetNumber( 2 ) );

	VerifyStream( stream );

	LUA->PushNumber( subchan->frag_num[stream] );

	return 1;
}

META_FUNCTION( subchannel_t, SetFragmentNumber )
{
	LUA->CheckType( 1, GET_META_ID( subchannel_t ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	subchannel_t *subchan = GET_META( 1, subchannel_t );

	int stream = static_cast<int>( LUA->GetNumber( 2 ) );

	VerifyStream( stream );

	subchan->frag_num[stream] = static_cast<int>( LUA->GetNumber( 3 ) );

	return 0;
}

META_FUNCTION( subchannel_t, GetSequence )
{
	LUA->CheckType( 1, GET_META_ID( subchannel_t ) );

	subchannel_t *subchan = GET_META( 1, subchannel_t );

	LUA->PushNumber( subchan->sequence );

	return 1;
}

META_FUNCTION( subchannel_t, SetSequence )
{
	LUA->CheckType( 1, GET_META_ID( subchannel_t ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	subchannel_t *subchan = GET_META( 1, subchannel_t );

	subchan->sequence = static_cast<int>( LUA->GetNumber( 2 ) );

	return 1;
}

META_FUNCTION( subchannel_t, GetState )
{
	LUA->CheckType( 1, GET_META_ID( subchannel_t ) );

	subchannel_t *subchan = GET_META( 1, subchannel_t );

	LUA->PushNumber( subchan->state );

	return 1;
}

META_FUNCTION( subchannel_t, SetState )
{
	LUA->CheckType( 1, GET_META_ID( subchannel_t ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	subchannel_t *subchan = GET_META( 1, subchannel_t );

	subchan->state = static_cast<int>( LUA->GetNumber( 2 ) );

	return 1;
}

META_FUNCTION( subchannel_t, GetIndex )
{
	LUA->CheckType( 1, GET_META_ID( subchannel_t ) );

	subchannel_t *subchan = GET_META( 1, subchannel_t );

	LUA->PushNumber( subchan->index );

	return 1;
}

META_FUNCTION( subchannel_t, SetIndex )
{
	LUA->CheckType( 1, GET_META_ID( subchannel_t ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	subchannel_t *subchan = GET_META( 1, subchannel_t );

	subchan->index = static_cast<int>( LUA->GetNumber( 2 ) );

	return 1;
}