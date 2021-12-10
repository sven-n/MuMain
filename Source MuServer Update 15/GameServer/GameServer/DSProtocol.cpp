#include "stdafx.h"
#include "resource.h"
#include "DSProtocol.h"
#include "BloodCastle.h"
#include "CashShop.h"
#include "CastleSiege.h"
#include "CastleSiegeSync.h"
#include "ChaosBox.h"
#include "CommandManager.h"
#include "CustomAttack.h"
#include "CustomNpcQuest.h"
#include "CustomQuest.h"
#include "CustomRanking.h"
#include "Crywolf.h"
#include "CrywolfSync.h"
#include "CSProtocol.h"
#include "CustomRankUser.h"
#include "CustomStore.h"
#include "CustomWing.h"
#include "ESProtocol.h"
#include "EventInventory.h"
#include "Filter.h"
#include "Fruit.h"
#include "GameMain.h"
#include "GensSystem.h"
#include "Guild.h"
#include "GuildMatching.h"
#include "Helper.h"
#include "IllusionTemple.h"
#include "ItemBagManager.h"
#include "JSProtocol.h"
#include "Log.h"
#include "LuckyCoin.h"
#include "LuckyItem.h"
#include "Map.h"
#include "MapServerManager.h"
#include "MasterSkillTree.h"
#include "Message.h"
#include "MuRummy.h"
#include "MuunSystem.h"
#include "Notice.h"
#include "NpcTalk.h"
#include "ObjectManager.h"
#include "Party.h"
#include "PartyMatching.h"
#include "PcPoint.h"
#include "PersonalShop.h"
#include "Quest.h"
#include "QuestWorld.h"
#include "Reconnect.h"
#include "ServerInfo.h"
#include "SocketManagerModern.h"
#include "Util.h"
#include "Viewport.h"
#include "Warehouse.h"
#include "ReiDoMU.h"

void DataServerProtocolCore(BYTE head,BYTE* lpMsg,int size) // OK
{
	switch(head)
	{
		case 0x00:
			DGServerInfoRecv((SDHP_DATA_SERVER_INFO_RECV*)lpMsg);
			break;
		case 0x01:
			DGCharacterListRecv((SDHP_CHARACTER_LIST_RECV*)lpMsg);
			break;
		case 0x02:
			DGCharacterCreateRecv((SDHP_CHARACTER_CREATE_RECV*)lpMsg);
			break;
		case 0x03:
			DGCharacterDeleteRecv((SDHP_CHARACTER_DELETE_RECV*)lpMsg);
			break;
		case 0x04:
			DGCharacterInfoRecv((SDHP_CHARACTER_INFO_RECV*)lpMsg);
			break;
		case 0x05:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					gWarehouse.DGWarehouseItemRecv((SDHP_WAREHOUSE_ITEM_RECV*)lpMsg);
					break;
				case 0x01:
					gWarehouse.DGWarehouseFreeRecv((SDHP_WAREHOUSE_FREE_RECV*)lpMsg);
					break;
				case 0x70:
					gWarehouse.DGWarehouseGuildItemRecv((SDHP_WAREHOUSE_ITEM_RECV*)lpMsg);
					break;
				case 0x71:
					gWarehouse.DGWarehouseGuildFreeRecv((SDHP_WAREHOUSE_FREE_RECV*)lpMsg);
					break;
				case 0x75:
					gWarehouse.GDWarehouseGuildClose((SDHP_WAREHOUSEGUILD_CLOSE_RECV*)lpMsg);
					break;
			}
			break;
		case 0x07:
			DGCreateItemRecv((SDHP_CREATE_ITEM_RECV*)lpMsg);
			break;
		case 0x08:
			DGOptionDataRecv((SDHP_OPTION_DATA_RECV*)lpMsg);
			break;
		case 0x09:
			DGPetItemInfoRecv((SDHP_PET_ITEM_INFO_RECV*)lpMsg);
			break;
		case 0x0A:
			#if(GAMESERVER_UPDATE>=401)
			DGCharacterNameCheckRecv((SDHP_CHARACTER_NAME_CHECK_RECV*)lpMsg);
			#endif
			break;
		case 0x0B:
			#if(GAMESERVER_UPDATE>=401)
			DGCharacterNameChangeRecv((SDHP_CHARACTER_NAME_CHANGE_RECV*)lpMsg);
			#endif
			break;
		case 0x0C:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					gQuest.DGQuestKillCountRecv((SDHP_QUEST_KILL_COUNT_RECV*)lpMsg);
					break;
			}
			break;
		case 0x0D:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					#if(GAMESERVER_UPDATE>=401)
					gMasterSkillTree.DGMasterSkillTreeRecv((SDHP_MASTER_SKILL_TREE_RECV*)lpMsg);
					#endif
					break;
			}
			break;
		case 0x0E:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					#if(GAMESERVER_UPDATE>=202)
					gNpcTalk.DGNpcLeoTheHelperRecv((SDHP_NPC_LEO_THE_HELPER_RECV*)lpMsg);
					#endif
					break;
				case 0x01:
					#if(GAMESERVER_UPDATE>=401)
					gNpcTalk.DGNpcSantaClausRecv((SDHP_NPC_SANTA_CLAUS_RECV*)lpMsg);
					#endif
					break;
			}
			break;
		case 0x0F:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					gCommandManager.DGCommandResetRecv((SDHP_COMMAND_RESET_RECV*)lpMsg);
					break;
				case 0x01:
					gCommandManager.DGCommandMasterResetRecv((SDHP_COMMAND_MASTER_RESET_RECV*)lpMsg);
					break;
                case 0x02:
                    gCommandManager.DGCommandMarryRecv((SDHP_COMMAND_MARRY_RECV*)lpMsg);
                    break;
                case 0x05:
                    gCommandManager.DGCommandRenameRecv((SDHP_COMMAND_RENAME_RECV*)lpMsg);
                    break;
                case 0x06:
                    gCommandManager.CommandBlocAccRecv((SDHP_COMMAND_BLOC_RECV*)lpMsg);
                    break;
                case 0x07:
                    gCommandManager.CommandBlocCharRecv((SDHP_COMMAND_BLOC_RECV*)lpMsg);
                    break;
                case 0x08:
                    gCommandManager.DGCommandGiftRecv((SDHP_GIFT_RECV*)lpMsg);
                    break;
                case 0x09:
                    gCommandManager.DGCommandTopRecv((SDHP_TOP_RECV*)lpMsg);
                    break;
			}
			break;
		case 0x10:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					#if(GAMESERVER_UPDATE>=501)
					gQuestWorld.DGQuestWorldRecv((SDHP_QUEST_WORLD_RECV*)lpMsg);
					#endif
					break;
			}
			break;
		case 0x11:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					#if(GAMESERVER_UPDATE>=501)
					gGensSystem.DGGensSystemInsertRecv((SDHP_GENS_SYSTEM_INSERT_RECV*)lpMsg);
					#endif
					break;
				case 0x01:
					#if(GAMESERVER_UPDATE>=501)
					gGensSystem.DGGensSystemDeleteRecv((SDHP_GENS_SYSTEM_DELETE_RECV*)lpMsg);
					#endif
					break;
				case 0x02:
					#if(GAMESERVER_UPDATE>=501)
					gGensSystem.DGGensSystemMemberRecv((SDHP_GENS_SYSTEM_MEMBER_RECV*)lpMsg);
					#endif
					break;
				case 0x03:
					#if(GAMESERVER_UPDATE>=501)
					gGensSystem.DGGensSystemUpdateRecv((SDHP_GENS_SYSTEM_UPDATE_RECV*)lpMsg);
					#endif
					break;
				case 0x04:
					#if(GAMESERVER_UPDATE>=501)
					gGensSystem.DGGensSystemRewardRecv((SDHP_GENS_SYSTEM_REWARD_RECV*)lpMsg);
					#endif
					break;
				case 0x70:
					#if(GAMESERVER_UPDATE>=501)
					gGensSystem.DGGensSystemCreateRecv((SDHP_GENS_SYSTEM_CREATE_RECV*)lpMsg);
					#endif
					break;
			}
			break;
		case 0x12:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					#if(GAMESERVER_UPDATE>=802)
					gMuRummy.GDAnsCardInfo((_tagPMSG_ANS_MURUMMY_SELECT_DS*)lpMsg);
					#endif
					break;
			}
			break;
		case 0x17:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					#if(GAMESERVER_UPDATE>=603)
					gHelper.DGHelperDataRecv((SDHP_HELPER_DATA_RECV*)lpMsg);
					#endif
					break;
			}
			break;
		case 0x18:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					gCashShop.DGCashShopPointRecv((SDHP_CASH_SHOP_POINT_RECV*)lpMsg);
					break;
				case 0x01:
					gCashShop.DGCashShopItemBuyRecv((SDHP_CASH_SHOP_ITEM_BUY_RECV*)lpMsg);
					break;
				case 0x02:
					gCashShop.DGCashShopItemGifRecv((SDHP_CASH_SHOP_ITEM_GIF_RECV*)lpMsg);
					break;
				case 0x03:
					gCashShop.DGCashShopItemNumRecv((SDHP_CASH_SHOP_ITEM_NUM_RECV*)lpMsg);
					break;
				case 0x04:
					gCashShop.DGCashShopItemUseRecv((SDHP_CASH_SHOP_ITEM_USE_RECV*)lpMsg);
					break;
				case 0x05:
					gCashShop.DGCashShopPeriodicItemRecv((SDHP_CASH_SHOP_PERIODIC_ITEM_RECV*)lpMsg);
					break;
				case 0x06:
					gCashShop.DGCashShopRecievePointRecv((SDHP_CASH_SHOP_RECIEVE_POINT_RECV*)lpMsg);
					break;
			}
			break;
		case 0x19:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					#if(GAMESERVER_UPDATE<=402)
					gPcPoint.DGPcPointPointRecv((SDHP_PC_POINT_POINT_RECV*)lpMsg);
					#endif
					break;
				case 0x01:
					#if(GAMESERVER_UPDATE<=402)
					gPcPoint.DGPcPointItemBuyRecv((SDHP_PC_POINT_ITEM_BUY_RECV*)lpMsg);
					#endif
					break;
				case 0x02:
					#if(GAMESERVER_UPDATE<=402)
					gPcPoint.DGPcPointRecievePointRecv((SDHP_PC_POINT_RECIEVE_POINT_RECV*)lpMsg);
					#endif
					break;
			}
			break;
		case 0x1A:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					#if(GAMESERVER_UPDATE>=402)
					gLuckyCoin.DGLuckyCoinCountRecv((SDHP_LUCKY_COIN_COUNT_RECV*)lpMsg);
					#endif
					break;
				case 0x01:
					#if(GAMESERVER_UPDATE>=402)
					gLuckyCoin.DGLuckyCoinRegisterRecv((SDHP_LUCKY_COIN_REGISTER_RECV*)lpMsg);
					#endif
					break;
				case 0x02:
					#if(GAMESERVER_UPDATE>=402)
					gLuckyCoin.DGLuckyCoinExchangeRecv((SDHP_LUCKY_COIN_EXCHANGE_RECV*)lpMsg);
					#endif
					break;
			}
			break;
		case 0x1E:
			DGCrywolfSyncRecv((SDHP_CRYWOLF_SYNC_RECV*)lpMsg);
			break;
		case 0x1F:
			DGCrywolfInfoRecv((SDHP_CRYWOLF_INFO_RECV*)lpMsg);
			break;
		case 0x20:
			DGGlobalPostRecv((SDHP_GLOBAL_POST_RECV*)lpMsg);
			break;
		case 0x21:
			DGGlobalNoticeRecv((SDHP_GLOBAL_NOTICE_RECV*)lpMsg);
			break;
		case 0x22:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					#if(GAMESERVER_UPDATE>=602)
					gLuckyItem.DGLuckyItemRecv((SDHP_LUCKY_ITEM_RECV*)lpMsg);
					#endif
					break;
			}
			break;
		case 0x23:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					#if(GAMESERVER_UPDATE>=701)
					gPentagramSystem.DGPentagramJewelInfoRecv((SDHP_PENTAGRAM_JEWEL_INFO_RECV*)lpMsg);
					#endif
					break;
			}
			break;
		case 0x24:
			#if(GAMESERVER_UPDATE>=801)
			DGSNSDataRecv((SDHP_SNS_DATA_RECV*)lpMsg);
			#endif
			break;
		case 0x25:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					#if(GAMESERVER_UPDATE>=802)
					gPersonalShop.DGPShopItemValueRecv((SDHP_PSHOP_ITEM_VALUE_RECV*)lpMsg);
					#endif
					break;
			}
			break;
		case 0x26:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					#if(GAMESERVER_UPDATE>=802)
					gEventInventory.DGEventInventoryRecv((SDHP_EVENT_INVENTORY_RECV*)lpMsg);
					#endif
					break;
			}
			break;
		case 0x27:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					#if(GAMESERVER_UPDATE>=803)
					gMuunSystem.DGMuunInventoryRecv((SDHP_MUUN_INVENTORY_RECV*)lpMsg);
					#endif
					break;
			}
			break;
		case 0x28:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					#if(GAMESERVER_UPDATE>=801)
					gGuildMatching.DGGuildMatchingListRecv((SDHP_GUILD_MATCHING_LIST_RECV*)lpMsg);
					#endif
					break;
				case 0x02:
					#if(GAMESERVER_UPDATE>=801)
					gGuildMatching.DGGuildMatchingInsertRecv((SDHP_GUILD_MATCHING_INSERT_RECV*)lpMsg);
					#endif
					break;
				case 0x03:
					#if(GAMESERVER_UPDATE>=801)
					gGuildMatching.DGGuildMatchingCancelRecv((SDHP_GUILD_MATCHING_CANCEL_RECV*)lpMsg);
					#endif
					break;
				case 0x04:
					#if(GAMESERVER_UPDATE>=801)
					gGuildMatching.DGGuildMatchingJoinInsertRecv((SDHP_GUILD_MATCHING_JOIN_INSERT_RECV*)lpMsg);
					#endif
					break;
				case 0x05:
					#if(GAMESERVER_UPDATE>=801)
					gGuildMatching.DGGuildMatchingJoinCancelRecv((SDHP_GUILD_MATCHING_JOIN_CANCEL_RECV*)lpMsg);
					#endif
					break;
				case 0x06:
					#if(GAMESERVER_UPDATE>=801)
					gGuildMatching.DGGuildMatchingJoinAcceptRecv((SDHP_GUILD_MATCHING_JOIN_ACCEPT_RECV*)lpMsg);
					#endif
					break;
				case 0x07:
					#if(GAMESERVER_UPDATE>=801)
					gGuildMatching.DGGuildMatchingJoinListRecv((SDHP_GUILD_MATCHING_JOIN_LIST_RECV*)lpMsg);
					#endif
					break;
				case 0x08:
					#if(GAMESERVER_UPDATE>=801)
					gGuildMatching.DGGuildMatchingJoinInfoRecv((SDHP_GUILD_MATCHING_JOIN_INFO_RECV*)lpMsg);
					#endif
					break;
				case 0x09:
					#if(GAMESERVER_UPDATE>=801)
					gGuildMatching.DGGuildMatchingNotifyRecv((SDHP_GUILD_MATCHING_NOTIFY_RECV*)lpMsg);
					#endif
					break;
				case 0x0A:
					#if(GAMESERVER_UPDATE>=801)
					gGuildMatching.DGGuildMatchingNotifyMasterRecv((SDHP_GUILD_MATCHING_NOTIFY_MASTER_RECV*)lpMsg);
					#endif
					break;
			}
			break;
		case 0x29:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					#if(GAMESERVER_UPDATE>=801)
					gPartyMatching.DGPartyMatchingInsertRecv((SDHP_PARTY_MATCHING_INSERT_RECV*)lpMsg);
					#endif
					break;
				case 0x01:
					#if(GAMESERVER_UPDATE>=801)
					gPartyMatching.DGPartyMatchingListRecv((SDHP_PARTY_MATCHING_LIST_RECV*)lpMsg);
					#endif
					break;
				case 0x02:
					#if(GAMESERVER_UPDATE>=801)
					gPartyMatching.DGPartyMatchingJoinInsertRecv((SDHP_PARTY_MATCHING_JOIN_INSERT_RECV*)lpMsg);
					#endif
					break;
				case 0x03:
					#if(GAMESERVER_UPDATE>=801)
					gPartyMatching.DGPartyMatchingJoinInfoRecv((SDHP_PARTY_MATCHING_JOIN_INFO_RECV*)lpMsg);
					#endif
					break;
				case 0x04:
					#if(GAMESERVER_UPDATE>=801)
					gPartyMatching.DGPartyMatchingJoinListRecv((SDHP_PARTY_MATCHING_JOIN_LIST_RECV*)lpMsg);
					#endif
					break;
				case 0x05:
					#if(GAMESERVER_UPDATE>=801)
					gPartyMatching.DGPartyMatchingJoinAcceptRecv((SDHP_PARTY_MATCHING_JOIN_ACCEPT_RECV*)lpMsg);
					#endif
					break;
				case 0x06:
					#if(GAMESERVER_UPDATE>=801)
					gPartyMatching.DGPartyMatchingJoinCancelRecv((SDHP_PARTY_MATCHING_JOIN_CANCEL_RECV*)lpMsg);
					#endif
					break;
				case 0x07:
					#if(GAMESERVER_UPDATE>=801)
					gPartyMatching.DGPartyMatchingNotifyRecv((SDHP_PARTY_MATCHING_NOTIFY_RECV*)lpMsg);
					#endif
					break;
				case 0x08:
					#if(GAMESERVER_UPDATE>=801)
					gPartyMatching.DGPartyMatchingNotifyLeaderRecv((SDHP_PARTY_MATCHING_NOTIFY_LEADER_RECV*)lpMsg);
					#endif
					break;
			}
			break;
		case 0x72:
			DGGlobalWhisperRecv((SDHP_GLOBAL_WHISPER_RECV*)lpMsg);
			break;
		case 0x73:
			DGGlobalWhisperEchoRecv((SDHP_GLOBAL_WHISPER_ECHO_RECV*)lpMsg);
			break;
		case 0x80:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					GS_DGAnsCastleTotalInfo(lpMsg);
					break;
				case 0x01:
					GS_DGAnsOwnerGuildMaster(lpMsg);
					break;
				case 0x03:
					GS_DGAnsCastleNpcBuy(lpMsg);
					break;
				case 0x04:
					GS_DGAnsCastleNpcRepair(lpMsg);
					break;
				case 0x05:
					GS_DGAnsCastleNpcUpgrade(lpMsg);
					break;
				case 0x06:
					GS_DGAnsTaxInfo(lpMsg);
					break;
				case 0x07:
					GS_DGAnsTaxRateChange(lpMsg);
					break;
				case 0x08:
					GS_DGAnsCastleMoneyChange(lpMsg);
					break;
				case 0x09:
					GS_DGAnsSiegeDateChange(lpMsg);
					break;
				case 0x0A:
					GS_DGAnsGuildMarkRegInfo(lpMsg);
					break;
				case 0x0B:
					GS_DGAnsSiegeEndedChange(lpMsg);
					break;
				case 0x0C:
					GS_DGAnsCastleOwnerChange(lpMsg);
					break;
				case 0x0D:
					GS_DGAnsRegAttackGuild(lpMsg);
					break;
				case 0x0E:
					GS_DGAnsRestartCastleState(lpMsg);
					break;
				case 0x0F:
					GS_DGAnsMapSvrMsgMultiCast(lpMsg);
					break;
				case 0x10:
					GS_DGAnsRegGuildMark(lpMsg);
					break;
				case 0x11:
					GS_DGAnsGuildMarkReset(lpMsg);
					break;
				case 0x12:
					GS_DGAnsGuildSetGiveUp(lpMsg);
					break;
				case 0x16:
					GS_DGAnsNpcRemove(lpMsg);
					break;
				case 0x17:
					GS_DGAnsCastleStateSync(lpMsg);
					break;
				case 0x18:
					GS_DGAnsCastleTributeMoney(lpMsg);
					break;
				case 0x19:
					GS_DGAnsResetCastleTaxInfo(lpMsg);
					break;
				case 0x1A:
					GS_DGAnsResetSiegeGuildInfo(lpMsg);
					break;
				case 0x1B:
					GS_DGAnsResetRegSiegeInfo(lpMsg);
					break;
			}
			break;
		case 0x81:
			GS_DGAnsCastleInitData(lpMsg);
			break;
		case 0x82:
			GS_DGAnsCastleNpcInfo(lpMsg);
			break;
		case 0x83:
			GS_DGAnsAllGuildMarkRegInfo(lpMsg);
			break;
		case 0x84:
			GS_DGAnsFirstCreateNPC(lpMsg);
			break;
		case 0x85:
			GS_DGAnsCalcREgGuildList(lpMsg);
			break;
		case 0x86:
			GS_DGAnsCsGulidUnionInfo(lpMsg);
			break;
		case 0x87:
			GS_DGAnsCsSaveTotalGuildInfo(lpMsg);
			break;
		case 0x88:
			GS_DGAnsCsLoadTotalGuildInfo(lpMsg);
			break;
		case 0x89:
			GS_DGAnsCastleNpcUpdate(lpMsg);
			break;
		case 0xE0:
			ESDataRecv(head,lpMsg,size);
			break;
		case 0xE1:
			CSDataRecv(head,lpMsg,size);
			break;
		case 0xF1:
			gCustomQuest.DGCustomQuestRecv((SDHP_CUSTOMQUEST_RECV*)lpMsg);
			break;
		case 0xF4:
			gCustomRanking.GDCustomRankingRecv((SDHP_CUSTOM_RANKING_RECV*)lpMsg);
			break;
		case 0xF5:
			gCustomAttack.DGCustomAttackResumeRecv((SDHP_CARESUME_RECV*)lpMsg);
			break;
		case 0xF7:
			switch(((lpMsg[0]==0xC1)?lpMsg[3]:lpMsg[4]))
			{
				case 0x00:
					gCustomNpcQuest.DGCustomNpcQuestRecv((SDHP_CUSTOMNPCQUEST_RECV*)lpMsg);
					break;
			}
			break;
	}
}

