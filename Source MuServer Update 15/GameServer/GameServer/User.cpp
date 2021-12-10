#include "stdafx.h"
#include "User.h"
#include "380ItemOption.h"
#include "Attack.h"
#include "BattleSoccerManager.h"
#include "BloodCastle.h"
#include "BonusManager.h"
#include "CannonTower.h"
#include "CashShop.h"
#include "CastleDeep.h"
#include "CastleSiege.h"
#include "CastleSiegeCrown.h"
#include "CastleSiegeCrownSwitch.h"
#include "CastleSiegeSync.h"
#include "CastleSiegeWeapon.h"
#include "ChaosBox.h"
#include "ChaosCastle.h"
#include "CommandManager.h"
#include "Crywolf.h"
#include "CrywolfAltar.h"
#include "CrywolfSync.h"
#include "CustomArena.h"
#include "CustomAttack.h"
#include "CustomEventDrop.h"
#include "CustomOnlineLottery.h"
#include "CustomQuiz.h"
#include "CustomPick.h"
#include "CustomRankUser.h"
#include "CustomStore.h"
#include "DarkSpirit.h"
#include "DefaultClassInfo.h"
#include "DevilSquare.h"
#include "DoubleGoer.h"
#include "DSProtocol.h"
#include "Duel.h"
#include "EffectManager.h"
#include "ESProtocol.h"
#include "EventInventory.h"
#include "EventHideAndSeek.h"
#include "EventKillAll.h"
#include "EventPvp.h"
#include "EventQuickly.h"
#include "EventRunAndCatch.h"
#include "EventRussianRoulette.h"
#include "EventStart.h"
#include "EventGvG.h"
#include "EventTvT.h"
#include "ItemManager.h"
#include "GameMain.h"
#include "GameMaster.h"
#include "Gate.h"
#include "GensSystem.h"
#include "GuardianStatue.h"
#include "Guild.h"
#include "GuildMatching.h"
#include "HackPacketCheck.h"
#include "Helper.h"
#include "IllusionTemple.h"
#include "ImperialGuardian.h"
#include "InvasionManager.h"
#include "InventoryEquipment.h"
#include "IpManager.h"
#include "JewelOfHarmonyOption.h"
#include "JSProtocol.h"
#include "Kalima.h"
#include "Kanturu.h"
#include "LifeStone.h"
#include "Map.h"
#include "MapManager.h"
#include "MapServerManager.h"
#include "MasterSkillTree.h"
#include "MemoryAllocator.h"
#include "Message.h"
#include "MiniMap.h"
#include "Monster.h"
#include "MonsterSkillManager.h"
#include "MossMerchant.h"
#include "Move.h"
#include "MoveSummon.h"
#include "MuRummy.h"
#include "MuunSystem.h"
#include "Notice.h"
#include "ObjectManager.h"
#include "Party.h"
#include "PartyMatching.h"
#include "PcPoint.h"
#include "PentagramSystem.h"
#include "PersonalShop.h"
#include "Quest.h"
#include "QuestWorld.h"
#include "ReiDoMU.h"
#include "Raklion.h"
#include "RaklionBattleUserMng.h"
#include "RaklionUtil.h"
#include "Reconnect.h"
#include "SerialCheck.h"
#include "ServerInfo.h"
#include "SetItemOption.h"
#include "SkillManager.h"
#include "SocketItemOption.h"
#include "Trade.h"
#include "Union.h"
#include "Util.h"
#include "Viewport.h"
#include "Warehouse.h"

int gObjCount;
int gObjMonCount;
int gObjBotCount;//MC bot
int gObjCallMonCount;
int gObjTotalUser;
int gObjTotalMonster;
int gObjTotalBot;//mc
int gCloseMsg;
int gCloseMsgTime;
int gGameServerLogOut;
int gGameServerDisconnect;
int gObjOffStore;
int gObjOffAttack;
int AuthCount=0;

OBJECTSTRUCT_HEADER gObj;

MESSAGE_STATE_MACHINE_HEADER gSMMsg;

MESSAGE_STATE_ATTACK_MACHINE_HEADER gSMAttackProcMsg;

DWORD gCheckSum[MAX_CHECKSUM_KEY];

DWORD gLevelExperience[MAX_CHARACTER_LEVEL+1];

//**************************************************************************//
// OBJECT MAIN FUNCTIONS ***************************************************//
//**************************************************************************//
void gObjEventRunProc() // OK
{
	gBloodCastle.MainProc();

	gReiDoMU.MainProc();

	gEventQuickly.MainProc();

	gEventRussianRoulette.MainProc();

	gEventStart.MainProc();

	gEventHideAndSeek.MainProc();

	gEventRunAndCatch.MainProc();

	gEventPvP.MainProc();

	gEventKillAll.MainProc();
	
	gBonusManager.MainProc();

	gCastleDeep.MainProc();

	#if(GAMESERVER_TYPE==1)

	gCastleSiege.MainProc();

	#endif

	gCastleSiegeWeapon.MainProc();

	gChaosCastle.MainProc();

	#if(GAMESERVER_TYPE==1)

	gCrywolf.MainProc();

	#endif

	gCustomArena.MainProc();

	gTvTEvent.MainProc();

	gGvGEvent.MainProc();

	gCustomEventDrop.MainProc();

	gCustomOnlineLottery.MainProc();

	gCustomQuiz.MainProc();

	gDevilSquare.MainProc();

	#if(GAMESERVER_UPDATE>=501)

	gDoubleGoer.MainProc();

	#endif

	#if(GAMESERVER_UPDATE>=402)

	gDuel.MainProc();

	#endif

	gIllusionTemple.MainProc();

	#if(GAMESERVER_UPDATE>=501)

	gImperialGuardian.MainProc();

	#endif

	gInvasionManager.MainProc();

	gKanturu.MainProc();

	#if(GAMESERVER_UPDATE>=402)

	gMossMerchant.MainProc();

	#endif

	gRaklion.MainProc();
}

void gObjViewportProc() // OK
{
	for(int n=0;n < MAX_OBJECT;n++)
	{
		gObjectManager.ObjectSetStateCreate(n);
	}

	for(int n=0;n < MAX_OBJECT;n++)
	{
		gObjViewportListDestroy(n);
	}

	for(int n=0;n < MAX_OBJECT;n++)
	{
		gObjViewportListCreate(n);
	}

	for(int n=0;n < MAX_OBJECT;n++)
	{
		gObjViewportListProtocol(n);
	}

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		gPersonalShop.GCPShopViewportSend(n);
	}

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		gObj[n].PShopItemChange = 0;
	}

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		gObjUnionUpdateProc(n);
	}

	gObjectManager.ObjectSetStateProc();
}

void gObjFirstProc() // OK
{
	for(int n=0;n < MAX_MAP;n++)
	{
		gMap[n].WeatherVariationProcess();
	}

	gObjSecondProc();

	gCashShop.MainProc();

	gCommandManager.MainProc();

	gDuel.CheckDuelUser();

	gEffectManager.MainProc();

	#if(GAMESERVER_UPDATE>=603)

	gHelper.MainProc();

	#endif

	#if(GAMESERVER_UPDATE>=501)

	gInventoryEquipment.MainProc();

	#endif

	#if(GAMESERVER_UPDATE>=803)

	gMuunSystem.MainProc();

	#endif

	gNotice.MainProc();

	gReconnect.MainProc();

	gCastleSiegeSync.AdjustTributeMoney();

	#if(GAMESERVER_TYPE==1)

	gCrywolf.CrywolfSecondAct();

	#endif

	#if(GAMESERVER_TYPE==1)

	if(gCastleSiege.GetDataLoadState() == CASTLESIEGE_DATALOAD_2)
	{
		gCastleSiege.DataRequest();
	}

	gCastleSiege.SendCastleStateSync();

	#endif

	int Coin1;
	int Coin2;
	int Coin3;

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) == 0)
		{
			continue;
		}

		LPOBJ lpObj = &gObj[n];

		Coin1 = 0;
		Coin2 = 0;
		Coin3 = 0;

		if((GetTickCount()-lpObj->OnlineRewardCoin1) >= ((DWORD)gServerInfo.m_OnlineRewardCoin1Delay*60000))
		{
			lpObj->OnlineRewardCoin1 = GetTickCount();
			Coin1 = gServerInfo.m_OnlineRewardCoin1Value[lpObj->AccountLevel];
		}

		if((GetTickCount()-lpObj->OnlineRewardCoin2) >= ((DWORD)gServerInfo.m_OnlineRewardCoin2Delay*60000))
		{
			lpObj->OnlineRewardCoin2 = GetTickCount();
			Coin2 = gServerInfo.m_OnlineRewardCoin2Value[lpObj->AccountLevel];
		}

		if((GetTickCount()-lpObj->OnlineRewardCoin3) >= ((DWORD)gServerInfo.m_OnlineRewardCoin3Delay*60000))
		{
			lpObj->OnlineRewardCoin3 = GetTickCount();
			Coin3 = gServerInfo.m_OnlineRewardCoin3Value[lpObj->AccountLevel];

			#if(GAMESERVER_UPDATE<=402)
			if((lpObj->Coin3+Coin3) > gServerInfo.m_PcPointMaxPoint)
			{
				Coin3 = gServerInfo.m_PcPointMaxPoint-lpObj->Coin3;
			}
			#endif	
		}

		if (Coin1 > 0 || Coin2 > 0 || Coin3 > 0)
			GDSetCoinSend(lpObj->Index, Coin1, Coin2, Coin3,"OnlineReward"); 
	}
}

void gObjCloseProc() // OK
{
	if(gCloseMsg != 0)
	{
		if((--gCloseMsgTime) <= 1)
		{
			if(gCloseMsgTime == 1)
			{
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(480));
				LogAdd(LOG_RED,gMessage.GetMessage(480));
			}
		}
		else
		{
			if((gCloseMsgTime%10) == 0)
			{
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(481),gCloseMsgTime);
				LogAdd(LOG_RED,gMessage.GetMessage(481),gCloseMsgTime);
			}
		}

		if(gCloseMsgTime < 0)
		{
			gCloseMsg = 0;
			gCloseMsgTime = 0;
			gObjAllLogOut();
		}
	}
}

void gObjCountProc() // OK
{
	int TotalUser = 0;
	int TotalBot = 0;
	int TotalMonster = 0;
	int TotalOffStore = 0;
	int TotalOffAttack = 0;

	for(int n=0;n < MAX_OBJECT;n++)
	{
		if(gObj[n].Connected != OBJECT_OFFLINE)
		{
			if(gObj[n].Type == OBJECT_BOTS)
			{
				TotalBot++;
			}
			if(gObj[n].Type == OBJECT_USER)
			{
				TotalUser++;
				if(gObj[n].PShopCustomOffline == 1)
				{
					TotalOffStore++;
				}
				if(gObj[n].AttackCustomOffline == 1)
				{
					TotalOffAttack++;
				}
			}
			else
			{
				TotalMonster++;
			}
		}
	}
	
	gObjTotalBot = TotalBot;//mc
	gObjTotalUser = TotalUser;
	gObjOffStore = TotalOffStore;
	gObjOffAttack = TotalOffAttack;

	gObjTotalMonster = TotalMonster;
}

void gObjAccountLevelProc() // OK
{
	LPOBJ lpObj;

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			GJAccountLevelSend(n);
		}

		lpObj = &gObj[n];

		if(lpObj->Connected >= OBJECT_LOGGED && lpObj->Type == OBJECT_USER && lpObj->CloseCount <= 0)
		{
			if(gServerInfo.m_ServerMinLevel != 0 && lpObj->Level < gServerInfo.m_ServerMinLevel)
			{
				lpObj->CloseCount = 6;
				lpObj->CloseType = 3;
				return;
			}

			if(gServerInfo.m_ServerMinReset != 0 && lpObj->Reset < gServerInfo.m_ServerMinReset)
			{
				lpObj->CloseCount = 6;
				lpObj->CloseType = 3;
				return;
			}

			if(gServerInfo.m_ServerMinMasterReset != 0 && lpObj->MasterReset < gServerInfo.m_ServerMinMasterReset)
			{
				lpObj->CloseCount = 6;
				lpObj->CloseType = 3;
				return;
			}

			if(gServerInfo.m_ServerMaxLevel != 0 && lpObj->Level > gServerInfo.m_ServerMaxLevel)
			{
				lpObj->CloseCount = 6;
				lpObj->CloseType = 4;
				return;
			}

			if(gServerInfo.m_ServerMaxReset != 0 && lpObj->Reset > gServerInfo.m_ServerMaxReset)
			{
				lpObj->CloseCount = 6;
				lpObj->CloseType = 4;
				return;
			}

			if(gServerInfo.m_ServerMaxMasterReset != 0 && lpObj->MasterReset > gServerInfo.m_ServerMaxMasterReset)
			{
				lpObj->CloseCount = 6;
				lpObj->CloseType = 4;
				return;
			}
		}
	}
}

void gObjPickProc() // OK
{
	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			gCustomPick.PickProc(&gObj[n]);
		}
	}
}

void gObjMathAuthenticatorProc() // OK
{
	#if(GAMESERVER_UPDATE>=701)

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			GCMathAuthenticatorSend(n);
		}
	}

	#endif
}

void gObjAuthProc() // OK
{
	return; 
}
//**************************************************************************//
// OBJECT BASE FUNCTIONS ***************************************************//
//**************************************************************************//
void gObjInit() // OK
{
	gObjCount = OBJECT_START_USER;

	gObjMonCount = OBJECT_START_MONSTER;

	gObjCallMonCount = MAX_OBJECT_MONSTER;

	memset(gObj.CommonStruct,0,sizeof(OBJECTSTRUCT));

	gMemoryAllocator.m_TempMemoryAllocatorInfo.Alloc();

	gMemoryAllocator.BindMemoryAllocatorInfo(OBJECT_START_USER,gMemoryAllocator.m_TempMemoryAllocatorInfo);
}

void gObjAllLogOut() // OK
{
	gGameServerLogOut = 1;

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObj[n].Connected != OBJECT_OFFLINE && gObj[n].Type == OBJECT_USER)
		{
			CloseClient(n);

			gCustomAttack.OnAttackAlreadyConnected(&gObj[n]);

			gCustomStore.OnPShopAlreadyConnected(&gObj[n]);
		}
	}
}

void gObjAllDisconnect() // OK
{
	gGameServerDisconnect = 1;

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObj[n].Connected != OBJECT_OFFLINE && gObj[n].Type == OBJECT_USER)
		{
			CloseClient(n);

			gCustomAttack.OnAttackAlreadyConnected(&gObj[n]);

			gCustomStore.OnPShopAlreadyConnected(&gObj[n]);
		}
	}
}

void gObjSetExperienceTable() // OK
{
	gLevelExperience[0] = 0;

	DWORD over = 1;

	for(int n=1;n <= MAX_CHARACTER_LEVEL;n++)
	{
		gLevelExperience[n] = (((n+9)*n)*n)*10;

		if(n > 255)
		{
			gLevelExperience[n] += (((over+9)*over)*over)*1000;
			over++;
		}
	}
}

