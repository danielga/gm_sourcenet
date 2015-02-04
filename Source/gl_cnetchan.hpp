#pragma once

#include <main.hpp>

EXT_META_ID( CNetChan, 3 );

EXT_META_FUNCTION( CNetChan, DumpMessages );

EXT_META_FUNCTION( CNetChan, Reset );
EXT_META_FUNCTION( CNetChan, Clear );
EXT_META_FUNCTION( CNetChan, Shutdown );
EXT_META_FUNCTION( CNetChan, Transmit );

EXT_META_FUNCTION( CNetChan, SendFile );
EXT_META_FUNCTION( CNetChan, DenyFile );
EXT_META_FUNCTION( CNetChan, RequestFile );

EXT_META_FUNCTION( CNetChan, GetOutgoingQueueSize );
EXT_META_FUNCTION( CNetChan, GetOutgoingQueueFragments );
EXT_META_FUNCTION( CNetChan, QueueOutgoingFragments );

EXT_META_FUNCTION( CNetChan, GetIncomingFragments );

EXT_META_FUNCTION( CNetChan, GetSubChannels );

EXT_META_FUNCTION( CNetChan, GetReliableBuffer );
EXT_META_FUNCTION( CNetChan, GetUnreliableBuffer );
EXT_META_FUNCTION( CNetChan, GetVoiceBuffer );

EXT_META_FUNCTION( CNetChan, GetNetChannelHandler );
EXT_META_FUNCTION( CNetChan, GetAddress );
EXT_META_FUNCTION( CNetChan, GetTime );
EXT_META_FUNCTION( CNetChan, GetLatency );
EXT_META_FUNCTION( CNetChan, GetAvgLatency );
EXT_META_FUNCTION( CNetChan, GetAvgLoss );
EXT_META_FUNCTION( CNetChan, GetAvgChoke );
EXT_META_FUNCTION( CNetChan, GetAvgData );
EXT_META_FUNCTION( CNetChan, GetAvgPackets );
EXT_META_FUNCTION( CNetChan, GetTotalData );
EXT_META_FUNCTION( CNetChan, GetSequenceNr );
EXT_META_FUNCTION( CNetChan, IsValidPacket );
EXT_META_FUNCTION( CNetChan, GetPacketTime );
EXT_META_FUNCTION( CNetChan, GetPacketBytes );
EXT_META_FUNCTION( CNetChan, GetStreamProgress );
EXT_META_FUNCTION( CNetChan, GetCommandInterpolationAmount );
EXT_META_FUNCTION( CNetChan, GetPacketResponseLatency );
EXT_META_FUNCTION( CNetChan, GetRemoteFramerate );

EXT_META_FUNCTION( CNetChan, SetInterpolationAmount );
EXT_META_FUNCTION( CNetChan, SetRemoteFramerate );
EXT_META_FUNCTION( CNetChan, SetMaxBufferSize );

EXT_META_FUNCTION( CNetChan, IsPlayback );

EXT_META_FUNCTION( CNetChan, GetTimeoutSeconds );
EXT_META_FUNCTION( CNetChan, SetTimeoutSeconds );

EXT_META_FUNCTION( CNetChan, GetConnectTime );
EXT_META_FUNCTION( CNetChan, SetConnectTime );

EXT_META_FUNCTION( CNetChan, GetLastReceivedTime );
EXT_META_FUNCTION( CNetChan, SetLastReceivedTime );

EXT_META_FUNCTION( CNetChan, GetName );
EXT_META_FUNCTION( CNetChan, SetName );

EXT_META_FUNCTION( CNetChan, GetRate );
EXT_META_FUNCTION( CNetChan, SetRate );

EXT_META_FUNCTION( CNetChan, GetBackgroundMode );
EXT_META_FUNCTION( CNetChan, SetBackgroundMode );

EXT_META_FUNCTION( CNetChan, GetCompressionMode );
EXT_META_FUNCTION( CNetChan, SetCompressionMode );

EXT_META_FUNCTION( CNetChan, GetMaxRoutablePayloadSize );
EXT_META_FUNCTION( CNetChan, SetMaxRoutablePayloadSize );

EXT_META_FUNCTION( CNetChan, __eq );

EXT_GLBL_FUNCTION( CNetChan );