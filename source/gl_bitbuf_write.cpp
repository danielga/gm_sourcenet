#include <gl_bitbuf_write.hpp>
#include <gl_ucharptr.hpp>
#include <bitbuf.h>

META_ID( sn4_bf_write, 1 );

META_FUNCTION( sn4_bf_write, GetBasePointer )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	PUSH_META( buf->GetBasePointer( ), UCHARPTR );

	return 1;
}

META_FUNCTION( sn4_bf_write, GetMaxNumBits )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	LUA->PushNumber( buf->GetMaxNumBits( ) );

	return 1;
}

META_FUNCTION( sn4_bf_write, GetNumBitsWritten )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	LUA->PushNumber( buf->GetNumBitsWritten( ) );

	return 1;
}

META_FUNCTION( sn4_bf_write, GetNumBytesWritten )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	LUA->PushNumber( buf->GetNumBytesWritten( ) );

	return 1;
}

META_FUNCTION( sn4_bf_write, GetNumBitsLeft )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	LUA->PushNumber( buf->GetNumBitsLeft( ) );

	return 1;
}

META_FUNCTION( sn4_bf_write, GetNumBytesLeft )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	LUA->PushNumber( buf->GetNumBytesLeft( ) );

	return 1;
}

META_FUNCTION( sn4_bf_write, IsOverflowed )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	LUA->PushBool( buf->IsOverflowed() );

	return 1;
}

META_FUNCTION( sn4_bf_write, Seek )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	buf->SeekToBit( static_cast<int>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteBitAngle )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	buf->WriteBitAngle( LUA->GetNumber( 2 ), static_cast<int>( LUA->GetNumber( 3 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteBits )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );
	LUA->CheckType( 2, GET_META_ID( UCHARPTR ) );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	LUA->PushBool( buf->WriteBits( GET_META( 2, UCHARPTR ), static_cast<int>( LUA->GetNumber( 3 ) ) ) );

	return 1;
}

META_FUNCTION( sn4_bf_write, WriteBitVec3Coord )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::VECTOR );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	buf->WriteBitVec3Coord( *GET_META( 2, Vector ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteByte )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	buf->WriteByte( static_cast<uint8_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteBytes )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );
	LUA->CheckType( 2, GET_META_ID( UCHARPTR ) );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	LUA->PushBool( buf->WriteBytes( GET_META( 2, UCHARPTR ), static_cast<int>( LUA->GetNumber( 3 ) ) ) );

	return 1;
}

META_FUNCTION( sn4_bf_write, WriteChar )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	buf->WriteChar( static_cast<int8_t>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteFloat )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	buf->WriteFloat( LUA->GetNumber( 2 ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteLong )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	buf->WriteLong( static_cast<long>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteOneBit )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	buf->WriteOneBit( static_cast<int>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteShort )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	buf->WriteShort( static_cast<short>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteString )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	buf->WriteString( LUA->GetString( 2 ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteSBitLong )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	buf->WriteSBitLong( static_cast<int>( LUA->GetNumber( 2 ) ), static_cast<int>( LUA->GetNumber( 3 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteUBitLong )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
	LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	buf->WriteUBitLong( static_cast<uint32_t>( LUA->GetNumber( 2 ) ), static_cast<int>( LUA->GetNumber( 3 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, WriteWord )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_write *buf = GET_META( 1, sn4_bf_write );

	buf->WriteWord( static_cast<int>( LUA->GetNumber( 2 ) ) );

	return 0;
}

META_FUNCTION( sn4_bf_write, FinishWriting )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_write ) );

	delete GET_META( 1, sn4_bf_write );

	return 0;
}

GLBL_FUNCTION( sn4_bf_write )
{
	LUA->CheckType( 1, GET_META_ID( UCHARPTR ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int bits = static_cast<int>( LUA->GetNumber( 2 ) );

	sn4_bf_write *buf = new sn4_bf_write( GET_META( 1, UCHARPTR ), BitByte( bits ), bits );

	PUSH_META( buf, sn4_bf_write );

	return 1;
}