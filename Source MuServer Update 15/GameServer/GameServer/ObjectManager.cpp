// ObjectManager.cpp: implementation of the CObjectManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ObjectManager.h"
#include "Attack.h"
#include "380ItemOption.h"
#include "BattleSoccerManager.h"
#include "BloodCastle.h"
#include "BonusManager.h"
#include "CastleDeep.h"
#include "CastleSiege.h"
#include "CastleSiegeSync.h"
#include "ChaosBox.h"
#include "ChaosCastle.h"
#include "CommandManager.h"
#include "Crywolf.h"
#include "CrywolfSync.h"
#include "CustomArena.h"
#include "CustomAttack.h"
#include "CustomDeathMessage.h"  
#include "CustomJewel.h"
#include "CustomMonster.h"
#include "CustomNpcQuest.h"
#include "CustomQuest.h"
#include "CustomPick.h"
#include "CustomRankUser.h"
#include "CustomStartItem.h"
#include "CustomWing.h"
#include "DarkSpirit.h"
#include "DevilSquare.h"
#include "DoubleGoer.h"
#include "DSProtocol.h"
#include "Duel.h"
#include "EffectManager.h"
#include "EventKillAll.h"
#include "ESProtocol.h"
#include "ExperienceTable.h"
#include "EventGvG.h"
#include "EventTvT.h"
#include "GameMaster.h"
#include "Gate.h"
#include "Guild.h"
#include "GuildMatching.h"
#include "Helper.h"
#include "IllusionTemple.h"
#include "ImperialGuardian.h"
#include "InvasionManager.h"
#include "InventoryEquipment.h"
#include "ItemOption.h"
#include "JewelOfHarmonyOption.h"
#include "JSProtocol.h"
#include "Kalima.h"
#include "Kanturu.h"
#include "KanturuBattleUserMng.h"
#include "LifeStone.h"
#include "Map.h"
#include "MapManager.h"
#include "MapServerManager.h"
#include "MasterSkillTree.h"
#include "Mercenary.h"
#include "Message.h"
#include "Monster.h"
#include "MonsterSkillManager.h"
#include "Move.h"
#include "MoveSummon.h"
#include "MuunSystem.h"
#include "Notice.h"
#include "Party.h"
#include "PartyMatching.h"
#include "PentagramSystem.h"
#include "Quest.h"
#include "QuestObjective.h"
#include "QuestWorld.h"
#include "QuestWorldObjective.h"
#include "Raklion.h"
#include "RaklionBattleUserMng.h"
#include "RaklionUtil.h"
#include "Reconnect.h"
#include "ReiDoMU.h"
#include "SerialCheck.h"
#include "ServerInfo.h"
#include "SetItemOption.h"
#include "SkillManager.h"
#include "SocketItemOption.h"
#include "Trade.h"
#include "Util.h"

CObjectManager gObjectManager;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CObjectManager::CObjectManager() // OK
{

}

CObjectManager::~CObjectManager() // OK
{

}

void CObjectManager::ObjectMsgProc(LPOBJ lpObj) // OK
{
	for(int n=0;n < MAX_MONSTER_SEND_MSG;n++)
	{
		if(gSMMsg[lpObj->Index][n].MsgCode != -1 && GetTickCount() > ((DWORD)gSMMsg[lpObj->Index][n].MsgTime))
		{
			if(lpObj->Type == OBJECT_MONSTER || lpObj->Type == OBJECT_NPC)
			{
				gObjMonsterStateProc(lpObj,gSMMsg[lpObj->Index][n].MsgCode,gSMMsg[lpObj->Index][n].SendUser,gSMMsg[lpObj->Index][n].SubCode);
				gSMMsg[lpObj->Index][n].Clear();
			}
			else
			{
				this->ObjectStateProc(lpObj,gSMMsg[lpObj->Index][n].MsgCode,gSMMsg[lpObj->Index][n].SendUser,gSMMsg[lpObj->Index][n].SubCode);
				gSMMsg[lpObj->Index][n].Clear();
			}
		}
	}
}

void CObjectManager::ObjectSetStateCreate(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnected(aIndex) == 0)
	{
		return;
	}

	if(lpObj->DieRegen == 1 && (GetTickCount()-lpObj->RegenTime) > (lpObj->MaxRegenTime+1000))
	{
		lpObj->DieRegen = 2;
		lpObj->State = OBJECT_DIECMD;
		this->CharacterCalcAttribute(aIndex);
	}

	if(lpObj->Type == OBJECT_USER && lpObj->Teleport == 2)
	{
		lpObj->Teleport = 3;

		lpObj->TX = lpObj->X;
		lpObj->TY = lpObj->Y;

		gMove.GCTeleportSend(aIndex,0,lpObj->Map,(BYTE)lpObj->X,(BYTE)lpObj->Y,lpObj->Dir);

		gObjViewportListProtocolCreate(lpObj);

		this->CharacterUpdateMapEffect(lpObj);

		lpObj->Teleport = 0;
		lpObj->PathCur = 0;
		lpObj->PathCount = 0;
		lpObj->PathStartEnd = 0;
	}

	if(lpObj->Type == OBJECT_MONSTER && lpObj->Teleport == 2)
	{
		lpObj->Teleport = 3;

		lpObj->TX = lpObj->X;
		lpObj->TY = lpObj->Y;

		gObjViewportListProtocolCreate(lpObj);

		lpObj->Teleport = 0;
		lpObj->PathCur = 0;
		lpObj->PathCount = 0;
		lpObj->PathStartEnd = 0;
	}

	if(lpObj->Type == OBJECT_NPC && lpObj->Teleport == 2)
	{
		lpObj->Teleport = 3;

		if(OBJECT_RANGE(lpObj->SummonIndex) == 0)
		{
			lpObj->X = lpObj->X+((GetLargeRand()%6)-3);
			lpObj->Y = lpObj->Y+((GetLargeRand()%6)-3);
			lpObj->TX = lpObj->X;
			lpObj->TY = lpObj->Y;
		}
		else
		{
			lpObj->TX = lpObj->X;
			lpObj->TY = lpObj->Y;
		}

		gObjViewportListProtocolCreate(lpObj);

		lpObj->Teleport = 0;
		lpObj->PathCur = 0;
		lpObj->PathCount = 0;
		lpObj->PathStartEnd = 0;
	}

	lpObj->Teleport = ((lpObj->Teleport==1)?((lpObj->DieRegen==0)?(((GetTickCount()-lpObj->TeleportTime)>100)?2:lpObj->Teleport):0):lpObj->Teleport);

	if(lpObj->Type == OBJECT_USER && lpObj->RegenOk == 2)
	{
		lpObj->RegenOk = 3;
		lpObj->State = OBJECT_CREATE;
		lpObj->X = lpObj->RegenMapX;
		lpObj->Y = lpObj->RegenMapY;
		lpObj->TX = lpObj->RegenMapX;
		lpObj->TY = lpObj->RegenMapY;
		lpObj->Map = lpObj->RegenMapNumber;

		this->CharacterUpdateMapEffect(lpObj);
	}
}

void CObjectManager::ObjectSetStateProc() // OK
{
	for(int n=0;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnected(n) == 0)
		{
			continue;
		}

		LPOBJ lpObj = &gObj[n];

		lpObj->Teleport = ((lpObj->Teleport==3)?0:lpObj->Teleport);

		if(lpObj->State == OBJECT_CREATE)
		{
			lpObj->State = OBJECT_PLAYING;
			lpObj->RegenOk = ((lpObj->RegenOk==3)?0:lpObj->RegenOk);
		}

		if(lpObj->Type == OBJECT_MONSTER)
		{
			if(lpObj->DieRegen != 2)
			{
				continue;
			}

			lpObj->AttackerKilled = 0;

			if(lpObj->CurrentAI != 0 && lpObj->RegenType != 0)
			{
				continue;
			}

			if(BC_MAP_RANGE(lpObj->Map) != 0 && lpObj->Class >= 131 && lpObj->Class <= 134)
			{
				gObjDel(lpObj->Index);
				continue;
			}

			if(CC_MAP_RANGE(lpObj->Map) != 0)
			{
				gObjDel(lpObj->Index);
				continue;
			}

			if(DG_MAP_RANGE(lpObj->Map) != 0)
			{
				gObjDel(lpObj->Index);
				continue;
			}

			if(IG_MAP_RANGE(lpObj->Map) != 0)
			{
				gObjDel(lpObj->Index);
				continue;
			}

			#if(GAMESERVER_TYPE==1)

			if(lpObj->Map == MAP_CRYWOLF && gCrywolf.GetCrywolfState() == CRYWOLF_STATE_READY)
			{
				continue;
			}

			#endif

			if(lpObj->Map == MAP_KANTURU3)
			{
				continue;
			}

			if(lpObj->Map == MAP_RAKLION2)
			{
				gObjDel(lpObj->Index);
				continue;
			}

			if(lpObj->Attribute == 60 || lpObj->Attribute == 61)
			{
				gObjDel(lpObj->Index);
				continue;
			}

			if(lpObj->Attribute == 62 && lpObj->Class == 295)
			{
				gObjDel(lpObj->Index);
				continue;
			}

			if(lpObj->Attribute == 62 && lpObj->Class >= 300 && lpObj->Class <= 303)
			{
				gObjDel(lpObj->Index);
				continue;
			}

			if(lpObj->Attribute == 62 && (GetTickCount()-lpObj->LastCheckTick) > 600000)
			{
				gObjDel(lpObj->Index);
				continue;
			}

			#if(GAMESERVER_TYPE==1)

			if(lpObj->Class == 278)
			{
				gLifeStone.DeleteLifeStone(lpObj->Index);
				gObjDel(lpObj->Index);
				continue;
			}

			if(lpObj->Class == 286 || lpObj->Class == 287)
			{
				gMercenary.DeleteMercenary(lpObj->Index);
				gObjDel(lpObj->Index);
				continue;
			}

			#endif

			if(lpObj->Class >= 647 && lpObj->Class <= 650)
			{
				gObjDel(lpObj->Index);
				continue;
			}

			if(lpObj->Class >= 652 && lpObj->Class <= 657)
			{
				gObjDel(lpObj->Index);
				continue;
			}

			if(lpObj->Class >= 674 && lpObj->Class <= 677)
			{
				gObjDel(lpObj->Index);
				continue;
			}

			lpObj->Live = 1;
			lpObj->ViewState = 0;
			lpObj->Teleport = 0;
			lpObj->Life = lpObj->MaxLife+lpObj->AddLife;
			lpObj->Mana = lpObj->MaxMana+lpObj->AddMana;

			gEffectManager.ClearAllEffect(lpObj);

			if(gObjMonsterRegen(lpObj) == 0)
			{
				continue;
			}

			lpObj->DieRegen = 0;
			lpObj->State = OBJECT_CREATE;

			gObjViewportListProtocolCreate(lpObj);
		}

		if(lpObj->Type == OBJECT_USER)
		{
			if(lpObj->DieRegen != 2)
			{
				continue;
			}

			lpObj->Live = 1;
			lpObj->ViewState = 0;
			lpObj->Teleport = 0;
			lpObj->Life = lpObj->MaxLife+lpObj->AddLife;
			lpObj->Mana = lpObj->MaxMana+lpObj->AddMana;
			lpObj->BP = lpObj->MaxBP+lpObj->AddBP;
			lpObj->Shield = lpObj->MaxShield+lpObj->AddShield;
			lpObj->MiniMapState = 0;
			lpObj->MiniMapValue = -1;
			lpObj->HPAutoRecuperationTime = GetTickCount();
			lpObj->MPAutoRecuperationTime = GetTickCount();
			lpObj->BPAutoRecuperationTime = GetTickCount();
			lpObj->SDAutoRecuperationTime = GetTickCount();
			lpObj->ResurrectionTalismanActive = 1;
			lpObj->ResurrectionTalismanMap = lpObj->Map;
			lpObj->ResurrectionTalismanX = lpObj->X;
			lpObj->ResurrectionTalismanY = lpObj->Y;

			memset(lpObj->SelfDefenseTime,0,sizeof(lpObj->SelfDefenseTime));

			gObjTimeCheckSelfDefense(lpObj);

			gEffectManager.ClearAllEffect(lpObj);

			gHelper.DisableHelper(lpObj);

			gObjClearViewport(lpObj);

			gDarkSpirit[lpObj->Index].SetMode(DARK_SPIRIT_MODE_NORMAL,-1);

			switch(lpObj->KillerType)
			{
				case 0:
					if(this->CharacterGetRespawnLocation(lpObj) == 0)
					{
						gMap[lpObj->Map].GetMapPos(lpObj->Map,&lpObj->X,&lpObj->Y);
					}
					break;
				case 1:
					if(this->CharacterGetRespawnLocation(lpObj) == 0)
					{
						gMap[lpObj->Map].GetMapPos(lpObj->Map,&lpObj->X,&lpObj->Y);
					}
					break;
				case 2:
					if(lpObj->Guild == 0 || lpObj->Guild->WarType != 1)
					{
						gMap[lpObj->Map].GetMapRandomPos(&lpObj->X,&lpObj->Y,18);
					}
					else
					{
						gBattleGetTeamPosition(lpObj->Guild->BattleGroundIndex,lpObj->Guild->BattleTeamCode,(short&)lpObj->X,(short&)lpObj->Y);
					}
					break;
				case 3:
					gDuel.RespawnDuelUser(lpObj);
					break;
			}

			if(this->CharacterMapServerMove(lpObj->Index,lpObj->Map,lpObj->X,lpObj->Y) != 0)
			{
				continue;
			}

			lpObj->TX = lpObj->X;
			lpObj->TY = lpObj->Y;
			lpObj->PathCur = 0;
			lpObj->PathCount = 0;
			lpObj->PathStartEnd = 0;
			lpObj->DieRegen = 0;
			lpObj->State = OBJECT_CREATE;

			GCCharacterRegenSend(lpObj);

			gObjViewportListProtocolCreate(lpObj);

			this->CharacterUpdateMapEffect(lpObj);
		}
	}

	for(int n=0;n < MAX_MAP;n++)
	{
		gMap[n].StateSetDestroy();
	}
}

void CObjectManager::ObjectStateProc(LPOBJ lpObj,int MessageCode,int aIndex,int SubCode) // OK
{
	if(gObjIsConnected(aIndex) == 0)
	{
		return;
	}

	switch(MessageCode)
	{
		case 2:
			if(lpObj->Live != 0)
			{
				gObjBackSpring(lpObj,&gObj[aIndex]);
			}
			break;
		case 3:
			if(lpObj->Live != 0)
			{
				this->CharacterMonsterDieHunt(lpObj,&gObj[aIndex]);
			}
			break;
		case 4:
			if(lpObj->Guild != 0 && lpObj->Guild->TargetGuildNode != 0)
			{
				gObjGuildWarEnd(lpObj->Guild,lpObj->Guild->TargetGuildNode);
			}
			break;
		case 5:
			if(lpObj->Guild != 0 && lpObj->Guild->WarState != 0 && lpObj->Guild->WarType == 1)
			{
				BattleSoccerGoalStart(0);
			}
			break;
		case 10:
			if(lpObj->Live != 0 && SubCode > 0)
			{
				gAttack.Attack(lpObj,&gObj[aIndex],0,0,0,SubCode,0,0);
			}
			break;
		case 16:
			if((lpObj->Life+SubCode) > (lpObj->MaxLife+lpObj->AddLife))
			{
				lpObj->Life = lpObj->MaxLife+lpObj->AddLife;
			}
			else
			{
				lpObj->Life += SubCode;
			}

			GCLifeSend(lpObj->Index,0xFF,(int)lpObj->Life,lpObj->Shield);
			break;
	}
}

void CObjectManager::ObjectStateAttackProc(LPOBJ lpObj,int MessageCode,int aIndex,int SubCode1,int SubCode2) // OK
{
	if(gObjIsConnected(aIndex) == 0)
	{
		return;
	}

	switch(MessageCode)
	{
		case 50:
			if(lpObj->Live != 0 && (SubCode1 == 0 || gSkillManager.GetSkill(lpObj,SubCode1) != 0))
			{
				gAttack.Attack(lpObj,&gObj[aIndex],gSkillManager.GetSkill(lpObj,SubCode1),0,0,0,0,(bool)(SubCode2&1));
			}
			break;
		case 51:
			if(lpObj->Live != 0)
			{
				gDarkSpirit[lpObj->Index].Attack(lpObj,&gObj[aIndex],0,(bool)(SubCode1&1),0,SubCode2);
			}
			break;
		case 52:
			if(lpObj->Live != 0 && (SubCode1 == 0 || gSkillManager.GetSkill(lpObj,SubCode1) != 0))
			{
				gAttack.Attack(lpObj,&gObj[aIndex],gSkillManager.GetSkill(lpObj,SubCode1),0,0,0,SubCode2,0);
			}
			break;
		case 53:
			if(lpObj->Live != 0 && (SubCode1 == 0 || gSkillManager.GetSkill(lpObj,SubCode1) != 0))
			{
				gAttack.Attack(lpObj,&gObj[aIndex],gSkillManager.GetSkill(lpObj,SubCode1),1,0,0,0,(bool)(SubCode2&1));
			}
			break;
		case 54:
			if(lpObj->Live != 0 && (SubCode1 == 0 || gSkillManager.GetSkill(lpObj,SubCode1) != 0))
			{
				gAttack.Attack(lpObj,&gObj[aIndex],gSkillManager.GetSkill(lpObj,SubCode1),1,0,0,SubCode2,0);
			}
			break;
		case 55:
			if(lpObj->Live != 0 && (SubCode1 == 0 || gSkillManager.GetSkill(lpObj,SubCode1) != 0))
			{
				gSkillManager.SkillGhostPhantomAreaAttack(lpObj->Index,aIndex,gSkillManager.GetSkill(lpObj,SubCode1),(BYTE)SET_NUMBERHW(SubCode2),(BYTE)SET_NUMBERLW(SubCode2));
			}
			break;
	}
}

void CObjectManager::ObjectMoveProc() // OK
{
	for(int n=0;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnected(n) == 0)
		{
			continue;
		}

		LPOBJ lpObj = &gObj[n];

		if(lpObj->State != OBJECT_PLAYING)
		{
			continue;
		}

		if(lpObj->CurrentAI != 0 && (lpObj->Type == OBJECT_MONSTER || lpObj->Type == OBJECT_NPC))
		{
			continue;
		}

		if(lpObj->PathCount == 0 || gEffectManager.CheckStunEffect(lpObj) != 0 || gEffectManager.CheckImmobilizeEffect(lpObj) != 0)
		{
			continue;
		}

		if(lpObj->Type == OBJECT_MONSTER && (KALIMA_ATTRIBUTE_RANGE(lpObj->Attribute) != 0 || lpObj->Class == 131 || lpObj->Class == 132 || lpObj->Class == 133 || lpObj->Class == 134 || lpObj->Class == 204 || lpObj->Class == 205 || lpObj->Class == 206 || lpObj->Class == 207 || lpObj->Class == 208 || lpObj->Class == 209 || lpObj->Class == 216 || lpObj->Class == 217 || lpObj->Class == 218 || lpObj->Class == 219 || lpObj->Class == 277 || lpObj->Class == 278 || lpObj->Class == 283 || lpObj->Class == 288 || lpObj->Class == 524 || lpObj->Class == 525 || lpObj->Class == 527 || lpObj->Class == 528))
		{
			continue;
		}

		DWORD MoveTime = 0;

		if((lpObj->PathDir[lpObj->PathCur]%2) == 0)
		{
			MoveTime = (DWORD)((lpObj->MoveSpeed+((lpObj->DelayLevel==0)?0:300))*(double)1.3);
		}
		else
		{
			MoveTime = (DWORD)((lpObj->MoveSpeed+((lpObj->DelayLevel==0)?0:300))*(double)1.0);
		}

		if((GetTickCount()-lpObj->PathTime) > MoveTime && lpObj->PathCur < (MAX_ROAD_PATH_TABLE-1))
		{
			if(gMap[lpObj->Map].CheckAttr(lpObj->PathX[lpObj->PathCur],lpObj->PathY[lpObj->PathCur],4) != 0 || gMap[lpObj->Map].CheckAttr(lpObj->PathX[lpObj->PathCur],lpObj->PathY[lpObj->PathCur],8) != 0)
			{
				lpObj->PathCur = 0;
				lpObj->PathCount = 0;
				lpObj->PathTime = GetTickCount();
				lpObj->PathStartEnd = ((lpObj->Type==OBJECT_USER)?lpObj->PathStartEnd:0);

				memset(lpObj->PathX,0,sizeof(lpObj->PathX));

				memset(lpObj->PathY,0,sizeof(lpObj->PathY));

				memset(lpObj->PathOri,0,sizeof(lpObj->PathOri));

				gObjSetPosition(lpObj->Index,lpObj->X,lpObj->Y);
			}
			else
			{
				lpObj->X = lpObj->PathX[lpObj->PathCur];
				lpObj->Y = lpObj->PathY[lpObj->PathCur];
				lpObj->Dir = lpObj->PathDir[lpObj->PathCur];
				lpObj->PathTime = GetTickCount();

				if((++lpObj->PathCur) >= lpObj->PathCount)
				{
					lpObj->PathCur = 0;
					lpObj->PathCount = 0;
					lpObj->PathStartEnd = ((lpObj->Type==OBJECT_USER)?lpObj->PathStartEnd:0);
				}
			}
		}
	}
}

void CObjectManager::ObjectMonsterAndMsgProc() // OK
{
	for(int n=0;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnected(n) != 0)
		{
			if(gObj[n].Type == OBJECT_MONSTER || gObj[n].Type == OBJECT_NPC)
			{
				if(gObj[n].CurrentAI == 0)
				{
					gObjMonsterProcess(&gObj[n]);
				}
			}
			else
			{
				gDarkSpirit[n].MainProc();
				gObjSkillNovaCheckTime(&gObj[n]);
				gObjectManager.ObjectMsgProc(&gObj[n]);
				gCustomAttack.OnAttackMonsterAndMsgProc(&gObj[n]);
			}

			for(int i=0;i < MAX_MONSTER_SEND_ATTACK_MSG;i++)
			{
				if(gSMAttackProcMsg[n][i].MsgCode != -1 && GetTickCount() > ((DWORD)gSMAttackProcMsg[n][i].MsgTime))
				{
					gObjectManager.ObjectStateAttackProc(&gObj[n],gSMAttackProcMsg[n][i].MsgCode,gSMAttackProcMsg[n][i].SendUser,gSMAttackProcMsg[n][i].SubCode,gSMAttackProcMsg[n][i].SubCode2);
					gSMAttackProcMsg[n][i].Clear();
				}
			}
		}
	}

	//for(int n=0;n < MAX_OBJECT;n++)
	//{
	//	if(gObjIsConnected(n) != 0)
	//	{
	//		for(int i=0;i < MAX_MONSTER_SEND_ATTACK_MSG;i++)
	//		{
	//			if(gSMAttackProcMsg[n][i].MsgCode != -1 && GetTickCount() > ((DWORD)gSMAttackProcMsg[n][i].MsgTime))
	//			{
	//				gObjectManager.ObjectStateAttackProc(&gObj[n],gSMAttackProcMsg[n][i].MsgCode,gSMAttackProcMsg[n][i].SendUser,gSMAttackProcMsg[n][i].SubCode,gSMAttackProcMsg[n][i].SubCode2);
	//				gSMAttackProcMsg[n][i].Clear();
	//			}
	//		}
	//	}
	//}
}