void DGServerInfoRecv(SDHP_DATA_SERVER_INFO_RECV* lpMsg) // OK
{

}

void DGCharacterListRecv(SDHP_CHARACTER_LIST_RECV* lpMsg) // OK
{
	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGCharacterListRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	BYTE send[256];

	PMSG_CHARACTER_LIST_SEND pMsg;

	#if(NEW_PROTOCOL_SYSTEM==0)
		pMsg.header.set(0xF3,0x00,0);
	#endif

	int size = sizeof(pMsg);

	pMsg.ClassCode = lpMsg->ExtClass;

	pMsg.MoveCnt = lpMsg->MoveCnt;

	pMsg.count = 0;

	//#if(GAMESERVER_UPDATE>=602)

	//pMsg.ExtWarehouse = lpMsg->ExtWarehouse;

	//#endif

	PMSG_CHARACTER_LIST info;

	WORD CharacterCreationLevel			= 0;
	DWORD CharacterCreationReset		= 0;
	DWORD CharacterCreationMasterReset	= 0;

	for(int n=0;n < lpMsg->count;n++)
	{
		SDHP_CHARACTER_LIST* lpInfo = (SDHP_CHARACTER_LIST*)(((BYTE*)lpMsg)+sizeof(SDHP_CHARACTER_LIST_RECV)+(sizeof(SDHP_CHARACTER_LIST)*n));

		info.slot = lpInfo->slot;

		memcpy(info.Name,lpInfo->name,sizeof(info.Name));

		info.Level = lpInfo->level;

		CharacterCreationLevel			= ((info.Level>CharacterCreationLevel)?info.Level:CharacterCreationLevel);
		CharacterCreationReset			= ((lpInfo->Reset>CharacterCreationReset)?lpInfo->Reset:CharacterCreationReset);
		CharacterCreationMasterReset	= ((lpInfo->MasterReset>CharacterCreationMasterReset)?lpInfo->MasterReset:CharacterCreationMasterReset);

		info.CtlCode = lpInfo->CtlCode;

		#pragma region CHAR_SET_CALC

		memset(info.CharSet,0,sizeof(info.CharSet));

		info.CharSet[0] = (lpInfo->Class%16)*16;
		info.CharSet[0] -= (info.CharSet[0]/32);
		info.CharSet[0] += (lpInfo->Class/16)*32;

		WORD TempInventory[12];

		for(int i=0;i < 9;i++)
		{
			if(i == 0 || i == 1)
			{
				if(lpInfo->Inventory[0+(i*5)] == 0xFF && (lpInfo->Inventory[2+(i*5)] & 0x80) == 0x80 && (lpInfo->Inventory[4+(i*5)] & 0xF0) == 0xF0)
				{
					TempInventory[i] = 0xFFFF;
				}
				else
				{
					TempInventory[i] = (lpInfo->Inventory[0+(i*5)]+((lpInfo->Inventory[2+(i*5)] & 0x80)*2)+((lpInfo->Inventory[4+(i*5)] & 0xF0)*32));
				}
			}
			else
			{
				if(lpInfo->Inventory[0+(i*5)] == 0xFF && (lpInfo->Inventory[2+(i*5)] & 0x80) == 0x80 && (lpInfo->Inventory[4+(i*5)] & 0xF0) == 0xF0)
				{
					TempInventory[i] = 0x1FF;
				}
				else
				{
					TempInventory[i] = (lpInfo->Inventory[0+(i*5)]+((lpInfo->Inventory[2+(i*5)] & 0x80)*2)+((lpInfo->Inventory[4+(i*5)] & 0xF0)*32))%MAX_ITEM_TYPE;
				}
			}
		}
		
		info.CharSet[1] = TempInventory[0]%256;
		info.CharSet[12] |= (TempInventory[0]/16) & 0xF0;

		info.CharSet[2] = TempInventory[1]%256;
		info.CharSet[13] |= (TempInventory[1]/16) & 0xF0;

		info.CharSet[3] |= (TempInventory[2] & 0x0F) << 4;
		info.CharSet[9] |= (TempInventory[2] & 0x10) << 3;
		info.CharSet[13] |= (TempInventory[2] & 0x1E0) >> 5;

		info.CharSet[3] |= (TempInventory[3] & 0x0F);
		info.CharSet[9] |= (TempInventory[3] & 0x10) << 2;
		info.CharSet[14] |= (TempInventory[3] & 0x1E0) >> 1;

		info.CharSet[4] |= (TempInventory[4] & 0x0F) << 4;
		info.CharSet[9] |= (TempInventory[4] & 0x10) << 1;
		info.CharSet[14] |= (TempInventory[4] & 0x1E0) >> 5;

		info.CharSet[4] |= (TempInventory[5] & 0x0F);
		info.CharSet[9] |= (TempInventory[5] & 0x10);
		info.CharSet[15] |= (TempInventory[5] & 0x1E0) >> 1;

		info.CharSet[5] |= (TempInventory[6] & 0x0F) << 4;
		info.CharSet[9] |= (TempInventory[6] & 0x10) >> 1;
		info.CharSet[15] |= (TempInventory[6] & 0x1E0) >> 5;

		int level = 0;

		BYTE table[7] = {1,0,6,5,4,3,2};

		for(int i=0;i < 7;i++)
		{
			if(TempInventory[i] != 0x1FF && TempInventory[i] != 0xFFFF)
			{
				level |= ((((lpInfo->Inventory[1+(i*5)]/8) & 0x0F)-1)/2) << (i*3);

				info.CharSet[10] |= ((lpInfo->Inventory[2+(i*5)] & 0x3F)?2:0) << table[i];
				info.CharSet[11] |= ((lpInfo->Inventory[3+(i*5)] & 0x03)?2:0) << table[i];
			}
		}

		info.CharSet[6] = level >> 16;
		info.CharSet[7] = level >> 8;
		info.CharSet[8] = level;

		#if(GAMESERVER_UPDATE>=601)

		if(TempInventory[7] >= 0 && TempInventory[7] <= 2)
		{
			info.CharSet[5] |= 4;
			info.CharSet[9] |= TempInventory[7]+1;
		}
		else if(TempInventory[7] >= 3 && TempInventory[7] <= 6)
		{
			info.CharSet[5] |= 8;
			info.CharSet[9] |= TempInventory[7]-2;
		}
		else if(TempInventory[7] == 30)
		{
			info.CharSet[5] |= 8;
			info.CharSet[9] |= 5;
		}
		else if(TempInventory[7] >= 36 && TempInventory[7] <= 40)
		{
			info.CharSet[5] |= 12;
			info.CharSet[9] |= TempInventory[7]-35;
		}
		else if(TempInventory[7] == 41)
		{
			info.CharSet[5] |= 4;
			info.CharSet[9] |= 4;
		}
		else if(TempInventory[7] == 42)
		{
			info.CharSet[5] |= 8;
			info.CharSet[9] |= 6;
		}
		else if(TempInventory[7] == 43)
		{
			info.CharSet[5] |= 12;
			info.CharSet[9] |= 6;
		}
		else if(TempInventory[7] == 49)
		{
			info.CharSet[5] |= 8;
			info.CharSet[9] |= 7;
		}
		else if(TempInventory[7] == 50)
		{
			info.CharSet[5] |= 12;
			info.CharSet[9] |= 7;
		}
		else if(TempInventory[7] >= 130 && TempInventory[7] <= 135)
		{
			info.CharSet[5] |= 12;
			info.CharSet[17] |= (TempInventory[7]-129) << 5;
		}
		else if(TempInventory[7] >= 262 && TempInventory[7] <= 265)
		{
			info.CharSet[5] |= 8;
			info.CharSet[16] |= (TempInventory[7]-261) << 2;
		}
		else if(TempInventory[7] == 266)
		{
			info.CharSet[5] |= 4;
			info.CharSet[9] |= 5;
		}
		else if(TempInventory[7] == 267)
		{
			info.CharSet[5] |= 4;
			info.CharSet[9] |= 6;
		}
		else if(gCustomWing.CheckCustomWingByItem(GET_ITEM(12,TempInventory[7])) != 0)
		{
			info.CharSet[5] |= 12;
			info.CharSet[17] |= (gCustomWing.GetCustomWingIndex(GET_ITEM(12,TempInventory[7]))+1) << 1;
		}

		#else

		if(TempInventory[7] == 0x1FF)
		{
			info.CharSet[5] |= 12;
		}
		else if(TempInventory[7] >= 0 && TempInventory[7] <= 2)
		{
			info.CharSet[5] |= TempInventory[7] << 2;
		}
		else if(TempInventory[7] >= 3 && TempInventory[7] <= 6)
		{
			info.CharSet[5] |= 12;
			info.CharSet[9] |= TempInventory[7]-2;
		}
		else if(TempInventory[7] == 30)
		{
			info.CharSet[5] |= 12;
			info.CharSet[9] |= 5;
		}
		else if(TempInventory[7] >= 36 && TempInventory[7] <= 40)
		{
			info.CharSet[5] |= 12;
			info.CharSet[16] |= (TempInventory[7]-35) << 2;
		}
		else if(TempInventory[7] == 41)
		{
			info.CharSet[5] |= 12;
			info.CharSet[9] |= 6;
		}
		else if(TempInventory[7] == 42)
		{
			info.CharSet[5] |= 12;
			info.CharSet[16] |= 28;
		}
		else if(TempInventory[7] == 43)
		{
			info.CharSet[5] |= 12;
			info.CharSet[16] |= 24;
		}
		else if(TempInventory[7] >= 130 && TempInventory[7] <= 135)
		{
			info.CharSet[5] |= 12;
			info.CharSet[17] |= (TempInventory[7]-129) << 5;
		}
		else if(gCustomWing.CheckCustomWingByItem(GET_ITEM(12,TempInventory[7])) != 0)
		{
			info.CharSet[5] |= 12;
			info.CharSet[17] |= (gCustomWing.GetCustomWingIndex(GET_ITEM(12,TempInventory[7]))+1) << 1;
		}

		#endif

		if(TempInventory[8] == 0x1FF)
		{
			info.CharSet[5] |= 3;
		}
		else if(TempInventory[8] >= 0 && TempInventory[8] <= 2)
		{
			info.CharSet[5] |= TempInventory[8];
		}
		else if(TempInventory[8] == 3)
		{
			info.CharSet[5] |= 3;
			info.CharSet[10] |= 1;
		}
		else if(TempInventory[8] == 4)
		{
			info.CharSet[5] |= 3;
			info.CharSet[12] |= 1;
		}
		else if(TempInventory[8] == 37)
		{
			info.CharSet[5] |= 3;
			info.CharSet[10] &= 0xFE;
			info.CharSet[12] &= 0xFE;
			info.CharSet[12] |= 4;

			if((lpInfo->Inventory[42] & 1) != 0)
			{
				info.CharSet[16] |= 1;
			}
			else if((lpInfo->Inventory[42] & 2) != 0)
			{
				info.CharSet[16] |= 2;
			}
			else if((lpInfo->Inventory[42] & 4) != 0)
			{
				info.CharSet[17] |= 1;
			}
		}
		else if(TempInventory[8] == 64 || TempInventory[8] == 65 || TempInventory[8] == 67)
		{
			info.CharSet[16] |= (TempInventory[8]-63) << 5;
		}
		else if(TempInventory[8] == 80)
		{
			info.CharSet[16] |= 0xE0;
		}
		else if(TempInventory[8] == 106)
		{
			info.CharSet[16] |= 0xA0;
		}
		else if(TempInventory[8] == 123)
		{
			info.CharSet[16] |= 0x60;
		}
		//else if(TempInventory[8] == 200) //New Pet
		//{
		//	info.CharSet[16] |= 225;
		//}
		//else if(TempInventory[8] >= 150 && TempInventory[8] <= 200)
		//{
		//	info.CharSet[16] |= ((TempInventory[8] - 150) + 1) << 2;
		//}

		#pragma endregion

		info.GuildStatus = lpInfo->GuildStatus;

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);

		pMsg.count++;
	}

	if(gServerInfo.m_MGCreateType == 0 && CharacterCreationLevel >= (WORD)gServerInfo.m_MGCreateLevel[gObj[lpMsg->index].AccountLevel] && CharacterCreationReset >= (DWORD)gServerInfo.m_MGCreateReset[gObj[lpMsg->index].AccountLevel] && CharacterCreationMasterReset >= (DWORD)gServerInfo.m_MGCreateMasterReset[gObj[lpMsg->index].AccountLevel])
	{
		pMsg.ClassCode |= 4;
	}

	if(gServerInfo.m_DLCreateType == 0 && CharacterCreationLevel >= (WORD)gServerInfo.m_DLCreateLevel[gObj[lpMsg->index].AccountLevel] && CharacterCreationReset >= (DWORD)gServerInfo.m_DLCreateReset[gObj[lpMsg->index].AccountLevel] && CharacterCreationMasterReset >= (DWORD)gServerInfo.m_DLCreateMasterReset[gObj[lpMsg->index].AccountLevel])
	{
		pMsg.ClassCode |= 2;
	}

	if(gServerInfo.m_SUCreateType == 0 && CharacterCreationLevel >= (WORD)gServerInfo.m_SUCreateLevel[gObj[lpMsg->index].AccountLevel] && CharacterCreationReset >= (DWORD)gServerInfo.m_SUCreateReset[gObj[lpMsg->index].AccountLevel] && CharacterCreationMasterReset >= (DWORD)gServerInfo.m_SUCreateMasterReset[gObj[lpMsg->index].AccountLevel])
	{
		pMsg.ClassCode |= 1;
	}

	if(gServerInfo.m_RFCreateType == 0 && CharacterCreationLevel >= (WORD)gServerInfo.m_RFCreateLevel[gObj[lpMsg->index].AccountLevel] && CharacterCreationReset >= (DWORD)gServerInfo.m_RFCreateReset[gObj[lpMsg->index].AccountLevel] && CharacterCreationMasterReset >= (DWORD)gServerInfo.m_RFCreateMasterReset[gObj[lpMsg->index].AccountLevel])
	{
		pMsg.ClassCode |= 8;
	}

	gObj[lpMsg->index].ClassCode = pMsg.ClassCode;

	gObj[lpMsg->index].ClassFlag = ((CharacterCreationLevel>=gServerInfo.m_MGCreateLevel[gObj[lpMsg->index].AccountLevel])?(gObj[lpMsg->index].ClassFlag | 4):gObj[lpMsg->index].ClassFlag);

	gObj[lpMsg->index].ClassFlag = ((CharacterCreationLevel>=gServerInfo.m_DLCreateLevel[gObj[lpMsg->index].AccountLevel])?(gObj[lpMsg->index].ClassFlag | 2):gObj[lpMsg->index].ClassFlag);

	gObj[lpMsg->index].ClassFlag = ((CharacterCreationLevel>=gServerInfo.m_SUCreateLevel[gObj[lpMsg->index].AccountLevel])?(gObj[lpMsg->index].ClassFlag | 1):gObj[lpMsg->index].ClassFlag);

	gObj[lpMsg->index].ClassFlag = ((CharacterCreationLevel>=gServerInfo.m_RFCreateLevel[gObj[lpMsg->index].AccountLevel])?(gObj[lpMsg->index].ClassFlag | 8):gObj[lpMsg->index].ClassFlag);

	#if(GAMESERVER_UPDATE>=602)

	//gObj[lpMsg->index].ExtWarehouse = pMsg.ExtWarehouse;

	#endif

	#if(NEW_PROTOCOL_SYSTEM==0)
		pMsg.header.size = size;
	#endif

	memcpy(send,&pMsg,sizeof(pMsg));

