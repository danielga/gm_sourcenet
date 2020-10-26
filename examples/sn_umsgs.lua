require("sourcenet")

local PLAYER = FindMetaTable("Player")
function PLAYER:GetNetChannel()
	return CNetChan(self:EntIndex())
end

function PLAYER:Test()
	local netchan = self:GetNetChannel()
	if netchan == nil then
		return
	end

	local reliablebuffer = netchan:GetReliableBuffer()

	reliablebuffer:WriteUInt(svc_UserMessage, NET_MESSAGE_BITS)
	reliablebuffer:WriteByte(19)

	local msgdata = UCHARPTR(256)
	local msgbuffer = sn_bf_write(msgdata)

	msgbuffer:WriteLong(-1)
	msgbuffer:WriteShort(100)
	msgbuffer:WriteByte(0)

	reliablebuffer:WriteUInt(msgbuffer:GetNumBitsWritten(), 11)
	reliablebuffer:WriteBits(msgdata)
end
