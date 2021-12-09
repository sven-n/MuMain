// NpcTalk.cpp: implementation of the CNpcTalk class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NpcTalk.h"
#include "ArcaBattle.h"
#include "BloodCastle.h"
#include "CastleSiege.h"
#include "CastleSiegeSync.h"
#include "CastleSiegeWeapon.h"
#include "ChaosBox.h"
#include "CustomNpcCollector.h"
#include "CustomNpcCommand.h"
#include "CustomNpcMove.h"
#include "CustomNpcQuest.h"
#include "DevilSquare.h"
#include "DoubleGoer.h"
#include "DSProtocol.h"
#include "Duel.h"
#include "EffectManager.h"
#include "EventQuickly.h"
#include "EventGvG.h"
#include "EventTvT.h"
#include "GameMain.h"
#include "GensSystem.h"
#include "IllusionTemple.h"
#include "ImperialGuardian.h"
#include "ItemBagManager.h"
#include "KanturuEntranceNPC.h"
#include "Map.h"
#include "Message.h"
#include "Notice.h"
#include "Quest.h"
#include "QuestWorld.h"
#include "ServerInfo.h"
#include "ShopManager.h"
#include "Trade.h"
#include "Util.h"
#include "Warehouse.h"

CNpcTalk gNpcTalk;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNpcTalk::CNpcTalk() // OK
{

}

CNpcTalk::~CNpcTalk() // OK
{

}