#if(NEW_PROTOCOL_SYSTEM==1)
	gSocketManagerModern.PacketSend(lpMsg->index,ProtocolHead::BOTH_CONNECT_CHARACTER,(uint8_t*)send,size);
#else
	DataSend(lpMsg->index,send,size);
#endif
	
	GCCharacterCreationEnableSend(lpMsg->index,0,gObj[lpMsg->index].ClassCode);
}

void DGCharacterCreateRecv(SDHP_CHARACTER_CREATE_RECV* lpMsg) // OK
{
	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGCharacterCreateRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	PMSG_CHARACTER_CREATE_SEND pMsg;

	pMsg.header.set(0xF3,0x01,sizeof(pMsg));

	pMsg.result = lpMsg->result;

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.slot = lpMsg->slot;

	pMsg.level = lpMsg->level;

	pMsg.Class = (lpMsg->Class%16)*16;
	pMsg.Class -= (pMsg.Class/32);
	pMsg.Class += (lpMsg->Class/16)*32;

	memcpy(pMsg.equipment,lpMsg->equipment,sizeof(pMsg.equipment));

	DataSend(lpMsg->index,(BYTE*)&pMsg,pMsg.header.size);
}

void DGCharacterDeleteRecv(SDHP_CHARACTER_DELETE_RECV* lpMsg) // OK
{
	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGCharacterDeleteRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	PMSG_CHARACTER_DELETE_SEND pMsg;

	pMsg.header.set(0xF3,0x02,sizeof(pMsg));

	pMsg.result = lpMsg->result;
	
	DataSend(lpMsg->index,(BYTE*)&pMsg,pMsg.header.size);
}

void DGCharacterInfoRecv(SDHP_CHARACTER_INFO_RECV* lpMsg) // OK
{
	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGCharacterInfoRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	if(lpMsg->result == 0)
	{
		CloseClient(lpMsg->index);
		return;
	}

	if((lpMsg->CtlCode & 1) != 0)
	{
		CloseClient(lpMsg->index);
		return;
	}

	if(gObjectManager.CharacterInfoSet((BYTE*)lpMsg,lpMsg->index) == 0)
	{
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpObj->MapServerMoveRequest == 0)
	{
		if((lpObj->NextServerCode=gMapServerManager.CheckMapServerMove(lpObj->Index,lpObj->Map,lpObj->LastServerCode)) != gServerInfo.m_ServerCode)
		{
			if(lpObj->NextServerCode == -1)
			{
				CloseClient(lpObj->Index);
				return;
			}
			else
			{
				GJMapServerMoveSend(lpObj->Index,lpObj->NextServerCode,lpObj->Map,(BYTE)lpObj->X,(BYTE)lpObj->Y);
				return;
			}
		}
	}

	lpObj->MapMoveDisable = 1;

	PMSG_CHARACTER_INFO_SEND pMsg;

	pMsg.header.set(0xF3,0x03,sizeof(pMsg));

	pMsg.X = (BYTE)lpObj->X;
	pMsg.Y = (BYTE)lpObj->Y;
	pMsg.Map = lpObj->Map;
	pMsg.Dir = lpObj->Dir;

	if(gMasterSkillTree.CheckMasterLevel(lpObj) == 0)
	{
		pMsg.Experience[0] = SET_NUMBERHB(SET_NUMBERHW(SET_NUMBERHDW(lpObj->Experience)));
		pMsg.Experience[1] = SET_NUMBERLB(SET_NUMBERHW(SET_NUMBERHDW(lpObj->Experience)));
		pMsg.Experience[2] = SET_NUMBERHB(SET_NUMBERLW(SET_NUMBERHDW(lpObj->Experience)));
		pMsg.Experience[3] = SET_NUMBERLB(SET_NUMBERLW(SET_NUMBERHDW(lpObj->Experience)));
		pMsg.Experience[4] = SET_NUMBERHB(SET_NUMBERHW(SET_NUMBERLDW(lpObj->Experience)));
		pMsg.Experience[5] = SET_NUMBERLB(SET_NUMBERHW(SET_NUMBERLDW(lpObj->Experience)));
		pMsg.Experience[6] = SET_NUMBERHB(SET_NUMBERLW(SET_NUMBERLDW(lpObj->Experience)));
		pMsg.Experience[7] = SET_NUMBERLB(SET_NUMBERLW(SET_NUMBERLDW(lpObj->Experience)));
	}
	else
	{
		pMsg.Experience[0] = SET_NUMBERHB(SET_NUMBERHW(SET_NUMBERHDW(lpObj->MasterExperience)));
		pMsg.Experience[1] = SET_NUMBERLB(SET_NUMBERHW(SET_NUMBERHDW(lpObj->MasterExperience)));
		pMsg.Experience[2] = SET_NUMBERHB(SET_NUMBERLW(SET_NUMBERHDW(lpObj->MasterExperience)));
		pMsg.Experience[3] = SET_NUMBERLB(SET_NUMBERLW(SET_NUMBERHDW(lpObj->MasterExperience)));
		pMsg.Experience[4] = SET_NUMBERHB(SET_NUMBERHW(SET_NUMBERLDW(lpObj->MasterExperience)));
		pMsg.Experience[5] = SET_NUMBERLB(SET_NUMBERHW(SET_NUMBERLDW(lpObj->MasterExperience)));
		pMsg.Experience[6] = SET_NUMBERHB(SET_NUMBERLW(SET_NUMBERLDW(lpObj->MasterExperience)));
		pMsg.Experience[7] = SET_NUMBERLB(SET_NUMBERLW(SET_NUMBERLDW(lpObj->MasterExperience)));
	}

	if(gMasterSkillTree.CheckMasterLevel(lpObj) == 0)
	{
		pMsg.NextExperience[0] = SET_NUMBERHB(SET_NUMBERHW(SET_NUMBERHDW(lpObj->NextExperience)));
		pMsg.NextExperience[1] = SET_NUMBERLB(SET_NUMBERHW(SET_NUMBERHDW(lpObj->NextExperience)));
		pMsg.NextExperience[2] = SET_NUMBERHB(SET_NUMBERLW(SET_NUMBERHDW(lpObj->NextExperience)));
		pMsg.NextExperience[3] = SET_NUMBERLB(SET_NUMBERLW(SET_NUMBERHDW(lpObj->NextExperience)));
		pMsg.NextExperience[4] = SET_NUMBERHB(SET_NUMBERHW(SET_NUMBERLDW(lpObj->NextExperience)));
		pMsg.NextExperience[5] = SET_NUMBERLB(SET_NUMBERHW(SET_NUMBERLDW(lpObj->NextExperience)));
		pMsg.NextExperience[6] = SET_NUMBERHB(SET_NUMBERLW(SET_NUMBERLDW(lpObj->NextExperience)));
		pMsg.NextExperience[7] = SET_NUMBERLB(SET_NUMBERLW(SET_NUMBERLDW(lpObj->NextExperience)));
	}
	else
	{
		pMsg.NextExperience[0] = SET_NUMBERHB(SET_NUMBERHW(SET_NUMBERHDW(lpObj->MasterNextExperience)));
		pMsg.NextExperience[1] = SET_NUMBERLB(SET_NUMBERHW(SET_NUMBERHDW(lpObj->MasterNextExperience)));
		pMsg.NextExperience[2] = SET_NUMBERHB(SET_NUMBERLW(SET_NUMBERHDW(lpObj->MasterNextExperience)));
		pMsg.NextExperience[3] = SET_NUMBERLB(SET_NUMBERLW(SET_NUMBERHDW(lpObj->MasterNextExperience)));
		pMsg.NextExperience[4] = SET_NUMBERHB(SET_NUMBERHW(SET_NUMBERLDW(lpObj->MasterNextExperience)));
		pMsg.NextExperience[5] = SET_NUMBERLB(SET_NUMBERHW(SET_NUMBERLDW(lpObj->MasterNextExperience)));
		pMsg.NextExperience[6] = SET_NUMBERHB(SET_NUMBERLW(SET_NUMBERLDW(lpObj->MasterNextExperience)));
		pMsg.NextExperience[7] = SET_NUMBERLB(SET_NUMBERLW(SET_NUMBERLDW(lpObj->MasterNextExperience)));
	}

	pMsg.LevelUpPoint = lpObj->LevelUpPoint;
	pMsg.Strength = GET_MAX_WORD_VALUE(lpObj->Strength);
	pMsg.Dexterity = GET_MAX_WORD_VALUE(lpObj->Dexterity);
	pMsg.Vitality = GET_MAX_WORD_VALUE(lpObj->Vitality);
	pMsg.Energy = GET_MAX_WORD_VALUE(lpObj->Energy);
	pMsg.Life = GET_MAX_WORD_VALUE(lpObj->Life);
	pMsg.MaxLife = GET_MAX_WORD_VALUE((lpObj->MaxLife+lpObj->AddLife));
	pMsg.Mana = GET_MAX_WORD_VALUE(lpObj->Mana);
	pMsg.MaxMana = GET_MAX_WORD_VALUE((lpObj->MaxMana+lpObj->AddMana));
	pMsg.Shield = GET_MAX_WORD_VALUE(lpObj->Shield);
	pMsg.MaxShield = GET_MAX_WORD_VALUE((lpObj->MaxShield+lpObj->AddShield));
	pMsg.BP = GET_MAX_WORD_VALUE(lpObj->BP);
	pMsg.MaxBP = GET_MAX_WORD_VALUE((lpObj->MaxBP+lpObj->AddBP));
	pMsg.Money = lpObj->Money;
	pMsg.PKLevel = lpObj->PKLevel;
	pMsg.CtlCode = lpMsg->CtlCode;
	pMsg.FruitAddPoint = lpObj->FruitAddPoint;
	pMsg.MaxFruitAddPoint = gFruit.GetMaxFruitPoint(lpObj);
	pMsg.Leadership = GET_MAX_WORD_VALUE(lpObj->Leadership);
	pMsg.FruitSubPoint = lpObj->FruitSubPoint;
	pMsg.MaxFruitSubPoint = gFruit.GetMaxFruitPoint(lpObj);
	#if(GAMESERVER_UPDATE>=602)
	pMsg.ExtInventory = lpObj->ExtInventory;
	#endif

	#if(GAMESERVER_EXTRA==1)
	pMsg.ViewReset = (DWORD)(lpObj->Reset);
	pMsg.ViewPoint = (DWORD)(lpObj->LevelUpPoint);
	pMsg.ViewCurHP = (DWORD)(lpObj->Life);
	pMsg.ViewMaxHP = (DWORD)(lpObj->MaxLife+lpObj->AddLife);
	pMsg.ViewCurMP = (DWORD)(lpObj->Mana);
	pMsg.ViewMaxMP = (DWORD)(lpObj->MaxMana+lpObj->AddMana);
	pMsg.ViewCurBP = (DWORD)(lpObj->BP);
	pMsg.ViewMaxBP = (DWORD)(lpObj->MaxBP+lpObj->AddBP);
	pMsg.ViewCurSD = (DWORD)(lpObj->Shield);
	pMsg.ViewMaxSD = (DWORD)(lpObj->MaxShield+lpObj->AddShield);
	pMsg.ViewStrength = lpObj->Strength;
	pMsg.ViewDexterity = lpObj->Dexterity;
	pMsg.ViewVitality = lpObj->Vitality;
	pMsg.ViewEnergy = lpObj->Energy;
	pMsg.ViewLeadership = lpObj->Leadership;
	#endif

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	GDConnectCharacterSend(lpObj->Index);

	GDOptionDataSend(lpObj->Index);

	DGGuildMemberInfoRequest(lpObj->Index);

	FriendListRequest(lpObj->Index);

	GDPetItemInfoSend(lpObj->Index,0);

	gQuest.GDQuestKillCountSend(lpObj->Index);

	#if(GAMESERVER_UPDATE>=401)

	gMasterSkillTree.GDMasterSkillTreeSend(lpObj->Index);

	#endif

	#if(GAMESERVER_UPDATE>=501)

	gQuestWorld.GDQuestWorldSend(lpObj->Index);

	gGensSystem.GDGensSystemMemberSend(lpObj->Index);

	#endif

	#if(GAMESERVER_UPDATE>=603)

	gHelper.GDHelperDataSend(lpObj->Index);

	#endif

	gCashShop.GDCashShopPeriodicItemSend(lpObj->Index);

	#if(GAMESERVER_UPDATE>=602)

	gLuckyItem.GDLuckyItemSend(lpObj->Index);

	#endif

	#if(GAMESERVER_UPDATE>=701)

	gPentagramSystem.GDPentagramJewelInfoSend(lpObj->Index,PENTAGRAM_JEWEL_TYPE_INVENTORY);

	//gPentagramSystem.GDPentagramJewelInfoSend(lpObj->Index,PENTAGRAM_JEWEL_TYPE_WAREHOUSE);

	#endif

	#if(GAMESERVER_UPDATE>=801)

	GDSNSDataSend(lpObj->Index);

	#endif

	#if(GAMESERVER_UPDATE>=802)

	gPersonalShop.GDPShopItemValueSend(lpObj->Index);

	gEventInventory.GDEventInventorySend(lpObj->Index);

	gMuRummy.GDReqCardInfo(lpObj);

	#endif

	#if(GAMESERVER_UPDATE>=803)

	gMuunSystem.GDMuunInventorySend(lpObj->Index);

	#endif

	gItemManager.GCItemListSend(lpObj->Index);

	gSkillManager.GCSkillListSend(lpObj,0);

	gQuest.GCQuestInfoSend(lpObj->Index);

	gCashShop.GCCashShopInitSend(lpObj);

	#if(GAMESERVER_UPDATE>=603)

	gHelper.GCHelperStartSend(lpObj->Index,0,0,1);

	#endif

	GCNewGensBattleInfoSend(lpObj);

	gReconnect.GetReconnectInfo(lpObj);

	gSkillManager.SkillChangeUse(lpObj->Index);

	gObjViewportListProtocolCreate(lpObj);

	gObjectManager.CharacterUpdateMapEffect(lpObj);

	if (gServerInfo.m_CustomRankUserType != 2)
	{
		gCustomRankUser.GCReqRankLevelUser(lpObj->Index, lpObj->Index);
	}

	gNotice.GCNoticeSend(lpObj->Index,0,0,0,0,0,0,gMessage.GetMessage(256),lpObj->Name);

	gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage((248+lpObj->AccountLevel)),lpObj->AccountExpireDate);

	lpObj->MapServerMoveRequest = 0;

	if( lpObj->Authority == 32)
	{
		if (gServerInfo.m_OnlineGmSwitch == 1) 
		{
			#if GAMESERVER_CLIENTE_UPDATE >= 2 
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(639),lpObj->Name);
			#endif
		}
	}
	else
	{
		if (gServerInfo.m_OnlineUserSwitch == 1) 
		{
			#if GAMESERVER_CLIENTE_UPDATE >= 2 
			PostMessageUserON(lpObj->Name,gMessage.GetMessage(640));
			#endif
		}
	}

	gCustomStore.GCOffActiveSend(lpObj->Index,0);

	gCashShop.CGCashShopPointRecv(lpObj->Index);

	if(lpObj->CloseCount <= 0)
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

	LogAddConnect(LOG_BLUE,"[Obj][%d] LogInCharacter (%s)",lpObj->Index,lpObj->Name);
}

