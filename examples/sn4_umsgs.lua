require( "sourcenet4" )

function _R.Player:GetNetChannel()
	return CNetChan( self:EntIndex() )
end

function _R.Player:Test()
	local netchan = self:GetNetChannel()
	
	if ( !netchan ) then return end

	local reliablebuffer = netchan:GetReliableBuffer()

	reliablebuffer:WriteUBitLong( svc_UserMessage, NET_MESSAGE_BITS )
	reliablebuffer:WriteByte( 19 )

	local msgdata = UCHARPTR( 256 )
	local msgbuffer = sn4_bf_write( msgdata, 256 * 8 )

	msgbuffer:WriteLong( -1 )
	msgbuffer:WriteShort( 100 )
	msgbuffer:WriteByte( 0 )

	reliablebuffer:WriteUBitLong( msgbuffer:GetNumBitsWritten(), 11 )
	reliablebuffer:WriteBits( msgdata, msgbuffer:GetNumBitsWritten() )
	
	msgdata:Delete()
	msgbuffer:FinishWriting()
end