bool CNpcTalk::NpcTalk(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	if(lpObj->Interface.use != 0)
	{
		return 1;
	}

	if(lpObj->TradeDuel != 0)
	{
		return 1;
	}

	if(gQuest.NpcTalk(lpNpc,lpObj) != 0)
	{
		return 1;
	}

	if (gCustomNpcQuest.CheckNpcQuest(lpObj,lpNpc) != 0)
	{
		return 1;
	}

	if (gCustomNpcCommand.GetNpcCommand(lpObj,lpNpc) != 0)
	{
		return 1;
	}

	if (gCustomNpcMove.GetNpcMove(lpObj,lpNpc->Class,lpNpc->Map,lpNpc->X,lpNpc->Y) != 0)
	{
		return 1;
	}

	if (gTvTEvent.Dialog(lpObj,lpNpc))
	{
		return 1;
	}

	if (gGvGEvent.Dialog(lpObj,lpNpc))
	{
		return 1;
	}

	switch(lpNpc->Class)
	{
		case 205:
			this->NpcCrywolfAltar(lpNpc,lpObj);
			break;
		case 206:
			this->NpcCrywolfAltar(lpNpc,lpObj);
			break;
		case 207: 
			this->NpcCrywolfAltar(lpNpc,lpObj);
			break;
		case 208:
			this->NpcCrywolfAltar(lpNpc,lpObj);
			break;
		case 209: 
			this->NpcCrywolfAltar(lpNpc,lpObj);
			break;
		case 216:
			this->NpcCastleSiegeCrown(lpNpc,lpObj);
			break;
		case 217:
			this->NpcCastleSiegeCrownSwitch(lpNpc,lpObj);
			break;
		case 218:
			this->NpcCastleSiegeCrownSwitch(lpNpc,lpObj);
			break;
		case 219:
			this->NpcCastleSiegeGateSwitch(lpNpc,lpObj);
			break;
		case 220:
			this->NpcCastleSiegeGuard(lpNpc,lpObj);
			break;
		case 221:
			this->NpcCastleSiegeWeaponOffense(lpNpc,lpObj);
			break;
		case 222:
			this->NpcCastleSiegeWeaponDefense(lpNpc,lpObj);
			break;
		case 223:
			this->NpcCastleSiegeSenior(lpNpc,lpObj);
			break;
		case 224:
			this->NpcCastleSiegeGuardsman(lpNpc,lpObj);
			break;
		case 226:
			this->NpcTrainer(lpNpc,lpObj);
			break;
		case 232:
			this->NpcAngelKing(lpNpc,lpObj);
			break;
		case 233:
			this->NpcAngelMessenger(lpNpc,lpObj);
			break;
		case 234:
			this->NpcServerTransfer(lpNpc,lpObj);
			break;
		case 236:
			gCustomNpcCollector.NpcTalk(lpNpc,lpObj);
			break;
		case 237:
			this->NpcCharon(lpNpc,lpObj);
			break;
		case 238:
			this->NpcChaosGoblin(lpNpc,lpObj);
			break;
		case 240:
			this->NpcWarehouse(lpNpc,lpObj);
			break;
		case 241:
			this->NpcGuildMaster(lpNpc,lpObj);
			break;
		case 249:
			this->NpcGuardTalk(lpNpc,lpObj);
			break;
		case 256:
			this->NpcLehap(lpNpc,lpObj);
			break;
		case 257:
			this->NpcElfBuffer(lpNpc,lpObj);
			break;
		case 367:
			this->NpcKanturuGate(lpNpc,lpObj);
			break;
		case 368:
			this->NpcElpis(lpNpc,lpObj);
			break;
		case 369:
			this->NpcOsbourne(lpNpc,lpObj);
			break;
		case 370:
			this->NpcJerridon(lpNpc,lpObj);
			break;
		case 371:
			this->NpcLeoTheHelper(lpNpc,lpObj);
			break;
		case 375:
			this->NpcChaosCardMaster(lpNpc,lpObj);
			break;
		case 380:
			this->NpcStoneStatue(lpNpc,lpObj);
			break;
		case 383:
			this->NpcYellowStorage(lpNpc,lpObj);
			break;
		case 384:
			this->NpcBlueStorage(lpNpc,lpObj);
			break;
		case 385:
			this->NpcMirageTheMummy(lpNpc,lpObj);
			break;
		case 450:
			this->NpcGovernorBlossom(lpNpc,lpObj);
			break;
		case 452:
			this->NpcSeedMaster(lpNpc,lpObj);
			break;
		case 453:
			this->NpcSeedResearcher(lpNpc,lpObj);
			break;
		case 465:
				if(gEventQuickly.m_NpcIndex > 0)
				{
					gEventQuickly.ClickNPC(lpObj);
					break;
				}
			this->NpcSantaClaus(lpNpc,lpObj);
			break;
		case 467:
			this->NpcSnowman(lpNpc,lpObj);
			break;
		case 468:
			this->NpcChristmas(lpNpc,lpObj);
			break;
		case 469:
			this->NpcChristmas(lpNpc,lpObj);
			break;
		case 470:
			this->NpcChristmas(lpNpc,lpObj);
			break;
		case 471:
			this->NpcChristmas(lpNpc,lpObj);
			break;
		case 472:
			this->NpcChristmas(lpNpc,lpObj);
			break;
		case 473:
			this->NpcChristmas(lpNpc,lpObj);
			break;
		case 474:
			this->NpcChristmas(lpNpc,lpObj);
			break;
		case 475:
			this->NpcChristmas(lpNpc,lpObj);
			break;
#if(GAMESERVER_UPDATE>=502)
		case 478:
			this->NpcDelgado(lpNpc,lpObj);
			break;
		case 479:
			this->NpcTitus(lpNpc,lpObj);
			break;
		case 522:
			this->NpcJerryTheAdviser(lpNpc,lpObj);
			break;
		case 540:
			this->NpcSartiganTheAngel(lpNpc,lpObj);
			break;
		case 541:
			this->NpcSilverRewardChest(lpNpc,lpObj);
			break;
		case 542:
			this->NpcGoldenRewardChest(lpNpc,lpObj);
			break;
		case 543:
			this->NpcGensDuprianSteward(lpNpc,lpObj);
			break;
		case 544:
			this->NpcGensVarnertSteward(lpNpc,lpObj);
			break;
		case 546:
			this->NpcRaul(lpNpc,lpObj);
			break;
		case 547:
			this->NpcJulia(lpNpc,lpObj);
			break;
		case 566:
			this->NpcTercia(lpNpc, lpObj);
			break;
		case 567:
			this->NpcVeina(lpNpc, lpObj);
			break;
		case 568:
			this->NpcZyro(lpNpc, lpObj);
			break;
		case 581:
			this->NpcDeruvish(lpNpc, lpObj);
			break;
		case 579:
			this->NpcDavid(lpNpc,lpObj);
			break;
#endif

#if(GAMESERVER_UPDATE>=803)
		case 580:
			this->NpcSloveCaptain(lpNpc,lpObj);
			break;
		case 582:
			this->NpcAdniel(lpNpc,lpObj);
			break;
		case 584:
			this->NpcLesnar(lpNpc,lpObj);
			break;
		case 651:
			this->NpcPersonalShopBoard(lpNpc,lpObj);
			break;
#endif
		default:
			return 0;
	}

	return 1;
}

void CNpcTalk::NpcCrywolfAltar(LPOBJ lpNpc,LPOBJ lpObj) // OK
{

}

