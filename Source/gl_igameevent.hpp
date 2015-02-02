#ifndef GL_IGAMEEVENT_H
#define GL_IGAMEEVENT_H

#include <main.hpp>

EXT_META_ID( IGameEvent, 13 );

EXT_META_FUNCTION( IGameEvent, GetName );

EXT_META_FUNCTION( IGameEvent, IsReliable );
EXT_META_FUNCTION( IGameEvent, IsLocal );
EXT_META_FUNCTION( IGameEvent, IsEmpty );

EXT_META_FUNCTION( IGameEvent, GetBool );
EXT_META_FUNCTION( IGameEvent, GetInt );
EXT_META_FUNCTION( IGameEvent, GetFloat );
EXT_META_FUNCTION( IGameEvent, GetString );

EXT_META_FUNCTION( IGameEvent, SetBool );
EXT_META_FUNCTION( IGameEvent, SetInt );
EXT_META_FUNCTION( IGameEvent, SetFloat );
EXT_META_FUNCTION( IGameEvent, SetString );

EXT_META_FUNCTION( IGameEvent, Delete );

#endif // GL_IGAMEEVENT_H