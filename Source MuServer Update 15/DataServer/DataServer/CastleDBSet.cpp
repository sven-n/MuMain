// CastleDBSet.cpp: implementation of the CCastleDBSet class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CastleDBSet.h"
#include "QueryManager.h"
#include "Util.h"

CCastleDBSet gCastleDBSet;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCastleDBSet::CCastleDBSet()
{

}

CCastleDBSet::~CCastleDBSet()
{

}

BOOL CCastleDBSet::DSDB_QueryCastleTotalInfo(int iMapSvrGroup, int iCastleEventCycle, CASTLE_DATA* lpCastleData)
{
	if(lpCastleData == NULL)
	{
		return TRUE;
	}

	if (gQueryManager.ExecQuery("EXEC WZ_CS_GetCastleTotalInfo %d, %d", iMapSvrGroup, iCastleEventCycle) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	short sqlRet = gQueryManager.Fetch();

	if(sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return TRUE;
	}

	lpCastleData->wStartYear = gQueryManager.GetAsInteger("SYEAR");
	lpCastleData->btStartMonth = gQueryManager.GetAsInteger("SMONTH");
	lpCastleData->btStartDay = gQueryManager.GetAsInteger("SDAY");
	lpCastleData->wEndYear = gQueryManager.GetAsInteger("EYEAR");
	lpCastleData->btEndMonth = gQueryManager.GetAsInteger("EMONTH");
	lpCastleData->btEndDay = gQueryManager.GetAsInteger("EDAY");
	lpCastleData->btIsSiegeGuildList = gQueryManager.GetAsInteger("SIEGE_GUILDLIST_SETTED");
	lpCastleData->btIsSiegeEnded = gQueryManager.GetAsInteger("SIEGE_ENDED");
	lpCastleData->btIsCastleOccupied = gQueryManager.GetAsInteger("CASTLE_OCCUPY");
	lpCastleData->i64CastleMoney = gQueryManager.GetAsInteger64("MONEY");
	lpCastleData->iTaxRateChaos = gQueryManager.GetAsInteger("TAX_RATE_CHAOS");
	lpCastleData->iTaxRateStore = gQueryManager.GetAsInteger("TAX_RATE_STORE");
	lpCastleData->iTaxHuntZone = gQueryManager.GetAsInteger("TAX_HUNT_ZONE");
	lpCastleData->iFirstCreate = gQueryManager.GetAsInteger("FIRST_CREATE");
	gQueryManager.GetAsString("OWNER_GUILD", lpCastleData->szCastleOwnGuild,sizeof(lpCastleData->szCastleOwnGuild));
	gQueryManager.Close();
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryOwnerGuildMaster(int iMapSvrGroup, CSP_ANS_OWNERGUILDMASTER* lpOwnerGuildMaster)
{
	if (lpOwnerGuildMaster == NULL)
	{
		return TRUE;
	}

	if (gQueryManager.ExecQuery("EXEC WZ_CS_GetOwnerGuildMaster %d", iMapSvrGroup) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	short sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return NULL;		
	}
	
	char szOwnerGuild[8+8] = {'\0'};
	char szOwnerGuildMaster[8+8] = {'\0'};
	
	lpOwnerGuildMaster->iResult = gQueryManager.GetAsInteger("QueryResult");
	
	gQueryManager.GetAsString("OwnerGuild", szOwnerGuild,sizeof(szOwnerGuild));
	gQueryManager.GetAsString("OwnerGuildMaster", szOwnerGuildMaster,sizeof(szOwnerGuildMaster));
	
	memcpy(lpOwnerGuildMaster->szCastleOwnGuild, szOwnerGuild, 8);
	memcpy(lpOwnerGuildMaster->szCastleOwnGuildMaster, szOwnerGuildMaster, 10);
	
	gQueryManager.Close();
	return FALSE;	
}

int CCastleDBSet::DSDB_QueryCastleNpcBuy(int iMapSvrGroup, CSP_REQ_NPCBUY* lpNpcBuy, int* lpiResult)
{
	if (lpiResult == NULL || lpNpcBuy == NULL)
	{
		return TRUE;
	}

	if (gQueryManager.ExecQuery("EXEC WZ_CS_ReqNpcBuy %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",iMapSvrGroup, lpNpcBuy->iNpcNumber, lpNpcBuy->iNpcIndex, lpNpcBuy->iNpcDfLevel,lpNpcBuy->iNpcRgLevel, lpNpcBuy->iNpcMaxHp, lpNpcBuy->iNpcHp, lpNpcBuy->btNpcX, lpNpcBuy->btNpcY, lpNpcBuy->btNpcDIR) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	short sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return TRUE;		
	}
	
	*lpiResult = gQueryManager.GetAsInteger("QueryResult");
	
	gQueryManager.Close();
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryCastleNpcRepair(int iMapSvrGroup, CSP_REQ_NPCREPAIR* lpNpcRepair, CSP_ANS_NPCREPAIR* lpNpcRepairResult, int* lpiResult)
{
	if (lpNpcRepair == NULL || lpNpcRepairResult == NULL)
	{
		return TRUE;
	}

	if (gQueryManager.ExecQuery("EXEC WZ_CS_ReqNpcRepair %d, %d, %d", iMapSvrGroup, lpNpcRepair->iNpcNumber, lpNpcRepair->iNpcIndex) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	short sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return TRUE;		
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");
	
	lpNpcRepairResult->iNpcHp = gQueryManager.GetAsInteger("NPC_HP");
	lpNpcRepairResult->iNpcMaxHp = gQueryManager.GetAsInteger("NPC_MAXHP");
	
	gQueryManager.Close();
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryCastleNpcUpgrade(int iMapSvrGroup, CSP_REQ_NPCUPGRADE* lpNpcUpgrade)
{
	if (lpNpcUpgrade == NULL)
	{
		return TRUE;
	}

	if (gQueryManager.ExecQuery("EXEC WZ_CS_ReqNpcUpgrade %d, %d, %d, %d, %d", iMapSvrGroup, lpNpcUpgrade->iNpcNumber, lpNpcUpgrade->iNpcIndex, lpNpcUpgrade->iNpcUpType, lpNpcUpgrade->iNpcUpValue) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	short sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return TRUE;		
	}

	int iResult = gQueryManager.GetAsInteger("QueryResult");

	if (iResult == FALSE)
	{
		gQueryManager.Close();
		return TRUE;		
	}

	gQueryManager.Close();
	
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryTaxInfo(int iMapSvrGroup, CSP_ANS_TAXINFO* lpTaxInfo)
{
	if (lpTaxInfo == NULL)
	{
		return TRUE;
	}
	
	if (gQueryManager.ExecQuery("EXEC WZ_CS_GetCastleTaxInfo %d", iMapSvrGroup) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	short sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return TRUE;
	}

	lpTaxInfo->i64CastleMoney = gQueryManager.GetAsInteger64("MONEY");
	lpTaxInfo->iTaxRateChaos = gQueryManager.GetAsInteger("TAX_RATE_CHAOS");
	lpTaxInfo->iTaxRateStore = gQueryManager.GetAsInteger("TAX_RATE_STORE");
	lpTaxInfo->iTaxHuntZone = gQueryManager.GetAsInteger("TAX_HUNT_ZONE");
	gQueryManager.Close();
	
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryTaxRateChange(int iMapSvrGroup, int iTaxType, int iTaxRate, CSP_ANS_TAXRATECHANGE* lpTaxRateChange, int* lpiResult)
{
	if (lpiResult == NULL || lpTaxRateChange == NULL)
	{
		return TRUE;
	}
	
	if (gQueryManager.ExecQuery("EXEC WZ_CS_ModifyTaxRate %d, %d, %d", iMapSvrGroup, iTaxType, iTaxRate) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	short sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	*lpiResult = gQueryManager.GetAsInteger("QueryResult");
	
	lpTaxRateChange->iTaxKind = gQueryManager.GetAsInteger("TaxKind");
	lpTaxRateChange->iTaxRate = gQueryManager.GetAsInteger("TaxRate");

	gQueryManager.Close();
	
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryCastleMoneyChange(int iMapSvrGroup, int iMoneyChange, __int64* i64ResultMoney, int* lpiResult)
{
	if (lpiResult == NULL || i64ResultMoney == NULL)
	{
		return TRUE;
	}

	if (gQueryManager.ExecQuery("EXEC WZ_CS_ModifyMoney %d, %d", iMapSvrGroup, iMoneyChange) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	short sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return TRUE;		
	}
	
	*lpiResult = gQueryManager.GetAsInteger("QueryResult");
	*i64ResultMoney = gQueryManager.GetAsInteger64("MONEY");
	
	gQueryManager.Close();
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QuerySiegeDateChange(int iMapSvrGroup, CSP_REQ_SDEDCHANGE* lpSdEdChange, int* lpiResult)
{
	if (lpSdEdChange == NULL)
	{
		return TRUE;
	}

	if (gQueryManager.ExecQuery("EXEC WZ_CS_ModifyCastleSchedule %d, '%d-%d-%d 00:00:00', '%d-%d-%d 00:00:00'",iMapSvrGroup, lpSdEdChange->wStartYear, lpSdEdChange->btStartMonth, lpSdEdChange->btStartDay,lpSdEdChange->wEndYear, lpSdEdChange->btEndMonth, lpSdEdChange->btEndDay) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	short sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return TRUE;		
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");
	gQueryManager.Close();
	
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryGuildMarkRegInfo(int iMapSvrGroup, char* lpszGuildName, CSP_ANS_GUILDREGINFO* lpGuildRegInfo, int* lpiResult)
{
	if (lpszGuildName  == NULL || lpGuildRegInfo == NULL)
	{
		return TRUE;
	}

	if (CheckTextSyntax(lpszGuildName,strlen(lpszGuildName)) == FALSE)
	{
		return TRUE;
	}

	if (gQueryManager.ExecQuery("EXEC WZ_CS_GetGuildMarkRegInfo %d, '%s'", iMapSvrGroup, lpszGuildName) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	short sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return TRUE;
	}

	memcpy(lpGuildRegInfo->szGuildName, lpszGuildName, 8);
	
	if (sqlRet == SQL_NO_DATA )
	{
		lpGuildRegInfo->iRegMarkCount = 0;
		*lpiResult = 2;
	}
	else
	{
		*lpiResult = 1;
		lpGuildRegInfo->iRegMarkCount = gQueryManager.GetAsInteger("REG_MARKS");
		lpGuildRegInfo->btRegRank = gQueryManager.GetAsInteger("SEQ_NUM");
		lpGuildRegInfo->bIsGiveUp = (gQueryManager.GetAsInteger("IS_GIVEUP") > 0)?true:false;	
	}

	gQueryManager.Close();
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QuerySiegeEndedChange(int iMapSvrGroup, int bIsCastleSiegeEnded, int* lpiResult)
{
	if (lpiResult == NULL)
	{
		return TRUE;
	}

	if (gQueryManager.ExecQuery("EXEC WZ_CS_ModifySiegeEnd %d, %d", iMapSvrGroup, bIsCastleSiegeEnded) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	short sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return TRUE;		
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");
	
	gQueryManager.Close();
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryCastleOwnerChange(int iMapSvrGroup, CSP_REQ_CASTLEOWNERCHANGE* lpCastleOwnerInfo, CSP_ANS_CASTLEOWNERCHANGE* lpCastleOwnerInfoResult, int* lpiResult)
{
	if (lpCastleOwnerInfo == NULL || lpCastleOwnerInfoResult == NULL || lpiResult == NULL)
	{
		return TRUE;
	}

	char szGuildName[9] = {'\0'};
	memcpy(szGuildName, lpCastleOwnerInfo->szOwnerGuildName, 8);
	
	if (CheckTextSyntax(szGuildName,strlen(szGuildName)) == FALSE)
	{
		return TRUE;
	}

	lpCastleOwnerInfoResult->bIsCastleOccupied = lpCastleOwnerInfo->bIsCastleOccupied;
	memcpy(lpCastleOwnerInfoResult->szOwnerGuildName, lpCastleOwnerInfo->szOwnerGuildName, 8);

	if (gQueryManager.ExecQuery("EXEC WZ_CS_ModifyCastleOwnerInfo %d, %d, '%s'", iMapSvrGroup, lpCastleOwnerInfo->bIsCastleOccupied,szGuildName) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	short sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return TRUE;		
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");
	
	gQueryManager.Close();
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryRegAttackGuild(int iMapSvrGroup, CSP_REQ_REGATTACKGUILD* lpRegAttackGuild, CSP_ANS_REGATTACKGUILD* lpRegAttackGuildResult, int* lpiResult)
{
	if (lpRegAttackGuild == NULL || lpRegAttackGuildResult == NULL || lpiResult == NULL)
	{
		return TRUE;
	}

	char szGuildName[9] = {'\0'};
	memcpy(szGuildName, lpRegAttackGuild->szEnemyGuildName, 8);
	
	if (CheckTextSyntax(szGuildName,strlen(szGuildName)) == FALSE)
	{
		return TRUE;
	}

	memcpy(lpRegAttackGuild->szEnemyGuildName, lpRegAttackGuildResult->szEnemyGuildName, 8);

	if (gQueryManager.ExecQuery("EXEC WZ_CS_ReqRegAttackGuild %d, '%s'", iMapSvrGroup, szGuildName) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	short sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return TRUE;		
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");
	
	gQueryManager.Close();
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryRestartCastleState(int iMapSvrGroup, CSP_REQ_CASTLESIEGEEND* lpCastleSiegeEnd, int* lpiResult)
{
	if (lpCastleSiegeEnd == NULL)
	{
		return TRUE;
	}

	if (gQueryManager.ExecQuery("EXEC WZ_CS_ResetCastleSiege %d", iMapSvrGroup) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	short sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return TRUE;
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");
	
	gQueryManager.Close();
	
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryGuildMarkRegMark(int iMapSvrGroup, char* lpszGuildName, CSP_ANS_GUILDREGMARK* lpGuildRegMark, int* lpiResult)
{
	if (lpszGuildName == NULL || lpGuildRegMark == NULL)
	{
		return TRUE;
	}

	if ( !CheckTextSyntax(lpszGuildName,strlen(lpszGuildName)) )
	{
		return TRUE;
	}

	if (gQueryManager.ExecQuery("EXEC WZ_CS_ReqRegGuildMark %d, '%s'",iMapSvrGroup,lpszGuildName) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	short sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return TRUE;		
	}
	
	memcpy(lpGuildRegMark->szGuildName, lpszGuildName, 8);
	
	*lpiResult = gQueryManager.GetAsInteger("QueryResult");
	
	lpGuildRegMark->iRegMarkCount = gQueryManager.GetAsInteger("REG_MARKS");
	
	gQueryManager.Close();
	
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryGuildMarkReset(int iMapSvrGroup, char* lpszGuildName, CSP_ANS_GUILDRESETMARK* lpGuildResetMark)
{
	if (lpszGuildName == NULL || lpGuildResetMark == NULL)
	{
		return TRUE;
	}

	if (CheckTextSyntax(lpszGuildName,strlen(lpszGuildName)) == FALSE)
	{
		return TRUE;
	}

	if (gQueryManager.ExecQuery("EXEC WZ_CS_ModifyGuildMarkReset %d, '%s'",iMapSvrGroup,lpszGuildName) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	short sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return TRUE;		
	}
	
	memcpy(lpGuildResetMark->szGuildName, lpszGuildName, 8);
	
	lpGuildResetMark->iResult = gQueryManager.GetAsInteger("QueryResult");
	lpGuildResetMark->iRegMarkCount = gQueryManager.GetAsInteger("DEL_MARKS");
	
	gQueryManager.Close();
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryGuildSetGiveUp(int iMapSvrGroup, char* lpszGuildName, int bIsGiveUp, CSP_ANS_GUILDSETGIVEUP* lpGuildSetGiveUp)
{
	if (lpszGuildName == NULL || lpGuildSetGiveUp == NULL)
	{
		return TRUE;
	}
	
	if (CheckTextSyntax(lpszGuildName,strlen(lpszGuildName)) == FALSE)
	{
		return TRUE;
	}

	if (gQueryManager.ExecQuery("EXEC WZ_CS_ModifyGuildGiveUp %d, '%s', %d",iMapSvrGroup,lpszGuildName,bIsGiveUp) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	short sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return TRUE;		
	}
	
	memcpy(lpGuildSetGiveUp->szGuildName, lpszGuildName, 8);
	
	lpGuildSetGiveUp->bIsGiveUp = bIsGiveUp;
	lpGuildSetGiveUp->iResult = gQueryManager.GetAsInteger("QueryResult");
	lpGuildSetGiveUp->iRegMarkCount = gQueryManager.GetAsInteger("DEL_MARKS");
	
	gQueryManager.Close();
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryCastleNpcRemove(int iMapSvrGroup, CSP_REQ_NPCREMOVE* lpNpcRemove, int* lpiResult)
{
	if (lpNpcRemove == NULL)
	{
		return TRUE;
	}

	if (gQueryManager.ExecQuery("EXEC WZ_CS_ReqNpcRemove %d, %d, %d",iMapSvrGroup,lpNpcRemove->iNpcNumber,lpNpcRemove->iNpcIndex) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	short sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return TRUE;		
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");
	
	gQueryManager.Close();
	return FALSE;
}

int CCastleDBSet::DSDB_QueryResetCastleTaxInfo(int iMapSvrGroup, int* lpiResult)
{
	if (gQueryManager.ExecQuery("EXEC WZ_CS_ResetCastleTaxInfo %d",iMapSvrGroup) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	short sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return TRUE;		
	}
	
	*lpiResult = gQueryManager.GetAsInteger("QueryResult");
	
	gQueryManager.Close();
	return FALSE;
}

int CCastleDBSet::DSDB_QueryResetSiegeGuildInfo(int iMapSvrGroup, int* lpiResult)
{
	if (gQueryManager.ExecQuery("EXEC WZ_CS_ResetSiegeGuildInfo %d",iMapSvrGroup) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	short sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return TRUE;
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");
	
	gQueryManager.Close();
	return FALSE;
}

int CCastleDBSet::DSDB_QueryResetRegSiegeInfo(int iMapSvrGroup, int* lpiResult)
{
	if (gQueryManager.ExecQuery("EXEC WZ_CS_ResetRegSiegeInfo %d",iMapSvrGroup) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	short sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return TRUE;		
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");
	
	gQueryManager.Close();
	return FALSE;
}

int CCastleDBSet::DSDB_QueryCastleNpcInfo(int iMapSvrGroup, CSP_NPCDATA* lpNpcData, int* lpiCount)
{
	if (lpNpcData == NULL || lpiCount == NULL)
	{
		return TRUE;
	}
	
	int iMAX_COUNT = *lpiCount;
	*lpiCount=0;
	
	if (gQueryManager.ExecQuery("EXEC WZ_CS_GetCastleNpcInfo %d",iMapSvrGroup) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	short sqlRet = gQueryManager.Fetch();
	
	while(sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
	{
		if (*lpiCount > iMAX_COUNT)
		{
			break;
		}

		lpNpcData[*lpiCount].iNpcNumber = gQueryManager.GetAsInteger("NPC_NUMBER");
		lpNpcData[*lpiCount].iNpcIndex = gQueryManager.GetAsInteger("NPC_INDEX");
		lpNpcData[*lpiCount].iNpcDfLevel = gQueryManager.GetAsInteger("NPC_DF_LEVEL");
		lpNpcData[*lpiCount].iNpcRgLevel = gQueryManager.GetAsInteger("NPC_RG_LEVEL");
		lpNpcData[*lpiCount].iNpcMaxHp = gQueryManager.GetAsInteger("NPC_MAXHP");
		lpNpcData[*lpiCount].iNpcHp = gQueryManager.GetAsInteger("NPC_HP");
		lpNpcData[*lpiCount].btNpcX = gQueryManager.GetAsInteger("NPC_X");
		lpNpcData[*lpiCount].btNpcY = gQueryManager.GetAsInteger("NPC_Y");
		lpNpcData[*lpiCount].btNpcDIR = gQueryManager.GetAsInteger("NPC_DIR");
		
		*lpiCount = *lpiCount + 1;
		
		sqlRet = gQueryManager.Fetch();
	}
	
	if (sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	gQueryManager.Close();
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryAllGuildMarkRegInfo(int iMapSvrGroup, CSP_GUILDREGINFO* lpGuildRegInfo, int* lpiCount)
{
	if (lpGuildRegInfo == NULL || lpiCount == NULL)
	{
		return TRUE;
	}

	int iMAX_COUNT = *lpiCount;
	
	if ( iMAX_COUNT > 100 )
	{
		iMAX_COUNT = 100;
	}
	
	*lpiCount=0;
	
	if (gQueryManager.ExecQuery("EXEC WZ_CS_GetAllGuildMarkRegInfo %d",iMapSvrGroup) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	short sqlRet = gQueryManager.Fetch();

	while(sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
	{
		if (*lpiCount > iMAX_COUNT)
		{
			break;
		}

		gQueryManager.GetAsString("REG_SIEGE_GUILD", lpGuildRegInfo[*lpiCount].szGuildName,sizeof(lpGuildRegInfo[*lpiCount].szGuildName));
		lpGuildRegInfo[*lpiCount].iRegMarkCount = gQueryManager.GetAsInteger("REG_MARKS");
		lpGuildRegInfo[*lpiCount].bIsGiveUp = (gQueryManager.GetAsInteger("IS_GIVEUP") > 0)?true:false;
		lpGuildRegInfo[*lpiCount].btRegRank = gQueryManager.GetAsInteger("SEQ_NUM");
		
		*lpiCount = *lpiCount + 1;
		
		sqlRet = gQueryManager.Fetch();
	}
	
	if (sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	gQueryManager.Close();
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryFirstCreateNPC(int iMapSvrGroup, CSP_REQ_NPCSAVEDATA* lpNpcSaveData)
{
	if (lpNpcSaveData == NULL)
	{
		return TRUE;
	}
	
	CSP_NPCSAVEDATA* lpMsgBody = (CSP_NPCSAVEDATA*)&lpNpcSaveData[1];
	
	if (gQueryManager.ExecQuery("DELETE MuCastle_NPC WHERE MAP_SVR_GROUP = %d",iMapSvrGroup) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	gQueryManager.Close();

	for (int iNPC_COUNT = 0; iNPC_COUNT < lpNpcSaveData->iCount; iNPC_COUNT++)
	{
		if (gQueryManager.ExecQuery("EXEC WZ_CS_ReqNpcBuy %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",iMapSvrGroup,lpMsgBody[iNPC_COUNT].iNpcNumber,lpMsgBody[iNPC_COUNT].iNpcIndex,lpMsgBody[iNPC_COUNT].iNpcDfLevel,lpMsgBody[iNPC_COUNT].iNpcRgLevel,lpMsgBody[iNPC_COUNT].iNpcMaxHp,lpMsgBody[iNPC_COUNT].iNpcHp,lpMsgBody[iNPC_COUNT].btNpcX,lpMsgBody[iNPC_COUNT].btNpcY,lpMsgBody[iNPC_COUNT].btNpcDIR) == FALSE)
		{
			gQueryManager.Close();
			return TRUE;
		}
		
		gQueryManager.Close();
	}

	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryCalcRegGuildList(int iMapSvrGroup, CSP_CALCREGGUILDLIST* lpCalcRegGuildList, int* lpiCount)
{
	if (lpCalcRegGuildList == NULL || lpiCount == NULL)
	{
		return TRUE;
	}
	
	int iMAX_COUNT = *lpiCount;
	
	if ( iMAX_COUNT > 100 )
	{
		iMAX_COUNT = 100;
	}
	
	*lpiCount=0;
	
	if (gQueryManager.ExecQuery("EXEC WZ_CS_GetCalcRegGuildList %d",iMapSvrGroup) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	short sqlRet = gQueryManager.Fetch();
	
	while(sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
	{
		if (*lpiCount > iMAX_COUNT)
		{
			break;
		}

		char szGuildName[8+8] = {'\0'};
		
		gQueryManager.GetAsString("REG_SIEGE_GUILD", szGuildName,sizeof(szGuildName));
		
		memcpy(&lpCalcRegGuildList[*lpiCount], szGuildName, 8);
		
		lpCalcRegGuildList[*lpiCount].iRegMarkCount = gQueryManager.GetAsInteger("REG_MARKS");
		lpCalcRegGuildList[*lpiCount].iGuildMemberCount = gQueryManager.GetAsInteger("GUILD_MEMBER");
		lpCalcRegGuildList[*lpiCount].iGuildMasterLevel = gQueryManager.GetAsInteger("GM_LEVEL");
		lpCalcRegGuildList[*lpiCount].iSeqNum = gQueryManager.GetAsInteger("SEQ_NUM");
		
		*lpiCount = *lpiCount + 1;
		
		sqlRet = gQueryManager.Fetch();
	}
	
	if (sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	gQueryManager.Close();
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryCsGuildUnionInfo(int iMapSvrGroup, char* lpszGuildName, int iCsGuildID, CSP_CSGUILDUNIONINFO* lpCsGuildUnionInfo, int* lpiCount)
{
	if (lpszGuildName == NULL || lpCsGuildUnionInfo == NULL || lpiCount == NULL)
	{
		return TRUE;
	}

	int iRET_COUNT = *lpiCount;
	
	if ( iRET_COUNT < 0 )
	{
		return TRUE;
	}
	
	if (gQueryManager.ExecQuery("EXEC WZ_CS_GetCsGuildUnionInfo '%s'",lpszGuildName) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	short sqlRet = gQueryManager.Fetch();

	while(sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
	{
		if (iRET_COUNT > 100)
		{
			break;
		}

		char szGuildName[8+8] = {'\0'};
		
		gQueryManager.GetAsString("GUILD_NAME", szGuildName,sizeof(szGuildName));
		
		memcpy(&lpCsGuildUnionInfo[iRET_COUNT], szGuildName, 8);
		lpCsGuildUnionInfo[iRET_COUNT].iCsGuildID = iCsGuildID;
		
		iRET_COUNT++;
		
		sqlRet = gQueryManager.Fetch();
	}
	
	if (sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	gQueryManager.Close();
	
	*lpiCount = iRET_COUNT;
	
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryCsClearTotalGuildInfo(int iMapSvrGroup)
{
	if (gQueryManager.ExecQuery("DELETE MuCastle_SIEGE_GUILDLIST WHERE MAP_SVR_GROUP = %d",iMapSvrGroup) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	gQueryManager.Close();
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryCsSaveTotalGuildInfo(int iMapSvrGroup, char* lpszGuildName, int iCsGuildID, int iCsGuildInvolved, int iCsGuildScore)
{
	if (lpszGuildName == NULL)
	{
		return TRUE;
	}

	if (gQueryManager.ExecQuery("EXEC WZ_CS_SetSiegeGuildInfo %d, '%s', %d, %d, %d",iMapSvrGroup, lpszGuildName, iCsGuildID, iCsGuildInvolved, iCsGuildScore) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	gQueryManager.Close();
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryCsSaveTotalGuildOK(int iMapSvrGroup, int* lpiResult)
{
	if (lpiResult == NULL)
	{
		return TRUE;
	}
	
	if (gQueryManager.ExecQuery("EXEC WZ_CS_SetSiegeGuildOK %d",iMapSvrGroup) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	*lpiResult = gQueryManager.GetAsInteger("QueryResult");
	
	gQueryManager.Close();
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryCsLoadTotalGuildInfo(int iMapSvrGroup, CSP_CSLOADTOTALGUILDINFO* lpLoadTotalGuildInfo, int* lpiCount)
{
	if (lpLoadTotalGuildInfo == NULL || lpiCount == NULL)
	{
		return TRUE;
	}
	
	int iMAX_COUNT = *lpiCount;
	
	if ( iMAX_COUNT > 100 )
	{
		iMAX_COUNT = 100;
	}
	
	*lpiCount=0;
	
	if (gQueryManager.ExecQuery("EXEC WZ_CS_GetSiegeGuildInfo %d",iMapSvrGroup) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}
	short sqlRet = gQueryManager.Fetch();

	while(sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
	{
		if (*lpiCount > iMAX_COUNT)
		{
			break;
		}
		
		char szGuildName[8+8] = {'\0'};
		
		gQueryManager.GetAsString("GUILD_NAME", szGuildName,sizeof(szGuildName));
		
		memcpy(&lpLoadTotalGuildInfo[*lpiCount], szGuildName, 8);
		
		lpLoadTotalGuildInfo[*lpiCount].iCsGuildID = gQueryManager.GetAsInteger("GUILD_ID");
		lpLoadTotalGuildInfo[*lpiCount].iGuildInvolved = gQueryManager.GetAsInteger("GUILD_INVOLVED");
		lpLoadTotalGuildInfo[*lpiCount].iGuildScore = gQueryManager.GetAsInteger("GUILD_SCORE");
		
		*lpiCount = *lpiCount + 1;
		
		sqlRet = gQueryManager.Fetch();
	}

	if (sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return TRUE;
	}
	
	gQueryManager.Close();
	return FALSE;
}

BOOL CCastleDBSet::DSDB_QueryCastleNpcUpdate(int iMapSvrGroup, CSP_REQ_NPCUPDATEDATA* lpNpcSaveData)
{
	if (lpNpcSaveData == NULL)
	{
		return TRUE;
	}

	CSP_NPCSAVEDATA* lpMsgBody = (CSP_NPCSAVEDATA*)&lpNpcSaveData[1];

	if (gQueryManager.ExecQuery("DELETE MuCastle_NPC WHERE MAP_SVR_GROUP = %d",iMapSvrGroup) == FALSE)
	{
		gQueryManager.Close();
		return TRUE;
	}

	gQueryManager.Close();
	
	for (int iNPC_COUNT = 0; iNPC_COUNT < lpNpcSaveData->iCount; iNPC_COUNT++)
	{
		if (gQueryManager.ExecQuery("EXEC WZ_CS_ReqNpcUpdate %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",iMapSvrGroup,lpMsgBody[iNPC_COUNT].iNpcNumber,lpMsgBody[iNPC_COUNT].iNpcIndex,lpMsgBody[iNPC_COUNT].iNpcDfLevel,lpMsgBody[iNPC_COUNT].iNpcRgLevel,lpMsgBody[iNPC_COUNT].iNpcMaxHp,lpMsgBody[iNPC_COUNT].iNpcHp,lpMsgBody[iNPC_COUNT].btNpcX,lpMsgBody[iNPC_COUNT].btNpcY,lpMsgBody[iNPC_COUNT].btNpcDIR) == FALSE)
		{
			gQueryManager.Close();
			return TRUE;
		}
		
		gQueryManager.Close();
	}

	return FALSE;
}