void gObjCharZeroSet(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	lpObj->CloseCount = -1;
	lpObj->CloseType = -1;
	lpObj->EnableDelCharacter = 1;
	lpObj->TimeCount = 0;
	lpObj->PKTickCount = 0;
	lpObj->CheckSumTableNum = -1;
	lpObj->CheckSumTime = 0;
	lpObj->Live = 0;

	memset(lpObj->Name,0,sizeof(lpObj->Name));

	lpObj->Level = 0;
	lpObj->LevelUpPoint = 0;
	lpObj->FruitAddPoint = 0;
	lpObj->FruitSubPoint = 0;
	lpObj->Money = 0;
	lpObj->Strength = 0;
	lpObj->Dexterity = 0;
	lpObj->Vitality = 0;
	lpObj->Energy = 0;
	lpObj->Leadership = 0;
	lpObj->AddStrength = 0;
	lpObj->AddDexterity = 0;
	lpObj->AddVitality = 0;
	lpObj->AddEnergy = 0;
	lpObj->AddLeadership = 0;
	lpObj->Life = 0;
	lpObj->MaxLife = 0;
	lpObj->AddLife = 0;
	lpObj->ScriptMaxLife = 0;
	lpObj->Mana = 0;
	lpObj->MaxMana = 0;
	lpObj->AddMana = 0;
	lpObj->BP;
	lpObj->MaxBP = 0;
	lpObj->AddBP = 0;
	lpObj->Shield = 0;
	lpObj->MaxShield = 0;
	lpObj->AddShield = 0;
	lpObj->ChatLimitTime = 0;
	lpObj->ChatLimitTimeSec = 0;
	lpObj->PKCount = 0;
	lpObj->PKLevel = 0;
	lpObj->PKTime = GetTickCount();
	lpObj->X = 0;
	lpObj->Y = 0;
	lpObj->TX = 0;
	lpObj->TY = 0;
	lpObj->MTX = 0;
	lpObj->MTY = 0;
	lpObj->OldX = 0;
	lpObj->OldY = 0;
	lpObj->StartX = 0;
	lpObj->StartY = 0;
	lpObj->Dir = 0;
	lpObj->Map = 0;
	lpObj->PathCount = 0;
	lpObj->PathCur = 0;
	lpObj->PathStartEnd = 0;
	lpObj->Authority = 0;
	lpObj->AuthorityCode = 0;
	lpObj->Penalty = 0;
	lpObj->State = OBJECT_EMPTY;
	lpObj->Rest = 0;
	lpObj->ViewState = 0;
	lpObj->DieRegen = 0;
	lpObj->RegenOk = 0;
	lpObj->RegenMapNumber = 0;
	lpObj->RegenMapX = 0;
	lpObj->RegenMapY = 0;
	lpObj->RegenTime = 0;
	lpObj->PosNum = -1;
	lpObj->DelayLevel = 0;
	lpObj->DrinkSpeed = 0;
	lpObj->DrinkLastTime = 0;
	lpObj->MonsterDeleteTime = 0;
	lpObj->KalimaGateExist = 0;
	lpObj->KalimaGateIndex = -1;
	lpObj->KalimaGateEnterCount = 0;
	lpObj->AttackObj = 0;
	lpObj->AttackerKilled = 0;

	for(int n=0;n < MAX_SELF_DEFENSE;n++)
	{
		lpObj->SelfDefense[n] = -1;
	}

	///Custom Pick

	lpObj->PickupEnable = 0;
	lpObj->PickupExc	= 0;
	lpObj->PickupSocket = 0;
	lpObj->PickupSetItem = 0;

	for (int i = 0; i < MAX_CUSTOMPICK; i++)
	{
		lpObj->Pickup[i] = -1;
	}

	memset(lpObj->SelfDefenseTime,0,sizeof(lpObj->SelfDefenseTime));

	memset(lpObj->GuildName,0,sizeof(lpObj->GuildName));

	lpObj->MySelfDefenseTime = 0;
	lpObj->PartyNumber = -1;
	lpObj->PartyTargetUser = -1;
	lpObj->GuildNumber = 0;
	lpObj->Guild = 0;
	lpObj->GuildStatus = -1;
	lpObj->GuildUnionTimeStamp = 0;
	lpObj->SummonIndex = -1;
	lpObj->Change = -1;
	lpObj->TargetNumber = -1;
	lpObj->TargetShopNumber = -1;
	lpObj->LastAttackerID = -1;
	lpObj->MagicDefense = 0;
	lpObj->Attribute = 0;
	lpObj->MultiSkillIndex = 0;
	lpObj->MultiSkillCount = 0;
	lpObj->RageFighterSkillIndex = 0;
	lpObj->RageFighterSkillCount = 0;
	lpObj->RageFighterSkillTarget = 10000;

	for(int n=0;n < MAX_SKILL_LIST;n++)
	{
		lpObj->Skill[n].Clear();
	}

	memset(lpObj->CharSet,0,sizeof(lpObj->CharSet));

	memset(lpObj->Resistance,0,sizeof(lpObj->Resistance));

	memset(lpObj->AddResistance,0,sizeof(lpObj->AddResistance));

	gObjClearViewport(lpObj);

	gObjMonsterInitHitDamage(lpObj);

	lpObj->Interface.use = 0;
	lpObj->Interface.type = INTERFACE_NONE;
	lpObj->Interface.state = 0;
	lpObj->InterfaceTime = 0;
	lpObj->Transaction = 0;

	gObjSetInventory1Pointer(lpObj);

	gObjSetEventInventory1Pointer(lpObj);

	gObjClearPlayerOption(lpObj);

	lpObj->TradeMoney = 0;
	lpObj->TradeOk = 0;
	lpObj->TradeDuel = 0;
	lpObj->WarehouseCount = 0;
	lpObj->WarehousePW = 0;
	lpObj->WarehouseLock = -1;
	lpObj->WarehouseMoney = 0;
	lpObj->WarehouseSave = 0;
	lpObj->ChaosMoney = 0;
	lpObj->ChaosSuccessRate = 0;
	lpObj->ChaosLock = 0;
	#if(GAMESERVER_UPDATE>=802)
	lpObj->LoadEventInventory = 0;
	lpObj->MuunItemStatus[0] = 0;
	lpObj->MuunItemStatus[1] = 0;
	lpObj->LoadMuunInventory = 0;
	#endif
	lpObj->Option = 3;
	lpObj->ChaosCastleBlowTime = 0;
	lpObj->DuelUserReserved = -1;
	lpObj->DuelUserRequested = -1;
	lpObj->DuelUser = -1;
	lpObj->DuelScore = 0;
	lpObj->DuelTickCount = 0;

	memset(lpObj->PShopText,0,sizeof(lpObj->PShopText));

	memset(lpObj->PShopDealerName,0,sizeof(lpObj->PShopDealerName));

	memset(lpObj->VpPShopPlayer,0,sizeof(lpObj->VpPShopPlayer));

	lpObj->PShopOpen = 0;
	lpObj->PShopTransaction = 0;
	lpObj->PShopItemChange = 0;
	lpObj->PShopRedrawAbs = 0;
	lpObj->PShopWantDeal = 0;
	lpObj->PShopDealerIndex = -1;
	lpObj->PShopCustom = 0;
	lpObj->PShopCustomType = 0;
	lpObj->PShopCustomTime = 0;
	lpObj->PShopCustomOffline = 0;
	lpObj->PShopCustomOfflineTime = 0;
	lpObj->VpPShopPlayerCount = 0;
	lpObj->IsInBattleGround = 0;
	lpObj->UseEventServer = 0;
	lpObj->LoadWarehouse = 0;
	lpObj->LoadGuildWarehouse = 0;

	memset(lpObj->Quest,0xFF,sizeof(lpObj->Quest));

	lpObj->SendQuestInfo = 0;
	lpObj->CheckLifeTime = 0;
	lpObj->LastTeleportTime = 0;
	lpObj->SkillNovaState = 0;
	lpObj->SkillNovaCount = 0;
	lpObj->SkillNovaTime = 0;
	lpObj->ReqWarehouseOpen = 0;
	lpObj->IsFullSetItem = 0;
	lpObj->SkillSummonPartyTime = 0;
	lpObj->SkillSummonPartyMap = 0;
	lpObj->SkillSummonPartyX = 0;
	lpObj->SkillSummonPartyY = 0;
	lpObj->IsChaosMixCompleted = 0;
	lpObj->SkillLongSpearChange = 0;
	lpObj->CharSaveTime = 0;
	lpObj->LoadQuestKillCount = 0;
	lpObj->QuestKillCountIndex = -1;
	lpObj->LoadMasterLevel = 0;
	lpObj->MasterLevel = 0;
	lpObj->MasterPoint = 0;
	lpObj->MasterExperience = 0;
	lpObj->MasterNextExperience = 0;

	for(int n=0;n < MAX_EFFECT_LIST;n++)
	{
		lpObj->Effect[n].Clear();
	}

	lpObj->ExtInventory = 0;
	lpObj->WarehouseNumber = 0;
	lpObj->AutoAddPointCount = 0;
	lpObj->AutoResetEnable = 0;

	memset(lpObj->AutoPartyPassword,0,sizeof(lpObj->AutoPartyPassword));

	memset(lpObj->AutoAddPointStats,0,sizeof(lpObj->AutoAddPointStats));

	memset(lpObj->AutoResetStats,0,sizeof(lpObj->AutoResetStats));

	lpObj->AttackCustom = 0;
	lpObj->AttackCustomSkill = 0;
	lpObj->AttackCustomDelay = 0;
	lpObj->AttackCustomZoneX = 0;
	lpObj->AttackCustomZoneY = 0;
	lpObj->AttackCustomZoneMap = 0;
	lpObj->AttackCustomOffline = 0;
	lpObj->AttackCustomOfflineTime = 0;
//	lpObj->AttackCustomOfflineMoneyDelay = 0;
	lpObj->AttackCustomAutoBuff = 0;
	lpObj->AttackCustomAutoBuffDelay = 0;
	lpObj->MiniMapState = 0;
	lpObj->MiniMapValue = -1;
	lpObj->MiningStage = 0;
	lpObj->MiningIndex = -1;
	lpObj->UseGuildMatching = 0;
	lpObj->UseGuildMatchingJoin = 0;
	lpObj->UsePartyMatching = 0;
	lpObj->UsePartyMatchingJoin = 0;

	memset(lpObj->CashShopTransaction,0,sizeof(lpObj->CashShopTransaction));

	memset(lpObj->CommandManagerTransaction,0,sizeof(lpObj->CommandManagerTransaction));

	memset(lpObj->LuckyCoinTransaction,0,sizeof(lpObj->LuckyCoinTransaction));

	memset(lpObj->PcPointTransaction,0,sizeof(lpObj->PcPointTransaction));

	lpObj->AutoAttackTime = 0;
	lpObj->TradeOkTime = 0;
	lpObj->PotionTime = 0;
	lpObj->ComboTime = 0;
	lpObj->HelperDelayTime = 0;
	lpObj->HelperTotalTime = 0;
	lpObj->OnlineRewardCoin1 = GetTickCount();
	lpObj->OnlineRewardCoin2 = GetTickCount();
	lpObj->OnlineRewardCoin3 = GetTickCount();
	lpObj->HPAutoRecuperationTime = 0;
	lpObj->MPAutoRecuperationTime = 0;
	lpObj->BPAutoRecuperationTime = 0;
	lpObj->SDAutoRecuperationTime = 0;
	lpObj->Reset = 0;
	lpObj->MasterReset = 0;
	lpObj->ChangeSkin = 0;
	lpObj->LoadQuestWorld = 0;
	lpObj->QuestWorldMonsterClass = -1;
	lpObj->LoadGens = 0;
	lpObj->GensFamily = 0;
	lpObj->GensRank = 0;
	lpObj->GensSymbol = 0;
	lpObj->GensContribution = 0;
	lpObj->GensNextContribution = 0;

	memset(&lpObj->EffectOption,0,sizeof(lpObj->EffectOption));

	gObjClearSpecialOption(lpObj);

	lpObj->HPRecoveryCount = 0;
	lpObj->MPRecoveryCount = 0;
	lpObj->BPRecoveryCount = 0;
	lpObj->SDRecoveryCount = 0;
	lpObj->ResurrectionTalismanActive = 0;
	lpObj->ResurrectionTalismanMap = 0;
	lpObj->ResurrectionTalismanX = 0;
	lpObj->ResurrectionTalismanY = 0;
	lpObj->MobilityTalismanActive = 0;
	lpObj->MobilityTalismanMap = 0;
	lpObj->MobilityTalismanX = 0;
	lpObj->MobilityTalismanY = 0;
	lpObj->MapServerMoveQuit = 0;
	#if(GAMESERVER_TYPE==1)
	lpObj->CsNpcExistVal = 0;
	lpObj->CsNpcType = 0;
	lpObj->CsGateOpen = 0;
	lpObj->CsGateLeverLinkIndex = -1;
	lpObj->CsNpcDfLevel = 0;
	lpObj->CsNpcRgLevel = 0;
	lpObj->CsJoinSide = 0;
	lpObj->CsGuildInvolved = 0;
	#endif
	lpObj->IsCastleNPCUpgradeCompleted = 0;
	lpObj->CsSiegeWeaponState = 0;
	lpObj->CsWeaponIndex = -1;
	lpObj->KillCount = 0;
	lpObj->AccumulatedDamage = 0;
	#if(GAMESERVER_TYPE==1)
	lpObj->LifeStoneCount = 0;
	lpObj->CreationState = 0;
	lpObj->CreatedActivationTime = 0;
	#endif

	lpObj->MonsterSkillElementOption.Reset();

	lpObj->Agro.ResetAll();

	lpObj->BasicAI = 0;
	lpObj->CurrentAI = 0;
	lpObj->CurrentAIState = 0;
	lpObj->LastAIRunTime = 0;
	lpObj->GroupNumber = 0;
	lpObj->SubGroupNumber = 0;
	lpObj->GroupMemberGuid = -1;
	lpObj->RegenType = 0;
	lpObj->LastAutomataRuntime = 0;
	lpObj->LastAutomataDelay = 0;
	lpObj->AccumulatedCrownAccessTime = 0;
	lpObj->CrywolfMVPScore = 0;
	lpObj->LastCheckTick = 0;

	lpObj->RenameActive = 0;
	lpObj->RunAndCatch = 0;
	lpObj->CustomQuest = 0;
	lpObj->CustomQuestMonsterIndex = -1;
	lpObj->CustomQuestMonsterQtd = -1;
	lpObj->CustomNpcQuest = -1;
	lpObj->CustomNpcQuestMonsterIndex = -1;
	lpObj->CustomNpcQuestMonsterQtd = -1;
	lpObj->CustomNpcQuestFinished = -1;
	lpObj->RussianRoulette = 0;

	lpObj->Kills = 0;
	lpObj->Deads = 0;

	lpObj->Coin1 = 0;
	lpObj->Coin2 = 0;
	lpObj->Coin3 = 0;
	lpObj->BuyVip = 0;

	lpObj->RenameEnable = 0;

	lpObj->DisablePvp = 0;

	lpObj->PvP = 0;

	lpObj->KillAll = 0;

	lpObj->InitCharacter = 0;

	lpObj->MapMoveDisable = 0;

	lpObj->SpeedHackDelay = 0;
	lpObj->SpeedHackCount = 0;
	lpObj->SpeedHackSkill = 0;
	lpObj->SpeedHackDialog = 0;

	lpObj->ShopDelay = 0;

	for(int n=0;n < 100;n++)
	{
		lpObj->CommandDelay[n] = 0;
		lpObj->CommandNotice[n] = 0;
	}

	for(int n=0;n < MAX_MONSTER_SEND_MSG;n++)
	{
		gSMMsg[aIndex][n].Clear();
	}

	for(int n=0;n < MAX_MONSTER_SEND_ATTACK_MSG;n++)
	{
		gSMAttackProcMsg[aIndex][n].Clear();
	}
}

void gObjClearPlayerOption(LPOBJ lpObj) // OK
{
	if(lpObj->Type != OBJECT_USER)
	{
		return;
	}

	for(int n=0;n < INVENTORY_SIZE;n++)
	{
		lpObj->Inventory[n].Clear();
	}

	for(int n=0;n < INVENTORY_SIZE;n++)
	{
		lpObj->Inventory1[n].Clear();
	}

	for(int n=0;n < INVENTORY_SIZE;n++)
	{
		lpObj->Inventory2[n].Clear();
	}

	for(int n=0;n < WAREHOUSE_SIZE;n++)
	{
		lpObj->Warehouse[n].Clear();
	}

	#if(GAMESERVER_UPDATE>=802)

	for(int n=0;n < EVENT_INVENTORY_SIZE;n++)
	{
		lpObj->EventInventory[n].Clear();
	}

	for(int n=0;n < EVENT_INVENTORY_SIZE;n++)
	{
		lpObj->EventInventory1[n].Clear();
	}

	for(int n=0;n < EVENT_INVENTORY_SIZE;n++)
	{
		lpObj->EventInventory2[n].Clear();
	}

	#endif

	#if(GAMESERVER_UPDATE>=803)

	for(int n=0;n < MUUN_INVENTORY_SIZE;n++)
	{
		lpObj->MuunInventory[n].Clear();
	}

	#endif

	#if(GAMESERVER_UPDATE>=802)

	lpObj->MuRummyInfo->Clear();

	#endif

	gTrade.ClearTrade(lpObj);

	memset(lpObj->InventoryMap,0xFF,INVENTORY_SIZE);

	memset(lpObj->InventoryMap1,0xFF,INVENTORY_SIZE);

	memset(lpObj->InventoryMap2,0xFF,INVENTORY_SIZE);

	memset(lpObj->WarehouseMap,0xFF,WAREHOUSE_SIZE);

	#if(GAMESERVER_UPDATE>=802)

	memset(lpObj->EventInventoryMap,0xFF,EVENT_INVENTORY_SIZE);

	memset(lpObj->EventInventoryMap1,0xFF,EVENT_INVENTORY_SIZE);

	memset(lpObj->EventInventoryMap2,0xFF,EVENT_INVENTORY_SIZE);

	memset(lpObj->MuunInventoryMap,0xFF,MUUN_INVENTORY_SIZE);

	#endif

	for(int n=0;n < MAX_MASTER_SKILL_LIST;n++)
	{
		lpObj->MasterSkill[n].Clear();
	}

	for(int n=0;n < MAX_QUEST_KILL_COUNT;n++)
	{
		lpObj->QuestKillCount[n].Clear();
	}

	for(int n=0;n < MAX_QUEST_WORLD_LIST;n++)
	{
		memset(&lpObj->QuestWorldList[n],0xFF,sizeof(lpObj->QuestWorldList[n]));
	}

	for(int n=0;n < MAX_GENS_SYSTEM_VICTIM;n++)
	{
		lpObj->GensVictimList[n].Reset();
	}

	#if(GAMESERVER_UPDATE>=701)

	for(int n=0;n < MAX_PENTAGRAM_JEWEL_INFO;n++)
	{
		lpObj->PentagramJewelInfo_Inventory[n].Clear();
	}

	for(int n=0;n < MAX_PENTAGRAM_JEWEL_INFO;n++)
	{
		lpObj->PentagramJewelInfo_Warehouse[n].Clear();
	}

	#endif

	for(int n=0;n < MAX_SKILL;n++)
	{
		memset(&lpObj->SkillDelay[n],0,sizeof(lpObj->SkillDelay[n]));
	}

	for(int n=0;n < MAX_HACK_PACKET_INFO;n++)
	{
		memset(&lpObj->HackPacketDelay[n],0,sizeof(lpObj->HackPacketDelay[n]));
	}

	for(int n=0;n < MAX_HACK_PACKET_INFO;n++)
	{
		memset(&lpObj->HackPacketCount[n],0,sizeof(lpObj->HackPacketCount[n]));
	}
}