bool CObjectManager::CharacterGameClose(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Connected != OBJECT_ONLINE)
	{
		return 0;
	}

	if(lpObj->Type != OBJECT_USER || lpObj->MapServerMoveQuit != 0)
	{
		return 0;
	}

	if(OBJECT_RANGE(lpObj->SummonIndex) != 0)
	{
		gObjSummonKill(lpObj->Index);
	}

	if(lpObj->Interface.use != 0 && (lpObj->Interface.type == INTERFACE_CHAOS_BOX || lpObj->Interface.type == INTERFACE_TRAINER))
	{
		if(lpObj->IsChaosMixCompleted != 0 || lpObj->IsCastleNPCUpgradeCompleted != 0)
		{
			gChaosBox.ChaosBoxItemSave(lpObj);
		}
		else
		{
			gObjInventoryRollback(aIndex);
		}
	}

	if(gServerInfo.m_DuelBet != 0)
	{
		if(lpObj->TradeDuel == 1)
		{
			gTrade.TradeDuelEnd(&gObj[lpObj->TargetNumber],lpObj);
		}
	}

	if(OBJECT_RANGE(lpObj->TargetNumber) != 0)
	{
		gTrade.ResetTrade(lpObj->TargetNumber);
		gTrade.GCTradeResultSend(lpObj->TargetNumber,0);
		gTrade.ResetTrade(aIndex);
	}

	if(OBJECT_RANGE(lpObj->DuelUserReserved) != 0)
	{
		gDuel.ResetDuel(&gObj[lpObj->DuelUserReserved]);
		gDuel.GCDuelStartSend(aIndex,0,lpObj->DuelUserReserved);
		gDuel.ResetDuel(lpObj);
	}

	if(OBJECT_RANGE(lpObj->DuelUserRequested) != 0)
	{
		gDuel.ResetDuel(&gObj[lpObj->DuelUserRequested]);
		gDuel.GCDuelStartSend(lpObj->DuelUserRequested,0,aIndex);
		gDuel.ResetDuel(lpObj);
	}

	if(OBJECT_RANGE(lpObj->DuelUser) != 0)
	{
		gDuel.ResetDuel(&gObj[lpObj->DuelUser]);
		gDuel.GCDuelEndSend(lpObj->DuelUser,0);
		gDuel.ResetDuel(lpObj);
		gDuel.GCDuelEndSend(aIndex,0);
	}

	if(OBJECT_RANGE(lpObj->MiningIndex) != 0)
	{
		gObj[lpObj->MiningIndex].MiningStage = 0;
		gObj[lpObj->MiningIndex].MiningIndex = -1;
	}

	gReconnect.SetReconnectInfo(lpObj);

	if(OBJECT_RANGE(lpObj->PartyNumber) != 0)
	{
		if(IT_MAP_RANGE(lpObj->Map) == 0 && DG_MAP_RANGE(lpObj->Map) == 0 && IG_MAP_RANGE(lpObj->Map) == 0)
		{
			if(gParty.GetMemberCount(lpObj->PartyNumber) <= 1)
			{
				gParty.Destroy(lpObj->PartyNumber);
			}
			else
			{
				gParty.DelMember(lpObj->PartyNumber,lpObj->Index);
			}
		}
	}

	if(lpObj->Guild != 0 && lpObj->Guild->WarState == 1)
	{
		gObjGuildWarMasterClose(lpObj);
	}

	if(lpObj->CustomNpcQuest >= 0 && lpObj->CustomNpcQuestMonsterQtd >= 0)
	{
		gCustomNpcQuest.DGCustomNpcQuestMonsterSaveSend(lpObj->Index, lpObj->CustomNpcQuest, lpObj->CustomNpcQuestMonsterQtd);
	}

	//rei do mu
	if(gReiDoMU.m_Active == 1 && lpObj->Map == MAP_DEVIAS)
	{
		lpObj->X = 222;
		lpObj->Y = 62;
	}

	if(BC_MAP_RANGE(lpObj->Map) != 0)
	{
		if(gBloodCastle.GetState(GET_BC_LEVEL(lpObj->Map)) == BC_STATE_START)
		{
			gBloodCastle.SearchUserDropEventItem(aIndex);
		}
		else
		{
			gBloodCastle.SearchUserDeleteEventItem(aIndex);
		}
	}

	if(IT_MAP_RANGE(lpObj->Map) != 0)
	{
		if(gIllusionTemple.GetState(GET_IT_LEVEL(lpObj->Map)) == IT_STATE_START)
		{
			gIllusionTemple.SearchUserDropEventItem(aIndex);
		}
		else
		{
			gIllusionTemple.SearchUserDeleteEventItem(aIndex);
		}
	}

	gObjFixInventoryPointer(aIndex);

	gObjFixEventInventoryPointer(aIndex);

	GDCharacterInfoSaveSend(aIndex);

	#if(GAMESERVER_UPDATE>=801)

	gPartyMatching.GDPartyMatchingJoinCancelSend(aIndex,0,1);

	gPartyMatching.GDPartyMatchingJoinCancelSend(aIndex,1,1);

	#endif

	GDDisconnectCharacterSend(aIndex);

	if(lpObj->Guild == 0)
	{
		GDCharClose(1,0,lpObj->Name);
	}
	else
	{
		GDCharClose(0,lpObj->Guild->Name,lpObj->Name);
	}

	gDarkSpirit[aIndex].SetMode(DARK_SPIRIT_MODE_NORMAL,-1);

	gObjClearViewport(lpObj);

	LogAddConnect(LOG_USER,"[Obj][%d] LogOutCharacter (%s)",lpObj->Index,lpObj->Name);

	memset(lpObj->Name,0,sizeof(lpObj->Name));

	lpObj->Connected = OBJECT_LOGGED;
	return 1;
}

void CObjectManager::CharacterGameCloseSet(int aIndex,int type) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->CloseCount > 0)
	{
		return;
	}

	if(lpObj->Connected == OBJECT_ONLINE)
	{
		if(lpObj->Interface.use != 0 && lpObj->Interface.type == INTERFACE_TRADE)
		{
			gNotice.GCNoticeSend(aIndex,1,0,0,0,0,0,gMessage.GetMessage(261));
			return;
		}

		if(lpObj->Interface.use != 0 && lpObj->Interface.type == INTERFACE_WAREHOUSE)
		{
			gNotice.GCNoticeSend(aIndex,1,0,0,0,0,0,gMessage.GetMessage(262));
			return;
		}

		if(lpObj->Interface.use != 0 && lpObj->Interface.type == INTERFACE_CHAOS_BOX)
		{
			gNotice.GCNoticeSend(aIndex,1,0,0,0,0,0,gMessage.GetMessage(263));
			return;
		}

		if(lpObj->Interface.use != 0 && lpObj->Interface.type == INTERFACE_TRAINER)
		{
			gNotice.GCNoticeSend(aIndex,1,0,0,0,0,0,gMessage.GetMessage(264));
			return;
		}

		if((GetTickCount()-lpObj->MySelfDefenseTime) < 30000)
		{
			gNotice.GCNoticeSend(aIndex,1,0,0,0,0,0,gMessage.GetMessage(265));
			return;
		}
	}

	lpObj->CloseCount = 1;

	lpObj->CloseType = type;

	lpObj->EnableDelCharacter = 1;
}

bool CObjectManager::CharacterMapServerMove(int aIndex,int map,int x,int y) // OK
{
	if((gObj[aIndex].NextServerCode=gMapServerManager.CheckMapServerMove(aIndex,map,gObj[aIndex].LastServerCode)) != -1)
	{
		if(gObj[aIndex].NextServerCode != gServerInfo.m_ServerCode)
		{
			GJMapServerMoveSend(aIndex,gObj[aIndex].NextServerCode,map,x,y);
			return 1;
		}
	}

	return 0;
}

void CObjectManager::CharacterUpdateMapEffect(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_TYPE==1)

	if(lpObj->Map == MAP_CASTLE_SIEGE)
	{
		GCAnsCsNotifyStart(lpObj->Index,((gCastleSiege.GetCastleState()==CASTLESIEGE_STATE_STARTSIEGE)?1:0));

		if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)
		{
			gCastleSiege.NotifySelfCsJoinSide(lpObj->Index);
			gCastleSiege.NotifyCsSelfLeftTime(lpObj->Index);
		}
	}

	#endif

	if(lpObj->Map == MAP_RAKLION1 || lpObj->Map == MAP_RAKLION2)
	{
		gRaklionUtil.NotifyRaklionState(lpObj->Index,gRaklion.GetRaklionState(),gRaklion.GetRaklionDetailState());
	}
}

bool CObjectManager::CharacterGetRespawnLocation(LPOBJ lpObj) // OK
{
	bool result = 0;

	int gate,map,x,y,dir,level;

	if(lpObj->KillAll != 0)
	{
		result = gEventKillAll.Respawn(lpObj,&gate,&map,&x,&y,&dir,&level);
	}
	else if(lpObj->PvP != 0)
	{
		gate	= 0;
		map		= lpObj->Map;
		x		= lpObj->X;
		y		= lpObj->Y;
		dir		= 3;
		level	= 0;
		result	= 1;
	}
	else if(gTvTEvent.CheckPlayerTarget(lpObj))
	{
		result = gTvTEvent.GetUserRespawnLocation(lpObj,&gate,&map,&x,&y,&dir,&level);
	}
	else if(gGvGEvent.CheckPlayerTarget(lpObj))
	{
		result = gGvGEvent.GetUserRespawnLocation(lpObj,&gate,&map,&x,&y,&dir,&level);
	}
	else if(CA_MAP_RANGE(lpObj->Map) != 0)
	{
		result = gCustomArena.GetUserRespawnLocation(lpObj,&gate,&map,&x,&y,&dir,&level);
	}
	else if(lpObj->Map == MAP_CASTLE_SIEGE)
	{
		#if(GAMESERVER_TYPE==0)

		result = gGate.GetGate(100,&gate,&map,&x,&y,&dir,&level);

		#else

		if(gLifeStone.SetReSpawnUserXY(lpObj->Index) == 0)
		{
			if(lpObj->CsJoinSide == 1)
			{
				result = gGate.GetGate(101,&gate,&map,&x,&y,&dir,&level);
			}
			else
			{
				result = gGate.GetGate(100,&gate,&map,&x,&y,&dir,&level);
			}
		}

		#endif
	}
	else if(lpObj->Map == MAP_CRYWOLF)
	{
		#if(GAMESERVER_TYPE==1)

		if(gCrywolf.GetCrywolfState() == CRYWOLF_STATE_START)
		{
			result = gGate.GetGate(118,&gate,&map,&x,&y,&dir,&level);
		}

		#endif
	}
	else if(IT_MAP_RANGE(lpObj->Map) != 0)
	{
		result = gIllusionTemple.GetUserRespawnLocation(lpObj,&gate,&map,&x,&y,&dir,&level);
	}
	//else if(lpObj->Map == MAP_LORENCIA)
	//{
	//	result = gGate.GetGate(17,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_DUNGEON)
	//{
	//	result = gGate.GetGate(17,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_DEVIAS)
	//{
	//	result = gGate.GetGate(22,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_NORIA)
	//{
	//	result = gGate.GetGate(27,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_LOST_TOWER)
	//{
	//	result = gGate.GetGate(42,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_ARENA)
	//{
	//	result = gGate.GetGate(115,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_ATLANS)
	//{
	//	result = gGate.GetGate(49,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_TARKAN)
	//{
	//	result = gGate.GetGate(57,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(DS_MAP_RANGE(lpObj->Map) != 0)
	//{
	//	result = gGate.GetGate(27,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_ICARUS)
	//{
	//	result = gGate.GetGate(22,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(BC_MAP_RANGE(lpObj->Map) != 0)
	//{
	//	result = gGate.GetGate(22,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(CC_MAP_RANGE(lpObj->Map) != 0)
	//{
	//	result = gGate.GetGate(17,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(KALIMA_MAP_RANGE(lpObj->Map) != 0)
	//{
	//	result = gGate.GetGate(17,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_LAND_OF_TRIALS)
	//{
	//	result = gGate.GetGate(106,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_AIDA)
	//{
	//	result = gGate.GetGate(119,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_RESERVED2)
	//{
	//	result = gGate.GetGate(401,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_KANTURU1)
	//{
	//	result = gGate.GetGate(138,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_KANTURU2)
	//{
	//	result = gGate.GetGate(139,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_KANTURU3)
	//{
	//	result = gGate.GetGate(137,&gate,&map,&x,&y,&dir,&level);

	//	gKanturuBattleUserMng.DelUserData(lpObj->Index);
	//}
	//else if(lpObj->Map == MAP_SILENT)
	//{
	//	result = gGate.GetGate(402,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_BARRACKS)
	//{
	//	result = gGate.GetGate(256,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_REFUGE)
	//{
	//	result = gGate.GetGate(256,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_ELBELAND)
	//{
	//	result = gGate.GetGate(267,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_SWAMP_OF_CALMNESS)
	//{
	//	result = gGate.GetGate(273,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_RAKLION1)
	//{
	//	result = gGate.GetGate(286,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_RAKLION2)
	//{
	//	result = gGate.GetGate(286,&gate,&map,&x,&y,&dir,&level);

	//	gRaklionBattleUserMng.DelUserData(lpObj->Index);
	//}
	//else if(lpObj->Map == MAP_SANTA_TOWN)
	//{
	//	result = gGate.GetGate(22,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_VULCANUS)
	//{
	//	result = gGate.GetGate(294,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(DA_MAP_RANGE(lpObj->Map) != 0)
	//{
	//	result = gGate.GetGate(294,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(DG_MAP_RANGE(lpObj->Map) != 0)
	//{
	//	result = gGate.GetGate(267,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(IG_MAP_RANGE(lpObj->Map) != 0)
	//{
	//	result = gGate.GetGate(22,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_LOREN_MARKET)
	//{
	//	result = gGate.GetGate(333,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_KARUTAN1)
	//{
	//	result = gGate.GetGate(335,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_KARUTAN2)
	//{
	//	result = gGate.GetGate(335,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_ARKANIA)
	//{
	//	result = gGate.GetGate(500,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_NEWARENA)
	//{
	//	result = gGate.GetGate(501,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_ABUSS)
	//{
	//	result = gGate.GetGate(502,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_DUNES)
	//{
	//	result = gGate.GetGate(503,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_ACHERON)
	//{
	//	result = gGate.GetGate(504,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_KALIMDOR)
	//{
	//	result = gGate.GetGate(505,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_NEW1)
	//{
	//	result = gGate.GetGate(506,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_NEW2)
	//{
	//	result = gGate.GetGate(507,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_NEW3)
	//{
	//	result = gGate.GetGate(508,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_ACHERON1)
	//{
	//	result = gGate.GetGate(417,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_ACHERON2)
	//{
	//	result = gGate.GetGate(426,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_DEVENTER1)
	//{
	//	result = gGate.GetGate(417,&gate,&map,&x,&y,&dir,&level);
	//}
	//else if(lpObj->Map == MAP_DEVENTER2)
	//{
	//	result = gGate.GetGate(426,&gate,&map,&x,&y,&dir,&level);
	//}
	else
	{
		result = gGate.GetGate(gMapManager.GetMapDeathGate(lpObj->Map),&gate,&map,&x,&y,&dir,&level);

		if(lpObj->Map == MAP_KANTURU3)
		{
			gKanturuBattleUserMng.DelUserData(lpObj->Index);
		}

		if(lpObj->Map == MAP_RAKLION2)
		{
			gRaklionBattleUserMng.DelUserData(lpObj->Index);
		}
	}

	if(result != 0)
	{
		lpObj->Map = map;
		lpObj->X = x;
		lpObj->Y = y;
		lpObj->Dir = dir;
	}

	return result;
}

void CObjectManager::CharacterCalcExperienceSplit(LPOBJ lpObj,LPOBJ lpMonster,int damage,int flag) // OK
{
	if(lpMonster->Type != OBJECT_MONSTER)
	{
		return;
	}

	gObjMonsterDelHitDamageUser(lpMonster);

	lpMonster->Money = 0;

	for(int n=0;n < MAX_HIT_DAMAGE;n++)
	{
		if(OBJECT_RANGE(lpMonster->HitDamage[n].index) == 0)
		{
			continue;
		}

		if(lpMonster->HitDamage[n].damage == 0)
		{
			continue;
		}

		if(lpMonster->HitDamage[n].index == lpObj->Index)
		{
			this->CharacterCalcExperienceAlone(&gObj[lpMonster->HitDamage[n].index],lpMonster,lpMonster->HitDamage[n].damage,flag,0,damage);
		}
		else
		{
			this->CharacterCalcExperienceAlone(&gObj[lpMonster->HitDamage[n].index],lpMonster,lpMonster->HitDamage[n].damage,flag,1,damage);
		}
	}
}

void CObjectManager::CharacterCalcExperienceAlone(LPOBJ lpObj,LPOBJ lpMonster,int damage,int flag,bool type,int AttackDamage) // OK
{
	if(lpMonster->Type != OBJECT_MONSTER)
	{
		return;
	}

	int level = ((lpMonster->Level+25)*lpMonster->Level)/3;

	if(gMasterSkillTree.CheckMasterLevel(lpObj) == 0)
	{
		level = (((lpMonster->Level+10)<lpObj->Level)?((level*(lpMonster->Level+10))/lpObj->Level):level);
	}
	else
	{
		level = (((lpMonster->Level+10)<(lpObj->Level+lpObj->MasterLevel))?((level*(lpMonster->Level+10))/(lpObj->Level+lpObj->MasterLevel)):level);
	}

	if(lpMonster->Level >= 65)
	{
		level += (lpMonster->Level-64)*(lpMonster->Level/4);
	}

	level = ((level<0)?0:level);

	damage = ((damage>lpMonster->MaxLife)?(int)lpMonster->MaxLife:damage);

	QWORD experience = level+(level/4);

	if(gMasterSkillTree.CheckMasterLevel(lpObj) == 0)
	{
			int expserver = 0;

			if (lpObj->AttackCustomOffline == 1) 
			{
				expserver = gServerInfo.m_OffAttackExperienceRate[lpObj->AccountLevel];
			}
			else if (lpObj->AutoResetEnable == 1) 
			{
				expserver = gServerInfo.m_AutoResetExperienceRate[lpObj->AccountLevel];
			}
			else 
			{
				expserver = gServerInfo.m_AddExperienceRate[lpObj->AccountLevel];
			}

		experience = ((damage*experience)/(int)lpMonster->MaxLife)*expserver;

		experience = (experience*(lpObj->ExperienceRate+lpObj->EffectOption.AddExperienceRate))/100;

		experience = (experience*(gMapManager.GetMapExperienceRate(lpObj->Map)+((lpObj->Map==MAP_VULCANUS)?lpObj->EffectOption.AddVolcanoExperienceRate:0)))/100;

		experience = (experience*gBonusManager.GetBonusValue(lpObj,BONUS_INDEX_EXPERIENCE_RATE,100,-1,-1,lpMonster->Class,lpMonster->Level))/100;

		experience = (experience*gCustomMonster.GetCustomMonsterExperienceRate(lpMonster->Class,lpMonster->Map))/100;

		experience = (experience*gExperienceTable.GetExperienceRate(lpObj))/100;
	}
	else
	{
		experience = ((damage*experience)/(int)lpMonster->MaxLife)*gServerInfo.m_AddMasterExperienceRate[lpObj->AccountLevel];

		experience = (experience*(lpObj->MasterExperienceRate+lpObj->EffectOption.AddMasterExperienceRate))/100;

		experience = (experience*(gMapManager.GetMapMasterExperienceRate(lpObj->Map)+((lpObj->Map==MAP_VULCANUS)?lpObj->EffectOption.AddVolcanoExperienceRate:0)))/100;

		experience = (experience*gBonusManager.GetBonusValue(lpObj,BONUS_INDEX_MASTER_EXPERIENCE_RATE,100,-1,-1,lpMonster->Class,lpMonster->Level))/100;

		experience = (experience*gCustomMonster.GetCustomMonsterMasterExperienceRate(lpMonster->Class,lpMonster->Map))/100;

		experience = (experience*gExperienceTable.GetMasterExperienceRate(lpObj))/100;
	}

	if(gCrywolfSync.CheckApplyPenalty() != 0)
	{
		if(gCrywolfSync.GetOccupationState() == 1)
		{
			experience = (experience*gCrywolfSync.GetGettingExpPenaltyRate())/100;
		}
	}

	lpMonster->Money += (DWORD)experience;

	this->CharacterPetLevelUp(lpObj,(DWORD)experience);

	if(gMasterSkillTree.CheckMasterLevel(lpObj) != 0 && lpMonster->Level < gServerInfo.m_MinMasterExperienceMonsterLevel[lpObj->AccountLevel])
	{
		if (gServerInfo.m_DisableExpMessage == 0) 
		{ 
			GCMonsterDieSend(lpObj->Index,lpMonster->Index,0,((type==0)?AttackDamage:0),flag);
		}
		return;
	}

	if(gEffectManager.CheckEffect(lpObj,EFFECT_SEAL_OF_SUSTENANCE1) != 0 || gEffectManager.CheckEffect(lpObj,EFFECT_SEAL_OF_SUSTENANCE2) != 0)
	{
		if (gServerInfo.m_DisableExpMessage == 0) 
		{ 
			GCMonsterDieSend(lpObj->Index,lpMonster->Index,0,((type==0)?AttackDamage:0),flag);
		}
		return;
	}

	if(this->CharacterLevelUp(lpObj,(DWORD)experience,gServerInfo.m_MaxLevelUp,EXPERIENCE_COMMON) == 0)
	{
		if (gServerInfo.m_DisableExpMessage == 0) 
		{ 
			GCMonsterDieSend(lpObj->Index,lpMonster->Index,(DWORD)experience,((type==0)?AttackDamage:0),flag);
		}
	}
	else
	{
		if (gServerInfo.m_DisableExpMessage == 0) 
		{ 
			GCMonsterDieSend(lpObj->Index,lpMonster->Index,0,((type==0)?AttackDamage:0),flag);
		}
	}
}

void CObjectManager::CharacterCalcExperienceParty(LPOBJ lpObj,LPOBJ lpMonster,int damage,int flag) // OK
{
	if(OBJECT_RANGE(lpObj->PartyNumber) == 0)
	{
		return;
	}

	if(lpMonster->Type != OBJECT_MONSTER)
	{
		return;
	}

	PARTY_INFO* lpParty = &gParty.m_PartyInfo[lpObj->PartyNumber];

	int PartyTopLevel = 0;

	for(int n=0;n < MAX_PARTY_USER;n++)
	{
		if(OBJECT_RANGE(lpParty->Index[n]) != 0)
		{
			if(lpMonster->Map == gObj[lpParty->Index[n]].Map && gObjCalcDistance(lpMonster,&gObj[lpParty->Index[n]]) < MAX_PARTY_DISTANCE)
			{
				if(gMasterSkillTree.CheckMasterLevel(&gObj[lpParty->Index[n]]) == 0)
				{
					PartyTopLevel = ((gObj[lpParty->Index[n]].Level>PartyTopLevel)?gObj[lpParty->Index[n]].Level:PartyTopLevel);
				}
				else
				{
					PartyTopLevel = (((gObj[lpParty->Index[n]].Level+gObj[lpParty->Index[n]].MasterLevel)>PartyTopLevel)?(gObj[lpParty->Index[n]].Level+gObj[lpParty->Index[n]].MasterLevel):PartyTopLevel);
				}
			}
		}
	}

	int TotalLevel = 0;
	int PartyLevel = 0;
	int PartyTable = 0;
	int PartyClass = 0;
	int PartyCount = 0;

	for(int n=0;n < MAX_PARTY_USER;n++)
	{
		if(OBJECT_RANGE(lpParty->Index[n]) != 0)
		{
			if(gObj[lpParty->Index[n]].Map == lpMonster->Map && gObjCalcDistance(lpMonster,&gObj[lpParty->Index[n]]) < MAX_PARTY_DISTANCE)
			{
				if(gMasterSkillTree.CheckMasterLevel(&gObj[lpParty->Index[n]]) == 0)
				{
					TotalLevel += (((gObj[lpParty->Index[n]].Level+200)<PartyTopLevel)?(gObj[lpParty->Index[n]].Level+200):gObj[lpParty->Index[n]].Level);
					PartyTable += (((PartyTable&(1<<gObj[lpParty->Index[n]].Class))==0)?(1<<gObj[lpParty->Index[n]].Class):((++PartyClass)*0));
					PartyCount++;
				}
				else
				{
					TotalLevel += ((((gObj[lpParty->Index[n]].Level+gObj[lpParty->Index[n]].MasterLevel)+200)<PartyTopLevel)?((gObj[lpParty->Index[n]].Level+gObj[lpParty->Index[n]].MasterLevel)+200):(gObj[lpParty->Index[n]].Level+gObj[lpParty->Index[n]].MasterLevel));
					PartyTable += (((PartyTable&(1<<gObj[lpParty->Index[n]].Class))==0)?(1<<gObj[lpParty->Index[n]].Class):((++PartyClass)*0));
					PartyCount++;
				}
			}
		}
	}

	if(PartyCount == 0){return;}

	int ExperienceRate = (((PartyClass>=3)?gServerInfo.m_PartySpecialExperience[(PartyCount-1)]:gServerInfo.m_PartyGeneralExperience[(PartyCount-1)])*PartyCount);

	PartyLevel = TotalLevel/PartyCount;

	int level = ((lpMonster->Level+25)*lpMonster->Level)/3;

	if((lpMonster->Level+10) < PartyLevel)
	{
		level = (level*(lpMonster->Level+10))/PartyLevel;
	}

	if(lpMonster->Level >= 65)
	{
		if(PartyCount == 1)
		{
			level += (lpMonster->Level-64)*(lpMonster->Level/4);
		}
		else
		{
			if(gMasterSkillTree.CheckMasterLevel(lpObj) == 0)
			{
				level += (200-(lpObj->Level/5));
			}
			else
			{
				level += (200-((lpObj->Level+lpObj->MasterLevel)/5));
			}
		}
	}

	level = ((level<0)?0:level);

	DWORD TotalExperience = level+(level/4);

	lpMonster->Money = 0;

	for(int n=0;n < MAX_PARTY_USER;n++)
	{
		if(OBJECT_RANGE(lpParty->Index[n]) == 0)
		{
			continue;
		}

		LPOBJ lpTarget = &gObj[lpParty->Index[n]];

		if(lpTarget->Map != lpMonster->Map || gObjCalcDistance(lpMonster,lpTarget) >= MAX_PARTY_DISTANCE)
		{
			continue;
		}

		QWORD experience = 0;

		if(gMasterSkillTree.CheckMasterLevel(lpTarget) == 0)
		{
			int expserver = 0;

			if (lpTarget->AttackCustomOffline == 1) 
			{
				expserver = gServerInfo.m_OffAttackExperienceRate[lpTarget->AccountLevel];
			}
			else if (lpTarget->AttackCustomOffline == 0 && lpTarget->AutoResetEnable == 1) 
			{
				expserver = gServerInfo.m_AutoResetExperienceRate[lpTarget->AccountLevel];
			}
			else 
			{
				expserver = gServerInfo.m_AddExperienceRate[lpTarget->AccountLevel];
			}

			experience = ((((TotalExperience*ExperienceRate)*lpTarget->Level)/TotalLevel)/100)*expserver;

			experience = (experience*(lpTarget->ExperienceRate+lpTarget->EffectOption.AddExperienceRate+(lpTarget->EffectOption.AddPartyBonusExperienceRate*(PartyCount-1))))/100;

			experience = (experience*(gMapManager.GetMapExperienceRate(lpTarget->Map)+((lpTarget->Map==MAP_VULCANUS)?lpTarget->EffectOption.AddVolcanoExperienceRate:0)))/100;

			experience = (experience*gBonusManager.GetBonusValue(lpTarget,BONUS_INDEX_EXPERIENCE_RATE,100,-1,-1,lpMonster->Class,lpMonster->Level))/100;

			experience = (experience*gCustomMonster.GetCustomMonsterExperienceRate(lpMonster->Class,lpMonster->Map))/100;

			experience = (experience*gExperienceTable.GetExperienceRate(lpTarget))/100;
		}
		else
		{
			experience = ((((TotalExperience*ExperienceRate)*(lpTarget->Level+lpTarget->MasterLevel))/TotalLevel)/100)*gServerInfo.m_AddMasterExperienceRate[lpTarget->AccountLevel];

			experience = (experience*(lpTarget->MasterExperienceRate+lpTarget->EffectOption.AddMasterExperienceRate+(lpTarget->EffectOption.AddPartyBonusExperienceRate*(PartyCount-1))))/100;

			experience = (experience*(gMapManager.GetMapMasterExperienceRate(lpTarget->Map)+((lpTarget->Map==MAP_VULCANUS)?lpTarget->EffectOption.AddVolcanoExperienceRate:0)))/100;

			experience = (experience*gBonusManager.GetBonusValue(lpTarget,BONUS_INDEX_MASTER_EXPERIENCE_RATE,100,-1,-1,lpMonster->Class,lpMonster->Level))/100;

			experience = (experience*gCustomMonster.GetCustomMonsterMasterExperienceRate(lpMonster->Class,lpMonster->Map))/100;

			experience = (experience*gExperienceTable.GetMasterExperienceRate(lpTarget))/100;
		}

		if(gCrywolfSync.CheckApplyPenalty() != 0)
		{
			if(gCrywolfSync.GetOccupationState() == 1)
			{
				experience = (experience*gCrywolfSync.GetGettingExpPenaltyRate())/100;
			}
		}

		lpMonster->Money += (DWORD)(experience/PartyCount);

		this->CharacterPetLevelUp(lpTarget,(DWORD)experience);

		if(gMasterSkillTree.CheckMasterLevel(lpTarget) != 0 && lpMonster->Level < gServerInfo.m_MinMasterExperienceMonsterLevel[lpTarget->AccountLevel])
		{
			if (gServerInfo.m_DisableExpMessage == 0) 
			{ 
				GCMonsterDieSend(lpTarget->Index,lpMonster->Index,0,damage,flag);
			}
			continue;
		}

		if(gEffectManager.CheckEffect(lpTarget,EFFECT_SEAL_OF_SUSTENANCE1) != 0 || gEffectManager.CheckEffect(lpTarget,EFFECT_SEAL_OF_SUSTENANCE2) != 0)
		{
			if (gServerInfo.m_DisableExpMessage == 0) 
			{ 
				GCMonsterDieSend(lpTarget->Index,lpMonster->Index,0,damage,flag);
			}
			continue;
		}

		if(this->CharacterLevelUp(lpTarget,(DWORD)experience,gServerInfo.m_MaxLevelUp,EXPERIENCE_PARTY) == 0)
		{
			if (gServerInfo.m_DisableExpMessage == 0) 
			{ 
				GCMonsterDieSend(lpTarget->Index,lpMonster->Index,(DWORD)experience,damage,flag);
			}
		}
		else
		{
			if (gServerInfo.m_DisableExpMessage == 0) 
			{ 
				GCMonsterDieSend(lpTarget->Index,lpMonster->Index,0,damage,flag);
			}
		}
	}
}

