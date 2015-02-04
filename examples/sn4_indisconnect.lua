include( "sn4_base_incoming.lua" )

FilterIncomingMessage( net_Disconnect, function( netchan, read, write )
	local reason = read:ReadString()

	write:WriteUBitLong( net_Disconnect, NET_MESSAGE_BITS )
	write:WriteString( string.reverse( reason ) )
end )