void CNpcTalk::NpcCastleSiegeCrown(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_TYPE==1)
	if ( gObjIsConnected(lpObj->Index) == FALSE )
	{
		return;
	}

	if(gCastleSiege.GetCastleState() != CASTLESIEGE_STATE_STARTSIEGE)
	{
		return;
	}
	
	if(lpObj->CsJoinSide < 2 || lpObj->CsGuildInvolved == FALSE )
	{
		return;
	}

	if(gCastleSiege.CheckUnionGuildMaster(lpObj->Index) == FALSE )
	{
		return;
	}

	if( (abs(lpObj->Y - lpNpc->Y)) > 3 || (abs(lpObj->X - lpNpc->X)) > 3)
	{
		return;
	}

	if(gCastleSiege.GetRegCrownAvailable() == FALSE )
	{
		return;
	}

	int iUserIndex = gCastleSiege.GetCrownUserIndex();

	if(gObjIsConnected(iUserIndex) == FALSE )
	{
		int iSwitchIndex1 = gCastleSiege.GetCrownSwitchUserIndex(217);
		int iSwitchIndex2 = gCastleSiege.GetCrownSwitchUserIndex(218);

		if(gObjIsConnected(iSwitchIndex1) == FALSE || gObjIsConnected(iSwitchIndex2) == FALSE )
		{
			GCAnsCsAccessCrownState(lpObj->Index,4);
			LogAdd(LOG_BLACK,"[CastleSiege] [%s][%s] Failed to Register Castle Crown (GUILD:%s)",lpObj->Account,lpObj->Name,lpObj->GuildName);
			return;
		}

		if(lpObj->CsJoinSide != gObj[iSwitchIndex1].CsJoinSide || lpObj->CsJoinSide != gObj[iSwitchIndex2].CsJoinSide)
		{
			GCAnsCsAccessCrownState(lpObj->Index,4);
			LogAdd(LOG_BLACK,"[CastleSiege] [%s][%s] Failed to Register Castle Crown (GUILD:%s) (S1:%s/%s)(S2:%s/%s)",lpObj->Account,lpObj->Name,lpObj->GuildName,gObj[iSwitchIndex1].Name,gObj[iSwitchIndex1].GuildName,gObj[iSwitchIndex2].Name,gObj[iSwitchIndex2].GuildName);
			return;
		}
		else
		{
			GCAnsCsAccessCrownState(lpObj->Index,0);
			gCastleSiege.SetCrownUserIndex(lpObj->Index);
			gCastleSiege.SetCrownAccessUserX((BYTE)lpObj->X);
			gCastleSiege.SetCrownAccessUserY((BYTE)lpObj->Y);
			gCastleSiege.SetCrownAccessTickCount();
			gCastleSiege.NotifyAllUserCsProgState(0,lpObj->GuildName);
			LogAdd(LOG_BLACK,"[CastleSiege] [%s][%s] Start to Register Castle Crown (GUILD:%s)",lpObj->Account,lpObj->Name,lpObj->GuildName);
		}
	}
	else if(lpObj->Index != iUserIndex)
	{
		GCAnsCsAccessCrownState(lpObj->Index,3);
	}
	#endif
}

void CNpcTalk::NpcCastleSiegeCrownSwitch(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_TYPE==1)
	if(gObjIsConnected(lpObj->Index) == FALSE )
	{
		return;
	}

	if(gCastleSiege.GetCastleState() != CASTLESIEGE_STATE_STARTSIEGE)
	{
		return;
	}
	
	if(lpObj->CsJoinSide < 2)
	{
		return;
	}

	if( (abs(lpObj->Y - lpNpc->Y)) > 3 || (abs(lpObj->X - lpNpc->X)) > 3)
	{
		return;
	}

	if(gCastleSiege.CheckGuardianStatueExist() == TRUE)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(427));
		return;
	}

	int iUserIndex = gCastleSiege.GetCrownSwitchUserIndex(lpNpc->Class);

	if(!gObjIsConnected(iUserIndex))
	{
		GCAnsCsAccessSwitchState(lpObj->Index,lpNpc->Index,-1,1);
		gCastleSiege.SetCrownSwitchUserIndex(lpNpc->Class,lpObj->Index);
		LogAdd(LOG_BLACK,"[CastleSiege] [%s][%s] Start to Push Castle Crown Switch (GUILD:%s) - CS X:%d/Y:%d",lpObj->Account,lpObj->Name,lpObj->GuildName,lpNpc->X,lpNpc->Y);
	}
	else if(lpObj->Index != iUserIndex)
	{
		GCAnsCsAccessSwitchState(lpObj->Index,lpNpc->Index,iUserIndex,2);
	}

	#endif
}

void CNpcTalk::NpcCastleSiegeGateSwitch(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_TYPE==1)

	if ( (lpObj->Interface.use) > 0 )
	{
		return;
	}

	BOOL bControlEnable = FALSE;
	BYTE btResult = 0;
	int iGateIndex = -1;

	if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)
	{
		if(lpObj->CsJoinSide != TRUE)
		{
			btResult = 4;
			bControlEnable = FALSE;
		}
		else
		{
			bControlEnable = TRUE;
		}
	}
	else
	{
		if(gCastleSiege.CheckCastleOwnerMember(lpObj->Index) == FALSE && gCastleSiege.CheckCastleOwnerUnionMember(lpObj->Index) == FALSE)
		{
			btResult = 4;
			bControlEnable = FALSE;
		}
		else
		{
			bControlEnable = TRUE;
		}
	}

	if(bControlEnable != FALSE)
	{
		if(gCastleSiege.CheckLeverAlive(lpNpc->Index) == TRUE)
		{
			if(gCastleSiege.CheckCsGateAlive(lpNpc->CsGateLeverLinkIndex) == TRUE)
			{
				btResult = 1;
				iGateIndex = lpNpc->CsGateLeverLinkIndex;
			}
			else
			{
				btResult = 2;
			}
		}
		else
		{
			btResult = 3;
		}

	}

	GCAnsCsGateState(lpObj->Index,btResult,iGateIndex);

	if(btResult != 1)
	{
		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = 12;
	#endif
}