bool CObjectManager::CharacterLevelUp(LPOBJ lpObj,DWORD AddExperience,int MaxLevelUp,int ExperienceType) // OK
{
	if(gMasterSkillTree.CheckMasterLevel(lpObj) == 0)
	{
		if(lpObj->Level >= MAX_CHARACTER_LEVEL)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(266));
			return 1;
		}

		if((lpObj->Experience+AddExperience) < lpObj->NextExperience)
		{
			lpObj->Experience += AddExperience;
			return 0;
		}

		while(true)
		{
			lpObj->Level++;

			lpObj->LevelUpPoint += gServerInfo.m_LevelUpPoint[lpObj->Class];

			lpObj->LevelUpPoint += ((lpObj->Level>220)?((gQuest.CheckQuestListState(lpObj,2,QUEST_FINISH)==0)?0:gServerInfo.m_PlusStatPoint):0);

			AddExperience -= (((--MaxLevelUp)==0)?AddExperience:(lpObj->NextExperience-lpObj->Experience));

			lpObj->Experience = lpObj->NextExperience;

			gObjCalcExperience(lpObj);

			if (gServerInfo.m_CustomRankUserType == 2)
			{
				gCustomRankUser.CheckUpdate(lpObj);
			}

			if(lpObj->Level >= MAX_CHARACTER_LEVEL)
			{
				AddExperience = 0;
				break;
			}

			if((lpObj->Experience+AddExperience) < lpObj->NextExperience)
			{
				lpObj->Experience += AddExperience;
				break;
			}
		}

		this->CharacterCalcAttribute(lpObj->Index);

		lpObj->Life = lpObj->MaxLife+lpObj->AddLife;

		lpObj->Mana = lpObj->MaxMana+lpObj->AddMana;

		lpObj->BP = lpObj->MaxBP+lpObj->AddBP;

		lpObj->Shield = lpObj->MaxShield+lpObj->AddShield;

		gCustomRankUser.GCReqRankLevelUser(lpObj->Index, lpObj->Index);

		GCLevelUpSend(lpObj);
	}
	else
	{
		if(lpObj->MasterLevel >= gServerInfo.m_MasterSkillTreeMaxLevel)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(267));
			return 1;
		}

		if((lpObj->MasterExperience+AddExperience) < lpObj->MasterNextExperience)
		{
			lpObj->MasterExperience += AddExperience;
			return 0;
		}

		while(true)
		{
			lpObj->MasterLevel++;

			lpObj->MasterPoint += gServerInfo.m_MasterSkillTreePoint;

			AddExperience -= (((--MaxLevelUp)==0)?AddExperience:(DWORD)(lpObj->MasterNextExperience-lpObj->MasterExperience));

			lpObj->MasterExperience = lpObj->MasterNextExperience;

			gObjCalcExperience(lpObj);

			if (gServerInfo.m_CustomRankUserType == 2)
			{
				gCustomRankUser.CheckUpdate(lpObj);
			}

			if(lpObj->MasterLevel >= gServerInfo.m_MasterSkillTreeMaxLevel)
			{
				AddExperience = 0;
				break;
			}

			if((lpObj->MasterExperience+AddExperience) < lpObj->MasterNextExperience)
			{
				lpObj->MasterExperience += AddExperience;
				break;
			}
		}

		this->CharacterCalcAttribute(lpObj->Index);

		lpObj->Life = lpObj->MaxLife+lpObj->AddLife;

		lpObj->Mana = lpObj->MaxMana+lpObj->AddMana;

		lpObj->BP = lpObj->MaxBP+lpObj->AddBP;

		lpObj->Shield = lpObj->MaxShield+lpObj->AddShield;

		gMasterSkillTree.GCMasterLevelUpSend(lpObj);

		gCustomRankUser.GCReqRankLevelUser(lpObj->Index, lpObj->Index);
	}

	if((GetTickCount()-lpObj->CharSaveTime) > 60000)
	{
		lpObj->CharSaveTime = GetTickCount();
		GDCharacterInfoSaveSend(lpObj->Index);
	}

	return 1;
}

bool CObjectManager::CharacterLevelUpPointAdd(LPOBJ lpObj,int type,int amount) // OK
{
	if(lpObj->Type != OBJECT_USER)
	{
		return 0;
	}

	if(lpObj->LevelUpPoint < amount)
	{
		return 0;
	}

	int* stat;

	switch(type)
	{
		case 0:
			stat = &lpObj->Strength;
			break;
		case 1:
			stat = &lpObj->Dexterity;
			break;
		case 2:
			stat = &lpObj->Vitality;
			break;
		case 3:
			stat = &lpObj->Energy;
			break;
		case 4:
			stat = &lpObj->Leadership;
			break;
		default:
			return 0;
	}

	if(((*stat)+amount) > gServerInfo.m_MaxStatPoint[lpObj->AccountLevel])
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(268),gServerInfo.m_MaxStatPoint[lpObj->AccountLevel]);
		return 0;
	}

	(*stat) += amount;

	lpObj->LevelUpPoint -= amount;

	this->CharacterCalcAttribute(lpObj->Index);
	return 1;
}

void CObjectManager::CharacterPetLevelUp(LPOBJ lpObj,DWORD AddExperience) // OK
{
	if(lpObj->Inventory[8].m_Index == GET_ITEM(13,4)) // Dark Horse
	{
		if(lpObj->Inventory[8].AddPetItemExp((AddExperience/10)) == 0)
		{
			GCPetItemInfoSend(lpObj->Index,1,0,8,lpObj->Inventory[8].m_PetItemLevel,lpObj->Inventory[8].m_PetItemExp,(BYTE)lpObj->Inventory[8].m_Durability);
		}
		else
		{
			if(gItemManager.IsValidItem(lpObj,&lpObj->Inventory[8]) == 0)
			{
				if(lpObj->Inventory[8].DecPetItemExp((AddExperience/10)) != 0)
				{
					GCPetItemInfoSend(lpObj->Index,1,0,8,lpObj->Inventory[8].m_PetItemLevel,lpObj->Inventory[8].m_PetItemExp,(BYTE)lpObj->Inventory[8].m_Durability);
				}
			}
			else
			{
				GCPetItemInfoSend(lpObj->Index,1,0xFE,8,lpObj->Inventory[8].m_PetItemLevel,lpObj->Inventory[8].m_PetItemExp,(BYTE)lpObj->Inventory[8].m_Durability);
			}
		}
	}

	if(lpObj->Inventory[1].m_Index == GET_ITEM(13,5)) // Dark Reaven
	{
		if(lpObj->Inventory[1].AddPetItemExp((AddExperience/10)) == 0)
		{
			GCPetItemInfoSend(lpObj->Index,0,0,1,lpObj->Inventory[1].m_PetItemLevel,lpObj->Inventory[1].m_PetItemExp,(BYTE)lpObj->Inventory[1].m_Durability);
		}
		else
		{
			if(gItemManager.IsValidItem(lpObj,&lpObj->Inventory[1]) == 0)
			{
				if(lpObj->Inventory[1].DecPetItemExp((AddExperience/10)) != 0)
				{
					GCPetItemInfoSend(lpObj->Index,0,0,1,lpObj->Inventory[1].m_PetItemLevel,lpObj->Inventory[1].m_PetItemExp,(BYTE)lpObj->Inventory[1].m_Durability);
				}
			}
			else
			{
				GCPetItemInfoSend(lpObj->Index,0,0xFE,1,lpObj->Inventory[1].m_PetItemLevel,lpObj->Inventory[1].m_PetItemExp,(BYTE)lpObj->Inventory[1].m_Durability);
			}
		}
	}
}

//MC
char CObjectManager::gObjFixClassPacket(int DbClass)    //OK
	{
	 char Preview = 0;
	
	    switch(DbClass)
	    {
	    case DB_DARK_WIZARD:
	        Preview = (char)PR_DARK_WIZARD;
	        break;
	
	    case DB_SOUL_MASTER:
	        Preview = (char)PR_SOUL_MASTER;
	        break;
	
	    case DB_GRAND_MASTER:
	        Preview = (char)PR_GRAND_MASTER;
	        break;
	
	    case DB_DARK_KNIGHT:
	        Preview = (char)PR_DARK_KNIGHT;
	        break;
	
	    case DB_BLADE_KNIGHT:
	        Preview = (char)PR_BLADE_KNIGHT;
	        break;
	
	    case DB_BLADE_MASTER:
	        Preview = (char)PR_BLADE_MASTER;
	        break;
	
	    case DB_FAIRY_ELF:
	        Preview = (char)PR_FAIRY_ELF;
	        break;
	
	    case DB_MUSE_ELF:
	        Preview = (char)PR_MUSE_ELF;
	        break;
	
	    case DB_HIGH_ELF:
	        Preview = (char)PR_HIGH_ELF;
	        break;
	
	    case DB_MAGIC_GLADIATOR:
	        Preview = (char)PR_MAGIC_GLADIATOR;
	        break;
	
	    case DB_DUEL_MASTER:
	        Preview = (char)PR_DUEL_MASTER;
	        break;
	
	    case DB_DUEL_MASTER+1:
	        Preview = (char)PR_DUEL_MASTER;
	        break;
	        
	    case DB_DARK_LORD:
	        Preview = (char)PR_DARK_LORD;
	        break;
	
	    case DB_LORD_EMPEROR:
	        Preview = (char)PR_LORD_EMPEROR;
	        break;
	
	    case DB_LORD_EMPEROR+1:
	        Preview = (char)PR_LORD_EMPEROR;
	        break;
	
	    case DB_SUMMONER:
	        Preview = (char)PR_SUMMONER;
	        break;
	
	    case DB_BLOODY_SUMMONER:
	        Preview = (char)PR_BLOODY_SUMMONER;
	        break;
	    case DB_DIMENSION_MASTER:
	        Preview = (char)PR_DIMENSION_MASTER;
	        break;
	
	    case DB_RAGEFIGHER:
	        Preview = (char)PR_RAGEFIGHER;
	        break;
	
	    case DB_RAGEFIGHER_EVO:
	    case DB_RAGEFIGHER_EVO+1:
	        Preview = (char)PR_RAGEFIGHER_EVO;
	        break;
	    }
	
	    return Preview;
	}

//mc

void CObjectManager::CharacterMakePreviewCharSet(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	memset(lpObj->CharSet,0,sizeof(lpObj->CharSet));

	lpObj->CharSet[0] = (lpObj->ChangeUp*16);
	lpObj->CharSet[0] -= (lpObj->CharSet[0]/32);
	lpObj->CharSet[0] += (lpObj->Class*32);

	lpObj->CharSet[0] = gObjFixClassPacket(lpObj->DBClass);//Mc
    lpObj->CharSet[0] |= CS_SET_CHANGEUP(lpObj->ChangeUp);//MC

	if(lpObj->ActionNumber == ACTION_SIT1)
	{
		lpObj->CharSet[0] |= 2;
	}
	
	if(lpObj->ActionNumber == ACTION_POSE1)
	{
		lpObj->CharSet[0] |= 3;
	}

	WORD TempInventory[INVENTORY_WEAR_SIZE];

	for(int n=0;n < INVENTORY_WEAR_SIZE;n++)
	{
		if(n == 0 || n == 1)
		{
			if(lpObj->Inventory[n].IsItem() == 0)
			{
				TempInventory[n] = 0xFFFF;
			}
			else
			{
				TempInventory[n] = lpObj->Inventory[n].m_Index;
			}
		}
		else
		{
			if(lpObj->Inventory[n].IsItem() == 0)
			{
				TempInventory[n] = 0x1FF;
			}
			else
			{
				TempInventory[n] = lpObj->Inventory[n].m_Index%MAX_ITEM_TYPE;
			}
		}
	}

	lpObj->CharSet[1] = TempInventory[0]%256;
	lpObj->CharSet[12] |= (TempInventory[0]/16) & 0xF0;

	lpObj->CharSet[2] = TempInventory[1]%256;
	lpObj->CharSet[13] |= (TempInventory[1]/16) & 0xF0;

	lpObj->CharSet[3] |= (TempInventory[2] & 0x0F) << 4;
	lpObj->CharSet[9] |= (TempInventory[2] & 0x10) << 3;
	lpObj->CharSet[13] |= (TempInventory[2] & 0x1E0) >> 5;

	lpObj->CharSet[3] |= (TempInventory[3] & 0x0F);
	lpObj->CharSet[9] |= (TempInventory[3] & 0x10) << 2;
	lpObj->CharSet[14] |= (TempInventory[3] & 0x1E0) >> 1;

	lpObj->CharSet[4] |= (TempInventory[4] & 0x0F) << 4;
	lpObj->CharSet[9] |= (TempInventory[4] & 0x10) << 1;
	lpObj->CharSet[14] |= (TempInventory[4] & 0x1E0) >> 5;

	lpObj->CharSet[4] |= (TempInventory[5] & 0x0F);
	lpObj->CharSet[9] |= (TempInventory[5] & 0x10);
	lpObj->CharSet[15] |= (TempInventory[5] & 0x1E0) >> 1;

	lpObj->CharSet[5] |= (TempInventory[6] & 0x0F) << 4;
	lpObj->CharSet[9] |= (TempInventory[6] & 0x10) >> 1;
	lpObj->CharSet[15] |= (TempInventory[6] & 0x1E0) >> 5;

	int level = 0;

	BYTE table[7] = {1,0,6,5,4,3,2};

	for(int n=0;n < 7;n++)
	{
		if(TempInventory[n] != 0x1FF && TempInventory[n] != 0xFFFF)
		{
			level |= ((lpObj->Inventory[n].m_Level-1)/2) << (n*3);

			lpObj->CharSet[10] |= ((lpObj->Inventory[n].m_NewOption & 0x3F)?2:0) << table[n];
			lpObj->CharSet[11] |= ((lpObj->Inventory[n].m_SetOption & 0x03)?2:0) << table[n];
		}
	}

	this->CharacterCalcAttribute(aIndex);

	if(lpObj->IsFullSetItem != 0)
	{
		lpObj->CharSet[11] |= 1;
	}

	lpObj->CharSet[6] = level >> 16;
	lpObj->CharSet[7] = level >> 8;
	lpObj->CharSet[8] = level;

	#if(GAMESERVER_UPDATE>=601)

	if(TempInventory[7] >= 0 && TempInventory[7] <= 2)
	{
		lpObj->CharSet[5] |= 4;
		lpObj->CharSet[9] |= TempInventory[7]+1;
	}
	else if(TempInventory[7] >= 3 && TempInventory[7] <= 6)
	{
		lpObj->CharSet[5] |= 8;
		lpObj->CharSet[9] |= TempInventory[7]-2;
	}
	else if(TempInventory[7] == 30)
	{
		lpObj->CharSet[5] |= 8;
		lpObj->CharSet[9] |= 5;
	}
	else if(TempInventory[7] >= 36 && TempInventory[7] <= 40)
	{
		lpObj->CharSet[5] |= 12;
		lpObj->CharSet[9] |= TempInventory[7]-35;
	}
	else if(TempInventory[7] == 41)
	{
		lpObj->CharSet[5] |= 4;
		lpObj->CharSet[9] |= 4;
	}
	else if(TempInventory[7] == 42)
	{
		lpObj->CharSet[5] |= 8;
		lpObj->CharSet[9] |= 6;
	}
	else if(TempInventory[7] == 43)
	{
		lpObj->CharSet[5] |= 12;
		lpObj->CharSet[9] |= 6;
	}
	else if(TempInventory[7] == 49)
	{
		lpObj->CharSet[5] |= 8;
		lpObj->CharSet[9] |= 7;
	}
	else if(TempInventory[7] == 50)
	{
		lpObj->CharSet[5] |= 12;
		lpObj->CharSet[9] |= 7;
	}
	else if(TempInventory[7] >= 130 && TempInventory[7] <= 135)
	{
		lpObj->CharSet[5] |= 12;
		lpObj->CharSet[17] |= (TempInventory[7]-129) << 5;
	}
	else if(TempInventory[7] >= 262 && TempInventory[7] <= 265)
	{
		lpObj->CharSet[5] |= 8;
		lpObj->CharSet[16] |= (TempInventory[7]-261) << 2;
	}
	else if(TempInventory[7] == 266)
	{
		lpObj->CharSet[5] |= 4;
		lpObj->CharSet[9] |= 5;
	}
	else if(TempInventory[7] == 267)
	{
		lpObj->CharSet[5] |= 4;
		lpObj->CharSet[9] |= 6;
	}
	else if(gCustomWing.CheckCustomWingByItem(GET_ITEM(12,TempInventory[7])) != 0)
	{
		lpObj->CharSet[5] |= 12;
		lpObj->CharSet[17] |= (gCustomWing.GetCustomWingIndex(GET_ITEM(12,TempInventory[7]))+1) << 1;
	}

	#else

	if(TempInventory[7] == 0x1FF)
	{
		lpObj->CharSet[5] |= 12;
	}
	else if(TempInventory[7] >= 0 && TempInventory[7] <= 2)
	{
		lpObj->CharSet[5] |= TempInventory[7] << 2;
	}
	else if(TempInventory[7] >= 3 && TempInventory[7] <= 6)
	{
		lpObj->CharSet[5] |= 12;
		lpObj->CharSet[9] |= TempInventory[7]-2;
	}
	else if(TempInventory[7] == 30)
	{
		lpObj->CharSet[5] |= 12;
		lpObj->CharSet[9] |= 5;
	}
	else if(TempInventory[7] >= 36 && TempInventory[7] <= 40)
	{
		lpObj->CharSet[5] |= 12;
		lpObj->CharSet[16] |= (TempInventory[7]-35) << 2;
	}
	else if(TempInventory[7] == 41)
	{
		lpObj->CharSet[5] |= 12;
		lpObj->CharSet[9] |= 6;
	}
	else if(TempInventory[7] == 42)
	{
		lpObj->CharSet[5] |= 12;
		lpObj->CharSet[16] |= 28;
	}
	else if(TempInventory[7] == 43)
	{
		lpObj->CharSet[5] |= 12;
		lpObj->CharSet[16] |= 24;
	}
	else if(TempInventory[7] >= 130 && TempInventory[7] <= 135)
	{
		lpObj->CharSet[5] |= 12;
		lpObj->CharSet[17] |= (TempInventory[7]-129) << 5;
	}
	else if(gCustomWing.CheckCustomWingByItem(GET_ITEM(12,TempInventory[7])) != 0)
	{
		lpObj->CharSet[5] |= 12;
		lpObj->CharSet[17] |= (gCustomWing.GetCustomWingIndex(GET_ITEM(12,TempInventory[7]))+1) << 1;
	}

	#endif

	if(TempInventory[8] == 0x1FF)
	{
		lpObj->CharSet[5] |= 3;
	}
	else if(TempInventory[8] >= 0 && TempInventory[8] <= 2)
	{
		lpObj->CharSet[5] |= TempInventory[8];
	}
	else if(TempInventory[8] == 3)
	{
		lpObj->CharSet[5] |= 3;
		lpObj->CharSet[10] |= 1;
	}
	else if(TempInventory[8] == 4)
	{
		lpObj->CharSet[5] |= 3;
		lpObj->CharSet[12] |= 1;
	}
	else if(TempInventory[8] == 37)
	{
		lpObj->CharSet[5] |= 3;
		lpObj->CharSet[10] &= 0xFE;
		lpObj->CharSet[12] &= 0xFE;
		lpObj->CharSet[12] |= 4;

		if((lpObj->Inventory[8].m_NewOption & 1) != 0)
		{
			lpObj->CharSet[16] |= 1;
		}
		else if((lpObj->Inventory[8].m_NewOption & 2) != 0)
		{
			lpObj->CharSet[16] |= 2;
		}
		else if((lpObj->Inventory[8].m_NewOption & 4) != 0)
		{
			lpObj->CharSet[17] |= 1;
		}
	}
	else if(TempInventory[8] == 64 || TempInventory[8] == 65 || TempInventory[8] == 67)
	{
		lpObj->CharSet[16] |= (TempInventory[8]-63) << 5;
	}
	else if(TempInventory[8] == 80)
	{
		lpObj->CharSet[16] |= 0xE0;
	}
	else if(TempInventory[8] == 106)
	{
		lpObj->CharSet[16] |= 0xA0;
	}
	else if(TempInventory[8] == 123)
	{
		lpObj->CharSet[16] |= 0x60;
	}
	//else if(TempInventory[8] == 200) //New Pet
	//{
	//	lpObj->CharSet[16] |= 225;
	//}
		//New Pet Custom
	//else if(TempInventory[8] >= 150 && TempInventory[8] <= 200)
	//{
	//	lpObj->CharSet[16] |= ((TempInventory[8] - 150) + 1) << 2;
	//}
}

