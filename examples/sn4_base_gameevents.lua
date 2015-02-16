if CLIENT then
	include("sn4_base_incoming.lua")
else
	include("sn4_base_outgoing.lua")
end

local function FilterGameEvent(netchan, read, write, hookname)
	local bits = read:ReadUInt(11)
	local data = read:ReadBits(bits)

	if not read:IsOverflowed() then
		local buffer = sn4_bf_read(data, bits)
		local event = IGameEventManager2():UnserializeEvent(buffer)

		local result = hook.Call(hookname, nil, netchan, event)

		if result ~= false then
			write:WriteUInt(svc_GameEvent, NET_MESSAGE_BITS)

			if type(result) == "IGameEvent" then
				local serialized_data = UCHARPTR(2048)
				local serialized_buffer = sn4_bf_write(serialized_data)

				IGameEventManager2():SerializeEvent(event, serialized_buffer)
				
				write:WriteUInt(serialized_buffer:GetNumBitsWritten(), 11)
				write:WriteBits(serialized_buffer:GetBasePointer())
			else
				write:WriteUInt(bits, 11)
				write:WriteBits(data)
			end
		end
	end
end

if CLIENT then
	FilterIncomingMessage(svc_GameEvent, function(netchan, read, write)
		FilterGameEvent(netchan, read, write, "ProcessGameEvent")
	end)
else
	FilterOutgoingMessage(svc_GameEvent, function(netchan, read, write)
		FilterGameEvent(netchan, read, write, "SendGameEvent")
	end)
end