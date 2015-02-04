include( "sn4_base_outgoing.lua" )

-- These don't even seem to be acknowledged

WEAPONSWEP_MSG_HOLSTER = 1
WEAPONSWEP_MSG_EQUIP = 3

function SendEntityMessage( netchan, entindex, classID, buffer )
	local messageType = buffer:ReadByte()
	local entity = Entity( entindex )
	
	if ( !ValidEntity( entity ) ) then return end

	if ( entity:IsWeapon() ) then -- There is no Entity.GetClassID function, so this is a workaround
		if ( messageType == WEAPONSWEP_MSG_HOLSTER ) then
			print( netchan:GetAddress():ToString(), "CWeaponSWEP::Holster", entity, entity:GetClass() )
		elseif ( messageType == WEAPONSWEP_MSG_EQUIP ) then
			print( netchan:GetAddress():ToString(), "CWeaponSWEP::Equip", entity, entity:GetClass() )
		end
	end
end

FilterOutgoingMessage( svc_EntityMessage, function( netchan, read, write )
	local entindex = read:ReadUBitLong( 11 )
	local classID = read:ReadUBitLong( 9 )
	local bits = read:ReadUBitLong( 11 )
	local data = read:ReadBits( bits )

	local buffer = sn4_bf_read( data, bits )

	SendEntityMessage( netchan, entindex, classID, buffer )

	buffer:FinishReading()

	write:WriteUBitLong( svc_EntityMessage, NET_MESSAGE_BITS )
	write:WriteUBitLong( entindex, 11 )
	write:WriteUBitLong( classID, 9 )
	write:WriteUBitLong( bits, 11 )
	write:WriteBits( data, bits )

	data:Delete()
end )