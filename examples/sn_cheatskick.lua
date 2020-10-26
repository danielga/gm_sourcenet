include("sourcenet/querycvar.lua")

function FindPlayerByNetChannel(netchan)
	for k, v in pairs(player.GetAll()) do
		if netchan == CNetChan(v:EntIndex()) then
			return v
		end
	end
end

hook.Add("PlayerInitialSpawn", "InitialCheatsCheck", function(ply)
	ply.CheatsCookie = ply:QueryConVarValue("sv_cheats")
end)

hook.Add("RespondCvarValue", "InitialCheatsCheck", function(netchan, cookie, status, cvarname, cvarvalue)
	if status ~= 0 or cvarname ~= "sv_cheats" or cvarvalue == GetConVarString("sv_cheats") then
		return
	end

	local ply = FindPlayerByNetChannel(netchan)
	if IsValid(ply) and cookie == ply.CheatsCookie then
		ply:Kick("Incorrect sv_cheats value")
	end
end)
