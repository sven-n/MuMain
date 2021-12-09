-- zGameServer Season 6 Episode 3
--======================================================================================================================================
-- Description:		Extend NPC control
--======================================================================================================================================
-- Last edit:		19:12 12.02.2013
--======================================================================================================================================
-- Objects:			lpObj (Player), lpNpc (NPC)
--======================================================================================================================================

function NpcTalk()
	npcnum = lpNpc.Class
	----
	if npcnum < 0 then
		return 0
	end
	----
	if TestLuaNPC() == 1 then
		return 1
	end
	----
	return 0
end
------------------------------------------------------------------------------------------

function TestLuaNPC()
	if lpNpc.MapNumber 	== 0 
	and lpNpc.X 		== 137 
	and lpNpc.Y 		== 133
	and lpNpc.Class		== 414 -- Helper Ellen
	then 
		if lpObj.Money < 3000000 then
			ChatTargetSend('You need more money to teleport', lpNpc.aIndex, lpObj.aIndex)
		elseif lpObj.Level < 400 then
			ChatTargetSend('You need 400 level to teleport', lpNpc.aIndex, lpObj.aIndex)
		else
			gObjTeleport(lpObj.aIndex, 40, 220, 15)
		end
		----
		return 1
	end
	----
	return 0
end
------------------------------------------------------------------------------------------