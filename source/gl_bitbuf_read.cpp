#include <gl_bitbuf_read.hpp>
#include <gl_ucharptr.hpp>
#include <bitbuf.h>

META_ID( sn4_bf_read, 2 );

META_FUNCTION( sn4_bf_read, GetBasePointer )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	PUSH_META( const_cast<uint8_t *>( buf->GetBasePointer( ) ), UCHARPTR );

	return 1;
}

META_FUNCTION( sn4_bf_read, GetNumBitsLeft )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	LUA->PushNumber( buf->GetNumBitsLeft( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, GetNumBytesLeft )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	LUA->PushNumber( buf->GetNumBytesLeft( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, GetNumBitsRead )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	LUA->PushNumber( buf->GetNumBitsRead( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, IsOverflowed )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	LUA->PushBool( buf->IsOverflowed( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadBitAngle )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	LUA->Push( buf->ReadBitAngle( static_cast<int>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadBitAngles )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	QAngle *ang = new QAngle;

	buf->ReadBitAngles( *ang );

	GarrysMod::Lua::UserData *userdata = static_cast<GarrysMod::Lua::UserData *>( LUA->NewUserdata( sizeof( GarrysMod::Lua::UserData ) ) );
	userdata->data = ang;
	userdata->type = GarrysMod::Lua::Type::ANGLE;

	LUA->CreateMetaTableType( "Angle", GarrysMod::Lua::Type::ANGLE );
	LUA->SetMetaTable( -2 );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadBits )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	uint8_t *data = new uint8_t[static_cast<int>( LUA->GetNumber( 2 ) )];
	if( data == nullptr )
	{
		Msg( "[gm_sourcenet4][sn4_bf_read::ReadBits] Failed allocating %i bytes\n", BitByte( static_cast<int>( LUA->GetNumber( 2 ) ) ) );
		LUA->ThrowError( "fatal error" );

		// Prevent further reading of the buffer
		buf->SetOverflowFlag( );
	}

	buf->ReadBits( data, static_cast<int>( LUA->GetNumber( 2 ) ) );

	PUSH_META( data, UCHARPTR );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadBitVec3Coord )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	Vector *vec = new Vector;

	buf->ReadBitVec3Coord( *vec );

	GarrysMod::Lua::UserData *userdata = static_cast<GarrysMod::Lua::UserData *>( LUA->NewUserdata( sizeof( GarrysMod::Lua::UserData ) ) );
	userdata->data = vec;
	userdata->type = GarrysMod::Lua::Type::VECTOR;

	LUA->CreateMetaTableType( "Vector", GarrysMod::Lua::Type::VECTOR );
	LUA->SetMetaTable( -2 );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadByte )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	LUA->PushNumber( buf->ReadByte( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadBytes )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	uint8_t *data = new uint8_t[static_cast<int>( LUA->GetNumber( 2 ) )];
	if( data == nullptr )
	{
		Msg( "[gm_sourcenet4][sn4_bf_read::ReadBytes] Failed allocating %i bytes\n", static_cast<int>( LUA->GetNumber( 2 ) ) );
		LUA->ThrowError( "fatal error" );

		// Prevent further reading of the buffer
		buf->SetOverflowFlag( );
	}

	buf->ReadBytes( data, static_cast<int>( LUA->GetNumber( 2 ) ) );

	PUSH_META( data, UCHARPTR );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadChar )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	LUA->PushNumber( buf->ReadChar( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadFloat )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	LUA->PushNumber( buf->ReadFloat( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadLong )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	LUA->PushNumber( buf->ReadLong( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadOneBit )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	LUA->PushNumber( buf->ReadOneBit( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadShort )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	LUA->PushNumber( buf->ReadShort( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadString )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	char str[1024] = { 0 };

	if( buf->ReadString( str, sizeof( str ) ) )
		LUA->PushString( str );
	else
		LUA->PushNil( );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadSBitLong )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	LUA->PushNumber( buf->ReadSBitLong( static_cast<int>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadUBitLong )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	LUA->PushNumber( buf->ReadUBitLong( static_cast<int>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, ReadWord )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	LUA->PushNumber( buf->ReadWord( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, Seek )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	LUA->PushBool( buf->Seek( static_cast<int>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, SeekRelative )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	LUA->PushBool( buf->SeekRelative( static_cast<int>( LUA->GetNumber( 2 ) ) ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, TotalBytesAvailable )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );

	sn4_bf_read *buf = GET_META( 1, sn4_bf_read );

	LUA->PushNumber( buf->TotalBytesAvailable( ) );

	return 1;
}

META_FUNCTION( sn4_bf_read, FinishReading )
{
	LUA->CheckType( 1, GET_META_ID( sn4_bf_read ) );

	delete GET_META( 1, sn4_bf_read );

	return 0;
}

GLBL_FUNCTION( sn4_bf_read )
{
	LUA->CheckType( 1, GET_META_ID( UCHARPTR ) );
	LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

	int bits = static_cast<int>( LUA->GetNumber( 2 ) );

	sn4_bf_read *buf = new sn4_bf_read( GET_META( 1, UCHARPTR ), BitByte( bits ), bits );

	PUSH_META( buf, sn4_bf_read );

	return 1;
}