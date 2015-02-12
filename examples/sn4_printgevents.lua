include("sn4_base_gameevents.lua")

hook.Add("ProcessGameEvent", "PrintIncomingEvent", function(source, event)
	print("Received game event '" .. event:GetName() .. "' from '" .. source:GetAddress():ToString() .. "'")
end)

hook.Add("SendGameEvent", "PrintOutgoingEvent", function(destination, event)
	print("Sending game event '" .. event:GetName() .. "' to '" .. destination:GetAddress():ToString() .. "'")
end)