bool CObjectManager::CharacterUseScroll(LPOBJ lpObj,CItem* lpItem) // OK
{
	int skill,slot;

	if((lpObj->Strength+lpObj->AddStrength) < lpItem->m_RequireStrength)
	{
		return 0;
	}

	if((lpObj->Dexterity+lpObj->AddDexterity) < lpItem->m_RequireDexterity)
	{
		return 0;
	}

	if(lpItem->m_Index >= GET_ITEM(12,8) && lpItem->m_Index <= GET_ITEM(12,24) || (lpItem->m_Index >= GET_ITEM(12,44) && lpItem->m_Index <= GET_ITEM(12,48)))
	{
		if(lpObj->Level < lpItem->m_RequireLevel)
		{
			return 0;
		}
	}

	if((lpItem->m_Index >= GET_ITEM(12,44) && lpItem->m_Index <= GET_ITEM(12,46)) || lpItem->m_Index == GET_ITEM(15,18) || lpItem->m_Index == GET_ITEM(15,27) || lpItem->m_Index == GET_ITEM(15,28))
	{
		if(gQuest.CheckQuestListState(lpObj,2,QUEST_FINISH) == 0)
		{
			return 0;
		}
	}

	if(gItemManager.CheckItemRequireClass(lpObj,lpItem->m_Index) == 0)
	{
		gItemManager.GCItemDeleteSend(lpObj->Index,-1,1);
		return 0;
	}

	if((skill=gSkillManager.GetSkillNumber(lpItem->m_Index,lpItem->m_Level)) == -1)
	{
		gItemManager.GCItemDeleteSend(lpObj->Index,-1,1);
		return 0;
	}

	if((slot=gSkillManager.AddSkill(lpObj,skill,lpItem->m_Level)) == -1)
	{
		gItemManager.GCItemDeleteSend(lpObj->Index,-1,1);
		return 0;
	}

	gSkillManager.GCSkillAddSend(lpObj->Index,slot,skill,(BYTE)lpItem->m_Level,0);
	return 1;
}

bool CObjectManager::CharacterUsePotion(LPOBJ lpObj,CItem* lpItem) // OK
{
	if(gServerInfo.m_CheckAutoPotionHack != 0 && (GetTickCount()-lpObj->PotionTime) < ((DWORD)gServerInfo.m_CheckAutoPotionHackTolerance))
	{
		return 0;
	}

	lpObj->PotionTime = GetTickCount();

	int HPValue = 0;
	int SDValue = 0;
	int MPValue = 0;
	int BPValue = 0;

	switch(lpItem->m_Index)
	{
		case GET_ITEM(14,0):
			HPValue = (int)(((lpObj->MaxLife+lpObj->AddLife)*gServerInfo.m_ApplePotionRate)/100);
			break;
		case GET_ITEM(14,1):
			HPValue = (int)(((lpObj->MaxLife+lpObj->AddLife)*gServerInfo.m_SmallLifePotionRate)/100);
			break;
		case GET_ITEM(14,2):
			HPValue = (int)(((lpObj->MaxLife+lpObj->AddLife)*gServerInfo.m_MidleLifePotionRate)/100);
			break;
		case GET_ITEM(14,3):
			HPValue = (int)(((lpObj->MaxLife+lpObj->AddLife)*gServerInfo.m_LargeLifePotionRate)/100);
			break;
		case GET_ITEM(14,4):
			MPValue = (int)(((lpObj->MaxMana+lpObj->AddMana)*gServerInfo.m_SmallManaPotionRate)/100);
			break;
		case GET_ITEM(14,5):
			MPValue = (int)(((lpObj->MaxMana+lpObj->AddMana)*gServerInfo.m_MidleManaPotionRate)/100);
			break;
		case GET_ITEM(14,6):
			MPValue = (int)(((lpObj->MaxMana+lpObj->AddMana)*gServerInfo.m_LargeManaPotionRate)/100);
			break;
		case GET_ITEM(14,35):
			SDValue = (int)(((lpObj->MaxShield+lpObj->AddShield)*gServerInfo.m_SmallShieldPotionRate)/100);
			break;
		case GET_ITEM(14,36):
			SDValue = (int)(((lpObj->MaxShield+lpObj->AddShield)*gServerInfo.m_MidleShieldPotionRate)/100);
			break;
		case GET_ITEM(14,37):
			SDValue = (int)(((lpObj->MaxShield+lpObj->AddShield)*gServerInfo.m_LargeShieldPotionRate)/100);
			break;
		case GET_ITEM(14,38):
			HPValue = (int)(((lpObj->MaxLife+lpObj->AddLife)*gServerInfo.m_SmallCompoundPotionRate1)/100);
			SDValue = (int)(((lpObj->MaxShield+lpObj->AddShield)*gServerInfo.m_SmallCompoundPotionRate2)/100);
			break;
		case GET_ITEM(14,39):
			HPValue = (int)(((lpObj->MaxLife+lpObj->AddLife)*gServerInfo.m_MidleCompoundPotionRate1)/100);
			SDValue = (int)(((lpObj->MaxShield+lpObj->AddShield)*gServerInfo.m_MidleCompoundPotionRate2)/100);
			break;
		case GET_ITEM(14,40):
			HPValue = (int)(((lpObj->MaxLife+lpObj->AddLife)*gServerInfo.m_LargeCompoundPotionRate1)/100);
			SDValue = (int)(((lpObj->MaxShield+lpObj->AddShield)*gServerInfo.m_LargeCompoundPotionRate2)/100);
			break;
		case GET_ITEM(14,70):
			HPValue = (int)(((lpObj->MaxLife+lpObj->AddLife)*gServerInfo.m_EliteLifePotionRate)/100);
			break;
		case GET_ITEM(14,71):
			MPValue = (int)(((lpObj->MaxMana+lpObj->AddMana)*gServerInfo.m_EliteManaPotionRate)/100);
			break;
		case GET_ITEM(14,133):
			SDValue = (int)(((lpObj->MaxShield+lpObj->AddShield)*gServerInfo.m_EliteShieldPotionRate)/100);
			break;
	}

	if((lpObj->Life+HPValue) > (lpObj->MaxLife+lpObj->AddLife))
	{
		lpObj->Life = lpObj->MaxLife+lpObj->AddLife;
	}
	else
	{
		lpObj->Life += HPValue;
	}

	if((lpObj->Shield+SDValue) > (lpObj->MaxShield+lpObj->AddShield))
	{
		lpObj->Shield = lpObj->MaxShield+lpObj->AddShield;
	}
	else
	{
		lpObj->Shield += SDValue;
	}

	GCLifeSend(lpObj->Index,0xFF,(int)lpObj->Life,lpObj->Shield);

	if((lpObj->Mana+MPValue) > (lpObj->MaxMana+lpObj->AddMana))
	{
		lpObj->Mana = lpObj->MaxMana+lpObj->AddMana;
	}
	else
	{
		lpObj->Mana += MPValue;
	}

	if((lpObj->BP+BPValue) > (lpObj->MaxBP+lpObj->AddBP))
	{
		lpObj->BP = lpObj->MaxBP+lpObj->AddBP;
	}
	else
	{
		lpObj->BP += BPValue;
	}

	GCManaSend(lpObj->Index,0xFF,(int)lpObj->Mana,lpObj->BP);
	return 1;
}

bool CObjectManager::CharacterUsePortal(LPOBJ lpObj,CItem* lpItem) // OK
{
	if(lpObj->Interface.use != 0)
	{
		return 0;
	}

	if(lpObj->Map == MAP_CASTLE_SIEGE)
	{
		#if(GAMESERVER_TYPE==0)

		gObjMoveGate(lpObj->Index,100);

		#else

		if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)
		{
			if(lpObj->CsJoinSide == 1)
			{
				gObjMoveGate(lpObj->Index,101);
			}
			else
			{
				gObjMoveGate(lpObj->Index,100);
			}
		}
		else
		{
			if(gCastleSiege.CheckCastleOwnerMember(lpObj->Index) != 0 || gCastleSiege.CheckCastleOwnerUnionMember(lpObj->Index) != 0)
			{
				gObjMoveGate(lpObj->Index,101);
			}
			else
			{
				gObjMoveGate(lpObj->Index,100);
			}
		}

		#endif
	}
	else
	{
		gObjMoveGate(lpObj->Index,gMapManager.GetMapDeathGate(lpObj->Map));
	}

	//if(lpObj->Map == MAP_LORENCIA)
	//{
	//	gObjMoveGate(lpObj->Index,17);
	//}
	//else if(lpObj->Map == MAP_DUNGEON)
	//{
	//	gObjMoveGate(lpObj->Index,17);
	//}
	//else if(lpObj->Map == MAP_DEVIAS)
	//{
	//	gObjMoveGate(lpObj->Index,22);
	//}
	//else if(lpObj->Map == MAP_NORIA)
	//{
	//	gObjMoveGate(lpObj->Index,27);
	//}
	//else if(lpObj->Map == MAP_LOST_TOWER)
	//{
	//	gObjMoveGate(lpObj->Index,42);
	//}
	//else if(lpObj->Map == MAP_ARENA)
	//{
	//	gObjMoveGate(lpObj->Index,115);
	//}
	//else if(lpObj->Map == MAP_ATLANS)
	//{
	//	gObjMoveGate(lpObj->Index,49);
	//}
	//else if(lpObj->Map == MAP_TARKAN)
	//{
	//	gObjMoveGate(lpObj->Index,57);
	//}
	//else if(DS_MAP_RANGE(lpObj->Map) != 0)
	//{
	//	gObjMoveGate(lpObj->Index,27);
	//}
	//else if(lpObj->Map == MAP_ICARUS)
	//{
	//	gObjMoveGate(lpObj->Index,42);
	//}
	//else if(BC_MAP_RANGE(lpObj->Map) != 0)
	//{
	//	gObjMoveGate(lpObj->Index,22);
	//}
	//else if(CC_MAP_RANGE(lpObj->Map) != 0)
	//{
	//	gObjMoveGate(lpObj->Index,17);
	//}
	//else if(KALIMA_MAP_RANGE(lpObj->Map) != 0)
	//{
	//	gObjMoveGate(lpObj->Index,17);
	//}
	//else if(lpObj->Map == MAP_LAND_OF_TRIALS)
	//{
	//	gObjMoveGate(lpObj->Index,106);
	//}
	//else if(lpObj->Map == MAP_AIDA)
	//{
	//	gObjMoveGate(lpObj->Index,119);
	//}
	//else if(lpObj->Map == MAP_CRYWOLF)
	//{
	//	gObjMoveGate(lpObj->Index,118);
	//}
	//else if(lpObj->Map == MAP_KANTURU1)
	//{
	//	gObjMoveGate(lpObj->Index,138);
	//}
	//else if(lpObj->Map == MAP_KANTURU2)
	//{
	//	gObjMoveGate(lpObj->Index,139);
	//}
	//else if(lpObj->Map == MAP_KANTURU3)
	//{
	//	gObjMoveGate(lpObj->Index,137);
	//}
	//else if(lpObj->Map == MAP_BARRACKS)
	//{
	//	gObjMoveGate(lpObj->Index,256);
	//}
	//else if(lpObj->Map == MAP_REFUGE)
	//{
	//	gObjMoveGate(lpObj->Index,256);
	//}
	//else if(IT_MAP_RANGE(lpObj->Map) != 0)
	//{
	//	gObjMoveGate(lpObj->Index,267);
	//}
	//else if(lpObj->Map == MAP_ELBELAND)
	//{
	//	gObjMoveGate(lpObj->Index,267);
	//}
	//else if(lpObj->Map == MAP_SWAMP_OF_CALMNESS)
	//{
	//	gObjMoveGate(lpObj->Index,273);
	//}
	//else if(lpObj->Map == MAP_RAKLION1)
	//{
	//	gObjMoveGate(lpObj->Index,286);
	//}
	//else if(lpObj->Map == MAP_RAKLION2)
	//{
	//	gObjMoveGate(lpObj->Index,286);
	//}
	//else if(lpObj->Map == MAP_SANTA_TOWN)
	//{
	//	gObjMoveGate(lpObj->Index,22);
	//}
	//else if(lpObj->Map == MAP_VULCANUS)
	//{
	//	gObjMoveGate(lpObj->Index,294);
	//}
	//else if(DA_MAP_RANGE(lpObj->Map) != 0)
	//{
	//	gObjMoveGate(lpObj->Index,294);
	//}
	//else if(DG_MAP_RANGE(lpObj->Map) != 0)
	//{
	//	gObjMoveGate(lpObj->Index,267);
	//}
	//else if(IG_MAP_RANGE(lpObj->Map) != 0)
	//{
	//	gObjMoveGate(lpObj->Index,22);
	//}
	//else if(lpObj->Map == MAP_LOREN_MARKET)
	//{
	//	gObjMoveGate(lpObj->Index,333);
	//}
	//else
	//{
	//	gObjMoveGate(lpObj->Index,17);
	//}

	return 1;
}

bool CObjectManager::CharacterUseJewelOfBles(LPOBJ lpObj,int SourceSlot,int TargetSlot) // OK
{
	if(INVENTORY_FULL_RANGE(SourceSlot) == 0)
	{
		return 0;
	}

	if(INVENTORY_FULL_RANGE(TargetSlot) == 0)
	{
		return 0;
	}

	if(lpObj->Inventory[SourceSlot].IsItem() == 0)
	{
		return 0;
	}

	if(lpObj->Inventory[TargetSlot].IsItem() == 0)
	{
		return 0;
	}

	if(lpObj->Inventory[TargetSlot].m_IsPeriodicItem != 0)
	{
		return 0;
	}

	CItem* lpItem = &lpObj->Inventory[TargetSlot];

	if(lpItem->m_Index == GET_ITEM(4,15)) // Disable Arrow
	{
		return 0;
	}

	if(lpItem->m_Index == GET_ITEM(4,7)) // Disable Bolt
	{
		return 0;
	}

	if(lpObj->Inventory[SourceSlot].m_Index == GET_ITEM(12,30) && lpItem->m_Index != GET_ITEM(0,41))
	{
		return 0;
	}

	if(lpItem->m_Index == GET_ITEM(0,41)) // Pick Axe
	{
		lpItem->m_Durability += ((lpObj->Inventory[SourceSlot].m_Index==GET_ITEM(12,30))?((lpObj->Inventory[SourceSlot].m_Level+1)*40):4);
		lpItem->m_Durability = ((lpItem->m_Durability>255)?255:lpItem->m_Durability);
		return 1;
	}

	if(lpItem->m_Index == GET_ITEM(13,37)) // Fenrir
	{
		lpItem->m_Durability = 255;
		return 1;
	}

	if(lpItem->m_Index > GET_ITEM(12,6) && (lpItem->m_Index < GET_ITEM(12,36) || lpItem->m_Index > GET_ITEM(12,43)) && lpItem->m_Index != GET_ITEM(12,49) && lpItem->m_Index != GET_ITEM(12,50) && (lpItem->m_Index < GET_ITEM(12,262) || lpItem->m_Index > GET_ITEM(12,265)) && lpItem->m_Index != GET_ITEM(13,30) && lpItem->IsPentagramItem() == 0 && gCustomWing.CheckCustomWingByItem(lpItem->m_Index) == 0)
	{
		return 0;
	}

	if(lpItem->m_Level >= 6)
	{
		return 0;
	}

	lpItem->m_Level++;

	float dur = (float)gItemManager.GetItemDurability(lpItem->m_Index,lpItem->m_Level,lpItem->IsExcItem(),lpItem->IsSetItem());

	lpItem->m_Durability = (dur*lpItem->m_Durability)/lpItem->m_BaseDurability;

	lpItem->Convert(lpItem->m_Index,lpItem->m_Option1,lpItem->m_Option2,lpItem->m_Option3,lpItem->m_NewOption,lpItem->m_SetOption,lpItem->m_JewelOfHarmonyOption,lpItem->m_ItemOptionEx,lpItem->m_SocketOption,lpItem->m_SocketOptionBonus);

	this->CharacterMakePreviewCharSet(lpObj->Index);
	return 1;
}

bool CObjectManager::CharacterUseJewelOfSoul(LPOBJ lpObj,int SourceSlot,int TargetSlot) // OK
{
	if(INVENTORY_FULL_RANGE(SourceSlot) == 0)
	{
		return 0;
	}

	if(INVENTORY_FULL_RANGE(TargetSlot) == 0)
	{
		return 0;
	}

	if(lpObj->Inventory[SourceSlot].IsItem() == 0)
	{
		return 0;
	}

	if(lpObj->Inventory[TargetSlot].IsItem() == 0)
	{
		return 0;
	}

	if(lpObj->Inventory[TargetSlot].m_IsPeriodicItem != 0)
	{
		return 0;
	}

	CItem* lpItem = &lpObj->Inventory[TargetSlot];

	if(lpItem->m_Index == GET_ITEM(4,15)) // Disable Arrow
	{
		return 0;
	}

	if(lpItem->m_Index == GET_ITEM(4,7)) // Disable Bolt
	{
		return 0;
	}

	if(lpItem->m_Index > GET_ITEM(12,6) && (lpItem->m_Index < GET_ITEM(12,36) || lpItem->m_Index > GET_ITEM(12,43)) && lpItem->m_Index != GET_ITEM(12,49) && lpItem->m_Index != GET_ITEM(12,50) && (lpItem->m_Index < GET_ITEM(12,262) || lpItem->m_Index > GET_ITEM(12,265)) && lpItem->m_Index != GET_ITEM(13,30) && lpItem->IsPentagramItem() == 0 && gCustomWing.CheckCustomWingByItem(lpItem->m_Index) == 0)
	{
		return 0;
	}

	if(lpItem->m_Level >= 9)
	{
		return 0;
	}

	int rate = gServerInfo.m_SoulSuccessRate[lpObj->AccountLevel];

	if(lpItem->m_Option2 != 0)
	{
		rate += gServerInfo.m_AddLuckSuccessRate1[lpObj->AccountLevel];
	}

	if((GetLargeRand()%100) < rate)
	{
		lpItem->m_Level++;
	}
	else
	{
		if(lpItem->m_Level >= 7)
		{
			lpItem->m_Level = 0;
		}
		else if(lpItem->m_Level >= 1)
		{
			lpItem->m_Level--;
		}
	}

	float dur = (float)gItemManager.GetItemDurability(lpItem->m_Index,lpItem->m_Level,lpItem->IsExcItem(),lpItem->IsSetItem());

	lpItem->m_Durability = (dur*lpItem->m_Durability)/lpItem->m_BaseDurability;

	lpItem->Convert(lpItem->m_Index,lpItem->m_Option1,lpItem->m_Option2,lpItem->m_Option3,lpItem->m_NewOption,lpItem->m_SetOption,lpItem->m_JewelOfHarmonyOption,lpItem->m_ItemOptionEx,lpItem->m_SocketOption,lpItem->m_SocketOptionBonus);

	this->CharacterMakePreviewCharSet(lpObj->Index);
	return 1;
}

bool CObjectManager::CharacterUseJewelOfLife(LPOBJ lpObj,int SourceSlot,int TargetSlot) // OK
{
	if(INVENTORY_FULL_RANGE(SourceSlot) == 0)
	{
		return 0;
	}

	if(INVENTORY_FULL_RANGE(TargetSlot) == 0)
	{
		return 0;
	}

	if(lpObj->Inventory[SourceSlot].IsItem() == 0)
	{
		return 0;
	}

	if(lpObj->Inventory[TargetSlot].IsItem() == 0)
	{
		return 0;
	}

	if(lpObj->Inventory[TargetSlot].m_IsPeriodicItem != 0)
	{
		return 0;
	}

	CItem* lpItem = &lpObj->Inventory[TargetSlot];

	if(lpItem->m_Index > GET_ITEM(12,6) && (lpItem->m_Index < GET_ITEM(12,36) || lpItem->m_Index > GET_ITEM(12,43)) && lpItem->m_Index != GET_ITEM(12,49) && lpItem->m_Index != GET_ITEM(12,50) && (lpItem->m_Index < GET_ITEM(12,262) || lpItem->m_Index > GET_ITEM(12,265)) && lpItem->m_Index != GET_ITEM(13,30) && gCustomWing.CheckCustomWingByItem(lpItem->m_Index) == 0)
	{
		return 0;
	}

	if(lpItem->m_Option3 >= gServerInfo.m_MaxItemOption)
	{
		return 0;
	}

	if((GetLargeRand()%100) < gServerInfo.m_LifeSuccessRate[lpObj->AccountLevel])
	{
		if(lpItem->m_Option3 == 0 && ((lpItem->m_Index >= GET_ITEM(12,3) && lpItem->m_Index <= GET_ITEM(12,6)) || lpItem->m_Index == GET_ITEM(12,42) || lpItem->m_Index == GET_ITEM(12,49)))
		{
			lpItem->m_NewOption &= 0xDF;
			lpItem->m_NewOption |= (32*(GetLargeRand()%2));
		}

		if(lpItem->m_Option3 == 0 && ((lpItem->m_Index >= GET_ITEM(12,36) && lpItem->m_Index <= GET_ITEM(12,40)) || lpItem->m_Index == GET_ITEM(12,43) || lpItem->m_Index == GET_ITEM(12,50)))
		{
			lpItem->m_NewOption &= 0xCF;
			lpItem->m_NewOption |= (16*(GetLargeRand()%3));
		}

		if(lpItem->m_Option3 == 0 && gCustomWing.CheckCustomWingByItem(lpItem->m_Index) != 0)
		{
			lpItem->m_NewOption &= 0xCF;
			lpItem->m_NewOption |= (16*(GetLargeRand()%3));
		}

		lpItem->m_Option3++;
	}
	else
	{
		lpItem->m_Option3 = 0;
	}

	float dur = (float)gItemManager.GetItemDurability(lpItem->m_Index,lpItem->m_Level,lpItem->IsExcItem(),lpItem->IsSetItem());

	lpItem->m_Durability = (dur*lpItem->m_Durability)/lpItem->m_BaseDurability;

	lpItem->Convert(lpItem->m_Index,lpItem->m_Option1,lpItem->m_Option2,lpItem->m_Option3,lpItem->m_NewOption,lpItem->m_SetOption,lpItem->m_JewelOfHarmonyOption,lpItem->m_ItemOptionEx,lpItem->m_SocketOption,lpItem->m_SocketOptionBonus);

	this->CharacterMakePreviewCharSet(lpObj->Index);
	return 1;
}

bool CObjectManager::CharacterUseCustomJewel(LPOBJ lpObj,int SourceSlot,int TargetSlot) // OK
{
	if(INVENTORY_FULL_RANGE(SourceSlot) == 0)
	{
		return 0;
	}

	if(INVENTORY_FULL_RANGE(TargetSlot) == 0)
	{
		return 0;
	}

	if(lpObj->Inventory[SourceSlot].IsItem() == 0)
	{
		return 0;
	}

	if(lpObj->Inventory[TargetSlot].IsItem() == 0)
	{
		return 0;
	}

	if(lpObj->Inventory[TargetSlot].m_IsPeriodicItem != 0)
	{
		return 0;
	}

	CItem* lpItem = &lpObj->Inventory[TargetSlot];

	if(gCustomJewel.CheckCustomJewelApplyItem(lpObj->Inventory[SourceSlot].m_Index,lpItem) == 0)
	{
		return 0;
	}

	if((GetLargeRand()%100) < gCustomJewel.GetCustomJewelSuccessRate(lpObj->Inventory[SourceSlot].m_Index,lpObj->AccountLevel))
	{
		CUSTOM_JEWEL_SUCCESS_INFO* lpInfo = gCustomJewel.GetSuccessInfo(lpObj->Inventory[SourceSlot].m_Index);

		lpItem->m_Level = (((lpItem->m_Level+lpInfo->Level)>15)?15:(lpItem->m_Level+lpInfo->Level));

		lpItem->m_Option1 = (((lpItem->m_Option1+lpInfo->Option1)>1)?1:(lpItem->m_Option1+lpInfo->Option1));

		lpItem->m_Option2 = (((lpItem->m_Option2+lpInfo->Option2)>1)?1:(lpItem->m_Option2+lpInfo->Option2));

		lpItem->m_Option3 = (((lpItem->m_Option3+lpInfo->Option3)>gServerInfo.m_MaxItemOption)?gServerInfo.m_MaxItemOption:(lpItem->m_Option3+lpInfo->Option3));

		gCustomJewel.GetCustomJewelNewOption(lpItem,lpInfo->NewOption);

		gCustomJewel.GetCustomJewelSetOption(lpItem,lpInfo->SetOption);

		gCustomJewel.GetCustomJewelSocketOption(lpItem,lpInfo->SocketOption);

		float dur = (float)gItemManager.GetItemDurability(lpItem->m_Index,lpItem->m_Level,lpItem->IsExcItem(),lpItem->IsSetItem());

		lpItem->m_Durability = (dur*lpItem->m_Durability)/lpItem->m_BaseDurability;

		lpItem->Convert(lpItem->m_Index,lpItem->m_Option1,lpItem->m_Option2,lpItem->m_Option3,lpItem->m_NewOption,lpItem->m_SetOption,lpItem->m_JewelOfHarmonyOption,lpItem->m_ItemOptionEx,lpItem->m_SocketOption,lpItem->m_SocketOptionBonus);

		this->CharacterMakePreviewCharSet(lpObj->Index);
	}
	else
	{
		CUSTOM_JEWEL_FAILURE_INFO* lpInfo = gCustomJewel.GetFailureInfo(lpObj->Inventory[SourceSlot].m_Index);

		lpItem->m_Level = (((lpItem->m_Level-lpInfo->Level)<0)?0:(lpItem->m_Level-lpInfo->Level));

		lpItem->m_Option1 = (((lpItem->m_Option1-lpInfo->Option1)<0)?0:(lpItem->m_Option1-lpInfo->Option1));

		lpItem->m_Option2 = (((lpItem->m_Option2-lpInfo->Option2)<0)?0:(lpItem->m_Option2-lpInfo->Option2));

		lpItem->m_Option3 = (((lpItem->m_Option3-lpInfo->Option3)<0)?0:(lpItem->m_Option3-lpInfo->Option3));

		float dur = (float)gItemManager.GetItemDurability(lpItem->m_Index,lpItem->m_Level,lpItem->IsExcItem(),lpItem->IsSetItem());

		lpItem->m_Durability = (dur*lpItem->m_Durability)/lpItem->m_BaseDurability;

		lpItem->Convert(lpItem->m_Index,lpItem->m_Option1,lpItem->m_Option2,lpItem->m_Option3,lpItem->m_NewOption,lpItem->m_SetOption,lpItem->m_JewelOfHarmonyOption,lpItem->m_ItemOptionEx,lpItem->m_SocketOption,lpItem->m_SocketOptionBonus);

		this->CharacterMakePreviewCharSet(lpObj->Index);
	}

	return 1;
}

