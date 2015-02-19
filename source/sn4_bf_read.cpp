#include <sn4_bf_read.hpp>
#include <ucharptr.hpp>
#include <bitbuf.h>

namespace sn4_bf_read
{

struct userdata
{
	bf_read *preader;
	uint8_t type;
	bf_read reader;
	int32_t bufref;
};

static const uint8_t metaid = Global::metabase + 2;
static const char *metaname = "sn4_bf_read";

bf_read **Push( lua_State *state, bf_read *reader, int32_t bufref )
{
	userdata *udata = static_cast<userdata *>( LUA->NewUserdata( sizeof( userdata ) ) );
	udata->type = metaid;
	udata->bufref = bufref;

	if( reader == nullptr )
		udata->preader = new( &udata->reader ) bf_read;
	else
		udata->preader = reader;

	LUA->CreateMetaTableType( metaname, metaid );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( state, -2 );

	return &udata->preader;
}

bf_read *Get( lua_State *state, int32_t index, int32_t *bufref, bool cleanup )
{
	Global::CheckType( state, index, metaid, metaname );

	userdata *udata = static_cast<userdata *>( LUA->GetUserdata( index ) );
	bf_read *reader = udata->preader;
	if( udata->preader == nullptr && !cleanup )
		Global::ThrowError( state, "invalid %s", metaname );

	if( bufref != nullptr )
		*bufref = udata->bufref;

	if( cleanup )
	{
		udata->preader = nullptr;
		udata->bufref = -1;
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

LUA_FUNCTION_STATIC( gc )
{
	int32_t bufref = -1;
	Get( state, 1, &bufref, true );

	if( bufref != -1 )
		LUA->ReferenceFree( bufref );

	return 0;
}

LUA_FUNCTION_STATIC( eq )
{
	bf_read *buf1 = Get( state, 1 );
	bf_read *buf2 = Get( state, 2 );

	LUA->PushBool( buf1 == buf2 );

	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	bf_read *buf = Get( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", metaname, buf );

	return 1;
}

LUA_FUNCTION_STATIC( IsValid )
{
	Global::CheckType( state, 1, metaid, metaname );

	LUA->PushBool( static_cast<userdata *>( LUA->GetUserdata( 1 ) )->preader != nullptr );

	return 1;
}

LUA_FUNCTION_STATIC( GetBasePointer )
{
	int32_t bufref = -1;
	bf_read *buf = Get( state, 1, &bufref );

	if( bufref != -1 )
		LUA->ReferencePush( bufref );
	else
		UCHARPTR::Push( state, buf->m_nDataBits, const_cast<uint8_t *>( buf->GetBasePointer( ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( TotalBytesAvailable )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushNumber( buf->TotalBytesAvailable( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetNumBitsLeft )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushNumber( buf->GetNumBitsLeft( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetNumBytesLeft )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushNumber( buf->GetNumBytesLeft( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetNumBitsRead )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushNumber( buf->GetNumBitsRead( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetNumBytesRead )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushNumber( buf->GetNumBytesRead( ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsOverflowed )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushBool( buf->IsOverflowed( ) );

	return 1;
}

LUA_FUNCTION_STATIC( Seek )
{
	bf_read *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushBool( buf->Seek( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( SeekRelative )
{
	bf_read *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	LUA->PushBool( buf->SeekRelative( static_cast<int32_t>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( ReadBitAngle )
{
	bf_read *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t bits = static_cast<int32_t>( LUA->GetNumber( 2 ) );
	if( bits < 0 )
		Global::ThrowError( state, "invalid number of bits to read (%d is less than 0)", bits );

	LUA->Push( buf->ReadBitAngle( bits ) );

	return 1;
}

LUA_FUNCTION_STATIC( ReadAngle )
{
	bf_read *buf = Get( state, 1 );

	QAngle *ang = new( std::nothrow ) QAngle;
	if( ang == nullptr )
		LUA->ThrowError( "failed to allocate Angle" );

	buf->ReadBitAngles( *ang );

	Push_Angle( state, ang );

	return 1;
}

LUA_FUNCTION_STATIC( ReadBits )
{
	bf_read *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t bits = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	uint8_t *data = UCHARPTR::Push( state, bits );

	buf->ReadBits( data, bits );

	return 1;
}

LUA_FUNCTION_STATIC( ReadVector )
{
	bf_read *buf = Get( state, 1 );

	Vector *vec = new( std::nothrow ) Vector;
	if( vec == nullptr )
		LUA->ThrowError( "failed to allocate Vector" );

	buf->ReadBitVec3Coord( *vec );

	Push_Vector( state, vec );

	return 1;
}

LUA_FUNCTION_STATIC( ReadNormal )
{
	bf_read *buf = Get( state, 1 );

	Vector *vec = new( std::nothrow ) Vector;
	if( vec == nullptr )
		LUA->ThrowError( "failed to allocate Vector" );

	buf->ReadBitVec3Normal( *vec );

	Push_Vector( state, vec );

	return 1;
}

LUA_FUNCTION_STATIC( ReadByte )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushNumber( buf->ReadByte( ) );

	return 1;
}

LUA_FUNCTION_STATIC( ReadBytes )
{
	bf_read *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t bytes = static_cast<int32_t>( LUA->GetNumber( 2 ) );

	uint8_t *data = UCHARPTR::Push( state, bytes * 8 );

	LUA->PushBool( buf->ReadBytes( data, bytes ) );

	return 2;
}

LUA_FUNCTION_STATIC( ReadChar )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushNumber( buf->ReadChar( ) );

	return 1;
}

LUA_FUNCTION_STATIC( ReadFloat )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushNumber( buf->ReadFloat( ) );

	return 1;
}

LUA_FUNCTION_STATIC( ReadDouble )
{
	bf_read *buf = Get( state, 1 );

	double num = 0.0;
	buf->ReadBits( &num, sizeof( double ) * 8 );
	LUA->PushNumber( num );

	return 1;
}

LUA_FUNCTION_STATIC( ReadLong )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushNumber( buf->ReadLong( ) );

	return 1;
}

LUA_FUNCTION_STATIC( ReadLongLong )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushNumber( buf->ReadLongLong( ) );

	return 1;
}

LUA_FUNCTION_STATIC( ReadBit )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushNumber( buf->ReadOneBit( ) );

	return 1;
}

LUA_FUNCTION_STATIC( ReadShort )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushNumber( buf->ReadShort( ) );

	return 1;
}

LUA_FUNCTION_STATIC( ReadString )
{
	bf_read *buf = Get( state, 1 );

	char str[2048] = { 0 };
	bool success = buf->ReadString( str, sizeof( str ) );

	LUA->PushString( str );
	LUA->PushBool( success );

	return 2;
}

LUA_FUNCTION_STATIC( ReadInt )
{
	bf_read *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t bits = static_cast<int32_t>( LUA->GetNumber( 2 ) );
	if( bits < 0 || bits > 32 )
		Global::ThrowError(
			state,
			"invalid number of bits to read (%d is not between 0 and 32)",
			bits
		);

	LUA->PushNumber( buf->ReadSBitLong( bits ) );

	return 1;
}

LUA_FUNCTION_STATIC( ReadUInt )
{
	bf_read *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int32_t bits = static_cast<int32_t>( LUA->GetNumber( 2 ) );
	if( bits < 0 || bits > 32 )
		Global::ThrowError(
			state,
			"invalid number of bits to read (%d is not between 0 and 32)",
			bits
		);

	LUA->PushNumber( buf->ReadUBitLong( bits ) );

	return 1;
}

LUA_FUNCTION_STATIC( ReadWord )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushNumber( buf->ReadWord( ) );

	return 1;
}

LUA_FUNCTION_STATIC( ReadSignedVarInt32 )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushNumber( buf->ReadSignedVarInt32( ) );

	return 1;
}

LUA_FUNCTION_STATIC( ReadVarInt32 )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushNumber( buf->ReadVarInt32( ) );

	return 1;
}

LUA_FUNCTION_STATIC( ReadSignedVarInt64 )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushNumber( buf->ReadSignedVarInt64( ) );

	return 1;
}

LUA_FUNCTION_STATIC( ReadVarInt64 )
{
	bf_read *buf = Get( state, 1 );

	LUA->PushNumber( buf->ReadVarInt64( ) );

	return 1;
}

LUA_FUNCTION_STATIC( Constructor )
{
	int32_t bits = 0;
	uint8_t *ptr = UCHARPTR::Get( state, 1, &bits );

	LUA->Push( 1 );
	bf_read *reader = *Push( state, nullptr, LUA->ReferenceCreate( ) );
	reader->StartReading( ptr, BitByte( bits ), 0, bits );

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

		LUA->PushCFunction( IsValid );
		LUA->SetField( -2, "IsValid" );

		LUA->PushCFunction( GetBasePointer );
		LUA->SetField( -2, "GetBasePointer" );

		LUA->PushCFunction( TotalBytesAvailable );
		LUA->SetField( -2, "TotalBytesAvailable" );

		LUA->PushCFunction( GetNumBitsLeft );
		LUA->SetField( -2, "GetNumBitsLeft" );

		LUA->PushCFunction( GetNumBytesLeft );
		LUA->SetField( -2, "GetNumBytesLeft" );

		LUA->PushCFunction( GetNumBitsRead );
		LUA->SetField( -2, "GetNumBitsRead" );

		LUA->PushCFunction( GetNumBytesRead );
		LUA->SetField( -2, "GetNumBytesRead" );

		LUA->PushCFunction( IsOverflowed );
		LUA->SetField( -2, "IsOverflowed" );

		LUA->PushCFunction( Seek );
		LUA->SetField( -2, "Seek" );

		LUA->PushCFunction( SeekRelative );
		LUA->SetField( -2, "SeekRelative" );

		LUA->PushCFunction( ReadBitAngle );
		LUA->SetField( -2, "ReadBitAngle" );

		LUA->PushCFunction( ReadAngle );
		LUA->SetField( -2, "ReadAngle" );

		LUA->PushCFunction( ReadBits );
		LUA->SetField( -2, "ReadBits" );

		LUA->PushCFunction( ReadVector );
		LUA->SetField( -2, "ReadVector" );

		LUA->PushCFunction( ReadNormal );
		LUA->SetField( -2, "ReadNormal" );

		LUA->PushCFunction( ReadByte );
		LUA->SetField( -2, "ReadByte" );

		LUA->PushCFunction( ReadBytes );
		LUA->SetField( -2, "ReadBytes" );

		LUA->PushCFunction( ReadChar );
		LUA->SetField( -2, "ReadChar" );

		LUA->PushCFunction( ReadFloat );
		LUA->SetField( -2, "ReadFloat" );

		LUA->PushCFunction( ReadDouble );
		LUA->SetField( -2, "ReadDouble" );

		LUA->PushCFunction( ReadLong );
		LUA->SetField( -2, "ReadLong" );

		LUA->PushCFunction( ReadLongLong );
		LUA->SetField( -2, "ReadLongLong" );

		LUA->PushCFunction( ReadBit );
		LUA->SetField( -2, "ReadBit" );

		LUA->PushCFunction( ReadShort );
		LUA->SetField( -2, "ReadShort" );

		LUA->PushCFunction( ReadString );
		LUA->SetField( -2, "ReadString" );

		LUA->PushCFunction( ReadInt );
		LUA->SetField( -2, "ReadInt" );

		LUA->PushCFunction( ReadUInt );
		LUA->SetField( -2, "ReadUInt" );

		LUA->PushCFunction( ReadWord );
		LUA->SetField( -2, "ReadWord" );

		LUA->PushCFunction( ReadSignedVarInt32 );
		LUA->SetField( -2, "ReadSignedVarInt32" );

		LUA->PushCFunction( ReadVarInt32 );
		LUA->SetField( -2, "ReadVarInt32" );

		LUA->PushCFunction( ReadSignedVarInt64 );
		LUA->SetField( -2, "ReadSignedVarInt64" );

		LUA->PushCFunction( ReadVarInt64 );
		LUA->SetField( -2, "ReadVarInt64" );

	LUA->Pop( 1 );

	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );

		LUA->PushCFunction( Constructor );
		LUA->SetField( -2, metaname );

	LUA->Pop( 1 );
}

void Deinitialize( lua_State *state )
{
	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_REG );

		LUA->PushNil( );
		LUA->SetField( -2, metaname );

	LUA->Pop( 1 );

	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );

		LUA->PushNil( );
		LUA->SetField( -2, metaname );

	LUA->Pop( 1 );
}

}