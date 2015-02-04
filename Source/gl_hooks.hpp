#pragma once

#include <main.hpp>

#define INIT_HOOK( name ) \
{ \
	lua_State *state = global_state; \
	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB ); \
	LUA->GetField( -1, "hook" ); \
	LUA->Remove( -2 ); \
	LUA->GetField( -1, "Call" ); \
	LUA->Remove( -2 ); \
	LUA->PushString( name ); \
	LUA->PushNil( ); \
	int argc = 0

#define DO_HOOK( code ) \
	code; \
	++argc

#define CALL_HOOK( returns ) \
	LUA->Call( 2 + argc, returns ); \
}

#define EXT_MONITOR_HOOK( name ) \
	extern bool attach_status__##name

#define MONITOR_HOOK( name ) \
	bool attach_status__##name = false

#define IS_ATTACHED( name ) \
	attach_status__##name

#define REGISTER_ATTACH( name ) \
	Msg( "Attach: %s\n", #name ); \
	attach_status__##name = true

#define REGISTER_DETACH( name ) \
	Msg( "Detach: %s\n", #name ); \
	attach_status__##name = false

EXT_MONITOR_HOOK( CNetChan_ProcessMessages );

EXT_GLBL_FUNCTION( Attach__CNetChan_SendDatagram );
EXT_GLBL_FUNCTION( Detach__CNetChan_SendDatagram );

EXT_GLBL_FUNCTION( Attach__CNetChan_ProcessPacket );
EXT_GLBL_FUNCTION( Detach__CNetChan_ProcessPacket );

EXT_GLBL_FUNCTION( Attach__CNetChan_Shutdown );
EXT_GLBL_FUNCTION( Detach__CNetChan_Shutdown );

EXT_GLBL_FUNCTION( Attach__INetChannelHandler_ConnectionStart );
EXT_GLBL_FUNCTION( Detach__INetChannelHandler_ConnectionStart );

EXT_GLBL_FUNCTION( Attach__INetChannelHandler_ConnectionClosing );
EXT_GLBL_FUNCTION( Detach__INetChannelHandler_ConnectionClosing );

EXT_GLBL_FUNCTION( Attach__INetChannelHandler_ConnectionCrashed );
EXT_GLBL_FUNCTION( Detach__INetChannelHandler_ConnectionCrashed );

EXT_GLBL_FUNCTION( Attach__INetChannelHandler_PacketStart );
EXT_GLBL_FUNCTION( Detach__INetChannelHandler_PacketStart );

EXT_GLBL_FUNCTION( Attach__INetChannelHandler_PacketEnd );
EXT_GLBL_FUNCTION( Detach__INetChannelHandler_PacketEnd );

EXT_GLBL_FUNCTION( Attach__INetChannelHandler_FileRequested );
EXT_GLBL_FUNCTION( Detach__INetChannelHandler_FileRequested );

EXT_GLBL_FUNCTION( Attach__INetChannelHandler_FileReceived );
EXT_GLBL_FUNCTION( Detach__INetChannelHandler_FileReceived );

EXT_GLBL_FUNCTION( Attach__INetChannelHandler_FileDenied );
EXT_GLBL_FUNCTION( Detach__INetChannelHandler_FileDenied );

#if defined _WIN32

typedef bool ( __thiscall *CNetChan_ProcessMessages_T )( class CNetChan *netchan, class bf_read &buf );

#elif defined __linux || defined __APPLE__

typedef bool ( *CNetChan_ProcessMessages_T )( class CNetChan *netchan, class bf_read &buf );

#endif

extern CNetChan_ProcessMessages_T CNetChan_ProcessMessages_O;

EXT_GLBL_FUNCTION( Attach__CNetChan_ProcessMessages );
EXT_GLBL_FUNCTION( Detach__CNetChan_ProcessMessages );