bool CObjectManager::CharacterUseTalismanOfResurrection(LPOBJ lpObj,int SourceSlot) // OK
{
	if(lpObj->ResurrectionTalismanActive == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(226));
		return 0;
	}

	if(gMoveSummon.CheckMoveSummon(lpObj,lpObj->ResurrectionTalismanMap,lpObj->ResurrectionTalismanX,lpObj->ResurrectionTalismanY) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(226));
		return 0;
	}

	gObjTeleport(lpObj->Index,lpObj->ResurrectionTalismanMap,lpObj->ResurrectionTalismanX,lpObj->ResurrectionTalismanY);

	lpObj->ResurrectionTalismanActive = 0;

	lpObj->ResurrectionTalismanMap = 0;

	lpObj->ResurrectionTalismanX = 0;

	lpObj->ResurrectionTalismanY = 0;

	GCServerCommandSend(lpObj->Index,60,0,0);
	return 1;
}

bool CObjectManager::CharacterUseTalismanOfMobility(LPOBJ lpObj,int SourceSlot) // OK
{
	if(lpObj->MobilityTalismanActive == 0)
	{
		lpObj->MobilityTalismanActive = 1;
		lpObj->MobilityTalismanMap = lpObj->Map;
		lpObj->MobilityTalismanX = lpObj->X;
		lpObj->MobilityTalismanY = lpObj->Y;
		lpObj->Inventory[SourceSlot].m_Durability = 1;
		gItemManager.GCItemDurSend(lpObj->Index,SourceSlot,(BYTE)lpObj->Inventory[SourceSlot].m_Durability,1);
		return 0;
	}

	if(gMoveSummon.CheckMoveSummon(lpObj,lpObj->MobilityTalismanMap,lpObj->MobilityTalismanX,lpObj->MobilityTalismanY) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(226));
		return 0;
	}

	gObjTeleport(lpObj->Index,lpObj->MobilityTalismanMap,lpObj->MobilityTalismanX,lpObj->MobilityTalismanY);

	lpObj->MobilityTalismanActive = 0;

	lpObj->MobilityTalismanMap = 0;

	lpObj->MobilityTalismanX = 0;

	lpObj->MobilityTalismanY = 0;

	GCServerCommandSend(lpObj->Index,61,0,0);
	return 1;
}

bool CObjectManager::CharacterUseCreationCard(LPOBJ lpObj,CItem* lpItem) // OK
{
	switch(lpItem->m_Index)
	{
		case GET_ITEM(14,91):
			if((lpObj->ClassCode & 1) == 0 && (lpObj->ClassFlag & 1) != 0)
			{
				lpObj->ClassCode |= 1;
				GDCreationCardSaveSend(lpObj->Index,(lpObj->ClassCode & 9));
				return 1;
			}
			break;
		case GET_ITEM(14,97):
			if((lpObj->ClassCode & 4) == 0 && (lpObj->ClassFlag & 4) != 0)
			{
				lpObj->ClassCode |= 4;
				GDCreationCardSaveSend(lpObj->Index,(lpObj->ClassCode & 9));
				return 1;
			}
			break;
		case GET_ITEM(14,98):
			if((lpObj->ClassCode & 2) == 0 && (lpObj->ClassFlag & 2) != 0)
			{
				lpObj->ClassCode |= 2;
				GDCreationCardSaveSend(lpObj->Index,(lpObj->ClassCode & 9));
				return 1;
			}
			break;
		case GET_ITEM(14,169):
			if((lpObj->ClassCode & 8) == 0 && (lpObj->ClassFlag & 8) != 0)
			{
				lpObj->ClassCode |= 8;
				GDCreationCardSaveSend(lpObj->Index,(lpObj->ClassCode & 9));
				return 1;
			}
			break;
	}

	return 0;
}

bool CObjectManager::CharacterUseInventoryExpansion(LPOBJ lpObj,CItem* lpItem) // OK
{
	if(lpObj->ExtInventory >= 2)
	{
		return 0;
	}

	lpObj->ExtInventory++;
	GDCharacterInfoSaveSend(lpObj->Index);
	return 1;
}

bool CObjectManager::CharacterUseWarehouseExpansion(LPOBJ lpObj,CItem* lpItem) // OK
{
	if(lpObj->ExtWarehouse >= 1)
	{
		return 0;
	}

	lpObj->ExtWarehouse++;
	GDCharacterInfoSaveSend(lpObj->Index);
	return 1;
}

bool CObjectManager::CharacterUseTradeableSeal(LPOBJ lpObj,int SourceSlot,int TargetSlot) // OK
{
	if(INVENTORY_FULL_RANGE(SourceSlot) == 0)
	{
		return 0;
	}

	if(INVENTORY_FULL_RANGE(TargetSlot) == 0)
	{
		return 0;
	}

	if(lpObj->Inventory[SourceSlot].IsItem() == 0)
	{
		return 0;
	}

	if(lpObj->Inventory[TargetSlot].IsItem() == 0)
	{
		return 0;
	}

	CItem* lpItem = &lpObj->Inventory[TargetSlot];

	if(lpItem->IsPentagramItem() == 0 || lpItem->m_Durability >= 255)
	{
		return 0;
	}

	lpItem->m_Durability++;

	return 1;
}

void CObjectManager::CharacterMonsterDieHunt(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	int HPValue = (int)(((lpObj->MaxLife+lpObj->AddLife)*lpObj->HuntHP)/100)+lpTarget->Level;
	int SDValue = (int)(((lpObj->MaxShield+lpObj->AddShield)*lpObj->HuntSD)/100);
	int MPValue = (int)(((lpObj->MaxMana+lpObj->AddMana)*lpObj->HuntMP)/100);
	int BPValue = (int)(((lpObj->MaxBP+lpObj->AddBP)*lpObj->HuntBP)/100);

	if(HPValue != 0 || SDValue != 0)
	{
		if((lpObj->Life+HPValue) > (lpObj->MaxLife+lpObj->AddLife))
		{
			lpObj->Life = lpObj->MaxLife+lpObj->AddLife;
		}
		else
		{
			lpObj->Life += HPValue;
		}

		if((lpObj->Shield+SDValue) > (lpObj->MaxShield+lpObj->AddShield))
		{
			lpObj->Shield = lpObj->MaxShield+lpObj->AddShield;
		}
		else
		{
			lpObj->Shield += SDValue;
		}

		GCLifeSend(lpObj->Index,0xFF,(int)lpObj->Life,lpObj->Shield);
	}

	if(MPValue != 0 || BPValue != 0)
	{
		if((lpObj->Mana+MPValue) > (lpObj->MaxMana+lpObj->AddMana))
		{
			lpObj->Mana = lpObj->MaxMana+lpObj->AddMana;
		}
		else
		{
			lpObj->Mana += MPValue;
		}

		if((lpObj->BP+BPValue) > (lpObj->MaxBP+lpObj->AddBP))
		{
			lpObj->BP = lpObj->MaxBP+lpObj->AddBP;
		}
		else
		{
			lpObj->BP += BPValue;
		}

		GCManaSend(lpObj->Index,0xFF,(int)lpObj->Mana,lpObj->BP);
	}
}

void CObjectManager::CharacterAutoRecuperation(LPOBJ lpObj) // OK
{
	int HPChange = 0;
	int SDChange = 0;
	int MPChange = 0;
	int BPChange = 0;

	if(((lpObj->HPRecoveryCount++)%5) == 0)
	{
		if(lpObj->Life != (lpObj->MaxLife+lpObj->AddLife))
		{
			int rate = gServerInfo.m_HPRecoveryRate[lpObj->Class];

			rate += (((GetTickCount()-lpObj->HPAutoRecuperationTime)>5000)?5:0);

			rate += lpObj->HPRecoveryRate;

			rate += lpObj->EffectOption.AddHPRecoveryRate;

			int value = (int)(((lpObj->MaxLife+lpObj->AddLife)*rate)/100);

			value += lpObj->HPRecovery;

			value += lpObj->EffectOption.AddHPRecovery;

			if((lpObj->Life+value) > (lpObj->MaxLife+lpObj->AddLife))
			{
				lpObj->Life = lpObj->MaxLife+lpObj->AddLife;
			}
			else
			{
				lpObj->Life += value;
			}

			HPChange = 1;
		}
	}

	if(((lpObj->SDRecoveryCount++)%5) == 0)
	{
		if(lpObj->Shield != (lpObj->MaxShield+lpObj->AddShield))
		{
			if(lpObj->SDRecoveryType != 0 || gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) != 0)
			{
				int rate = gServerInfo.m_SDRecoveryRate[lpObj->Class];

				rate += (((GetTickCount()-lpObj->SDAutoRecuperationTime)>5000)?5:0);

				rate += lpObj->SDRecoveryRate;

				rate += lpObj->EffectOption.AddSDRecoveryRate;

				int value = (int)(((lpObj->MaxShield+lpObj->AddShield)*rate)/100);

				value += lpObj->SDRecovery;

				value += lpObj->EffectOption.AddSDRecovery;

				if((lpObj->Shield+value) > (lpObj->MaxShield+lpObj->AddShield))
				{
					lpObj->Shield = lpObj->MaxShield+lpObj->AddShield;
				}
				else
				{
					lpObj->Shield += value;
				}

				SDChange = 1;
			}
		}
	}

	if(((lpObj->MPRecoveryCount++)%3) == 0)
	{
		if(lpObj->Mana != (lpObj->MaxMana+lpObj->AddMana))
		{
			int rate = gServerInfo.m_MPRecoveryRate[lpObj->Class];

			rate += (((GetTickCount()-lpObj->MPAutoRecuperationTime)>5000)?3:0);

			rate += lpObj->MPRecoveryRate;

			rate += lpObj->EffectOption.AddMPRecoveryRate;

			int value = (int)(((lpObj->MaxMana+lpObj->AddMana)*rate)/100);

			value += lpObj->MPRecovery;

			value += lpObj->EffectOption.AddMPRecovery;

			if((lpObj->Mana+value) > (lpObj->MaxMana+lpObj->AddMana))
			{
				lpObj->Mana = lpObj->MaxMana+lpObj->AddMana;
			}
			else
			{
				lpObj->Mana += value;
			}

			MPChange = 1;
		}
	}

	if(((lpObj->BPRecoveryCount++)%3) == 0)
	{
		if(lpObj->BP != (lpObj->MaxBP+lpObj->AddBP))
		{
			int rate = gServerInfo.m_BPRecoveryRate[lpObj->Class];

			rate += (((GetTickCount()-lpObj->BPAutoRecuperationTime)>5000)?3:0);

			rate += lpObj->BPRecoveryRate;

			rate += lpObj->EffectOption.AddBPRecoveryRate;

			int value = (int)(((lpObj->MaxBP+lpObj->AddBP)*rate)/100);

			value += lpObj->BPRecovery;

			value += lpObj->EffectOption.AddBPRecovery;

			if((lpObj->BP+value) > (lpObj->MaxBP+lpObj->AddBP))
			{
				lpObj->BP = lpObj->MaxBP+lpObj->AddBP;
			}
			else
			{
				lpObj->BP += value;
			}

			BPChange = 1;
		}
	}

	if(HPChange != 0 || SDChange != 0)
	{
		GCLifeSend(lpObj->Index,0xFF,(int)lpObj->Life,lpObj->Shield);
	}

	if(MPChange != 0 || BPChange != 0)
	{
		GCManaSend(lpObj->Index,0xFF,(int)lpObj->Mana,lpObj->BP);
	}
}

void CObjectManager::CharacterItemDurationDown(LPOBJ lpObj) // OK
{
	if(((lpObj->TimeCount++)%10) != 0)
	{
		return;
	}

	for(int n=2;n < 10;n++)
	{
		if(lpObj->Inventory[n].IsItem() != 0)
		{
			bool result = 0;

			switch(n)
			{
				case 2:
					result = lpObj->Inventory[n].LuckyDurabilityDown(lpObj->Index,10);
					break;
				case 3:
					result = lpObj->Inventory[n].LuckyDurabilityDown(lpObj->Index,10);
					break;
				case 4:
					result = lpObj->Inventory[n].LuckyDurabilityDown(lpObj->Index,10);
					break;
				case 5:
					result = lpObj->Inventory[n].LuckyDurabilityDown(lpObj->Index,10);
					break;
				case 6:
					result = lpObj->Inventory[n].LuckyDurabilityDown(lpObj->Index,10);
					break;
				case 7:
					result = lpObj->Inventory[n].WingDurabilityDown(lpObj->Index,((gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1)==0)?1:0));
					break;
				case 9:
					result = lpObj->Inventory[n].PendantDurabilityDown(lpObj->Index,((gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1)==0)?1:0));
					break;
			}

			if(result != 0)
			{
				gItemManager.GCItemDurSend(lpObj->Index,n,(BYTE)lpObj->Inventory[n].m_Durability,0);
			}
		}
	}

	for(int n=10;n < 12;n++)
	{
		if(lpObj->Inventory[n].IsItem() != 0)
		{
			bool result = 0;

			switch(lpObj->Inventory[n].m_Index)
			{
				case GET_ITEM(13,10):
					result = lpObj->Inventory[n].RingDurabilityDown(lpObj->Index,113);
					break;
				case GET_ITEM(13,20):
					result = lpObj->Inventory[n].RingDurabilityDown(lpObj->Index,((gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1)==0)?70:0));
					break;
				case GET_ITEM(13,38):
					result = lpObj->Inventory[n].RingDurabilityDown(lpObj->Index,((gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1)==0)?63:0));
					break;
				case GET_ITEM(13,39):
					result = lpObj->Inventory[n].RingDurabilityDown(lpObj->Index,113);
					break;
				case GET_ITEM(13,40):
					result = lpObj->Inventory[n].RingDurabilityDown(lpObj->Index,113);
					break;
				case GET_ITEM(13,41):
					result = lpObj->Inventory[n].RingDurabilityDown(lpObj->Index,113);
					break;
				case GET_ITEM(13,42):
					result = lpObj->Inventory[n].RingDurabilityDown(lpObj->Index,0);
					break;
				case GET_ITEM(13,68):
					result = lpObj->Inventory[n].RingDurabilityDown(lpObj->Index,113);
					break;
				case GET_ITEM(13,76):
					result = lpObj->Inventory[n].RingDurabilityDown(lpObj->Index,113);
					break;
				case GET_ITEM(13,122):
					result = lpObj->Inventory[n].RingDurabilityDown(lpObj->Index,113);
					break;
				default:
					result = lpObj->Inventory[n].RingDurabilityDown(lpObj->Index,((gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1)==0)?1:0));
					break;
			}

			if(result != 0)
			{
				gItemManager.GCItemDurSend(lpObj->Index,n,(BYTE)lpObj->Inventory[n].m_Durability,0);

				if(gSkillManager.SkillChangeUse(lpObj->Index) != 0)
				{
					gObjViewportListProtocolCreate(lpObj);
					this->CharacterUpdateMapEffect(lpObj);
				}
			}
		}
	}
}

void CObjectManager::CharacterCalcBP(LPOBJ lpObj) // OK
{
	switch(lpObj->Class)
	{
		case CLASS_DW:
			lpObj->MaxBP = (int)(((lpObj->Strength+lpObj->AddStrength)*0.20)+((lpObj->Dexterity+lpObj->AddDexterity)*0.40)+((lpObj->Vitality+lpObj->AddVitality)*0.30)+((lpObj->Energy+lpObj->AddEnergy)*0.20));
			break;
		case CLASS_DK:
			lpObj->MaxBP = (int)(((lpObj->Strength+lpObj->AddStrength)*0.15)+((lpObj->Dexterity+lpObj->AddDexterity)*0.20)+((lpObj->Vitality+lpObj->AddVitality)*0.30)+((lpObj->Energy+lpObj->AddEnergy)*1.00));
			break;
		case CLASS_FE:
			lpObj->MaxBP = (int)(((lpObj->Strength+lpObj->AddStrength)*0.30)+((lpObj->Dexterity+lpObj->AddDexterity)*0.20)+((lpObj->Vitality+lpObj->AddVitality)*0.30)+((lpObj->Energy+lpObj->AddEnergy)*0.20));
			break;
		case CLASS_MG:
			lpObj->MaxBP = (int)(((lpObj->Strength+lpObj->AddStrength)*0.20)+((lpObj->Dexterity+lpObj->AddDexterity)*0.25)+((lpObj->Vitality+lpObj->AddVitality)*0.30)+((lpObj->Energy+lpObj->AddEnergy)*0.15));
			break;
		case CLASS_DL:
			lpObj->MaxBP = (int)(((lpObj->Strength+lpObj->AddStrength)*0.30)+((lpObj->Dexterity+lpObj->AddDexterity)*0.20)+((lpObj->Vitality+lpObj->AddVitality)*0.10)+((lpObj->Energy+lpObj->AddEnergy)*0.15)+((lpObj->Leadership+lpObj->AddLeadership)*0.30));
			break;
		case CLASS_SU:
			lpObj->MaxBP = (int)(((lpObj->Strength+lpObj->AddStrength)*0.20)+((lpObj->Dexterity+lpObj->AddDexterity)*0.25)+((lpObj->Vitality+lpObj->AddVitality)*0.30)+((lpObj->Energy+lpObj->AddEnergy)*0.15));
			break;
		case CLASS_RF:
			lpObj->MaxBP = (int)(((lpObj->Strength+lpObj->AddStrength)*0.15)+((lpObj->Dexterity+lpObj->AddDexterity)*0.20)+((lpObj->Vitality+lpObj->AddVitality)*0.30)+((lpObj->Energy+lpObj->AddEnergy)*1.00));
			break;
	}
}

void CObjectManager::CharacterCalcSD(LPOBJ lpObj) // OK
{
	int value = (lpObj->Strength+lpObj->AddStrength)+(lpObj->Dexterity+lpObj->AddDexterity)+(lpObj->Vitality+lpObj->AddVitality)+(lpObj->Energy+lpObj->AddEnergy);

	if(lpObj->Class == CLASS_DL)
	{
		value += (lpObj->Leadership+lpObj->AddLeadership);
	}

	lpObj->MaxShield = (((value*gServerInfo.m_ShieldGaugeConstA)/10)+(((lpObj->Level+lpObj->MasterLevel)*(lpObj->Level+lpObj->MasterLevel))/gServerInfo.m_ShieldGaugeConstB))+(lpObj->Defense/2);
}

void CObjectManager::CharacterCalcPvPAccessoryOption(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	for(int n=9;n < INVENTORY_WEAR_SIZE;n++)
	{
		CItem* lpItem = &lpObj->Inventory[n];

		if(lpItem->IsItem() == 0 || lpItem->m_IsValidItem == 0 || lpItem->m_Durability == 0)
		{
			continue;
		}

		switch(lpItem->m_Index)
		{
			case GET_ITEM(13,171):
				lpObj->ResistIgnoreDefenseRate += ((lpItem->IsExcItem()==0)?1:3)+lpItem->m_Level;
				break;
			case GET_ITEM(13,172):
				lpObj->ResistIgnoreShieldGaugeRate += ((lpItem->IsExcItem()==0)?1:3)+lpItem->m_Level;
				break;
			case GET_ITEM(13,173):
				lpObj->ResistDoubleDamageRate += ((lpItem->IsExcItem()==0)?1:3)+lpItem->m_Level;
				break;
			case GET_ITEM(13,174):
				lpObj->ResistStunRate += ((lpItem->IsExcItem()==0)?1:3)+lpItem->m_Level;
				break;
			case GET_ITEM(13,175):
				lpObj->ResistExcellentDamageRate += ((lpItem->IsExcItem()==0)?1:3)+lpItem->m_Level;
				break;
			case GET_ITEM(13,176):
				lpObj->ResistCriticalDamageRate += ((lpItem->IsExcItem()==0)?1:3)+lpItem->m_Level;
				break;
		}
	}

	#endif
}

