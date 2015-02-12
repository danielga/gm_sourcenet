if SERVER then
	include("sn4_base_sv.lua")
else
	include("sn4_base_cl.lua")
end

include("sn4_base_netmessages.lua")

-- Initialization

HookNetChannel(
	-- nochan prevents a net channel being passed to the attach/detach functions
	-- CNetChan::ProcessMessages doesn't use a virtual hook, so we don't need to pass the net channel
	{name = "CNetChan::ProcessMessages", nochan = true}
)

local function CopyBufferEnd(dst, src)
	local bitsleft = src:GetNumBitsLeft()
	local data = src:ReadBits(bitsleft)
	
	dst:WriteBits(data, bitsleft)
end

hook.Add("PreProcessMessages", "InFilter", function(netchan, read, write, localchan)
	local totalbits = read:GetNumBitsLeft() + read:GetNumBitsRead()

	if not game.IsDedicated() then
		if netchan == localchan then
			if SERVER then CopyBufferEnd(write, read) return end
		else
			if CLIENT then CopyBufferEnd(write, read)	return end
		end
	end

	hook.Call("BASE_PreProcessMessages", nil, netchan, read, write)

	local changeLevelState = false

	while read:GetNumBitsLeft() >= NET_MESSAGE_BITS do
		local msg = read:ReadUInt(NET_MESSAGE_BITS)

		if CLIENT then
			-- Hack to prevent changelevel crashes
			if msg == net_SignonState then
				local state = read:ReadByte()
				
				if state == SIGNONSTATE_CHANGELEVEL then
					changeLevelState = true
					--print( "[gm_sourcenet4] Received changelevel packet" )
				end
				
				read:Seek(read:GetNumBitsRead() - 8)
			end
		end

		local handler = NET_MESSAGES[msg]

		--[[if msg ~= net_NOP and msg ~= 3 and msg ~= 9 then
			Msg("(in) Pre Message: " .. msg .. ", bits: " .. read:GetNumBitsRead() .. "/" .. totalbits .. "\n")
		end--]]
	
		if not handler then
			if CLIENT then
				handler = NET_MESSAGES.SVC[msg]
			else
				handler = NET_MESSAGES.CLC[msg]
			end

			if not handler then
				Msg("Unknown incoming message: " .. msg .. "\n")
					
				write:Seek(totalbits)

				break
			end
		end

		local func = handler.IncomingCopy or handler.DefaultCopy
	
		if func(netchan, read, write) == false then
			Msg("Failed to filter message " .. msg .. "\n")

			write:Seek(totalbits)

			break
		end

		--[[if msg ~= net_NOP and msg ~= 3 and msg ~= 9 then
			Msg("(in) Post Message: " .. msg .. " bits: " .. read:GetNumBitsRead() .. "/" .. totalbits .. "\n")
		end--]]
	end
	
	if CLIENT then
		if changeLevelState then
			--print("[gm_sourcenet4] Server is changing level, calling PreNetChannelShutdown")
			hook.Call("PreNetChannelShutdown", nil, netchan, "Server Changing Level")
		end
	end
end)

function FilterIncomingMessage(msg, func)
	local handler = NET_MESSAGES[msg]
	
	if not handler then
		if CLIENT then
			handler = NET_MESSAGES.SVC[msg]
		else
			handler = NET_MESSAGES.CLC[msg]
		end
	end
	
	if not handler then return end

	handler.IncomingCopy = func
end

function UnFilterIncomingMessage(msg)
	FilterIncomingMessage(msg, nil)
end