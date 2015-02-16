#include <gl_filehandle_t.hpp>

struct FileHandle_userdata
{
	FileHandle_t file;
	uint8_t type;
};

void Push_FileHandle( lua_State *state, FileHandle_t file )
{
	if( file == nullptr )
	{
		LUA->PushNil( );
		return;
	}

	FileHandle_userdata *userdata = static_cast<FileHandle_userdata *>(
		LUA->NewUserdata( sizeof( FileHandle_userdata ) )
	);
	userdata->type = GET_META_ID( FileHandle_t );
	userdata->file = file;

	LUA->CreateMetaTableType( GET_META_NAME( FileHandle_t ), GET_META_ID( FileHandle_t ) );
	LUA->SetMetaTable( -2 );
}

FileHandle_t Get_FileHandle( lua_State *state, int32_t index )
{
	CheckType( state, index, GET_META_ID( FileHandle_t ), GET_META_NAME( FileHandle_t ) );
	return static_cast<FileHandle_userdata *>( LUA->GetUserdata( index ) )->file;
}

META_ID( FileHandle_t, 6 );

META_FUNCTION( FileHandle_t, __eq )
{
	FileHandle_t handle1 = Get_FileHandle( state, 1 );
	FileHandle_t handle2 = Get_FileHandle( state, 2 );

	LUA->PushBool( handle1 == handle2 );

	return 1;
}

META_FUNCTION( FileHandle_t, __tostring )
{
	FileHandle_t ptr = Get_FileHandle( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", GET_META_NAME( FileHandle_t ), ptr );

	return 1;
}