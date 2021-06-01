include("incoming.lua")

QUERYCVAR_COOKIE = QUERYCVAR_COOKIE or 0

local meta = FindMetaTable("Player")

function meta:GetNetChannel()
	return CNetChan(self:EntIndex())
end

function meta:QueryConVarValue(cvarname)
	local netchan = self:GetNetChannel()
	if netchan == nil then
		return
	end

	local buffer = netchan:GetReliableBuffer()

	QUERYCVAR_COOKIE = QUERYCVAR_COOKIE + 1

	buffer:WriteUInt(svc_GetCvarValue, NET_MESSAGE_BITS)
	buffer:WriteInt(QUERYCVAR_COOKIE, 32)
	buffer:WriteString(cvarname)

	return QUERYCVAR_COOKIE
end

FilterIncomingMessage(clc_RespondCvarValue, function(netchan, read, write)
	write:WriteUInt(clc_RespondCvarValue, NET_MESSAGE_BITS)

	local cookie = read:ReadInt(32) -- 10h
	write:WriteInt(cookie, 32)

	local status = read:ReadInt(4) -- 1Ch
	write:WriteInt(status, 4)

	local cvarname = read:ReadString() -- 14h
	write:WriteString(cvarname)

	local cvarvalue = read:ReadString() -- 18h
	write:WriteString(cvarvalue)

	hook.Call("RespondCvarValue", nil, netchan, cookie, status, cvarname, cvarvalue)
end)