void DGCreateItemRecv(SDHP_CREATE_ITEM_RECV* lpMsg) // OK
{
	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGCreateItemRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpMsg->Map != 0xFF && lpMsg->Map != 0xFE && lpMsg->Map != 0xED && lpMsg->Map != 0xEC && lpMsg->Map != 0xEB && MAP_RANGE(lpMsg->Map) == 0)
	{
		return;
	}

	if(lpMsg->Map == 0xFE || lpMsg->Map == 0xFF)
	{
		if((lpMsg->Map == 0xFE && lpObj->Interface.type != INTERFACE_TRAINER) || (lpMsg->Map == 0xFF && lpObj->Interface.type != INTERFACE_CHAOS_BOX))
		{
			return;
		}

		CItem item;

		item.m_Level = lpMsg->Level;

		item.m_Serial = lpMsg->Serial;

		if(lpMsg->ItemIndex == GET_ITEM(13,18) || lpMsg->ItemIndex == GET_ITEM(13,37) || lpMsg->ItemIndex == GET_ITEM(13,51) || lpMsg->ItemIndex == GET_ITEM(14,7) || lpMsg->ItemIndex == GET_ITEM(14,19) || lpMsg->ItemIndex == GET_ITEM(14,109))
		{
			item.m_Durability = (float)lpMsg->Dur;
		}
		else
		{
			item.m_Durability = (float)gItemManager.GetItemDurability(lpMsg->ItemIndex,lpMsg->Level,lpMsg->NewOption,lpMsg->SetOption);
		}

		item.Convert(lpMsg->ItemIndex,lpMsg->Option1,lpMsg->Option2,lpMsg->Option3,lpMsg->NewOption,lpMsg->SetOption,lpMsg->JewelOfHarmonyOption,lpMsg->ItemOptionEx,lpMsg->SocketOption,lpMsg->SocketOptionBonus);

		if(lpMsg->ItemIndex == GET_ITEM(13,4) || lpMsg->ItemIndex == GET_ITEM(13,5))
		{
			item.SetPetItemInfo(1,0);
		}

		item.m_IsPeriodicItem = ((lpMsg->Duration>0)?1:0);

		item.m_LoadPeriodicItem = ((lpMsg->Duration>0)?1:0);

		item.m_PeriodicItemTime = ((lpMsg->Duration>0)?lpMsg->Duration:0);

		gChaosBox.ChaosBoxInit(lpObj);

		gItemManager.ChaosBoxAddItem(lpObj->Index,item,0);

		gChaosBox.GCChaosMixSend(lpObj->Index,((lpMsg->Map==0xFF)?1:100),&item);
	}
	else if(lpMsg->Map == 0xEB)
	{
		CItem item;

		item.m_Level = lpMsg->Level;

		item.m_Serial = lpMsg->Serial;

		item.m_Durability = lpMsg->Dur;

		item.Convert(lpMsg->ItemIndex,lpMsg->Option1,lpMsg->Option2,lpMsg->Option3,lpMsg->NewOption,lpMsg->SetOption,lpMsg->JewelOfHarmonyOption,lpMsg->ItemOptionEx,lpMsg->SocketOption,lpMsg->SocketOptionBonus);

		if(lpMsg->ItemIndex == GET_ITEM(13,4) || lpMsg->ItemIndex == GET_ITEM(13,5))
		{
			item.SetPetItemInfo(1,0);
		}

		item.m_IsPeriodicItem = ((lpMsg->Duration>0)?1:0);

		item.m_LoadPeriodicItem = ((lpMsg->Duration>0)?1:0);

		item.m_PeriodicItemTime = ((lpMsg->Duration>0)?lpMsg->Duration:0);

		BYTE slot = gItemManager.InventoryInsertItem(lpObj->Index,item);

		if(slot != 0xFF)
		{
			gItemManager.GCItemModifySend(lpObj->Index,slot);
			if(lpMsg->Duration > 0){gCashShop.GCCashShopPeriodicItemSend(lpObj->Index,item.m_Index,slot,item.m_PeriodicItemTime);}
		}
	}
	else if(lpMsg->Map == 0xEC)
	{
		CItem item;

		item.m_Level = lpMsg->Level;

		item.m_Serial = lpMsg->Serial;

		item.m_Durability = lpMsg->Dur;

		item.Convert(lpMsg->ItemIndex,lpMsg->Option1,lpMsg->Option2,lpMsg->Option3,lpMsg->NewOption,lpMsg->SetOption,lpMsg->JewelOfHarmonyOption,lpMsg->ItemOptionEx,lpMsg->SocketOption,lpMsg->SocketOptionBonus);

		item.m_IsPeriodicItem = ((lpMsg->Duration>0)?1:0);

		item.m_LoadPeriodicItem = ((lpMsg->Duration>0)?1:0);

		item.m_PeriodicItemTime = ((lpMsg->Duration>0)?lpMsg->Duration:0);

		BYTE slot = gEventInventory.EventInventoryInsertItem(lpObj->Index,item);

		if(slot != 0xFF){gEventInventory.GCEventItemModifySend(lpObj->Index,slot);}
	}
	else if(lpMsg->Map == 0xED)
	{
		CItem item;

		item.m_Level = lpMsg->Level;

		item.m_Serial = lpMsg->Serial;

		item.m_Durability = lpMsg->Dur;

		item.Convert(lpMsg->ItemIndex,lpMsg->Option1,lpMsg->Option2,lpMsg->Option3,lpMsg->NewOption,lpMsg->SetOption,lpMsg->JewelOfHarmonyOption,lpMsg->ItemOptionEx,lpMsg->SocketOption,lpMsg->SocketOptionBonus);

		item.m_IsPeriodicItem = ((lpMsg->Duration>0)?1:0);

		item.m_LoadPeriodicItem = ((lpMsg->Duration>0)?1:0);

		item.m_PeriodicItemTime = ((lpMsg->Duration>0)?lpMsg->Duration:0);

		BYTE slot = gMuunSystem.MuunInventoryInsertItem(lpObj->Index,item);

		if(slot != 0xFF){gMuunSystem.GCMuunItemModifySend(lpObj->Index,slot);}
	}
	else
	{
		if(gMap[lpMsg->Map].MonsterItemDrop(lpMsg->ItemIndex,lpMsg->Level,lpMsg->Dur,lpMsg->X,lpMsg->Y,lpMsg->Option1,lpMsg->Option2,lpMsg->Option3,lpMsg->NewOption,lpMsg->SetOption,lpMsg->LootIndex,lpMsg->Serial,lpMsg->JewelOfHarmonyOption,lpMsg->ItemOptionEx,lpMsg->SocketOption,lpMsg->SocketOptionBonus,lpMsg->Duration) != 0)
		{
			if(BC_MAP_RANGE(lpMsg->Map) != 0)
			{
				gBloodCastle.SetEventItemSerial(lpMsg->Map,lpMsg->ItemIndex,lpMsg->Serial);
			}

			if(IT_MAP_RANGE(lpMsg->Map) != 0)
			{
				gIllusionTemple.SetEventItemSerial(lpMsg->Map,lpMsg->ItemIndex,lpMsg->Serial);
			}
		}
	}
}

void DGPetItemInfoRecv(SDHP_PET_ITEM_INFO_RECV* lpMsg) // OK
{
	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGPetItemInfoRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpMsg->type == 0)
	{
		for(int n=0;n < lpMsg->count;n++)
		{
			SDHP_PET_ITEM_INFO1* lpInfo = (SDHP_PET_ITEM_INFO1*)(((BYTE*)lpMsg)+sizeof(SDHP_PET_ITEM_INFO_RECV)+(sizeof(SDHP_PET_ITEM_INFO1)*n));

			if(INVENTORY_RANGE(lpInfo->slot) == 0)
			{
				continue;
			}

			if(lpObj->Inventory[lpInfo->slot].IsItem() == 0)
			{
				continue;
			}

			if(lpObj->Inventory[lpInfo->slot].m_Serial == lpInfo->serial)
			{
				lpObj->Inventory[lpInfo->slot].SetPetItemInfo(lpInfo->level,lpInfo->experience);

				if(INVENTORY_WEAR_RANGE(lpInfo->slot) != 0)
				{
					gObjectManager.CharacterCalcAttribute(lpObj->Index);
				}
			}
		}
	}
	
	if(lpMsg->type == 1)
	{
		for(int n=0;n < lpMsg->count;n++)
		{
			SDHP_PET_ITEM_INFO1* lpInfo = (SDHP_PET_ITEM_INFO1*)(((BYTE*)lpMsg)+sizeof(SDHP_PET_ITEM_INFO_RECV)+(sizeof(SDHP_PET_ITEM_INFO1)*n));

			if(WAREHOUSE_RANGE(lpInfo->slot) == 0)
			{
				continue;
			}

			if(lpObj->Warehouse[lpInfo->slot].IsItem() == 0)
			{
				continue;
			}

			if(lpObj->Warehouse[lpInfo->slot].m_Serial == lpInfo->serial)
			{
				lpObj->Warehouse[lpInfo->slot].SetPetItemInfo(lpInfo->level,lpInfo->experience);
			}
		}
	}
}

void DGOptionDataRecv(SDHP_OPTION_DATA_RECV* lpMsg) // OK
{
	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGOptionDataRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	#if(GAMESERVER_UPDATE>=701)

	lpObj->ChangeSkin = lpMsg->ChangeSkin;

	gObjViewportListProtocolCreate(lpObj);

	gObjectManager.CharacterUpdateMapEffect(lpObj);

	#endif

	PMSG_OPTION_DATA_SEND pMsg;

	pMsg.header.set(0xF3,0x30,sizeof(pMsg));

	memcpy(pMsg.SkillKey,lpMsg->SkillKey,sizeof(pMsg.SkillKey));

	pMsg.GameOption = lpMsg->GameOption;

	pMsg.QKey = lpMsg->QKey;

	pMsg.WKey = lpMsg->WKey;

	pMsg.EKey = lpMsg->EKey;

	pMsg.ChatWindow = lpMsg->ChatWindow;

	pMsg.RKey = lpMsg->RKey;

	pMsg.QWERLevel = lpMsg->QWERLevel;

	DataSend(lpMsg->index,(BYTE*)&pMsg,pMsg.header.size);
}

