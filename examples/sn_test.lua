include("sn_base_incoming.lua")
include("sn_base_outgoing.lua")

FilterOutgoingMessage(net_StringCmd, function(netchan, read, write)
	local cmd = read:ReadString()
	
	print(string.format("Sending command \"%s\"", cmd))

	write:WriteUInt(net_StringCmd, NET_MESSAGE_BITS)
	write:WriteString(cmd)
end)

FilterIncomingMessage(net_StringCmd, function(netchan, read, write)
	local cmd = read:ReadString()
	
	print(string.format("Executing command \"%s\"", cmd))

	write:WriteUInt(net_StringCmd, NET_MESSAGE_BITS)
	write:WriteString(cmd)
end)