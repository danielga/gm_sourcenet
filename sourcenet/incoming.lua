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

local function CopyBufferEnd(dst, src)
	local bitsleft = src:GetNumBitsLeft()
	local data = src:ReadBits(bitsleft)
	
	dst:WriteBits(data)
end

local specialmsg
local specialhandler = {
	DefaultCopy = function(netchan, read, write)
		specialmsg:ReadFromBuffer(read)
		specialmsg:WriteToBuffer(write)
	end
}
hook.Add("PreProcessMessages", "InFilter", function(netchan, read, write, localchan)
	local totalbits = read:GetNumBitsLeft() + read:GetNumBitsRead()

	local islocal = netchan == localchan
	if not game.IsDedicated() and ((islocal and SERVER) or (not islocal and CLIENT)) then
		CopyBufferEnd(write, read)
		return
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
					--print( "[gm_sourcenet] Received changelevel packet" )
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
				for i = 1, netchan:GetNetMessageNum() do
					local m = netchan:GetNetMessage(i)
					if m:GetType() == msg then
						handler = specialhandler
						specialmsg = m
						break
					end
				end

				if not handler then
					Msg("Unknown outgoing message: " .. msg .. "\n")
						
					write:Seek(totalbits)

					break
				end
			end
		end

		local func = handler.IncomingCopy or handler.DefaultCopy

		local success, ret = xpcall(func, debug.traceback, netchan, read, write)
		if not success then
			print(ret)

			break
		elseif ret == false then
		--if func(netchan, read, write) == false then
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
			--print("[gm_sourcenet] Server is changing level, calling PreNetChannelShutdown")
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

	if handler then
		handler.IncomingCopy = func
	end
end

function UnFilterIncomingMessage(msg)
	FilterIncomingMessage(msg, nil)
end
