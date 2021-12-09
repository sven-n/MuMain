-- zGameServer Season 6 Episode 3
--======================================================================================================================================
-- Description:		NPC control script
--======================================================================================================================================
-- Last edit:		12:10 11.02.2013
--======================================================================================================================================

function NpcTalk()
	LogAddTD('[LuaNPC] Start')
	----
	npcnum = lpNpc.Class
	----
	if npcnum < 0 then
		return 0
	end
	----
	if lpObj.m_IfStateUse > 0 then
		return 1
	end
	----
	if TestLuaNPC() == 1 then
		return 1
	end
	----
	--if lpNpc.Class == 229 then
	--	gQeustNpcTeleport_TalkRefAdd();
	--	lpObj.TargetShopNumber = npcnum;
	--end
	----
	--if NpcQuestCheck(lpNpc, lpObj) == 1 then
	--	return 1
	--end
	----
	if lpNpc.Class 		== 205 
	or lpNpc.Class 		== 206
	or lpNpc.Class 		== 207 
	or lpNpc.Class 		== 208
	or lpNpc.Class 		== 209 then
		if NpcCryWolfAltar() == 1 then	--OK
			return 1
		end
	----
	--elseif lpNpc.Class 	== 216 then
	--	if NpcCastleCrown() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 217 
	--or lpNpc.Class 		== 218 then
	--	if NpcCastleSwitch() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 219 then
	--	if NpcCastleGateLever() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 220 then
	--	if NpcCastleHuntZoneGuard() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 221 then
	--	if NpcSiegeMachine_Attack() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 222 then
	--	if NpcSiegeMachine_Defense() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 223 then
	--	if NpcElderCircle() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 224 then
	--	if NpcCastleGuard() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 226 then
	--	if NpcDarkSpiritTrainer() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 232 then
	--	if NpcAngelKing() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 233 then
	--	if NpcAngelMessanger() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 234 then
	--	if NpcServerDivision() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 236 then	
	--	if NpcEventChipNPC() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 237 then
	--	if NpcRusipher() == 1 then
	--		return 1
	--	end
	----
	elseif lpNpc.Class 	== 238 then
		if NpcChaosGoblelin() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 239 then
		if NpcBattleAnnouncer() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 240 then	
		if NpcWarehouse() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 241 then
		if NpcGuildMasterTalk() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 242 then
		if NpcNoriaRara() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 243 then
		if NpcNoriJangIn() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 244 then		
		if NpcDeviasMadam() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 245 then	
		if NpcDeviasWizard() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 246 then
		if NpcDeviasWeapon() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 247 then	
		if NpcDeviasGuard() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 249 then
		if NpcRolensiaGuard() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 250 then	
		if NpcRoadMerchant() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 251 then
		if NpcRorensiaSmith() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 253 then	
		if NpcPosionGirl() == 1 then	--OK
			return 1
		end
	----
	--elseif lpNpc.Class 	== 254 then
	--	if NpcFasi() == 1 then
	--		return 1
	--	end
	----
	elseif lpNpc.Class 	== 255 then
		if NpcRolensiaMadam() == 1 then	--OK
			return 1
		end
	----
	--elseif lpNpc.Class 	== 256 then
	--	if NpcJewelMixDealer() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 257
	--or lpNpc.Class 		== 543
	--or lpNpc.Class 		== 544
	--or lpNpc.Class 		== 566	
	--or lpNpc.Class 		== 567 then	
	--	if Npc_Dialog() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 259 then	
	--	if NpcReira() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 367 then
	--	if NpcMainatenceMachine() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 368 
	--or lpNpc.Class 		== 369
	--or lpNpc.Class 		== 370 then
	--	if NpcJewelOfHarmony() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 375 then
	--	if NpcChaosCardMaster() == 1 then
	--		return 1
	--	end
	----
	elseif lpNpc.Class 	== 376 then
		if NpcQuartermasterPamella() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 377 then
		if NpcQuartermasterAngela() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 379 then
		if NPcFireCrackerStore() == 1 then	--OK
			return 1
		end
	----
	--elseif lpNpc.Class 	== 380 then
	--	if NpcIllusionHolyRelicsStatue() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 383 then	
	--	if NpcHolyRelicsBoxAllied() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 384 then
	--	if NpcHolyRelicsBoxIllusion() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 385 then
	--	if NpcPlatformMirage() == 1 then
	--		return 1
	--	end
	----
	elseif lpNpc.Class 	== 415 then
		if NpcSilviaTalk() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 416 then
		if NpcRheaTalk() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 417 then
		if NpcMarceTalk() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 450 then	
		if NpcCherryTalk() == 1 then	--OK
			return 1
		end
	----
	--elseif lpNpc.Class 	== 452 then
	--	if NpcSeedMaster() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 453 then
	--	if NpcSeedResearcher() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 465 then	
	--	if Npc_SantaTalk() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 467 then	
	--	if Npc_SnowManTalk() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 468 then
	--	if Npc_LittleSantaTalk(94) == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 469 then		
	--	if Npc_LittleSantaTalk(95) == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 470 then	
	--	if Npc_LittleSantaTalk(0) == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 471 then		
	--	if Npc_LittleSantaTalk(1) == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 472 then
	--	if Npc_LittleSantaTalk(92) == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 473 then			
	--	if Npc_LittleSantaTalk(93) == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 474 then	
	--	if Npc_LittleSantaTalk(96) == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 475 then		
	--	if Npc_LittleSantaTalk(97) == 1 then
	--		return 1
	--	end
	----
	elseif lpNpc.Class 	== 478 then
		if NpcLuckyCoinDelgado() == 1 then	--OK
			return 1
		end
	----
	--elseif lpNpc.Class 	== 479 then	
	--	if DoorkeeperTitus() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 522 then	
	--	if Npc_EventDungeonTalk() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 540 then
	--	if Npc_LugardTalk() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 541 then
	--	if Npc_MiddleTreasureTalk() == 1 then
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 542 then
	--	if Npc_LastTreasureTalk() == 1 then
	--		return 1
	--	end
	----
	elseif lpNpc.Class 	== 545 then
		if NpcKristenTalk() == 1 then	--OK
			return 1
		end
	----
	--elseif lpNpc.Class 	== 546 then
	--	if NpcRaulTalk() == 1 then	--OK
	--		return 1
	--	end
	----
	--elseif lpNpc.Class 	== 547 then
	--	if NpcJuliaTalk() == 1 then
	--		return 1
	--	end
	----
	elseif lpNpc.Class 	== 577 then
		if NpcRehina() == 1 then	--OK
			return 1
		end
	----
	elseif lpNpc.Class 	== 578 then
		if NpcVolo() == 1 then	--OK
			return 1
		end
	----
	--elseif lpNpc.Class 	== 579 then
	--	if NpcDabide() == 1 then
	--		return 1
	--	end
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
			--gObjTeleport(lpObj.aIndex, 40, 220, 15)
		end
		----
		return 1
	end
	----
	return 0