void CNpcTalk::NpcCastleSiegeGuard(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_TYPE==1)

	if(gObjIsConnected(lpObj->Index) == FALSE )
	{
		return;
	}

	if ( (lpObj->Interface.use ) > 0 )
	{
		return;
	}

	PMSG_ANS_GUARD_IN_CASTLE_HUNTZONE pResult = {0};

	pResult.h.set(0xB9,0x03,sizeof(PMSG_ANS_GUARD_IN_CASTLE_HUNTZONE));
	pResult.btResult = 1;
	pResult.iMaxPrice = 300000;
	pResult.iUnitOfPrice = 10000;
	pResult.btUsable = gCastleSiege.GetHuntZoneEnter();
	pResult.iCurrentPrice = gCastleSiegeSync.GetTaxHuntZone(lpObj->Index,FALSE);

	if(lpObj->Guild != NULL)
	{
		if(gCastleSiege.CheckCastleOwnerMember(lpObj->Index))
		{
			if(lpObj->GuildStatus == 0x80)
			{
				pResult.btResult = 3;
			}
			else
			{
				pResult.btResult = 2;
				pResult.btUsable = 0;
			}
		}
		else if(gCastleSiege.CheckCastleOwnerUnionMember(lpObj->Index))
		{
			pResult.btResult = 2;
			pResult.btUsable = 0;
		}
	}

	DataSend(lpObj->Index,(LPBYTE)&pResult,pResult.h.size);

	#endif
}

void CNpcTalk::NpcCastleSiegeWeaponOffense(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	gCastleSiegeWeapon.NpcCastleSiegeWeaponOffense(lpNpc,lpObj);
}

void CNpcTalk::NpcCastleSiegeWeaponDefense(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	gCastleSiegeWeapon.NpcCastleSiegeWeaponDefense(lpNpc,lpObj);
}

void CNpcTalk::NpcCastleSiegeSenior(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_TYPE==0)

	GCChatTargetSend(lpObj,lpNpc->Index,gMessage.GetMessage(240));

	#else

	if(lpObj->PShopOpen != 0)
	{
		return;
	}

	if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(241));
		return;
	}

	if(gCastleSiege.CheckGuildOwnCastle(lpObj->GuildName) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(242));
		return;
	}

	if(gCastleSiege.CheckGuildOwnCastle(lpObj->GuildName) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(242));
		return;
	}

	if(lpObj->GuildStatus != 0x40 && lpObj->GuildStatus != 0x80)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(243));
		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_CHAOS_BOX;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 12;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	GCTaxInfoSend(lpObj->Index,1,gCastleSiegeSync.GetTaxRateChaos(lpObj->Index));

	lpObj->IsChaosMixCompleted = 0;

	lpObj->CsGuildInvolved = 0;

	gObjInventoryTransaction(lpObj->Index);

	#endif
}

void CNpcTalk::NpcGuardTalk(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	GCChatTargetSend(lpObj,lpNpc->Index,gMessage.GetMessage(514));

}

void CNpcTalk::NpcCastleSiegeGuardsman(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_TYPE==1)

	if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)
	{
		GCChatTargetSend(lpObj,lpNpc->Index,gMessage.GetMessage(244));
		return;
	}

	#endif
	
	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_COMMON;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 13;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
}

void CNpcTalk::NpcTrainer(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	if(lpObj->PShopOpen != 0)
	{
		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_TRAINER;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 7;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	GCTaxInfoSend(lpObj->Index,1,gCastleSiegeSync.GetTaxRateChaos(lpObj->Index));

	lpObj->IsChaosMixCompleted = 0;

	gObjInventoryTransaction(lpObj->Index);
}

void CNpcTalk::NpcAngelKing(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	gBloodCastle.NpcAngelKing(lpNpc,lpObj);
}

void CNpcTalk::NpcAngelMessenger(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	gBloodCastle.NpcAngelMessenger(lpNpc,lpObj);
}

void CNpcTalk::NpcServerTransfer(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	if(lpObj->Guild != 0)
	{
		GCServerCommandSend(lpObj->Index,6,0,0);
		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_TRANSFER;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 5;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
}

void CNpcTalk::NpcCharon(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	gDevilSquare.NpcCharon(lpNpc,lpObj);
}

void CNpcTalk::NpcChaosGoblin(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	if(lpObj->PShopOpen != 0)
	{
		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_CHAOS_BOX;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 3;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	GCTaxInfoSend(lpObj->Index,1,gCastleSiegeSync.GetTaxRateChaos(lpObj->Index));

	lpObj->IsChaosMixCompleted = 0;

	gObjInventoryTransaction(lpObj->Index);
}

void CNpcTalk::NpcWarehouse(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_WAREHOUSE;
	lpObj->Interface.state = 0;

	gWarehouse.GDWarehouseItemSend(lpObj->Index,lpObj->Account);
}

void CNpcTalk::NpcGuildMaster(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	if(lpObj->GuildNumber > 0)
	{
		if (gServerInfo.m_GuildWarehouseEnable == 1)
		{
			if (gServerInfo.m_GuildWarehouseStatus != 0 && lpObj->GuildStatus < gServerInfo.m_GuildWarehouseStatus)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(840));
			}
			else
			{
				gWarehouse.GDWarehouseGuildItemSend(lpObj->Index,lpObj->GuildName);
			}
		}
		else
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(245));
		}
		return;
	}

	if(lpObj->Level < gServerInfo.m_GuildCreateMinLevel)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(246));
		return;
	}

	GCGuildMasterQuestionSend(lpObj->Index);
}