void DGCharacterNameCheckRecv(SDHP_CHARACTER_NAME_CHECK_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=401)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGCharacterNameCheckRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	PMSG_CHARACTER_NAME_CHECK_SEND pMsg;
	
	pMsg.header.set(0xF3,0x15,sizeof(pMsg));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = lpMsg->result;

	DataSend(lpMsg->index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void DGCharacterNameChangeRecv(SDHP_CHARACTER_NAME_CHANGE_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=401)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGCharacterNameChangeRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	PMSG_CHARACTER_NAME_CHANGE_SEND pMsg;

	pMsg.header.set(0xF3,0x16,sizeof(pMsg));

	memcpy(pMsg.OldName,lpMsg->OldName,sizeof(pMsg.OldName));

	memcpy(pMsg.NewName,lpMsg->NewName,sizeof(pMsg.NewName));

	pMsg.result = lpMsg->result;

	DataSend(lpMsg->index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void DGCrywolfSyncRecv(SDHP_CRYWOLF_SYNC_RECV* lpMsg) // OK
{
	if(gMapServerManager.GetMapServerGroup() != lpMsg->MapServerGroup)
	{
		return;
	}

	if(gCrywolfSync.GetCrywolfState() == lpMsg->CrywolfState && gCrywolfSync.GetOccupationState() == lpMsg->OccupationState)
	{
		return;
	}

	gCrywolfSync.SetCrywolfState(lpMsg->CrywolfState);

	gCrywolfSync.SetOccupationState(lpMsg->OccupationState);

	LogAdd(LOG_BLUE,"[ CRYWOLF ] SYNC Occupation: %d, State: %d",lpMsg->OccupationState,lpMsg->CrywolfState);
}

void DGCrywolfInfoRecv(SDHP_CRYWOLF_INFO_RECV* lpMsg) // OK
{
	if(gMapServerManager.GetMapServerGroup() != lpMsg->MapServerGroup)
	{
		return;
	}

	if(lpMsg->OccupationState == 0 || lpMsg->OccupationState == 1 || lpMsg->OccupationState == 2)
	{
		gCrywolf.ApplyCrywolfDBInfo(lpMsg->OccupationState);
	}
	else
	{
		LogAdd(LOG_RED,"[ Crywolf ] Incorrect DB OccupationState!!!");
	}
}

void DGGlobalPostRecv(SDHP_GLOBAL_POST_RECV* lpMsg) // OK
{
	if(gMapServerManager.GetMapServerGroup() != lpMsg->MapServerGroup)
	{
		return;
	}

	switch(lpMsg->type)
	{
		case 0:
			PostMessage1(lpMsg->name,gMessage.GetMessage(69),lpMsg->message);
			break;
		case 1:
			PostMessage2(lpMsg->name,gMessage.GetMessage(69),lpMsg->message);
			break;
		case 2:
			PostMessage3(lpMsg->name,gMessage.GetMessage(69),lpMsg->message);
			break;
		case 3:
			PostMessage4(lpMsg->name,gMessage.GetMessage(69),lpMsg->message);
			break;
	}
}

void DGGlobalNoticeRecv(SDHP_GLOBAL_NOTICE_RECV* lpMsg) // OK
{
	if(gMapServerManager.GetMapServerGroup() != lpMsg->MapServerGroup)
	{
		return;
	}

	gNotice.GCNoticeSendToAll(lpMsg->type,lpMsg->count,lpMsg->opacity,lpMsg->delay,lpMsg->color,lpMsg->speed,"%s",lpMsg->message);
}

void DGSNSDataRecv(SDHP_SNS_DATA_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGSNSDataRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	PMSG_SNS_DATA_SEND pMsg;

	pMsg.header.set(0xFB,sizeof(pMsg));

	pMsg.result = lpMsg->result;

	memcpy(pMsg.data,lpMsg->data,sizeof(pMsg.data));

	DataSend(lpMsg->index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void DGGlobalWhisperRecv(SDHP_GLOBAL_WHISPER_RECV* lpMsg) // OK
{
	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGGlobalWhisperRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	if(lpMsg->result == 0)
	{
		GCServerMsgSend(lpMsg->index,0);
		return;
	}

	lpMsg->message[(sizeof(lpMsg->message)-1)] = ((strlen(lpMsg->message)>(sizeof(lpMsg->message)-1))?0:lpMsg->message[(sizeof(lpMsg->message)-1)]);

	gLog.Output(LOG_CHAT,"[Whisper][%s][%s] - (Name: %s, Message: %s)",lpMsg->account,lpMsg->name,lpMsg->TargetName,lpMsg->message);
}

void DGGlobalWhisperEchoRecv(SDHP_GLOBAL_WHISPER_ECHO_RECV* lpMsg) // OK
{
	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGGlobalWhisperEchoRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	lpMsg->message[(sizeof(lpMsg->message)-1)] = ((strlen(lpMsg->message)>(sizeof(lpMsg->message)-1))?0:lpMsg->message[(sizeof(lpMsg->message)-1)]);

	gFilter.CheckSyntax(lpMsg->message);

	GCChatWhisperSend(lpMsg->index,lpMsg->SourceName,lpMsg->message);
}

void GDServerInfoSend() // OK
{
	SDHP_DATA_SERVER_INFO_SEND pMsg;

	pMsg.header.set(0x00,sizeof(pMsg));

	pMsg.type = 1;

	pMsg.ServerPort = (WORD)gServerInfo.m_ServerPort;

	strcpy_s(pMsg.ServerName,gServerInfo.m_ServerName);

	pMsg.ServerCode = (WORD)gServerInfo.m_ServerCode;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GDCharacterListSend(int aIndex) // OK
{
	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_CHARACTER_LIST_SEND pMsg;

	pMsg.header.set(0x01,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GDCharacterCreateSend(int aIndex,char* name,BYTE Class) // OK
{
	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_CHARACTER_CREATE_SEND pMsg;

	pMsg.header.set(0x02,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,name,sizeof(pMsg.name));

	pMsg.Class = Class;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GDCharacterDeleteSend(int aIndex,char* name) // OK
{
	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_CHARACTER_DELETE_SEND pMsg;

	pMsg.header.set(0x03,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,name,sizeof(pMsg.name));

	pMsg.guild = 0;

	if(gObj[aIndex].Guild != 0)
	{
		if(strcmp(gObj[aIndex].Name,gObj[aIndex].Guild->Names[0]) == 0)
		{
			pMsg.guild = 1; // Guild Master
		}
		else
		{
			pMsg.guild = 2; // Guild Member
		}

		memcpy(pMsg.GuildName,gObj[aIndex].Guild->Name,sizeof(pMsg.GuildName));
	}

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GDCharacterInfoSend(int aIndex,char* name) // OK
{
	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_CHARACTER_INFO_SEND pMsg;

	pMsg.header.set(0x04,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GDCreateItemSend(int aIndex,BYTE map,BYTE x,BYTE y,int index,BYTE level,BYTE dur,BYTE Option1,BYTE Option2,BYTE Option3,int LootIndex,BYTE NewOption,BYTE SetOption,BYTE JewelOfHarmonyOption,BYTE ItemOptionEx,BYTE SocketOption[MAX_SOCKET_OPTION],BYTE SocketOptionBonus,DWORD duration) // OK
{
	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_CREATE_ITEM_SEND pMsg;

	pMsg.header.set(0x07,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	pMsg.X = x;
	pMsg.Y = y;
	pMsg.Map = ((map==0xEB)?((gEventInventory.IsEventItem(index)==0)?((gMuunSystem.IsMuunItem(index)==0)?((gMuunSystem.IsMuunUtil(index)==0)?map:0xED):0xED):0xEC):map);
	pMsg.ItemIndex = index;
	pMsg.Level = level;
	pMsg.Dur = ((dur==0)?gItemManager.GetItemDurability(index,level,NewOption,SetOption):dur);
	pMsg.Option1 = Option1;
	pMsg.Option2 = Option2;
	pMsg.Option3 = Option3;
	pMsg.NewOption = NewOption;
	pMsg.LootIndex = LootIndex;
	pMsg.SetOption = SetOption;
	pMsg.JewelOfHarmonyOption = JewelOfHarmonyOption;
	pMsg.ItemOptionEx = ItemOptionEx;
	pMsg.SocketOptionBonus = SocketOptionBonus;
	pMsg.Duration = duration;

	if(SocketOption == 0)
	{
		memset(pMsg.SocketOption,0xFF,MAX_SOCKET_OPTION);
	}
	else
	{
		memcpy(pMsg.SocketOption,SocketOption,MAX_SOCKET_OPTION);
	}

	#if(GAMESERVER_UPDATE>=701)

	if(gPentagramSystem.IsPentagramItem(index) != 0 || gPentagramSystem.IsPentagramJewel(index) != 0 || gPentagramSystem.IsPentagramMithril(index) != 0)
	{
		pMsg.SocketOptionBonus = ((SocketOptionBonus==0xFF)?((gPentagramSystem.IsPentagramMithril(index)==0)?(((GetLargeRand()%5)+1) | 16):((GetLargeRand()%5)+1)):SocketOptionBonus);
	}
	else
	{
		pMsg.SocketOptionBonus = SocketOptionBonus;
	}

	#endif

	#if(GAMESERVER_UPDATE>=803)

	gMuunSystem.GetMuunItemSocketOption(index,&pMsg.Level,&pMsg.SocketOption[0],&pMsg.SocketOptionBonus);

	gMuunSystem.GetMuunUtilSocketOption(index,&pMsg.Level,&pMsg.SocketOption[0],&pMsg.SocketOptionBonus);

	#endif

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GDOptionDataSend(int aIndex) // OK
{
	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_OPTION_DATA_SEND pMsg;

	pMsg.header.set(0x08,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GDPetItemInfoSend(int aIndex,int type) // OK
{
	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	BYTE send[4096];

	SDHP_PET_ITEM_INFO_SEND pMsg;

	pMsg.header.set(0x09,0);

	int size = sizeof(pMsg);

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	pMsg.type = type;

	pMsg.count = 0;

	SDHP_PET_ITEM_INFO2 info;

	if(type == 0)
	{
		for(int n=0;n < INVENTORY_SIZE;n++)
		{
			if(lpObj->Inventory[n].IsItem() == 0)
			{
				continue;
			}

			if(lpObj->Inventory[n].m_Index == GET_ITEM(13,4) || lpObj->Inventory[n].m_Index == GET_ITEM(13,5)) // Dark Horse,Dark Spirit
			{
				info.slot = n;

				info.serial = lpObj->Inventory[n].m_Serial;

				memcpy(&send[size],&info,sizeof(info));
				size += sizeof(info);

				pMsg.count++;
			}
		}
	}
	else if(type == 1)
	{
		for(int n=0;n < WAREHOUSE_SIZE;n++)
		{
			if(lpObj->Warehouse[n].IsItem() == 0)
			{
				continue;
			}

			if(lpObj->Warehouse[n].m_Index == GET_ITEM(13,4) || lpObj->Warehouse[n].m_Index == GET_ITEM(13,5)) // Dark Horse,Dark Spirit
			{
				info.slot = n;

				info.serial = lpObj->Warehouse[n].m_Serial;

				memcpy(&send[size],&info,sizeof(info));
				size += sizeof(info);

				pMsg.count++;
			}
		}
	}

	if(pMsg.count > 0)
	{
		pMsg.header.size[0] = SET_NUMBERHB(size);
		pMsg.header.size[1] = SET_NUMBERLB(size);

		memcpy(send,&pMsg,sizeof(pMsg));

		gDataServerConnection.DataSend(send,size);
	}
}

void GDCharacterNameCheckSend(int aIndex,char* name) // OK
{
	#if(GAMESERVER_UPDATE>=401)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_CHARACTER_NAME_CHECK_SEND pMsg;

	pMsg.header.set(0x0A,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void GDCharacterNameChangeSend(int aIndex,char* OldName,char* NewName) // OK
{
	#if(GAMESERVER_UPDATE>=401)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_CHARACTER_NAME_CHANGE_SEND pMsg;

	pMsg.header.set(0x0B,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.OldName,OldName,sizeof(pMsg.OldName));

	memcpy(pMsg.NewName,NewName,sizeof(pMsg.NewName));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void GDCrywolfSyncSend(int MapServerGroup,int CrywolfState,int OccupationState) // OK
{
	SDHP_CRYWOLF_SYNC_SEND pMsg;

	pMsg.header.set(0x1E,sizeof(pMsg));

	pMsg.MapServerGroup = MapServerGroup;

	pMsg.CrywolfState = CrywolfState;

	pMsg.OccupationState = OccupationState;

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));
}

void GDCrywolfInfoSend(int MapServerGroup) // OK
{
	SDHP_CRYWOLF_INFO_SEND pMsg;

	pMsg.header.set(0x1F,sizeof(pMsg));

	pMsg.MapServerGroup = MapServerGroup;

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));
}

void GDGlobalPostSend(int MapServerGroup,BYTE type,char* name,char* message) // OK
{
	SDHP_GLOBAL_POST_SEND pMsg;

	pMsg.header.set(0x20,sizeof(pMsg));

	pMsg.MapServerGroup = MapServerGroup;

	pMsg.type = type;

	memcpy(pMsg.name,name,sizeof(pMsg.name));

	memcpy(pMsg.message,message,sizeof(pMsg.message));

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));
}

void GDGlobalNoticeSend(int MapServerGroup,BYTE type,BYTE count,BYTE opacity,WORD delay,DWORD color,BYTE speed,char* message) // OK
{
	SDHP_GLOBAL_NOTICE_SEND pMsg;

	pMsg.header.set(0x21,sizeof(pMsg));

	pMsg.MapServerGroup = MapServerGroup;

	pMsg.type = type;

	pMsg.count = count;

	pMsg.opacity = opacity;

	pMsg.delay = delay;

	pMsg.color = color;

	pMsg.speed = speed;

	memcpy(pMsg.message,message,sizeof(pMsg.message));

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));
}

void GDSNSDataSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_SNS_DATA_SEND pMsg;

	pMsg.header.set(0x24,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void GDCharacterInfoSaveSend(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->MapServerMoveQuit != 0)
	{
		return;
	}

	SDHP_CHARACTER_INFO_SAVE_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.index = lpObj->Index;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.Level = lpObj->Level;
	pMsg.Class = lpObj->DBClass;
	pMsg.LevelUpPoint = lpObj->LevelUpPoint;
	pMsg.Experience = lpObj->Experience;
	pMsg.Money = lpObj->Money;
	pMsg.Strength = lpObj->Strength;
	pMsg.Dexterity = lpObj->Dexterity;
	pMsg.Vitality = lpObj->Vitality;
	pMsg.Energy = lpObj->Energy;
	pMsg.Leadership = lpObj->Leadership;
	pMsg.Life = (DWORD)lpObj->Life;
	pMsg.MaxLife = (DWORD)lpObj->MaxLife;
	pMsg.Mana = (DWORD)lpObj->Mana;
	pMsg.MaxMana = (DWORD)lpObj->MaxMana;
	pMsg.BP = (DWORD)lpObj->BP;
	pMsg.MaxBP = (DWORD)lpObj->MaxBP;
	pMsg.Shield = (DWORD)lpObj->Shield;
	pMsg.MaxShield = (DWORD)lpObj->MaxShield;

	pMsg.Kills = (DWORD)lpObj->Kills;
	pMsg.Deads = (DWORD)lpObj->Deads;

	for(int n=0;n < INVENTORY_SIZE;n++){gItemManager.DBItemByteConvert(pMsg.Inventory[n],&lpObj->Inventory[n]);}

	for(int n=0;n < MAX_SKILL_LIST;n++){gSkillManager.SkillByteConvert(pMsg.Skill[n],&lpObj->Skill[n]);}

	pMsg.Map = lpObj->Map;
	pMsg.X = (BYTE)lpObj->X;
	pMsg.Y = (BYTE)lpObj->Y;
	pMsg.Dir = lpObj->Dir;
	pMsg.PKCount = lpObj->PKCount;
	pMsg.PKLevel = lpObj->PKLevel;
	pMsg.PKTime = lpObj->PKTime;

	memcpy(pMsg.Quest,lpObj->Quest,sizeof(pMsg.Quest));

	pMsg.FruitAddPoint = lpObj->FruitAddPoint;
	pMsg.FruitSubPoint = lpObj->FruitSubPoint;

	for(int n=0;n < MAX_EFFECT_LIST;n++){gEffectManager.EffectByteConvert(pMsg.Effect[n],&lpObj->Effect[n]);}

	#if(GAMESERVER_UPDATE>=602)
	//pMsg.ExtInventory = lpObj->ExtInventory;
	//pMsg.ExtWarehouse = lpObj->ExtWarehouse;
	#endif

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));

	GDPetItemInfoSaveSend(aIndex,0);

	if (lpObj->LoadGuildWarehouse != 0)
	{
		gWarehouse.GDWarehouseGuildItemSaveSend(aIndex);
	}
	else
	{
		gWarehouse.GDWarehouseItemSaveSend(aIndex);
	}

	gQuest.GDQuestKillCountSaveSend(aIndex);

	#if(GAMESERVER_UPDATE>=401)

	gMasterSkillTree.GDMasterSkillTreeSaveSend(aIndex);

	#endif

	#if(GAMESERVER_UPDATE>=501)

	gQuestWorld.GDQuestWorldSaveSend(aIndex);

	#endif

	gCashShop.GDCashShopPeriodicItemSaveSend(aIndex);

	#if(GAMESERVER_UPDATE>=602)

	gLuckyItem.GDLuckyItemSaveSend(aIndex);

	#endif

	#if(GAMESERVER_UPDATE>=701)

	gPentagramSystem.GDPentagramJewelInfoSaveSend(aIndex,PENTAGRAM_JEWEL_TYPE_INVENTORY);

	gPentagramSystem.GDPentagramJewelInfoSaveSend(aIndex,PENTAGRAM_JEWEL_TYPE_WAREHOUSE);

	#endif

	#if(GAMESERVER_UPDATE>=802)

	gPersonalShop.GDPShopItemValueSaveSend(aIndex);

	gEventInventory.GDEventInventorySaveSend(aIndex);

	gMuRummy.GDReqMuRummyInfoUpdate(lpObj);

	#endif

	#if(GAMESERVER_UPDATE>=803)

	gMuunSystem.GDMuunInventorySaveSend(aIndex);

	#endif
}

void GDInventoryItemSaveSend(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	SDHP_INVENTORY_ITEM_SAVE_SEND pMsg;

	pMsg.header.set(0x31,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	for(int n=0;n < INVENTORY_SIZE;n++){gItemManager.DBItemByteConvert(pMsg.Inventory[n],&lpObj->Inventory[n]);}

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));
}

void GDOptionDataSaveSend(int aIndex,BYTE* SkillKey,BYTE GameOption,BYTE QKey,BYTE WKey,BYTE EKey,BYTE ChatWindow,BYTE RKey,DWORD QWERLevel) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	SDHP_OPTION_DATA_SAVE_SEND pMsg;

	pMsg.header.set(0x33,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	memcpy(pMsg.SkillKey,SkillKey,sizeof(pMsg.SkillKey));

	pMsg.GameOption = GameOption;

	pMsg.QKey = QKey;

	pMsg.WKey = WKey;

	pMsg.EKey = EKey;

	pMsg.ChatWindow = ChatWindow;

	pMsg.RKey = RKey;

	pMsg.QWERLevel = QWERLevel;

	#if(GAMESERVER_UPDATE>=701)

	pMsg.ChangeSkin = lpObj->ChangeSkin;

	#endif

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GDPetItemInfoSaveSend(int aIndex,int type) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	BYTE send[4096];

	SDHP_PET_ITEM_INFO_SAVE_SEND pMsg;

	pMsg.header.set(0x34,0);

	int size = sizeof(pMsg);

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	pMsg.count = 0;

	SDHP_PET_ITEM_INFO_SAVE info;

	if(type == 0)
	{
		for(int n=0;n < INVENTORY_SIZE;n++)
		{
			if(lpObj->Inventory[n].IsItem() == 0)
			{
				continue;
			}

			if(lpObj->Inventory[n].m_Index == GET_ITEM(13,4) || lpObj->Inventory[n].m_Index == GET_ITEM(13,5)) // Dark Horse,Dark Spirit
			{
				if(lpObj->Inventory[n].m_LoadPetItemInfo != 0)
				{
					info.serial = lpObj->Inventory[n].m_Serial;

					info.level = lpObj->Inventory[n].m_PetItemLevel;

					info.experience = lpObj->Inventory[n].m_PetItemExp;

					memcpy(&send[size],&info,sizeof(info));
					size += sizeof(info);

					pMsg.count++;
				}
			}
		}
	}
	else if(type == 1)
	{
		for(int n=0;n < WAREHOUSE_SIZE;n++)
		{
			if(lpObj->Warehouse[n].IsItem() == 0)
			{
				continue;
			}

			if(lpObj->Warehouse[n].m_Index == GET_ITEM(13,4) || lpObj->Warehouse[n].m_Index == GET_ITEM(13,5)) // Dark Horse,Dark Spirit
			{
				if(lpObj->Warehouse[n].m_LoadPetItemInfo != 0)
				{
					info.serial = lpObj->Warehouse[n].m_Serial;

					info.level = lpObj->Warehouse[n].m_PetItemLevel;

					info.experience = lpObj->Warehouse[n].m_PetItemExp;

					memcpy(&send[size],&info,sizeof(info));
					size += sizeof(info);

					pMsg.count++;
				}
			}
		}
	}

	if(pMsg.count > 0)
	{
		pMsg.header.size[0] = SET_NUMBERHB(size);
		pMsg.header.size[1] = SET_NUMBERLB(size);

		memcpy(send,&pMsg,sizeof(pMsg));

		gDataServerConnection.DataSend(send,size);
	}
}

void GDResetInfoSaveSend(int aIndex,int ResetDay,int ResetWek,int ResetMon) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	SDHP_RESET_INFO_SAVE_SEND pMsg;

	pMsg.header.set(0x39,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.Reset = lpObj->Reset;

	pMsg.ResetDay = ResetDay;

	pMsg.ResetWek = ResetWek;

	pMsg.ResetMon = ResetMon;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GDMasterResetInfoSaveSend(int aIndex,int MasterResetDay,int MasterResetWek,int MasterResetMon) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	SDHP_MASTER_RESET_INFO_SAVE_SEND pMsg;

	pMsg.header.set(0x3A,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.Reset = lpObj->Reset;

	pMsg.MasterReset = lpObj->MasterReset;

	pMsg.MasterResetDay = MasterResetDay;

	pMsg.MasterResetWek = MasterResetWek;

	pMsg.MasterResetMon = MasterResetMon;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GDRankingDuelSaveSend(int aIndex,DWORD WinScore,DWORD LoseScore) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	SDHP_RANKING_DUEL_SAVE_SEND pMsg;

	pMsg.header.set(0x3C,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.WinScore = WinScore;

	pMsg.LoseScore = LoseScore;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GDRankingBloodCastleSaveSend(int aIndex,DWORD Score) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	SDHP_RANKING_BLOOD_CASTLE_SAVE_SEND pMsg;

	pMsg.header.set(0x3D,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.score = Score;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GDRankingChaosCastleSaveSend(int aIndex,DWORD Score) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	SDHP_RANKING_CHAOS_CASTLE_SAVE_SEND pMsg;

	pMsg.header.set(0x3E,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.score = Score;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GDRankingDevilSquareSaveSend(int aIndex,DWORD Score) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	SDHP_RANKING_DEVIL_SQUARE_SAVE_SEND pMsg;

	pMsg.header.set(0x3F,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.score = Score;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GDRankingIllusionTempleSaveSend(int aIndex,DWORD Score) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	SDHP_RANKING_ILLUSION_TEMPLE_SAVE_SEND pMsg;

	pMsg.header.set(0x40,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.score = Score;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GDCreationCardSaveSend(int aIndex,BYTE ExtClass) // OK
{
	SDHP_CREATION_CARD_SAVE_SEND pMsg;

	pMsg.header.set(0x42,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	pMsg.ExtClass = ExtClass;

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));
}

void GDCrywolfInfoSaveSend(int MapServerGroup,int CrywolfState,int OccupationState) // OK
{
	SDHP_CRYWOLF_INFO_SAVE_SEND pMsg;

	pMsg.header.set(0x49,sizeof(pMsg));

	pMsg.MapServerGroup = MapServerGroup;

	pMsg.CrywolfState = CrywolfState;

	pMsg.OccupationState = OccupationState;

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));
}

void GDSNSDataSaveSend(int aIndex,BYTE* data) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	SDHP_SNS_DATA_SAVE_SEND pMsg;

	pMsg.header.set(0x4E,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.account));

	memcpy(pMsg.data,data,sizeof(pMsg.data));

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void GDCustomMonsterRewardSaveSend(int aIndex,int MonsterClass,int MapNumber,int RewardValue1,int RewardValue2) // OK
{
	SDHP_CUSTOM_MONSTER_REWARD_SAVE_SEND pMsg;

	pMsg.header.set(0x52,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	pMsg.MonsterClass = MonsterClass;

	pMsg.MapNumber = MapNumber;

	pMsg.RewardValue1 = RewardValue1;

	pMsg.RewardValue2 = RewardValue2;

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));
}

void GDRankingCustomArenaSaveSend(int aIndex,DWORD number,DWORD score,DWORD rank) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	SDHP_RANKING_CUSTOM_ARENA_SAVE_SEND pMsg;

	pMsg.header.set(0x55,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.number = number;

	pMsg.score = score;

	pMsg.rank = rank;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GDConnectCharacterSend(int aIndex) // OK
{
	SDHP_CONNECT_CHARACTER_SEND pMsg;

	pMsg.header.set(0x70,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));
}

void GDDisconnectCharacterSend(int aIndex) // OK
{
	SDHP_DISCONNECT_CHARACTER_SEND pMsg;

	pMsg.header.set(0x71,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));
}

void GDGlobalWhisperSend(int aIndex,char* TargetName,char* message) // OK
{
	SDHP_GLOBAL_WHISPER_SEND pMsg;

	pMsg.header.set(0x72,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	memcpy(pMsg.TargetName,TargetName,sizeof(pMsg.TargetName));

	memcpy(pMsg.message,message,sizeof(pMsg.message));

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));
}

//**************************************************************************//
// RAW FUNCTIONS ***********************************************************//
//**************************************************************************//

void GS_GDReqCastleTotalInfo(int iMapSvrGroup, int iCastleEventCycle)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	CSP_REQ_CASTLEDATA pMsg;

	pMsg.h.set(0x80, 0x00, sizeof(CSP_REQ_CASTLEDATA));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iCastleEventCycle = iCastleEventCycle;

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqOwnerGuildMaster(int iMapSvrGroup, int aIndex)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	if ( OBJECT_RANGE(aIndex) == FALSE )
	{
		return;
	}


	CSP_REQ_OWNERGUILDMASTER pMsg;

	pMsg.h.set(0x80, 0x01, sizeof(CSP_REQ_OWNERGUILDMASTER));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iIndex = aIndex;

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqCastleNpcBuy(int iMapSvrGroup, int aIndex, int iNpcNumber, int iNpcIndex, int iNpcDfLevel, int iNpcRgLevel, int iNpcMaxHP, int iNpcHP, BYTE btNpcX, BYTE btNpcY, BYTE btNpcDIR, int iBuyCost)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	if ( OBJECT_RANGE(aIndex) == FALSE )
	{
		return;
	}


	CSP_REQ_NPCBUY pMsg;

	pMsg.h.set(0x80, 0x03, sizeof(CSP_REQ_NPCBUY));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iIndex = aIndex;
	pMsg.iNpcNumber = iNpcNumber;
	pMsg.iNpcIndex = iNpcIndex;
	pMsg.iNpcDfLevel = iNpcDfLevel;
	pMsg.iNpcRgLevel = iNpcRgLevel;
	pMsg.iNpcMaxHp = iNpcMaxHP;
	pMsg.iNpcHp = iNpcHP;
	pMsg.btNpcX = btNpcX;
	pMsg.btNpcY = btNpcY;
	pMsg.btNpcDIR = btNpcDIR;
	pMsg.iBuyCost = iBuyCost;

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqCastleNpcRepair(int iMapSvrGroup, int aIndex, int iNpcNumber, int iNpcIndex, int iRepairCost)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	if ( OBJECT_RANGE(aIndex) == FALSE )
	{
		return;
	}


	CSP_REQ_NPCREPAIR pMsg;

	pMsg.h.set(0x80, 0x04, sizeof(CSP_REQ_NPCREPAIR));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iIndex = aIndex;
	pMsg.iNpcNumber = iNpcNumber;
	pMsg.iNpcIndex = iNpcIndex;
	pMsg.iRepairCost = iRepairCost;

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqCastleNpcUpgrade(int iMapSvrGroup, int aIndex, int iNpcNumber, int iNpcIndex, int iNpcUpType, int iNpcUpValue, int iNpcUpIndex)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	if ( OBJECT_RANGE(aIndex) == FALSE )
	{
		return;
	}


	CSP_REQ_NPCUPGRADE pMsg;

	pMsg.h.set(0x80, 0x05, sizeof(CSP_REQ_NPCUPGRADE));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iIndex = aIndex;
	pMsg.iNpcNumber = iNpcNumber;
	pMsg.iNpcIndex = iNpcIndex;
	pMsg.iNpcUpType = iNpcUpType;
	pMsg.iNpcUpValue = iNpcUpValue;
	pMsg.iNpcUpIndex = iNpcUpIndex;

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqTaxInfo(int iMapSvrGroup, int aIndex)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	if ( OBJECT_RANGE(aIndex) == FALSE )
	{
		return;
	}


	CSP_REQ_TAXINFO pMsg;

	pMsg.h.set(0x80, 0x06, sizeof(CSP_REQ_TAXINFO));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iIndex = aIndex;

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqTaxRateChange( int iMapSvrGroup, int aIndex, int iTaxType, int iTaxRate)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	if ( OBJECT_RANGE(aIndex) == FALSE )
	{
		return;
	}


	CSP_REQ_TAXRATECHANGE pMsg;

	pMsg.h.set(0x80, 0x07, sizeof(CSP_REQ_TAXRATECHANGE));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iIndex = aIndex;
	pMsg.iTaxRate = iTaxRate; 
	pMsg.iTaxKind = iTaxType;
	
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqCastleMoneyChange(int iMapSvrGroup, int aIndex,int iMoneyChange)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	if ( OBJECT_RANGE(aIndex) == FALSE )
	{
		return;
	}


	CSP_REQ_MONEYCHANGE pMsg;

	pMsg.h.set(0x80, 0x08, sizeof(CSP_REQ_MONEYCHANGE));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iIndex = aIndex;
	pMsg.iMoneyChanged = iMoneyChange;
	
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqSiegeDateChange(int iMapSvrGroup, int aIndex,WORD wStartYear, BYTE btStartMonth, BYTE btStartDay, WORD wEndYear, BYTE btEndMonth, BYTE btEndDay)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	CSP_REQ_SDEDCHANGE pMsg;

	pMsg.h.set(0x80, 0x09, sizeof(CSP_REQ_SDEDCHANGE));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iIndex = aIndex;
	pMsg.wStartYear = wStartYear;
	pMsg.btStartMonth = btStartMonth;
	pMsg.btStartDay = btStartDay;
	pMsg.wEndYear = wEndYear;
	pMsg.btEndMonth = btEndMonth;
	pMsg.btEndDay = btEndDay;
	
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqGuildMarkRegInfo(int iMapSvrGroup, int aIndex)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	if ( OBJECT_RANGE(aIndex) == FALSE )
	{
		return;
	}


	CSP_REQ_GUILDREGINFO pMsg;

	pMsg.h.set(0x80, 0x0A, sizeof(CSP_REQ_GUILDREGINFO));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iIndex = aIndex;
	memcpy(pMsg.szGuildName, gObj[aIndex].GuildName, 8);
	
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqSiegeEndedChange(int iMapSvrGroup, BOOL bIsSiegeEnded)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}


	CSP_REQ_SIEGEENDCHANGE pMsg;

	pMsg.h.set(0x80, 0x0B, sizeof(CSP_REQ_SIEGEENDCHANGE));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.bIsSiegeEnded = bIsSiegeEnded;
	
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqCastleOwnerChange( int iMapSvrGroup, BOOL bIsCastleOccupied, char* lpszGuildName)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	if ( lpszGuildName == NULL )
	{
		return;
	}


	CSP_REQ_CASTLEOWNERCHANGE pMsg;

	pMsg.h.set(0x80, 0x0C, sizeof(CSP_REQ_CASTLEOWNERCHANGE));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.bIsCastleOccupied = bIsCastleOccupied;
	memcpy(pMsg.szOwnerGuildName, lpszGuildName, 8);
	
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqRegAttackGuild( int iMapSvrGroup, int aIndex)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	if ( OBJECT_RANGE(aIndex) == FALSE )
	{
		return;
	}


	CSP_REQ_REGATTACKGUILD pMsg;

	pMsg.h.set(0x80, 0x0D, sizeof(CSP_REQ_REGATTACKGUILD));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iIndex = aIndex;
	memcpy(pMsg.szEnemyGuildName, gObj[aIndex].GuildName, 8);
	
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqRestartCastleState( int iMapSvrGroup)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	CSP_REQ_CASTLESIEGEEND pMsg;

	pMsg.h.set(0x80, 0x0E, sizeof(CSP_REQ_CASTLESIEGEEND));
	pMsg.wMapSvrNum = iMapSvrGroup;
	
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqMapSvrMsgMultiCast( int iMapSvrGroup,char * lpszMsgText)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	if ( lpszMsgText == NULL )
	{
		return;
	}


	CSP_REQ_MAPSVRMULTICAST pMsg;

	pMsg.h.set(0x80, 0x0F, sizeof(CSP_REQ_MAPSVRMULTICAST));
	pMsg.wMapSvrNum = iMapSvrGroup;
	strcpy_s(pMsg.szMsgText, lpszMsgText);
	pMsg.szMsgText[127] = 0;
	
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqRegGuildMark( int iMapSvrGroup, int aIndex, int iItemPos)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	if ( OBJECT_RANGE(aIndex) == FALSE )
	{
		return;
	}

	CSP_REQ_GUILDREGMARK pMsg;

	pMsg.h.set(0x80,0x10, sizeof(CSP_REQ_GUILDREGMARK));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iIndex = aIndex;
	pMsg.iItemPos = iItemPos;
	memcpy(&pMsg.szGuildName,gObj[aIndex].GuildName,sizeof(pMsg.szGuildName));
	
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqGuildMarkReset( int iMapSvrGroup, int aIndex, char* lpszGuildName)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	if ( OBJECT_RANGE(aIndex) == FALSE )
	{
		return;
	}

	if ( lpszGuildName == NULL )
	{
		return;
	}

	CSP_REQ_GUILDRESETMARK pMsg;

	pMsg.h.set(0x80, 0x11, sizeof(CSP_REQ_GUILDRESETMARK));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iIndex = aIndex;
	memcpy(pMsg.szGuildName, lpszGuildName, 8);
	
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqGuildSetGiveUp( int iMapSvrGroup, int aIndex, BOOL bIsGiveUp)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	if ( OBJECT_RANGE(aIndex) == FALSE )
	{
		return;
	}

	CSP_REQ_GUILDSETGIVEUP pMsg;

	pMsg.h.set(0x80, 0x12, sizeof(CSP_REQ_GUILDSETGIVEUP));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iIndex = aIndex;
	pMsg.bIsGiveUp = bIsGiveUp;
	memcpy(pMsg.szGuildName, gObj[aIndex].GuildName, 8);
	
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqNpcRemove(int iMapSvrGroup, int iNpcNumber, int iNpcIndex)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	CSP_REQ_NPCREMOVE pMsg;

	pMsg.h.set(0x80, 0x16, sizeof(CSP_REQ_NPCREMOVE));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iNpcNumber = iNpcNumber;
	pMsg.iNpcIndex = iNpcIndex;
	
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqCastleStateSync( int iMapSvrGroup, int iCastleState, int iTaxRateChaos, int iTaxRateStore, int iTaxHuntZone, char* lpszOwnerGuild)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	CSP_REQ_CASTLESTATESYNC pMsg;

	pMsg.h.set(0x80, 0x17, sizeof(CSP_REQ_CASTLESTATESYNC));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iCastleState = iCastleState;
	pMsg.iTaxRateChaos = iTaxRateChaos;
	pMsg.iTaxRateStore = iTaxRateStore;
	pMsg.iTaxHuntZone = iTaxHuntZone;
	memcpy(pMsg.szOwnerGuildName, lpszOwnerGuild, 8);
	
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqCastleTributeMoney(int iMapSvrGroup, int iCastleTributeMoney)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	if ( iCastleTributeMoney < 0 )
	{
		return;
	}

	CSP_REQ_CASTLETRIBUTEMONEY pMsg;

	pMsg.h.set(0x80, 0x18, sizeof(CSP_REQ_CASTLETRIBUTEMONEY));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iCastleTributeMoney = iCastleTributeMoney;

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqResetCastleTaxInfo( int iMapSvrGroup)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	CSP_REQ_RESETCASTLETAXINFO pMsg;

	pMsg.h.set(0x80, 0x19, sizeof(CSP_REQ_RESETCASTLETAXINFO));
	pMsg.wMapSvrNum = iMapSvrGroup;

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqResetSiegeGuildInfo( int iMapSvrGroup)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	CSP_REQ_RESETSIEGEGUILDINFO pMsg;

	pMsg.h.set(0x80, 0x1A, sizeof(CSP_REQ_RESETSIEGEGUILDINFO));
	pMsg.wMapSvrNum = iMapSvrGroup;

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqResetRegSiegeInfo( int iMapSvrGroup)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	CSP_REQ_RESETREGSIEGEINFO pMsg;

	pMsg.h.set(0x80, 0x1B, sizeof(CSP_REQ_RESETREGSIEGEINFO));
	pMsg.wMapSvrNum = iMapSvrGroup;

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqCastleInitData( int iMapSvrGroup, int iCastleEventCycle)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	CSP_REQ_CSINITDATA pMsg;

	pMsg.h.set(0x81, sizeof(CSP_REQ_CSINITDATA));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iCastleEventCycle = iCastleEventCycle;

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqCastleNpcInfo( int iMapSvrGroup, int aIndex)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	CSP_REQ_NPCDATA pMsg;

	pMsg.h.set(0x82, sizeof(CSP_REQ_NPCDATA));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iIndex = aIndex;

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqAllGuildMarkRegInfo( int iMapSvrGroup, int aIndex)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	CSP_REQ_ALLGUILDREGINFO pMsg;

	pMsg.h.set(0x83, sizeof(CSP_REQ_ALLGUILDREGINFO));
	pMsg.wMapSvrNum = iMapSvrGroup;
	pMsg.iIndex = aIndex;

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqCalcRegGuildList(int iMapSvrGroup)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	CSP_REQ_CALCREGGUILDLIST pMsg;

	pMsg.h.set(0x85, sizeof(CSP_REQ_CALCREGGUILDLIST));
	pMsg.wMapSvrNum = iMapSvrGroup;

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_GDReqCsLoadTotalGuildInfo(int iMapSvrGroup)
{
	if ( iMapSvrGroup < 0 )
	{
		return;
	}

	CSP_REQ_CSLOADTOTALGUILDINFO pMsg;

	pMsg.h.set(0x88, sizeof(CSP_REQ_CSLOADTOTALGUILDINFO));
	pMsg.wMapSvrNum = iMapSvrGroup;

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}

void GS_DGAnsCastleTotalInfo(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_CASTLEDATA* lpMsg = (CSP_ANS_CASTLEDATA*)lpRecv;

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x80][0x00] GS_DGAnsCastleTotalInfo() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()");
#else
	return;
#endif
}

void GS_DGAnsOwnerGuildMaster(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_OWNERGUILDMASTER* lpMsg = (CSP_ANS_OWNERGUILDMASTER*)lpRecv;

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x80][0x00] GS_DGAnsCastleTotalInfo() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}

	GCAnsCastleSiegeState(lpMsg->iIndex,lpMsg->iResult,lpMsg->szCastleOwnGuild,lpMsg->szCastleOwnGuildMaster);
#else
	return;
#endif
}

void GS_DGAnsCastleNpcBuy(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_NPCBUY* lpMsg = (CSP_ANS_NPCBUY*)lpRecv;

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x80][0x03] GS_DGAnsCastleNpcBuy() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}

	if (lpMsg->iResult == 1)
	{
		BOOL bAddResult = gCastleSiege.AddDbNPC(lpMsg->iNpcNumber,lpMsg->iNpcIndex);
	
		if ( bAddResult == TRUE)
		{
			if(gObjIsConnected(lpMsg->iIndex))
			{
				gObj[lpMsg->iIndex].Money -= lpMsg->iBuyCost;

				if(gObj[lpMsg->iIndex].Money < 0 )
				{
					gObj[lpMsg->iIndex].Money = 0;
				}

				GCMoneySend(lpMsg->iIndex,gObj[lpMsg->iIndex].Money);
			}

			LogAdd(LOG_BLACK,"[CastleSiege] GS_DGAnsCastleNpcBuy() - CCastleSiege::AddDbNPC() OK - Npc:(CLS:%d, IDX:%d)",lpMsg->iNpcNumber,lpMsg->iNpcIndex);
		}
		else
		{
			LogAdd(LOG_BLACK,"[CastleSiege] GS_DGAnsCastleNpcBuy() - CCastleSiege::AddDbNPC() FAILED - Npc:(CLS:%d, IDX:%d)",lpMsg->iNpcNumber,lpMsg->iNpcIndex);
		}
	}
	else
	{
		LogAdd(LOG_BLACK,"[CastleSiege] GS_DGAnsCastleNpcBuy() FAILED - Result:(%d), Npc:(CLS:%d, IDX:%d)",lpMsg->iResult,lpMsg->iNpcNumber,lpMsg->iNpcIndex); 
	}

	GCAnsNpcBuy(lpMsg->iIndex,lpMsg->iResult,lpMsg->iNpcNumber,lpMsg->iNpcIndex);
#else
	return;
#endif
}

void GS_DGAnsCastleNpcRepair(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_NPCREPAIR* lpMsg = (CSP_ANS_NPCREPAIR*) lpRecv;

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x80][0x04] GS_DGAnsCastleNpcRepair() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}

	if(lpMsg->iResult == 1)
	{
		BOOL bRepairResult =  gCastleSiege.RepairDbNPC(lpMsg->iNpcNumber,lpMsg->iNpcIndex,lpMsg->iNpcHp,lpMsg->iNpcMaxHp);
		if( bRepairResult == TRUE)
		{
			if(gObjIsConnected(lpMsg->iIndex))
			{
				gObj[lpMsg->iIndex].Money -= lpMsg->iRepairCost;

				if(gObj[lpMsg->iIndex].Money < 0 )
				{
					gObj[lpMsg->iIndex].Money = 0;
				}

				GCMoneySend(lpMsg->iIndex,gObj[lpMsg->iIndex].Money);
			}

			LogAdd(LOG_BLACK,"[CastleSiege] GS_DGAnsCastleNpcRepair() - CCastleSiege::RepairDbNPC() OK - Npc:(CLS:%d, IDX:%d)",lpMsg->iNpcNumber,lpMsg->iNpcIndex);
		}
		else
		{
			LogAdd(LOG_BLACK,"[CastleSiege] GS_DGAnsCastleNpcRepair() - CCastleSiege::RepairDbNPC() FAILED - Npc:(CLS:%d, IDX:%d)",lpMsg->iNpcNumber,lpMsg->iNpcIndex);
		}
	}
	else
	{
		LogAdd(LOG_BLACK,"[CastleSiege] GS_DGAnsCastleNpcRepair() FAILED - Result:(%d), Npc:(CLS:%d, IDX:%d)",lpMsg->iResult,lpMsg->iNpcNumber,lpMsg->iNpcIndex);
	}

	GCAnsNpcRepair(lpMsg->iIndex,lpMsg->iResult,lpMsg->iNpcNumber,lpMsg->iNpcIndex,lpMsg->iNpcHp,lpMsg->iNpcMaxHp);
