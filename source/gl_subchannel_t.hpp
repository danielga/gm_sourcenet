#pragma once

#include <main.hpp>

typedef struct subchannel_s subchannel_t;
class CNetChan;

void Push_subchannel( lua_State *state, subchannel_t *subchan, CNetChan *netchan );

EXT_META_ID( subchannel_t, 4 );

EXT_META_FUNCTION( subchannel_t, __eq );
EXT_META_FUNCTION( subchannel_t, __tostring );

EXT_META_FUNCTION( subchannel_t, IsValid );

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