void gObjClearSpecialOption(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	memset(&lpObj->PentagramOption,0,sizeof(lpObj->PentagramOption));

	memset(&lpObj->PentagramJewelOption,0,sizeof(lpObj->PentagramJewelOption));

	#endif

	lpObj->ArmorSetBonus = 0;
	lpObj->SkillDamageBonus = 0;
	lpObj->DoubleDamageRate = 0;
	lpObj->TripleDamageRate = 0;
	lpObj->IgnoreDefenseRate = 0;
	lpObj->IgnoreShieldGaugeRate = 0;
	lpObj->CriticalDamageRate = 0;
	lpObj->CriticalDamage = 0;
	lpObj->ExcellentDamageRate = 0;
	lpObj->ExcellentDamage = 0;
	lpObj->ResistDoubleDamageRate = 0;
	lpObj->ResistIgnoreDefenseRate = 0;
	lpObj->ResistIgnoreShieldGaugeRate = 0;
	lpObj->ResistCriticalDamageRate = 0;
	lpObj->ResistExcellentDamageRate = 0;
	lpObj->ResistStunRate = 0;
	lpObj->ExperienceRate = 100;
	lpObj->MasterExperienceRate = 100;
	lpObj->ItemDropRate = 100;
	lpObj->MoneyAmountDropRate = 100;
	lpObj->HPRecovery = 0;
	lpObj->MPRecovery = 0;
	lpObj->BPRecovery = 2;
	lpObj->SDRecovery = 0;
	lpObj->HPRecoveryRate = 0;
	lpObj->MPRecoveryRate = 0;
	lpObj->BPRecoveryRate = 0;
	lpObj->SDRecoveryRate = 0;
	lpObj->SDRecoveryType = 0;
	lpObj->MPConsumptionRate = 100;
	lpObj->BPConsumptionRate = 100;
	lpObj->ShieldGaugeRate = gServerInfo.m_ShieldGaugeRate;
	lpObj->DecreaseShieldGaugeRate = 0;
	lpObj->DamagePvP = 0;
	lpObj->DefensePvP = 0;
	lpObj->AttackSuccessRatePvP = 0;
	lpObj->DefenseSuccessRatePvP = 0;
	lpObj->ShieldDamageReduction = gServerInfo.m_DefenseConstA;
	lpObj->ShieldDamageReductionTime = 0;

	memset(lpObj->DamageReduction,0,sizeof(lpObj->DamageReduction));

	lpObj->DamageReflect = 0;
	lpObj->HuntHP = 0;
	lpObj->HuntMP = 0;
	lpObj->HuntBP = 0;
	lpObj->HuntSD = 0;
	lpObj->WeaponDurabilityRate = 100;
	lpObj->ArmorDurabilityRate = 100;
	lpObj->WingDurabilityRate = 100;
	lpObj->GuardianDurabilityRate = 100;
	lpObj->PendantDurabilityRate = 100;
	lpObj->RingDurabilityRate = 100;
	lpObj->PetDurabilityRate = 100;
	lpObj->FullDamageReflectRate = 0;
	lpObj->DefensiveFullHPRestoreRate = 0;
	lpObj->DefensiveFullMPRestoreRate = 0;
	lpObj->DefensiveFullSDRestoreRate = 0;
	lpObj->DefensiveFullBPRestoreRate = 0;
	lpObj->OffensiveFullHPRestoreRate = 0;
	lpObj->OffensiveFullMPRestoreRate = 0;
	lpObj->OffensiveFullSDRestoreRate = 0;
	lpObj->OffensiveFullBPRestoreRate = 0;
}

void gObjCalcExperience(LPOBJ lpObj) // OK
{
	lpObj->Experience = ((lpObj->Experience<gLevelExperience[(lpObj->Level-1)])?gLevelExperience[(lpObj->Level-1)]:lpObj->Experience);

	lpObj->NextExperience = gLevelExperience[((lpObj->Level>=MAX_CHARACTER_LEVEL)?MAX_CHARACTER_LEVEL:lpObj->Level)];

	gMasterSkillTree.CalcMasterLevelNextExperience(lpObj);
}

bool gObjGetRandomFreeLocation(int map,int* ox,int* oy,int tx,int ty,int count) // OK
{
	int x = (*ox);
	int y = (*oy);

	tx = ((tx<1)?1:tx);
	ty = ((ty<1)?1:ty);

	for(int n=0;n < count;n++)
	{
		(*ox) = ((GetLargeRand()%(tx+1))*((GetLargeRand()%2==0)?-1:1))+x;
		(*oy) = ((GetLargeRand()%(ty+1))*((GetLargeRand()%2==0)?-1:1))+y;

		if(gMap[map].CheckAttr((*ox),(*oy),255) == 0)
		{
			return 1;
		}
	}

	return 0;
}

bool gObjAllocData(int aIndex) // OK
{
	CMemoryAllocatorInfo MemoryAllocatorInfo;

	if(gMemoryAllocator.GetMemoryAllocatorInfo(&MemoryAllocatorInfo,aIndex) == 0)
	{
		gObj.ObjectStruct[aIndex] = new OBJECTSTRUCT;

		memset(gObj.ObjectStruct[aIndex],0,sizeof(OBJECTSTRUCT));

		InitializeCriticalSection(&gObj.ObjectStruct[aIndex]->PShopTrade);

		MemoryAllocatorInfo.m_Index = aIndex;

		MemoryAllocatorInfo.m_Active = 1;

		MemoryAllocatorInfo.m_ActiveTime = GetTickCount();

		MemoryAllocatorInfo.Alloc();

		gMemoryAllocator.InsertMemoryAllocatorInfo(MemoryAllocatorInfo);

		gMemoryAllocator.BindMemoryAllocatorInfo(aIndex,MemoryAllocatorInfo);

		if(OBJECT_MONSTER_RANGE(aIndex) != 0){gObjMonCount = (((++gObjMonCount)>=MAX_OBJECT_MONSTER)?OBJECT_START_MONSTER:gObjMonCount);}

		if(OBJECT_BOTS_RANGE(aIndex) != 0){gObjBotCount = (((++gObjBotCount)>=MAX_OBJECT_BOTS)?OBJECT_START_BOTS:gObjBotCount);}

		if(OBJECT_SUMMON_RANGE(aIndex) != 0){gObjCallMonCount = (((++gObjCallMonCount)>=OBJECT_START_USER)?MAX_OBJECT_MONSTER:gObjCallMonCount);}

		if(OBJECT_USER_RANGE(aIndex) != 0){gObjCount = (((++gObjCount)>=MAX_OBJECT)?OBJECT_START_USER:gObjCount);}
	}
	else
	{
		MemoryAllocatorInfo.m_Index = aIndex;

		MemoryAllocatorInfo.m_Active = 1;

		MemoryAllocatorInfo.m_ActiveTime = GetTickCount();

		gMemoryAllocator.InsertMemoryAllocatorInfo(MemoryAllocatorInfo);

		gMemoryAllocator.BindMemoryAllocatorInfo(aIndex,MemoryAllocatorInfo);
	}

	return 1;
}

void gObjFreeData(int aIndex) // OK
{
	CMemoryAllocatorInfo MemoryAllocatorInfo;

	if(gMemoryAllocator.GetMemoryAllocatorInfo(&MemoryAllocatorInfo,aIndex) != 0)
	{
		MemoryAllocatorInfo.m_Index = aIndex;

		MemoryAllocatorInfo.m_Active = 0;

		MemoryAllocatorInfo.m_ActiveTime = GetTickCount();

		gMemoryAllocator.InsertMemoryAllocatorInfo(MemoryAllocatorInfo);
	}
}

short gObjAddSearch(SOCKET socket,char* IpAddress) // OK
{
	int index = -1;
	int count = gObjCount;

	if(gGameServerLogOut != 0)
	{
		GCConnectAccountSend(0,2,socket);
		return -1;
	}

	if(gGameServerDisconnect != 0)
	{
		GCConnectAccountSend(0,2,socket);
		return -1;
	}

	if(gObjTotalUser >= gServerInfo.m_ServerMaxUserNumber)
	{
		GCConnectAccountSend(0,4,socket);
		return -1;
	}

	if(gMemoryAllocator.GetMemoryAllocatorFree(&index,OBJECT_START_USER,MAX_OBJECT,10000) != 0)
	{
		return index;
	}

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObj[count].Connected == OBJECT_OFFLINE)
		{
			return count;
		}
		else
		{
			count = (((++count)>=MAX_OBJECT)?OBJECT_START_USER:count);
		}
	}

	return -1;
}

short gObjAdd(SOCKET socket,char* IpAddress,int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return -1;
	}

	if(gObj[aIndex].Connected != OBJECT_OFFLINE)
	{
		return -1;
	}

	if(gObjAllocData(aIndex) == 0)
	{
		return -1;
	}

	LPOBJ lpObj = &gObj[aIndex];

	gObjCharZeroSet(aIndex);

	lpObj->Index = aIndex;
	lpObj->Connected = OBJECT_CONNECTED;
	lpObj->LoginMessageSend = 0;
	lpObj->LoginMessageCount = 0;
	lpObj->Socket = socket;

	strcpy_s(lpObj->IpAddr,IpAddress);

	lpObj->AutoSaveTime = GetTickCount();
	lpObj->ConnectTickCount = GetTickCount();
	lpObj->Type = OBJECT_USER;
	lpObj->ExtWarehouse = 0;

	memset(lpObj->Account,0,sizeof(lpObj->Account));

	gSerialCheck[aIndex].Init();

	gIpManager.InsertIpAddress(lpObj->IpAddr);

	LogAddConnect(LOG_GREEN,"[Obj][%d] AddClient (%s)",aIndex,lpObj->IpAddr);

	return aIndex;
}

short gObjAddNew(char* IpAddress,int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return -1;
	}

	if(gObj[aIndex].Connected != OBJECT_OFFLINE)
	{
		return -1;
	}

	if(gObjAllocData(aIndex) == 0)
	{
		return -1;
	}

	LPOBJ lpObj = &gObj[aIndex];

	gObjCharZeroSet(aIndex);

	lpObj->Index = aIndex;
	lpObj->Connected = OBJECT_CONNECTED;
	lpObj->LoginMessageSend = 0;
	lpObj->LoginMessageCount = 0;

	strcpy_s(lpObj->IpAddr,IpAddress);

	lpObj->AutoSaveTime = GetTickCount();
	lpObj->ConnectTickCount = GetTickCount();
	lpObj->Type = OBJECT_USER;
	lpObj->ExtWarehouse = 0;

	memset(lpObj->Account,0,sizeof(lpObj->Account));

	gSerialCheck[aIndex].Init();

	gIpManager.InsertIpAddress(lpObj->IpAddr);

	LogAddConnect(LOG_GREEN,"[Obj][%d] AddClientNew (%s)",aIndex,lpObj->IpAddr);

	return aIndex;
}

short gObjDel(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return -1;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Connected == OBJECT_OFFLINE)
	{
		return -1;
	}

	if(MAP_RANGE(lpObj->Map) != 0)
	{
		gMap[lpObj->Map].DelStandAttr(lpObj->X,lpObj->Y);
		gMap[lpObj->Map].DelStandAttr(lpObj->OldX,lpObj->OldY);
	}

	if(lpObj->Type == OBJECT_USER)
	{
		if(lpObj->MapServerMoveQuit == 0)
		{
			gObjectManager.CharacterGameClose(aIndex);

			GJDisconnectAccountSend(aIndex,lpObj->Account,lpObj->IpAddr);
		}

		if(lpObj->Account[0] != 0){LogAddConnect(LOG_ALERT,"[Obj][%d] DelAccount (%s)",aIndex,lpObj->Account);}

		LogAddConnect(LOG_ALERT,"[Obj][%d] DelClient (%s)",aIndex,lpObj->IpAddr);

		memset(lpObj->Account,0,sizeof(lpObj->Account));

		memset(lpObj->PersonalCode,0,sizeof(lpObj->PersonalCode));

		gIpManager.RemoveIpAddress(lpObj->IpAddr);
	}

	lpObj->Connected = OBJECT_OFFLINE;

	gObjFreeData(aIndex);

	return aIndex;
}

LPOBJ gObjFind(char* name) // OK
{
	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0 && strcmp(gObj[n].Name,name) == 0)
		{
			return &gObj[n];
		}
	}

	return 0;
}

int gObjCalcDistance(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	return (int)sqrt(pow(((float)lpObj->X-(float)lpTarget->X),2)+pow(((float)lpObj->Y-(float)lpTarget->Y),2));
}
//**************************************************************************//
// OBJECT CHECK FUNCTIONS **************************************************//
//**************************************************************************//
bool gObjIsConnected(int aIndex) // OK
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

	return 1;
}

bool gObjIsConnectedGP(int aIndex) // OK
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

	if(lpObj->Type != OBJECT_USER  && lpObj->Type != OBJECT_BOTS  || (lpObj->CloseCount > 0 || lpObj->MapServerMoveQuit != 0))
	{
		return 0;
	}

	return 1;
}

bool gObjIsConnectedGS(int aIndex) // OK
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

	if(lpObj->Type == OBJECT_USER && lpObj->Type != OBJECT_BOTS && (lpObj->CloseCount > 0 || lpObj->MapServerMoveQuit != 0))
	{
		return 0;
	}

	return 1;
}

bool gObjIsNameValid(int aIndex,char* name) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Connected == OBJECT_OFFLINE)
	{
		return 0;
	}

	if(strcmp(name,"") == 0)
	{
		return 0;
	}

	if(strcmp(lpObj->Name,name) != 0)
	{
		return 0;
	}

	return 1;
}

bool gObjIsAccountValid(int aIndex,char* account) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Connected == OBJECT_OFFLINE)
	{
		return 0;
	}

	if(strcmp(account,"") == 0)
	{
		return 0;
	}

	if(strcmp(lpObj->Account,account) != 0)
	{
		return 0;
	}

	return 1;
}

bool gObjIsChangeSkin(int aIndex) // OK
{
	if(gObj[aIndex].Change < 0)
	{
		return 0;
	}

	if(gObj[aIndex].Change == gServerInfo.m_TransformationRing1 || gObj[aIndex].Change == gServerInfo.m_TransformationRing2 || gObj[aIndex].Change == gServerInfo.m_TransformationRing3 || gObj[aIndex].Change == gServerInfo.m_TransformationRing4 || gObj[aIndex].Change == gServerInfo.m_TransformationRing5 || gObj[aIndex].Change == gServerInfo.m_TransformationRing6)
	{
		return ((gObj[aIndex].ChangeSkin==0)?1:0);
	}

	if(gObj[aIndex].Change == 372 || gObj[aIndex].Change == 373 || gObj[aIndex].Change == 374 || gObj[aIndex].Change == 378 || gObj[aIndex].Change == 477 || gObj[aIndex].Change == 503 || gObj[aIndex].Change == 548 || gObj[aIndex].Change == 616 || gObj[aIndex].Change == 617 || gObj[aIndex].Change == 625 || gObj[aIndex].Change == 626 || gObj[aIndex].Change == 642)
	{
		return ((gObj[aIndex].ChangeSkin==0)?1:0);
	}

	return 1;
}

bool gObjCheckMaxMoney(int aIndex,DWORD AddMoney) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	if(((QWORD)gObj[aIndex].Money+(QWORD)AddMoney) > (QWORD)MAX_MONEY)
	{
		return 0;
	}

	return 1;
}

bool gObjCheckPersonalCode(int aIndex,char* PersonalCode) // OK
{
	if(gServerInfo.m_PersonalCodeCheck == 0)
	{
		return 1;
	}

	if(strncmp(PersonalCode,&gObj[aIndex].PersonalCode[6],7) == 0)
	{
		return 1;
	}

	return 0;
}

bool gObjCheckResistance(LPOBJ lpObj,int type) // OK
{
	BYTE resist = lpObj->Resistance[type];

	if(resist == 0xFF)
	{
		return 1;
	}

	if((lpObj->Authority & 32) != 0 && (lpObj->Inventory[10].m_Index == GET_ITEM(13,42) || lpObj->Inventory[11].m_Index == GET_ITEM(13,42)))
	{
		return 1;
	}

	if(gEffectManager.CheckEffect(lpObj,EFFECT_IRON_DEFENSE) != 0 || gEffectManager.CheckEffect(lpObj,EFFECT_IRON_DEFENSE_IMPROVED) != 0)
	{
		return 1;
	}

	if((type == 0 || type == 2) && gEffectManager.CheckEffect(lpObj,EFFECT_SOUL_POTION) != 0)
	{
		resist += (resist*50)/100;
	}

	if((GetLargeRand()%(resist+1)) == 0)
	{
		return 0;
	}

	return 1;
}

bool gObjCheckTeleportArea(int aIndex,int x,int y) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Live == 0 || lpObj->Teleport != 0)
	{
		return 0;
	}

	if(x < (lpObj->X-8) || x > (lpObj->X+8) || y < (lpObj->Y-8) || y > (lpObj->Y+8))
	{
		return 0;
	}

	if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) != 0 || gMap[lpObj->Map].CheckAttr(x,y,1) != 0)
	{
		return 0;
	}

	return 1;
}

