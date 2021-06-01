if SERVER then
	include("server.lua")
else
	include("client.lua")
end

include("netmessages.lua")

-- Initialization
HookNetChannel(
	{name = "CNetChan::SendDatagram"}
)

local function HandleStream(name, netchan, write)
	if not write then
		return false
	end

	local totalbits = write:GetNumBitsWritten()
	if totalbits <= 0 then
		return true
	end

	local read = sn_bf_read(write:GetBasePointer(), totalbits)
	while read:GetNumBitsLeft() >= NET_MESSAGE_BITS do
		local msg = read:ReadUInt(NET_MESSAGE_BITS)
		local handler = NetMessage(netchan, msg, SERVER)
		if not handler then
			MsgC(Color(255, 0, 0), "Unknown outgoing message " .. msg .. " on " .. name .. " stream with " .. read:GetNumBitsLeft() .. " bit(s) left\n")
			return false
		end

		local success, ret = xpcall(handler.ReadFromBuffer, debug.traceback, handler, read)
		if not success then
			MsgC(Color(255, 0, 0), "Failed to filter message " .. handler:GetName() .. " on " .. name .. " stream with " .. read:GetNumBitsLeft() .. " bit(s) left\n" .. ret .. "\n")
			return false
		elseif ret == false then
			MsgC(Color(255, 0, 0), "Failed to filter message " .. handler:GetName() .. " on " .. name .. " stream with " .. read:GetNumBitsLeft() .. " bit(s) left\n")
			return false
		end

		success, ret = xpcall(tostring, debug.traceback, handler)
		if not success then
			MsgC(Color(255, 0, 0), "NetMessage failed tostring from " .. name .. " stream with type " .. handler:GetName() .. "\n")
		end

		MsgC(Color(255, 255, 0), "NetMessage from " .. name .. " stream: " .. tostring(handler) .. "\n")
	end

	local bitsleft = read:GetNumBitsLeft()
	if bitsleft > 0 then
		-- Should be inocuous padding bits but just to be sure, let's copy them
		local data = read:ReadBits(bitsleft)
		write:WriteBits(data)
	end

	MsgC(Color(0, 255, 0), "Fully parsed " .. name .. " stream with " .. totalbits .. " bit(s) written\n")
	return true
end

hook.Add("PreSendDatagram", "OutFilter", function(netchan, localchan, data, reliablestream, unreliablestream, voicestream)
	local islocal = netchan == localchan
	if not game.IsDedicated() and ((islocal and SERVER) or (not islocal and CLIENT)) then
		return
	end

	HandleStream("data", netchan, data)
	HandleStream("reliable", netchan, reliablestream)
	HandleStream("unreliable", netchan, unreliablestream)
	HandleStream("voice", netchan, voicestream)
end)

function FilterOutgoingMessage(msg, func)
	--[[local handler = NET_MESSAGES[msg]
	
	if not handler then
		if CLIENT then
			handler = NET_MESSAGES.CLC[msg]
		else
			handler = NET_MESSAGES.SVC[msg]
		end
	end
	
	if not handler then return end

	handler.OutgoingCopy = func]]
end

function UnFilterOutgoingMessage(msg)
	FilterOutgoingMessage(msg, nil)
end
