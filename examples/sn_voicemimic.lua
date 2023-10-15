include("sourcenet/incoming.lua")

FilterIncomingMessage(svc_VoiceData, function(netchan, read, write)
	write:WriteUInt(svc_VoiceData, NET_MESSAGE_BITS)

	local client = read:ReadByte()
	write:WriteByte(client)

	local proximity = read:ReadByte()
	write:WriteByte(proximity)

	local bits = read:ReadWord()
	write:WriteWord(bits)

	local voicedata = read:ReadBits(bits)
	write:WriteBits(voicedata)

	if client ~= LocalPlayer():EntIndex() - 1 then
		local voicebuf = netchan:GetVoiceBuffer()

		voicebuf:WriteUInt(clc_VoiceData, NET_MESSAGE_BITS)
		voicebuf:WriteWord(bits)
		voicebuf:WriteBits(voicedata)

		netchan:Transmit()
	end
end)
