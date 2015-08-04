include("sourcenet/outgoing.lua")

-- These don't even seem to be acknowledged

WEAPONSWEP_MSG_HOLSTER = 1
WEAPONSWEP_MSG_EQUIP = 3

function SendEntityMessage(netchan, entindex, classID, buffer)
	local messageType = buffer:ReadByte()
	local entity = Entity(entindex)
	
	if not IsValid(entity) then return end

	if entity:IsWeapon() then -- There is no Entity.GetClassID function, so this is a workaround
		if messageType == WEAPONSWEP_MSG_HOLSTER then
			print(netchan:GetAddress():ToString(), "CWeaponSWEP::Holster", entity, entity:GetClass())
		elseif messageType == WEAPONSWEP_MSG_EQUIP then
			print(netchan:GetAddress():ToString(), "CWeaponSWEP::Equip", entity, entity:GetClass())
		end
	end
end

FilterOutgoingMessage(svc_EntityMessage, function(netchan, read, write)
	local entindex = read:ReadUInt(11)
	local classID = read:ReadUInt(9)
	local bits = read:ReadUInt(11)
	local data = read:ReadBits(bits)

	local buffer = sn_bf_read(data)

	SendEntityMessage(netchan, entindex, classID, buffer)

	write:WriteUInt(svc_EntityMessage, NET_MESSAGE_BITS)
	write:WriteUInt(entindex, 11)
	write:WriteUInt(classID, 9)
	write:WriteUInt(bits, 11)
	write:WriteBits(data)
end)
