#include <gl_datafragments_t.hpp>
#include <gl_filehandle_t.hpp>
#include <gl_ucharptr.hpp>
#include <gl_cnetchan.hpp>

struct dataFragments_userdata
{
	dataFragments_t *datafrag;
	uint8_t type;
	CNetChan *netchan;
};

static bool IsValid_dataFragments( dataFragments_t *datafrag, CNetChan *netchan )
{
	return datafrag != nullptr && ( netchan == nullptr || IsValid_CNetChan( netchan ) );
}

void Push_dataFragments( lua_State *state, dataFragments_t *datafrag, CNetChan *netchan )
{
	if( datafrag == nullptr )
	{
		LUA->PushNil( );
		return;
	}

	dataFragments_userdata *userdata = static_cast<dataFragments_userdata *>(
		LUA->NewUserdata( sizeof( dataFragments_userdata ) )
	);
	userdata->type = GET_META_ID( dataFragments_t );
	userdata->datafrag = datafrag;
	userdata->netchan = netchan;

	LUA->CreateMetaTableType( GET_META_NAME( dataFragments_t ), GET_META_ID( dataFragments_t ) );
	LUA->SetMetaTable( -2 );
}

dataFragments_t *Get_dataFragments(
	lua_State *state,
	int32_t index,
	CNetChan **netchan,
	bool cleanup
)
{
	LUA->CheckType( index, GET_META_ID( dataFragments_t ) );

	dataFragments_userdata *userdata = static_cast<dataFragments_userdata *>(
		LUA->GetUserdata( index )
	);
	dataFragments_t *datafrag = userdata->datafrag;
	if( !IsValid_dataFragments( datafrag, userdata->netchan ) )
		LUA->ThrowError( "invalid dataFragments_t" );

	if( netchan != nullptr )
		*netchan = userdata->netchan;

	if( cleanup )
	{
		userdata->datafrag = nullptr;
		userdata->netchan = nullptr;
	}

	return datafrag;
}

META_ID( dataFragments_t, 5 );

META_FUNCTION( dataFragments_t, __gc )
{
	CNetChan *netchan = nullptr;
	dataFragments_t *fragments = Get_dataFragments( state, 1, &netchan, true );

	if( netchan == nullptr )
		delete fragments;

	return 0;
}

META_FUNCTION( dataFragments_t, __eq )
{
	dataFragments_t *fragments1 = Get_dataFragments( state, 1 );
	dataFragments_t *fragments2 = Get_dataFragments( state, 2 );

	LUA->PushBool( fragments1 == fragments2 );

	return 1;
}

META_FUNCTION( dataFragments_t, __tostring )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", GET_META_NAME( dataFragments_t ), fragments );

	return 1;
}

META_FUNCTION( dataFragments_t, IsValid )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );

	dataFragments_userdata *userdata = static_cast<dataFragments_userdata *>(
		LUA->GetUserdata( 1 )
	);
	LUA->PushBool( IsValid_dataFragments( userdata->datafrag, userdata->netchan ) );

	return 1;
}

META_FUNCTION( dataFragments_t, GetFileHandle )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );

	Push_FileHandle( state, fragments->hfile );

	return 1;
}

META_FUNCTION( dataFragments_t, SetFileHandle )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );

	int32_t argt = LUA->GetType( 2 );
	if( argt == GET_META_ID( FileHandle_t ) )
		fragments->hfile = Get_FileHandle( state, 2 );
	else if( argt == GarrysMod::Lua::Type::NUMBER && LUA->GetNumber( 2 ) == 0 )
		fragments->hfile = nullptr;
	else
		luaL_typerror( state, 2, GET_META_NAME( FileHandle_t ) );

	return 0;
}

META_FUNCTION( dataFragments_t, GetFileName )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );

	LUA->PushString( fragments->filename );

	return 1;
}

META_FUNCTION( dataFragments_t, SetFileName )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );
	LUA->CheckType( 2 , GarrysMod::Lua::Type::STRING );

	strncpy( fragments->filename, LUA->GetString( 2 ), MAX_PATH );
	fragments->filename[sizeof( fragments->filename )] = '\0';

	return 0;
}

META_FUNCTION( dataFragments_t, GetFileTransferID )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );

	LUA->PushNumber( fragments->transferid );

	return 1;
}

META_FUNCTION( dataFragments_t, SetFileTransferID )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	fragments->transferid = static_cast<uint32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( dataFragments_t, GetBuffer )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );

	Push_UCHARPTR( state, fragments->bits, fragments->buffer );

	return 1;
}

META_FUNCTION( dataFragments_t, SetBuffer )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );

	int32_t argt = LUA->GetType( 2 );
	if( argt == GET_META_ID( UCHARPTR ) )
	{
		uint8_t *oldbuf = fragments->buffer;

		int32_t bits = 0;
		fragments->buffer = Get_UCHARPTR( state, 2, &bits, true );
		fragments->bits = bits;
		fragments->bytes = BitByte( bits );

		delete[] oldbuf;
	}
	else if( argt == GarrysMod::Lua::Type::NUMBER && LUA->GetNumber( 2 ) == 0 )
	{
		delete[] fragments->buffer;

		fragments->buffer = nullptr;
		fragments->bits = 0;
		fragments->bytes = 0;
	}
	else
	{
		luaL_typerror( state, 2, GET_META_NAME( UCHARPTR ) );
	}
		
	return 0;
}


META_FUNCTION( dataFragments_t, GetBytes )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );

	LUA->PushNumber( fragments->bytes );

	return 1;
}

META_FUNCTION( dataFragments_t, SetBytes )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	fragments->bytes = static_cast<uint32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( dataFragments_t, GetBits )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );

	LUA->PushNumber( fragments->bits );

	return 1;
}

META_FUNCTION( dataFragments_t, SetBits )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	fragments->bits = static_cast<uint32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( dataFragments_t, GetActualSize )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );

	LUA->PushNumber( fragments->actualsize );

	return 1;
}

META_FUNCTION( dataFragments_t, SetActualSize )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	fragments->actualsize = static_cast<uint32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( dataFragments_t, GetCompressed )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );

	LUA->PushBool( fragments->compressed );

	return 1;
}

META_FUNCTION( dataFragments_t, SetCompressed )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	fragments->compressed = LUA->GetBool( 2 );

	return 0;
}

META_FUNCTION( dataFragments_t, GetStream )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );

	LUA->PushBool( fragments->stream );

	return 1;
}

META_FUNCTION( dataFragments_t, SetStream )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	fragments->stream = LUA->GetBool( 2 );

	return 0;
}

META_FUNCTION( dataFragments_t, GetTotal )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );

	LUA->PushNumber( fragments->total );

	return 1;
}

META_FUNCTION( dataFragments_t, SetTotal )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	fragments->total = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( dataFragments_t, GetProgress )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );

	LUA->PushNumber( fragments->progress );

	return 1;
}

META_FUNCTION( dataFragments_t, SetProgress )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	fragments->progress = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( dataFragments_t, GetNum )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );

	LUA->PushNumber( fragments->num );

	return 1;
}

META_FUNCTION( dataFragments_t, SetNum )
{
	dataFragments_t *fragments = Get_dataFragments( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	fragments->num = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

GLBL_FUNCTION( dataFragments_t )
{
	Push_dataFragments( state, new( std::nothrow ) dataFragments_t );

	return 1;
}