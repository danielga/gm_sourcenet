#include <gl_bitbuf_read.hpp>
#include <gl_ucharptr.hpp>
#include <bitbuf.h>

struct sn4_bf_read_userdata
{
	sn4_bf_read *preader;
	uint8_t type;
	sn4_bf_read reader;
	int32_t bufref;
};

sn4_bf_read **Push_sn4_bf_read( lua_State *state, sn4_bf_read *reader, int32_t bufref )
{
	sn4_bf_read_userdata *userdata = static_cast<sn4_bf_read_userdata *>(
		LUA->NewUserdata( sizeof( sn4_bf_read_userdata ) )
	);
	userdata->type = GET_META_ID( sn4_bf_read );
	userdata->bufref = bufref;

	if( reader == nullptr )
		userdata->preader = new( &userdata->reader ) sn4_bf_read;
	else
		userdata->preader = reader;

	LUA->CreateMetaTableType( GET_META_NAME( sn4_bf_read ), GET_META_ID( sn4_bf_read ) );
	LUA->SetMetaTable( -2 );

	return &userdata->preader;
}

sn4_bf_read *Get_sn4_bf_read( lua_State *state, int32_t index, int32_t *bufref, bool cleanup )
{
	CheckType( state, index, GET_META_ID( sn4_bf_read ), GET_META_NAME( sn4_bf_read ) );

	sn4_bf_read_userdata *userdata = static_cast<sn4_bf_read_userdata *>(
		LUA->GetUserdata( index )
	);
	sn4_bf_read *reader = userdata->preader;
	if( userdata->preader == nullptr )
		LUA->ThrowError( "invalid sn4_bf_read" );

	if( bufref != nullptr )
		*bufref = userdata->bufref;

	if( cleanup )
	{
		userdata->preader = nullptr;
		userdata->bufref = -1;
	}

	return reader;
}

static void Push_Angle( lua_State *state, QAngle *ang )
{
	GarrysMod::Lua::UserData *userdata = static_cast<GarrysMod::Lua::UserData *>(
		LUA->NewUserdata( sizeof( GarrysMod::Lua::UserData ) )
	);
	userdata->type = GarrysMod::Lua::Type::VECTOR;
	userdata->data = ang;

	LUA->CreateMetaTableType( "Angle", GarrysMod::Lua::Type::ANGLE );
	LUA->SetMetaTable( -2 );
}

static void Push_Vector( lua_State *state, Vector *vec )
{
	GarrysMod::Lua::UserData *userdata = static_cast<GarrysMod::Lua::UserData *>(
		LUA->NewUserdata( sizeof( GarrysMod::Lua::UserData ) )
	);
	userdata->type = GarrysMod::Lua::Type::VECTOR;
	userdata->data = vec;

	LUA->CreateMetaTableType( "Vector", GarrysMod::Lua::Type::VECTOR );
	LUA->SetMetaTable( -2 );
}

META_ID( sn4_bf_read, 2 );

META_FUNCTION( sn4_bf_read, __gc )
{
	int32_t bufref = -1;
	Get_sn4_bf_read( state, 1, &bufref, true );

	if( bufref != -1 )
		LUA->ReferenceFree( bufref );

	return 0;
}

META_FUNCTION( sn4_bf_read, __eq )
{
	sn4_bf_read *buf1 = Get_sn4_bf_read( state, 1 );
	sn4_bf_read *buf2 = Get_sn4_bf_read( state, 2 );

	LUA->PushBool( buf1 == buf2 );

	return 1;
}

META_FUNCTION( sn4_bf_read, __tostring )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", GET_META_NAME( sn4_bf_read ), buf );

	return 1;
}

META_FUNCTION( sn4_bf_read, IsValid )
{
	CheckType( state, 1, GET_META_ID( sn4_bf_read ), GET_META_NAME( sn4_bf_read ) );

	void *userdata = LUA->GetUserdata( 1 );
	LUA->PushBool( static_cast<sn4_bf_read_userdata *>( userdata )->preader != nullptr );

	return 1;
}

