#ifndef SOURCENET3_H
#define SOURCENET3_H

#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/Lua/LuaInterface.h>
#include <cstdint>

#undef LUA
#define LUA static_cast<GarrysMod::Lua::ILuaInterface *>( state->luabase )

// Enable/disable SendDatagram hooking
extern bool g_bPatchedNetChunk;

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

#define CopyUserDataOrNull( arg, meta, dst, dsttype ) \
	int arg___t = LUA->GetType( arg ); \
	if( arg___t == GET_META_ID( meta ) ) \
		dst = static_cast<dsttype>( GET_META( arg, meta ) ); \
	else if ( arg___t == GarrysMod::Lua::Type::NUMBER && static_cast<int>( LUA->GetNumber( arg ) ) == 0 ) \
		dst = static_cast<dsttype>( nullptr ); \
	else \
		LUA->TypeError( GET_META_NAME( meta ), arg )

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

// Multiple Lua state support

#include <utllinkedlist.h>

struct multiStateInfo
{
	lua_State *state;
	int ref_hook_Call;
};

typedef CUtlLinkedList<multiStateInfo> luaStateList_t;

luaStateList_t *GetLuaStates( );

#define BEGIN_MULTISTATE_HOOK( name ) \
{ \
	luaStateList_t *states = GetLuaStates( ); \
	if( states ) \
	{ \
		for( int i = 0; i < states->Count( ); ++i ) \
		{ \
			multiStateInfo msi = states->Element( i ); \
			lua_State *state = msi.state; \
			LUA->ReferencePush( msi.ref_hook_Call ); \
			LUA->PushString( name ); \
			LUA->PushNil( ); \
			int argc = 0

#define DO_MULTISTATE_HOOK( code ) \
			code; \
			++argc

#define CALL_MULTISTATE_HOOK( returns ) \
			LUA->Call( 2 + argc, returns )

#define STOP_MULTISTATE_HOOK( ) \
			break;

#define END_MULTISTATE_HOOK( ) \
		} \
	} \
	else \
	{ \
		Msg( "GetLuaStates() returned NULL\n" ); \
	} \
}

// Source interfaces

#include <interface.h>

extern CreateInterfaceFn fnEngineFactory;

#ifdef IVENGINESERVER_INTERFACE

#include <eiface.h>

extern IVEngineServer *g_pEngineServer;

#endif

#ifdef IVENGINECLIENT_INTERFACE

#include <cdll_int.h>

extern IVEngineClient *g_pEngineClient;

#endif

#ifdef ICVAR_INTERFACE

#include <icvar.h>

extern ICvar *g_pCVarClient;
extern ICvar *g_pCVarServer;

#endif

// Platform definitions

#ifdef _WIN32

#include <windows.h>
#undef GetObject
#undef CreateEvent

#define BEGIN_MEMEDIT( addr, size ) \
{ \
	DWORD previous; \
	VirtualProtect( addr, \
			size, \
			PAGE_EXECUTE_READWRITE, \
			&previous ); \

#define FINISH_MEMEDIT( addr, size ) \
	VirtualProtect( addr, \
			size, \
			previous, \
			NULL ); \
} \

#elif defined _LINUX

#include <sys/mman.h>
#include <unistd.h>

inline unsigned char *PageAlign( unsigned char *addr, long page )
{
	return addr - ( (DWORD)addr % page );
}

#define BEGIN_MEMEDIT( addr, size ) \ 
{ \
	long page = sysconf( _SC_PAGESIZE ); \
	mprotect( PageAlign( (unsigned char *)addr, page ), \
			page, \
			PROT_EXEC | PROT_READ | PROT_WRITE );

#define FINISH_MEMEDIT( addr, size ) \
	mprotect( PageAlign( (unsigned char *)addr, page ), \
			page, \
			PROT_EXEC | PROT_READ ); \
}

#endif

#endif // SOURCENET3_H