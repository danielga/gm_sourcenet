#pragma once

#include <main.hpp>
#include <bitbuf.h>

typedef bf_read sn4_bf_read;

sn4_bf_read **Push_sn4_bf_read(
	lua_State *state,
	sn4_bf_read *reader = nullptr,
	int32_t bufref = -1
);

sn4_bf_read *Get_sn4_bf_read( lua_State *state, int32_t index, int32_t *bufref = nullptr );

EXT_META_ID( sn4_bf_read, 2 );

EXT_META_FUNCTION( sn4_bf_read, __gc );
EXT_META_FUNCTION( sn4_bf_read, __eq );
EXT_META_FUNCTION( sn4_bf_read, __tostring );

EXT_META_FUNCTION( sn4_bf_read, IsValid );

EXT_META_FUNCTION( sn4_bf_read, GetBasePointer );
EXT_META_FUNCTION( sn4_bf_read, TotalBytesAvailable );
EXT_META_FUNCTION( sn4_bf_read, GetNumBitsLeft );
EXT_META_FUNCTION( sn4_bf_read, GetNumBytesLeft );
EXT_META_FUNCTION( sn4_bf_read, GetNumBitsRead );
EXT_META_FUNCTION( sn4_bf_read, GetNumBytesRead );

EXT_META_FUNCTION( sn4_bf_read, IsOverflowed );

EXT_META_FUNCTION( sn4_bf_read, Seek );
EXT_META_FUNCTION( sn4_bf_read, SeekRelative );

EXT_META_FUNCTION( sn4_bf_read, ReadBitAngle );
EXT_META_FUNCTION( sn4_bf_read, ReadAngle );
EXT_META_FUNCTION( sn4_bf_read, ReadBits );
EXT_META_FUNCTION( sn4_bf_read, ReadVector );
EXT_META_FUNCTION( sn4_bf_read, ReadNormal );
EXT_META_FUNCTION( sn4_bf_read, ReadByte );
EXT_META_FUNCTION( sn4_bf_read, ReadBytes );
EXT_META_FUNCTION( sn4_bf_read, ReadChar );
EXT_META_FUNCTION( sn4_bf_read, ReadFloat );
EXT_META_FUNCTION( sn4_bf_read, ReadDouble );
EXT_META_FUNCTION( sn4_bf_read, ReadLong );
EXT_META_FUNCTION( sn4_bf_read, ReadLongLong );
EXT_META_FUNCTION( sn4_bf_read, ReadBit );
EXT_META_FUNCTION( sn4_bf_read, ReadShort );
EXT_META_FUNCTION( sn4_bf_read, ReadString );
EXT_META_FUNCTION( sn4_bf_read, ReadInt );
EXT_META_FUNCTION( sn4_bf_read, ReadUInt );
EXT_META_FUNCTION( sn4_bf_read, ReadWord );
EXT_META_FUNCTION( sn4_bf_read, ReadSignedVarInt32 );
EXT_META_FUNCTION( sn4_bf_read, ReadVarInt32 );
EXT_META_FUNCTION( sn4_bf_read, ReadSignedVarInt64 );
EXT_META_FUNCTION( sn4_bf_read, ReadVarInt64 );

EXT_GLBL_FUNCTION( sn4_bf_read );