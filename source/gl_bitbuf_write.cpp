#include <gl_bitbuf_write.hpp>
#include <gl_ucharptr.hpp>
#include <bitbuf.h>

struct sn4_bf_write_userdata
{
	sn4_bf_write *pwriter;
	uint8_t type;
	sn4_bf_write writer;
	int32_t bufref;
};

sn4_bf_write **Push_sn4_bf_write( lua_State *state, sn4_bf_write *writer, int32_t bufref )
{
	sn4_bf_write_userdata *userdata = static_cast<sn4_bf_write_userdata *>(
		LUA->NewUserdata( sizeof( sn4_bf_write_userdata ) )
	);
	userdata->type = GET_META_ID( sn4_bf_write );
	userdata->bufref = bufref;

	if( writer == nullptr )
		userdata->pwriter = new( &userdata->writer ) sn4_bf_write;
	else
		userdata->pwriter = writer;

	LUA->CreateMetaTableType( GET_META_NAME( sn4_bf_write ), GET_META_ID( sn4_bf_write ) );
	LUA->SetMetaTable( -2 );

	return &userdata->pwriter;
}

sn4_bf_write *Get_sn4_bf_write( lua_State *state, int32_t index, int32_t *bufref, bool cleanup )
{
	CheckType( state, index, GET_META_ID( sn4_bf_write ), GET_META_NAME( sn4_bf_write ) );

	sn4_bf_write_userdata *userdata = static_cast<sn4_bf_write_userdata *>(
		LUA->GetUserdata( index )
	);
	sn4_bf_write *writer = userdata->pwriter;
	if( writer == nullptr )
		LUA->ThrowError( "invalid sn4_bf_write" );

	if( bufref != nullptr )
		*bufref = userdata->bufref;

	if( cleanup )
	{
		userdata->pwriter = nullptr;
		userdata->bufref = -1;
	}

	return writer;
}

META_ID( sn4_bf_write, 1 );

META_FUNCTION( sn4_bf_write, __gc )
{
	int32_t bufref = -1;
	Get_sn4_bf_write( state, 1, &bufref, true );

	if( bufref != -1 )
		LUA->ReferenceFree( bufref );

	return 0;
}

META_FUNCTION( sn4_bf_write, __eq )
{
	sn4_bf_write *buf1 = Get_sn4_bf_write( state, 1 );
	sn4_bf_write *buf2 = Get_sn4_bf_write( state, 2 );

	LUA->PushBool( buf1 == buf2 );

	return 1;
}

META_FUNCTION( sn4_bf_write, __tostring )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );

	lua_pushfstring( state, "%s: 0x%p", GET_META_NAME( sn4_bf_write ), buf );

	return 1;
}

META_FUNCTION( sn4_bf_write, IsValid )
{
	CheckType( state, 1, GET_META_ID( sn4_bf_write ), GET_META_NAME( sn4_bf_write ) );

	void *userdata = LUA->GetUserdata( 1 );
	LUA->PushBool( static_cast<sn4_bf_write_userdata *>( userdata )->pwriter != nullptr );

	return 1;
}

META_FUNCTION( sn4_bf_write, GetBasePointer )
{
	int32_t bufref = -1;
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1, &bufref );

	if( bufref != -1 )
		LUA->ReferencePush( bufref );
	else
		Push_UCHARPTR( state, buf->m_nDataBits, buf->GetBasePointer( ) );

	return 1;
}

META_FUNCTION( sn4_bf_write, GetMaxNumBits )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );

	LUA->PushNumber( buf->GetMaxNumBits( ) );

	return 1;
}

META_FUNCTION( sn4_bf_write, GetNumBitsWritten )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );

	LUA->PushNumber( buf->GetNumBitsWritten( ) );

	return 1;
}

META_FUNCTION( sn4_bf_write, GetNumBytesWritten )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );

	LUA->PushNumber( buf->GetNumBytesWritten( ) );

	return 1;
}

META_FUNCTION( sn4_bf_write, GetNumBitsLeft )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );

	LUA->PushNumber( buf->GetNumBitsLeft( ) );

	return 1;
}

META_FUNCTION( sn4_bf_write, GetNumBytesLeft )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );

	LUA->PushNumber( buf->GetNumBytesLeft( ) );

	return 1;
}

META_FUNCTION( sn4_bf_write, IsOverflowed )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );

	LUA->PushBool( buf->IsOverflowed() );

	return 1;
}

META_FUNCTION( sn4_bf_write, Seek )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->SeekToBit( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteBitAngle )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	buf->WriteBitAngle( LUA->GetNumber( 2 ), static_cast<int32_t>( LUA->GetNumber( 3 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteAngle )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::ANGLE );

	buf->WriteBitAngles( *static_cast<QAngle *>(
		static_cast<GarrysMod::Lua::UserData *>( LUA->GetUserdata( 2 ) )->data
	) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteBits )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	int32_t bits = 0;
	UCHARPTR ptr = Get_UCHARPTR( state, 2, &bits );

	LUA->PushBool( buf->WriteBits( ptr, bits ) );

	return 1;
}

META_FUNCTION( sn4_bf_write, WriteVector )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::VECTOR );

	buf->WriteBitVec3Coord( *static_cast<Vector *>(
		static_cast<GarrysMod::Lua::UserData *>( LUA->GetUserdata( 2 ) )->data
	) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteNormal )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::VECTOR );

	buf->WriteBitVec3Normal( *static_cast<Vector *>(
		static_cast<GarrysMod::Lua::UserData *>( LUA->GetUserdata( 2 ) )->data
	) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteByte )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteByte( static_cast<uint8_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteBytes )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	int32_t bits = 0;
	UCHARPTR ptr = Get_UCHARPTR( state, 2, &bits );

	LUA->PushBool( buf->WriteBytes( ptr, BitByte( bits ) ) );

	return 1;
}

META_FUNCTION( sn4_bf_write, WriteChar )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteChar( static_cast<int8_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteFloat )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteFloat( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteDouble )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	const double num = LUA->GetNumber( 2 );
	buf->WriteBits( &num, sizeof( double ) * 8 );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteLong )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteLong( static_cast<long>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteLongLong )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteLongLong( static_cast<int64_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteBit )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteOneBit( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteShort )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteShort( static_cast<int16_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteString )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	buf->WriteString( LUA->GetString( 2 ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteInt )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	buf->WriteSBitLong(
		static_cast<int32_t>( LUA->GetNumber( 2 ) ),
		static_cast<int32_t>( LUA->GetNumber( 3 ) )
	);

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteUInt )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	buf->WriteUBitLong(
		static_cast<uint32_t>( LUA->GetNumber( 2 ) ),
		static_cast<int32_t>( LUA->GetNumber( 3 ) )
	);

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteWord )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteWord( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteSignedVarInt32 )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteSignedVarInt32( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteVarInt32 )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteVarInt32( static_cast<uint32_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteSignedVarInt64 )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteSignedVarInt64( static_cast<int64_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteVarInt64 )
{
	sn4_bf_write *buf = Get_sn4_bf_write( state, 1 );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	buf->WriteVarInt64( static_cast<uint64_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

GLBL_FUNCTION( sn4_bf_write )
{
	int32_t bits = 0;
	UCHARPTR ptr = Get_UCHARPTR( state, 1, &bits );

	LUA->Push( 1 );
	sn4_bf_write *writer = *Push_sn4_bf_write( state, nullptr, LUA->ReferenceCreate( ) );
	writer->StartWriting( ptr, BitByte( bits ), 0, bits );

	return 1;
}