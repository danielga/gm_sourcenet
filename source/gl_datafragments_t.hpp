#pragma once

#include <main.hpp>
#include <net.h>

typedef CNetChan::dataFragments_t dataFragments_t;

EXT_META_ID( dataFragments_t, 5 );

EXT_META_FUNCTION( dataFragments_t, GetFileHandle );
EXT_META_FUNCTION( dataFragments_t, SetFileHandle );

EXT_META_FUNCTION( dataFragments_t, GetFileName );
EXT_META_FUNCTION( dataFragments_t, SetFileName );

EXT_META_FUNCTION( dataFragments_t, GetFileTransferID );
EXT_META_FUNCTION( dataFragments_t, SetFileTransferID );

EXT_META_FUNCTION( dataFragments_t, GetBuffer );
EXT_META_FUNCTION( dataFragments_t, SetBuffer );

EXT_META_FUNCTION( dataFragments_t, GetBytes );
EXT_META_FUNCTION( dataFragments_t, SetBytes );

EXT_META_FUNCTION( dataFragments_t, GetBits );
EXT_META_FUNCTION( dataFragments_t, SetBits );

EXT_META_FUNCTION( dataFragments_t, GetActualSize );
EXT_META_FUNCTION( dataFragments_t, SetActualSize );

EXT_META_FUNCTION( dataFragments_t, GetCompressed );
EXT_META_FUNCTION( dataFragments_t, SetCompressed );

EXT_META_FUNCTION( dataFragments_t, GetStream );
EXT_META_FUNCTION( dataFragments_t, SetStream );

EXT_META_FUNCTION( dataFragments_t, GetTotal );
EXT_META_FUNCTION( dataFragments_t, SetTotal );

EXT_META_FUNCTION( dataFragments_t, GetProgress );
EXT_META_FUNCTION( dataFragments_t, SetProgress );

EXT_META_FUNCTION( dataFragments_t, GetNum );
EXT_META_FUNCTION( dataFragments_t, SetNum );

EXT_META_FUNCTION( dataFragments_t, Delete );

EXT_GLBL_FUNCTION( dataFragments_t );