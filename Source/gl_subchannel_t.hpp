#ifndef GL_SUBCHANNEL_T_H
#define GL_SUBCHANNEL_T_H

#include <main.hpp>

EXT_META_ID( subchannel_t, 4 );

EXT_META_FUNCTION( subchannel_t, GetFragmentOffset );
EXT_META_FUNCTION( subchannel_t, SetFragmentOffset );

EXT_META_FUNCTION( subchannel_t, GetFragmentNumber );
EXT_META_FUNCTION( subchannel_t, SetFragmentNumber );

EXT_META_FUNCTION( subchannel_t, GetSequence );
EXT_META_FUNCTION( subchannel_t, SetSequence );

EXT_META_FUNCTION( subchannel_t, GetState );
EXT_META_FUNCTION( subchannel_t, SetState );

EXT_META_FUNCTION( subchannel_t, GetIndex );
EXT_META_FUNCTION( subchannel_t, SetIndex );

#endif // GL_SUBCHANNEL_T_H