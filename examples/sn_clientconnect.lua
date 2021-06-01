include("sourcenet/gameevents.lua")

hook.Add("ProcessGameEvent", "PlayerConnect", function(netchan, event)
	if event:GetName() ~= "player_connect" then
		return
	end

	event:SetString("name", string.reverse(event:GetString("name")))

	Msg("player_connect:\n")
	Msg("\tname = " .. event:GetString("name") .. "\n")
	Msg("\tindex = " .. event:GetInt("index") .. "\n")
	Msg("\tuserid = " .. event:GetInt("userid") .. "\n")
	Msg("\tnetworkid = " .. event:GetString("networkid") .. "\n")
	Msg("\taddress = " .. event:GetString("address") .. "\n")

	return event
end)