#else
	return;
#endif
}

void GS_DGAnsCastleNpcUpgrade(LPBYTE lpRecv)
{
#if (GAMESERVER_TYPE==1)
	CSP_ANS_NPCUPGRADE* lpMsg = (CSP_ANS_NPCUPGRADE*)lpRecv;

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x80][0x05] GS_DGAnsCastleNpcUpgrade() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}

	if(lpMsg->iResult == 0)
	{
		LogAdd(LOG_RED,"[CastleSiege] ERROR - Castle NPC Upgrade Fail() (CLS:%d, IDX:%d, UPTYPE:%d, UPVAL:%d)",lpMsg->iNpcNumber,lpMsg->iNpcIndex,lpMsg->iNpcUpType,lpMsg->iNpcUpValue);
	}
	else
	{
		gCastleSiege.UpgradeDbNPC(lpMsg->iIndex,lpMsg->iNpcNumber,lpMsg->iNpcIndex,lpMsg->iNpcUpType,lpMsg->iNpcUpValue,lpMsg->iNpcUpIndex);
		LogAdd(LOG_BLACK,"[CastleSiege] [0x80][0x06] GS_DGAnsTaxInfo() - Npc Upgrade OK (CLS:%d, IDX:%d, UPTYPE:%d, UPVAL:%d)",lpMsg->iNpcNumber, lpMsg->iNpcIndex,lpMsg->iNpcUpType,lpMsg->iNpcUpValue);
	}

	GCAnsNpcUpgrade(lpMsg->iIndex,lpMsg->iResult,lpMsg->iNpcNumber,lpMsg->iNpcIndex,lpMsg->iNpcUpType,lpMsg->iNpcUpValue);

