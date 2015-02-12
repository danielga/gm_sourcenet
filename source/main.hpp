#pragma once

#include <GarrysMod/Lua/Interface.h>
#include <interface.h>
#include <cstdint>

extern "C"
{

#include <lua.h>

}

extern lua_State *global_state;

// Source interfaces

extern CreateInterfaceFn fnEngineFactory;

#if defined IVENGINESERVER_INTERFACE

#include <eiface.h>

extern IVEngineServer *g_pEngineServer;

#endif

#if defined IVENGINECLIENT_INTERFACE

#include <cdll_int.h>

extern IVEngineClient *g_pEngineClient;

#endif

#if defined ISERVER_INTERFACE

#include <iserver.h>

extern IServer *g_pServer;

#endif

#if defined __linux || defined __APPLE__

#undef min
#undef max

#endif

extern void TypeError( lua_State *state, const char *name, int32_t index );

// Utility macros

#define SOURCENET_META_BASE 100

#define GLBL_FUNCTION( name ) LUA_FUNCTION( _G__##name )
#define EXT_GLBL_FUNCTION( name ) extern GLBL_FUNCTION( name )

#define META_FUNCTION( meta, name ) LUA_FUNCTION( meta##__##name )
#define EXT_META_FUNCTION( meta, name ) extern META_FUNCTION( meta, name )

#define	META_ID( name, id ) \
	const uint8_t META_##name##_id = SOURCENET_META_BASE + id; \
	const char *META_##name##_name = #name

#define EXT_META_ID( name, id ) \
	extern const uint8_t META_##name##_id; \
	extern const char *META_##name##_name

#define GET_META_ID( name ) META_##name##_id
#define GET_META_NAME( name ) META_##name##_name