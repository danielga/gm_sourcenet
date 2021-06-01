include("sourcenet/server.lua")

-- Initialization
HookNetChannel(
	{name = "CNetChan::ProcessPacket"}
)

-- Check progress every incoming packet (Source seems to clear fragments here)
local history = {}

hook.Add("PreProcessPacket", "TransferStatus", function(netchan)
	for i = 0, MAX_STREAMS - 1 do
		for j = 0, netchan:GetOutgoingQueueSize(i) - 1 do
			local fragments = netchan:GetOutgoingQueueFragments(i, j)
			local filename = fragments:GetFileName()

			if #filename ~= 0 and not table.HasValue(history, filename) and fragments:GetProgress() + fragments:GetNum() >= fragments:GetTotal() then
				print("Finished " .. filename)

				umsg.Start("fstream_complete")
					umsg.String(filename)
				umsg.End()

				table.insert(history, filename)
			end
		end
	end
end)

-- Tests
function QueueFile(netchan, filename)
	netchan:SendFile(filename, 1)
end

hook.Add("PlayerInitialSpawn", "BeginTransfer", function(ply)
	local netchan = CNetChan(ply:EntIndex())
	if netchan == nil then
		return
	end

	QueueFile(netchan, "cl.db")
	QueueFile(netchan, "gameinfo.txt")
end)
