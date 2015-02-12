include("sn4_base_querycvar.lua")

function FindPlayerByNetChannel(netchan)
	local adr = netchan:GetAddress():ToString()

	for k, v in pairs(player.GetAll()) do
		if adr == v:IPAddress() then
			return v
		end
	end
end

hook.Add("PlayerInitialSpawn", "InitialCheatsCheck", function(ply)
	ply.CheatsCookie = ply:QueryConVarValue("sv_cheats")
end)

hook.Add("RespondCvarValue", "InitialCheatsCheck", function(netchan, cookie, status, cvarname, cvarvalue)
	if status ~= 0 then return end
	if cvarname ~= "sv_cheats" then return end
	if cvarvalue == GetConVarString("sv_cheats") then return end
	
	local ply = FindPlayerByNetChannel(netchan)
	
	if IsValid(ply) and cookie == ply.CheatsCookie then
		ply:Kick("Incorrect sv_cheats value")
	end
end)