#include <gl_bitbuf_write.hpp>
#include <gl_ucharptr.hpp>
#include <bitbuf.h>

namespace sn4_bf_write
{

struct userdata
{
	bf_write *pwriter;
	uint8_t type;
	bf_write writer;
	int32_t bufref;
};

static const uint8_t metaid = Global::metabase + 1;
static const char *metaname = "sn4_bf_write";

bf_write **Push( lua_State *state, bf_write *writer, int32_t bufref )
{
	userdata *udata = static_cast<userdata *>( LUA->NewUserdata( sizeof( userdata ) ) );
	udata->type = metaid;
	udata->bufref = bufref;

	if( writer == nullptr )
		udata->pwriter = new( &udata->writer ) bf_write;
	else
		udata->pwriter = writer;

	LUA->CreateMetaTableType( metaname, metaid );
	LUA->SetMetaTable( -2 );

	LUA->CreateTable( );
	lua_setfenv( state, -2 );

	return &udata->pwriter;
}

bf_write *Get( lua_State *state, int32_t index, int32_t *bufref, bool cleanup )
{
	Global::CheckType( state, index, metaid, metaname );

	userdata *udata = static_cast<userdata *>( LUA->GetUserdata( index ) );
	bf_write *writer = udata->pwriter;
	if( writer == nullptr && !cleanup )
		Global::ThrowError( state, "invalid %s", metaname );

	if( bufref != nullptr )
		*bufref = udata->bufref;

	if( cleanup )
	{
		udata->pwriter = nullptr;
		udata->bufref = -1;
	}

	return writer;
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
	bf_write *buf1 = Get( state, 1 );
	bf_write *buf2 = Get( state, 2 );

	LUA->PushBool( buf1 == buf2 );

	return 1;
}

LUA_FUNCTION_STATIC( tostring )
{
	bf_write *buf = Get( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", metaname, buf );

	return 1;
}

LUA_FUNCTION_STATIC( IsValid )
{
	Global::CheckType( state, 1, metaid, metaname );

	LUA->PushBool( static_cast<userdata *>( LUA->GetUserdata( 1 ) )->pwriter != nullptr );

	return 1;
}

LUA_FUNCTION_STATIC( GetBasePointer )
{
	int32_t bufref = -1;
	bf_write *buf = Get( state, 1, &bufref );

	if( bufref != -1 )
		LUA->ReferencePush( bufref );
	else
		UCHARPTR::Push( state, buf->m_nDataBits, buf->GetBasePointer( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetMaxNumBits )
{
	bf_write *buf = Get( state, 1 );

	LUA->PushNumber( buf->GetMaxNumBits( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetNumBitsWritten )
{
	bf_write *buf = Get( state, 1 );

	LUA->PushNumber( buf->GetNumBitsWritten( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetNumBytesWritten )
{
	bf_write *buf = Get( state, 1 );

	LUA->PushNumber( buf->GetNumBytesWritten( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetNumBitsLeft )
{
	bf_write *buf = Get( state, 1 );

	LUA->PushNumber( buf->GetNumBitsLeft( ) );

	return 1;
}

LUA_FUNCTION_STATIC( GetNumBytesLeft )
{
	bf_write *buf = Get( state, 1 );

	LUA->PushNumber( buf->GetNumBytesLeft( ) );

	return 1;
}

LUA_FUNCTION_STATIC( IsOverflowed )
{
	bf_write *buf = Get( state, 1 );

	LUA->PushBool( buf->IsOverflowed( ) );

	return 1;
}

LUA_FUNCTION_STATIC( Seek )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->SeekToBit( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( WriteBitAngle )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	int32_t bits = static_cast<int32_t>( LUA->GetNumber( 3 ) );
	if( bits < 0 )
		Global::ThrowError( state, "invalid number of bits to write (%d is less than 0)", bits );

	buf->WriteBitAngle( LUA->GetNumber( 2 ), bits );

	return 0;
}

LUA_FUNCTION_STATIC( WriteAngle )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::ANGLE );

	buf->WriteBitAngles( *static_cast<QAngle *>(
		static_cast<GarrysMod::Lua::UserData *>( LUA->GetUserdata( 2 ) )->data
	) );

	return 0;
}

LUA_FUNCTION_STATIC( WriteBits )
{
	bf_write *buf = Get( state, 1 );
	int32_t bits = 0;
	uint8_t *ptr = UCHARPTR::Get( state, 2, &bits );

	LUA->PushBool( buf->WriteBits( ptr, bits ) );

	return 1;
}

LUA_FUNCTION_STATIC( WriteVector )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::VECTOR );

	buf->WriteBitVec3Coord( *static_cast<Vector *>(
		static_cast<GarrysMod::Lua::UserData *>( LUA->GetUserdata( 2 ) )->data
	) );

	return 0;
}

LUA_FUNCTION_STATIC( WriteNormal )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::VECTOR );

	buf->WriteBitVec3Normal( *static_cast<Vector *>(
		static_cast<GarrysMod::Lua::UserData *>( LUA->GetUserdata( 2 ) )->data
	) );

	return 0;
}

LUA_FUNCTION_STATIC( WriteByte )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteByte( static_cast<uint8_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( WriteBytes )
{
	bf_write *buf = Get( state, 1 );
	int32_t bits = 0;
	uint8_t *ptr = UCHARPTR::Get( state, 2, &bits );

	LUA->PushBool( buf->WriteBytes( ptr, BitByte( bits ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( WriteChar )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteChar( static_cast<int8_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( WriteFloat )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteFloat( LUA->GetNumber( 2 ) );

	return 0;
}

LUA_FUNCTION_STATIC( WriteDouble )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	const double num = LUA->GetNumber( 2 );
	LUA->PushBool( buf->WriteBits( &num, sizeof( double ) * 8 ) );

	return 1;
}

LUA_FUNCTION_STATIC( WriteLong )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteLong( static_cast<long>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( WriteLongLong )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteLongLong( static_cast<int64_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( WriteBit )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteOneBit( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( WriteShort )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteShort( static_cast<int16_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( WriteString )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	LUA->PushBool( buf->WriteString( LUA->GetString( 2 ) ) );

	return 1;
}

LUA_FUNCTION_STATIC( WriteInt )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	int32_t bits = static_cast<int32_t>( LUA->GetNumber( 3 ) );
	if( bits < 0 || bits > 32 )
		Global::ThrowError(
			state,
			"invalid number of bits to write (%d is not between 0 and 32)",
			bits
		);

	buf->WriteSBitLong( static_cast<int32_t>( LUA->GetNumber( 2 ) ), bits );

	return 0;
}

LUA_FUNCTION_STATIC( WriteUInt )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	int32_t bits = static_cast<int32_t>( LUA->GetNumber( 3 ) );
	if( bits < 0 || bits > 32 )
		Global::ThrowError(
			state,
			"invalid number of bits to write (%d is not between 0 and 32)",
			bits
		);

	buf->WriteUBitLong( static_cast<uint32_t>( LUA->GetNumber( 2 ) ), bits );

	return 0;
}

LUA_FUNCTION_STATIC( WriteWord )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteWord( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( WriteSignedVarInt32 )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteSignedVarInt32( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( WriteVarInt32 )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteVarInt32( static_cast<uint32_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( WriteSignedVarInt64 )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteSignedVarInt64( static_cast<int64_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( WriteVarInt64 )
{
	bf_write *buf = Get( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteVarInt64( static_cast<uint64_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

LUA_FUNCTION_STATIC( Constructor )
{
	int32_t bits = 0;
	uint8_t *ptr = UCHARPTR::Get( state, 1, &bits );

	LUA->Push( 1 );
	bf_write *writer = *Push( state, nullptr, LUA->ReferenceCreate( ) );
	writer->StartWriting( ptr, BitByte( bits ), 0, bits );

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

		LUA->PushCFunction( GetMaxNumBits );
		LUA->SetField( -2, "GetMaxNumBits" );

		LUA->PushCFunction( GetNumBitsWritten );
		LUA->SetField( -2, "GetNumBitsWritten" );

		LUA->PushCFunction( GetNumBytesWritten );
		LUA->SetField( -2, "GetNumBytesWritten" );

		LUA->PushCFunction( GetNumBitsLeft );
		LUA->SetField( -2, "GetNumBitsLeft" );

		LUA->PushCFunction( GetNumBytesLeft );
		LUA->SetField( -2, "GetNumBytesLeft" );

		LUA->PushCFunction( IsOverflowed );
		LUA->SetField( -2, "IsOverflowed" );

		LUA->PushCFunction( Seek );
		LUA->SetField( -2, "Seek" );

		LUA->PushCFunction( WriteBitAngle );
		LUA->SetField( -2, "WriteBitAngle" );

		LUA->PushCFunction( WriteAngle );
		LUA->SetField( -2, "WriteAngle" );

		LUA->PushCFunction( WriteBits );
		LUA->SetField( -2, "WriteBits" );

		LUA->PushCFunction( WriteVector );
		LUA->SetField( -2, "WriteVector" );

		LUA->PushCFunction( WriteNormal );
		LUA->SetField( -2, "WriteNormal" );

		LUA->PushCFunction( WriteByte );
		LUA->SetField( -2, "WriteByte" );

		LUA->PushCFunction( WriteBytes );
		LUA->SetField( -2, "WriteBytes" );

		LUA->PushCFunction( WriteChar );
		LUA->SetField( -2, "WriteChar" );

		LUA->PushCFunction( WriteFloat );
		LUA->SetField( -2, "WriteFloat" );

		LUA->PushCFunction( WriteDouble );
		LUA->SetField( -2, "WriteDouble" );

		LUA->PushCFunction( WriteLong );
		LUA->SetField( -2, "WriteLong" );

		LUA->PushCFunction( WriteLongLong );
		LUA->SetField( -2, "WriteLongLong" );

		LUA->PushCFunction( WriteBit );
		LUA->SetField( -2, "WriteBit" );

		LUA->PushCFunction( WriteShort );
		LUA->SetField( -2, "WriteShort" );

		LUA->PushCFunction( WriteString );
		LUA->SetField( -2, "WriteString" );

		LUA->PushCFunction( WriteInt );
		LUA->SetField( -2, "WriteInt" );

		LUA->PushCFunction( WriteUInt );
		LUA->SetField( -2, "WriteUInt" );

		LUA->PushCFunction( WriteWord );
		LUA->SetField( -2, "WriteWord" );

		LUA->PushCFunction( WriteSignedVarInt32 );
		LUA->SetField( -2, "WriteSignedVarInt32" );

		LUA->PushCFunction( WriteVarInt32 );
		LUA->SetField( -2, "WriteVarInt32" );

		LUA->PushCFunction( WriteSignedVarInt64 );
		LUA->SetField( -2, "WriteSignedVarInt64" );

		LUA->PushCFunction( WriteVarInt64 );
		LUA->SetField( -2, "WriteVarInt64" );

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