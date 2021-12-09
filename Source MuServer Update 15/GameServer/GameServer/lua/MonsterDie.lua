-- zGameServer Season 6 Episode 3
--======================================================================================================================================
-- Description:		Extend monster drop control
--======================================================================================================================================
-- Last edit:		1:16 13.02.2013
--======================================================================================================================================
-- Objects:			lpObj (Player), lpMonster (Monster)
--======================================================================================================================================

Monster			= lpMonster.Class
------------------------------------------------------------------------------------------

function ITEM(ItemID, ItemIndex)
	return ItemID * 512 + ItemIndex
end
------------------------------------------------------------------------------------------

function LogOut(Text)
	LogAddTD(string.format('[LuaMonsterDie] %s', Text))
end
------------------------------------------------------------------------------------------

function ItemDrop()
	local Killer = gObjMonsterTopHitDamageUser(lpMonster.aIndex)
	----
	for i = 1, DropData.Count do
		ItemSerialCreateSend(Killer, lpMonster.MapNumber, lpMonster.X, lpMonster.Y, DropData.ItemID, DropData.Level, 0, 0, 0, 0, lpObj.aIndex, 0, 0)
	end
end
------------------------------------------------------------------------------------------

function gObjMonsterDieGiveItem()
	DropData = -- For simply macro, can use strong changes
	{
		ItemID 	= 0,
		Level	= 0,
		Count	= 1
	}
	----
	if TestMonsterDieProc() == 1 then
		return 1
	end
	----
	return 0
end
------------------------------------------------------------------------------------------

-- Sample (Elite Yeti)
function TestMonsterDieProc()
	if Monster == 20 then					
		if math.random(0, 100) < 30 then	-- 30% for item drop
			DropData.ItemID = ITEM(14, 16)	-- Jewel Of Chaos
			----
			GCServerMsgStringSend(string.format('[Lua] Monster %d', Monster), lpObj.aIndex, 1)
			LogOut(string.format('Monster %d killed', Monster))
			FireCrackerOpen(lpMonster.aIndex)
			ItemDrop()
		else
			return 0						-- Return to original if drop not succefull
		end
	else
		return 0							-- Return to original if monster not 20
	end
	----
	return 1								-- Stop original if monster drop finished
end
------------------------------------------------------------------------------------------