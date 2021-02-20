require("sourcenet")

NET_CHANNEL_INDICES = NET_CHANNEL_INDICES or {}
NET_HOOKS = NET_HOOKS or {attach = {}, detach = {}}

local function StandardNetHook(netchan, nethook)
	local args = {}

	if nethook.func then
		table.insert(args, nethook.func(netchan))
	elseif not nethook.nochan then
		table.insert(args, netchan)
	end

	if nethook.args then
		for k, v in pairs(nethook.args) do
			table.insert(args, v)
		end
	end

	nethook.hook(unpack(args))
end

local function AttachNetChannel(netchan)
	if not netchan then return false end

	Attach__CNetChan_Shutdown(netchan)

	for k, v in pairs(NET_HOOKS.attach) do
		StandardNetHook(netchan, v)
	end

	return true
end

local function DetachNetChannel(netchan)
	if not netchan then return false end

	Detach__CNetChan_Shutdown(netchan)

	for k, v in pairs(NET_HOOKS.detach) do
		StandardNetHook(netchan, v)
	end

	return true
end

function HookNetChannel(...)
	for k, v in pairs({...}) do
		local name = v.name:gsub("::", "_")
		local exists = false

		for k, v in pairs(NET_HOOKS.attach) do
			if v.name == name then
				exists = true
				break
			end
		end

		if not exists then
			table.insert(NET_HOOKS.attach, {name = name, hook = _G["Attach__" .. name], func = v.func, args = v.args, nochan = v.nochan})
			table.insert(NET_HOOKS.detach, {name = name, hook = _G["Detach__" .. name], func = v.func, args = v.args, nochan = v.nochan})
		end
	end

	local attached = false

	for i = 1, 256 do
		local netchan = CNetChan(i)

		if netchan then
			if not attached then
				AttachNetChannel(netchan)
				attached = true
			end

			if not table.HasValue(NET_CHANNEL_INDICES, i) then
				table.insert(NET_CHANNEL_INDICES, i)
			end
		end
	end

	hook.Add("PreNetChannelShutdown", "DetachHooks", function(netchan, reason)
		for k, v in pairs(NET_CHANNEL_INDICES) do
			local a1 = netchan:GetAddress()
			local a2 = CNetChan(v):GetAddress()

			if a1:GetIP() == a2:GetIP() and a1:GetPort() == a2:GetPort() then
				table.remove(NET_CHANNEL_INDICES, k)
				break
			end
		end

		if #NET_CHANNEL_INDICES == 0 then
			DetachNetChannel(netchan)
		end
	end)

	hook.Add("ShutDown", "DetachHooks", function()
		if #NET_CHANNEL_INDICES > 0 then
			DetachNetChannel(CNetChan(NET_CHANNEL_INDICES[1]))
		end
	end)
end

hook.Add("PlayerConnect", "CreateNetChannel", function(name, address)
	if address == "none" then
		return -- Bots don't have a net channel
	end

	local index
	if #NET_CHANNEL_INDICES > 0 then
		for i = 1, 256 do
			if not table.HasValue(NET_CHANNEL_INDICES, i) then
				index = i
				break
			end
		end
	else
		index = 1 -- If there are no bots or players then the index will be 1
	end

	local netchan = CNetChan(index)
	if netchan ~= nil then
		if #NET_CHANNEL_INDICES == 0 then
			AttachNetChannel(netchan)
		end

		table.insert(NET_CHANNEL_INDICES, index)

		hook.Call("PostNetChannelInit", nil, netchan)
	end
end)