end
------------------------------------------------------------------------------------------

-- Complete
function NpcCryWolfAltar()	
	return 1
end
------------------------------------------------------------------------------------------

--
function NpcCastleCrown()	
	----
end
------------------------------------------------------------------------------------------

--
function NpcCastleSwitch()
	----
end
------------------------------------------------------------------------------------------

--
function NpcCastleGateLever() 
	----
end
------------------------------------------------------------------------------------------

--
function NpcCastleHuntZoneGuard()	
	----
end
------------------------------------------------------------------------------------------

--
function NpcSiegeMachine_Attack()	
	----
end
------------------------------------------------------------------------------------------

--
function NpcSiegeMachine_Defense()	
	----
end
------------------------------------------------------------------------------------------

--
function NpcElderCircle()	
	----
end
------------------------------------------------------------------------------------------

--
function NpcCastleGuard()	
	----
end
------------------------------------------------------------------------------------------

--
function NpcDarkSpiritTrainer()	
	----
end
------------------------------------------------------------------------------------------

function NpcAngelKing()
	----
end
------------------------------------------------------------------------------------------

function NpcAngelMessanger()
	----
end
------------------------------------------------------------------------------------------

--
function NpcServerDivision()
	if gEnableServerDivision < 1 then
		return 1
	end
	----
	if lpObj.lpGuild > 0 then
		GCServerCmd(lpObj.aIndex, 6, 0, 0)
		return 1
	end
	----
	--lpObj.m_IfStateType		= 11
	--lpObj.m_IfStateState	= 0
	--lpObj.m_IfStateUse		= 1
	----
	return 0
end
------------------------------------------------------------------------------------------

function NpcEventChipNPC()
	----
end
------------------------------------------------------------------------------------------

function NpcRusipher()
	----
end
------------------------------------------------------------------------------------------

-- Complete
function NpcChaosGoblelin()
	gUserFindDevilSquareKeyEyes(lpObj.aIndex)
	return 0
end
------------------------------------------------------------------------------------------

-- Complete
function NpcBattleAnnouncer()
	return 0;
end
------------------------------------------------------------------------------------------

-- Complete
function NpcWarehouse()
	if gObjCalDistance(lpNpc.aIndex, lpObj.aIndex) > 10 then
		return 0
	end
	----
	return 1
end
------------------------------------------------------------------------------------------

