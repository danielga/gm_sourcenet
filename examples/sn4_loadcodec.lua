require("sourcenet4" )

concommand.Add("loadcodec", function(ply, cmd, args)
	if not IsValid(ply) then return end
	if not args[1] then return end

	local buffer = CNetChan(ply:EntIndex()):GetReliableBuffer()
	
	buffer:WriteUInt(svc_VoiceInit, NET_MESSAGE_BITS)
	buffer:WriteString(args[1])
	buffer:WriteByte(1)
end )