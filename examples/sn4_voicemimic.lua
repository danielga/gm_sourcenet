include( "sn4_base_incoming.lua" )

FilterIncomingMessage( svc_VoiceData, function( netchan, read, write )
	write:WriteUBitLong( svc_VoiceData, NET_MESSAGE_BITS )

	local client = read:ReadByte()		
	write:WriteByte( client )

	local proximity = read:ReadByte()
	write:WriteByte( proximity )

	local bits = read:ReadWord()
	write:WriteWord( bits )

	local voicedata = read:ReadBits( bits )
	write:WriteBits( voicedata, bits )
	
	if ( client != LocalPlayer():EntIndex() - 1 ) then
		local voicebuf = CNetChan():GetVoiceBuffer()
		
		voicebuf:WriteUBitLong( clc_VoiceData, NET_MESSAGE_BITS )
		voicebuf:WriteWord( bits )
		voicebuf:WriteBits( voicedata, bits )

		CNetChan():Transmit()
	end

	voicedata:Delete()
end )