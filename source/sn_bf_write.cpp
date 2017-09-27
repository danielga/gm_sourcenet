#include <sn_bf_write.hpp>
#include <ucharptr.hpp>
#include <bitbuf.h>

namespace sn_bf_write
{
	struct Container
	{
		bf_write *pwriter;
		bf_write writer;
		int32_t bufref;
	};

	static int32_t metatype = 0;
	static const char *metaname = "sn_bf_write";

	bf_write **Push( GarrysMod::Lua::ILuaBase *LUA, bf_write *writer, int32_t bufref )
	{
		Container *container = LUA->NewUserType<Container>( metatype );
		container->bufref = bufref;

		if( writer == nullptr )
			container->pwriter = new( &container->writer ) bf_write;
		else
			container->pwriter = writer;

		LUA->PushMetaTable( metatype );
		LUA->SetMetaTable( -2 );

		LUA->CreateTable( );
		LUA->SetFEnv( -2 );

		return &container->pwriter;
	}

	inline Container *GetUserData( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
	{
		global::CheckType( LUA, index, metatype, metaname );
		return LUA->GetUserType<Container>( index, metatype );
	}

	bf_write *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index, int32_t *bufref )
	{
		Container *container = GetUserData( LUA, index );
		bf_write *writer = container->pwriter;
		if( writer == nullptr )
			LUA->FormattedError( "invalid %s", metaname );

		if( bufref != nullptr )
			*bufref = container->bufref;

		return writer;
	}

	LUA_FUNCTION_STATIC( gc )
	{
		Container *container = GetUserData( LUA, 1 );

		LUA->ReferenceFree( container->bufref );

		LUA->SetUserType( 1, nullptr );

		return 0;
	}

	LUA_FUNCTION_STATIC( eq )
	{
		bf_write *buf1 = Get( LUA, 1 );
		bf_write *buf2 = Get( LUA, 2 );

		LUA->PushBool( buf1 == buf2 );

		return 1;
	}

	LUA_FUNCTION_STATIC( tostring )
	{
		bf_write *buf = Get( LUA, 1 );

		LUA->PushFormattedString( global::tostring_format, metaname, buf );

		return 1;
	}

	LUA_FUNCTION_STATIC( IsValid )
	{
		LUA->PushBool( GetUserData( LUA, 1 )->pwriter != nullptr );

		return 1;
	}

	LUA_FUNCTION_STATIC( GetBasePointer )
	{
		int32_t bufref = LUA_NOREF;
		bf_write *buf = Get( LUA, 1, &bufref );

		if( bufref != LUA_NOREF )
			LUA->ReferencePush( bufref );
		else
			UCHARPTR::Push( LUA, buf->m_nDataBits, buf->GetBasePointer( ) );

		return 1;
	}

	LUA_FUNCTION_STATIC( GetMaxNumBits )
	{
		bf_write *buf = Get( LUA, 1 );

		LUA->PushNumber( buf->GetMaxNumBits( ) );

		return 1;
	}

	LUA_FUNCTION_STATIC( GetNumBitsWritten )
	{
		bf_write *buf = Get( LUA, 1 );

		LUA->PushNumber( buf->GetNumBitsWritten( ) );

		return 1;
	}

	LUA_FUNCTION_STATIC( GetNumBytesWritten )
	{
		bf_write *buf = Get( LUA, 1 );

		LUA->PushNumber( buf->GetNumBytesWritten( ) );

		return 1;
	}

	LUA_FUNCTION_STATIC( GetNumBitsLeft )
	{
		bf_write *buf = Get( LUA, 1 );

		LUA->PushNumber( buf->GetNumBitsLeft( ) );

		return 1;
	}

	LUA_FUNCTION_STATIC( GetNumBytesLeft )
	{
		bf_write *buf = Get( LUA, 1 );

		LUA->PushNumber( buf->GetNumBytesLeft( ) );

		return 1;
	}

	LUA_FUNCTION_STATIC( IsOverflowed )
	{
		bf_write *buf = Get( LUA, 1 );

		LUA->PushBool( buf->IsOverflowed( ) );

		return 1;
	}

	LUA_FUNCTION_STATIC( Seek )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		buf->SeekToBit( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteBitAngle )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
		LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