void CNpcTalk::NpcLehap(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_COMMON;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 9;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
}

void CNpcTalk::NpcElfBuffer(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	gQuestWorld.NpcElfBuffer(lpNpc,lpObj);

	#else

	if(lpObj->Level > gServerInfo.m_ElfBufferMaxLevel[lpObj->AccountLevel])
	{
		GCServerCommandSend(lpObj->Index,13,0,0);
		return;
	}

	if(lpObj->Reset > gServerInfo.m_ElfBufferMaxReset[lpObj->AccountLevel])
	{
		GCServerCommandSend(lpObj->Index,13,0,0);
		return;
	}

	gEffectManager.AddEffect(lpObj,0,EFFECT_ELF_BUFFER,1800,(gServerInfo.m_ElfBufferDamageConstA+(lpObj->Level/gServerInfo.m_ElfBufferDamageConstB)),(gServerInfo.m_ElfBufferDefenseConstA+(lpObj->Level/gServerInfo.m_ElfBufferDefenseConstB)),0,0);

	#endif
}

void CNpcTalk::NpcKanturuGate(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	gKanturuEntranceNPC.CGKanturuEnterInfoRecv(lpObj->Index);
}

void CNpcTalk::NpcElpis(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	if(lpObj->PShopOpen != 0)
	{
		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_CHAOS_BOX;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 17;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	GCTaxInfoSend(lpObj->Index,1,gCastleSiegeSync.GetTaxRateChaos(lpObj->Index));

	lpObj->IsChaosMixCompleted = 0;

	gObjInventoryTransaction(lpObj->Index);
}

void CNpcTalk::NpcOsbourne(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	if(lpObj->PShopOpen != 0)
	{
		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_CHAOS_BOX;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 18;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	GCTaxInfoSend(lpObj->Index,1,gCastleSiegeSync.GetTaxRateChaos(lpObj->Index));

	lpObj->IsChaosMixCompleted = 0;

	gObjInventoryTransaction(lpObj->Index);
}

void CNpcTalk::NpcJerridon(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	if(lpObj->PShopOpen != 0)
	{
		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_CHAOS_BOX;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 19;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	GCTaxInfoSend(lpObj->Index,1,gCastleSiegeSync.GetTaxRateChaos(lpObj->Index));

	lpObj->IsChaosMixCompleted = 0;

	gObjInventoryTransaction(lpObj->Index);
}

void CNpcTalk::NpcLeoTheHelper(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=202)

	GCServerCommandSend(lpObj->Index,15,1,0);

	#endif
}

void CNpcTalk::NpcChaosCardMaster(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	if(lpObj->PShopOpen != 0)
	{
		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_CHAOS_BOX;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 21;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	GCTaxInfoSend(lpObj->Index,1,gCastleSiegeSync.GetTaxRateChaos(lpObj->Index));

	lpObj->IsChaosMixCompleted = 0;

	gObjInventoryTransaction(lpObj->Index);
}

void CNpcTalk::NpcStoneStatue(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	gIllusionTemple.NpcStoneStatue(lpNpc,lpObj);
}

void CNpcTalk::NpcYellowStorage(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	gIllusionTemple.NpcYellowStorage(lpNpc,lpObj);
}

void CNpcTalk::NpcBlueStorage(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	gIllusionTemple.NpcBlueStorage(lpNpc,lpObj);
}

void CNpcTalk::NpcMirageTheMummy(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	gIllusionTemple.NpcMirageTheMummy(lpNpc,lpObj);
}

void CNpcTalk::NpcGovernorBlossom(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	if(lpObj->PShopOpen != 0)
	{
		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_CHAOS_BOX;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 22;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	GCTaxInfoSend(lpObj->Index,1,gCastleSiegeSync.GetTaxRateChaos(lpObj->Index));

	lpObj->IsChaosMixCompleted = 0;

	gObjInventoryTransaction(lpObj->Index);
}

void CNpcTalk::NpcSeedMaster(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=401)

	if(lpObj->PShopOpen != 0)
	{
		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_CHAOS_BOX;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 23;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	GCTaxInfoSend(lpObj->Index,1,gCastleSiegeSync.GetTaxRateChaos(lpObj->Index));

	lpObj->IsChaosMixCompleted = 0;

	gObjInventoryTransaction(lpObj->Index);

	#endif
}

void CNpcTalk::NpcSeedResearcher(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=401)

	if(lpObj->PShopOpen != 0)
	{
		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_CHAOS_BOX;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 24;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	GCTaxInfoSend(lpObj->Index,1,gCastleSiegeSync.GetTaxRateChaos(lpObj->Index));

	lpObj->IsChaosMixCompleted = 0;

	gObjInventoryTransaction(lpObj->Index);

	#endif
}

void CNpcTalk::NpcSantaClaus(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=401)

	GCServerCommandSend(lpObj->Index,16,0,0);

	#endif
}

void CNpcTalk::NpcSnowman(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=401)

	GCServerCommandSend(lpObj->Index,17,0,0);

	#endif
}

void CNpcTalk::NpcChristmas(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=401)

	if(lpNpc->Class == 470)
	{
		lpObj->Life = lpObj->MaxLife+lpObj->AddLife;
		GCLifeSend(lpObj->Index,0xFF,(int)lpObj->Life,lpObj->Shield);
		return;
	}

	if(lpNpc->Class == 471)
	{
		lpObj->Mana = lpObj->MaxMana+lpObj->AddMana;
		GCManaSend(lpObj->Index,0xFF,(int)lpObj->Mana,lpObj->BP);
		return;
	}
	
	switch(lpNpc->Class)
	{
		case 468:
			gEffectManager.AddEffect(lpObj,0,EFFECT_CHRISTMAS4,1800,30,0,0,0);
			break;
		case 469:
			gEffectManager.AddEffect(lpObj,0,EFFECT_CHRISTMAS5,1800,100,0,0,0);
			break;
		case 472:
			gEffectManager.AddEffect(lpObj,0,EFFECT_CHRISTMAS2,1800,500,0,0,0);
			break;
		case 473:
			gEffectManager.AddEffect(lpObj,0,EFFECT_CHRISTMAS3,1800,500,0,0,0);
			break;
		case 474:
			gEffectManager.AddEffect(lpObj,0,EFFECT_CHRISTMAS6,1800,15,0,0,0);
			break;
		case 475:
			gEffectManager.AddEffect(lpObj,0,EFFECT_CHRISTMAS7,1800,10,0,0,0);
			break;
	}

	#endif
}

void CNpcTalk::NpcDelgado(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=402)

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 32;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CNpcTalk::NpcTitus(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=402)

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_COMMON;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 33;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	gDuel.GCDuelNpcInfoSend(lpObj->Index);

	#endif
}

