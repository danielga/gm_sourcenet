include("sourcenet/gameevents.lua")

hook.Add("SendGameEvent", "ChangeReason", function(netchan, event)
	if event:GetName() ~= "player_disconnect" then return end

	local reason = event:GetString("reason")

	if reason == "Disconnect by user." then
		event:SetString("reason", "Disconnected after " .. math.floor(netchan:GetTime() - netchan:GetConnectTime()) .. " seconds")
		return event
	end
end)