-- Complete
function NpcGuildMasterTalk()
	if gGuildCreate == 0 then
		ChatTargetSend('You cannot create guild for now', lpNpc.aIndex, lpObj.aIndex)
		return 1
	end
	----
	if lpObj.GuildNumber > 0 then
		ChatTargetSend(GetWtf(4, 116), lpNpc.aIndex, lpObj.aIndex)
		return 1
	end
	----
	--if gGensSystem_GuildCreateReqGens == 1
	--	if gGensSystem_GetGensInfluence(lpObj.aIndex) == 0 then
	--		GCServerMsgStringSend("You need Gens family to create guild", lpObj.aIndex, 1)
	--	end
	--end
	----
	nRet = gObjGuildMasterCapacityTest(lpObj.aIndex)
	----
	if nRet == 0 then
		ChatTargetSend(GetWtf(4, 117), lpNpc.aIndex, lpObj.aIndex)
	else
		GCGuildMasterQuestionSend(lpObj.aIndex)
	end
	----
	return 1
end
------------------------------------------------------------------------------------------

-- Complete
function NpcNoriaRara()
	if gLanguage == 0 and gEnableEventNPCTalk then
		GCServerCmd(lpObj.aIndex, 4, 2, 0)
	end
	----
	return 0
end
------------------------------------------------------------------------------------------

-- Complete
function NpcNoriJangIn()
	return 0
end
------------------------------------------------------------------------------------------

-- Complete
function NpcDeviasMadam()
	if gLanguage == 0 and gEnableEventNPCTalk then
		GCServerCmd(lpObj.aIndex, 4, 1, 0)
	end
	----
	return 0
end
------------------------------------------------------------------------------------------

-- Complete
function NpcDeviasWizard()
	return 0
end
------------------------------------------------------------------------------------------

-- Complete
function NpcDeviasWeapon()
	return 0
end
------------------------------------------------------------------------------------------

-- Complete
function NpcDeviasGuard()	
	return 1
end
------------------------------------------------------------------------------------------

-- Complete
function NpcRolensiaGuard()
	if gLanguage == 0 and gEnableEventNPCTalk then
		GCServerCmd(lpObj.aIndex, 4, 0, 0)
	end
	----
	return 1
end
------------------------------------------------------------------------------------------

-- Complete
function NpcRoadMerchant()
	return 0
end
------------------------------------------------------------------------------------------

-- Complete
function NpcRorensiaSmith()
	return 0
end
------------------------------------------------------------------------------------------

-- Complete
function NpcPosionGirl()
	return 0
end
------------------------------------------------------------------------------------------

-- Need some fixes & checks
function NpcFasi()
	if gLanguage > 0 and gLanguage ~= 8 and gDevilSquareEvent > 0 then
		GCServerCmd(lpObj.aIndex, 1, 6, 0)
	end
	----
	PKFlag = 0
	----
	if lpObj.PartyNumber >= 0 then -- Need check syntax
		if gParty_GetPKPartyPenalty(lpObj.PartyNumber) > 4 then
			PKFlag = 1
		end
	elseif lpObj.m_PK_Level > 4 then
		PKFlag = 1
	end
	----
	if PKFlag == 1 then
		hour 	= 0
		min 	= 0
		----
		if lpNpc.TargetNumber ~= lpObj.aIndex then
			--if gObjPkTime(lpObj.aIndex, hour, min) == 1 then
			--	ChatTargetSend(GetWtf(4, 79), lpNpc.aIndex, lpObj.aIndex)
				--lpNpc.TargetNumber = lpObj.aIndex
				----
			--	if gPkLimitFree < 1 then
			--		return 1
			--	end
			--end
		elseif gPkLimitFree < 1 then
			--TextID --= rand() % 2
			--ChatTargetSend(GetWtf(4, 100 - TextID), lpNpc.aIndex, lpObj.aIndex)
			--lpNpc.TargetNumber = 0
			return 1
		end
	end
	----
	return 0
end
------------------------------------------------------------------------------------------

-- Complete
function NpcRolensiaMadam()
	return 0
end
------------------------------------------------------------------------------------------

function NpcJewelMixDealer()
	----
end
------------------------------------------------------------------------------------------

function Npc_Dialog()
	----
end
------------------------------------------------------------------------------------------

-- Complete
function NpcReira()
	return 0
end
------------------------------------------------------------------------------------------

--
function NpcMainatenceMachine()
	g_KanturuEntranceNPC_NotifyEntranceInfo(lpObj.aIndex)
	return 1
end
------------------------------------------------------------------------------------------

function NpcJewelOfHarmony()
	----
end
------------------------------------------------------------------------------------------

function NpcChaosCardMaster()
	----
end
------------------------------------------------------------------------------------------

-- Complete
function NpcQuartermasterPamella()
	return 1
end
------------------------------------------------------------------------------------------

-- Complete
function NpcQuartermasterAngela()
	return 1
end
------------------------------------------------------------------------------------------

