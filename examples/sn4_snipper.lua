include("sn4_base_outgoing.lua")

FilterOutgoingMessage(svc_Print, function( netchan, read, write)
	local msg = read:ReadString():gsub("STEAM_0:1:26262689", "-snip-")

	print(string.format("Sending message \"%s\" to %s", msg, netchan:GetAddress():ToString()))

	write:WriteUInt(svc_Print, NET_MESSAGE_BITS)
	write:WriteString(msg)
end )