bool gObjCheckMapTile(LPOBJ lpObj,int type) // OK
{
	if(lpObj->Type != OBJECT_USER)
	{
		return 0;
	}

	if(CC_MAP_RANGE(lpObj->Map) != 0 && gChaosCastle.GetState(GET_CC_LEVEL(lpObj->Map)) == CC_STATE_START)
	{
		return 0;
	}

	for(int x=0;x < 3;x++)
	{
		for(int y=0;y < 3;y++)
		{
			if(gMap[lpObj->Map].CheckAttr((lpObj->X+x),(lpObj->Y+y),4) == 0 && gMap[lpObj->Map].CheckAttr((lpObj->X+x),(lpObj->Y+y),8) == 0)
			{
				return 0;
			}
		}
	}

	switch(lpObj->Class)
	{
		case CLASS_DW:
			gObjMoveGate(lpObj->Index,17);
			break;
		case CLASS_DK:
			gObjMoveGate(lpObj->Index,17);
			break;
		case CLASS_FE:
			gObjMoveGate(lpObj->Index,27);
			break;
		case CLASS_MG:
			gObjMoveGate(lpObj->Index,17);
			break;
		case CLASS_DL:
			gObjMoveGate(lpObj->Index,17);
			break;
		case CLASS_SU:
			gObjMoveGate(lpObj->Index,267);
			break;
		case CLASS_RF:
			gObjMoveGate(lpObj->Index,17);
			break;
	}

	return 1;
}
//**************************************************************************//
// ITEM TRANSACTION FUNCTIONS **********************************************//
//**************************************************************************//
bool gObjFixInventoryPointer(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Inventory == lpObj->Inventory1)
	{
		return 1;
	}

	if(lpObj->Inventory == lpObj->Inventory2)
	{
		if(lpObj->Transaction == 1)
		{
			return 0;
		}
		else
		{
			for(int n=0;n < INVENTORY_SIZE;n++)
			{
				lpObj->Inventory2[n].Clear();
			}
		}
	}

	gObjSetInventory1Pointer(lpObj);
	return 0;
}

void gObjSetInventory1Pointer(LPOBJ lpObj) // OK
{
	lpObj->Inventory = lpObj->Inventory1;
	lpObj->InventoryMap = lpObj->InventoryMap1;
}

void gObjSetInventory2Pointer(LPOBJ lpObj) // OK
{
	lpObj->Inventory = lpObj->Inventory2;
	lpObj->InventoryMap = lpObj->InventoryMap2;
}

bool gObjFixEventInventoryPointer(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->EventInventory == lpObj->EventInventory1)
	{
		return 1;
	}

	if(lpObj->EventInventory == lpObj->EventInventory2)
	{
		if(lpObj->Transaction == 1)
		{
			return 0;
		}
		else
		{
			for(int n=0;n < EVENT_INVENTORY_SIZE;n++)
			{
				lpObj->EventInventory2[n].Clear();
			}
		}
	}

	gObjSetEventInventory1Pointer(lpObj);
	return 0;

	#else

	return 1;

	#endif
}

void gObjSetEventInventory1Pointer(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	lpObj->EventInventory = lpObj->EventInventory1;
	lpObj->EventInventoryMap = lpObj->EventInventoryMap1;

	#endif
}

void gObjSetEventInventory2Pointer(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	lpObj->EventInventory = lpObj->EventInventory2;
	lpObj->EventInventoryMap = lpObj->EventInventoryMap2;

	#endif
}

bool gObjInventoryTransaction(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Transaction == 1)
	{
		return 0;
	}

	for(int n=0;n < MAX_SKILL_LIST;n++)
	{
		lpObj->SkillBackup[n] = lpObj->Skill[n];
	}

	for(int n=0;n < INVENTORY_SIZE;n++)
	{
		lpObj->Inventory2[n] = lpObj->Inventory1[n];
	}

	#if(GAMESERVER_UPDATE>=802)

	for(int n=0;n < EVENT_INVENTORY_SIZE;n++)
	{
		lpObj->EventInventory2[n] = lpObj->EventInventory1[n];
	}

	#endif

	memcpy(lpObj->InventoryMap2,lpObj->InventoryMap1,INVENTORY_SIZE);

	#if(GAMESERVER_UPDATE>=802)

	memcpy(lpObj->EventInventoryMap2,lpObj->EventInventoryMap1,EVENT_INVENTORY_SIZE);

	#endif

	gObjSetInventory2Pointer(lpObj);

	gObjSetEventInventory2Pointer(lpObj);

	lpObj->Transaction = 1;
	return 1;
}

bool gObjInventoryCommit(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Transaction != 1)
	{
		return 0;
	}

	for(int n=0;n < INVENTORY_SIZE;n++)
	{
		lpObj->Inventory1[n] = lpObj->Inventory2[n];
	}

	#if(GAMESERVER_UPDATE>=802)

	for(int n=0;n < EVENT_INVENTORY_SIZE;n++)
	{
		lpObj->EventInventory1[n] = lpObj->EventInventory2[n];
	}

	#endif

	memcpy(lpObj->InventoryMap1,lpObj->InventoryMap2,INVENTORY_SIZE);

	#if(GAMESERVER_UPDATE>=802)

	memcpy(lpObj->EventInventoryMap1,lpObj->EventInventoryMap2,EVENT_INVENTORY_SIZE);

	#endif

	gObjSetInventory1Pointer(lpObj);

	gObjSetEventInventory1Pointer(lpObj);

	lpObj->Transaction = 2;
	return 1;
}

bool gObjInventoryRollback(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Transaction != 1)
	{
		return 0;
	}

	for(int n=0;n < MAX_SKILL_LIST;n++)
	{
		lpObj->Skill[n] = lpObj->SkillBackup[n];
	}

	for(int n=0;n < INVENTORY_SIZE;n++)
	{
		lpObj->Inventory2[n].Clear();
	}

	#if(GAMESERVER_UPDATE>=802)

	for(int n=0;n < EVENT_INVENTORY_SIZE;n++)
	{
		lpObj->EventInventory2[n].Clear();
	}

	#endif

	gObjSetInventory1Pointer(lpObj);

	gObjSetEventInventory1Pointer(lpObj);

	lpObj->Transaction = 3;
	return 1;
}
//**************************************************************************//
// VIEWPORT FUNCTIONS ******************************************************//
//**************************************************************************//
void gObjSetViewport(int aIndex,int state) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	for(int n=0;n < MAX_VIEWPORT;n++)
	{
		if(lpObj->VpPlayer[n].state == state)
		{
			switch(state)
			{
				case VIEWPORT_SEND:
					lpObj->VpPlayer[n].state = VIEWPORT_WAIT;
					break;
				case VIEWPORT_DESTROY:
					lpObj->VpPlayer[n].state = VIEWPORT_NONE;
					lpObj->VpPlayer[n].index = -1;
					lpObj->VPCount--;
					break;
			}
		}

		if(lpObj->VpPlayerItem[n].state == state)
		{
			switch(state)
			{
				case VIEWPORT_SEND:
					lpObj->VpPlayerItem[n].state = VIEWPORT_WAIT;
					break;
				case VIEWPORT_DESTROY:
					lpObj->VpPlayerItem[n].state = VIEWPORT_NONE;
					lpObj->VpPlayerItem[n].index = -1;
					lpObj->VPCountItem--;
					break;
			}
		}
	}
}

void gObjClearViewport(LPOBJ lpObj) // OK
{
	for(int n=0;n < MAX_VIEWPORT;n++)
	{
		lpObj->VpPlayer[n].state = VIEWPORT_NONE;
		lpObj->VpPlayer[n].index = -1;
		lpObj->VpPlayer2[n].state = VIEWPORT_NONE;
		lpObj->VpPlayer2[n].index = -1;
		lpObj->VpPlayerItem[n].state = VIEWPORT_NONE;
		lpObj->VpPlayerItem[n].index = -1;
	}

	lpObj->VPCount = 0;
	lpObj->VPCount2 = 0;
	lpObj->VPCountItem = 0;
}

void gObjViewportListProtocolDestroy(LPOBJ lpObj) // OK
{
	gViewport.GCViewportSimpleDestroySend(lpObj);
}

void gObjViewportListProtocolCreate(LPOBJ lpObj) // OK
{
	if(lpObj->Type == OBJECT_USER || lpObj->Type == OBJECT_BOTS) //MC
	{
		gViewport.GCViewportSimplePlayerSend(lpObj);
		gViewport.GCViewportSimpleChangeSend(lpObj);
		gViewport.GCViewportSimpleGuildSend(lpObj);
		gViewport.GCViewportSimpleGensSystemSend(lpObj);
	}
	else
	{
		gViewport.GCViewportSimpleMonsterSend(lpObj);
		gViewport.GCViewportSimpleSummonSend(lpObj);
	}
}

void gObjViewportListProtocol(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Connected != OBJECT_ONLINE)
	{
		return;
	}

	if(lpObj->Type == OBJECT_USER || lpObj->Type == OBJECT_BOTS) //MC
	{
		gViewport.GCViewportDestroySend(aIndex);
		gViewport.GCViewportDestroyItemSend(aIndex);
	}

	gObjSetViewport(aIndex,VIEWPORT_DESTROY);

	if(lpObj->Type == OBJECT_USER)
	{
		gViewport.GCViewportPlayerSend(aIndex);
		gViewport.GCViewportMonsterSend(aIndex);
		gViewport.GCViewportSummonSend(aIndex);
		gViewport.GCViewportItemSend(aIndex);
		gViewport.GCViewportChangeSend(aIndex);
		gViewport.GCViewportGuildSend(aIndex);
		gViewport.GCViewportGensSystemSend(aIndex);
	}

	gObjSetViewport(aIndex,VIEWPORT_SEND);
}

void gObjViewportListDestroy(int aIndex) // OK
{
	if(gObjIsConnected(aIndex) == 0)
	{
		return;
	}

	gViewport.DestroyViewportPlayer1(aIndex);

	gViewport.DestroyViewportPlayer2(aIndex);

	gViewport.DestroyViewportMonster1(aIndex);

	gViewport.DestroyViewportMonster2(aIndex);

	gViewport.DestroyViewportItem(aIndex);
}

void gObjViewportListCreate(int aIndex) // OK
{
	if(gObjIsConnected(aIndex) == 0)
	{
		return;
	}

	if(gObj[aIndex].RegenOk > 0)
	{
		return;
	}

	gViewport.CreateViewportPlayer(aIndex);

	gViewport.CreateViewportMonster(aIndex);

	gViewport.CreateViewportItem(aIndex);
}
//**************************************************************************//
// USER FUNCTIONS **********************************************************//
//**************************************************************************//
void gObjSetKillCount(int aIndex,int type) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(type == 0)
	{
		lpObj->KillCount = 0;
	}

	if(type == 1 && lpObj->KillCount < 255)
	{
		lpObj->KillCount++;
	}

	if(type == 2 && lpObj->KillCount > 0)
	{
		lpObj->KillCount--;
	}

	PMSG_KILL_COUNT_SEND pMsg;

	pMsg.header.set(0xB8,0x01,sizeof(pMsg));

	pMsg.count = lpObj->KillCount;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void gObjTeleportMagicUse(int aIndex,int x,int y) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Teleport != 0)
	{
		return;
	}

	lpObj->TeleportTime = GetTickCount();
	lpObj->PathCount = 0;
	lpObj->Teleport = 1;
	lpObj->ViewState = 1;
	lpObj->X = x;
	lpObj->Y = y;
	lpObj->TX = x;
	lpObj->TY = y;

	gMap[lpObj->Map].DelStandAttr(lpObj->OldX,lpObj->OldY);
	gMap[lpObj->Map].SetStandAttr(lpObj->TX,lpObj->TY);

	lpObj->OldX = lpObj->TX;
	lpObj->OldY = lpObj->TY;

	gObjViewportListProtocolDestroy(lpObj);
}

void gObjInterfaceCheckTime(LPOBJ lpObj) // OK
{
	if(lpObj->Interface.use == 0)
	{
		return;
	}

	if((GetTickCount()-lpObj->InterfaceTime) < 5000)
	{
		return;
	}

	if(lpObj->Interface.type == INTERFACE_TRADE)
	{
		if(lpObj->Interface.state == 0)
		{
			if(OBJECT_RANGE(lpObj->TargetNumber) != 0)
			{
				gObj[lpObj->TargetNumber].Interface.use = 0;
				gObj[lpObj->TargetNumber].Interface.type = INTERFACE_NONE;
				gObj[lpObj->TargetNumber].Interface.state = 0;
				gObj[lpObj->TargetNumber].TargetNumber = -1;

				gTrade.GCTradeResultSend(lpObj->TargetNumber,3);
			}

			lpObj->Interface.use = 0;
			lpObj->Interface.type = INTERFACE_NONE;
			lpObj->Interface.state = 0;
			lpObj->TargetNumber = -1;

			gTrade.GCTradeResultSend(lpObj->Index,3);
		}
	}

	if(lpObj->Interface.type == INTERFACE_PARTY)
	{
		if(lpObj->Interface.state == 0)
		{
			if(OBJECT_RANGE(lpObj->TargetNumber) != 0)
			{
				gObj[lpObj->TargetNumber].Interface.use = 0;
				gObj[lpObj->TargetNumber].Interface.type = INTERFACE_NONE;
				gObj[lpObj->TargetNumber].Interface.state = 0;
				gObj[lpObj->TargetNumber].TargetNumber = -1;
				gObj[lpObj->TargetNumber].PartyTargetUser = -1;

				gParty.GCPartyResultSend(lpObj->TargetNumber,0);
			}

			lpObj->Interface.use = 0;
			lpObj->Interface.type = INTERFACE_NONE;
			lpObj->Interface.state = 0;
			lpObj->TargetNumber = -1;
			lpObj->PartyTargetUser = -1;

			gParty.GCPartyResultSend(lpObj->Index,0);
		}
	}

	lpObj->InterfaceTime = GetTickCount();
}

void gObjSkillNovaCheckTime(LPOBJ lpObj) // OK
{
	if(lpObj->SkillNovaState == 0)
	{
		return;
	}

	if((GetTickCount()-lpObj->SkillNovaTime) < 500)
	{
		return;
	}

	lpObj->SkillNovaTime = GetTickCount();

	if((++lpObj->SkillNovaCount) == 12)
	{
		CSkill* lpSkill = gSkillManager.GetSkill(lpObj,SKILL_NOVA);

		if(lpSkill != 0)
		{
			gSkillManager.RunningSkill(lpObj->Index,0,lpSkill,0,0,0,0);
		}
	}
	else
	{
		PMSG_SKILL_NOVA_SEND pMsg;

		pMsg.header.set(0xBA,sizeof(pMsg));

		pMsg.index[0] = SET_NUMBERHB(lpObj->Index);

		pMsg.index[1] = SET_NUMBERLB(lpObj->Index);

		pMsg.type = SKILL_NOVA;

		pMsg.count = lpObj->SkillNovaCount;

		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		MsgSendV2(lpObj,(BYTE*)&pMsg,pMsg.header.size);
	}
}

void gObjPKDownCheckTime(LPOBJ lpObj,int TargetLevel) // OK
{
	if(lpObj->PKLevel == 3)
	{
		return;
	}

	lpObj->PKTime += TargetLevel;

	if(lpObj->PKLevel < 3)
	{
		if(lpObj->PKTime > gServerInfo.m_PKDownTime1)
		{
			if(lpObj->PKCount < 100)
			{
				lpObj->PKCount++;
			}

			lpObj->PKLevel++;

			lpObj->PKTime = 0;

			GCPKLevelSend(lpObj->Index,lpObj->PKLevel);
		}
	}
	else
	{
		if(lpObj->PKTime > gServerInfo.m_PKDownTime2)
		{
			if(lpObj->PKCount > 0)
			{
				lpObj->PKCount--;
			}

			if(lpObj->PKCount == 0)
			{
				lpObj->PKLevel = 3;
			}
			else if(lpObj->PKCount == 1)
			{
				lpObj->PKLevel = 4;
			}
			else if(lpObj->PKCount == 2)
			{
				lpObj->PKLevel = 5;
			}

			lpObj->PKTime = 0;

			GCPKLevelSend(lpObj->Index,lpObj->PKLevel);
		}
	}
}

