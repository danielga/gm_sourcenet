include( "sn4_base_incoming.lua" )

LUASTRINGS_TABLE_NAME = "pfhremThHuasw"

function ReadUserMessageString( buf )
	if ( buf:ReadOneBit() == 1 ) then
		local container = INetworkStringTableContainer()
		
		if ( !container ) then return end
		
		local pool = container:FindTable( LUASTRINGS_TABLE_NAME )

		if ( !pool ) then return end

		local str = pool:GetString( buf:ReadShort() )
		
		return str || "[STRING NOT POOLED]"
	else
		return buf:ReadString()
	end
end

function ProcessUserMessage( msg, data, bits )
	local buf = sn4_bf_read( data, bits )

	if ( msg == 34 ) then
		local umsgName = ReadUserMessageString( buf )
		
		Msg( string.format( "Received Lua user message: curtime %f, name '%s', bytes %i\n", CurTime(), umsgName, buf:GetNumBytesLeft() ) )
	elseif ( msg == 40 ) then
		local varUnknown = buf:ReadLong()
		local varType = buf:ReadByte()
		local varName = ReadUserMessageString( buf )
		local varValue

		if ( varType == 1 ) then
			varValue = buf:ReadString()
		elseif ( varType == 2 ) then
			varValue = buf:ReadFloat()
		elseif ( varType == 4 ) then
			varValue = buf:ReadOneBit()
		elseif ( varType == 7 ) then
			varValue = "ent " .. buf:ReadLong() & 0x7FF
		elseif ( varType == 8 ) then
			varValue = buf:ReadBitVec3Coord()
		elseif ( varType == 9 ) then
			varValue = buf:ReadBitAngles()
		end

		Msg( string.format( "Received NWVar: curtime %f, type %i, name '%s', value '%s'\n", CurTime(), varType, varName, tostring( varValue ) ) )
	end
	
	buf:FinishReading()
end

FilterIncomingMessage( svc_UserMessage, function( netchan, read, write )
	write:WriteUBitLong( svc_UserMessage, NET_MESSAGE_BITS )
	
	local msg = read:ReadByte()
	local bits = read:ReadUBitLong( 11 )
	local data = read:ReadBits( bits )

	ProcessUserMessage( msg, data, bits )

	write:WriteByte( msg )
	write:WriteUBitLong( bits, 11 )
	write:WriteBits( data, bits )

	data:Delete()
end )