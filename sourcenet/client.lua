require("sourcenet")

MENU = LocalPlayer == nil

NET_HOOKS = NET_HOOKS or {attach = {}, detach = {}}

NET_ATTACHED = false

function HookNetChannel(...)
	local args = {...}

	for k, v in pairs(args) do
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
		if NET_ATTACHED then return false end
		if not netchan then return false end

		Attach__CNetChan_Shutdown(netchan)

		NET_ATTACHED = true

		for k, v in pairs(NET_HOOKS.attach) do
			StandardNetHook(netchan, v)
		end

		return true
	end

	local function DetachNetChannel(netchan)
		if not NET_ATTACHED then return false end
		if not netchan then return false end

		Detach__CNetChan_Shutdown(netchan)

		NET_ATTACHED = false

		for k, v in pairs(NET_HOOKS.detach) do
			StandardNetHook(netchan, v)
		end

		return true
	end

	if not AttachNetChannel(CNetChan()) then
		hook.Add("Think", "CreateNetChannel", function() -- Wait until channel is created
			local netchan = CNetChan()
			if netchan ~= nil and AttachNetChannel(netchan) then
				hook.Remove("Think", "CreateNetChannel")
			end
		end )
	end

	hook.Add("PreNetChannelShutdown", "DetachHooks", function(netchan, reason)
		--print("[gm_sourcenet] PreNetChannelShutdown called, netchan=" .. tostring(netchan) .. ", reason=" .. reason)

		DetachNetChannel(netchan)

		--[[if DetachNetChannel(netchan) then
			if MENU then
				NET_HOOKS = NET_HOOKS or {attach = {}, detach = {}}

				hook.Add("Think", "DestroyNetChannel", function() -- Ensure the current channel is destroyed before waiting for a new one
					if not CNetChan() then
						HookNetChannel(unpack(args))
						
						hook.Remove("Think", "DestroyNetChannel")
					end
				end)
			end
		end--]]
	end)
end
