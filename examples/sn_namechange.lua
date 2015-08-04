require("sourcenet")

concommand.Add("setname", function(ply, cmd, args)
	if not args[1] then
		print("Syntax: setname <name>")
		
		return
	end
	
	local netchan = CNetChan()
	
	if not netchan then
		print("setname: invalid netchan")
		
		return
	end

	local buffer = netchan:GetReliableBuffer()
	
	if not buffer then
		print("setname: invalid buffer")
		
		return
	end
	
	buffer:WriteUInt(net_SetConVar, NET_MESSAGE_BITS) -- message type
	buffer:WriteByte(1) -- convar count
	buffer:WriteString("name") -- convar name
	buffer:WriteString(args[1]) -- convar value
end)
