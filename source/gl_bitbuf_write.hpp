#pragma once

#include <main.hpp>
#include <bitbuf.h>

typedef bf_write sn4_bf_write;

sn4_bf_write **Push_sn4_bf_write( lua_State *state, sn4_bf_write *writer = nullptr, int32_t bufref = -1 );

sn4_bf_write *Get_sn4_bf_write( lua_State *state, int32_t index );

EXT_META_ID( sn4_bf_write, 1 );

EXT_META_FUNCTION( sn4_bf_write, __gc );
EXT_META_FUNCTION( sn4_bf_write, __eq );
EXT_META_FUNCTION( sn4_bf_write, __tostring );

EXT_META_FUNCTION( sn4_bf_write, IsValid );

EXT_META_FUNCTION( sn4_bf_write, GetBasePointer );
EXT_META_FUNCTION( sn4_bf_write, GetMaxNumBits );
EXT_META_FUNCTION( sn4_bf_write, GetNumBitsWritten );
EXT_META_FUNCTION( sn4_bf_write, GetNumBytesWritten );
EXT_META_FUNCTION( sn4_bf_write, GetNumBitsLeft );
EXT_META_FUNCTION( sn4_bf_write, GetNumBytesLeft );

EXT_META_FUNCTION( sn4_bf_write, IsOverflowed );

EXT_META_FUNCTION( sn4_bf_write, Seek );

EXT_META_FUNCTION( sn4_bf_write, WriteBitAngle );
EXT_META_FUNCTION( sn4_bf_write, WriteAngle );
EXT_META_FUNCTION( sn4_bf_write, WriteBits );
EXT_META_FUNCTION( sn4_bf_write, WriteVector );
EXT_META_FUNCTION( sn4_bf_write, WriteNormal );
EXT_META_FUNCTION( sn4_bf_write, WriteByte );
EXT_META_FUNCTION( sn4_bf_write, WriteBytes );
EXT_META_FUNCTION( sn4_bf_write, WriteChar );
EXT_META_FUNCTION( sn4_bf_write, WriteFloat );
EXT_META_FUNCTION( sn4_bf_write, WriteDouble );
EXT_META_FUNCTION( sn4_bf_write, WriteLong );
EXT_META_FUNCTION( sn4_bf_write, WriteLongLong );
EXT_META_FUNCTION( sn4_bf_write, WriteBit );
EXT_META_FUNCTION( sn4_bf_write, WriteShort );
EXT_META_FUNCTION( sn4_bf_write, WriteString );
EXT_META_FUNCTION( sn4_bf_write, WriteInt );
EXT_META_FUNCTION( sn4_bf_write, WriteUInt );
EXT_META_FUNCTION( sn4_bf_write, WriteWord );
EXT_META_FUNCTION( sn4_bf_write, WriteSignedVarInt32 );
EXT_META_FUNCTION( sn4_bf_write, WriteVarInt32 );
EXT_META_FUNCTION( sn4_bf_write, WriteSignedVarInt64 );
EXT_META_FUNCTION( sn4_bf_write, WriteVarInt64 );

EXT_GLBL_FUNCTION( sn4_bf_write );