void gObjUserDie(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	if(lpObj->Type != OBJECT_USER)
	{
		return;
	}

	gObjSetKillCount(lpObj->Index,0);

	if(CA_MAP_RANGE(lpObj->Map) != 0)
	{
		gCustomArena.UserDieProc(lpObj,lpTarget);
		return;
	}
	else if(DS_MAP_RANGE(lpObj->Map) != 0)
	{
		gDevilSquare.UserDieProc(lpObj,lpTarget);
		return;
	}
	else if(BC_MAP_RANGE(lpObj->Map) != 0)
	{
		gBloodCastle.UserDieProc(lpObj,lpTarget);
		return;
	}
	else if(CC_MAP_RANGE(lpObj->Map) != 0)
	{
		gChaosCastle.UserDieProc(lpObj,lpTarget);
		return;
	}
	else if(IT_MAP_RANGE(lpObj->Map) != 0)
	{
		gIllusionTemple.UserDieProc(lpObj,lpTarget);
		return;
	}
	else if(DG_MAP_RANGE(lpObj->Map) != 0)
	{
		gDoubleGoer.UserDieProc(lpObj,lpTarget);
		return;
	}
	else if(IG_MAP_RANGE(lpObj->Map) != 0)
	{
		gImperialGuardian.UserDieProc(lpObj,lpTarget);
		return;
	}

	if(gObjTargetGuildWarCheck(lpObj,lpTarget) != 0)
	{
		return;
	}

	if (lpObj->PvP == 1 && lpTarget->PvP == 1)
	{
		gEventPvP.EventPvPDead(lpObj->Index,lpTarget->Index);
		return;
	}

	if (lpObj->KillAll == 1 && lpTarget->KillAll == 1)
	{
		gEventKillAll.UserDieProc(lpObj,lpTarget);
		return;
	}

	if (gTvTEvent.CheckPlayerTarget(lpObj))
	{
		gTvTEvent.UserDieProc(lpObj,lpTarget);
	}

	if (gGvGEvent.CheckPlayerTarget(lpObj))
	{
		gGvGEvent.UserDieProc(lpObj,lpTarget);
	}

	if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER)
	{
		//GDKillSystemSend(lpTarget->Index,lpObj->Index);

		lpTarget->Kills++;
		lpObj->Deads++;

		gCustomRankUser.GCReqRankLevelUser(lpObj->Index, lpObj->Index);
		gCustomRankUser.GCReqRankLevelUser(lpTarget->Index, lpTarget->Index);

		LogAdd(LOG_BLACK,"[%s][%s] Kill [%s][%s]",lpTarget->Account,lpTarget->Name,lpObj->Account,lpObj->Name);

	}

	//Die System User
	if(gServerInfo.m_DieUserSwitch == 1 && lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER)
	{
		PostMessagePK(lpTarget->Name,gMessage.GetMessage(515),lpObj->Name);
	}
	//Die System Monster
	if(gServerInfo.m_DieMonsterSwitch == 1 && lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_MONSTER)
	{
		PostMessagePK(lpTarget->Name,gMessage.GetMessage(516),lpObj->Name);
	}

	int itemdrop = 1;
	int count = 24;
	int number = 0;
	int dropresult = 0;

	//Drop item system
	if(gServerInfo.m_PkItemDropSwitch == 1 && gServerInfo.m_PkItemDropEnable[lpObj->AccountLevel] == 1 && gServerInfo.m_PkItemDropRate >= rand()%100)
	{
		if(lpObj->PKLevel >= 6 && gMapManager.GetMapPkDropItem(lpObj->Map) == 1)
		{
			count = 24;

			while(count-- != 0)
			{
				number = rand()%12;

				if(lpObj->Inventory[number].IsItem()==1)
				{
						PMSG_ITEM_DROP_RECV lpMsg;
						lpMsg.slot = number;
						lpMsg.x = (BYTE)lpObj->X;
						lpMsg.y = (BYTE)lpObj->Y;

						if(gItemManager.CGPkDrop(&lpMsg,lpObj->Index) == 1)
						{
							LogAdd(LOG_USER,"[%s][%s] PK Drop, ItemName: %s",lpObj->Account,lpObj->Name,gItemManager.GetItemName(lpObj->Inventory[number].m_Index));
							dropresult = 1;
							break;
						}
				}
			}

			if(dropresult == 0)
			{
				count = INVENTORY_MAIN_SIZE;

				while(count-- != 0)
				{
					number = rand()%INVENTORY_MAIN_SIZE + 12;

					if(lpObj->Inventory[number].IsItem()==1)
					{
							if(lpObj->Inventory[number].m_Index >= GET_ITEM(13,20) && (lpObj->Inventory[number].m_Level >= 1 && lpObj->Inventory[number].m_Level <= 2 ))
							{
								continue;
							}

							PMSG_ITEM_DROP_RECV lpMsg;
							lpMsg.slot = number;
							lpMsg.x = (BYTE)lpObj->X;
							lpMsg.y = (BYTE)lpObj->Y;

							if(gItemManager.CGPkDrop(&lpMsg,lpObj->Index) == 1)
							{
								dropresult = 1;
								LogAdd(LOG_USER,"[%s][%s] Pk Drop, ItemName: %s",lpObj->Account,lpObj->Name,gItemManager.GetItemName(lpObj->Inventory[number].m_Index));
								break;
							}
					}
				}
			}

		}
	}

	gDuel.UserDieProc(lpObj,lpTarget);

	gGensSystem.UserDieProc(lpObj,lpTarget);
}

void gObjPlayerKiller(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	if(lpObj->Type != OBJECT_USER || lpTarget->Type != OBJECT_USER)
	{
		return;
	}

	if(lpObj->Authority == 32 || lpTarget->Authority == 32)
	{
		return;
	}

	if(gDuel.CheckDuel(lpObj,lpTarget) != 0)
	{
		return;
	}

	if(lpObj->PvP != 0 && lpTarget->PvP != 0)
	{
		return;
	}

	if(lpObj->KillAll != 0 && lpTarget->KillAll != 0)
	{
		return;
	}

	if(gObjGetRelationShip(lpObj,lpTarget) == 2)
	{
		return;
	}

	if(gObjTargetGuildWarCheck(lpObj,lpTarget) != 0)
	{
		return;
	}

	if(gGensSystem.CheckGens(lpObj,lpTarget) != 0)
	{
		return;
	}

	if (gTvTEvent.CheckPlayerTarget(lpObj) && gTvTEvent.CheckPlayerTarget(lpTarget))
	{
		return;
	}

	if (gGvGEvent.CheckPlayerTarget(lpObj) && gGvGEvent.CheckPlayerTarget(lpTarget))
	{
		return;
	}

	#if(GAMESERVER_TYPE==1)

	if(lpObj->Map == MAP_CASTLE_SIEGE)
	{
		if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE && lpObj->CsJoinSide != 0)
		{
			return;
		}
		else if(lpObj->Y > 113)
		{
			if(gCastleSiege.CheckCastleOwnerUnionMember(lpObj->Index) != 0)
			{
				return;
			}
		}

		if(lpTarget->PKLevel >= 6)
		{
			return;
		}
	}

	#endif

	if(CA_MAP_RANGE(lpObj->Map) != 0 && CA_MAP_RANGE(lpTarget->Map) != 0)
	{
		return;
	}

	if(CC_MAP_RANGE(lpObj->Map) != 0 && CC_MAP_RANGE(lpTarget->Map) != 0)
	{
		return;
	}

	if(IT_MAP_RANGE(lpObj->Map) != 0 && IT_MAP_RANGE(lpTarget->Map) != 0)
	{
		return;
	}

	if(gMapManager.GetMapNonOutlaw(lpObj->Map) != 0)
	{
		return;
	}

	for(int n=0;n < MAX_SELF_DEFENSE;n++)
	{
		if(OBJECT_RANGE(lpTarget->SelfDefense[n]) != 0)
		{
			if(lpObj->Index == lpTarget->SelfDefense[n])
			{
				return;
			}
		}
	}

	if(lpObj->PKLevel > 3)
	{
		if(lpTarget->PKLevel > 4)
		{
			return;
		}

		if(lpObj->PKCount < 100)
		{
			lpObj->PKCount++;
		}
	}
	else
	{
		if(lpTarget->PKLevel < 5)
		{
			lpObj->PKCount = 1;
		}
		else //if(lpObj->PKCount > -3)
		{
			lpObj->PKCount--;
		}
	}

	if(lpObj->PKCount <= -3)
	{
		lpObj->PKLevel = 1;
	}
	else if(lpObj->PKCount == 0)
	{
		lpObj->PKLevel = 3;
	}
	else if(lpObj->PKCount == 1)
	{
		lpObj->PKLevel = 4;
	}
	else if(lpObj->PKCount == 2)
	{
		lpObj->PKLevel = 5;
	}
	else if(lpObj->PKCount >= 3)
	{
		lpObj->PKLevel = 6;
	}

	lpObj->PKTime = 0;
	
	GCPKLevelSend(lpObj->Index,lpObj->PKLevel);
}

BOOL gObjMoveGate(int aIndex,int gate) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Type == OBJECT_USER)
	{
		gDarkSpirit[aIndex].SetMode(DARK_SPIRIT_MODE_NORMAL,-1);
	}

	if(lpObj->SkillSummonPartyTime != 0)
	{
		lpObj->SkillSummonPartyTime = 0;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(272));
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

	if(lpObj->RegenOk != 0 || gGate.IsGate(gate) == 0)
	{
		goto ERROR_JUMP;
	}

	int TargetGate,map,x,y,dir,level;

	if(gGate.GetGate(gate,&TargetGate,&map,&x,&y,&dir,&level) == 0)
	{
		/*
		lpObj->State = OBJECT_DELCMD;
		lpObj->RegenOk = 1;
		gMove.GCTeleportSend(aIndex,gate,lpObj->Map,(BYTE)lpObj->X,(BYTE)lpObj->Y,lpObj->Dir);
		return 0;
		*/
		goto ERROR_JUMP;
	}

	if(lpObj->PShopOpen != 0 && ((lpObj->Map == MAP_CASTLE_SIEGE && map == MAP_LORENCIA) || (lpObj->Map == MAP_LORENCIA && map == MAP_CASTLE_SIEGE)))
	{
		goto ERROR_JUMP;
	}

	if(lpObj->GensFamily == 0 && gMapManager.GetMapGensBattle(map) != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(229));
		goto ERROR_JUMP;
	}

	if(BC_MAP_RANGE(map) != 0 || CC_MAP_RANGE(map) != 0 || DS_MAP_RANGE(map) != 0 || IT_MAP_RANGE(map) != 0 || DG_MAP_RANGE(map) != 0 || IG_MAP_RANGE(map) != 0)
	{
		gEffectManager.DelEffect(lpObj,EFFECT_ELF_BUFFER);
	}

	#if(GAMESERVER_TYPE==1)

	if(TargetGate == 97)
	{
		if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)
		{
			if(lpObj->CsJoinSide != 1 && gCastleSiege.GetCastleTowerAccessable() == 0)
			{
				goto ERROR_JUMP;
			}

			if(lpObj->CsJoinSide == 0)
			{
				goto ERROR_JUMP;
			}
		}
		else
		{
			if(gCastleSiege.CheckCastleOwnerMember(lpObj->Index) == 0 && gCastleSiege.CheckCastleOwnerUnionMember(lpObj->Index) == 0)
			{
				goto ERROR_JUMP;
			}
		}
	}

	#endif

	if(map == MAP_ATLANS && lpObj->Inventory[8].IsItem() != 0 && (lpObj->Inventory[8].m_Index == GET_ITEM(13,2) || lpObj->Inventory[8].m_Index == GET_ITEM(13,3))) // Uniria,Dinorant
	{
		gNotice.GCNoticeSend(aIndex,1,0,0,0,0,0,gMessage.GetMessage(274));
		goto ERROR_JUMP;
	}

	if((map == MAP_ICARUS || map == MAP_KANTURU3) && lpObj->Inventory[7].IsItem() == 0 && lpObj->Inventory[8].m_Index != GET_ITEM(13,3) && lpObj->Inventory[8].m_Index != GET_ITEM(13,37))
	{
		gNotice.GCNoticeSend(aIndex,1,0,0,0,0,0,gMessage.GetMessage(275));
		goto ERROR_JUMP;
	}

	if(KALIMA_MAP_RANGE(lpObj->Map) == 0 && lpObj->Map != map)
	{
		gKalima.DeleteKalimaGate(aIndex);
	}

	if(KALIMA_MAP_RANGE(lpObj->Map) != 0 && lpObj->Map != map)
	{
		gKalima.DeleteKalimaGate(aIndex);
	}

	if(map == MAP_RAKLION2)
	{
		if(gRaklion.GetRaklionState() == RAKLION_STATE_CLOSE_DOOR || gRaklion.GetRaklionState() == RAKLION_STATE_ALL_USER_DIE || gRaklion.GetRaklionState() == RAKLION_STATE_NOTIFY4 || gRaklion.GetRaklionState() == RAKLION_STATE_END)
		{
			gNotice.GCNoticeSend(aIndex,1,0,0,0,0,0,gMessage.GetMessage(325));
			goto ERROR_JUMP;
		}
		else
		{
			if(gate == 290)
			{
				gRaklionBattleUserMng.AddUserData(aIndex);
			}
		}
	}

	if(map == MAP_RAKLION1)
	{
		if(gate == 292)
		{
			if(gRaklion.GetRaklionState() == RAKLION_STATE_CLOSE_DOOR || gRaklion.GetRaklionState() == RAKLION_STATE_ALL_USER_DIE || gRaklion.GetRaklionState() == RAKLION_STATE_NOTIFY4 || gRaklion.GetRaklionState() == RAKLION_STATE_END)
			{
				goto ERROR_JUMP;
			}
			else
			{
				gRaklionBattleUserMng.DelUserData(aIndex);
			}
		}
		else
		{
			gRaklionUtil.NotifyRaklionState(aIndex,gRaklion.GetRaklionState(),gRaklion.GetRaklionDetailState());
		}
	}

	if(lpObj->Interface.use != 0 || lpObj->DieRegen != 0)
	{
		lpObj->State = OBJECT_DELCMD;
		lpObj->RegenOk = 1;
		gMove.GCTeleportSend(aIndex,gate,lpObj->Map,(BYTE)lpObj->X,(BYTE)lpObj->Y,lpObj->Dir);
		return 0;
	}

	lpObj->State = OBJECT_DELCMD;

	if(lpObj->Map != map)
	{
		short NextServerCode = gMapServerManager.CheckMapServerMove(aIndex,map,lpObj->LastServerCode);

		if(NextServerCode != gServerInfo.m_ServerCode)
		{
			if(NextServerCode == -1)
			{
				return 0;
			}

			GJMapServerMoveSend(aIndex,NextServerCode,map,x,y);
			return 1;
		}
	}

	lpObj->X = x;
	lpObj->Y = y;
	lpObj->TX = x;
	lpObj->TY = y;
	lpObj->Map = map;
	lpObj->Dir = dir;
	lpObj->PathCount = 0;
	lpObj->Teleport = 0;
	lpObj->ViewState = 0;
	lpObj->MiniMapState = 0;
	lpObj->MiniMapValue = -1;

	gHelper.DisableHelper(lpObj);

	gObjClearViewport(lpObj);

	gMove.GCTeleportSend(aIndex,gate,lpObj->Map,(BYTE)lpObj->X,(BYTE)lpObj->Y,lpObj->Dir);

	gObjViewportListProtocolCreate(lpObj);

	gObjectManager.CharacterUpdateMapEffect(lpObj);

	lpObj->RegenMapNumber = lpObj->Map;
	lpObj->RegenMapX = (BYTE)lpObj->X;
	lpObj->RegenMapY = (BYTE)lpObj->Y;
	lpObj->RegenOk = 1;

	if(lpObj->Type == OBJECT_USER)
	{
		lpObj->LastTeleportTime = 10;
	}

	return 1;

	ERROR_JUMP:

	gObjClearViewport(lpObj);

	gMove.GCTeleportSend(aIndex,gate,lpObj->Map,(BYTE)lpObj->X,(BYTE)lpObj->Y,lpObj->Dir);

	gObjViewportListProtocolCreate(lpObj);

	gObjectManager.CharacterUpdateMapEffect(lpObj);

	lpObj->RegenMapNumber = lpObj->Map;
	lpObj->RegenMapX = (BYTE)lpObj->X;
	lpObj->RegenMapY = (BYTE)lpObj->Y;
	lpObj->RegenOk = 1;
	return 0;
}

void gObjTeleport(int aIndex,int map,int x,int y) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return;
	}

	if(MAP_RANGE(map) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	lpObj->State = OBJECT_DELCMD;

	if(lpObj->Map != map)
	{
		short NextServerCode = gMapServerManager.CheckMapServerMove(aIndex,map,lpObj->LastServerCode);

		if(NextServerCode != gServerInfo.m_ServerCode)
		{
			if(NextServerCode == -1)
			{
				return;
			}

			GJMapServerMoveSend(aIndex,NextServerCode,map,x,y);
			return;
		}
	}

	if(lpObj->Interface.use != 0 && lpObj->Interface.type == INTERFACE_TRADE)
	{
		gTrade.CGTradeCancelButtonRecv(aIndex);
	}

	if(lpObj->Interface.use != 0 && lpObj->Interface.type == INTERFACE_WAREHOUSE)
	{
		gWarehouse.CGWarehouseClose(aIndex);
	}

	lpObj->X = x;
	lpObj->Y = y;
	lpObj->TX = x;
	lpObj->TY = y;
	lpObj->Map = map;
	lpObj->PathCount = 0;
	lpObj->Teleport = 0;
	lpObj->ViewState = 0;
	lpObj->MiniMapState = 0;
	lpObj->MiniMapValue = -1;

	gHelper.DisableHelper(lpObj);

	gObjClearViewport(lpObj);

	gMove.GCTeleportSend(aIndex,1,lpObj->Map,(BYTE)lpObj->X,(BYTE)lpObj->Y,lpObj->Dir);

	gObjViewportListProtocolCreate(lpObj);

	gObjectManager.CharacterUpdateMapEffect(lpObj);

	lpObj->RegenMapNumber = lpObj->Map;
	lpObj->RegenMapX = (BYTE)lpObj->X;
	lpObj->RegenMapY = (BYTE)lpObj->Y;
	lpObj->RegenOk = 1;
}

void gObjSummonAlly(LPOBJ lpObj,int map,int x,int y) // OK
{
	lpObj->SkillSummonPartyTime = 0;
	lpObj->SkillSummonPartyMap = 0;
	lpObj->SkillSummonPartyX = 0;
	lpObj->SkillSummonPartyY = 0;

	if(lpObj->Map == map)
	{
		gSkillManager.GCSkillAttackSend(lpObj,SKILL_TELEPORT,lpObj->Index,1);
		gObjTeleportMagicUse(lpObj->Index,x,y);
		return;
	}

	lpObj->X = x;
	lpObj->Y = y;
	lpObj->TX = x;
	lpObj->TY = y;
	lpObj->Map = map;
	lpObj->PathCount = 0;
	lpObj->Teleport = 0;
	lpObj->ViewState = 0;
	lpObj->MiniMapState = 0;
	lpObj->MiniMapValue = -1;

	gHelper.DisableHelper(lpObj);

	gObjClearViewport(lpObj);

	gMove.GCTeleportSend(lpObj->Index,-1,lpObj->Map,(BYTE)lpObj->X,(BYTE)lpObj->Y,lpObj->Dir);

	gObjViewportListProtocolCreate(lpObj);

	gObjectManager.CharacterUpdateMapEffect(lpObj);

	lpObj->RegenMapNumber = lpObj->Map;
	lpObj->RegenMapX = (BYTE)lpObj->X;
	lpObj->RegenMapY = (BYTE)lpObj->Y;
	lpObj->RegenOk = 1;
}