		int32_t bits = static_cast<int32_t>( LUA->GetNumber( 3 ) );
		if( bits < 0 )
			LUA->FormattedError( "invalid number of bits to write (%d is less than 0)", bits );

		buf->WriteBitAngle( static_cast<float>( LUA->GetNumber( 2 ) ), bits );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteAngle )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::ANGLE );

		buf->WriteBitAngles( LUA->GetAngle( 2 ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteBits )
	{
		bf_write *buf = Get( LUA, 1 );
		int32_t bits = 0;
		uint8_t *ptr = UCHARPTR::Get( LUA, 2, &bits );

		LUA->PushBool( buf->WriteBits( ptr, bits ) );

		return 1;
	}

	LUA_FUNCTION_STATIC( WriteVector )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::VECTOR );

		buf->WriteBitVec3Coord( LUA->GetVector( 2 ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteNormal )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::VECTOR );

		buf->WriteBitVec3Normal( LUA->GetVector( 2 ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteByte )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		buf->WriteByte( static_cast<uint8_t>( LUA->GetNumber( 2 ) ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteBytes )
	{
		bf_write *buf = Get( LUA, 1 );
		int32_t bits = 0;
		uint8_t *ptr = UCHARPTR::Get( LUA, 2, &bits );

		LUA->PushBool( buf->WriteBytes( ptr, BitByte( bits ) ) );

		return 1;
	}

	LUA_FUNCTION_STATIC( WriteChar )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		buf->WriteChar( static_cast<int8_t>( LUA->GetNumber( 2 ) ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteFloat )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		buf->WriteFloat( static_cast<float>( LUA->GetNumber( 2 ) ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteDouble )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		const double num = LUA->GetNumber( 2 );
		LUA->PushBool( buf->WriteBits( &num, sizeof( double ) * 8 ) );

		return 1;
	}

	LUA_FUNCTION_STATIC( WriteLong )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		buf->WriteLong( static_cast<long>( LUA->GetNumber( 2 ) ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteLongLong )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		buf->WriteLongLong( static_cast<int64_t>( LUA->GetNumber( 2 ) ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteBit )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		buf->WriteOneBit( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteShort )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		buf->WriteShort( static_cast<int16_t>( LUA->GetNumber( 2 ) ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteString )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::STRING );

		LUA->PushBool( buf->WriteString( LUA->GetString( 2 ) ) );

		return 1;
	}

	LUA_FUNCTION_STATIC( WriteInt )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
		LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

		int32_t bits = static_cast<int32_t>( LUA->GetNumber( 3 ) );
		if( bits < 0 || bits > 32 )
			LUA->FormattedError(
				"invalid number of bits to write (%d is not between 0 and 32)",
				bits
			);

		buf->WriteSBitLong( static_cast<int32_t>( LUA->GetNumber( 2 ) ), bits );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteUInt )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
		LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

		int32_t bits = static_cast<int32_t>( LUA->GetNumber( 3 ) );
		if( bits < 0 || bits > 32 )
			LUA->FormattedError(
				"invalid number of bits to write (%d is not between 0 and 32)",
				bits
			);

		buf->WriteUBitLong( static_cast<uint32_t>( LUA->GetNumber( 2 ) ), bits );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteWord )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		buf->WriteWord( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteSignedVarInt32 )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		buf->WriteSignedVarInt32( static_cast<int32_t>( LUA->GetNumber( 2 ) ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteVarInt32 )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		buf->WriteVarInt32( static_cast<uint32_t>( LUA->GetNumber( 2 ) ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteSignedVarInt64 )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		buf->WriteSignedVarInt64( static_cast<int64_t>( LUA->GetNumber( 2 ) ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( WriteVarInt64 )
	{
		bf_write *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		buf->WriteVarInt64( static_cast<uint64_t>( LUA->GetNumber( 2 ) ) );

		return 0;
	}

	LUA_FUNCTION_STATIC( Constructor )
	{
		int32_t bits = 0;
		uint8_t *ptr = UCHARPTR::Get( LUA, 1, &bits );

		LUA->Push( 1 );
		bf_write *writer = *Push( LUA, nullptr, LUA->ReferenceCreate( ) );
		writer->StartWriting( ptr, BitByte( bits ), 0, bits );

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
