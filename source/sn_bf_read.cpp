#include "sn_bf_read.hpp"
#include "ucharptr.hpp"

#include <bitbuf.h>
#include <algorithm>
#include <vector>

namespace sn_bf_read
{
	struct Container
	{
		bf_read *preader;
		bf_read reader;
		int32_t bufref;
	};

	static int32_t metatype = 0;
	static const char *metaname = "sn_bf_read";

	bf_read **Push( GarrysMod::Lua::ILuaBase *LUA, bf_read *reader, int32_t bufref )
	{
		Container *container = LUA->NewUserType<Container>( metatype );
		container->bufref = bufref;

		if( reader == nullptr )
			container->preader = new( &container->reader ) bf_read;
		else
			container->preader = reader;

		LUA->PushMetaTable( metatype );
		LUA->SetMetaTable( -2 );

		LUA->CreateTable( );
		LUA->SetFEnv( -2 );

		return &container->preader;
	}

	inline Container *GetUserData( GarrysMod::Lua::ILuaBase *LUA, int32_t index )
	{
		global::CheckType( LUA, index, metatype, metaname );
		return LUA->GetUserType<Container>( index, metatype );
	}

	bf_read *Get( GarrysMod::Lua::ILuaBase *LUA, int32_t index, int32_t *bufref )
	{
		Container *container = GetUserData( LUA, index );
		bf_read *reader = container->preader;
		if( reader == nullptr )
			LUA->FormattedError( "invalid %s", metaname );

		if( bufref != nullptr )
			*bufref = container->bufref;

		return reader;
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
		LUA->PushBool( Get( LUA, 1 ) == Get( LUA, 2 ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( tostring )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushFormattedString( global::tostring_format, metaname, buf );
		return 1;
	}

	LUA_FUNCTION_STATIC( IsValid )
	{
		global::CheckType( LUA, 1, metatype, metaname );
		LUA->PushBool( GetUserData( LUA, 1 )->preader != nullptr );
		return 1;
	}

	LUA_FUNCTION_STATIC( GetBasePointer )
	{
		int32_t bufref = LUA_NOREF;
		bf_read *buf = Get( LUA, 1, &bufref );

		if( bufref != LUA_NOREF )
			LUA->ReferencePush( bufref );
		else
			UCHARPTR::Push( LUA, buf->m_nDataBits, const_cast<uint8_t *>( buf->GetBasePointer( ) ) );

		return 1;
	}

	LUA_FUNCTION_STATIC( TotalBytesAvailable )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushNumber( buf->TotalBytesAvailable( ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( GetNumBitsLeft )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushNumber( buf->GetNumBitsLeft( ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( GetNumBytesLeft )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushNumber( buf->GetNumBytesLeft( ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( GetNumBitsRead )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushNumber( buf->GetNumBitsRead( ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( GetNumBytesRead )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushNumber( buf->GetNumBytesRead( ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( IsOverflowed )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushBool( buf->IsOverflowed( ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( Seek )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushBool( buf->Seek( global::GetNumber<int32_t>( LUA, 2, 0 ) ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( SeekRelative )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushBool( buf->SeekRelative( global::GetNumber<int32_t>( LUA, 2 ) ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadBitAngle )
	{
		bf_read *buf = Get( LUA, 1 );
		int32_t bits = global::GetNumber<int32_t>( LUA, 2, 1, 32 );
		LUA->PushNumber( buf->ReadBitAngle( bits ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadAngle )
	{
		bf_read *buf = Get( LUA, 1 );
		QAngle ang;
		buf->ReadBitAngles( ang );
		LUA->PushAngle( ang );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadBits )
	{
		bf_read *buf = Get( LUA, 1 );
		int32_t bits = global::GetNumber<int32_t>( LUA, 2, 1 );
		uint8_t *data = UCHARPTR::Push( LUA, bits );
		buf->ReadBits( data, bits );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadVector )
	{
		bf_read *buf = Get( LUA, 1 );
		Vector vec;
		buf->ReadBitVec3Coord( vec );
		LUA->PushVector( vec );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadNormal )
	{
		bf_read *buf = Get( LUA, 1 );
		Vector vec;
		buf->ReadBitVec3Normal( vec );
		LUA->PushVector( vec );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadByte )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushNumber( buf->ReadByte( ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadBytes )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

		int32_t bytes = global::GetNumber<int32_t>( LUA, 2, 1,
			BitByte( std::numeric_limits<int32_t>::max( ) ) );

		uint8_t *data = UCHARPTR::Push( LUA, bytes * 8 );
		LUA->PushBool( buf->ReadBytes( data, bytes ) );
		return 2;
	}

	LUA_FUNCTION_STATIC( ReadChar )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushNumber( buf->ReadChar( ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadFloat )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushNumber( buf->ReadFloat( ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadDouble )
	{
		bf_read *buf = Get( LUA, 1 );
		double num = 0.0;
		buf->ReadBits( &num, sizeof( double ) * 8 );
		LUA->PushNumber( num );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadLong )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushNumber( buf->ReadLong( ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadLongLong )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushNumber( static_cast<double>( buf->ReadLongLong( ) ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadBit )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushNumber( buf->ReadOneBit( ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadShort )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushNumber( buf->ReadShort( ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadString )
	{
		bf_read *buf = Get( LUA, 1 );
		const int32_t max_size = buf->GetNumBytesLeft( ) + 1;
		std::vector<char> str( static_cast<size_t>( max_size ) );
		int32_t read = 0;
		bool success = buf->ReadString( str.data( ), max_size, false, &read );
		if( success )
			LUA->PushString( str.data( ), static_cast<size_t>( read ) );
		else
			LUA->PushNil( );
		LUA->PushBool( success );
		return 2;
	}

	LUA_FUNCTION_STATIC( ReadInt )
	{
		bf_read *buf = Get( LUA, 1 );
		int32_t bits = global::GetNumber<int32_t>( LUA, 2, 1, 32 );
		LUA->PushNumber( buf->ReadSBitLong( bits ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadUInt )
	{
		bf_read *buf = Get( LUA, 1 );
		int32_t bits = global::GetNumber<int32_t>( LUA, 2, 1, 32 );
		LUA->PushNumber( buf->ReadUBitLong( bits ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadWord )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushNumber( buf->ReadWord( ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadSignedVarInt32 )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushNumber( buf->ReadSignedVarInt32( ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadVarInt32 )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushNumber( buf->ReadVarInt32( ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadSignedVarInt64 )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushNumber( static_cast<double>( buf->ReadSignedVarInt64( ) ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( ReadVarInt64 )
	{
		bf_read *buf = Get( LUA, 1 );
		LUA->PushNumber( static_cast<double>( buf->ReadVarInt64( ) ) );
		return 1;
	}

	LUA_FUNCTION_STATIC( Constructor )
	{
		int32_t bits = 0;
		uint8_t *ptr = UCHARPTR::Get( LUA, 1, &bits );
		if( LUA->Top( ) >= 2 )
			bits = global::GetNumber<int32_t>( LUA, 2, 1, bits );

		LUA->Push( 1 );
		bf_read *reader = *Push( LUA, nullptr, LUA->ReferenceCreate( ) );
		reader->StartReading( ptr, BitByte( bits ), 0, bits );
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