void gObjSkillUseProc(LPOBJ lpObj) // OK
{
	CMonsterSkillElementOption::CheckSkillElementOptionProc(lpObj);

	if(lpObj->Type == OBJECT_USER && lpObj->SkillSummonPartyTime > 0)
	{
		if((--lpObj->SkillSummonPartyTime) == 0)
		{
			if(gMoveSummon.CheckMoveSummon(lpObj,lpObj->SkillSummonPartyMap,lpObj->SkillSummonPartyX,lpObj->SkillSummonPartyY) == 0)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(272));
			}
			else
			{
				gObjSummonAlly(lpObj,lpObj->SkillSummonPartyMap,lpObj->SkillSummonPartyX,lpObj->SkillSummonPartyY);
			}
		}
		else
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(273),lpObj->SkillSummonPartyTime);
		}
	}

	if(lpObj->Type == OBJECT_USER && lpObj->DrinkSpeed > 0)
	{
		if(GetTickCount() >= lpObj->DrinkLastTime)
		{
			lpObj->DrinkSpeed = 0;
			lpObj->DrinkLastTime = 0;

			gObjectManager.CharacterCalcAttribute(lpObj->Index);
		}
	}
}

void gObjUserKill(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->CloseCount <= 0)
	{
		lpObj->CloseType = 0;
		lpObj->CloseCount = 1;
	}
}

bool gObjInventorySearchSerialNumber(LPOBJ lpObj,DWORD serial) // OK
{
	int count = 0;

	for(int n=0;n < INVENTORY_SIZE;n++)
	{
		if(lpObj->Inventory[n].m_Serial != 0 && lpObj->Inventory[n].m_Serial == serial && (count++) > 0)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(482));
			gObjUserKill(lpObj->Index);
			return 0;
		}
	}

	return 1;
}

bool gObjWarehouseSearchSerialNumber(LPOBJ lpObj,DWORD serial) // OK
{
	int count = 0;

	for(int n=0;n < WAREHOUSE_SIZE;n++)
	{
		if(lpObj->Warehouse[n].m_Serial != 0 && lpObj->Warehouse[n].m_Serial == serial && (count++) > 0)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(482));
			gObjUserKill(lpObj->Index);
			return 0;
		}
	}

	return 1;
}

void gObjAddMsgSend(LPOBJ lpObj,int MsgCode,int SendUser,int SubCode) // OK
{
	for(int n=0;n < MAX_MONSTER_SEND_MSG;n++)
	{
		if(gSMMsg[lpObj->Index][n].MsgCode == -1)
		{
			if(gSMMsg.ObjectStruct[lpObj->Index] == gSMMsg.CommonStruct){gSMMsg.ObjectStruct[lpObj->Index] = new MESSAGE_STATE_MACHINE_COMMON;}

			if(gSMMsg.ObjectStruct[lpObj->Index]->ObjectStruct[n] == gSMMsg.ObjectStruct[lpObj->Index]->CommonStruct){gSMMsg.ObjectStruct[lpObj->Index]->ObjectStruct[n] = new MESSAGE_STATE_MACHINE;}

			gSMMsg[lpObj->Index][n].MsgCode = MsgCode;
			gSMMsg[lpObj->Index][n].MsgTime = GetTickCount();
			gSMMsg[lpObj->Index][n].SendUser = SendUser;
			gSMMsg[lpObj->Index][n].SubCode = SubCode;
			return;
		}
	}
}

void gObjAddMsgSendDelay(LPOBJ lpObj,int MsgCode,int SendUser,int MsgTimeDelay,int SubCode) // OK
{
	for(int n=0;n < MAX_MONSTER_SEND_MSG;n++)
	{
		if(gSMMsg[lpObj->Index][n].MsgCode == -1)
		{
			if(gSMMsg.ObjectStruct[lpObj->Index] == gSMMsg.CommonStruct){gSMMsg.ObjectStruct[lpObj->Index] = new MESSAGE_STATE_MACHINE_COMMON;}

			if(gSMMsg.ObjectStruct[lpObj->Index]->ObjectStruct[n] == gSMMsg.ObjectStruct[lpObj->Index]->CommonStruct){gSMMsg.ObjectStruct[lpObj->Index]->ObjectStruct[n] = new MESSAGE_STATE_MACHINE;}

			gSMMsg[lpObj->Index][n].MsgCode = MsgCode;
			gSMMsg[lpObj->Index][n].MsgTime = GetTickCount()+MsgTimeDelay;
			gSMMsg[lpObj->Index][n].SendUser = SendUser;
			gSMMsg[lpObj->Index][n].SubCode = SubCode;
			return;
		}
	}
}

void gObjAddAttackProcMsgSendDelay(LPOBJ lpObj,int MsgCode,int SendUser,int MsgTimeDelay,int SubCode,int SubCode2) // OK
{
	for(int n=0;n < MAX_MONSTER_SEND_ATTACK_MSG;n++)
	{
		if(gSMAttackProcMsg[lpObj->Index][n].MsgCode == -1)
		{
			if(gSMAttackProcMsg.ObjectStruct[lpObj->Index] == gSMAttackProcMsg.CommonStruct){gSMAttackProcMsg.ObjectStruct[lpObj->Index] = new MESSAGE_STATE_ATTACK_MACHINE_COMMON;}

			if(gSMAttackProcMsg.ObjectStruct[lpObj->Index]->ObjectStruct[n] == gSMAttackProcMsg.ObjectStruct[lpObj->Index]->CommonStruct){gSMAttackProcMsg.ObjectStruct[lpObj->Index]->ObjectStruct[n] = new MESSAGE_STATE_ATTACK_MACHINE;}

			gSMAttackProcMsg[lpObj->Index][n].MsgCode = MsgCode;
			gSMAttackProcMsg[lpObj->Index][n].MsgTime = GetTickCount()+MsgTimeDelay;
			gSMAttackProcMsg[lpObj->Index][n].SendUser = SendUser;
			gSMAttackProcMsg[lpObj->Index][n].SubCode = SubCode;
			gSMAttackProcMsg[lpObj->Index][n].SubCode2 = SubCode2;
			return;
		}
	}
}

//**************************************************************************//
// RAW FUNCTIONS ***********************************************************//
//**************************************************************************//

void gObjSecondProc()
{
	int n;
	LPOBJ lpObj;

	for(n = 0; n < MAX_OBJECT;n++)
	{
		lpObj = &gObj[n];

		if(lpObj->Connected > OBJECT_LOGGED)
		{
			if(lpObj->MapServerMoveQuit == 1)
			{
				if(GetTickCount() - lpObj->MapServerMoveQuitTickCount > 30000)
				{
					gObjDel(lpObj->Index);
					continue;
				}
			}

			gObjSkillUseProc(lpObj);

			if(lpObj->Type == OBJECT_MONSTER)
			{
				#if(GAMESERVER_TYPE==1)
				if(lpObj->Class == 283)
				{
					gGuardianStatue.GuardianStatueAct(lpObj->Index);
					continue;
				}
				
				if(lpObj->Class == 278)
				{
					gLifeStone.LifeStoneAct(lpObj->Index);
					continue;
				}

				if(lpObj->Class == 288)
				{
					gCannonTower.CannonTowerAct(lpObj->Index);
					continue;
				}

				gCrywolf.CrywolfMonsterAct(lpObj->Index);
				#endif
				if(lpObj->MonsterDeleteTime != 0 && GetTickCount() >= lpObj->MonsterDeleteTime)
				{
					gObjDel(lpObj->Index);
					continue;
				}
			}
			if(lpObj->Type == OBJECT_NPC)
			{
				#if(GAMESERVER_TYPE==1)
				if( (lpObj->Class < 204)?FALSE:(lpObj->Class > 209)?FALSE:TRUE)
				{
					gCrywolf.CrywolfNpcAct(lpObj->Index);
				}
				
				if(lpObj->Class == 216)
				{
					gCastleSiegeCrown.CastleSiegeCrownAct(lpObj->Index);
					continue;
				}
				
				if(lpObj->Class == 217 || lpObj->Class == 218)
				{
					gCastleSiegeCrownSwitch.CastleSiegeCrownSwitchAct(lpObj->Index);
					continue;
				}
				#endif
				if(lpObj->Class == 221 || lpObj->Class == 222)
				{
					gCastleSiegeWeapon.CastleSiegeWeaponAct(lpObj->Index);
				}
			}

			if(lpObj->Type == OBJECT_USER)
			{
				gCustomAttack.OnAttackSecondProc(lpObj);

				gCustomStore.OnPShopSecondProc(lpObj);

				gObjCheckMapTile(lpObj,3);

				//GCNewHealthBarSend(lpObj);

				if(lpObj->ChatLimitTime > 0)
				{
					lpObj->ChatLimitTimeSec++;

					if(lpObj->ChatLimitTimeSec > 60)
					{
						lpObj->ChatLimitTimeSec = 0;
						lpObj->ChatLimitTime--;

						if(lpObj->ChatLimitTime < 1)
						{
							lpObj->ChatLimitTime = 0;
							gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(483));
						}
					}
				}

				#if(GAMESERVER_TYPE==1)

				if(lpObj->Map == MAP_CASTLE_SIEGE && gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)
				{
					if((lpObj->X < 150 || lpObj->X > 200) || (lpObj->Y < 175 || lpObj->Y > 225))
					{
						if(lpObj->AccumulatedCrownAccessTime > 0)
						{
							lpObj->AccumulatedCrownAccessTime = (((lpObj->AccumulatedCrownAccessTime-gServerInfo.m_CastleSiegeDecayAccumulatedTimeValue)<0)?0:(lpObj->AccumulatedCrownAccessTime-gServerInfo.m_CastleSiegeDecayAccumulatedTimeValue));
						}
					}
				}

				#endif

				gObjectManager.CharacterAutoRecuperation(lpObj);
	
				if(lpObj->Type == OBJECT_USER && lpObj->LastTeleportTime > 0)
				{
					lpObj->LastTeleportTime--;
				}
	
				gObjDelayLifeCheck(n);

				gObjectManager.CharacterItemDurationDown(lpObj);

				if(lpObj->PartyNumber >= 0)
				{
					gParty.GCPartyLifeSend(lpObj->PartyNumber);

					#if(GAMESERVER_UPDATE>=802)

					gMiniMap.GCMiniMapPartyInfoSend(lpObj->Index);

					#endif
				}

				#if(GAMESERVER_UPDATE>=801)

				gGuildMatching.GDGuildMatchingInsertSaveSend(lpObj->Index);

				gPartyMatching.GDPartyMatchingInsertSaveSend(lpObj->Index);

				#endif

				int BattleGround = 1;
				int m_BattleTimer = gCheckBattleGroundTimer();
	
				if((lpObj->Authority & 2) == 2)
				{
					BattleGround = 0;
					GCTimeViewSend(lpObj->Index,m_BattleTimer);
				}
	
				if(lpObj->Map == MAP_ARENA && BattleGround == 1)
				{
					BattleGround = 1;
	
					if(lpObj->Guild != 0)
					{
						if(lpObj->Guild->WarState != 0)
						{
							int CheckBattleGround = gCheckBattleGround(lpObj);
	
							if(CheckBattleGround != lpObj->Guild->BattleGroundIndex)
							{
								if(lpObj->Guild->WarType == 1)
								{
									BattleGround = 0;
								}
								else
								{
									BattleGround = 0;
								}
							}
							else
							{
								BattleGround = 0;
							}
						}
					}
					else
					{
						BattleGround = 1;
					}
	
					if(BattleGround != 0)
					{
						int CheckBattleGround = gCheckBattleGround(lpObj);
		
						if(CheckBattleGround >= 0)
						{
							gObjMoveGate(lpObj->Index,17);
						}
					}
				}
			}
		}

		if(lpObj->Connected >= OBJECT_LOGGED && lpObj->Type == OBJECT_USER && lpObj->CloseCount > 0)
		{

			if(lpObj->CloseCount == 1)
			{
				if(lpObj->CloseType == 1)
				{
					if(gObjectManager.CharacterGameClose(lpObj->Index) == 1)
					{
						GCCloseClientSend(lpObj->Index,1);
					}
				}
				else if(lpObj->CloseType == 3)
				{
					if(gObjectManager.CharacterGameClose(lpObj->Index) == 1)
					{
						GCCloseClientSend(lpObj->Index,1);
					}
				}
				else if(lpObj->CloseType == 4)
				{
					if(gObjectManager.CharacterGameClose(lpObj->Index) == 1)
					{
						GCCloseClientSend(lpObj->Index,1);
					}
				}
				else if(lpObj->CloseType == 0)
				{
					GCCloseClientSend(lpObj->Index,0);
				}
				else if(lpObj->CloseType == 2)
				{
					GCCloseClientSend(lpObj->Index,2);
				}
			}
			else
			{
				if(lpObj->CloseType == 3)
				{
					gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(859),(lpObj->CloseCount-1));
				}
				else if(lpObj->CloseType == 4)
				{
					gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(858),(lpObj->CloseCount-1));
				}
				else
				{
					gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(484),(lpObj->CloseCount-1));
				}
			}
			lpObj->CloseCount--;
		}

		if(lpObj->Connected > OBJECT_LOGGED &&	lpObj->Type == OBJECT_USER)
		{
			if(GetTickCount() - lpObj->AutoSaveTime > 600000)
			{
				GDCharacterInfoSaveSend(lpObj->Index);
				lpObj->AutoSaveTime = GetTickCount();
			}

			if(lpObj->CheckSumTime > 0 && GetTickCount() - lpObj->CheckSumTime > 5000)
			{
				LogAdd(LOG_BLACK,"[%s][%s] CheckSumTime Error",lpObj->Account, lpObj->Name);
				GCCloseClientSend(n,0);
			}

			gObjPKDownCheckTime(lpObj,1);
			gObjInterfaceCheckTime(lpObj);
			gObjTimeCheckSelfDefense(lpObj);
		}

		if(lpObj->Connected == OBJECT_CONNECTED ||	lpObj->Connected == OBJECT_LOGGED || lpObj->Connected == OBJECT_ONLINE)
		{
			if(lpObj->Type == OBJECT_USER)
			{
				if(lpObj->Connected >= OBJECT_LOGGED)
				{
					if(GetTickCount() - lpObj->ConnectTickCount > 60000)
					{
						LogAdd(LOG_BLACK,"Game response error causes conclusion [%d][%s][%s][%s]",lpObj->Index,lpObj->Account,lpObj->Name,lpObj->IpAddr);
						CloseClient(n);
						gObjDel(lpObj->Index);
					}
				}
				else
				{
					if(GetTickCount() - lpObj->ConnectTickCount > 30000)
					{
						LogAdd(LOG_BLACK,"Response error after connection causes conclusion [%d][%s][%s][%s]",lpObj->Index,lpObj->Account,lpObj->Name,lpObj->IpAddr);
						CloseClient(n);
						gObjDel(lpObj->Index);
					}
				}
			}
		}
	}
}

void gObjDelayLifeCheck(int aIndex)
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->CheckLifeTime > 0)
	{
		lpObj->CheckLifeTime--;

		if(lpObj->CheckLifeTime <= 0)
		{
			lpObj->CheckLifeTime = 0;

			if(lpObj->Life < 0)
			{
				lpObj->Life = 0;
			}

			if(lpObj->AttackObj != 0)
			{
				gObjectManager.CharacterLifeCheck(lpObj->AttackObj,lpObj,0,1,0,0,0,0);
			}
		}
	}
}

BOOL gObjBackSpring(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	LPOBJ lpTargetObj = lpTarget;
	int tdir;

	if (MAP_RANGE(lpObj->Map) == FALSE )
	{
		return FALSE;
	}

	if ( lpObj->Type == OBJECT_USER )
	{
		if (lpObj->Teleport != 0)
		{
			return FALSE;
		}
	}

#if(GAMESERVER_TYPE==1)
	if( lpObj->Type == OBJECT_USER)
	{
		if((lpObj->Authority &0x02) == 2)
		{
			return FALSE;
		}
	}
#endif

	if ( lpObj->Class >= 131 && lpObj->Class <= 134 )
	{
		return FALSE;
	}

#if(GAMESERVER_TYPE==1)
	if( lpObj->Class == 277 ||
		lpObj->Class == 283 ||
		lpObj->Class == 288 ||
		lpObj->Class == 278 ||
		lpObj->Class == 215 ||
		lpObj->Class == 216 ||
		lpObj->Class == 217 ||
		lpObj->Class == 218 ||
		lpObj->Class == 219 )
	{
		return FALSE;
	}
	
	if(gCastleSiege.GetCrownUserIndex() == lpObj->Index)
	{
		if(lpObj->Inventory[8].m_Index == GET_ITEM(13,4))
		{
			return FALSE;
		}
	}
	
	if(gCrywolf.GetCrywolfState() == 4 || gCrywolf.GetCrywolfState() == 3)
	{
		if(lpObj->Type == OBJECT_USER)
		{
			for(int i=205;i<=209;i++)
			{
				int iAltarIndex = gCrywolfAltar.GetAltarUserIndex(i);
				
				if(iAltarIndex != -1)
				{
					if(iAltarIndex == lpObj->Index)
					{
						return FALSE;
					}
				}
			}
		}
	}

	if(lpObj->Class >= 204 && lpObj->Class <= 209)
	{
		return FALSE;
	}
	if(lpObj->Class == 348)
	{
		return FALSE;
	}
#endif

	if(lpObj->Class == 275)
	{
		return FALSE;
	}

	if(lpObj->Class == 459 ||
		lpObj->Class == 460 ||
		lpObj->Class == 461 ||
		lpObj->Class == 462)
	{
		return FALSE;
	}

	if(lpObj->Class == 524 || lpObj->Class == 525 || lpObj->Class == 527 || lpObj->Class == 528)
	{
		return FALSE;
	}
	//GAMBI FIX
	/*
	if(gEffectManager.CheckEffect(lpObj,EFFECT_ICE_ARROW) != 0 || gEffectManager.CheckEffect(lpObj,EFFECT_STERN) != 0 || gEffectManager.CheckEffect(lpObj,EFFECT_SLEEP) != 0)
	{
		return FALSE;
	}
	*/
	
	if ( (GetLargeRand()%3) == 0 )
	{
		if ( lpTargetObj->Dir < 4 )
		{
			tdir = lpTargetObj->Dir + 4;
		}
		else
		{
			tdir = lpTargetObj->Dir - 4;
		}

		tdir *= 2;
	}
	else
	{
		tdir = lpTargetObj->Dir * 2;
	}

	int x;
	int y;
	BYTE attr;

	x = lpObj->X;
	y = lpObj->Y;
	x += RoadPathTable[tdir];
	y += RoadPathTable[1+tdir];
	attr = gMap[lpObj->Map].GetAttr(x, y);

	if ( (attr&1)==1 ||
		(attr&2)==2 || 
		(attr&4)== 4 || 
		(attr&8)== 8 
		||(attr&16)== 16 
		)
	{
		return FALSE;
	}

	PMSG_POSITION_RECV pMsg;
	pMsg.x = x;
	pMsg.y = y;

	lpObj->Rest = 0;
	gMap[lpObj->Map].DelStandAttr(lpObj->OldX, lpObj->OldY);
	gMap[lpObj->Map].SetStandAttr(x, y);
	lpObj->OldX = x;
	lpObj->OldY = y;
	CGPositionRecv(&pMsg,lpObj->Index);

	if ( lpObj->Type == OBJECT_USER )
	{
		lpObj->PathCount = 0;
	}

	return TRUE;
}

