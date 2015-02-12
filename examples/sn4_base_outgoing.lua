if SERVER then
	include("sn4_base_sv.lua")
else
	include("sn4_base_cl.lua")
end

include("sn4_base_netmessages.lua")

-- Initialization

HookNetChannel(
	{name = "CNetChan::SendDatagram"}
)

hook.Add("PreSendDatagram", "OutFilter", function(netchan, localchan, ...)
	if not game.IsDedicated() then
		if netchan == localchan then
			if SERVER then return end
		else
			if CLIENT then return end
		end
	end

	local buffers = {...}

	for k, write in pairs(buffers) do
		local totalbits = write:GetNumBitsWritten()
		local read = sn4_bf_read(write:GetBasePointer(), totalbits)

		write:Seek(0)
		
		while read:GetNumBitsLeft() >= NET_MESSAGE_BITS do
			local msg = read:ReadUInt(NET_MESSAGE_BITS)
			local handler = NET_MESSAGES[msg]

			--[[if msg ~= net_NOP and msg ~= 3 and msg ~= 9 then
				Msg("(out) Pre Message: " .. msg .. ", bits: " .. read:GetNumBitsRead() .. "/" .. totalbits .. "\n")
			end--]]

			if not handler then
				if CLIENT then
					handler = NET_MESSAGES.CLC[msg]
				else
					handler = NET_MESSAGES.SVC[msg]
				end

				if not handler then
					Msg("Unknown outgoing message: " .. msg .. "\n")

					write:Seek(totalbits)

					break
				end
			end

			local func = handler.OutgoingCopy or handler.DefaultCopy
			
			if func(netchan, read, write) == false then
				Msg("Failed to filter message " .. msg .. "\n")

				write:Seek(totalbits)

				break
			end
			
			--[[if msg ~= net_NOP and msg ~= 3 and msg ~= 9 then
				Msg("(out) Post Message: " .. msg .. ", bits: " .. read:GetNumBitsRead() .. "/" .. totalbits .. "\n")
			end--]]
		end
	end
end )

function FilterOutgoingMessage(msg, func)
	local handler = NET_MESSAGES[msg]
	
	if not handler then
		if CLIENT then
			handler = NET_MESSAGES.CLC[msg]
		else
			handler = NET_MESSAGES.SVC[msg]
		end
	end
	
	if not handler then return end

	handler.OutgoingCopy = func
end

function UnFilterOutgoingMessage(msg)
	FilterOutgoingMessage(msg, nil)
end