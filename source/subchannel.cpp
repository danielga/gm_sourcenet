#include <subchannel.hpp>
#include <netchannel.hpp>
#include <net.hpp>

namespace subchannel
{
	struct Container
	{
		subchannel_t *subchan;
		CNetChan *netchan;
	};

	static int32_t metatype = 0;
	static const char *metaname = "subchannel_t";

	static bool IsValid( subchannel_t *subchan, CNetChan *netchan )
	{
		return subchan != nullptr && NetChannel::IsValid( netchan );
	}

	void Push( GarrysMod::Lua::ILuaBase *LUA, subchannel_t *subchan, CNetChan *netchan )
	{
		Container *udata = LUA->NewUserType<Container>( metatype );
		udata->subchan = subchan;
		udata->netchan = netchan;

		LUA->PushMetaTable( metatype );
		LUA->SetMetaTable( -2 );

		LUA->CreateTable( );
		LUA->SetFEnv( -2 );
	}

	static subchannel_t *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
	{
		global::CheckType( LUA, index, metatype, metaname );
		Container *udata = LUA->GetUserType<Container>( index, metatype );
		subchannel_t *subchan = udata->subchan;
		if( !IsValid( subchan, udata->netchan ) )
			LUA->FormattedError( "invalid %s", metaname );

		return subchan;
	}

	inline int32_t VerifyStream( GarrysMod::Lua::ILuaBase *LUA, int32_t stream )
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
		subchannel_t *subchan1 = Get( LUA, 1 );
		subchannel_t *subchan2 = Get( LUA, 2 );

		LUA->PushBool( subchan1 == subchan2 );

		return 1;
	}

	LUA_FUNCTION_STATIC( tostring )
	{
		subchannel_t *subchan = Get( LUA, 1 );

		LUA->PushFormattedString( global::tostring_format, metaname, subchan );

		return 1;
	}

	LUA_FUNCTION_STATIC( IsValid )
	{
		global::CheckType( LUA, 1, metatype, metaname );

		Container *udata = LUA->GetUserType<Container>( 1, metatype );
		LUA->PushBool( IsValid( udata->subchan, udata->netchan ) );

		return 1;
	}

	LUA_FUNCTION_STATIC( GetFragmentOffset )
	{
		subchannel_t *subchan = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );

		int32_t ret = VerifyStream( LUA, stream );
		if( ret != 0 )
			return ret;

		LUA->PushNumber( subchan->frag_ofs[stream] );

		return 1;
	}

	LUA_FUNCTION_STATIC( SetFragmentOffset )
	{
		subchannel_t *subchan = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
		LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

		int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );

		int32_t ret = VerifyStream( LUA, stream );
		if( ret != 0 )
			return ret;

		subchan->frag_ofs[stream] = static_cast<int32_t>( LUA->GetNumber( 3 ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( GetFragmentNumber )
	{
		subchannel_t *subchan = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );

		int32_t ret = VerifyStream( LUA, stream );
		if( ret != 0 )
			return ret;

		LUA->PushNumber( subchan->frag_num[stream] );

		return 1;
	}

	LUA_FUNCTION_STATIC( SetFragmentNumber )
	{
		subchannel_t *subchan = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
		LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

		int32_t stream = static_cast<int32_t>( LUA->GetNumber( 2 ) );

		int32_t ret = VerifyStream( LUA, stream );
		if( ret != 0 )
			return ret;

		subchan->frag_num[stream] = static_cast<int32_t>( LUA->GetNumber( 3 ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( GetSequence )
	{
		subchannel_t *subchan = Get( LUA, 1 );

		LUA->PushNumber( subchan->sequence );

		return 1;
	}

	LUA_FUNCTION_STATIC( SetSequence )
	{
		subchannel_t *subchan = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		subchan->sequence = static_cast<int32_t>( LUA->GetNumber( 2 ) );

		return 1;
	}

	LUA_FUNCTION_STATIC( GetState )
	{
		subchannel_t *subchan = Get( LUA, 1 );

		LUA->PushNumber( subchan->state );

		return 1;
	}

	LUA_FUNCTION_STATIC( SetState )
	{
		subchannel_t *subchan = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		subchan->state = static_cast<int32_t>( LUA->GetNumber( 2 ) );

		return 1;
	}

	LUA_FUNCTION_STATIC( GetIndex )
	{
		subchannel_t *subchan = Get( LUA, 1 );

		LUA->PushNumber( subchan->index );

		return 1;
	}

	LUA_FUNCTION_STATIC( SetIndex )
	{
		subchannel_t *subchan = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		subchan->index = static_cast<int32_t>( LUA->GetNumber( 2 ) );

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

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		LUA->PushNil( );
		LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, metaname );
	}
}