BOOL BackSpringCheck(int & x, int & y, int & dir, BYTE map)
{
	BYTE attr;
	int tx = x;
	int ty = y;
	int tdir = dir/2;

	tx = tx + RoadPathTable[dir];
	ty = ty + RoadPathTable[1+dir];

	attr = gMap[map].GetAttr(tx, ty);

	if	(  (attr&1)==1 || 
		(attr&4)== 4 || 
		(attr&8)== 8
		||(attr&16)== 16 
		)
	{
		tdir += 4;
		
		if ( tdir > 7 )
		{
			tdir -= 8;
		}
		
		dir = tdir * 2;
		return FALSE;
	}

	x = tx;
	y = ty;

	return TRUE;
}

BOOL gObjBackSpring2(LPOBJ lpObj, LPOBJ lpTargetObj, int count)
{
	int tdir;

	if ( MAP_RANGE(lpObj->Map) == FALSE )
	{
		return FALSE;
	}

	if ( lpObj->Type == OBJECT_USER )
	{
		if (lpObj->Teleport != 0)
		{
			return FALSE;
		}
	}

#if(GAMESERVER_TYPE==1)
	if( lpObj->Type == OBJECT_USER)
	{
		if((lpObj->Authority &0x02) == 2)
		{
			return FALSE;
		}
	}
#endif

	if ( lpObj->Class == 287 || lpObj->Class == 286 )
	{
		return FALSE;
	}

#if(GAMESERVER_TYPE==1)
	if ( lpObj->Class == 278 )
	{
		return FALSE;
	}
#endif

	if ( lpObj->Class >= 131 && lpObj->Class <= 134 )
	{
		return FALSE;
	}

#if(GAMESERVER_TYPE==1)
	if( lpObj->Class == 277 ||
		lpObj->Class == 283 ||
		lpObj->Class == 288 ||
		lpObj->Class == 278 ||
		lpObj->Class == 215 ||
		lpObj->Class == 216 ||
		lpObj->Class == 217 ||
		lpObj->Class == 218 ||
		lpObj->Class == 219 )
	{
		return FALSE;
	}
	
	if(gCastleSiege.GetCrownUserIndex() == lpObj->Index)
	{
		if(lpObj->Inventory[8].m_Index == GET_ITEM(13,4))
		{
			return FALSE;
		}
	}
	
	if(gCrywolf.GetCrywolfState() == 4 || gCrywolf.GetCrywolfState() == 3)
	{
		if(lpObj->Type == OBJECT_USER)
		{
			for(int i=205;i<=209;i++)
			{
				int iAltarIndex = gCrywolfAltar.GetAltarUserIndex(i);
				
				if(iAltarIndex != -1)
				{
					if(iAltarIndex == lpObj->Index)
					{
						return FALSE;
					}
				}
			}
		}
	}
	
	if(lpObj->Class >= 204 && lpObj->Class <= 209)
	{
		return FALSE;
	}
	if(lpObj->Class == 348)
	{
		return FALSE;
	}
#endif
	
	if(lpObj->Class == 275)
	{
		return FALSE;
	}

	if(lpObj->Class == 459 ||
		lpObj->Class == 460 ||
		lpObj->Class == 461 ||
		lpObj->Class == 462)
	{
		return FALSE;
	}

	if(lpObj->Class == 524 || lpObj->Class == 525 || lpObj->Class == 527 || lpObj->Class == 528)
	{
		return FALSE;
	}

	////GAMBI FIX
	/*
	if(gEffectManager.CheckEffect(lpObj,EFFECT_ICE_ARROW) != 0 || gEffectManager.CheckEffect(lpObj,EFFECT_STERN) != 0 || gEffectManager.CheckEffect(lpObj,EFFECT_SLEEP) != 0)
	{
		return FALSE;
	}
	*/

#if(GAMESERVER_TYPE==1)
	tdir = GetPathPacketDirPos(lpObj->X - lpTargetObj->X, lpObj->Y - lpTargetObj->Y) * 2;
#else
	tdir = GetPathPacketDirPos(lpObj->X - lpTargetObj->X, lpObj->Y - lpTargetObj->Y) * 2;
#endif

	int x = lpObj->X;
	int y = lpObj->Y;

	for ( int n=0;n<count;n++)
	{
		if ( n >= 2 )
		{
			if ( lpObj->Class == 4 || lpObj->Class == 3 || lpObj->Class == 1 )
			{
				BackSpringCheck(x, y, tdir, lpObj->Map);
			}
		}
		else
		{
			BackSpringCheck(x, y, tdir, lpObj->Map);
		}
	}

	PMSG_POSITION_RECV pMsg;
	pMsg.x = x;
	pMsg.y = y;

	lpObj->Rest = 0;
	gMap[lpObj->Map].DelStandAttr(lpObj->OldX, lpObj->OldY);
	gMap[lpObj->Map].SetStandAttr(x, y);
	lpObj->OldX = x;
	lpObj->OldY = y;
	CGPositionRecv(&pMsg,lpObj->Index);

	return TRUE;
}

bool gObjIsSelfDefense(LPOBJ lpObj, int aTargetIndex)
{
	if ( !OBJECT_RANGE(aTargetIndex) )
	{
		return false;
	}

	for ( int n=0;n<MAX_SELF_DEFENSE;n++)
	{
		if ( lpObj->SelfDefense[n] >= 0 )
		{
			if ( lpObj->SelfDefense[n] == aTargetIndex )
			{
				return true;
			}
		}
	}

	return false;
}

void gObjCheckSelfDefense(LPOBJ lpObj, int aTargetIndex)
{
	int n;

	if(!OBJECT_RANGE(aTargetIndex))
	{
		return;
	}

	if(gObj[aTargetIndex].PKLevel > 4)
	{
		return;
	}

	int iPartyNumber = lpObj->PartyNumber;
	int loc3 = -1;

	for(n = 0; n < MAX_SELF_DEFENSE;n++)
	{
		if(gObj[aTargetIndex].SelfDefense[n] >= 0)
		{
			if(gObj[aTargetIndex].SelfDefense[n]==lpObj->Index)
			{
				return;
			}

			if(iPartyNumber >= 0)
			{
				if(gObj[gObj[aTargetIndex].SelfDefense[n]].PartyNumber == iPartyNumber)
				{
					return;
				}
			}
		}
	}

	int blank = -1;

	for(n = 0; n < MAX_SELF_DEFENSE;n++)
	{
		if(lpObj->SelfDefense[n] >= 0)
		{
			if(lpObj->SelfDefense[n] == aTargetIndex)
			{
				lpObj->SelfDefenseTime[n] = GetTickCount()+60000;
				return;
			}
		}
		else
		{
			blank = n;
		}
	}

	if(blank < 0)
	{
		return;
	}

	lpObj->MySelfDefenseTime = GetTickCount();
	lpObj->SelfDefense[blank] = aTargetIndex;
	lpObj->SelfDefenseTime[blank] = GetTickCount() + 60000;

	char szTemp[64];

	wsprintf(szTemp,gMessage.GetMessage(485),lpObj->Name,gObj[aTargetIndex].Name);

	iPartyNumber = gObj[aTargetIndex].PartyNumber;

	if(iPartyNumber >= 0)
	{
		int iPartyNumIndex = -1;

		for(n = 0; n < MAX_PARTY_USER; n++)
		{
			iPartyNumIndex = gParty.m_PartyInfo[iPartyNumber].Index[n];

			if( iPartyNumIndex >= 0  )
			{
				gNotice.GCNoticeSend(iPartyNumIndex,1,0,0,0,0,0,szTemp);
			}
		}
	}
	else
	{
		gNotice.GCNoticeSend(aTargetIndex,1,0,0,0,0,0,szTemp);
	}

	gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,szTemp);

	//LogAdd(LOG_BLACK,"[%s][%s] Set SelfDefence [%s][%s]",lpObj->Account,lpObj->Name,gObj[aTargetIndex].Account,gObj[aTargetIndex].Name);
}

void gObjTimeCheckSelfDefense(LPOBJ lpObj)
{
	char szTemp[64];

	for ( int n=0;n<MAX_SELF_DEFENSE;n++)
	{
		if ( lpObj->SelfDefense[n] >= 0 )
		{
			if ( GetTickCount() > lpObj->SelfDefenseTime[n] )
			{
				wsprintf(szTemp, gMessage.GetMessage(486), lpObj->Name);
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,szTemp);
				gNotice.GCNoticeSend(lpObj->SelfDefense[n],1,0,0,0,0,0,szTemp);
				//LogAdd(LOG_BLACK,"[%s][%s] ReSet SelfDefence [%s][%s]", lpObj->Account, lpObj->Name, gObj[lpObj->SelfDefense[n]].Account, gObj[lpObj->SelfDefense[n]].Name);
				lpObj->SelfDefense[n] = -1;
			}
		}
	}
}

BOOL gObjTargetGuildWarCheck(LPOBJ lpObj, LPOBJ lpTargetObj)
{
	if(lpObj->Type != OBJECT_USER)
	{
		return false;
	}

	if(lpTargetObj->Type != OBJECT_USER)
	{
		return false;
	}

	if(lpObj->GuildNumber < 1 || lpTargetObj->GuildNumber < 1)
	{
		return false;
	}

	if(lpObj->Guild->WarState != 1 || lpTargetObj->Guild->WarState != 1)
	{
		return false;
	}

	if(strcmp(lpObj->Guild->TargetGuildName,lpTargetObj->Guild->Name))
	{
		return false;
	}
	return true;
}

void gObjGuildWarEndSend(LPOBJ lpObj, BYTE Result1, BYTE Result2)
{
	if(lpObj == 0)
	{
		return;
	}

	if(lpObj->Guild == 0)
	{
		return;
	}

	if(lpObj->Guild->WarState != 1)
	{
		return;
	}

	if(lpObj->Guild->TargetGuildNode == 0)
	{
		return;
	}

	if(Result1 == 1)
	{
		gObjGuildWarItemGive(lpObj->Guild,lpObj->Guild->TargetGuildNode);
	}
	else if(Result1 == 3)
	{
		gObjGuildWarItemGive(lpObj->Guild->TargetGuildNode,lpObj->Guild);
	}
	else if(Result1 == 1)
	{
		gObjGuildWarItemGive(lpObj->Guild->TargetGuildNode,lpObj->Guild);
	}

	for(int n = 0; n < MAX_GUILD_USER;n++)
	{
		if(lpObj->Guild->Use[n] && lpObj->Guild->Index[n] >= 0)
		{
			gObj[lpObj->Guild->Index[n]].IsInBattleGround = 0;
			gGuild.GCGuildWarEndSend(lpObj->Guild->Index[n],Result1,lpObj->Guild->TargetGuildNode->Name);
		}
	}

	for(int n = 0; n < MAX_GUILD_USER;n++)
	{
		if(lpObj->Guild->TargetGuildNode->Use[n] && lpObj->Guild->TargetGuildNode->Index[n] >= 0)
		{
			gObj[lpObj->Guild->TargetGuildNode->Index[n]].IsInBattleGround = 0;
			gGuild.GCGuildWarEndSend(lpObj->Guild->TargetGuildNode->Index[n],Result2,lpObj->Guild->Name);
		}
	}
}

void gObjGuildWarEndSend2(GUILD_INFO_STRUCT * lpGuild1, GUILD_INFO_STRUCT * lpGuild2, BYTE Result1, BYTE Result2)
{
	if(lpGuild1 == 0 || lpGuild2 == 0)
	{
		return;
	}

	if(lpGuild1->WarState != 1 || lpGuild2->WarState != 1)
	{
		return;
	}

	for(int n = 0; n < MAX_GUILD_USER;n++)
	{
		if(lpGuild1->Use[n] && lpGuild1->Index[n] >= 0)
		{
			gObj[lpGuild1->Index[n]].IsInBattleGround = 0;
			gGuild.GCGuildWarEndSend(lpGuild1->Index[n],Result1,lpGuild2->Name);
		}
	}

	for(int n = 0; n < MAX_GUILD_USER;n++)
	{
		if(lpGuild2->Use[n] && lpGuild2->Index[n] >= 0)
		{
			gObj[lpGuild2->Index[n]].IsInBattleGround = 0;
			gGuild.GCGuildWarEndSend(lpGuild2->Index[n],Result2,lpGuild1->Name);
		}
	}
}

void gObjGuildWarEnd(GUILD_INFO_STRUCT * lpGuild, GUILD_INFO_STRUCT * lpTargetGuild)
{
	if(lpGuild == 0 || lpTargetGuild == 0)
	{
		return;
	}

	lpGuild->WarDeclareState = 0;
	lpGuild->WarState = 0;

	if(lpTargetGuild != 0)
	{
		lpTargetGuild->WarDeclareState = 0;
		lpTargetGuild->WarState = 0;

		if(lpGuild->TargetGuildNode != 0)
		{
			GUILD_INFO_STRUCT * lpTguild = lpGuild->TargetGuildNode;

			lpGuild->TargetGuildNode = 0;
			lpTguild->TargetGuildNode = 0;
		}
	}
	else
	{
		lpGuild->TargetGuildNode = 0;
	}
}

BOOL gObjGuildWarProc(GUILD_INFO_STRUCT * lpGuild1, GUILD_INFO_STRUCT * lpGuild2, int score)
{
	int totalscore;
	int n;
	int maxscore;
	char szTemp[0x100];

	if(lpGuild1 == 0 || lpGuild2 == 0)
	{
		return false;
	}

	totalscore = 1;
	maxscore = 20;

	lpGuild1->PlayScore += score;

	wsprintf(szTemp,"%s ( %d ) VS %s ( %d )",lpGuild1->Name,lpGuild1->PlayScore,lpGuild2->Name,lpGuild2->PlayScore);
	LogAdd(LOG_BLACK,szTemp);

	if(lpGuild1->WarType == 1)
	{
		SetBattleTeamScore(lpGuild1->BattleGroundIndex,lpGuild1->BattleTeamCode,lpGuild1->PlayScore);
		maxscore = 100;
	}
	else
	{
		maxscore = 20;
	}

	if(lpGuild1->PlayScore >= maxscore)
	{
		if(lpGuild1->PlayScore > maxscore && lpGuild2->PlayScore == 0)
		{
			totalscore = 3;
		}
		else if(lpGuild1->PlayScore > maxscore && lpGuild2->PlayScore <= 10)
		{
			totalscore = 2;
		}

		lpGuild1->TotalScore += totalscore;

		DGGuildScoreUpdate(lpGuild1->Name,lpGuild1->TotalScore);

		DGGuildScoreUpdate(lpGuild2->Name,lpGuild2->TotalScore);

		return true;
	}

	for(n = 0; n < MAX_GUILD_USER; n++)
	{
		if(lpGuild1->Use[n] && lpGuild1->Index[n] >= 0)
		{
			gGuild.GCGuildWarScoreSend(lpGuild1->Index[n]);
		}
	}

	for(n = 0; n < MAX_GUILD_USER; n++)
	{
		if(lpGuild2->Use[n] && lpGuild2->Index[n] >= 0)
		{
			gGuild.GCGuildWarScoreSend(lpGuild2->Index[n]);
		}
	}
	return false;
}

BOOL gObjGuildWarCheck(LPOBJ lpObj, LPOBJ lpTargetObj)
{
	int score = 1;

	if(gObjTargetGuildWarCheck(lpObj,lpTargetObj)==0)
	{
		return false;
	}

	if(strcmp(lpTargetObj->Name,lpTargetObj->Guild->Names[0])==0)
	{
		score = 2;
	}

	GUILD_INFO_STRUCT * lpGuild = lpObj->Guild;
	GUILD_INFO_STRUCT * lpTargetGuild = lpTargetObj->Guild;

	if(gObjGuildWarProc(lpGuild,lpTargetGuild,score)==1)
	{
		if(lpGuild->WarType == 1)
		{
			gObjGuildWarEndSend2(lpGuild,lpTargetGuild,1,0);
			gBattleGroundEnable(lpGuild->BattleGroundIndex,0);
			gObjGuildWarEnd(lpGuild,lpTargetGuild);
		}
		else
		{
			gObjGuildWarEndSend(lpObj,1,0);
			gObjAddMsgSendDelay(lpObj,4,lpObj->Index,2000,0);
		}
	}
	return true;
}

