#ifndef GL_BITBUF_WRITE_H
#define GL_BITBUF_WRITE_H

#include <main.hpp>

#include <bitbuf.h>

typedef bf_write sn4_bf_write;

EXT_META_ID( sn4_bf_write, 1 );

EXT_META_FUNCTION( sn4_bf_write, GetBasePointer );
EXT_META_FUNCTION( sn4_bf_write, GetMaxNumBits );
EXT_META_FUNCTION( sn4_bf_write, GetNumBitsWritten );
EXT_META_FUNCTION( sn4_bf_write, GetNumBytesWritten );
EXT_META_FUNCTION( sn4_bf_write, GetNumBitsLeft );
EXT_META_FUNCTION( sn4_bf_write, GetNumBytesLeft );

EXT_META_FUNCTION( sn4_bf_write, IsOverflowed );

EXT_META_FUNCTION( sn4_bf_write, Seek );

EXT_META_FUNCTION( sn4_bf_write, WriteBitAngle );
EXT_META_FUNCTION( sn4_bf_write, WriteBits );
EXT_META_FUNCTION( sn4_bf_write, WriteBitVec3Coord );
EXT_META_FUNCTION( sn4_bf_write, WriteByte );
EXT_META_FUNCTION( sn4_bf_write, WriteBytes );
EXT_META_FUNCTION( sn4_bf_write, WriteChar );
EXT_META_FUNCTION( sn4_bf_write, WriteFloat );
EXT_META_FUNCTION( sn4_bf_write, WriteLong );
EXT_META_FUNCTION( sn4_bf_write, WriteOneBit );
EXT_META_FUNCTION( sn4_bf_write, WriteShort );
EXT_META_FUNCTION( sn4_bf_write, WriteString );
EXT_META_FUNCTION( sn4_bf_write, WriteSBitLong );
EXT_META_FUNCTION( sn4_bf_write, WriteUBitLong );
EXT_META_FUNCTION( sn4_bf_write, WriteWord );

EXT_META_FUNCTION( sn4_bf_write, FinishWriting );

EXT_GLBL_FUNCTION( sn4_bf_write );

#endif // GL_BITBUF_WRITE_H