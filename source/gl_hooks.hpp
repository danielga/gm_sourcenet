#pragma once

#include <main.hpp>

void UnloadHooks( lua_State *state );

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

typedef bool ( __thiscall *CNetChan_ProcessMessages_T )(
	class CNetChan *netchan,
	class bf_read &buf
);

#elif defined __linux || defined __APPLE__

typedef bool ( *CNetChan_ProcessMessages_T )( class CNetChan *netchan, class bf_read &buf );

#endif

extern CNetChan_ProcessMessages_T CNetChan_ProcessMessages_O;

EXT_GLBL_FUNCTION( Attach__CNetChan_ProcessMessages );
EXT_GLBL_FUNCTION( Detach__CNetChan_ProcessMessages );