require("sourcenet4")

function CustomDisconnect(reason)
	local netchan = CNetChan()
	local buf = netchan:GetReliableBuffer()

	buf:WriteUInt(net_Disconnect, NET_MESSAGE_BITS)
	buf:WriteString(reason)

	netchan:Transmit()
end