BOOL gObjGuildWarMasterClose(LPOBJ lpObj)
{
	if(lpObj->GuildNumber < 1)
	{
		return false;
	}

	if(strcmp(lpObj->Guild->Names[0],lpObj->Name))
	{
		return false;
	}

	gObjGuildWarEndSend(lpObj,3,2);

	if(lpObj->Guild->WarType == 1 && lpObj->Guild->BattleGroundIndex < 1)
	{
		gBattleGroundEnable(lpObj->Guild->BattleGroundIndex,0);
	}
	gObjGuildWarEnd(lpObj->Guild,lpObj->Guild->TargetGuildNode);
	return true;
}

int gObjGuildWarItemGive(GUILD_INFO_STRUCT * lpWinGuild, GUILD_INFO_STRUCT * lpLoseGuild)
{
	return true;
	int n;
	int r_userindex[MAX_GUILD_USER];
	int r_usercount;

	for(n = 0; n < MAX_GUILD_USER; n++)
	{
		if(lpLoseGuild->Use[n] > 0)
		{
			r_userindex[r_usercount] = lpLoseGuild->Index[n];
			r_usercount++;
		}
	}

	if(r_usercount < 1)
	{
		return false;
	}

	int lose_user = r_userindex[GetLargeRand()%r_usercount];

	if(lose_user < 0)
	{
		return false;
	}

	if(gObj[lose_user].Connected < OBJECT_ONLINE)
	{
		return false;
	}

	r_usercount = 0;

	for(n = 0; n < MAX_GUILD_USER;n++)
	{
		if(lpWinGuild->Use[n] > 0)
		{
			r_userindex[r_usercount] = lpWinGuild->Index[n];
			r_usercount++;
		}
	}

	if(r_usercount < 1)
	{
		return false;
	}

	int win_user = r_userindex[GetLargeRand()%r_usercount];

	if(win_user < 0)
	{
		return false;
	}

	if(gObj[win_user].Connected < OBJECT_ONLINE)
	{
		return false;
	}

	LPOBJ lpObj = &gObj[lose_user];

	int count = 24;

	int number;

	while(count--)
	{
		number = GetLargeRand()%12;

		if(lpObj->Inventory[number].IsItem()==1)
		{
			if(gItemManager.InventoryInsertItem(win_user,lpObj->Inventory[number]) != 0xFF)
			{
				return true;
			}
			break;
		}
	}

	count = 64;

	while(count--)
	{
		number = GetLargeRand()%64+12;

		if(lpObj->Inventory[number].IsItem()==1)
		{
			if(gItemManager.InventoryInsertItem(win_user,lpObj->Inventory[number]) != 0xFF)
			{
				return true;
			}
			break;
		}
	}
	return true;
}

void gObjSetPosition(int aIndex, int x, int y)
{
	LPOBJ lpObj = &gObj[aIndex];

	PMSG_POSITION_RECV pMove;
	pMove.x = x;
	pMove.y = y;

	lpObj->Rest = 0;

	gMap[lpObj->Map].DelStandAttr(lpObj->OldX,lpObj->OldY);
	gMap[lpObj->Map].SetStandAttr(x,y);

	lpObj->OldX = x;
	lpObj->OldY = y;

	CGPositionRecv(&pMove,lpObj->Index);
}

void gObjAuthorityCodeSet(LPOBJ lpObj)
{
	if ( (lpObj->Authority&1) == 1 )
	{
		return;
	}

	if ( (lpObj->Authority&2)== 2 )
	{
		lpObj->AuthorityCode = -1;
		return;
	}

	if ( (lpObj->Authority&4) == 4 )
	{
		return;
	}

	if ( (lpObj->Authority&8) == 8 )
	{
		lpObj->AuthorityCode |= 1;
		lpObj->AuthorityCode |= 2;
		lpObj->AuthorityCode |= 4;
		lpObj->AuthorityCode |= 8;
		lpObj->AuthorityCode |= 16;
		lpObj->AuthorityCode |= 32;

		return;
	}

	if ( (lpObj->Authority&16) == 16 )
	{
		return;
	}

	if ( (lpObj->Authority&32) == 32 )
	{
		lpObj->AuthorityCode |= 1;
		lpObj->AuthorityCode |= 2;
		lpObj->AuthorityCode |= 4;
		lpObj->AuthorityCode |= 8;
		lpObj->AuthorityCode |= 16;
		lpObj->AuthorityCode |= 32;

		return;
	}

}

int gObjGetGuildUnionNumber(LPOBJ lpObj)
{
	int iUnion=0;

	if ( lpObj->Guild != NULL )
	{
		iUnion = (lpObj->Guild->GuildUnion == 0)? lpObj->Guild->Number : lpObj->Guild->GuildUnion;
	}

	return iUnion;
}

void gObjGetGuildUnionName(LPOBJ lpObj, char* szUnionName, int iUnionNameLen)
{
	szUnionName[0] = 0;
	
	if ( lpObj->Guild == NULL )
	{
		return;
	}

	if ( lpObj->Guild->GuildUnion == 0 )
	{
		return;
	}

	CUnionInfo * pUnionInfo = gUnionManager.SearchUnion(lpObj->Guild->GuildUnion);

	if ( pUnionInfo != NULL )
	{
		memcpy(szUnionName, pUnionInfo->m_szMasterGuild, iUnionNameLen);
	}
}

BOOL gObjCheckRival(LPOBJ lpObj, LPOBJ lpTargetObj)
{
	if(lpTargetObj->Type != OBJECT_USER)
	{
		return false;
	}

	if(lpObj->Type != OBJECT_USER)
	{
		return false;
	}

	if(lpTargetObj->Guild == 0 || lpObj->Guild == 0)
	{
		return false;
	}

	if(lpTargetObj->Guild->GuildRival == 0 || lpObj->Guild->GuildRival == 0)
	{
		return false;
	}

	int iUnion = !lpObj->Guild->GuildUnion ? lpObj->Guild->Number : lpObj->Guild->GuildUnion;

	if(gUnionManager.GetGuildRelationShip(iUnion,lpTargetObj->Guild->Number) == 2)
	{
		return true;
	}
	return false;
}

int gObjGetRelationShip(LPOBJ lpObj, LPOBJ lpTargetObj)
{
	if ( lpObj == NULL || lpTargetObj == NULL )
	{
		return false;
	}

	if ( lpObj->Guild == NULL || lpTargetObj->Guild == NULL )
	{
		return 0;
	}

	int iUnion = (!lpObj->Guild->GuildUnion) ? lpObj->Guild->Number : lpObj->Guild->GuildUnion;

	return gUnionManager.GetGuildRelationShip(iUnion, lpTargetObj->Guild->Number);
}

void gObjNotifyUpdateUnionV1(LPOBJ lpObj)
{
	if(lpObj == 0)
	{
		return;
	}

	char cBUFFER_V1[6000];
	int iVp1Count = 0;

	memset(cBUFFER_V1,0x00,sizeof(cBUFFER_V1));

	PMSG_UNION_VIEWPORT_NOTIFY_COUNT * lpMsg = (PMSG_UNION_VIEWPORT_NOTIFY_COUNT *)&cBUFFER_V1;
	PMSG_UNION_VIEWPORT_NOTIFY * lpMsgBody = (PMSG_UNION_VIEWPORT_NOTIFY * )&cBUFFER_V1[sizeof(PMSG_UNION_VIEWPORT_NOTIFY_COUNT)];

	for(int n = 0; n < MAX_VIEWPORT; n++)
	{
		if(!OBJECT_RANGE(lpObj->VpPlayer[n].index))
		{
			continue;
		}

		if(lpObj->VpPlayer[n].state == 2 && lpObj->VpPlayer[n].type == OBJECT_USER)
		{
			LPOBJ lpTargetObj = &gObj[lpObj->VpPlayer[n].index];

			if(lpTargetObj == 0)
			{
				continue;
			}

			if(lpTargetObj->Guild == 0)
			{
				continue;
			}

			gObjGetGuildUnionName(lpTargetObj,lpMsgBody[iVp1Count].szUnionName,sizeof(lpMsgBody[iVp1Count].szUnionName));

			lpMsgBody[iVp1Count].btGuildRelationShip = gObjGetRelationShip(lpObj,lpTargetObj);
			lpMsgBody[iVp1Count].btNumberL = SET_NUMBERLB(WORD(lpTargetObj->Index));
			lpMsgBody[iVp1Count].btNumberH = SET_NUMBERHB(WORD(lpTargetObj->Index));
			lpMsgBody[iVp1Count].iGuildNumber = lpTargetObj->Guild->Number;
			iVp1Count++;
		}
	}

	if(iVp1Count > 0 && iVp1Count <= MAX_VIEWPORT)
	{
		lpMsg->btCount = iVp1Count;

		lpMsg->h.set(0x67,iVp1Count * sizeof(PMSG_UNION_VIEWPORT_NOTIFY) + sizeof(PMSG_UNION_VIEWPORT_NOTIFY_COUNT));

		DataSend(lpObj->Index,(LPBYTE)lpMsg,((lpMsg->h.size[1] & 0xFF) & 0xFF | ((lpMsg->h.size[0] & 0xFF) & 0xFF) << 8) & 0xFFFF);
	}
	else
	{
		if(iVp1Count != 0)
		{
			LogAdd(LOG_BLACK,"[Union ViewPort] ERROR : iVp1Count is OUT of BOUND: %d",iVp1Count);
		}
	}
}

void gObjNotifyUpdateUnionV2(LPOBJ lpObj)
{
	if(lpObj == 0)
	{
		return;
	}

	if(lpObj->Guild == 0)
	{
		return;
	}

	char cBUFFER_V2[100] = {0};

	PMSG_UNION_VIEWPORT_NOTIFY_COUNT * lpMsg2 = (PMSG_UNION_VIEWPORT_NOTIFY_COUNT *)&cBUFFER_V2;
	PMSG_UNION_VIEWPORT_NOTIFY * lpMsgBody2 = (PMSG_UNION_VIEWPORT_NOTIFY *)&cBUFFER_V2[sizeof(PMSG_UNION_VIEWPORT_NOTIFY_COUNT)];

	gObjGetGuildUnionName(lpObj,lpMsgBody2->szUnionName,sizeof(lpMsgBody2->szUnionName));
	lpMsgBody2->btNumberL = SET_NUMBERLB(WORD(lpObj->Index));
	lpMsgBody2->btNumberH = SET_NUMBERHB(WORD(lpObj->Index));

	lpMsgBody2->iGuildNumber = lpObj->Guild->Number;
	lpMsgBody2->btGuildRelationShip = 0;
	lpMsg2->btCount = 1;

	lpMsg2->h.set(0x67,sizeof(lpMsg2) + sizeof(lpMsgBody2[0])+1);

	DataSend(lpObj->Index,(LPBYTE)lpMsg2,((lpMsg2->h.size[1] & 0xFF) & 0xFF | ((lpMsg2->h.size[0] & 0xFF) & 0xFF) << 8) & 0xFFFF);

	for(int n = 0; n < MAX_VIEWPORT; n++)
	{
		if(lpObj->VpPlayer2[n].type == OBJECT_USER && lpObj->VpPlayer2[n].state != 0)
		{
			LPOBJ lpTargetObj = &gObj[lpObj->VpPlayer2[n].index];

			if(lpTargetObj->Guild != 0)
			{
				lpMsgBody2->btGuildRelationShip = gObjGetRelationShip(lpTargetObj,lpObj);
			}

			if(lpMsgBody2->btGuildRelationShip != 1)
			{
				DataSend(lpObj->VpPlayer2[n].index,(LPBYTE)lpMsg2,((lpMsg2->h.size[1] & 0xFF) & 0xFF | ((lpMsg2->h.size[0] & 0xFF) & 0xFF) << 8) & 0xFFFF);
			}
		}
	}
}

void gObjUnionUpdateProc(int iIndex)
{
	if ( gObjIsConnected(iIndex) == FALSE )
	{
		return;
	}

	LPOBJ lpObj = &gObj[iIndex];

	if ( lpObj->Type != OBJECT_USER )
	{
		return;
	}

	if ( lpObj->RegenOk > 0 )
	{
		return;
	}

	if ( lpObj->CloseCount > -1 )
	{
		return;
	}

	if ( lpObj->Guild == NULL )
	{
		return;
	}

	if ( lpObj->Guild->CheckTimeStamp(lpObj->GuildUnionTimeStamp) != FALSE )
	{
		return;
	}

	lpObj->GuildUnionTimeStamp = lpObj->Guild->GetTimeStamp();
	gObjNotifyUpdateUnionV1(lpObj);
	gObjNotifyUpdateUnionV2(lpObj);
}

bool gObjRebuildMasterSkillTree(LPOBJ lpObj)
{
	if(gMasterSkillTree.CheckMasterLevel(lpObj) == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_SKILL_LIST;n++)
	{
		if(lpObj->Skill[n].IsMasterSkill() != 0)
		{
			if(lpObj->Skill[n].m_skill >= 300)
			{
				lpObj->Skill[n].Clear();
				continue;
			}

			lpObj->Skill[n].m_level = 0;

			lpObj->Skill[n].m_skill = lpObj->Skill[n].m_skill;

			lpObj->Skill[n].m_index = lpObj->Skill[n].m_skill;

			lpObj->Skill[n].m_DamageMin = gSkillManager.GetSkillDamage(lpObj->Skill[n].m_skill);

			lpObj->Skill[n].m_DamageMax = lpObj->Skill[n].m_DamageMin+(lpObj->Skill[n].m_DamageMin/2);
		}
	}

	#if(GAMESERVER_UPDATE>=602)

	for(int n=0;n < MAX_MASTER_SKILL_LIST;n++)
	{
		lpObj->MasterSkill[n].Clear();
	}

	#endif

	lpObj->MasterPoint = (lpObj->MasterLevel * gServerInfo.m_MasterSkillTreePoint);

	#if(GAMESERVER_UPDATE>=602)

	gMasterSkillTree.GCMasterSkillListSend(lpObj->Index);

	#endif

	gSkillManager.GCSkillListSend(lpObj,0);

	gObjectManager.CharacterCalcAttribute(lpObj->Index);

	gMasterSkillTree.GCMasterInfoSend(lpObj);

	return 1;
}

void gObjUseDrink(LPOBJ lpObj,int level)
{
	switch ( level )
	{
	case 2: // Remedy of Love
		GCItemUseSpecialTimeSend(lpObj->Index, 1, 90);
		break;
	case 1: //ALE+1
		GCItemUseSpecialTimeSend(lpObj->Index, 0, 180);
		lpObj->DrinkSpeed = 20;
		lpObj->DrinkLastTime = GetTickCount()+(180*1000);
		gObjectManager.CharacterCalcAttribute(lpObj->Index);
		break;
	default: //ALE+0
		GCItemUseSpecialTimeSend(lpObj->Index, 0, 80);
		lpObj->DrinkSpeed = 20;
		lpObj->DrinkLastTime = GetTickCount()+(80*1000);
		gObjectManager.CharacterCalcAttribute(lpObj->Index);
		break;
  }
}

void gObjCustomLogPlusChaosMix(LPOBJ lpObj,int type,int index)
{
	#if(GAMESERVER_UPDATE>=501)

	if(type != 5)
	{
		return;
	}

	#else

	if(type != 3)
	{
		return;
	}

	#endif

	if(gServerInfo.m_AnnounceChaosMix != 0)
	{
		ITEM_INFO ItemInfo;

		if(gItemManager.GetInfo(index,&ItemInfo) != 0)
		{
			char buff[256];

			wsprintf(buff,gServerInfo.m_AnnounceChaosMixText,lpObj->Name,ItemInfo.Name);

			if(gServerInfo.m_AnnounceChaosMix == 1)
			{
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,buff);
			}
			else
			{
				GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(),0,0,0,0,0,0,buff);
			}
		}
	}
}

bool gObjCheckAutoParty(LPOBJ lpObj,LPOBJ lpTarget)
{
	if((lpTarget->Option & 4) == 0)
	{
		return 0;
	}

	if(lpTarget->AutoPartyPassword[0] != 0 && strcmp(lpObj->AutoPartyPassword,lpTarget->AutoPartyPassword) != 0)
	{
		return 0;
	}

	return 1;
}

bool gObjCheckEventJoin(LPOBJ lpObj)
{
	if(KALIMA_MAP_RANGE(lpObj->Map) != 0 || CA_MAP_RANGE(lpObj->Map) != 0 || DS_MAP_RANGE(lpObj->Map) != 0 || BC_MAP_RANGE(lpObj->Map) != 0 || CC_MAP_RANGE(lpObj->Map) != 0 || IT_MAP_RANGE(lpObj->Map) != 0 || DA_MAP_RANGE(lpObj->Map) != 0 || DG_MAP_RANGE(lpObj->Map) != 0 || IG_MAP_RANGE(lpObj->Map) != 0 || lpObj->Map == MAP_CASTLE_SIEGE || lpObj->Map == MAP_LAND_OF_TRIALS)
	{
		return 1;
	}

	if(gReiDoMU.m_Active == 1 && lpObj->Map == gServerInfo.m_ReiDoMUMap)
	{
		return 1;
	}

	if(lpObj->PvP == 1)
	{
		return 1;
	}

	if(lpObj->KillAll == 1)
	{
		return 1;
	}

	if(lpObj->RunAndCatch == 1)
	{
		return 1;
	}

	return 0;
}
