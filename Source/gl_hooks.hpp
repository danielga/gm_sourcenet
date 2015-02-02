#ifndef GL_HOOKS_H
#define GL_HOOKS_H

#include <main.hpp>

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

extern void *CNetChan_ProcessMessages_T;

EXT_GLBL_FUNCTION( Attach__CNetChan_ProcessMessages );
EXT_GLBL_FUNCTION( Detach__CNetChan_ProcessMessages );

#endif // GL_HOOKS_H