-- Complete
function NPcFireCrackerStore()
	return 1
end
------------------------------------------------------------------------------------------

function NpcIllusionHolyRelicsStatue()
	----
end
------------------------------------------------------------------------------------------

function NpcHolyRelicsBoxAllied()
	----
end
------------------------------------------------------------------------------------------

function NpcHolyRelicsBoxIllusion()
	----
end
------------------------------------------------------------------------------------------

function NpcPlatformMirage()
	----
end
------------------------------------------------------------------------------------------

-- Complete
function NpcSilviaTalk()
	return 1
end
------------------------------------------------------------------------------------------

-- Complete
function NpcRheaTalk()
	return 1
end
------------------------------------------------------------------------------------------

-- Complete
function NpcMarceTalk()
	return 1
end
------------------------------------------------------------------------------------------

-- Complete
function NpcCherryTalk()
	return 0
end
------------------------------------------------------------------------------------------

function NpcSeedMaster()
	----
end
------------------------------------------------------------------------------------------

function NpcSeedResearcher()
	----
end
------------------------------------------------------------------------------------------

--
function Npc_SantaTalk()
	--GEReqCheckEventNPCTarget?
	--EGAnsRegXMasGetPayItem(lpObj.aIndex, 1)
	return 1
end
------------------------------------------------------------------------------------------

--
function Npc_SnowManTalk()
	--CGReqCheckSnowManNPC(lpObj.aIndex)
	return 1
end
------------------------------------------------------------------------------------------

--
function Npc_LittleSantaTalk(BuffEffect)
	if BuffEffect 		== 0 then	--AT_NULL_BUFF_EFFECT
		--lpObj.Life = lpObj.AddLife + lpObj.MaxLife;
		--GCReFillSend(lpObj.aIndex, lpObj.Life, 0xFF, 0, lpObj.iShield)
	elseif BuffEffect 	== 1 then	--AT_INCREASE_ATTACK
		--lpObj.Mana = lpObj.AddMana + lpObj.MaxMana;
		--GCManaSend(lpObj.aIndex, lpObj.Mana, 0xFF, 0, lpObj.BP)
	elseif BuffEffect 	== 94 then	--AT_XMASS_ATTACK
		gObjAddBuffEffect(lpObj.aIndex, BuffEffect, 57, 30, 0, 0, 1800)
	elseif BuffEffect 	== 95 then	--AT_XMASS_DEFENSE
		gObjAddBuffEffect(lpObj.aIndex, BuffEffect, 58, 100, 0, 0, 1800)
	elseif BuffEffect 	== 92 then	--AT_XMASS_HEALTH
		gObjAddBuffEffect(lpObj.aIndex, BuffEffect, 4, 500, 0, 0, 1800)
	elseif BuffEffect 	== 93 then	--AT_XMASS_MANA
		gObjAddBuffEffect(lpObj.aIndex, BuffEffect, 5, 500, 0, 0, 1800)
	elseif BuffEffect 	== 96 then	--AT_XMASS_DEXTERITY
		gObjAddBuffEffect(lpObj.aIndex, BuffEffect, 1, 15, 0, 0, 1800)
	elseif BuffEffect 	== 97 then	--AT_XMASS_AG_RECOVERY
		gObjAddBuffEffect(lpObj.aIndex, BuffEffect, 59, 10, 0, 0, 1800)
	end
	----
	return 1
end
------------------------------------------------------------------------------------------

-- Complete
function NpcLuckyCoinDelgado()
	return 1
end
------------------------------------------------------------------------------------------

--
function DoorkeeperTitus()
	----
end
------------------------------------------------------------------------------------------

--
function Npc_EventDungeonTalk()
	----
end
------------------------------------------------------------------------------------------

--
function Npc_LugardTalk()
	----
end
------------------------------------------------------------------------------------------

--
function Npc_MiddleTreasureTalk()
	--CDoppelganger::MiddleTreasureAct(lpNpc, lpObj)
	return 1
end
------------------------------------------------------------------------------------------

--
function Npc_LastTreasureTalk()
	--CDoppelganger::LastTreasureAct(lpNpc, lpObj)
	return 1
end
------------------------------------------------------------------------------------------

-- Complete
function NpcKristenTalk()
	return 1
end
------------------------------------------------------------------------------------------

-- Complete
function NpcRaulTalk()
	NpcJewelMixDealer()
	return 1
end
------------------------------------------------------------------------------------------

--
function NpcJuliaTalk()
	----
end
------------------------------------------------------------------------------------------

-- Complete
function NpcRehina()
	return 1
end
------------------------------------------------------------------------------------------

-- Complete
function NpcVolo()
	return 1
end
------------------------------------------------------------------------------------------

--
function NpcDabide()
	----
end
------------------------------------------------------------------------------------------