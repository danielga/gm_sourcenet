include("sourcenet/incoming.lua")

LUASTRINGS_TABLE_NAME = "networkstring"

function ReadUserMessageString(buf)
	if buf:ReadBit() == 1 then
		local container = INetworkStringTableContainer()
		if container == nil then
			return
		end

		local pool = container:FindTable(LUASTRINGS_TABLE_NAME)
		if pool == nil then
			return
		end

		local str = pool:GetString(buf:ReadShort())
		return str or "[STRING NOT POOLED]"
	else
		return buf:ReadString()
	end
end

function ProcessUserMessage(msg, data)
	local buf = sn_bf_read(data)

	if msg == 34 then
		local umsgName = ReadUserMessageString(buf)
		Msg(string.format("Received Lua user message: curtime %f, name '%s', bytes %i\n", CurTime(), umsgName, buf:GetNumBytesLeft()))
	elseif msg == 40 then
		buf:ReadLong()
		local varType = buf:ReadByte()
		local varName = ReadUserMessageString(buf)
		local varValue

		if varType == 1 then
			varValue = buf:ReadString()
		elseif varType == 2 then
			varValue = buf:ReadFloat()
		elseif varType == 4 then
			varValue = buf:ReadBit()
		elseif varType == 7 then
			varValue = "ent " .. bit.band(buf:ReadLong(), 0x7FF)
		elseif varType == 8 then
			varValue = buf:ReadVector()
		elseif varType == 9 then
			varValue = buf:ReadAngle()
		end

		Msg(string.format("Received NWVar: curtime %f, type %i, name '%s', value '%s'\n", CurTime(), varType, varName, tostring(varValue)))
	end
end

FilterIncomingMessage(svc_UserMessage, function(netchan, read, write)
	write:WriteUInt(svc_UserMessage, NET_MESSAGE_BITS)

	local msg = read:ReadByte()
	local bits = read:ReadUInt(11)
	local data = read:ReadBits(bits)

	ProcessUserMessage(msg, data)

	write:WriteByte(msg)
	write:WriteUInt(bits, 11)
	write:WriteBits(data)
end)
