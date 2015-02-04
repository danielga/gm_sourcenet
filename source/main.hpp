#pragma once

#include <GarrysMod/Lua/Interface.h>
#include <interface.h>
#include <utllinkedlist.h>
#include <cstdint>

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

extern void TypeError( lua_State *state, const char *name, int index );

// Utility macros

#define SOURCENET_META_BASE 100

#define VerifyStream( stream ) \
	if( stream < 0 || stream >= MAX_STREAMS ) \
	{ \
		LUA->PushNil( ); \
		return 1; \
	}

#define VerifyOffset( vec, offset ) \
	if( offset < 0 || offset >= vec.Count( ) ) \
	{ \
		LUA->PushNil( ); \
		return 1; \
	}

#define GET_META( index, name )	static_cast<name *>( static_cast<GarrysMod::Lua::UserData *>( LUA->GetUserdata( index ) )->data )

#define PUSH_META( _data, name ) \
	{ \
		if( _data != nullptr ) \
		{ \
			GarrysMod::Lua::UserData *userdata = static_cast<GarrysMod::Lua::UserData *>( LUA->NewUserdata( sizeof( GarrysMod::Lua::UserData ) ) ); \
			userdata->data = _data; \
			userdata->type = GET_META_ID( name ); \
			LUA->CreateMetaTableType( GET_META_NAME( name ), GET_META_ID( name ) ); \
			LUA->SetMetaTable( -2 ); \
		} \
		else \
		{ \
			LUA->PushNil( ); \
		} \
	} \

#define META_FUNCTION( meta, name ) LUA_FUNCTION( meta##__##name )

#define	META_ID( name, id ) \
	const int META_##name##_id = SOURCENET_META_BASE + id; \
	const char *META_##name##_name = #name

#define EXT_META_FUNCTION( meta, name ) extern META_FUNCTION( meta, name )

#define EXT_META_ID( name, id ) \
	extern const int META_##name##_id; \
	extern const char *META_##name##_name

#define GET_META_ID( name ) META_##name##_id
#define GET_META_NAME( name ) META_##name##_name

#define BEGIN_META_REGISTRATION( name ) \
	{ \
		LUA->CreateMetaTableType( GET_META_NAME( name ), GET_META_ID( name ) )

#define REG_META_FUNCTION( meta, name ) \
	LUA->PushCFunction( meta##__##name ); \
	LUA->SetField( -2, #name )

#define REG_META_CALLBACK( meta, name ) \
	LUA->PushCFunction( meta##__##name ); \
	LUA->SetField( -2, #name )

#define END_META_REGISTRATION( ) \
		LUA->Push( -1 ); \
		LUA->SetField( -1, "__index" ); \
		LUA->Pop( 1 ); \
	}

#define BEGIN_ENUM_REGISTRATION( name ) \
	{ \
		const char *__name = #name; \
		LUA->CreateTable( )

#define REG_ENUM( name, value ) \
	LUA->PushNumber( static_cast<double>( value ) ); \
	LUA->SetField( -2, #name )

#define END_ENUM_REGISTRATION( ) \
		LUA->SetField( -2, __name ); \
	}

#define GLBL_FUNCTION( name ) LUA_FUNCTION( _G__##name )
#define EXT_GLBL_FUNCTION( name ) extern GLBL_FUNCTION( name )

#define REG_GLBL_FUNCTION( name ) \
	LUA->PushCFunction( _G__##name ); \
	LUA->SetField( -2, #name )

#define REG_GLBL_NUMBER( name ) \
	LUA->PushNumber( static_cast<double>( name ) ); \
	LUA->SetField( -2, #name )

#define REG_GLBL_STRING( name ) \
	LUA->PushString( static_cast<const char *>( name ) ); \
	LUA->SetField( -2, #name )