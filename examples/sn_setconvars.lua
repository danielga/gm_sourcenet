require("sourcenet")

local PLAYER = FindMetaTable("Player")
function PLAYER:GetNetChannel()
	return CNetChan(self:EntIndex())
end

function PLAYER:SetConVar(name, value)
	local netchan = self:GetNetChannel()
	if netchan == nil then
		return
	end

	local buf = netchan:GetReliableBuffer()

	buf:WriteUInt(net_SetConVar, NET_MESSAGE_BITS)
	buf:WriteByte(1)
	buf:WriteString(name)
	buf:WriteString(value)
end
