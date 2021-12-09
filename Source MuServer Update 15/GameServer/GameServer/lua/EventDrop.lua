-- zGameServer Season 6 Episode 3
--======================================================================================================================================
-- Description:		Extend Event drop control
--======================================================================================================================================
-- Last edit:		19:12 12.02.2013
--======================================================================================================================================
-- Objects:			lpObj (Player)
--======================================================================================================================================

function ITEM(ItemID, ItemIndex)
	return ItemID * 512 + ItemIndex
end
------------------------------------------------------------------------------------------

function BloodCastleEventDrop()
	iMapNumber = 227 + lpObj.MapNumber
	----
	if lpObj.MapNumber == 52 then	-- Blood Castle 8
		iMapNumber = 245
	end
	----
	DropItem = ITEM(12, 15)
	----
	ItemSerialCreateSend(lpObj.aIndex, iMapNumber, lpObj.X, lpObj.Y, DropItem, 0, 0, 0, 0, 0, lpObj.aIndex, 0, 0)
	----
	return 1
end
------------------------------------------------------------------------------------------

function ChaosCastleEventDrop(ChaosCastleIndex)
	DropData = 
	{
		Type	= -1,	-- 0 = Item | 1 = Ancient
		ItemID 	= 0,
		Level	= 0,
		Count	= 1
	}
	----
	if ChaosCastleIndex 	== 0 then
		if math.random(0, 100) < 20 then	-- 80%
			DropData.Type 	= 0
			DropData.ItemID	= ITEM(14, 14)
			DropData.Count	= 1
		else
			DropData.Type	= 1
		end
	elseif ChaosCastleIndex	== 1 then
		if math.random(0, 100) < 30 then	-- 70%
			DropData.Type 	= 0
			DropData.ItemID	= ITEM(14, 14)
			DropData.Count	= 2
		else
			DropData.Type	= 1
		end
	elseif ChaosCastleIndex	== 2 then
		if math.random(0, 100) < 40 then	-- 60%
			DropData.Type 	= 0
			DropData.ItemID	= ITEM(14, 14)
			DropData.Count	= 3
		else
			DropData.Type	= 1
		end
	elseif ChaosCastleIndex	== 3 then
		if math.random(0, 100) < 50 then	-- 50%
			DropData.Type 	= 0
			DropData.ItemID	= ITEM(14, 13)
			DropData.Count	= 2
		else
			DropData.Type	= 1
		end
	elseif ChaosCastleIndex	== 4 then
		if math.random(0, 100) < 60 then	-- 40%
			DropData.Type 	= 0
			DropData.ItemID	= ITEM(14, 13)
			DropData.Count	= 3
		else
			DropData.Type	= 1
		end
	elseif ChaosCastleIndex	== 5 then
		if math.random(0, 100) < 70 then	-- 30%
			DropData.Type 	= 0
			DropData.ItemID	= ITEM(14, 16)
			DropData.Count	= 2
		else
			DropData.Type	= 1
		end
	elseif ChaosCastleIndex	== 6 then
		if math.random(0, 100) < 80 then	-- 20%
			DropData.Type 	= 0
			DropData.ItemID	= ITEM(14, 16)
			DropData.Count	= 3
		else
			DropData.Type	= 1
		end
	end
	----
	if DropData.Type 		== 0 then
		for i = 0, DropData.Count do
			ItemSerialCreateSend(lpObj.aIndex, lpObj.MapNumber, lpObj.X, lpObj.Y, DropData.ItemID, DropData.Level, 0, 0, 0, 0, lpObj.aIndex, 0, 0)
		end
	elseif DropData.Type 	== 1 then
		MakeRandomSetItem(lpObj.aIndex)
	else
		return 0
	end
	----
	return 1
end
------------------------------------------------------------------------------------------