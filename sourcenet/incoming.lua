if SERVER then
	include("server.lua")
else
	include("client.lua")
end

include("netmessages.lua")

-- Initialization
HookNetChannel(
	-- nochan prevents a net channel being passed to the attach/detach functions
	-- CNetChan::ProcessMessages doesn't use a virtual hook, so we don't need to pass the net channel
	{name = "CNetChan::ProcessMessages", nochan = true}
)

hook.Add("PreProcessMessages", "InFilter", function(netchan, read, write, localchan)
	local islocal = netchan == localchan
	if not game.IsDedicated() and ((islocal and SERVER) or (not islocal and CLIENT)) then
		MsgC(Color(255, 255, 0), "Ignoring a stream\n")
		return false
	end

	local totalbits = read:GetNumBitsLeft()

	while read:GetNumBitsLeft() >= NET_MESSAGE_BITS do
		local msg = read:ReadUInt(NET_MESSAGE_BITS)
		local handler = NetMessage(netchan, msg, not SERVER)
		if not handler then
			MsgC(Color(255, 0, 0), "Unknown outgoing message " .. msg .. " with " .. read:GetNumBitsLeft() .. " bit(s) left\n")
			return false
		end

		--[[local success =]] handler:ReadFromBuffer(read)
		--[[if not success then
			MsgC(Color(255, 0, 0), "Failed to read message " .. handler:GetName() .. " with " .. read:GetNumBitsLeft() .. " bit(s) left\n")
			return false
		end]]

		local success = handler:WriteToBuffer(write)
		if not success then
			MsgC(Color(255, 0, 0), "Failed to write message " .. handler:GetName() .. " with " .. read:GetNumBitsLeft() .. " bit(s) left\n")
			return false
		end

		--MsgC(Color(255, 255, 255), "NetMessage: " .. tostring(handler) .. "\n")
	end

	local bitsleft = read:GetNumBitsLeft()
	if bitsleft > 0 then
		-- Should be inocuous padding bits but just to be sure, let's copy them
		local data = read:ReadBits(bitsleft)
		write:WriteBits(data)
	end

	--MsgC(Color(0, 255, 0), "Fully parsed stream with " .. totalbits .. " bit(s) written\n")
	return true
end)

function FilterIncomingMessage(msg, func)
	--[[local handler = NET_MESSAGES[msg]
	if handler == nil then
		if CLIENT then
			handler = NET_MESSAGES.SVC[msg]
		else
			handler = NET_MESSAGES.CLC[msg]
		end
	end

	if handler ~= nil then
		handler.IncomingCopy = func
	end]]
end

function UnFilterIncomingMessage(msg)
	FilterIncomingMessage(msg, nil)
end
