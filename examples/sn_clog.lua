include("sourcenet/outgoing.lua")

FilterOutgoingMessage(net_StringCmd, function(netchan, read, write)
	local cmd = read:ReadString()

	print(string.format("Sending command \"%s\"", cmd))

	if string.Left(cmd, 6) == "status" then
		print("Stopped status command being sent")
		return
	end

	write:WriteUInt(net_StringCmd, NET_MESSAGE_BITS)
	write:WriteString(cmd)
end)