#else
	return;
#endif
}

void GS_DGAnsTaxInfo(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_TAXINFO* lpMsg = (CSP_ANS_TAXINFO*)lpRecv;

	if(lpMsg == NULL)
		return;
	
	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x80][0x06] GS_DGAnsTaxInfo() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}

	if(lpMsg->iResult == TRUE)
	{
		GCAnsTaxMoneyInfo(lpMsg->iIndex,lpMsg->iResult,lpMsg->iTaxRateChaos,lpMsg->iTaxRateStore,lpMsg->i64CastleMoney);
		gCastleSiege.SetCastleMoney(lpMsg->i64CastleMoney);
	}
#else
	return;
#endif
}

void GS_DGAnsTaxRateChange(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_TAXRATECHANGE* lpMsg = (CSP_ANS_TAXRATECHANGE*)lpRecv;

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x80][0x07] GS_DGAnsTaxRateChange() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}

	if(lpMsg->iResult == 1)
	{
		gCastleSiege.SetTaxRate(lpMsg->iTaxKind,lpMsg->iTaxRate);
		GCAnsTaxRateChange(lpMsg->iIndex,lpMsg->iResult,lpMsg->iTaxKind,lpMsg->iTaxRate);
	}
#else
	return;
#endif
}

void GS_DGAnsCastleMoneyChange(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_MONEYCHANGE* lpMsg = (CSP_ANS_MONEYCHANGE*)lpRecv;

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x80][0x08] GS_DGAnsCastleMoneyChange() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}

	if(lpMsg->iResult == TRUE)
	{
		if(gObjIsConnected(lpMsg->iIndex))
		{
			if(lpMsg->iMoneyChanged < 0)
			{
				gObj[lpMsg->iIndex].Money -= lpMsg->iMoneyChanged;

				if(gObj[lpMsg->iIndex].Money > MAX_MONEY)
				{
					gObj[lpMsg->iIndex].Money = MAX_MONEY;
				}
				GCMoneySend(lpMsg->iIndex,gObj[lpMsg->iIndex].Money);

				LogAdd(LOG_BLACK,"[CastleSiege] [0x80][0x08] GS_DGAnsCastleMoneyChange() - Withdraw Request OK [%s][%s] (ReqMoney:%d, TotMoney:%I64d)",gObj[lpMsg->iIndex].Account,
				gObj[lpMsg->iIndex].Name,lpMsg->iMoneyChanged,lpMsg->i64CastleMoney);
			}
		}
		gCastleSiege.SetCastleMoney(lpMsg->i64CastleMoney);
	}
	
	GCAnsMoneyDrawOut(lpMsg->iIndex,lpMsg->iResult,lpMsg->i64CastleMoney);
#else
	return;
#endif
}

void GS_DGAnsSiegeDateChange(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_SDEDCHANGE* lpMsg = (CSP_ANS_SDEDCHANGE*)lpRecv;

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x80][0x09] GS_DGAnsSiegeDateChange() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}
#else
	return;
#endif
}

void GS_DGAnsGuildMarkRegInfo(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_GUILDREGINFO* lpMsg = (CSP_ANS_GUILDREGINFO*)lpRecv;

	if(lpMsg == NULL)
		return;
	
	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x80][0x0A] GS_DGAnsGuildMarkRegInfo() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}

	GCAnsGuildRegInfo(lpMsg->iIndex,lpMsg->iResult,lpMsg);
#else
	return;
#endif

}

void GS_DGAnsSiegeEndedChange(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_SIEGEENDCHANGE* lpMsg = (CSP_ANS_SIEGEENDCHANGE*)lpRecv;

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x80][0x0B] GS_DGAnsSiegeEndedChange() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}
#else
	return;
#endif

}

void GS_DGAnsCastleOwnerChange(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_CASTLEOWNERCHANGE* lpMsg = (CSP_ANS_CASTLEOWNERCHANGE*)lpRecv;

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x80][0x0C] GS_DGAnsCastleOwnerChange() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}
#else
	return;
#endif
}

void GS_DGAnsRegAttackGuild(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_REGATTACKGUILD* lpMsg = (CSP_ANS_REGATTACKGUILD*)lpRecv;

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x80][0x0D] GS_DGAnsRegAttackGuild() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}

	GCAnsRegCastleSiege(lpMsg->iIndex,lpMsg->iResult,lpMsg->szEnemyGuildName);
#else
	return;
#endif
}

void GS_DGAnsRestartCastleState(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_CASTLESIEGEEND* lpMsg = (CSP_ANS_CASTLESIEGEEND*)lpRecv;

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x80][0x0E] GS_DGAnsRestartCastleState() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}
	else
	{
		gCastleSiege.ResetCastleCycle();
	}
#else
	return;
#endif
}

void GS_DGAnsMapSvrMsgMultiCast(LPBYTE lpRecv)
{
	CSP_ANS_MAPSVRMULTICAST * lpMsg = (CSP_ANS_MAPSVRMULTICAST *)lpRecv;

	if ( lpMsg == NULL )
	{
		return;
	}

	if ( lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup() )
	{
		LogAdd(LOG_RED, "[CastleSiege] PACKET-ERROR [0x80][0x0F] GS_DGAnsMapSvrMsgMultiCast() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()");
		return;
	}

	char szNotice[144] = {0};
	memcpy(szNotice, lpMsg->szMsgText, 128);

	if ( strlen(szNotice) > 1 )
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"%s",szNotice);
	}

}

void GS_DGAnsRegGuildMark(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_GUILDREGMARK* lpMsg = (CSP_ANS_GUILDREGMARK*)lpRecv;
		
	if ( lpMsg == NULL )
	{
		return;
	}

	if ( lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup() )
	{
		LogAdd(LOG_RED, "[CastleSiege] PACKET-ERROR [0x80][0x10] GS_DGAnsRegGuildMark() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()");
		return;
	}
	else
	{
		GCAnsRegGuildMark(lpMsg->iIndex,lpMsg->iResult,lpMsg);
	}
#else
	return;
#endif
}

void GS_DGAnsGuildMarkReset(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_GUILDRESETMARK* lpMsg = (CSP_ANS_GUILDRESETMARK*)lpRecv;
	
	if ( lpMsg == NULL )
	{
		return;
	}

	if ( lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup() )
	{
		LogAdd(LOG_RED, "[CastleSiege] PACKET-ERROR [0x80][0x11] GS_DGAnsGuildMarkReset() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()");
		return;
	}
#else
	return;
#endif
}

