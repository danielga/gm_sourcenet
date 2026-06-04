include("sourcenet/incoming.lua")

FilterIncomingMessage(net_StringCmd, function(netchan, read, write)
	local cmd = read:ReadString()

	print(string.format("Client %s ran command \"%s\"", netchan:GetAddress():ToString(), cmd))

	if string.Left(cmd, 6) == "status" then
		print("Blocked status command")
		return
	end

	write:WriteUInt(net_StringCmd, NET_MESSAGE_BITS)
	write:WriteString(cmd)
end)
