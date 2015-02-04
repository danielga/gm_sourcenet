#pragma once

#include <main.hpp>
#include <bitbuf.h>

typedef bf_read sn4_bf_read;

EXT_META_ID( sn4_bf_read, 2 );

EXT_META_FUNCTION( sn4_bf_read, GetBasePointer );
EXT_META_FUNCTION( sn4_bf_read, GetNumBitsLeft );
EXT_META_FUNCTION( sn4_bf_read, GetNumBytesLeft );
EXT_META_FUNCTION( sn4_bf_read, GetNumBitsRead );

EXT_META_FUNCTION( sn4_bf_read, IsOverflowed );

EXT_META_FUNCTION( sn4_bf_read, ReadBitAngle );
EXT_META_FUNCTION( sn4_bf_read, ReadBitAngles );
EXT_META_FUNCTION( sn4_bf_read, ReadBits );
EXT_META_FUNCTION( sn4_bf_read, ReadBitVec3Coord );
EXT_META_FUNCTION( sn4_bf_read, ReadByte );
EXT_META_FUNCTION( sn4_bf_read, ReadBytes );
EXT_META_FUNCTION( sn4_bf_read, ReadChar );
EXT_META_FUNCTION( sn4_bf_read, ReadFloat );
EXT_META_FUNCTION( sn4_bf_read, ReadLong );
EXT_META_FUNCTION( sn4_bf_read, ReadOneBit );
EXT_META_FUNCTION( sn4_bf_read, ReadShort );
EXT_META_FUNCTION( sn4_bf_read, ReadString );
EXT_META_FUNCTION( sn4_bf_read, ReadSBitLong );
EXT_META_FUNCTION( sn4_bf_read, ReadUBitLong );
EXT_META_FUNCTION( sn4_bf_read, ReadWord );

EXT_META_FUNCTION( sn4_bf_read, Seek );
EXT_META_FUNCTION( sn4_bf_read, SeekRelative );

EXT_META_FUNCTION( sn4_bf_read, TotalBytesAvailable );

EXT_META_FUNCTION( sn4_bf_read, FinishReading );

EXT_GLBL_FUNCTION( sn4_bf_read );