META_FUNCTION( sn4_bf_read, GetBasePointer )
{
	int32_t bufref = -1;
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1, &bufref );

	if( bufref != -1 )
		LUA->ReferencePush( bufref );
	else
		Push_UCHARPTR( state, buf->m_nDataBits, const_cast<UCHARPTR>( buf->GetBasePointer( ) ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, TotalBytesAvailable )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushNumber( buf->TotalBytesAvailable( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, GetNumBitsLeft )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushNumber( buf->GetNumBitsLeft( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, GetNumBytesLeft )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushNumber( buf->GetNumBytesLeft( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, GetNumBitsRead )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushNumber( buf->GetNumBitsRead( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, GetNumBytesRead )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushNumber( buf->GetNumBytesRead( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, IsOverflowed )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushBool( buf->IsOverflowed( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, Seek )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushBool( buf->Seek( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, SeekRelative )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushBool( buf->SeekRelative( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadBitAngle )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->Push( buf->ReadBitAngle( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadAngle )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	QAngle *ang = new( std::nothrow ) QAngle;
	if( ang == nullptr )
		LUA->ThrowError( "failed to allocate Angle" );

	buf->ReadBitAngles( *ang );

	Push_Angle( state, ang );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadBits )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t bits = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	UCHARPTR data = Push_UCHARPTR( state, bits );

	buf->ReadBits( data, bits );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadVector )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	Vector *vec = new( std::nothrow ) Vector;
	if( vec == nullptr )
		LUA->ThrowError( "failed to allocate Vector" );

	buf->ReadBitVec3Coord( *vec );

	Push_Vector( state, vec );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadNormal )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	Vector *vec = new( std::nothrow ) Vector;
	if( vec == nullptr )
		LUA->ThrowError( "failed to allocate Vector" );

	buf->ReadBitVec3Normal( *vec );

	Push_Vector( state, vec );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadByte )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushNumber( buf->ReadByte( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadBytes )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t bytes = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	UCHARPTR data = Push_UCHARPTR( state, bytes * 8 );

	buf->ReadBytes( data, bytes );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadChar )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushNumber( buf->ReadChar( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadFloat )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushNumber( buf->ReadFloat( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadDouble )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	double num = 0.0;
	buf->ReadBits( &num, sizeof( double ) * 8 );
	LUA->PushNumber( num );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadLong )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushNumber( buf->ReadLong( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadLongLong )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushNumber( buf->ReadLongLong( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadBit )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushNumber( buf->ReadOneBit( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadShort )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushNumber( buf->ReadShort( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadString )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	char str[1024] = { 0 };

	if( buf->ReadString( str, sizeof( str ) ) )
		LUA->PushString( str );
	else
		LUA->PushNil( );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadInt )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( buf->ReadSBitLong( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadUInt )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushNumber( buf->ReadUBitLong( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadWord )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushNumber( buf->ReadWord( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadSignedVarInt32 )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushNumber( buf->ReadSignedVarInt32( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadVarInt32 )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushNumber( buf->ReadVarInt32( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadSignedVarInt64 )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushNumber( buf->ReadSignedVarInt64( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadVarInt64 )
{
	sn4_bf_read *buf = Get_sn4_bf_read( state, 1 );

	LUA->PushNumber( buf->ReadVarInt64( ) );

	return 1;
}

GLBL_FUNCTION( sn4_bf_read )
{
	int32_t bits = 0;
	UCHARPTR ptr = Get_UCHARPTR( state, 1, &bits );

	LUA->Push( 1 );
	sn4_bf_read *reader = *Push_sn4_bf_read( state, nullptr, LUA->ReferenceCreate( ) );
	reader->StartReading( ptr, BitByte( bits ), 0, bits );

	return 1;
}