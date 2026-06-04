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

local NET_MESSAGES_INSTANCES = {}

local function GetNetMessageInstance(netchan, msgtype)
	local handler = NET_MESSAGES_INSTANCES[msgtype]
	if handler == nil then
		handler = NetMessage(netchan, msgtype, SERVER)
		NET_MESSAGES_INSTANCES[msgtype] = handler
	else
		handler:Reset()
	end

	return handler
end

local NET_MESSAGES_OUTGOING_COPY = {
	NET = {},
	CLC = {},
	SVC = {}
}

local function GetOutgoingCopyTableForMessageType(msgtype)
	if NET_MESSAGES.NET[msgtype] ~= nil then
		return NET_MESSAGES_OUTGOING_COPY.NET
	end

	if CLIENT and NET_MESSAGES.CLC[msgtype] ~= nil then
		return NET_MESSAGES_OUTGOING_COPY.CLC
	end

	if SERVER and NET_MESSAGES.SVC[msgtype] ~= nil then
		return NET_MESSAGES_OUTGOING_COPY.SVC
	end

	return nil
end

local function DefaultCopy(netchan, read, write, handler)
	handler:ReadFromBuffer(read)
	handler:WriteToBuffer(write)
end

local function HandleStream(name, netchan, write)
	if write == nil then
		return false
	end

	local totalbits = write:GetNumBitsWritten()
	if totalbits <= 0 then
		return true
	end

	local read = sn_bf_read(write:GetBasePointer(), totalbits)
	write:Seek(0)
	while read:GetNumBitsLeft() >= NET_MESSAGE_BITS do
		local msgtype = read:ReadUInt(NET_MESSAGE_BITS)
		local handler = GetNetMessageInstance(netchan, msgtype)
		if handler == nil then
			SourceNetMsg(Color(255, 0, 0), "Unknown outgoing message " .. msgtype .. " on " .. name .. " stream with " .. read:GetNumBitsLeft() .. " bit(s) left\n")
			return false
		end

		local outgoing_copy_table = GetOutgoingCopyTableForMessageType(msgtype)
		local copy_function = outgoing_copy_table ~= nil and outgoing_copy_table[msgtype] or DefaultCopy
		copy_function(netchan, read, write, handler)

		SourceNetMsg(Color(255, 255, 0), "NetMessage from " .. name .. " stream: " .. tostring(handler) .. "\n")
	end

	local bitsleft = read:GetNumBitsLeft()
	if bitsleft > 0 then
		-- Should be inocuous padding bits but just to be sure, let's copy them
		local data = read:ReadBits(bitsleft)
		write:WriteBits(data)
		SourceNetMsg(Color(255, 255, 0), "Copied " .. bitsleft .. " bit(s)\n")
	end

	SourceNetMsg(Color(0, 255, 0), "Fully parsed " .. name .. " stream with " .. totalbits .. " bit(s) written\n")
	return true
end

hook.Add("PreSendDatagram", "OutFilter", function(netchan, localchan, data, reliablestream, unreliablestream, voicestream)
	local islocal = netchan == localchan
	if not game.IsDedicated() and ((islocal and SERVER) or (not islocal and CLIENT)) then
		return
	end

	local data_handled = HandleStream("data", netchan, data)
	local reliable_handled = HandleStream("reliable", netchan, reliablestream)
	local unreliable_handled = HandleStream("unreliable", netchan, unreliablestream)
	local voice_handled = HandleStream("voice", netchan, voicestream)
	return data_handled and reliable_handled and unreliable_handled and voice_handled
end)

function FilterOutgoingMessage(msgtype, func)
	local outgoing_copy_table = GetOutgoingCopyTableForMessageType(msgtype)
	if outgoing_copy_table == nil then
		return false
	end

	outgoing_copy_table[msgtype] = func
	return true
end

function UnFilterOutgoingMessage(msgtype)
	return FilterOutgoingMessage(msgtype, nil)
end
