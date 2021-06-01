require("sourcenet")

concommand.Add("loadcodec", function(ply, cmd, args)
	if not IsValid(ply) or args[1] == nil then
		return
	end

	local buffer = CNetChan(ply:EntIndex()):GetReliableBuffer()

	buffer:WriteUInt(svc_VoiceInit, NET_MESSAGE_BITS)
	buffer:WriteString(args[1])
	buffer:WriteByte(1)
end)
