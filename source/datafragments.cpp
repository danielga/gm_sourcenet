#include <datafragments.hpp>
#include <filehandle.hpp>
#include <ucharptr.hpp>
#include <netchannel.hpp>

extern "C"
{

#include <lauxlib.h>

}

namespace dataFragments
{

struct userdata
{
	dataFragments_t *datafrag;
	uint8_t type;
	CNetChan *netchan;
};

static const uint8_t metaid = Global::metabase + 5;
static const char *metaname = "dataFragments_t";

static bool IsValid( dataFragments_t *datafrag, CNetChan *netchan )
{
	return datafrag != nullptr && ( netchan == nullptr || NetChannel::IsValid( netchan ) );
}

void Push( lua_State *state, dataFragments_t *datafrag, CNetChan *netchan )
{
	if( datafrag == nullptr )
	{
		LUA->PushNil( );
		return;
	}

	userdata *udata = static_cast<userdata *>( LUA->NewUserdata( sizeof( userdata ) ) );
	udata->type = metaid;
	udata->datafrag = datafrag;
	udata->netchan = netchan;

	LUA->CreateMetaTableType( metaname, metaid );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( state, -2 );
}

dataFragments_t *Get( lua_State *state, int32_t index, CNetChan **netchan, bool cleanup )
{
	Global::CheckType( state, index, metaid, metaname );

	userdata *udata = static_cast<userdata *>( LUA->GetUserdata( index ) );
	dataFragments_t *datafrag = udata->datafrag;
	if( !IsValid( datafrag, udata->netchan ) && !cleanup )
		Global::ThrowError( state, "invalid %s", metaname );

	if( netchan != nullptr )
		*netchan = udata->netchan;

	if( cleanup )
	{
		udata->datafrag = nullptr;
		udata->netchan = nullptr;
	}

	return datafrag;
}

LUA_FUNCTION_STATIC( gc )
{
	CNetChan *netchan = nullptr;
	dataFragments_t *fragments = Get( state, 1, &netchan, true );

	if( netchan == nullptr )
		delete fragments;

	return 0;
}

LUA_FUNCTION_STATIC( eq )
{
	dataFragments_t *fragments1 = Get( state, 1 );
	dataFragments_t *fragments2 = Get( state, 2 );

	LUA->PushBool( fragments1 == fragments2 );

	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	dataFragments_t *fragments = Get( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", metaname, fragments );

	return 1;
}

LUA_FUNCTION_STATIC( IsValid )
{
	Global::CheckType( state, 1, metaid, metaname );

	userdata *udata = static_cast<userdata *>( LUA->GetUserdata( 1 ) );
	LUA->PushBool( IsValid( udata->datafrag, udata->netchan ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetFileHandle )
{
	dataFragments_t *fragments = Get( state, 1 );

	FileHandle::Push( state, fragments->hfile );

	return 1;
}

LUA_FUNCTION_STATIC( SetFileHandle )
{
	dataFragments_t *fragments = Get( state, 1 );

	int32_t argt = LUA->GetType( 2 );
	if( argt == FileHandle::metaid )
		fragments->hfile = FileHandle::Get( state, 2 );
	else if( argt == GarrysMod::Lua::Type::NUMBER && LUA->GetNumber( 2 ) == 0 )
		fragments->hfile = nullptr;
	else
		luaL_typerror( state, 2, FileHandle::metaname );

	return 0;
}

LUA_FUNCTION_STATIC( GetFileName )
{
	dataFragments_t *fragments = Get( state, 1 );

	LUA->PushString( fragments->filename );

	return 1;
}

LUA_FUNCTION_STATIC( SetFileName )
{
	dataFragments_t *fragments = Get( state, 1 );
	LUA->CheckType( 2 , GarrysMod::Lua::Type::STRING );

	strncpy( fragments->filename, LUA->GetString( 2 ), MAX_PATH );
	fragments->filename[sizeof( fragments->filename )] = '\0';

	return 0;
}

LUA_FUNCTION_STATIC( GetFileTransferID )
{
	dataFragments_t *fragments = Get( state, 1 );

	LUA->PushNumber( fragments->transferid );

	return 1;
}

LUA_FUNCTION_STATIC( SetFileTransferID )
{
	dataFragments_t *fragments = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	fragments->transferid = static_cast<uint32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( GetBuffer )
{
	dataFragments_t *fragments = Get( state, 1 );

	UCHARPTR::Push( state, fragments->bits, fragments->buffer );

	return 1;
}

LUA_FUNCTION_STATIC( SetBuffer )
{
	dataFragments_t *fragments = Get( state, 1 );

	int32_t argt = LUA->GetType( 2 );
	if( argt == UCHARPTR::metaid )
	{
		uint8_t *oldbuf = fragments->buffer;

		int32_t bits = 0;
		fragments->buffer = UCHARPTR::Get( state, 2, &bits, true );
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
		luaL_typerror( state, 2, UCHARPTR::metaname );
	}
		
	return 0;
}


LUA_FUNCTION_STATIC( GetBytes )
{
	dataFragments_t *fragments = Get( state, 1 );

	LUA->PushNumber( fragments->bytes );

	return 1;
}

LUA_FUNCTION_STATIC( SetBytes )
{
	dataFragments_t *fragments = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	fragments->bytes = static_cast<uint32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( GetBits )
{
	dataFragments_t *fragments = Get( state, 1 );

	LUA->PushNumber( fragments->bits );

	return 1;
}

LUA_FUNCTION_STATIC( SetBits )
{
	dataFragments_t *fragments = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	fragments->bits = static_cast<uint32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( GetActualSize )
{
	dataFragments_t *fragments = Get( state, 1 );

	LUA->PushNumber( fragments->actualsize );

	return 1;
}

LUA_FUNCTION_STATIC( SetActualSize )
{
	dataFragments_t *fragments = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	fragments->actualsize = static_cast<uint32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( GetCompressed )
{
	dataFragments_t *fragments = Get( state, 1 );

	LUA->PushBool( fragments->compressed );

	return 1;
}

LUA_FUNCTION_STATIC( SetCompressed )
{
	dataFragments_t *fragments = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	fragments->compressed = LUA->GetBool( 2 );

	return 0;
}

LUA_FUNCTION_STATIC( GetStream )
{
	dataFragments_t *fragments = Get( state, 1 );

	LUA->PushBool( fragments->stream );

	return 1;
}

LUA_FUNCTION_STATIC( SetStream )
{
	dataFragments_t *fragments = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	fragments->stream = LUA->GetBool( 2 );

	return 0;
}

LUA_FUNCTION_STATIC( GetTotal )
{
	dataFragments_t *fragments = Get( state, 1 );

	LUA->PushNumber( fragments->total );

	return 1;
}

LUA_FUNCTION_STATIC( SetTotal )
{
	dataFragments_t *fragments = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	fragments->total = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( GetProgress )
{
	dataFragments_t *fragments = Get( state, 1 );

	LUA->PushNumber( fragments->progress );

	return 1;
}

LUA_FUNCTION_STATIC( SetProgress )
{
	dataFragments_t *fragments = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	fragments->progress = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( GetNum )
{
	dataFragments_t *fragments = Get( state, 1 );

	LUA->PushNumber( fragments->num );

	return 1;
}

LUA_FUNCTION_STATIC( SetNum )
{
	dataFragments_t *fragments = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	fragments->num = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( Constructor )
{
	Push( state, new( std::nothrow ) dataFragments_t );

	return 1;
}

void Initialize( lua_State *state )
{
	LUA->CreateMetaTableType( metaname, metaid );

		LUA->PushCFunction( gc );
		LUA->SetField( -2, "__gc" );

		LUA->PushCFunction( eq );
		LUA->SetField( -2, "__eq" );

		LUA->PushCFunction( tostring );
		LUA->SetField( -2, "__tostring" );

		LUA->PushCFunction( Global::index );
		LUA->SetField( -2, "__index" );

		LUA->PushCFunction( Global::newindex );
		LUA->SetField( -2, "__newindex" );

		LUA->PushCFunction( Global::GetTable );
		LUA->SetField( -2, "GetTable" );

		LUA->PushCFunction( IsValid );
		LUA->SetField( -2, "IsValid" );

		LUA->PushCFunction( GetFileHandle );
		LUA->SetField( -2, "GetFileHandle" );

		LUA->PushCFunction( SetFileHandle );
		LUA->SetField( -2, "SetFileHandle" );

		LUA->PushCFunction( GetFileName );
		LUA->SetField( -2, "GetFileName" );

		LUA->PushCFunction( SetFileName );
		LUA->SetField( -2, "SetFileName" );

		LUA->PushCFunction( GetFileTransferID );
		LUA->SetField( -2, "GetFileTransferID" );

		LUA->PushCFunction( SetFileTransferID );
		LUA->SetField( -2, "SetFileTransferID" );

		LUA->PushCFunction( GetBuffer );
		LUA->SetField( -2, "GetBuffer" );

		LUA->PushCFunction( SetBuffer );
		LUA->SetField( -2, "SetBuffer" );

		LUA->PushCFunction( GetBytes );
		LUA->SetField( -2, "GetBytes" );

		LUA->PushCFunction( SetBytes );
		LUA->SetField( -2, "SetBytes" );

		LUA->PushCFunction( GetBits );
		LUA->SetField( -2, "GetBits" );

		LUA->PushCFunction( SetBits );
		LUA->SetField( -2, "SetBits" );

		LUA->PushCFunction( GetActualSize );
		LUA->SetField( -2, "GetActualSize" );

		LUA->PushCFunction( SetActualSize );
		LUA->SetField( -2, "SetActualSize" );

		LUA->PushCFunction( GetCompressed );
		LUA->SetField( -2, "GetCompressed" );

		LUA->PushCFunction( SetCompressed );
		LUA->SetField( -2, "SetCompressed" );

		LUA->PushCFunction( GetStream );
		LUA->SetField( -2, "GetStream" );

		LUA->PushCFunction( SetStream );
		LUA->SetField( -2, "SetStream" );

		LUA->PushCFunction( GetTotal );
		LUA->SetField( -2, "GetTotal" );

		LUA->PushCFunction( SetTotal );
		LUA->SetField( -2, "SetTotal" );

		LUA->PushCFunction( GetProgress );
		LUA->SetField( -2, "GetProgress" );

		LUA->PushCFunction( SetProgress );
		LUA->SetField( -2, "SetProgress" );

		LUA->PushCFunction( GetNum );
		LUA->SetField( -2, "GetNum" );

		LUA->PushCFunction( SetNum );
		LUA->SetField( -2, "SetNum" );

	LUA->Pop( 1 );

	LUA->PushCFunction( Constructor );
	LUA->SetField( -2, metaname );
}

void Deinitialize( lua_State *state )
{
	LUA->PushNil( );
	LUA->SetField( -3, metaname );

	LUA->PushNil( );
	LUA->SetField( -2, metaname );
}

}