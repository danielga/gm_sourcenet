include("sourcenet/incoming.lua")

FilterIncomingMessage(net_Disconnect, function(netchan, read, write)
	local reason = read:ReadString()

	write:WriteUInt(net_Disconnect, NET_MESSAGE_BITS)
	write:WriteString(string.reverse(reason))
end)
