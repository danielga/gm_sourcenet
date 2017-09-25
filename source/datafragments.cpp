#include <datafragments.hpp>
#include <filehandle.hpp>
#include <ucharptr.hpp>
#include <netchannel.hpp>

namespace dataFragments
{

struct Container
{
	dataFragments_t *datafrag;
	CNetChan *netchan;
};

static uint8_t metatype = 0;
static const char *metaname = "dataFragments_t";

static bool IsValid( dataFragments_t *datafrag, CNetChan *netchan )
{
	return datafrag != nullptr && NetChannel::IsValid( netchan );
}

void Push( GarrysMod::Lua::ILuaBase *LUA, dataFragments_t *datafrag, CNetChan *netchan )
{
	if( datafrag == nullptr )
	{
		datafrag = new( std::nothrow ) dataFragments_t;
		if( datafrag == nullptr )
		{
			LUA->PushNil( );
			return;
		}
	}

	Container *udata = LUA->NewUserType<Container>( metatype );
	udata->datafrag = datafrag;
	udata->netchan = netchan;

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

dataFragments_t *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index, CNetChan **netchan )
{
	Container *udata = GetUserData( LUA, index );
	dataFragments_t *datafrag = udata->datafrag;
	if( !IsValid( datafrag, udata->netchan ) )
		LUA->FormattedError( "invalid %s", metaname );

	if( netchan != nullptr )
		*netchan = udata->netchan;

	return datafrag;
}

LUA_FUNCTION_STATIC( gc )
{
	Container *udata = GetUserData( LUA, 1 );

	if( udata->netchan != nullptr )
		delete udata->datafrag;

	LUA->SetUserType( 1, nullptr );

	return 0;
}

LUA_FUNCTION_STATIC( eq )
{
	LUA->PushBool( Get( LUA, 1 ) == Get( LUA, 2 ) );
	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	LUA->PushFormattedString( global::tostring_format, metaname, Get( LUA, 1 ) );
	return 1;
}

LUA_FUNCTION_STATIC( IsValid )
{
	Container *udata = GetUserData( LUA, 1 );
	LUA->PushBool( IsValid( udata->datafrag, udata->netchan ) );
	return 1;
}

LUA_FUNCTION_STATIC( GetFileHandle )
{
	FileHandle::Push( LUA, Get( LUA, 1 )->hfile );
	return 1;
}

LUA_FUNCTION_STATIC( SetFileHandle )
{
	dataFragments_t *fragments = Get( LUA, 1 );

	int32_t argt = LUA->GetType( 2 );
	if( argt == FileHandle::metatype )
		fragments->hfile = FileHandle::Get( LUA, 2 );
	else if( argt == GarrysMod::Lua::Type::NIL || ( argt == GarrysMod::Lua::Type::NUMBER && LUA->GetNumber( 2 ) == 0 ) )
		fragments->hfile = nullptr;
	else
		LUA->TypeError( 2, FileHandle::metaname );

	return 0;
}

LUA_FUNCTION_STATIC( GetFileName )
{
	LUA->PushString( Get( LUA, 1 )->filename );
	return 1;
}

LUA_FUNCTION_STATIC( SetFileName )
{
	dataFragments_t *fragments = Get( LUA, 1 );

	strncpy( fragments->filename, LUA->CheckString( 2 ), MAX_PATH );
	fragments->filename[sizeof( fragments->filename ) - 1] = '\0';

	return 0;
}

LUA_FUNCTION_STATIC( GetFileTransferID )
{
	LUA->PushNumber( Get( LUA, 1 )->transferid );
	return 1;
}

LUA_FUNCTION_STATIC( SetFileTransferID )
{
	Get( LUA, 1 )->transferid = static_cast<uint32_t>( LUA->CheckNumber( 2 ) );
	return 0;
}

LUA_FUNCTION_STATIC( GetBuffer )
{
	dataFragments_t *fragments = Get( LUA, 1 );
	UCHARPTR::Push( LUA, fragments->bits, fragments->buffer );
	return 1;
}

LUA_FUNCTION_STATIC( SetBuffer )
{
	dataFragments_t *fragments = Get( LUA, 1 );

	int32_t argt = LUA->GetType( 2 );
	if( argt == UCHARPTR::metatype )
	{
		uint8_t *oldbuf = fragments->buffer;

		int32_t bits = 0;
		fragments->buffer = UCHARPTR::Release( LUA, 2, &bits );
		fragments->bits = bits;
		fragments->bytes = BitByte( bits );

		delete[] oldbuf;
	}
	else if( argt == GarrysMod::Lua::Type::NIL || ( argt == GarrysMod::Lua::Type::NUMBER && LUA->GetNumber( 2 ) == 0 ) )
	{
		delete[] fragments->buffer;

		fragments->buffer = nullptr;
		fragments->bits = 0;
		fragments->bytes = 0;
	}
	else
	{
		LUA->TypeError( 2, UCHARPTR::metaname );
	}
		
	return 0;
}


LUA_FUNCTION_STATIC( GetBytes )
{
	LUA->PushNumber( Get( LUA, 1 )->bytes );
	return 1;
}

LUA_FUNCTION_STATIC( SetBytes )
{
	Get( LUA, 1 )->bytes = static_cast<uint32_t>( LUA->CheckNumber( 2 ) );
	return 0;
}

LUA_FUNCTION_STATIC( GetBits )
{
	LUA->PushNumber( Get( LUA, 1 )->bits );
	return 1;
}

LUA_FUNCTION_STATIC( SetBits )
{
	Get( LUA, 1 )->bits = static_cast<uint32_t>( LUA->CheckNumber( 2 ) );
	return 0;
}

LUA_FUNCTION_STATIC( GetActualSize )
{
	LUA->PushNumber( Get( LUA, 1 )->actualsize );
	return 1;
}

LUA_FUNCTION_STATIC( SetActualSize )
{
	Get( LUA, 1 )->actualsize = static_cast<uint32_t>( LUA->CheckNumber( 2 ) );
	return 0;
}

LUA_FUNCTION_STATIC( GetCompressed )
{
	LUA->PushBool( Get( LUA, 1 )->compressed );
	return 1;
}

LUA_FUNCTION_STATIC( SetCompressed )
{
	dataFragments_t *fragments = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	fragments->compressed = LUA->GetBool( 2 );
	return 0;
}

LUA_FUNCTION_STATIC( GetStream )
{
	LUA->PushBool( Get( LUA, 1 )->stream );
	return 1;
}

LUA_FUNCTION_STATIC( SetStream )
{
	dataFragments_t *fragments = Get( LUA, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

	fragments->stream = LUA->GetBool( 2 );
	return 0;
}

LUA_FUNCTION_STATIC( GetTotal )
{
	LUA->PushNumber( Get( LUA, 1 )->total );
	return 1;
}

LUA_FUNCTION_STATIC( SetTotal )
{
	Get( LUA, 1 )->total = static_cast<int32_t>( LUA->CheckNumber( 2 ) );
	return 0;
}

LUA_FUNCTION_STATIC( GetProgress )
{
	LUA->PushNumber( Get( LUA, 1 )->progress );
	return 1;
}

LUA_FUNCTION_STATIC( SetProgress )
{
	Get( LUA, 1 )->progress = static_cast<int32_t>( LUA->CheckNumber( 2 ) );
	return 0;
}

LUA_FUNCTION_STATIC( GetNum )
{
	LUA->PushNumber( Get( LUA, 1 )->num );
	return 1;
}

LUA_FUNCTION_STATIC( SetNum )
{
	Get( LUA, 1 )->num = static_cast<int32_t>( LUA->CheckNumber( 2 ) );
	return 0;
}

LUA_FUNCTION_STATIC( Constructor )
{
	Push( LUA );
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

		LUA->PushCFunction( global::GetTable );
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
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, metaname );
}

void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
{
	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, metaname );

	LUA->PushNil( );
	LUA->SetField( GarrysMod::Lua::INDEX_REGISTRY, metaname );
}

}
