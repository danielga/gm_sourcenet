#include <gl_datafragments_t.hpp>
#include <gl_filehandle_t.hpp>
#include <gl_ucharptr.hpp>

META_ID( dataFragments_t, 5 );

META_FUNCTION( dataFragments_t, GetFileHandle )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	PUSH_META( fragments->hfile, FileHandle_t );

	return 1;
}

META_FUNCTION( dataFragments_t, SetFileHandle )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	int arg___t = LUA->GetType( 2 );
	if( arg___t == GET_META_ID( FileHandle_t ) )
		fragments->hfile = static_cast<FileHandle_t>( GET_META( 2, FileHandle_t ) );
	else if( arg___t == GarrysMod::Lua::Type::NUMBER && static_cast<int>( LUA->GetNumber( 2 ) ) == 0 )
		fragments->hfile = static_cast<FileHandle_t>( nullptr );
	else
		TypeError( state, GET_META_NAME( FileHandle_t ), 2 );

	return 0;
}

META_FUNCTION( dataFragments_t, GetFileName )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	LUA->PushString( fragments->filename );

	return 1;
}

META_FUNCTION( dataFragments_t, SetFileName )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );
	LUA->CheckType( 2 , GarrysMod::Lua::Type::STRING );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	strcpy_s( fragments->filename, MAX_PATH, LUA->GetString( 2 ) );

	return 0;
}

META_FUNCTION( dataFragments_t, GetFileTransferID )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	LUA->PushNumber( fragments->transferid );

	return 1;
}

META_FUNCTION( dataFragments_t, SetFileTransferID )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	fragments->transferid = static_cast<uint32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( dataFragments_t, GetBuffer )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	PUSH_META( fragments->buffer, UCHARPTR );

	return 1;
}

META_FUNCTION( dataFragments_t, SetBuffer )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );
	
	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	int arg___t = LUA->GetType( 2 );
	if( arg___t == GET_META_ID( UCHARPTR ) )
		fragments->buffer = static_cast<UCHARPTR>( GET_META( 2, uint8_t ) );
	else if( arg___t == GarrysMod::Lua::Type::NUMBER && static_cast<int>( LUA->GetNumber( 2 ) ) == 0 )
		fragments->buffer = static_cast<UCHARPTR>( nullptr );
	else
		TypeError( state, GET_META_NAME( UCHARPTR ), 2 );
		
	return 0;
}


META_FUNCTION( dataFragments_t, GetBytes )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	LUA->PushNumber( fragments->bytes );

	return 1;
}

META_FUNCTION( dataFragments_t, SetBytes )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	fragments->bytes = static_cast<uint32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( dataFragments_t, GetBits )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	LUA->PushNumber( fragments->bits );

	return 1;
}

META_FUNCTION( dataFragments_t, SetBits )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	fragments->bits = static_cast<uint32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( dataFragments_t, GetActualSize )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	LUA->PushNumber( fragments->actualsize );

	return 1;
}

META_FUNCTION( dataFragments_t, SetActualSize )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	fragments->actualsize = static_cast<uint32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( dataFragments_t, GetCompressed )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	LUA->PushBool( fragments->compressed );

	return 1;
}

META_FUNCTION( dataFragments_t, SetCompressed )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	fragments->compressed = LUA->GetBool( 2 );

	return 0;
}

META_FUNCTION( dataFragments_t, GetStream )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	LUA->PushBool( fragments->stream );

	return 1;
}

META_FUNCTION( dataFragments_t, SetStream )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	fragments->stream = LUA->GetBool( 2 );

	return 0;
}

META_FUNCTION( dataFragments_t, GetTotal )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	LUA->PushNumber( fragments->total );

	return 1;
}

META_FUNCTION( dataFragments_t, SetTotal )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	fragments->total = static_cast<int>( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( dataFragments_t, GetProgress )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	LUA->PushNumber( fragments->progress );

	return 1;
}

META_FUNCTION( dataFragments_t, SetProgress )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	fragments->progress = static_cast<int>( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( dataFragments_t, GetNum )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	LUA->PushNumber( fragments->num );

	return 1;
}

META_FUNCTION( dataFragments_t, SetNum )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	dataFragments_t *fragments = GET_META( 1, dataFragments_t );

	fragments->num = static_cast<int>( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( dataFragments_t, Delete )
{
	LUA->CheckType( 1, GET_META_ID( dataFragments_t ) );

	delete GET_META( 1, dataFragments_t );

	return 0;
}

GLBL_FUNCTION( dataFragments_t )
{
	dataFragments_t *fragments = new dataFragments_t;

	PUSH_META( fragments, dataFragments_t );

	return 1;
}