void CNpcTalk::NpcJerryTheAdviser(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	gImperialGuardian.NpcJerryTheAdviser(lpNpc,lpObj);

	#endif
}

void CNpcTalk::NpcSartiganTheAngel(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	gDoubleGoer.NpcSartiganTheAngel(lpNpc,lpObj);

	#endif
}

void CNpcTalk::NpcSilverRewardChest(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	gDoubleGoer.NpcSilverRewardChest(lpNpc,lpObj);

	#endif
}

void CNpcTalk::NpcGoldenRewardChest(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	gDoubleGoer.NpcGoldenRewardChest(lpNpc,lpObj);

	#endif
}

void CNpcTalk::NpcGensDuprianSteward(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	gGensSystem.NpcGensDuprianSteward(lpNpc,lpObj);

	#endif
}

void CNpcTalk::NpcGensVarnertSteward(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	gGensSystem.NpcGensVarnertSteward(lpNpc,lpObj);

	#endif
}

void CNpcTalk::NpcRaul(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_COMMON;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 9;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CNpcTalk::NpcJulia(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 37;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CNpcTalk::NpcDavid(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	if(lpObj->PShopOpen != 0)
	{
		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_CHAOS_BOX;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 38;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	GCTaxInfoSend(lpObj->Index,1,gCastleSiegeSync.GetTaxRateChaos(lpObj->Index));

	lpObj->IsChaosMixCompleted = 0;

	gObjInventoryTransaction(lpObj->Index);

	#endif
}

void CNpcTalk::NpcSloveCaptain(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 39;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CNpcTalk::NpcAdniel(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(lpObj->PShopOpen != 0)
	{
		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_CHAOS_BOX;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 41;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	GCTaxInfoSend(lpObj->Index,1,gCastleSiegeSync.GetTaxRateChaos(lpObj->Index));

	lpObj->IsChaosMixCompleted = 0;

	gObjInventoryTransaction(lpObj->Index);

	#endif
}

void CNpcTalk::NpcLesnar(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=702)

	gArcaBattle.NpcLesnar(lpNpc,lpObj);

	#endif
}

void CNpcTalk::NpcTercia(LPOBJ lpNpc, LPOBJ lpObj) // OK
{
#if(GAMESERVER_UPDATE>=602)

lpObj->Interface.use = 1;
lpObj->Interface.type = INTERFACE_QUEST_WORLD;
lpObj->Interface.state = 0;
lpObj->QuestWorldMonsterClass = lpNpc->Class;

PMSG_QUEST_WORLD_NPC_TALK_SEND pMsg;

pMsg.header.set(0xF9, 0x01, sizeof(pMsg));

pMsg.MonsterClass = lpNpc->Class;

pMsg.contribution = 0;

DataSend(lpObj->Index, (BYTE*)&pMsg, pMsg.header.size);

#endif
}

void CNpcTalk::NpcVeina(LPOBJ lpNpc, LPOBJ lpObj) // OK
{
#if(GAMESERVER_UPDATE>=602)

lpObj->Interface.use = 1;
lpObj->Interface.type = INTERFACE_QUEST_WORLD;
lpObj->Interface.state = 0;
lpObj->QuestWorldMonsterClass = lpNpc->Class;

PMSG_QUEST_WORLD_NPC_TALK_SEND pMsg;

pMsg.header.set(0xF9, 0x01, sizeof(pMsg));

pMsg.MonsterClass = lpNpc->Class;

pMsg.contribution = 0;

DataSend(lpObj->Index, (BYTE*)&pMsg, pMsg.header.size);

#endif
}

void CNpcTalk::NpcZyro(LPOBJ lpNpc, LPOBJ lpObj) // OK
{
#if(GAMESERVER_UPDATE>=602)

lpObj->Interface.use = 1;
lpObj->Interface.type = INTERFACE_QUEST_WORLD;
lpObj->Interface.state = 0;
lpObj->QuestWorldMonsterClass = lpNpc->Class;

PMSG_QUEST_WORLD_NPC_TALK_SEND pMsg;

pMsg.header.set(0xF9, 0x01, sizeof(pMsg));

pMsg.MonsterClass = lpNpc->Class;

pMsg.contribution = 0;

DataSend(lpObj->Index, (BYTE*)&pMsg, pMsg.header.size);

// ----
return;
#endif
}

void CNpcTalk::NpcDeruvish(LPOBJ lpNpc, LPOBJ lpObj)
{
#if(GAMESERVER_UPDATE>=701)

lpObj->Interface.use = 1;
lpObj->Interface.type = INTERFACE_QUEST_WORLD;
lpObj->Interface.state = 0;
lpObj->QuestWorldMonsterClass = lpNpc->Class;

PMSG_QUEST_WORLD_NPC_TALK_SEND pMsg;

pMsg.header.set(0xF9, 0x01, sizeof(pMsg));

pMsg.MonsterClass = lpNpc->Class;

pMsg.contribution = 0;

DataSend(lpObj->Index, (BYTE*)&pMsg, pMsg.header.size);
// ----
return;
#endif
}

void CNpcTalk::NpcPersonalShopBoard(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 48;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CNpcTalk::CGNpcTalkRecv(PMSG_NPC_TALK_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpObj->Interface.use != 0)
	{
		return;
	}

	if(lpObj->Live == 0)
	{
		return;
	}

	int bIndex = MAKE_NUMBERW(lpMsg->index[0],lpMsg->index[1]);

	if(OBJECT_RANGE(bIndex) == 0)
	{
		return;
	}

	LPOBJ lpNpc = &gObj[bIndex];

	if(lpNpc->Type != OBJECT_NPC || lpObj->Map != lpNpc->Map)
	{
		return;
	}

	if(lpNpc->Class == 367)
	{
		if(lpObj->X < (lpNpc->X-5) || lpObj->X > (lpNpc->X+5) || lpObj->Y < (lpNpc->Y-10) || lpObj->Y > (lpNpc->Y+5))
		{
			return;
		}
	}
	else
	{
		if(lpObj->X < (lpNpc->X-5) || lpObj->X > (lpNpc->X+5) || lpObj->Y < (lpNpc->Y-5) || lpObj->Y > (lpNpc->Y+5))
		{
			return;
		}
	}

	if(this->NpcTalk(lpNpc,lpObj) != 0)
	{
		return;
	}

	if(SHOP_RANGE(lpNpc->ShopNumber) == 0)
	{
		return;
	}

	if(gServerInfo.m_PKDisableShop == 1 && lpObj->PKLevel >= 5)
	{
		GCChatTargetSend(lpObj,lpNpc->Index,gMessage.GetMessage(861));
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(861));
		return;
	}

	if(gShopManager.GetItemCountByIndex(lpNpc->ShopNumber) == 0)
	{
		GCChatTargetSend(lpObj,lpNpc->Index,gMessage.GetMessage(247));
		return;
	}

	lpObj->ShopDelay = GetTickCount();

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_SHOP;
	lpObj->Interface.state = 0;

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 0;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	lpObj->TargetShopNumber = lpNpc->ShopNumber;

	//if (gShopManager.GCShopItemListSendByMonsterClass(lpNpc->Class,lpNpc->Map,lpNpc->X,lpNpc->Y,aIndex) == 0)
	//{
		gShopManager.GCShopItemListSendByIndex(lpNpc->ShopNumber,aIndex);
	//}

	GCTaxInfoSend(aIndex,2,gCastleSiegeSync.GetTaxRateStore(aIndex));
}

void CNpcTalk::CGNpcTalkCloseRecv(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpObj->Interface.use == 0 || lpObj->Interface.type == INTERFACE_NONE || lpObj->Interface.type == INTERFACE_CHAOS_BOX)
	{
		return;
	}

	switch(lpObj->Interface.type)
	{
		case INTERFACE_TRADE:
			gTrade.CGTradeCancelButtonRecv(aIndex);
			break;
		case INTERFACE_SHOP:
			if(((lpObj->Interface.state==1)?(lpObj->Interface.state=0):1)==0){return;}
			lpObj->TargetShopNumber = -1;
			break;
		case INTERFACE_WAREHOUSE:
			gWarehouse.CGWarehouseClose(aIndex);
			break;
		case INTERFACE_TRAINER:
			gChaosBox.ChaosBoxInit(lpObj);
			gObjInventoryCommit(aIndex);
			break;
		case INTERFACE_QUEST_WORLD:
			lpObj->QuestWorldMonsterClass = -1;
			break;
	}

	lpObj->Interface.use = 0;
	lpObj->Interface.type = INTERFACE_NONE;
	lpObj->Interface.state = 0;
}

void CNpcTalk::CGNpcLeoTheHelperRecv(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=202)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	this->GDNpcLeoTheHelperSend(aIndex);

	#endif
}

void CNpcTalk::CGNpcSnowmanRecv(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=401)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	gObjMoveGate(aIndex,22);

	#endif
}