void GS_DGAnsGuildSetGiveUp(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_GUILDSETGIVEUP* lpMsg = (CSP_ANS_GUILDSETGIVEUP*)lpRecv;

	if ( lpMsg == NULL )
	{
		return;
	}

	if ( lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup() )
	{
		LogAdd(LOG_RED, "[CastleSiege] PACKET-ERROR [0x80][0x12] GS_DGAnsGuildSetGiveUp() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()");
		return;
	}
	
	GCAnsGiveUpCastleSiege(lpMsg->iIndex,lpMsg->iResult,lpMsg->bIsGiveUp,lpMsg->iRegMarkCount,lpMsg->szGuildName);
#else
	return;
#endif
}

void GS_DGAnsNpcRemove(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_NPCREMOVE* lpMsg = (CSP_ANS_NPCREMOVE*)lpRecv;
	
	if ( lpMsg == NULL )
	{
		return;
	}

	if ( lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup() )
	{
		LogAdd(LOG_RED, "[CastleSiege] PACKET-ERROR [0x80][0x16] GS_DGAnsNpcRemove() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()");
		return;
	}
#else
	return;
#endif
}

void GS_DGAnsCastleStateSync(LPBYTE lpRecv) //Identical gs-cs 56
{
	CSP_ANS_CASTLESTATESYNC * lpMsg = (CSP_ANS_CASTLESTATESYNC *)lpRecv;

	if ( lpMsg == NULL )
	{
		return;
	}

	if ( lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup() )
	{
		return;
	}

	gCastleSiegeSync.SetCastleState(lpMsg->iCastleState); 
	gCastleSiegeSync.SetTaxRateChaos(lpMsg->iTaxRateChaos);
	gCastleSiegeSync.SetTaxRateStore(lpMsg->iTaxRateStore);
	gCastleSiegeSync.SetTaxHuntZone(lpMsg->iTaxHuntZone);
	gCastleSiegeSync.SetCastleOwnerGuild(lpMsg->szOwnerGuildName);
}

void GS_DGAnsCastleTributeMoney(LPBYTE lpRecv)
{
	CSP_ANS_CASTLETRIBUTEMONEY * lpMsg = (CSP_ANS_CASTLETRIBUTEMONEY *)lpRecv;

	if ( lpMsg == NULL )
	{
		return;
	}

	if ( lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup() )
	{
		LogAdd(LOG_RED, "[CastleSiege] PACKET-ERROR [0x80][0x18] GS_DGAnsCastleTributeMoney() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()");
		return;
	}
	
	LogAdd(LOG_BLACK,"[CastleSiege] [0x80][0x18] GS_DGAnsCastleTributeMoney() - Money Tribute OK (%d)",gCastleSiegeSync.GetTributeMoney());
	gCastleSiegeSync.ResetTributeMoney();
	
}

void GS_DGAnsResetCastleTaxInfo(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_RESETCASTLETAXINFO* lpMsg = (CSP_ANS_RESETCASTLETAXINFO*)lpRecv;

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x80][0x19] GS_DGAnsResetCastleTaxInfo() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}
	else
	{
		if(lpMsg->iResult == 1)
		{
			gCastleSiege.ResetCastleTaxInfo();
		}
	}
#else
	return;
#endif
}

void GS_DGAnsResetSiegeGuildInfo(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_RESETSIEGEGUILDINFO* lpMsg = (CSP_ANS_RESETSIEGEGUILDINFO*)lpRecv;

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x80][0x1A] GS_DGAnsResetSiegeGuildInfo() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}
#else
	return;
#endif
}

void GS_DGAnsResetRegSiegeInfo(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_RESETREGSIEGEINFO* lpMsg = (CSP_ANS_RESETREGSIEGEINFO*)lpRecv;

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x80][0x1B] GS_DGAnsResetRegSiegeInfo() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}
#else
	return;
#endif
}

void GS_DGAnsCastleInitData(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_CSINITDATA* lpMsg = (CSP_ANS_CSINITDATA*)lpRecv;
	CSP_CSINITDATA* lpMsgBody = (CSP_CSINITDATA*)(lpRecv + sizeof(CSP_ANS_CSINITDATA));

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x81] GS_DGAnsCastleInitData() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}

	if(gCastleSiege.GetDataLoadState() != 2)
	{
		LogAdd(LOG_RED,"[CastleSiege] CASTLE SIEGE DATA SETTING FAILED [0x81] - m_iCastleDataLoadState != CASTLESIEGE_DATALOAD_2 (%d)",gCastleSiege.GetDataLoadState()) ;
		return;
	}

	gCastleSiege.SetDataLoadState(3);

	if(lpMsg->iResult == FALSE)
	{
		LogAdd(LOG_RED,"[CastleSiege] CASTLE SIEGE DATA SETTING FAILED [0x81] - lpMsg->iResult == 0");
		return;
	}
	
	BOOL bRET_VAL = FALSE;

	bRET_VAL = gCastleSiege.SetCastleInitData(lpMsg);

	if(bRET_VAL == FALSE)
	{
		LogAdd(LOG_RED,"[CastleSiege] CASTLE SIEGE DATA SETTING FAILED [0x81] - g_CastleSiege.SetCastleInitData() == FALSE");
		return;
	}

	bRET_VAL = gCastleSiege.SetCastleNpcData(lpMsgBody,lpMsg->iCount);
	
	if(bRET_VAL == FALSE)
	{
		LogAdd(LOG_RED,"[CastleSiege] CASTLE SIEGE DATA SETTING FAILED [0x81] - g_CastleSiege.SetCastleNpcData() == FALSE");
		return;
	}

	gCastleSiege.SetDbDataLoadOK(TRUE);

	gCastleSiege.SetDataLoadState(4);

	if(lpMsg->iFirstCreate == 1)
	{
		gCastleSiege.FirstCreateDbNPC();
	}

	gCastleSiege.Init();

#else
	return;
#endif
}

void GS_DGAnsCastleNpcInfo(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_NPCDATA* lpMsg = (CSP_ANS_NPCDATA*)lpRecv;
	CSP_NPCDATA* lpMsgBody = (CSP_NPCDATA*)(lpRecv+sizeof(CSP_ANS_NPCDATA));

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x82] GS_DGAnsCastleNpcInfo() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}
#else
	return;
#endif
}

void GS_DGAnsAllGuildMarkRegInfo(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_ALLGUILDREGINFO* lpMsg = (CSP_ANS_ALLGUILDREGINFO*)lpRecv;
	CSP_GUILDREGINFO* lpMsgBody = (CSP_GUILDREGINFO*)(lpRecv+sizeof(CSP_ANS_ALLGUILDREGINFO));
	char cBUFFER[1668];
	PMSG_ANS_CSREGGUILDLIST* lpMsgSend;
	PMSG_CSREGGUILDLIST* lpMsgSendBody;

	if(lpMsg == NULL)
		return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x83] GS_DGAnsAllGuildMarkRegInfo() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}

	if(!gObjIsConnected(lpMsg->iIndex))
		return;

	lpMsgSend = (PMSG_ANS_CSREGGUILDLIST*)cBUFFER;
	lpMsgSendBody = (PMSG_CSREGGUILDLIST*)&cBUFFER[12];

	lpMsgSend->iCount = 0;
	lpMsgSend->btResult = lpMsg->iResult;
	
	if(lpMsg->iResult == TRUE)
	{
		lpMsgSend->iCount = lpMsg->iCount;

		for(int i=0;i< lpMsg->iCount;i++)
		{
			lpMsgSendBody[i].btSeqNum = lpMsgBody[i].btRegRank;
			lpMsgSendBody[i].btIsGiveUp = lpMsgBody[i].bIsGiveUp;
			lpMsgSendBody[i].btRegMarks1 = SET_NUMBERHB(SET_NUMBERHW(lpMsgBody[i].iRegMarkCount));
			lpMsgSendBody[i].btRegMarks2 = SET_NUMBERLB(SET_NUMBERHW(lpMsgBody[i].iRegMarkCount));
			lpMsgSendBody[i].btRegMarks3 = SET_NUMBERHB(SET_NUMBERLW(lpMsgBody[i].iRegMarkCount));
			lpMsgSendBody[i].btRegMarks4 = SET_NUMBERLB(SET_NUMBERLW(lpMsgBody[i].iRegMarkCount));
			memcpy(&lpMsgSendBody[i].szGuildName,lpMsgBody[i].szGuildName,sizeof(lpMsgSendBody[i].szGuildName));
		}
	}
	
	lpMsgSend->h.set(0xB4,(lpMsgSend->iCount*sizeof(PMSG_CSREGGUILDLIST)+sizeof(PMSG_ANS_CSREGGUILDLIST)));

	DataSend(lpMsg->iIndex,(LPBYTE)lpMsgSend,(lpMsgSend->iCount*sizeof(PMSG_CSREGGUILDLIST)+sizeof(PMSG_ANS_CSREGGUILDLIST)));
#else
	return;
#endif
}

void GS_DGAnsFirstCreateNPC(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_NPCSAVEDATA* lpMsg = (CSP_ANS_NPCSAVEDATA*)lpRecv;

	if(lpMsg == NULL)
	return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x84] GS_DGAnsFirstCreateNPC() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}

	if(lpMsg->iResult == TRUE)
	{
		if(gCastleSiege.GetDbNpcCreated() == FALSE)
		{
			gCastleSiege.CreateDbNPC();
			gCastleSiege.SetDbNpcCreated(1);
		}
	}
#else
	return;
#endif
}

void GS_DGAnsCalcREgGuildList(LPBYTE lpRecv)
{
#if (GAMESERVER_TYPE==1)
	CSP_ANS_CALCREGGUILDLIST* lpMsg = (CSP_ANS_CALCREGGUILDLIST*)lpRecv;
	CSP_CALCREGGUILDLIST* lpMsgBody = (CSP_CALCREGGUILDLIST*)(lpRecv+sizeof(CSP_ANS_CALCREGGUILDLIST));
	
	if(lpMsg == NULL)
	return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x85] GS_DGAnsCalcREgGuildList() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}

	if(lpMsg->iResult == TRUE)
	{
		gCastleSiege.SetCalcRegGuildList(lpMsgBody,lpMsg->iCount);
		return;
	}

	LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x85] GS_DGAnsCalcREgGuildList() - lpMsg->iResult != 1 (%d)",lpMsg->iResult) ;
#else
	return;
#endif
}

void GS_DGAnsCsGulidUnionInfo(LPBYTE lpRecv)
{
#if (GAMESERVER_TYPE==1)
	CSP_ANS_CSGUILDUNIONINFO* lpMsg = (CSP_ANS_CSGUILDUNIONINFO*)lpRecv;
	CSP_CSGUILDUNIONINFO* lpMsgBody = (CSP_CSGUILDUNIONINFO*)(lpRecv+sizeof(CSP_ANS_CSGUILDUNIONINFO));
	
	if(lpMsg == NULL)
	return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x86] GS_DGAnsCsGulidUnionInfo() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}

	if(lpMsg->iResult == TRUE)
	{
		gCastleSiege.MakeCsTotalGuildInfo(lpMsgBody,lpMsg->iCount);
		return;
	}
#else
	return;
#endif
}

void GS_DGAnsCsSaveTotalGuildInfo(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_CSSAVETOTALGUILDINFO* lpMsg = (CSP_ANS_CSSAVETOTALGUILDINFO*)lpRecv;
	
	if(lpMsg == NULL)
	return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x87] GS_DGAnsCsSaveTotalGuildInfo() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}

	if(lpMsg->iResult == TRUE)
	{
		gCastleSiege.SetIsSiegeGuildList(TRUE);
		return;
	}
	gCastleSiege.SetIsSiegeGuildList(FALSE);
#else
	return;
#endif
}

void GS_DGAnsCsLoadTotalGuildInfo(LPBYTE lpRecv)
{
#if (GAMESERVER_TYPE==1)
	CSP_ANS_CSLOADTOTALGUILDINFO* lpMsg = (CSP_ANS_CSLOADTOTALGUILDINFO*)lpRecv;
	CSP_CSLOADTOTALGUILDINFO* lpMsgBody = (CSP_CSLOADTOTALGUILDINFO*)(lpRecv+sizeof(CSP_ANS_CSLOADTOTALGUILDINFO));

	if(lpMsg == NULL)
	return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x88] GS_DGAnsCsLoadTotalGuildInfo() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}

	if(lpMsg->iResult == TRUE)
	{
		gCastleSiege.SetCsTotalGuildInfo(lpMsgBody,lpMsg->iCount);
		return;
	}
#else
	return;
#endif
}

void GS_DGAnsCastleNpcUpdate(LPBYTE lpRecv)
{
#if(GAMESERVER_TYPE==1)
	CSP_ANS_NPCUPDATEDATA* lpMsg = (CSP_ANS_NPCUPDATEDATA*)lpRecv;

	if(lpMsg == NULL)
	return;

	if(lpMsg->wMapSvrNum != gMapServerManager.GetMapServerGroup())
	{
		LogAdd(LOG_RED,"[CastleSiege] PACKET-ERROR [0x89] GS_DGAnsCastleNpcUpdate() - lpMsg->wMapSvrNum != g_MapServerManager.GetMapServerGroup()") ;
		return;
	}

	char* szResult;

	if(lpMsg->iResult != FALSE)
	{
		szResult = "SUCCEED";
	}
	else
	{
		szResult = "FAIL";
	}

	LogAdd(LOG_BLACK,"[CastleSiege] [0x89] GS_DGAnsCastleNpcUpdate() - Npc Data Update Result : '%s'",szResult);
#else
	return;
#endif
}

//marry system
void GDMarryInfoSaveSend(int aIndex,char* marryname,char* mode) // OK
{
    LPOBJ lpObj = &gObj[aIndex];


    SDHP_MARRY_INFO_SAVE_SEND pMsg;


    pMsg.header.set(0xF0,sizeof(pMsg));


    pMsg.index = aIndex;


    memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));


    memcpy(pMsg.marryname,marryname,sizeof(pMsg.marryname));


    memcpy(pMsg.mode,mode,sizeof(pMsg.mode));


    //pMsg.name = name;


    //pMsg.marryname = marryname;


    gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

//Set Coin System
void GDSetCoinSend(int aIndex, int value1, int value2, int value3, char * LogName) // OK
{
    LPOBJ lpObj = &gObj[aIndex];

    SDHP_SETCOIN_SEND pMsg;

    pMsg.header.set(0xF3,sizeof(pMsg));

    pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

    memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	//pMsg.value1 = (value1 > 0) ? value1 : 0;
	//pMsg.value2 = (value2 > 0) ? value2 : 0;
	//pMsg.value3 = (value3 > 0) ? value3 : 0;

	pMsg.value1 = (!value1) ? 0 : value1;
	pMsg.value2 = (!value2) ? 0 : value2;
	pMsg.value3 = (!value3) ? 0 : value3;

    gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

		if (value1 > 0)
		{
			lpObj->Coin1 += value1;
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(730),value1);
		}
		if (value2 > 0)
		{
			lpObj->Coin2 += value2;
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(731),value2);
		}
		if (value3 > 0)
		{
			lpObj->Coin3 += value3;
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(732),value3);
		}

		if (value1 < 0)
		{
			lpObj->Coin1 += value1;
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(760),value1);
		}
		if (value2 < 0)
		{
			lpObj->Coin2 += value2;
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(761),value2);
		}
		if (value3 < 0)
		{
			lpObj->Coin3 += value3;
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(762),value3);
		}

		gCashShop.CGCashShopPointRecv(lpObj->Index);

		LogAdd(LOG_ALERT,"[%s][%s] WZ_SetCoin(%s) Val1: %d, Val2: %d, Val3: %d",lpObj->Account,lpObj->Name,LogName,value1,value2,value3);
}

void GDKillSystemSend(int aIndex,int bIndex) // OK
{
	return;
	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0 || gObjIsAccountValid(bIndex,gObj[bIndex].Account) == 0)
	{
		return;
	}

	SDHP_KILL_SYSTEM_SEND pMsg;

	pMsg.header.set(0xF3,sizeof(pMsg));

	memcpy(pMsg.killer,gObj[aIndex].Name,sizeof(pMsg.killer));

	memcpy(pMsg.dead,gObj[bIndex].Name,sizeof(pMsg.dead));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);


}

void GDRankingTvTEventSaveSend(int aIndex, DWORD KillCount, DWORD DeathCount)
{
	LPOBJ lpObj = &gObj[aIndex];

	SDHP_RANKING_TVT_EVENT_SAVE_SEND pMsg;

	pMsg.header.set(0x56,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.killcount = KillCount;

	pMsg.deathcount = DeathCount;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}