void CObjectManager::CharacterCalcAttribute(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Type != OBJECT_USER)
	{
		return;
	}

	CItem* Right = &lpObj->Inventory[0];
	CItem* Left = &lpObj->Inventory[1];
	CItem* Gloves = &lpObj->Inventory[5];
	CItem* Wings = &lpObj->Inventory[7];
	CItem* Helper = &lpObj->Inventory[8];
	CItem* Amulet = &lpObj->Inventory[9];
	CItem* Ring1 = &lpObj->Inventory[10];
	CItem* Ring2 = &lpObj->Inventory[11];
	#if(GAMESERVER_UPDATE>=701)
	CItem* Pentagram = &lpObj->Inventory[236];
	#endif

	float TotalHP = 100;
	float TotalSD = 100;
	float TotalMP = 100;
	float TotalBP = 100;

	if((lpObj->MaxLife+lpObj->AddLife) != 0)
	{
		TotalHP = (float)(lpObj->Life*100)/(float)(lpObj->MaxLife+lpObj->AddLife);
	}

	if((lpObj->MaxShield+lpObj->AddShield) != 0)
	{
		TotalSD = (float)(lpObj->Shield*100)/(float)(lpObj->MaxShield+lpObj->AddShield);
	}

	if((lpObj->MaxMana+lpObj->AddMana) != 0)
	{
		TotalMP = (float)(lpObj->Mana*100)/(lpObj->MaxMana+lpObj->AddMana);
	}

	if((lpObj->MaxBP+lpObj->AddBP) != 0)
	{
		TotalBP = (float)(lpObj->BP*100)/(float)(lpObj->MaxBP+lpObj->AddBP);
	}

	lpObj->HaveWeaponInHand = 1;

	if(Right->IsItem() == 0 && Left->IsItem() == 0)
	{
		lpObj->HaveWeaponInHand = 0;
	}
	else if(Left->IsItem() == 0 && Right->m_Index == GET_ITEM(4,15)) // Arrow
	{
		lpObj->HaveWeaponInHand = 0;
	}
	else if(Right->IsItem() == 0)
	{
		if(Left->m_Index == GET_ITEM(4,7)) // Bolt
		{
			lpObj->HaveWeaponInHand = 0;
		}
		else if(Left->m_Index >= GET_ITEM(6,0) && Left->m_Index < GET_ITEM(7,0)) // Shields
		{
			lpObj->HaveWeaponInHand = 0;
		}
	}

	for(int n=0;n < INVENTORY_WEAR_SIZE;n++)
	{
		if(lpObj->Inventory[n].IsItem() != 0)
		{
			lpObj->Inventory[n].m_IsValidItem = 1;
		}
	}

	#if(GAMESERVER_UPDATE>=701)

	for(int n=INVENTORY_FULL_SIZE;n < INVENTORY_SIZE;n++)
	{
		if(lpObj->Inventory[n].IsItem() != 0)
		{
			lpObj->Inventory[n].m_IsValidItem = 1;
		}
	}

	#endif

	lpObj->AddLife = 0;
	lpObj->AddMana = 0;
	lpObj->AddBP = 0;
	lpObj->AddShield = 0;
	lpObj->SkillLongSpearChange = 0;

	memset(lpObj->AddResistance,0,sizeof(lpObj->AddResistance));

	gObjClearSpecialOption(lpObj);

	bool change = 1;

	while(change != 0)
	{
		change = 0;
		lpObj->AddStrength = lpObj->EffectOption.AddStrength;
		lpObj->AddDexterity = lpObj->EffectOption.AddDexterity;
		lpObj->AddVitality = lpObj->EffectOption.AddVitality;
		lpObj->AddEnergy = lpObj->EffectOption.AddEnergy;
		lpObj->AddLeadership = lpObj->EffectOption.AddLeadership;

		gSetItemOption.CalcSetItemStat(lpObj);
		gItemOption.CalcItemCommonOption(lpObj,1);
		gSetItemOption.CalcSetItemOption(lpObj,1);
		gJewelOfHarmonyOption.CalcJewelOfHarmonyOption(lpObj,1);
		g380ItemOption.Calc380ItemOption(lpObj,1);
		gSocketItemOption.CalcSocketItemOption(lpObj,1);
		gMasterSkillTree.CalcMasterSkillTreeOption(lpObj,1);
		#if(GAMESERVER_UPDATE>=701)
		gPentagramSystem.CalcPentagramOption(lpObj,1);
		gPentagramSystem.CalcPentagramJewelOption(lpObj,1);
		#endif
		#if(GAMESERVER_UPDATE>=803)
		gMuunSystem.CalcMuunOption(lpObj,1);
		#endif

		for(int n=0;n < INVENTORY_WEAR_SIZE;n++)
		{
			if(lpObj->Inventory[n].m_IsValidItem != 0)
			{
				if(gItemManager.IsValidItem(lpObj,&lpObj->Inventory[n]) == 0)
				{
					change = 1;
					lpObj->Inventory[n].m_IsValidItem = 0;
				}
			}
		}

		#if(GAMESERVER_UPDATE>=701)

		for(int n=INVENTORY_FULL_SIZE;n < INVENTORY_SIZE;n++)
		{
			if(lpObj->Inventory[n].m_IsValidItem != 0)
			{
				if(gItemManager.IsValidItem(lpObj,&lpObj->Inventory[n]) == 0)
				{
					change = 1;
					lpObj->Inventory[n].m_IsValidItem = 0;
				}
			}
		}

		#endif
	}

	int Strength = lpObj->Strength+lpObj->AddStrength;
	int Dexterity = lpObj->Dexterity+lpObj->AddDexterity;
	int Vitality = lpObj->Vitality+lpObj->AddVitality;
	int Energy = lpObj->Energy+lpObj->AddEnergy;
	int Leadership = lpObj->Leadership+lpObj->AddLeadership;

	if(lpObj->Class == CLASS_DW)
	{
		lpObj->PhysiDamageMinRight = Strength/gServerInfo.m_DWPhysiDamageMinConstA;
		lpObj->PhysiDamageMaxRight = Strength/gServerInfo.m_DWPhysiDamageMaxConstA;
		lpObj->PhysiDamageMinLeft = Strength/gServerInfo.m_DWPhysiDamageMinConstA;
		lpObj->PhysiDamageMaxLeft = Strength/gServerInfo.m_DWPhysiDamageMaxConstA;
		lpObj->MagicDamageMin = Energy/gServerInfo.m_DWMagicDamageMinConstA;
		lpObj->MagicDamageMax = Energy/gServerInfo.m_DWMagicDamageMaxConstA;
		lpObj->CurseDamageMin = Energy/gServerInfo.m_DWMagicDamageMinConstA;
		lpObj->CurseDamageMax = Energy/gServerInfo.m_DWMagicDamageMaxConstA;
	}
	else if(lpObj->Class == CLASS_DK)
	{
		lpObj->PhysiDamageMinRight = Strength/gServerInfo.m_DKPhysiDamageMinConstA;
		lpObj->PhysiDamageMaxRight = Strength/gServerInfo.m_DKPhysiDamageMaxConstA;
		lpObj->PhysiDamageMinLeft = Strength/gServerInfo.m_DKPhysiDamageMinConstA;
		lpObj->PhysiDamageMaxLeft = Strength/gServerInfo.m_DKPhysiDamageMaxConstA;
		lpObj->MagicDamageMin = Energy/gServerInfo.m_DKMagicDamageMinConstA;
		lpObj->MagicDamageMax = Energy/gServerInfo.m_DKMagicDamageMaxConstA;
		lpObj->CurseDamageMin = Energy/gServerInfo.m_DKMagicDamageMinConstA;
		lpObj->CurseDamageMax = Energy/gServerInfo.m_DKMagicDamageMaxConstA;
		lpObj->DKDamageMultiplierRate = 200+(Energy/gServerInfo.m_DKDamageMultiplierConstA);
		lpObj->DKDamageMultiplierRate = ((lpObj->DKDamageMultiplierRate>gServerInfo.m_DKDamageMultiplierMaxRate)?gServerInfo.m_DKDamageMultiplierMaxRate:lpObj->DKDamageMultiplierRate);
	}
	else if(lpObj->Class == CLASS_FE)
	{
		if((Right->m_Index >= GET_ITEM(4,0) && Right->m_Index < GET_ITEM(5,0) && Right->m_Index != GET_ITEM(4,15)) || (Left->m_Index >= GET_ITEM(4,0) && Left->m_Index < GET_ITEM(5,0) && Left->m_Index != GET_ITEM(4,7)))
		{
			if((Right->IsItem() != 0 && Right->m_IsValidItem == 0) || (Left->IsItem() != 0 && Left->m_IsValidItem == 0))
			{
				lpObj->PhysiDamageMinRight = (Strength+Dexterity)/gServerInfo.m_FEPhysiDamageMinConstA;
				lpObj->PhysiDamageMaxRight = (Strength+Dexterity)/gServerInfo.m_FEPhysiDamageMaxConstA;
				lpObj->PhysiDamageMinLeft = (Strength+Dexterity)/gServerInfo.m_FEPhysiDamageMinConstA;
				lpObj->PhysiDamageMaxLeft = (Strength+Dexterity)/gServerInfo.m_FEPhysiDamageMaxConstA;
			}
			else
			{
				lpObj->PhysiDamageMinRight = (Strength/gServerInfo.m_FEPhysiDamageMinBowConstA)+(Dexterity/gServerInfo.m_FEPhysiDamageMinBowConstB);
				lpObj->PhysiDamageMaxRight = (Strength/gServerInfo.m_FEPhysiDamageMaxBowConstA)+(Dexterity/gServerInfo.m_FEPhysiDamageMaxBowConstB);
				lpObj->PhysiDamageMinLeft = (Strength/gServerInfo.m_FEPhysiDamageMinBowConstA)+(Dexterity/gServerInfo.m_FEPhysiDamageMinBowConstB);
				lpObj->PhysiDamageMaxLeft = (Strength/gServerInfo.m_FEPhysiDamageMaxBowConstA)+(Dexterity/gServerInfo.m_FEPhysiDamageMaxBowConstB);
			}
		}
		else
		{
			lpObj->PhysiDamageMinRight = (Strength+Dexterity)/gServerInfo.m_FEPhysiDamageMinConstA;
			lpObj->PhysiDamageMaxRight = (Strength+Dexterity)/gServerInfo.m_FEPhysiDamageMaxConstA;
			lpObj->PhysiDamageMinLeft = (Strength+Dexterity)/gServerInfo.m_FEPhysiDamageMinConstA;
			lpObj->PhysiDamageMaxLeft = (Strength+Dexterity)/gServerInfo.m_FEPhysiDamageMaxConstA;
		}

		lpObj->MagicDamageMin = Energy/gServerInfo.m_FEMagicDamageMinConstA;
		lpObj->MagicDamageMax = Energy/gServerInfo.m_FEMagicDamageMaxConstA;
		lpObj->CurseDamageMin = Energy/gServerInfo.m_FEMagicDamageMinConstA;
		lpObj->CurseDamageMax = Energy/gServerInfo.m_FEMagicDamageMaxConstA;
	}
	else if(lpObj->Class == CLASS_MG)
	{
		lpObj->PhysiDamageMinRight = (Strength/gServerInfo.m_MGPhysiDamageMinConstA)+(Energy/gServerInfo.m_MGPhysiDamageMinConstB);
		lpObj->PhysiDamageMaxRight = (Strength/gServerInfo.m_MGPhysiDamageMaxConstA)+(Energy/gServerInfo.m_MGPhysiDamageMaxConstB);
		lpObj->PhysiDamageMinLeft = (Strength/gServerInfo.m_MGPhysiDamageMinConstA)+(Energy/gServerInfo.m_MGPhysiDamageMinConstB);
		lpObj->PhysiDamageMaxLeft = (Strength/gServerInfo.m_MGPhysiDamageMaxConstA)+(Energy/gServerInfo.m_MGPhysiDamageMaxConstB);
		lpObj->MagicDamageMin = Energy/gServerInfo.m_MGMagicDamageMinConstA;
		lpObj->MagicDamageMax = Energy/gServerInfo.m_MGMagicDamageMaxConstA;
		lpObj->CurseDamageMin = Energy/gServerInfo.m_MGMagicDamageMinConstA;
		lpObj->CurseDamageMax = Energy/gServerInfo.m_MGMagicDamageMaxConstA;
	}
	else if(lpObj->Class == CLASS_DL)
	{
		lpObj->PhysiDamageMinRight = (Strength/gServerInfo.m_DLPhysiDamageMinConstA)+(Energy/gServerInfo.m_DLPhysiDamageMinConstB);
		lpObj->PhysiDamageMaxRight = (Strength/gServerInfo.m_DLPhysiDamageMaxConstA)+(Energy/gServerInfo.m_DLPhysiDamageMaxConstB);
		lpObj->PhysiDamageMinLeft = (Strength/gServerInfo.m_DLPhysiDamageMinConstA)+(Energy/gServerInfo.m_DLPhysiDamageMinConstB);
		lpObj->PhysiDamageMaxLeft = (Strength/gServerInfo.m_DLPhysiDamageMaxConstA)+(Energy/gServerInfo.m_DLPhysiDamageMaxConstB);
		lpObj->MagicDamageMin = Energy/gServerInfo.m_DLMagicDamageMinConstA;
		lpObj->MagicDamageMax = Energy/gServerInfo.m_DLMagicDamageMaxConstA;
		lpObj->CurseDamageMin = Energy/gServerInfo.m_DLMagicDamageMinConstA;
		lpObj->CurseDamageMax = Energy/gServerInfo.m_DLMagicDamageMaxConstA;
		lpObj->DLDamageMultiplierRate = 200+(Energy/gServerInfo.m_DLDamageMultiplierConstA);
		lpObj->DLDamageMultiplierRate = ((lpObj->DLDamageMultiplierRate>gServerInfo.m_DLDamageMultiplierMaxRate)?gServerInfo.m_DLDamageMultiplierMaxRate:lpObj->DLDamageMultiplierRate);
	}
	else if(lpObj->Class == CLASS_SU)
	{
		lpObj->PhysiDamageMinRight = (Strength+Dexterity)/gServerInfo.m_SUPhysiDamageMinConstA;
		lpObj->PhysiDamageMaxRight = (Strength+Dexterity)/gServerInfo.m_SUPhysiDamageMaxConstA;
		lpObj->PhysiDamageMinLeft = (Strength+Dexterity)/gServerInfo.m_SUPhysiDamageMinConstA;
		lpObj->PhysiDamageMaxLeft = (Strength+Dexterity)/gServerInfo.m_SUPhysiDamageMaxConstA;
		lpObj->MagicDamageMin = Energy/gServerInfo.m_SUMagicDamageMinConstA;
		lpObj->MagicDamageMax = Energy/gServerInfo.m_SUMagicDamageMaxConstA;
		lpObj->CurseDamageMin = Energy/gServerInfo.m_SUMagicDamageMinConstA;
		lpObj->CurseDamageMax = Energy/gServerInfo.m_SUMagicDamageMaxConstA;
	}
	else if(lpObj->Class == CLASS_RF)
	{
		lpObj->PhysiDamageMinRight = (Strength/gServerInfo.m_RFPhysiDamageMinConstA)+(Vitality/gServerInfo.m_RFPhysiDamageMinConstB);
		lpObj->PhysiDamageMaxRight = (Strength/gServerInfo.m_RFPhysiDamageMaxConstA)+(Vitality/gServerInfo.m_RFPhysiDamageMaxConstB);
		lpObj->PhysiDamageMinLeft = (Strength/gServerInfo.m_RFPhysiDamageMinConstA)+(Vitality/gServerInfo.m_RFPhysiDamageMinConstB);
		lpObj->PhysiDamageMaxLeft = (Strength/gServerInfo.m_RFPhysiDamageMaxConstA)+(Vitality/gServerInfo.m_RFPhysiDamageMaxConstB);
		lpObj->MagicDamageMin = Energy/gServerInfo.m_RFMagicDamageMinConstA;
		lpObj->MagicDamageMax = Energy/gServerInfo.m_RFMagicDamageMaxConstA;
		lpObj->CurseDamageMin = Energy/gServerInfo.m_RFMagicDamageMinConstA;
		lpObj->CurseDamageMax = Energy/gServerInfo.m_RFMagicDamageMaxConstA;
		lpObj->RFDamageMultiplierRate[0] = 50+(Vitality/gServerInfo.m_RFDamageMultiplierConstB);
		lpObj->RFDamageMultiplierRate[0] = ((lpObj->RFDamageMultiplierRate[0]>gServerInfo.m_RFDamageMultiplierMaxRate)?gServerInfo.m_RFDamageMultiplierMaxRate:lpObj->RFDamageMultiplierRate[0]);
		lpObj->RFDamageMultiplierRate[1] = 50+(Energy/gServerInfo.m_RFDamageMultiplierConstC);
		lpObj->RFDamageMultiplierRate[1] = ((lpObj->RFDamageMultiplierRate[1]>gServerInfo.m_RFDamageMultiplierMaxRate)?gServerInfo.m_RFDamageMultiplierMaxRate:lpObj->RFDamageMultiplierRate[1]);
		lpObj->RFDamageMultiplierRate[2] = 100+(Dexterity/gServerInfo.m_RFDamageMultiplierConstA)+(Energy/gServerInfo.m_RFDamageMultiplierConstC);
		lpObj->RFDamageMultiplierRate[2] = ((lpObj->RFDamageMultiplierRate[2]>gServerInfo.m_RFDamageMultiplierMaxRate)?gServerInfo.m_RFDamageMultiplierMaxRate:lpObj->RFDamageMultiplierRate[2]);
	}

	if(Right->IsItem() != 0)
	{
		if(Right->m_Index >= GET_ITEM(5,0) && Right->m_Index < GET_ITEM(6,0))
		{
			lpObj->PhysiDamageMinRight += Right->GetDamageMin()/2;
			lpObj->PhysiDamageMaxRight += Right->GetDamageMax()/2;
			lpObj->SkillLongSpearChange = ((Right->m_SkillChange==0)?lpObj->SkillLongSpearChange:1);
		}
		else
		{
			lpObj->PhysiDamageMinRight += Right->GetDamageMin()/1;
			lpObj->PhysiDamageMaxRight += Right->GetDamageMax()/1;
			lpObj->SkillLongSpearChange = ((Right->m_SkillChange==0)?lpObj->SkillLongSpearChange:1);
		}
	}

	if(Left->IsItem() != 0)
	{
		if(Left->m_Index >= GET_ITEM(5,0) && Left->m_Index < GET_ITEM(6,0))
		{
			lpObj->PhysiDamageMinLeft += Left->GetDamageMin()/2;
			lpObj->PhysiDamageMaxLeft += Left->GetDamageMax()/2;
			lpObj->SkillLongSpearChange = ((Left->m_SkillChange==0)?lpObj->SkillLongSpearChange:1);
		}
		else
		{
			lpObj->PhysiDamageMinLeft += Left->GetDamageMin()/1;
			lpObj->PhysiDamageMaxLeft += Left->GetDamageMax()/1;
			lpObj->SkillLongSpearChange = ((Left->m_SkillChange==0)?lpObj->SkillLongSpearChange:1);
		}
	}

	if(lpObj->Class == CLASS_DW)
	{
		lpObj->AttackSuccessRate = ((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_DWAttackSuccessRateConstA)+((Dexterity*gServerInfo.m_DWAttackSuccessRateConstB)/gServerInfo.m_DWAttackSuccessRateConstC)+(Strength/gServerInfo.m_DWAttackSuccessRateConstD);
	}
	else if(lpObj->Class == CLASS_DK)
	{
		lpObj->AttackSuccessRate = ((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_DKAttackSuccessRateConstA)+((Dexterity*gServerInfo.m_DKAttackSuccessRateConstB)/gServerInfo.m_DKAttackSuccessRateConstC)+(Strength/gServerInfo.m_DKAttackSuccessRateConstD);
	}
	else if(lpObj->Class == CLASS_FE)
	{
		lpObj->AttackSuccessRate = ((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_FEAttackSuccessRateConstA)+((Dexterity*gServerInfo.m_FEAttackSuccessRateConstB)/gServerInfo.m_FEAttackSuccessRateConstC)+(Strength/gServerInfo.m_FEAttackSuccessRateConstD);
	}
	else if(lpObj->Class == CLASS_MG)
	{
		lpObj->AttackSuccessRate = ((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_MGAttackSuccessRateConstA)+((Dexterity*gServerInfo.m_MGAttackSuccessRateConstB)/gServerInfo.m_MGAttackSuccessRateConstC)+(Strength/gServerInfo.m_MGAttackSuccessRateConstD);
	}
	else if(lpObj->Class == CLASS_DL)
	{
		lpObj->AttackSuccessRate = ((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_DLAttackSuccessRateConstA)+((Dexterity*gServerInfo.m_DLAttackSuccessRateConstB)/gServerInfo.m_DLAttackSuccessRateConstC)+(Strength/gServerInfo.m_DLAttackSuccessRateConstD)+(Leadership/gServerInfo.m_DLAttackSuccessRateConstE);
	}
	else if(lpObj->Class == CLASS_SU)
	{
		lpObj->AttackSuccessRate = ((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_SUAttackSuccessRateConstA)+((Dexterity*gServerInfo.m_SUAttackSuccessRateConstB)/gServerInfo.m_SUAttackSuccessRateConstC)+(Strength/gServerInfo.m_SUAttackSuccessRateConstD);
	}
	else if(lpObj->Class == CLASS_RF)
	{
		lpObj->AttackSuccessRate = ((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_RFAttackSuccessRateConstA)+((Dexterity*gServerInfo.m_RFAttackSuccessRateConstB)/gServerInfo.m_RFAttackSuccessRateConstC)+(Strength/gServerInfo.m_RFAttackSuccessRateConstD);
	}

	if(lpObj->Class == CLASS_DW)
	{
		lpObj->AttackSuccessRatePvP = (((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_DWAttackSuccessRatePvPConstA)/gServerInfo.m_DWAttackSuccessRatePvPConstB)+((Dexterity*gServerInfo.m_DWAttackSuccessRatePvPConstC)/gServerInfo.m_DWAttackSuccessRatePvPConstD);
	}
	else if(lpObj->Class == CLASS_DK)
	{
		lpObj->AttackSuccessRatePvP = (((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_DKAttackSuccessRatePvPConstA)/gServerInfo.m_DKAttackSuccessRatePvPConstB)+((Dexterity*gServerInfo.m_DKAttackSuccessRatePvPConstC)/gServerInfo.m_DKAttackSuccessRatePvPConstD);
	}
	else if(lpObj->Class == CLASS_FE)
	{
		lpObj->AttackSuccessRatePvP = (((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_FEAttackSuccessRatePvPConstA)/gServerInfo.m_FEAttackSuccessRatePvPConstB)+((Dexterity*gServerInfo.m_FEAttackSuccessRatePvPConstC)/gServerInfo.m_FEAttackSuccessRatePvPConstD);
	}
	else if(lpObj->Class == CLASS_MG)
	{
		lpObj->AttackSuccessRatePvP = (((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_MGAttackSuccessRatePvPConstA)/gServerInfo.m_MGAttackSuccessRatePvPConstB)+((Dexterity*gServerInfo.m_MGAttackSuccessRatePvPConstC)/gServerInfo.m_MGAttackSuccessRatePvPConstD);
	}
	else if(lpObj->Class == CLASS_DL)
	{
		lpObj->AttackSuccessRatePvP = (((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_DLAttackSuccessRatePvPConstA)/gServerInfo.m_DLAttackSuccessRatePvPConstB)+((Dexterity*gServerInfo.m_DLAttackSuccessRatePvPConstC)/gServerInfo.m_DLAttackSuccessRatePvPConstD);
	}
	else if(lpObj->Class == CLASS_SU)
	{
		lpObj->AttackSuccessRatePvP = (((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_SUAttackSuccessRatePvPConstA)/gServerInfo.m_SUAttackSuccessRatePvPConstB)+((Dexterity*gServerInfo.m_SUAttackSuccessRatePvPConstC)/gServerInfo.m_SUAttackSuccessRatePvPConstD);
	}
	else if(lpObj->Class == CLASS_RF)
	{
		lpObj->AttackSuccessRatePvP = (((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_RFAttackSuccessRatePvPConstA)/gServerInfo.m_RFAttackSuccessRatePvPConstB)+((Dexterity*gServerInfo.m_RFAttackSuccessRatePvPConstC)/gServerInfo.m_RFAttackSuccessRatePvPConstD);
	}

	if(lpObj->Class == CLASS_DW)
	{
		lpObj->PhysiSpeed = Dexterity/gServerInfo.m_DWPhysiSpeedConstA;
		lpObj->MagicSpeed = Dexterity/gServerInfo.m_DWMagicSpeedConstA;
	}
	if(lpObj->Class == CLASS_DK)
	{
		lpObj->PhysiSpeed = Dexterity/gServerInfo.m_DKPhysiSpeedConstA;
		lpObj->MagicSpeed = Dexterity/gServerInfo.m_DKMagicSpeedConstA;
	}
	else if(lpObj->Class == CLASS_FE)
	{
		lpObj->PhysiSpeed = Dexterity/gServerInfo.m_FEPhysiSpeedConstA;
		lpObj->MagicSpeed = Dexterity/gServerInfo.m_FEMagicSpeedConstA;
	}
	else if(lpObj->Class == CLASS_MG)
	{
		lpObj->PhysiSpeed = Dexterity/gServerInfo.m_MGPhysiSpeedConstA;
		lpObj->MagicSpeed = Dexterity/gServerInfo.m_MGMagicSpeedConstA;
	}
	else if(lpObj->Class == CLASS_DL)
	{
		lpObj->PhysiSpeed = Dexterity/gServerInfo.m_DLPhysiSpeedConstA;
		lpObj->MagicSpeed = Dexterity/gServerInfo.m_DLMagicSpeedConstA;
	}
	else if(lpObj->Class == CLASS_SU)
	{
		lpObj->PhysiSpeed = Dexterity/gServerInfo.m_SUPhysiSpeedConstA;
		lpObj->MagicSpeed = Dexterity/gServerInfo.m_SUMagicSpeedConstA;
	}
	else if(lpObj->Class == CLASS_RF)
	{
		lpObj->PhysiSpeed = Dexterity/gServerInfo.m_RFPhysiSpeedConstA;
		lpObj->MagicSpeed = Dexterity/gServerInfo.m_RFMagicSpeedConstA;
	}

	lpObj->PhysiSpeed += lpObj->DrinkSpeed;
	lpObj->MagicSpeed += lpObj->DrinkSpeed;

	lpObj->PhysiSpeed += lpObj->EffectOption.AddPhysiSpeed;
	lpObj->MagicSpeed += lpObj->EffectOption.AddMagicSpeed;

	lpObj->PhysiSpeed += (lpObj->PhysiSpeed*lpObj->EffectOption.MulPhysiSpeed)/100;
	lpObj->MagicSpeed += (lpObj->MagicSpeed*lpObj->EffectOption.MulMagicSpeed)/100;

	lpObj->PhysiSpeed += (lpObj->PhysiSpeed*lpObj->EffectOption.DivPhysiSpeed)/100;
	lpObj->MagicSpeed += (lpObj->MagicSpeed*lpObj->EffectOption.DivMagicSpeed)/100;

	bool RightItem = 0;

	if(Right->m_Index >= GET_ITEM(0,0) && Right->m_Index < GET_ITEM(6,0) && Right->m_Index != GET_ITEM(4,7) && Right->m_Index != GET_ITEM(4,15))
	{
		if(Right->m_IsValidItem != 0)
		{
			RightItem = 1;
		}
	}

	bool LeftItem = 0;

	if(Left->m_Index >= GET_ITEM(0,0) && Left->m_Index < GET_ITEM(6,0) && Left->m_Index != GET_ITEM(4,7) && Left->m_Index != GET_ITEM(4,15))
	{
		if(Left->m_IsValidItem != 0)
		{
			LeftItem = 1;
		}
	}

	if(RightItem != 0 && LeftItem != 0)
	{
		lpObj->PhysiSpeed += (Right->m_AttackSpeed+Left->m_AttackSpeed)/2;
		lpObj->MagicSpeed += (Right->m_AttackSpeed+Left->m_AttackSpeed)/2;
	}
	else if(RightItem != 0)
	{
		lpObj->PhysiSpeed += Right->m_AttackSpeed;
		lpObj->MagicSpeed += Right->m_AttackSpeed;
	}
	else if(LeftItem != 0)
	{
		lpObj->PhysiSpeed += Left->m_AttackSpeed;
		lpObj->MagicSpeed += Left->m_AttackSpeed;
	}

	if(Gloves->IsItem() != 0 && Gloves->m_IsValidItem != 0)
	{
		lpObj->PhysiSpeed += Gloves->m_AttackSpeed;
		lpObj->MagicSpeed += Gloves->m_AttackSpeed;
	}

	if(Helper->IsItem() != 0 && Helper->m_IsValidItem != 0)
	{
		lpObj->PhysiSpeed += Helper->m_AttackSpeed;
		lpObj->MagicSpeed += Helper->m_AttackSpeed;
	}

	if(Amulet->IsItem() != 0 && Amulet->m_IsValidItem != 0)
	{
		lpObj->PhysiSpeed += Amulet->m_AttackSpeed;
		lpObj->MagicSpeed += Amulet->m_AttackSpeed;
	}

	if(lpObj->Class == CLASS_DW)
	{
		lpObj->DefenseSuccessRate = Dexterity/gServerInfo.m_DWDefenseSuccessRateConstA;
	}
	else if(lpObj->Class == CLASS_DK)
	{
		lpObj->DefenseSuccessRate = Dexterity/gServerInfo.m_DKDefenseSuccessRateConstA;
	}
	else if(lpObj->Class == CLASS_FE)
	{
		lpObj->DefenseSuccessRate = Dexterity/gServerInfo.m_FEDefenseSuccessRateConstA;
	}
	else if(lpObj->Class == CLASS_MG)
	{
		lpObj->DefenseSuccessRate = Dexterity/gServerInfo.m_MGDefenseSuccessRateConstA;
	}
	else if(lpObj->Class == CLASS_DL)
	{
		lpObj->DefenseSuccessRate = Dexterity/gServerInfo.m_DLDefenseSuccessRateConstA;
	}
	else if(lpObj->Class == CLASS_SU)
	{
		lpObj->DefenseSuccessRate = Dexterity/gServerInfo.m_SUDefenseSuccessRateConstA;
	}
	else if(lpObj->Class == CLASS_RF)
	{
		lpObj->DefenseSuccessRate = Dexterity/gServerInfo.m_RFDefenseSuccessRateConstA;
	}

	lpObj->DefenseSuccessRate += lpObj->Inventory[1].GetDefenseSuccessRate();

	lpObj->DefenseSuccessRate += lpObj->Inventory[2].GetDefenseSuccessRate();

	lpObj->DefenseSuccessRate += lpObj->Inventory[3].GetDefenseSuccessRate();

	lpObj->DefenseSuccessRate += lpObj->Inventory[4].GetDefenseSuccessRate();

	lpObj->DefenseSuccessRate += lpObj->Inventory[5].GetDefenseSuccessRate();

	lpObj->DefenseSuccessRate += lpObj->Inventory[6].GetDefenseSuccessRate();

	lpObj->DefenseSuccessRate += lpObj->Inventory[7].GetDefenseSuccessRate();

	if(lpObj->Class == CLASS_DW)
	{
		lpObj->DefenseSuccessRatePvP = (((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_DWDefenseSuccessRatePvPConstA)/gServerInfo.m_DWDefenseSuccessRatePvPConstB)+(Dexterity/gServerInfo.m_DWDefenseSuccessRatePvPConstC);
	}
	else if(lpObj->Class == CLASS_DK)
	{
		lpObj->DefenseSuccessRatePvP = (((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_DKDefenseSuccessRatePvPConstA)/gServerInfo.m_DKDefenseSuccessRatePvPConstB)+(Dexterity/gServerInfo.m_DKDefenseSuccessRatePvPConstC);
	}
	else if(lpObj->Class == CLASS_FE)
	{
		lpObj->DefenseSuccessRatePvP = (((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_FEDefenseSuccessRatePvPConstA)/gServerInfo.m_FEDefenseSuccessRatePvPConstB)+(Dexterity/gServerInfo.m_FEDefenseSuccessRatePvPConstC);
	}
	else if(lpObj->Class == CLASS_MG)
	{
		lpObj->DefenseSuccessRatePvP = (((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_MGDefenseSuccessRatePvPConstA)/gServerInfo.m_MGDefenseSuccessRatePvPConstB)+(Dexterity/gServerInfo.m_MGDefenseSuccessRatePvPConstC);
	}
	else if(lpObj->Class == CLASS_DL)
	{
		lpObj->DefenseSuccessRatePvP = (((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_DLDefenseSuccessRatePvPConstA)/gServerInfo.m_DLDefenseSuccessRatePvPConstB)+(Dexterity/gServerInfo.m_DLDefenseSuccessRatePvPConstC);
	}
	else if(lpObj->Class == CLASS_SU)
	{
		lpObj->DefenseSuccessRatePvP = (((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_SUDefenseSuccessRatePvPConstA)/gServerInfo.m_SUDefenseSuccessRatePvPConstB)+(Dexterity/gServerInfo.m_SUDefenseSuccessRatePvPConstC);
	}
	else if(lpObj->Class == CLASS_RF)
	{
		lpObj->DefenseSuccessRatePvP = (((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_RFDefenseSuccessRatePvPConstA)/gServerInfo.m_RFDefenseSuccessRatePvPConstB)+(Dexterity/gServerInfo.m_RFDefenseSuccessRatePvPConstC);
	}

	int LastItemIndex = -1;

	for(int n=2;n <= 6;n++)
	{
		if(n == 2 && lpObj->Class == CLASS_MG)
		{
			continue;
		}

		if(n == 5 && lpObj->Class == CLASS_RF)
		{
			continue;
		}

		if(lpObj->Inventory[n].IsItem() == 0 || lpObj->Inventory[n].m_IsValidItem == 0 || (LastItemIndex != -1 && (lpObj->Inventory[n].m_Index%MAX_ITEM_TYPE) != LastItemIndex))
		{
			lpObj->ArmorSetBonus = 0;
			break;
		}
		else
		{
			lpObj->ArmorSetBonus = 1;
			LastItemIndex = lpObj->Inventory[n].m_Index%MAX_ITEM_TYPE; 
		}
	}

	int Level11Count = 0;
	int Level10Count = 0;
	int Level12Count = 0;
	int Level13Count = 0;
	int Level14Count = 0;
	int Level15Count = 0;

	if(lpObj->ArmorSetBonus != 0)
	{
		for(int n=2;n <= 6;n++)
		{
			if(n == 2 && lpObj->Class == CLASS_MG)
			{
				Level15Count++;
				continue;
			}

			if(n == 5 && lpObj->Class == CLASS_RF)
			{
				Level15Count++;
				continue;
			}

			if(lpObj->Inventory[n].m_Level == 10)
			{
				Level10Count++;
			}
			else if(lpObj->Inventory[n].m_Level == 11)
			{
				Level11Count++;
			}
			else if(lpObj->Inventory[n].m_Level == 12)
			{
				Level12Count++;
			}
			else if(lpObj->Inventory[n].m_Level == 13)
			{
				Level13Count++;
			}
			else if(lpObj->Inventory[n].m_Level == 14)
			{
				Level14Count++;
			}
			else if(lpObj->Inventory[n].m_Level == 15)
			{
				Level15Count++;
			}
		}

		lpObj->DefenseSuccessRate += (lpObj->DefenseSuccessRate*10)/100;
	}

	if(lpObj->Class == CLASS_DW)
	{
		lpObj->Defense = Dexterity/gServerInfo.m_DWDefenseConstA;
	}
	else if(lpObj->Class == CLASS_DK)
	{
		lpObj->Defense = Dexterity/gServerInfo.m_DKDefenseConstA;
	}
	else if(lpObj->Class == CLASS_FE)
	{
		lpObj->Defense = Dexterity/gServerInfo.m_FEDefenseConstA;
	}
	else if(lpObj->Class == CLASS_MG)
	{
		lpObj->Defense = Dexterity/gServerInfo.m_MGDefenseConstA;
	}
	else if(lpObj->Class == CLASS_DL)
	{
		lpObj->Defense = Dexterity/gServerInfo.m_DLDefenseConstA;
	}
	else if(lpObj->Class == CLASS_SU)
	{
		lpObj->Defense = Dexterity/gServerInfo.m_SUDefenseConstA;
	}
	else if(lpObj->Class == CLASS_RF)
	{
		lpObj->Defense = Dexterity/gServerInfo.m_RFDefenseConstA;
	}

	lpObj->Defense += lpObj->Inventory[1].GetDefense();

	lpObj->Defense += lpObj->Inventory[2].GetDefense();

	lpObj->Defense += lpObj->Inventory[3].GetDefense();

	lpObj->Defense += lpObj->Inventory[4].GetDefense();

	lpObj->Defense += lpObj->Inventory[5].GetDefense();

	lpObj->Defense += lpObj->Inventory[6].GetDefense();

	lpObj->Defense += lpObj->Inventory[7].GetDefense();

	if(Helper->IsItem() != 0 && Helper->m_IsValidItem != 0 && Helper->m_Durability != 0)
	{
		if(Helper->m_Index == GET_ITEM(13,4))
		{
			lpObj->Defense += (Dexterity/20)+(Helper->m_PetItemLevel*2)+5;
		}
	}

	if(lpObj->ArmorSetBonus != 0 && (Level15Count+Level14Count+Level13Count+Level12Count+Level11Count+Level10Count) >= 5)
	{
		if(Level15Count == 5)
		{
			lpObj->Defense += (lpObj->Defense*30)/100;
		}
		else if(Level14Count == 5 || (Level14Count+Level15Count) == 5)
		{
			lpObj->Defense += (lpObj->Defense*25)/100;
		}
		else if(Level13Count == 5 || (Level13Count+Level14Count+Level15Count) == 5)
		{
			lpObj->Defense += (lpObj->Defense*20)/100;
		}
		else if(Level12Count == 5 || (Level12Count+Level13Count+Level14Count+Level15Count) == 5)
		{
			lpObj->Defense += (lpObj->Defense*15)/100;
		}
		else if(Level11Count == 5 || (Level11Count+Level12Count+Level13Count+Level14Count+Level15Count) == 5)
		{
			lpObj->Defense += (lpObj->Defense*10)/100;
		}
		else if(Level10Count == 5 || (Level10Count+Level11Count+Level12Count+Level13Count+Level14Count+Level15Count) == 5)
		{
			lpObj->Defense += (lpObj->Defense*5)/100;
		}
	}

	gDarkSpirit[lpObj->Index].Set(lpObj->Index,&lpObj->Inventory[1]);

	lpObj->Resistance[0] = GET_MAX_RESISTANCE(Amulet->m_Resistance[0],Ring1->m_Resistance[0],Ring2->m_Resistance[0]);

	lpObj->Resistance[1] = GET_MAX_RESISTANCE(Amulet->m_Resistance[1],Ring1->m_Resistance[1],Ring2->m_Resistance[1]);

	lpObj->Resistance[2] = GET_MAX_RESISTANCE(Amulet->m_Resistance[2],Ring1->m_Resistance[2],Ring2->m_Resistance[2]);

	lpObj->Resistance[3] = GET_MAX_RESISTANCE(Amulet->m_Resistance[3],Ring1->m_Resistance[3],Ring2->m_Resistance[3]);

	lpObj->Resistance[4] = GET_MAX_RESISTANCE(Amulet->m_Resistance[4],Ring1->m_Resistance[4],Ring2->m_Resistance[4]);

	lpObj->Resistance[5] = GET_MAX_RESISTANCE(Amulet->m_Resistance[5],Ring1->m_Resistance[5],Ring2->m_Resistance[5]);

	lpObj->Resistance[6] = GET_MAX_RESISTANCE(Amulet->m_Resistance[6],Ring1->m_Resistance[6],Ring2->m_Resistance[6]);

	#if(GAMESERVER_UPDATE>=701)

	if(lpObj->Class == CLASS_DW)
	{
		lpObj->ElementalAttribute = 0;

		lpObj->ElementalDefense = Dexterity/gServerInfo.m_DWElementalDefenseConstA;

		lpObj->ElementalDamageMin = Energy/gServerInfo.m_DWElementalDamageMinConstA;

		lpObj->ElementalDamageMax = Energy/gServerInfo.m_DWElementalDamageMaxConstA;

		lpObj->ElementalAttackSuccessRate = ((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_DWElementalAttackSuccessRateConstA)+(Strength/gServerInfo.m_DWElementalAttackSuccessRateConstB)+((Dexterity*gServerInfo.m_DWElementalAttackSuccessRateConstC)/gServerInfo.m_DWElementalAttackSuccessRateConstD);

		lpObj->ElementalDefenseSuccessRate = Dexterity/gServerInfo.m_DWElementalDefenseSuccessRateConstA;
	}
	if(lpObj->Class == CLASS_DK)
	{
		lpObj->ElementalAttribute = 0;

		lpObj->ElementalDefense = Dexterity/gServerInfo.m_DKElementalDefenseConstA;

		lpObj->ElementalDamageMin = Strength/gServerInfo.m_DKElementalDamageMinConstA;

		lpObj->ElementalDamageMax = Strength/gServerInfo.m_DKElementalDamageMaxConstA;

		lpObj->ElementalAttackSuccessRate = ((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_DKElementalAttackSuccessRateConstA)+(Strength/gServerInfo.m_DKElementalAttackSuccessRateConstB)+((Dexterity*gServerInfo.m_DKElementalAttackSuccessRateConstC)/gServerInfo.m_DKElementalAttackSuccessRateConstD);

		lpObj->ElementalDefenseSuccessRate = Dexterity/gServerInfo.m_DKElementalDefenseSuccessRateConstA;
	}
	else if(lpObj->Class == CLASS_FE)
	{
		lpObj->ElementalAttribute = 0;

		lpObj->ElementalDefense = Dexterity/gServerInfo.m_FEElementalDefenseConstA;

		lpObj->ElementalDamageMin = (Strength/gServerInfo.m_FEElementalDamageMinConstA)+(Dexterity/gServerInfo.m_FEElementalDamageMinConstB);

		lpObj->ElementalDamageMax = (Strength/gServerInfo.m_FEElementalDamageMaxConstA)+(Dexterity/gServerInfo.m_FEElementalDamageMaxConstB);

		lpObj->ElementalAttackSuccessRate = ((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_FEElementalAttackSuccessRateConstA)+(Strength/gServerInfo.m_FEElementalAttackSuccessRateConstB)+((Dexterity*gServerInfo.m_FEElementalAttackSuccessRateConstC)/gServerInfo.m_FEElementalAttackSuccessRateConstD);

		lpObj->ElementalDefenseSuccessRate = Dexterity/gServerInfo.m_FEElementalDefenseSuccessRateConstA;
	}
	else if(lpObj->Class == CLASS_MG)
	{
		lpObj->ElementalAttribute = 0;

		lpObj->ElementalDefense = Dexterity/gServerInfo.m_MGElementalDefenseConstA;

		lpObj->ElementalDamageMin = (Strength/gServerInfo.m_MGElementalDamageMinConstA)+(Energy/gServerInfo.m_MGElementalDamageMinConstB);

		lpObj->ElementalDamageMax = (Strength/gServerInfo.m_MGElementalDamageMaxConstA)+(Energy/gServerInfo.m_MGElementalDamageMaxConstB);

		lpObj->ElementalAttackSuccessRate = ((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_MGElementalAttackSuccessRateConstA)+(Strength/gServerInfo.m_MGElementalAttackSuccessRateConstB)+((Dexterity*gServerInfo.m_MGElementalAttackSuccessRateConstC)/gServerInfo.m_MGElementalAttackSuccessRateConstD);

		lpObj->ElementalDefenseSuccessRate = Dexterity/gServerInfo.m_MGElementalDefenseSuccessRateConstA;
	}
	else if(lpObj->Class == CLASS_DL)
	{
		lpObj->ElementalAttribute = 0;

		lpObj->ElementalDefense = Dexterity/gServerInfo.m_DLElementalDefenseConstA;

		lpObj->ElementalDamageMin = (Strength/gServerInfo.m_DLElementalDamageMinConstA)+(Energy/gServerInfo.m_DLElementalDamageMinConstB);

		lpObj->ElementalDamageMax = (Strength/gServerInfo.m_DLElementalDamageMaxConstA)+(Energy/gServerInfo.m_DLElementalDamageMaxConstB);

		lpObj->ElementalAttackSuccessRate = ((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_DLElementalAttackSuccessRateConstA)+(Strength/gServerInfo.m_DLElementalAttackSuccessRateConstB)+((Dexterity*gServerInfo.m_DLElementalAttackSuccessRateConstC)/gServerInfo.m_DLElementalAttackSuccessRateConstD);

		lpObj->ElementalDefenseSuccessRate = Dexterity/gServerInfo.m_DLElementalDefenseSuccessRateConstA;
	}
	else if(lpObj->Class == CLASS_SU)
	{
		lpObj->ElementalAttribute = 0;

		lpObj->ElementalDefense = Dexterity/gServerInfo.m_SUElementalDefenseConstA;

		lpObj->ElementalDamageMin = Energy/gServerInfo.m_SUElementalDamageMinConstA;

		lpObj->ElementalDamageMax = Energy/gServerInfo.m_SUElementalDamageMaxConstA;

		lpObj->ElementalAttackSuccessRate = ((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_SUElementalAttackSuccessRateConstA)+(Strength/gServerInfo.m_SUElementalAttackSuccessRateConstB)+((Dexterity*gServerInfo.m_SUElementalAttackSuccessRateConstC)/gServerInfo.m_SUElementalAttackSuccessRateConstD);

		lpObj->ElementalDefenseSuccessRate = Dexterity/gServerInfo.m_SUElementalDefenseSuccessRateConstA;
	}
	else if(lpObj->Class == CLASS_RF)
	{
		lpObj->ElementalAttribute = 0;

		lpObj->ElementalDefense = Dexterity/gServerInfo.m_RFElementalDefenseConstA;

		lpObj->ElementalDamageMin = (Strength/gServerInfo.m_RFElementalDamageMinConstA)+(Vitality/gServerInfo.m_RFElementalDamageMinConstB);

		lpObj->ElementalDamageMax = (Strength/gServerInfo.m_RFElementalDamageMaxConstA)+(Vitality/gServerInfo.m_RFElementalDamageMaxConstB);

		lpObj->ElementalAttackSuccessRate = ((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_RFElementalAttackSuccessRateConstA)+(Strength/gServerInfo.m_RFElementalAttackSuccessRateConstB)+((Dexterity*gServerInfo.m_RFElementalAttackSuccessRateConstC)/gServerInfo.m_RFElementalAttackSuccessRateConstD);

		lpObj->ElementalDefenseSuccessRate = Dexterity/gServerInfo.m_RFElementalDefenseSuccessRateConstA;
	}

	if(Pentagram->IsItem() != 0 && Pentagram->IsPentagramItem() != 0 && Pentagram->m_IsValidItem != 0)
	{
		lpObj->ElementalAttribute = Pentagram->m_SocketOptionBonus%16;

		lpObj->ElementalDefense += Pentagram->m_Defense;

		lpObj->ElementalDamageMin += Pentagram->m_DamageMin;

		lpObj->ElementalDamageMax += Pentagram->m_DamageMax;
	}

	#endif

	gObjCalcExperience(lpObj);
	gItemOption.CalcItemCommonOption(lpObj,0);
	gSetItemOption.CalcSetItemOption(lpObj,0);
	gJewelOfHarmonyOption.CalcJewelOfHarmonyOption(lpObj,0);
	g380ItemOption.Calc380ItemOption(lpObj,0);
	gSocketItemOption.CalcSocketItemOption(lpObj,0);
	gMasterSkillTree.CalcMasterSkillTreeOption(lpObj,0);
	#if(GAMESERVER_UPDATE>=701)
	gPentagramSystem.CalcPentagramOption(lpObj,0);
	gPentagramSystem.CalcPentagramJewelOption(lpObj,0);
	#endif
	#if(GAMESERVER_UPDATE>=802)
	this->CharacterCalcPvPAccessoryOption(lpObj);
	#endif
	#if(GAMESERVER_UPDATE>=803)
	gMuunSystem.CalcMuunOption(lpObj,0);
	#endif

	if(Right->m_Index >= GET_ITEM(4,0) && Right->m_Index < GET_ITEM(5,0) && Right->m_Index != GET_ITEM(4,15) && Right->m_Slot == 0)
	{
		if(Left->m_Index == GET_ITEM(4,7) && Left->m_Level > 0)
		{
			lpObj->PhysiDamageMinRight += ((lpObj->PhysiDamageMinRight*((Left->m_Level*2)+1))/100)+1;
			lpObj->PhysiDamageMaxRight += ((lpObj->PhysiDamageMinRight*((Left->m_Level*2)+1))/100)+1;
		}
	}
	else if(Left->m_Index >= GET_ITEM(4,0) && Left->m_Index < GET_ITEM(5,0) && Left->m_Index != GET_ITEM(4,7) && Left->m_Slot == 1)
	{
		if(Right->m_Index == GET_ITEM(4,15) && Right->m_Level > 0)
		{
			lpObj->PhysiDamageMinLeft += ((lpObj->PhysiDamageMinLeft*((Right->m_Level*2)+1))/100)+1;
			lpObj->PhysiDamageMaxLeft += ((lpObj->PhysiDamageMinLeft*((Right->m_Level*2)+1))/100)+1;
		}
	}

	if(lpObj->Class == CLASS_DK || lpObj->Class == CLASS_MG || lpObj->Class == CLASS_DL || lpObj->Class == CLASS_RF)
	{
		if(Right->IsItem() != 0 && Left->IsItem() != 0)
		{
			if(Right->m_Index >= GET_ITEM(0,0) && Right->m_Index < GET_ITEM(4,0) && Left->m_Index >= GET_ITEM(0,0) && Left->m_Index < GET_ITEM(4,0))
			{
				if(lpObj->Class == CLASS_RF)
				{
					lpObj->PhysiDamageMinRight = (lpObj->PhysiDamageMinRight*60)/100;
					lpObj->PhysiDamageMaxRight = (lpObj->PhysiDamageMaxRight*65)/100;
					lpObj->PhysiDamageMinLeft = (lpObj->PhysiDamageMinLeft*60)/100;
					lpObj->PhysiDamageMaxLeft = (lpObj->PhysiDamageMaxLeft*65)/100;
				}
				else
				{
					lpObj->PhysiDamageMinRight = (lpObj->PhysiDamageMinRight*55)/100;
					lpObj->PhysiDamageMaxRight = (lpObj->PhysiDamageMaxRight*55)/100;
					lpObj->PhysiDamageMinLeft = (lpObj->PhysiDamageMinLeft*55)/100;
					lpObj->PhysiDamageMaxLeft = (lpObj->PhysiDamageMaxLeft*55)/100;
				}
			}
		}
	}

	lpObj->MaxLife = gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].MaxLife;
	lpObj->MaxMana = gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].MaxMana;

	lpObj->MaxLife += gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].LevelLife*(lpObj->Level-1);
	lpObj->MaxMana += gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].LevelMana*(lpObj->Level-1);

	if(gMasterSkillTree.CheckMasterLevel(lpObj) != 0)
	{
		lpObj->MaxLife += gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].LevelLife*lpObj->MasterLevel;
		lpObj->MaxMana += gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].LevelMana*lpObj->MasterLevel;
	}

	lpObj->MaxLife += ((lpObj->Vitality-gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].Vitality)+lpObj->AddVitality)*lpObj->VitalityToLife;
	lpObj->MaxMana += ((lpObj->Energy-gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].Energy)+lpObj->AddEnergy)*lpObj->EnergyToMana;

	this->CharacterCalcBP(lpObj);

	this->CharacterCalcSD(lpObj);

	lpObj->AddLife += lpObj->EffectOption.AddMaxHP;
	lpObj->AddLife += (int)(lpObj->MaxLife*lpObj->EffectOption.MulMaxHP)/100;
	lpObj->AddLife -= (int)(lpObj->MaxLife*lpObj->EffectOption.DivMaxHP)/100;

	lpObj->AddMana += lpObj->EffectOption.AddMaxMP;
	lpObj->AddMana += (int)(lpObj->MaxMana*lpObj->EffectOption.MulMaxMP)/100;
	lpObj->AddMana -= (int)(lpObj->MaxMana*lpObj->EffectOption.DivMaxMP)/100;

	lpObj->AddBP += lpObj->EffectOption.AddMaxBP;
	lpObj->AddBP += (int)(lpObj->MaxBP*lpObj->EffectOption.MulMaxBP)/100;
	lpObj->AddBP -= (int)(lpObj->MaxBP*lpObj->EffectOption.DivMaxBP)/100;

	lpObj->AddShield += lpObj->EffectOption.AddMaxSD;
	lpObj->AddShield += (int)(lpObj->MaxShield*lpObj->EffectOption.MulMaxSD)/100;
	lpObj->AddShield -= (int)(lpObj->MaxShield*lpObj->EffectOption.DivMaxSD)/100;

	//Sistema HP bonus
	lpObj->AddLife += ((lpObj->MasterReset*gServerInfo.m_HpBonus[lpObj->AccountLevel]) > gServerInfo.m_HpBonusMax)?gServerInfo.m_HpBonusMax:(lpObj->MasterReset*gServerInfo.m_HpBonus[lpObj->AccountLevel]);
	//FIm

	lpObj->Life = ((lpObj->MaxLife+lpObj->AddLife)*TotalHP)/100;

	lpObj->Life = ((lpObj->Life>(lpObj->MaxLife+lpObj->AddLife))?(lpObj->MaxLife+lpObj->AddLife):lpObj->Life);

	lpObj->Shield = (int)(((lpObj->MaxShield+lpObj->AddShield)*TotalSD)/100);

	lpObj->Shield = (int)(((lpObj->Shield>(lpObj->MaxShield+lpObj->AddShield))?(lpObj->MaxShield+lpObj->AddShield):lpObj->Shield));

	lpObj->Mana = ((lpObj->MaxMana+lpObj->AddMana)*TotalMP)/100;

	lpObj->Mana = ((lpObj->Mana>(lpObj->MaxMana+lpObj->AddMana))?(lpObj->MaxMana+lpObj->AddMana):lpObj->Mana);

	lpObj->BP = (int)(((lpObj->MaxBP+lpObj->AddBP)*TotalBP)/100);

	lpObj->BP = (int)(((lpObj->BP>(lpObj->MaxBP+lpObj->AddBP))?(lpObj->MaxBP+lpObj->AddBP):lpObj->BP));

	#if(GAMESERVER_EXTRA==0)

	GCLifeSend(aIndex,0xFE,(int)(lpObj->MaxLife+lpObj->AddLife),(lpObj->MaxShield+lpObj->AddShield));
	GCLifeSend(aIndex,0xFF,(int)lpObj->Life,lpObj->Shield);

	GCManaSend(aIndex,0xFE,(int)(lpObj->MaxMana+lpObj->AddMana),(lpObj->MaxBP+lpObj->AddBP));
	GCManaSend(aIndex,0xFF,(int)lpObj->Mana,lpObj->BP);

	#else

	GCNewCharacterCalcSend(lpObj);

	#endif

	#if(GAMESERVER_UPDATE>=701)

	GCCharacterAttackSpeedSend(lpObj);

	#endif
}

bool CObjectManager::CharacterInfoSet(BYTE* aRecv,int aIndex) // OK
{
	SDHP_CHARACTER_INFO_RECV* lpMsg = (SDHP_CHARACTER_INFO_RECV*)aRecv;

	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Connected == OBJECT_OFFLINE || lpObj->Connected == OBJECT_CONNECTED)
	{
		return 0;
	}

	if(lpMsg->Class != DB_CLASS_DW && lpMsg->Class != DB_CLASS_SM && lpMsg->Class != DB_CLASS_GM && lpMsg->Class != DB_CLASS_DK && lpMsg->Class != DB_CLASS_BK && lpMsg->Class != DB_CLASS_BM && lpMsg->Class != DB_CLASS_FE && lpMsg->Class != DB_CLASS_ME && lpMsg->Class != DB_CLASS_HE && lpMsg->Class != DB_CLASS_MG && lpMsg->Class != DB_CLASS_DM && lpMsg->Class != DB_CLASS_DL && lpMsg->Class != DB_CLASS_LE && lpMsg->Class != DB_CLASS_SU && lpMsg->Class != DB_CLASS_BS && lpMsg->Class != DB_CLASS_DS && lpMsg->Class != DB_CLASS_RF && lpMsg->Class != DB_CLASS_FM)
	{
		return 0;
	}

	gObjCharZeroSet(lpObj->Index);

	gChaosBox.ChaosBoxInit(lpObj);

	lpObj->PosNum = -1;
	lpObj->Level = lpMsg->Level;
	lpObj->LevelUpPoint = lpMsg->LevelUpPoint;
	lpObj->DBClass = lpMsg->Class;
	lpObj->Class = lpMsg->Class/16;
	lpObj->ChangeUp = lpMsg->Class%16;
	#if(GAMESERVER_UPDATE>=602)
	lpObj->ExtInventory = lpMsg->ExtInventory;
	//lpObj->ExtWarehouse = lpMsg->ExtWarehouse;
	#endif
	lpObj->Reset = lpMsg->Reset;
	lpObj->MasterReset = lpMsg->MasterReset;
	#if(GAMESERVER_UPDATE>=801)
	lpObj->UseGuildMatching = lpMsg->UseGuildMatching;
	lpObj->UseGuildMatchingJoin = lpMsg->UseGuildMatchingJoin;
	#endif

	memcpy(lpObj->Name,lpMsg->name,sizeof(lpObj->Name));

	lpObj->Map = lpMsg->Map;
	lpObj->X = lpMsg->X;
	lpObj->Y = lpMsg->Y;
	lpObj->StartX = lpMsg->X;
	lpObj->StartY = lpMsg->Y;

	bool result = 0;

	int gate,map,x,y,dir,level;

	if(MAP_RANGE(lpObj->Map) == 0)
	{
		lpObj->Map = MAP_LORENCIA;
		gMap[lpObj->Map].GetMapPos(lpObj->Map,&lpObj->X,&lpObj->Y);
	}

	if(lpObj->Level < 6 && lpObj->Class == CLASS_DW && gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) == 0)
	{
		result = gGate.GetGate(17,&gate,&map,&x,&y,&dir,&level);
	}
	else if(lpObj->Level < 6 && lpObj->Class == CLASS_DK && gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) == 0)
	{
		result = gGate.GetGate(17,&gate,&map,&x,&y,&dir,&level);
	}
	else if(lpObj->Level < 6 && lpObj->Class == CLASS_FE && gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) == 0)
	{
		result = gGate.GetGate(27,&gate,&map,&x,&y,&dir,&level);
	}
	else if(lpObj->Level < 6 && lpObj->Class == CLASS_MG && gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) == 0)
	{
		result = gGate.GetGate(17,&gate,&map,&x,&y,&dir,&level);
	}
	else if(lpObj->Level < 6 && lpObj->Class == CLASS_DL && gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) == 0)
	{
		result = gGate.GetGate(17,&gate,&map,&x,&y,&dir,&level);
	}
	else if(lpObj->Level < 6 && lpObj->Class == CLASS_SU && gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) == 0)
	{
		result = gGate.GetGate(267,&gate,&map,&x,&y,&dir,&level);
	}
	else if(lpObj->Level < 6 && lpObj->Class == CLASS_RF && gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) == 0)
	{
		result = gGate.GetGate(17,&gate,&map,&x,&y,&dir,&level);
	}
	else if(CA_MAP_RANGE(lpObj->Map) != 0)
	{
		result = gGate.GetGate(17,&gate,&map,&x,&y,&dir,&level);
	}
	else if(DS_MAP_RANGE(lpObj->Map) != 0)
	{
		result = gGate.GetGate(27,&gate,&map,&x,&y,&dir,&level);
	}
	else if(BC_MAP_RANGE(lpObj->Map) != 0)
	{
		result = gGate.GetGate(22,&gate,&map,&x,&y,&dir,&level);
	}
	else if(CC_MAP_RANGE(lpObj->Map) != 0)
	{
		result = gGate.GetGate(17,&gate,&map,&x,&y,&dir,&level);
	}
	else if(lpObj->Map == MAP_CASTLE_SIEGE && gCastleSiegeSync.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)
	{
		result = gGate.GetGate(100,&gate,&map,&x,&y,&dir,&level);
	}
	else if(lpObj->Map == MAP_LAND_OF_TRIALS)
	{
		result = gGate.GetGate(106,&gate,&map,&x,&y,&dir,&level);
	}
	else if(IT_MAP_RANGE(lpObj->Map) != 0)
	{
		result = gGate.GetGate(267,&gate,&map,&x,&y,&dir,&level);
	}
	else if(lpObj->Map == MAP_KANTURU3)
	{
		result = gGate.GetGate(139,&gate,&map,&x,&y,&dir,&level);
	}
	else if(lpObj->Map == MAP_RAKLION2)
	{
		result = gGate.GetGate(286,&gate,&map,&x,&y,&dir,&level);
	}
	else if(DA_MAP_RANGE(lpObj->Map) != 0)
	{
		result = gGate.GetGate(17,&gate,&map,&x,&y,&dir,&level);
	}
	else if(lpObj->Map == MAP_SANTA_TOWN)
	{
		result = gGate.GetGate(22,&gate,&map,&x,&y,&dir,&level);
	}
	else if(DG_MAP_RANGE(lpObj->Map) != 0)
	{
		result = gGate.GetGate(267,&gate,&map,&x,&y,&dir,&level);
	}
	else if(IG_MAP_RANGE(lpObj->Map) != 0)
	{
		result = gGate.GetGate(22,&gate,&map,&x,&y,&dir,&level);
	}

	if(result != 0)
	{
		lpObj->Map = map;
		lpObj->X = x;
		lpObj->Y = y;
		lpObj->Dir = dir;
	}

	if(lpObj->MapServerMoveRequest != 0)
	{
		if(gMapServerManager.CheckMapServerMove(aIndex,lpObj->DestMap,lpObj->LastServerCode) == gServerInfo.m_ServerCode)
		{
			lpObj->Map = (BYTE)lpObj->DestMap;
			lpObj->X = lpObj->DestX;
			lpObj->Y = lpObj->DestY;
			lpObj->StartX = lpObj->DestX;
			lpObj->StartY = lpObj->DestY;
		}
		else
		{
			gObjDel(aIndex);
			return 0;
		}
	}

	if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,4) != 0 || gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,8) != 0)
	{
		gMap[lpObj->Map].GetMapPos(lpObj->Map,&lpObj->X,&lpObj->Y);
	}

	gMap[lpObj->Map].SetStandAttr(lpObj->X,lpObj->Y);

	lpObj->TX = lpObj->X;
	lpObj->TY = lpObj->Y;
	lpObj->OldX = lpObj->X;
	lpObj->OldY = lpObj->Y;
	lpObj->Experience = lpMsg->Experience;
	lpObj->Strength = lpMsg->Strength;
	lpObj->Dexterity = lpMsg->Dexterity;
	lpObj->Vitality = lpMsg->Vitality;
	lpObj->Energy = lpMsg->Energy;
	lpObj->Leadership = lpMsg->Leadership;
	lpObj->Life = (float)lpMsg->Life;
	lpObj->MaxLife = (float)lpMsg->MaxLife;
	lpObj->Mana = (float)lpMsg->Mana;
	lpObj->MaxMana = (float)lpMsg->MaxMana;
	lpObj->BP = lpMsg->BP;
	lpObj->MaxBP = lpMsg->MaxBP;
	lpObj->Shield = lpMsg->Shield;
	lpObj->MaxShield = lpMsg->MaxShield;
	lpObj->Money = lpMsg->Money;
	lpObj->FruitAddPoint = lpMsg->FruitAddPoint;
	lpObj->FruitSubPoint = lpMsg->FruitSubPoint;
	lpObj->VitalityToLife = gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].VitalityToLife;
	lpObj->EnergyToMana = gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].EnergyToMana;
	lpObj->PKCount = (char)lpMsg->PKCount;
	lpObj->PKLevel = (char)lpMsg->PKLevel;
	lpObj->PKTime = lpMsg->PKTime;
	lpObj->MaxRegenTime = 4000;
	lpObj->MoveSpeed = 400;

	lpObj->Kills = lpMsg->Kills;
	lpObj->Deads = lpMsg->Deads;

	if(lpObj->PKLevel == 3)
	{
		lpObj->PKCount = 0;
		lpObj->PKTime = 0;
	}

	memcpy(lpObj->Quest,lpMsg->Quest,sizeof(lpObj->Quest));

	gObjSetInventory1Pointer(lpObj);

	lpObj->GuildNumber = 0;
	lpObj->Guild = 0;

	for(int n=0;n < MAX_SKILL_LIST;n++)
	{
		CSkill skill;

		lpObj->Skill[n].Clear();

		if(gSkillManager.ConvertSkillByte(&skill,lpMsg->Skill[n]) == 0)
		{
			continue;
		}

		gSkillManager.AddSkill(lpObj,skill.m_index,skill.m_level);
	}

	if(lpObj->Class == CLASS_DW)
	{
		gSkillManager.AddSkill(lpObj,SKILL_ENERGY_BALL,0);
		gSkillManager.AddSkill(lpObj,SKILL_MANA_GLAIVE,0);
	}
	else if(lpObj->Class == CLASS_DK)
	{
		gSkillManager.AddSkill(lpObj,SKILL_CRESCENT_MOON_SLASH,0);
	}
	else if(lpObj->Class == CLASS_FE)
	{
		gSkillManager.AddSkill(lpObj,SKILL_STAR_FALL,0);

		if(gQuest.CheckQuestListState(lpObj,2,QUEST_FINISH) != 0)
		{
			gSkillManager.AddSkill(lpObj,SKILL_INFINITY_ARROW,0);
		}
	}
	else if(lpObj->Class == CLASS_MG)
	{
		gSkillManager.AddSkill(lpObj,SKILL_SPIRAL_SLASH,0);
		gSkillManager.AddSkill(lpObj,SKILL_MANA_RAYS,0);
	}
	else if(lpObj->Class == CLASS_DL)
	{
		gSkillManager.AddSkill(lpObj,SKILL_FORCE,0);
		gSkillManager.AddSkill(lpObj,SKILL_FIRE_BLAST,0);
	}
	else if(lpObj->Class == CLASS_SU)
	{
		gSkillManager.AddSkill(lpObj,SKILL_MANA_GLAIVE,0);
	}
	else if(lpObj->Class == CLASS_RF)
	{
		gSkillManager.AddSkill(lpObj,SKILL_CHARGE,0);
	}

	gSkillManager.AddSkill(lpObj,SKILL_STERN,0);
	gSkillManager.AddSkill(lpObj,SKILL_REMOVE_STERN,0);
	gSkillManager.AddSkill(lpObj,SKILL_GREATER_MANA,0);
	gSkillManager.AddSkill(lpObj,SKILL_INVISIBILITY,0);
	gSkillManager.AddSkill(lpObj,SKILL_REMOVE_INVISIBILITY,0);
	gSkillManager.AddSkill(lpObj,SKILL_REMOVE_ALL_EFFECT,0);

	if (lpObj->InitCharacter != 0)
	{
		for(int n=0;n < INVENTORY_WEAR_SIZE;n++)
		{
			CItem item;

			gItemManager.InventoryAddItem(aIndex,item,n);
		}
	}

	for(int n=0;n < INVENTORY_SIZE;n++)
	{
		CItem item;

		lpObj->Inventory[n].Clear();

		if(gItemManager.ConvertItemByte(&item,lpMsg->Inventory[n]) == 0)
		{
			continue;
		}

		gItemManager.InventoryAddItem(aIndex,item,n);
	}

	this->CharacterMakePreviewCharSet(aIndex);

	for(int n=0;n < MAX_EFFECT_LIST;n++)
	{
		CEffect effect;

		lpObj->Effect[n].Clear();

		if(gEffectManager.ConvertEffectByte(&effect,lpMsg->Effect[n]) == 0)
		{
			continue;
		}

		gEffectManager.AddEffect(lpObj,1,effect.m_index,effect.m_count,effect.m_value[0],effect.m_value[1],effect.m_value[2],effect.m_value[3]);
	}

	for(int n=0;n < MAX_SKILL_LIST;n++)
	{
		#if(GAMESERVER_UPDATE>=602)
		if(lpObj->Skill[n].IsSkill() != 0)
		#else
		if(lpObj->Skill[n].IsSkill() != 0 && lpObj->Skill[n].IsMasterSkill() == 0)
		#endif
		{
			if(gSkillManager.CheckSkillRequireWeapon(lpObj,lpObj->Skill[n].m_skill) == 0)
			{
				gSkillManager.DelSkill(lpObj,lpObj->Skill[n].m_index);
			}
		}
	}

	lpObj->Live = 1;
	lpObj->Type = OBJECT_USER;
	lpObj->State = OBJECT_CREATE;
	lpObj->Connected = OBJECT_ONLINE;
	lpObj->TargetNumber = -1;

	if(lpObj->Life == 0)
	{
		lpObj->Live = 1;
		lpObj->State = OBJECT_DYING;
		lpObj->DieRegen = 1;
		lpObj->RegenTime = GetTickCount();
	}

	lpObj->Authority = 1;

	if((lpMsg->CtlCode & 8) != 0)
	{
		lpObj->Authority = 2;
	}

	if((lpMsg->CtlCode & 32) != 0)
	{
		lpObj->Authority = 32;
		gGameMaster.SetGameMasterLevel(lpObj,0);
		gEffectManager.AddEffect(lpObj,0,EFFECT_GAME_MASTER,0,0,0,0,0);
	}

	lpObj->Penalty = 0;

	if((lpMsg->CtlCode & 2) != 0)
	{
		lpObj->Penalty = 4;
	}

	if (lpMsg->StartItem == 0)
	{
		gCustomStartItem.SetStartItem(lpObj);
	}

	gObjAuthorityCodeSet(lpObj);

	gInventoryEquipment.InsertInventoryEquipment(lpObj);

	GCWeatherSend(lpObj->Index,(gMap[lpObj->Map].GetWeather()/16));

	GCMainCheckSend(aIndex);

	gCustomQuest.DGCustomQuestSend(lpObj->Index);

	gCustomAttack.DGCustomAttackResumeSend(lpObj->Index);

	gDarkSpirit[aIndex].SetMode(DARK_SPIRIT_MODE_NORMAL,-1);

	lpObj->HPAutoRecuperationTime = GetTickCount();
	lpObj->MPAutoRecuperationTime = GetTickCount();
	lpObj->BPAutoRecuperationTime = GetTickCount();
	lpObj->SDAutoRecuperationTime = GetTickCount();
	lpObj->CharSaveTime = GetTickCount();
	return 1;
}

void CObjectManager::CharacterLifeCheck(LPOBJ lpObj,LPOBJ lpTarget,int damage,int DamageType,int flag,int type,int skill,int ShieldDamage) // OK
{

	if(lpObj->Connected != OBJECT_ONLINE)
	{
		return;
	}

	if(KALIMA_ATTRIBUTE_RANGE(lpObj->Attribute) != 0 || KALIMA_ATTRIBUTE_RANGE(lpTarget->Attribute) != 0)
	{
		return;
	}

	int SummonIndex = lpObj->Index;

	if(lpObj->Type == OBJECT_MONSTER && OBJECT_RANGE(lpObj->SummonIndex) != 0)
	{
		SummonIndex = lpObj->SummonIndex;
	}

	if(gObj[SummonIndex].Type == OBJECT_USER && lpTarget->Type == OBJECT_MONSTER && damage > 0)
	{
		gObjMonsterSetHitDamage(lpTarget,SummonIndex,damage);
	}

	if(lpTarget->Type == OBJECT_MONSTER && OBJECT_RANGE(lpTarget->SummonIndex) != 0 && damage > 0)
	{
		GCSummonLifeSend(lpTarget->SummonIndex,(int)lpTarget->Life,(int)lpTarget->MaxLife);
	}

	if(lpTarget->Life <= 0 && lpTarget->Live != 0)
	{
		if(gObj[SummonIndex].Type == OBJECT_MONSTER)
		{
			lpTarget->KillerType = 1;
			gObjAddMsgSend(&gObj[SummonIndex],3,lpTarget->Index,0);
		}

		if(gObj[SummonIndex].Type == OBJECT_USER)
		{
			gDarkSpirit[SummonIndex].ResetTarget(lpTarget->Index);
		}

		if(gObj[SummonIndex].Type == OBJECT_USER && lpTarget->Type == OBJECT_USER)
		{
			gObjSetKillCount(SummonIndex,1);
		}

		if(lpTarget->Type == OBJECT_MONSTER)
		{
			if(lpTarget->Class == 275 || lpTarget->Class == 295 || lpTarget->Map == MAP_CRYWOLF)
			{
				gObjAddMsgSendDelay(lpTarget,1,SummonIndex,500,0);
			}
			else
			{
				gObjAddMsgSendDelay(lpTarget,1,SummonIndex,500,0);
			}

			if (lpObj->CustomQuestMonsterIndex >= 0)
			{
				if (lpObj->CustomQuestMonsterIndex == lpTarget->Class && lpObj->CustomQuestMonsterQtd != 0)
				{
					lpObj->CustomQuestMonsterQtd--;
					
					if (lpObj->CustomQuestMonsterQtd == 0)
					{
						gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(855));
					}
					else
					{
						gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(854),lpObj->CustomQuestMonsterQtd);
					}
				}
			}

			if (lpObj->CustomNpcQuestMonsterIndex >= 0)
			{
				if (lpObj->CustomNpcQuestMonsterIndex == lpTarget->Class && lpObj->CustomNpcQuestMonsterQtd != 0)
				{
					lpObj->CustomNpcQuestMonsterQtd--;
					
					if (lpObj->CustomNpcQuestMonsterQtd == 0)
					{
						gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(855));
					}
					else
					{
						gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(854),lpObj->CustomNpcQuestMonsterQtd);
					}
				}
			}

			gQuestObjective.MonsterKill(lpTarget);

			gQuestWorldObjective.MonsterKill(lpTarget);

			gCustomMonster.MonsterDieProc(lpTarget,&gObj[SummonIndex]);

			gInvasionManager.MonsterDieProc(lpTarget,&gObj[SummonIndex]);

			if(BC_MAP_RANGE(lpTarget->Map) != 0)
			{
				gBloodCastle.MonsterDieProc(lpTarget,&gObj[SummonIndex]);
			}

			if(CC_MAP_RANGE(lpTarget->Map) != 0)
			{
				gChaosCastle.MonsterDieProc(lpTarget,&gObj[SummonIndex]);
			}

			if(DS_MAP_RANGE(lpTarget->Map) != 0)
			{
				gDevilSquare.MonsterDieProc(lpTarget,&gObj[SummonIndex]);
			}

			if(DG_MAP_RANGE(lpTarget->Map) != 0)
			{
				gDoubleGoer.MonsterDieProc(lpTarget,&gObj[SummonIndex]);
			}

			if(IT_MAP_RANGE(lpTarget->Map) != 0)
			{
				gIllusionTemple.MonsterDieProc(lpTarget,&gObj[SummonIndex]);
			}

			if(IG_MAP_RANGE(lpTarget->Map) != 0)
			{
				gImperialGuardian.MonsterDieProc(lpTarget,&gObj[SummonIndex]);
			}

			if(lpTarget->Map == MAP_CASTLE_SIEGE)
			{
				gCastleDeep.MonsterDieProc(lpTarget,&gObj[SummonIndex]);
			}

			#if(GAMESERVER_TYPE==1)

			if(lpTarget->Map == MAP_CRYWOLF)
			{
				gCrywolf.CrywolfMonsterDieProc(lpTarget,&gObj[SummonIndex]);
			}

			#endif

			if(lpTarget->Map == MAP_KANTURU3)
			{
				gKanturu.KanturuMonsterDieProc(lpTarget,&gObj[SummonIndex]);
			}

			if(lpTarget->Map == MAP_RAKLION2)
			{
				gRaklion.RaklionMonsterDieProc(lpTarget,&gObj[SummonIndex]);
			}

			if(gObj[SummonIndex].Type == OBJECT_USER)
			{
				gObjPKDownCheckTime(&gObj[SummonIndex],lpTarget->Level);
				gObjAddMsgSendDelay(&gObj[SummonIndex],3,lpTarget->Index,2000,0);
			}
		}

		if(lpTarget->Type == OBJECT_USER)
		{
			gObjUserDie(lpTarget,&gObj[SummonIndex]);

			if(lpTarget->AttackerKilled == 0)
			{
				gObjPlayerKiller(&gObj[SummonIndex],lpTarget);
			}

			lpTarget->AttackerKilled = 0;

			if(gObjGuildWarCheck(&gObj[SummonIndex],lpTarget) == 0)
			{
				lpTarget->KillerType = 0;
			}
			else
			{
				lpTarget->KillerType = 2;
			}

			gDuel.CheckDuelScore(&gObj[SummonIndex],lpTarget);

			if(OBJECT_RANGE(lpTarget->SummonIndex) != 0)
			{
				gObjSummonKill(lpTarget->Index);
			}
		}

		lpTarget->Live = 0;
		lpTarget->State = OBJECT_DYING;
		lpTarget->RegenTime = GetTickCount();
		lpTarget->DieRegen = 1;
		lpTarget->PathCount = 0;
		lpTarget->Teleport = ((lpTarget->Teleport==1)?0:lpTarget->Teleport);

		GCUserDieSend(lpTarget,lpTarget->Index,skill,lpObj->Index);

		//CustomDeathMessage
		if(lpObj->Type == OBJECT_MONSTER)
		{
			gCustomDeathMessage.GetDeathText(lpTarget,lpObj, lpObj->Class);
		}

		if(lpTarget->Type == OBJECT_MONSTER)
		{

			if(OBJECT_RANGE(lpTarget->SummonIndex) != 0)
			{
				return;
			}

			if(lpTarget->Class == 275 || lpTarget->Class == 277 || lpTarget->Class == 283 || lpTarget->Class == 286 || lpTarget->Class == 287 || lpTarget->Class == 288 || lpTarget->Class == 295 || lpTarget->Class == 340 || lpTarget->Class == 348 || lpTarget->Class == 349 || lpTarget->Class == 361 || lpTarget->Class == 362 || lpTarget->Class == 363 || lpTarget->Class == 524 || lpTarget->Class == 525 || lpTarget->Class == 527 || lpTarget->Class == 528 || lpTarget->Class == 605 || lpTarget->Class == 606 || lpTarget->Class == 607)
			{
				return;
			}
		}

		if(gObj[SummonIndex].Type == OBJECT_USER && lpTarget->Type == OBJECT_MONSTER)
		{
			if(gParty.IsParty(gObj[SummonIndex].PartyNumber) == 0)
			{
				this->CharacterCalcExperienceSplit(&gObj[SummonIndex],lpTarget,damage,flag);
			}
			else
			{
				this->CharacterCalcExperienceParty(&gObj[SummonIndex],lpTarget,damage,flag);
			}
		}
	}
	else
	{
		if(DamageType == 0)
		{
			GCDamageSend(SummonIndex,lpTarget->Index,flag,damage,type,ShieldDamage);
		}
		else if(DamageType == 1)
		{
			GCMonsterDamageSend(lpTarget->Index,damage,ShieldDamage);
		}
		else if(DamageType == 2)
		{
			GCDamageSend(SummonIndex,lpTarget->Index,flag,damage,5,ShieldDamage);
		}
		else if(DamageType == 3)
		{
			GCDamageSend(SummonIndex,lpTarget->Index,flag,damage,6,ShieldDamage);
		}
		else if(DamageType == 4)
		{
			#if(GAMESERVER_UPDATE>=701)
			GCElementalDamageSend(SummonIndex,lpTarget->Index,type,damage);
			#endif
		}

		#if(GAMESERVER_UPDATE>=701)

		GCLifeUpdateSend(lpTarget);
		return;

		#endif
	}
}