void CNpcTalk::CGNpcSantaClausRecv(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=401)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	this->GDNpcSantaClausSend(aIndex);

	#endif
}

void CNpcTalk::CGNpcJuliaRecv(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpObj->Map == MAP_LOREN_MARKET)
	{
		gObjMoveGate(lpObj->Index,17);
	}
	else
	{
		gObjMoveGate(lpObj->Index,333);
	}

	#endif
}

void CNpcTalk::DGNpcLeoTheHelperRecv(SDHP_NPC_LEO_THE_HELPER_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=202)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGNpcLeoTheHelperRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpMsg->status != 0)
	{
		GCServerCommandSend(lpObj->Index,15,0,0);
		return;
	}

	gItemBagManager.DropItemBySpecialValue(ITEM_BAG_LEO_THE_HELPER,lpObj,lpObj->Map,lpObj->X,lpObj->Y);

	this->GDNpcLeoTheHelperSaveSend(lpObj->Index,1);

	#endif
}

void CNpcTalk::DGNpcSantaClausRecv(SDHP_NPC_SANTA_CLAUS_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=401)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGNpcSantaClausRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpMsg->status != 0)
	{
		GCServerCommandSend(lpObj->Index,16,2,0);
		return;
	}

	gItemBagManager.DropItemBySpecialValue(ITEM_BAG_SANTA_CLAUS,lpObj,lpObj->Map,lpObj->X,lpObj->Y);

	this->GDNpcSantaClausSaveSend(lpObj->Index,1);

	#endif
}

void CNpcTalk::GDNpcLeoTheHelperSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=202)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_NPC_LEO_THE_HELPER_SEND pMsg;

	pMsg.header.set(0x0E,0x00,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CNpcTalk::GDNpcSantaClausSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=401)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_NPC_SANTA_CLAUS_SEND pMsg;

	pMsg.header.set(0x0E,0x01,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CNpcTalk::GDNpcLeoTheHelperSaveSend(int aIndex,BYTE status) // OK
{
	#if(GAMESERVER_UPDATE>=202)

	LPOBJ lpObj = &gObj[aIndex];

	SDHP_NPC_LEO_THE_HELPER_SAVE_SEND pMsg;

	pMsg.header.set(0x0E,0x30,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.status = status;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CNpcTalk::GDNpcSantaClausSaveSend(int aIndex,BYTE status) // OK
{
	#if(GAMESERVER_UPDATE>=401)

	LPOBJ lpObj = &gObj[aIndex];

	SDHP_NPC_SANTA_CLAUS_SAVE_SEND pMsg;

	pMsg.header.set(0x0E,0x31,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.status = status;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}
