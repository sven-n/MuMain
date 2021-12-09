// CastleSiege.cpp: implementation of the CCastleSiege class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CastleSiege.h"
#include "CastleSiegeSync.h"
#include "DSProtocol.h"
#include "EffectManager.h"
#include "GameMain.h"
#include "Guild.h"
#include "GuildClass.h"
#include "LifeStone.h"
#include "Map.h"
#include "MapServerManager.h"
#include "MemScript.h"
#include "Mercenary.h"
#include "Message.h"
#include "Monster.h"
#include "Move.h"
#include "Notice.h"
#include "ObjectManager.h"
#include "ServerInfo.h"
#include "Union.h"
#include "User.h"
#include "Util.h"
#include "Viewport.h"

#if(GAMESERVER_TYPE==1)

CCastleSiege gCastleSiege;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCastleSiege::CCastleSiege()
{
	InitializeCriticalSection(&this->m_critScheduleData);
	InitializeCriticalSection(&this->m_critNpcData);
	InitializeCriticalSection(&this->m_critCsTotalGuildInfo);
	InitializeCriticalSection(&this->m_critCsBasicGuildInfo);
	InitializeCriticalSection(&this->m_critCsMiniMap);

	this->Clear();
}

CCastleSiege::~CCastleSiege()
{
	DeleteCriticalSection(&this->m_critScheduleData);
	DeleteCriticalSection(&this->m_critNpcData);
	DeleteCriticalSection(&this->m_critCsTotalGuildInfo);
	DeleteCriticalSection(&this->m_critCsBasicGuildInfo);
	DeleteCriticalSection(&this->m_critCsMiniMap);
}

BYTE CCastleSiege::ScheduleStateCompFunc(_CS_SCHEDULE_DATA & A,_CS_SCHEDULE_DATA & B)
{
	if(A.m_iSTATE > B.m_iSTATE)
	{
		return false;
	}

	return true;
}

void CCastleSiege::Clear()
{
	this->m_bCastleSiegeEnable = TRUE;
	this->m_bFileDataLoadOK = FALSE;
	this->m_bDbDataLoadOK = FALSE;
	this->m_bDoRun = FALSE;

	this->m_iMapSvrGroup = -1;

	this->m_iCastleSiegeCycle = 14;
	this->m_iCastleSiegeState = -1;

	this->m_bFixCastleCycleStartDate = FALSE;

	this->m_i64CastleMoney = 0;

	memset(&this->m_tmFixCastleCycleStartDate,0,sizeof(this->m_tmFixCastleCycleStartDate));

	this->m_iFixCastleSpecificState = 0;
	this->m_bFixCastleStateStartDate = FALSE;

	memset(&this->m_tmFixCastleStateStartDate,0,sizeof(this->m_tmFixCastleStateStartDate));

	this->m_iCastleDataLoadState = 0;
	this->m_dwDbDataLoadTickCount = 0;
	this->m_iCS_REMAIN_MSEC = -1;
	this->m_iCS_TARGET_MSEC = -1;

	memset(&this->m_tmStateStartDate,0,sizeof(this->m_tmStateStartDate));
	memset(&this->m_tmLeftCastleSiegeDate,0,sizeof(this->m_tmLeftCastleSiegeDate));

	this->m_bCsBasicGuildInfoLoadOK = FALSE;
	this->m_bCsTotalGuildInfoLoadOK = FALSE;

	this->m_btIsSiegeGuildList = 0;
	this->m_btIsCastleOccupied = 0;

	memset(this->m_szCastleOwnerGuild,0,sizeof(this->m_szCastleOwnerGuild));

	this->m_bEnterHuntZone = FALSE;

	this->m_dwNPC_DBSAVE_TICK_COUNT = GetTickCount();
	this->m_dwCALC_LEFTSIEGE_TICK_COUNT = 0;
	this->m_dwEVENT_MSG_TICK_COUNT = 0;
	this->m_dwCHECK_GATE_ALIVE_COUNT = 0;
	this->m_dwCS_LEFTTIME_TICK_COUNT = 0;
	this->m_dwCS_STARTTIME_TICK_COUNT = 0;
	this->m_dwCS_JOINSIDE_REFRESH_TICK_COUNT = 0;
	this->m_dwCS_JOINSIDE_REFRESH_TOTAL_COUNT = 0;
	this->m_dwCS_MINIMAP_SEND_TICK_COUNT = 0;

	this->m_bCastleTowerAccessable = FALSE;
	this->m_bRegCrownAvailable = FALSE;

	this->m_iCastleCrownAccessUser = -1;

	this->m_btCastleCrownAccessUserX = 0;
	this->m_btCastleCrownAccessUserY = 0;

	this->m_iCastleSwitchAccessUser1 = -1;
	this->m_iCastleSwitchAccessUser2 = -1;

	this->m_dwCrownAccessTime = 0;

	memset(this->m_szMiddleWinnerGuild,0,sizeof(this->m_szMiddleWinnerGuild));

	EnterCriticalSection(&this->m_critScheduleData);
	this->m_vtScheduleData.clear();
	LeaveCriticalSection(&this->m_critScheduleData);

	EnterCriticalSection(&this->m_critNpcData);
	this->m_vtNpcData.clear();
	LeaveCriticalSection(&this->m_critNpcData);

	this->m_bDbNpcCreated = FALSE;
}

int CCastleSiege::Ready(int iMapSvrGroup)
{
	if(iMapSvrGroup < 0)
	{
		return false;
	}

	this->Clear();
	this->ClearAllNPC();

	this->m_iMapSvrGroup = iMapSvrGroup;
	this->m_iCastleDataLoadState = 1;

	return true;
}

int CCastleSiege::LoadPreFixData(char * lpszFileName)
{
	if(lpszFileName == 0)
	{
		return false;
	}

	_SYSTEMTIME tmToDay;

	GetLocalTime(&tmToDay);

	int iTODAY_DATE_NUM = MACRO2(( MACRO1(tmToDay.wDay) | MACRO1(tmToDay.wMonth) << 8 ) & 0xFFFF ) | MACRO2(tmToDay.wYear) << 16;
	__int64 iTODAY_DATE_NUM64 = (MACRO2(tmToDay.wMinute) | MACRO2(tmToDay.wHour) << 16) | ((__int64)(MACRO2(( MACRO1(tmToDay.wDay) | MACRO1(tmToDay.wMonth) << 8 ) & 0xFFFF ) | MACRO2(tmToDay.wYear) << 16) << 0x20);

	this->m_bCastleSiegeEnable = GetPrivateProfileInt("GameServerInfo","CastleSiegeEvent",0,lpszFileName);
	this->m_tmFixCastleCycleStartDate.wYear = GetPrivateProfileInt("GameServerInfo","CastleSiegeCycleStartYear",0,lpszFileName);
	this->m_tmFixCastleCycleStartDate.wMonth = GetPrivateProfileInt("GameServerInfo","CastleSiegeCycleStartMonth",0,lpszFileName);
	this->m_tmFixCastleCycleStartDate.wDay = GetPrivateProfileInt("GameServerInfo","CastleSiegeCycleStartDay",0,lpszFileName);

	this->m_iFixCastleSpecificState = GetPrivateProfileInt("GameServerInfo","CastleSiegeSpecificState",0,lpszFileName);

	this->m_tmFixCastleStateStartDate.wYear = GetPrivateProfileInt("GameServerInfo","CastleSiegeStateStartYear",0,lpszFileName);
	this->m_tmFixCastleStateStartDate.wMonth = GetPrivateProfileInt("GameServerInfo","CastleSiegeStateStartMonth",0,lpszFileName);
	this->m_tmFixCastleStateStartDate.wDay = GetPrivateProfileInt("GameServerInfo","CastleSiegeStateStartDay",0,lpszFileName);
	this->m_tmFixCastleStateStartDate.wHour = GetPrivateProfileInt("GameServerInfo","CastleSiegeStateStartHour",0,lpszFileName);
	this->m_tmFixCastleStateStartDate.wMinute = GetPrivateProfileInt("GameServerInfo","CastleSiegeStateStartMinute",0,lpszFileName);

	if(this->m_tmFixCastleCycleStartDate.wYear == 0 || this->m_tmFixCastleCycleStartDate.wMonth == 0 || this->m_tmFixCastleCycleStartDate.wDay == 0)
	{
		this->m_bFixCastleCycleStartDate = FALSE;
	}
	else
	{
		_SYSTEMTIME tmEndDate = this->m_tmFixCastleCycleStartDate;
		GetNextDay(&tmEndDate, this->m_iCastleSiegeCycle, 0, 0, 0);

		int iEVENT_END_DATE_NUM = MACRO2(( MACRO1(tmEndDate.wDay) | MACRO1(tmEndDate.wMonth) << 8 ) & 0xFFFF ) | MACRO2(tmEndDate.wYear) << 16;

		if(iEVENT_END_DATE_NUM <= iTODAY_DATE_NUM)
		{
			this->m_bFixCastleCycleStartDate = FALSE;
			ErrorMessageBox("[CastleSiege] CCastleSiege::LoadPreFixData() - iEVENT_END_DATE_NUM (%04d-%02d-%02d) <= iTODAY_DATE_NUM (%04d-%02d-%02d)", tmEndDate.wYear,tmEndDate.wMonth,tmEndDate.wDay,tmToDay.wYear,tmToDay.wMonth,tmToDay.wDay);
		}
		else
		{
			this->m_bFixCastleCycleStartDate = TRUE;
		}

		if(MONTH_MACRO(this->m_tmFixCastleCycleStartDate.wMonth) == FALSE	|| DAY_MACRO(this->m_tmFixCastleCycleStartDate.wDay) == FALSE)
		{
			this->m_bFixCastleCycleStartDate = FALSE;
		}
	}

	if(this->m_tmFixCastleStateStartDate.wYear == 0 || this->m_tmFixCastleStateStartDate.wMonth == 0 || this->m_tmFixCastleStateStartDate.wDay == 0)
	{
		this->m_bFixCastleStateStartDate = FALSE;
	}
	else if(this->m_iFixCastleSpecificState < CASTLESIEGE_STATE_REGSIEGE || this->m_iFixCastleSpecificState > CASTLESIEGE_STATE_ENDCYCLE)
	{
		this->m_bFixCastleStateStartDate = FALSE;
	}
	else
	{
		__int64 iSIEGE_DATE_NUM64 = (MACRO2(m_tmFixCastleStateStartDate.wMinute) | MACRO2(m_tmFixCastleStateStartDate.wHour) << 16) | ((__int64)(MACRO2(( MACRO1(m_tmFixCastleStateStartDate.wDay) | MACRO1(m_tmFixCastleStateStartDate.wMonth) << 8 ) & 0xFFFF ) | MACRO2(m_tmFixCastleStateStartDate.wYear) << 16) << 0x20);

		if(iSIEGE_DATE_NUM64 <= iTODAY_DATE_NUM64)
		{
			this->m_bFixCastleStateStartDate = FALSE;
			ErrorMessageBox("[CastleSiege] CCastleSiege::LoadPreFixData() - iSIEGE_DATE_NUM64 (STATE:%d, %04d-%02d-%02d %02d:%02d:%02d) <= iTODAY_DATE_NUM64 (%04d-%02d-%02d %02d:%02d:%02d)", this->m_iFixCastleSpecificState, this->m_tmFixCastleStateStartDate.wYear, this->m_tmFixCastleStateStartDate.wMonth, this->m_tmFixCastleStateStartDate.wDay, this->m_tmFixCastleStateStartDate.wHour, this->m_tmFixCastleStateStartDate.wMinute, this->m_tmFixCastleStateStartDate.wSecond, tmToDay.wYear, tmToDay.wMonth, tmToDay.wDay, tmToDay.wHour, tmToDay.wMinute, tmToDay.wSecond);
		}
		else
		{
			this->m_bFixCastleStateStartDate = TRUE;
		}

		if(this->m_iFixCastleSpecificState == CASTLESIEGE_STATE_ENDSIEGE)
		{
			ErrorMessageBox("[CastleSiege] CCastleSiege::LoadPreFixData() - FIX STATE 'CASTLESIEGE_STATE_ENDSIEGE' CAN'T BE USED");
			this->m_bFixCastleStateStartDate = FALSE;
		}

		if(MONTH_MACRO(this->m_tmFixCastleStateStartDate.wMonth) == FALSE || DAY_MACRO(this->m_tmFixCastleStateStartDate.wDay) == FALSE || HOUR_MACRO(this->m_tmFixCastleStateStartDate.wHour) == FALSE || MINUTE_MACRO(this->m_tmFixCastleStateStartDate.wMinute) == FALSE)
		{
			this->m_bFixCastleStateStartDate = FALSE;
		}
	}

	WritePrivateProfileString("GameServerInfo","CastleSiegeCycleStartYear","0",lpszFileName);
	WritePrivateProfileString("GameServerInfo","CastleSiegeCycleStartMonth","0",lpszFileName);
	WritePrivateProfileString("GameServerInfo","CastleSiegeCycleStartDay","0",lpszFileName);
	WritePrivateProfileString("GameServerInfo","CastleSiegeSpecificState","0",lpszFileName);
	WritePrivateProfileString("GameServerInfo","CastleSiegeStateStartYear","0",lpszFileName);
	WritePrivateProfileString("GameServerInfo","CastleSiegeStateStartMonth","0",lpszFileName);
	WritePrivateProfileString("GameServerInfo","CastleSiegeStateStartDay","0",lpszFileName);
	WritePrivateProfileString("GameServerInfo","CastleSiegeStateStartHour","0",lpszFileName);
	WritePrivateProfileString("GameServerInfo","CastleSiegeStateStartMinute","0",lpszFileName);

	return this->m_bFixCastleCycleStartDate & this->m_bFixCastleStateStartDate;
}

int CCastleSiege::LoadData(char * lpszFileName)
{
	using namespace std;

	if(lpszFileName == NULL || strcmp(lpszFileName,"")==NULL)
	{
		ErrorMessageBox("[CastleSiege] file load error - File Name Error");
		return false;
	}

	if(this->m_iCastleDataLoadState != CASTLESIEGE_DATALOAD_1)
	{
		LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::LoadData() - m_iCastleDataLoadState != CASTLESIEGE_DATALOAD_1 (%d)", this->m_iCastleDataLoadState);
		return false;
	}

	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,lpszFileName);
		return 0;
	}

	if(lpMemScript->SetBuffer(lpszFileName) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return 0;
	}

	int type = -1;

	std::vector<_CS_SCHEDULE_DATA> vtScheduleData;
	std::vector<_CS_NPC_DATA> vtNpcData;
	std::map<UINT,int> mpNpcDataCheck;

	int iCountDfUpgrade_Gate = 0;
	int iCountDfUpgrade_Statue = 0;
	int iCountHpUpgrade_Gate = 0;
	int iCountHpUpgrade_Statue = 0;
	int iCountRgUpgrade_Statue = 0;
	int iCountDfValue_Gate = 0;
	int iCountDfValue_Statue = 0;

	while( true )
	{
		if(lpMemScript->GetToken() == TOKEN_END)
		{
			break;
		}

		type = (int)lpMemScript->GetNumber();

		while ( true )
		{
			if(type == 0)
			{
				lpMemScript->GetToken();
				if(strcmp("end",lpMemScript->GetString())==NULL)
				{
					break;
				}

				int iCastleSiegeCycle = lpMemScript->GetNumber();

				if(iCastleSiegeCycle <= 0)
				{
					LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::LoadData() - iCastleSiegeCycle <= 0 : %d",iCastleSiegeCycle);
					return false;
				}
				else
				{
					this->m_iCastleSiegeCycle = iCastleSiegeCycle;
				}
			}
			else if(type == 1)
			{
				int iSTATE = -1;
				int iSTART_DAY = -1;
				int iSTART_HOUR = -1;
				int iSTART_MIN = -1;

				lpMemScript->GetToken();
				if(strcmp("end",lpMemScript->GetString())==NULL)
				{
					break;
				}

				iSTATE = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iSTART_DAY = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iSTART_HOUR = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iSTART_MIN = lpMemScript->GetNumber();

				if(!vtScheduleData.empty())
				{
					for(std::vector<_CS_SCHEDULE_DATA>::iterator it = vtScheduleData.begin(); it != vtScheduleData.end(); it++)
					{
						_CS_SCHEDULE_DATA pScheduleData = _CS_SCHEDULE_DATA(*it);

						if(pScheduleData.m_bIN_USE != 0)
						{
							if(pScheduleData.m_iSTATE == iSTATE)
							{
								LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::LoadData() - Same State Exist : %d",iSTATE);
								return false;
							}
							
							if(pScheduleData.m_iSTATE > iSTATE)
							{
								int iSTATE1_TIME = pScheduleData.m_iADD_DAY * 24 * 60 + pScheduleData.m_iADD_HOUR * 60 + pScheduleData.m_iADD_MIN;
								int iSTATE2_TIME = iSTART_DAY * 24 * 60 + iSTART_HOUR * 60 + iSTART_MIN;
	
								if(iSTATE1_TIME <= iSTATE2_TIME)
								{
									LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::LoadData() - Date Order is wrong : %d",iSTATE2_TIME);
									return false;
								}
							}
							else
							{
								int iSTATE1_TIME = pScheduleData.m_iADD_DAY * 24 * 60 + pScheduleData.m_iADD_HOUR * 60 + pScheduleData.m_iADD_MIN;
								int iSTATE2_TIME = iSTART_DAY * 24 * 60 + iSTART_HOUR * 60 + iSTART_MIN;
								if(iSTATE1_TIME >= iSTATE2_TIME)
								{
									LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::LoadData() - Date Order is wrong : %d",iSTATE2_TIME);
									return false;
								}
							}
						}
					}
				}

				_CS_SCHEDULE_DATA pScheduleDataInsert;

				pScheduleDataInsert.m_bIN_USE = TRUE;
				pScheduleDataInsert.m_iSTATE = iSTATE;
				pScheduleDataInsert.m_iADD_DAY = iSTART_DAY;
				pScheduleDataInsert.m_iADD_HOUR = iSTART_HOUR;
				pScheduleDataInsert.m_iADD_MIN = iSTART_MIN;

				vtScheduleData.push_back(pScheduleDataInsert);
			}
			else if(type == 2)
			{
				int iNPC_NUM = -1;
				int iNPC_INDEX = -1;
				int iNPC_SIDE = 0;
				int iNPC_DBSAVE = 0;
				int iNPC_DF_LEVEL = 0;
				int iNPC_RG_LEVEL = 0;
				int iNPC_MAXHP = 0;
				int iNPC_SX = -1;
				int iNPC_SY = -1;
				int iNPC_DX = -1;
				int iNPC_DY = -1;
				int iNPC_DIR = -1;

				lpMemScript->GetToken();
				if(strcmp("end",lpMemScript->GetString())==NULL)
				{
					break;
				}

				iNPC_NUM = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iNPC_INDEX = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iNPC_DBSAVE = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iNPC_SIDE = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iNPC_DF_LEVEL = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iNPC_RG_LEVEL = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iNPC_MAXHP = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iNPC_SX = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iNPC_SY = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iNPC_DX = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iNPC_DY = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iNPC_DIR = lpMemScript->GetNumber();


				if(iNPC_NUM < 0)
				{
					ErrorMessageBox("[CastleSiege] CCastleSiege::LoadData() - Data Fail (iNPC_NUM < 0)");
					return false;
				}

				if(iNPC_INDEX < 0)
				{
					ErrorMessageBox("[CastleSiege] CCastleSiege::LoadData() - Data Fail (iNPC_INDEX < 0)");
					return false;
				}

				if(iNPC_SIDE < 0)
				{
					ErrorMessageBox("[CastleSiege] CCastleSiege::LoadData() - Data Fail (iNPC_SIDE < 0)");
					return false;
				}

				if(iNPC_DF_LEVEL < 0)
				{
					ErrorMessageBox("[CastleSiege] CCastleSiege::LoadData() - Data Fail (iNPC_DF_LEVEL < 0)");
					return false;
				}

				if(iNPC_RG_LEVEL < 0)
				{
					ErrorMessageBox("[CastleSiege] CCastleSiege::LoadData() - Data Fail (iNPC_RG_LEVEL < 0)");
					return false;
				}

				if(iNPC_MAXHP < 0)
				{
					ErrorMessageBox("[CastleSiege] CCastleSiege::LoadData() - Data Fail (iNPC_MAXHP < 0)");
					return false;
				}

				if(iNPC_SX < 0)
				{
					ErrorMessageBox("[CastleSiege] CCastleSiege::LoadData() - Data Fail (iNPC_SX < 0)");
					return false;
				}

				if(iNPC_SY < 0)
				{
					ErrorMessageBox("[CastleSiege] CCastleSiege::LoadData() - Data Fail (iNPC_SY < 0)");
					return false;
				}

				UINT uNpcKey = MACRO2(iNPC_INDEX) | (MACRO2(iNPC_NUM) << 16);

				std::map<UINT,int>::iterator it = mpNpcDataCheck.find(uNpcKey);

				if(it != mpNpcDataCheck.end())
				{
					ErrorMessageBox("[CastleSiege] CCastleSiege::LoadData() - Same Npc Key Exist : NUM:%d-IDX:%d",iNPC_NUM,iNPC_INDEX);
					return false;
				}

				mpNpcDataCheck.insert(pair <UINT, int > ( ( MACRO2(iNPC_INDEX) | (MACRO2(iNPC_NUM) << 16) ), 0));

				_CS_NPC_DATA pNpcDataInsert;

				pNpcDataInsert.m_bIN_USE = TRUE;
				pNpcDataInsert.m_iNPC_NUM = iNPC_NUM;
				pNpcDataInsert.m_iNPC_INDEX = iNPC_INDEX;
				pNpcDataInsert.m_iNPC_SIDE = iNPC_SIDE;
				pNpcDataInsert.m_bNPC_DBSAVE = iNPC_DBSAVE;

				pNpcDataInsert.m_iNPC_BASE_DF_LEVEL = iNPC_DF_LEVEL;
				pNpcDataInsert.m_iNPC_DF_LEVEL = pNpcDataInsert.m_iNPC_BASE_DF_LEVEL;

				pNpcDataInsert.m_iNPC_BASE_RG_LEVEL = iNPC_RG_LEVEL;
				pNpcDataInsert.m_iNPC_RG_LEVEL = pNpcDataInsert.m_iNPC_BASE_RG_LEVEL;

				pNpcDataInsert.m_iNPC_BASE_MAXHP = iNPC_MAXHP;
				pNpcDataInsert.m_iNPC_MAXHP = pNpcDataInsert.m_iNPC_BASE_MAXHP;

				pNpcDataInsert.m_iNPC_BASE_HP = iNPC_MAXHP;
				pNpcDataInsert.m_iNPC_HP = pNpcDataInsert.m_iNPC_BASE_HP;

				pNpcDataInsert.m_iNPC_BASE_SX = iNPC_SX;
				pNpcDataInsert.m_iNPC_SX = pNpcDataInsert.m_iNPC_BASE_SX;

				pNpcDataInsert.m_iNPC_BASE_SY = iNPC_SY;
				pNpcDataInsert.m_iNPC_SY = pNpcDataInsert.m_iNPC_BASE_SY;

				pNpcDataInsert.m_iNPC_BASE_DX = iNPC_DX;
				pNpcDataInsert.m_iNPC_DX = pNpcDataInsert.m_iNPC_BASE_DX;

				pNpcDataInsert.m_iNPC_BASE_DY = iNPC_DY;
				pNpcDataInsert.m_iNPC_DY = pNpcDataInsert.m_iNPC_BASE_DY;

				pNpcDataInsert.m_iNPC_BASE_DIR = iNPC_DIR;
				pNpcDataInsert.m_iNPC_DIR = pNpcDataInsert.m_iNPC_BASE_DIR;

				vtNpcData.push_back(pNpcDataInsert);
			}
			else if(type == 3)
			{
				int iLEVEL = 0;
				int iGEMCOUNT = 0;
				int iZEN = 0;

				lpMemScript->GetToken();
				if(strcmp("end",lpMemScript->GetString())==NULL)
				{
					break;
				}

				iLEVEL = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iGEMCOUNT = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iZEN = lpMemScript->GetNumber();

				if(iCountDfUpgrade_Gate >= CS_MAX_UPGRADE)
				{
					break;
				}

				g_iNpcUpDfLevel_CGATE[iCountDfUpgrade_Gate][0] = iLEVEL;
				g_iNpcUpDfLevel_CGATE[iCountDfUpgrade_Gate][1] = iGEMCOUNT;
				g_iNpcUpDfLevel_CGATE[iCountDfUpgrade_Gate][2] = iZEN;

				iCountDfUpgrade_Gate++;
			}
			else if(type == 4)
			{
				int iLEVEL = 0;
				int iGEMCOUNT = 0;
				int iZEN = 0;

				lpMemScript->GetToken();
				if(strcmp("end",lpMemScript->GetString())==NULL)
				{
					break;
				}

				iLEVEL = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iGEMCOUNT = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iZEN = lpMemScript->GetNumber();

				if(iCountDfUpgrade_Statue >= CS_MAX_UPGRADE)
				{
					break;
				}

				g_iNpcUpDfLevel_CSTATUE[iCountDfUpgrade_Statue][0] = iLEVEL;
				g_iNpcUpDfLevel_CSTATUE[iCountDfUpgrade_Statue][1] = iGEMCOUNT;
				g_iNpcUpDfLevel_CSTATUE[iCountDfUpgrade_Statue][2] = iZEN;

				iCountDfUpgrade_Statue++;
			}
			else if(type == 5)
			{
				int iHP = 0;
				int iGEMCOUNT = 0;
				int iZEN = 0;

				lpMemScript->GetToken();
				if(strcmp("end",lpMemScript->GetString())==NULL)
				{
					break;
				}

				iHP = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iGEMCOUNT = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iZEN = lpMemScript->GetNumber();

				if(iCountHpUpgrade_Gate >= CS_MAX_UPGRADE)
				{
					break;
				}

				g_iNpcUpMaxHP_CGATE[iCountHpUpgrade_Gate][0] = iHP;
				g_iNpcUpMaxHP_CGATE[iCountHpUpgrade_Gate][1] = iGEMCOUNT;
				g_iNpcUpMaxHP_CGATE[iCountHpUpgrade_Gate][2] = iZEN;

				iCountHpUpgrade_Gate++;
			}
			else if(type == 6)
			{
				int iHP = 0;
				int iGEMCOUNT = 0;
				int iZEN = 0;

				lpMemScript->GetToken();
				if(strcmp("end",lpMemScript->GetString())==NULL)
				{
					break;
				}

				iHP = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iGEMCOUNT = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iZEN = lpMemScript->GetNumber();

				if(iCountHpUpgrade_Statue >= CS_MAX_UPGRADE)
				{
					break;
				}

				g_iNpcUpMaxHP_CSTATUE[iCountHpUpgrade_Statue][0] = iHP;
				g_iNpcUpMaxHP_CSTATUE[iCountHpUpgrade_Statue][1] = iGEMCOUNT;
				g_iNpcUpMaxHP_CSTATUE[iCountHpUpgrade_Statue][2] = iZEN;

				iCountHpUpgrade_Statue++;
			}
			else if(type == 7)
			{
				int iLEVEL = 0;
				int iGEMCOUNT = 0;
				int iZEN = 0;

				lpMemScript->GetToken();
				if(strcmp("end",lpMemScript->GetString())==NULL)
				{
					break;
				}

				iLEVEL = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iGEMCOUNT = lpMemScript->GetNumber();
				lpMemScript->GetToken();
				iZEN = lpMemScript->GetNumber();

				if(iCountRgUpgrade_Statue >= CS_MAX_UPGRADE)
				{
					break;
				}

				g_iNpcUpRgLevel_CSTATUE[iCountRgUpgrade_Statue][0] = iLEVEL;
				g_iNpcUpRgLevel_CSTATUE[iCountRgUpgrade_Statue][1] = iGEMCOUNT;
				g_iNpcUpRgLevel_CSTATUE[iCountRgUpgrade_Statue][2] = iZEN;

				iCountRgUpgrade_Statue++;
			}
			else if(type == 8)
			{
				int iDFVALUE = 0;

				lpMemScript->GetToken();
				if(strcmp("end",lpMemScript->GetString())==NULL)
				{
					break;
				}

				iDFVALUE = lpMemScript->GetNumber();

				if(iCountDfValue_Gate >= CS_MAX_UPGRADE + 1)
				{
					break;
				}

				g_iNpcDefense_CGATE[iCountDfValue_Gate] = iDFVALUE;

				iCountDfValue_Gate++;
			}
			else if(type == 9)
			{
				int iDFVALUE = 0;

				lpMemScript->GetToken();
				if(strcmp("end",lpMemScript->GetString())==NULL)
				{
					break;
				}

				iDFVALUE = lpMemScript->GetNumber();

				if(iCountDfValue_Statue >= CS_MAX_UPGRADE + 1)
				{
					break;
				}

				g_iNpcDefense_CSTATUE[iCountDfValue_Statue] = iDFVALUE;

				iCountDfValue_Statue++;
			}
		}
	}

	delete lpMemScript;

	EnterCriticalSection(&this->m_critScheduleData);

	if(!vtScheduleData.empty())
	{
		for(std::vector<_CS_SCHEDULE_DATA>::iterator it = vtScheduleData.begin(); it != vtScheduleData.end(); it++)
		{
			this->m_vtScheduleData.push_back(*it);
		}
	}

	sort(this->m_vtScheduleData.begin(),this->m_vtScheduleData.end(),this->ScheduleStateCompFunc);

	if(!m_vtScheduleData.empty())
	{
		for(int iIDX = 0; (DWORD)iIDX < this->m_vtScheduleData.size(); iIDX++)
		{
			if((DWORD)(iIDX+1) < this->m_vtScheduleData.size())
			{
				int iIDX_SEC1 = this->m_vtScheduleData[iIDX].m_iADD_DAY * 24 * 60 * 60 + this->m_vtScheduleData[iIDX].m_iADD_HOUR * 60 * 60 + this->m_vtScheduleData[iIDX].m_iADD_MIN * 60;
				int iIDX_SEC2 = this->m_vtScheduleData[iIDX+1].m_iADD_DAY * 24 * 60 * 60 + this->m_vtScheduleData[iIDX+1].m_iADD_HOUR * 60 * 60 + this->m_vtScheduleData[iIDX+1].m_iADD_MIN * 60;

				int iIDX_RESULT = iIDX_SEC2 - iIDX_SEC1;

				if(iIDX_RESULT < 0)
				{
					LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::LoadData() - Date Order is wrong (sort fail) : %d-%d", this->m_vtScheduleData[iIDX].m_iSTATE, this->m_vtScheduleData[iIDX+1].m_iSTATE);
				}
				else
				{
					this->m_vtScheduleData[iIDX].m_iGAP_SEC = iIDX_RESULT;
				}
			}
			else
			{
				this->m_vtScheduleData[iIDX].m_iGAP_SEC = 0;
			}
		}
	}

	LeaveCriticalSection(&this->m_critScheduleData);

	EnterCriticalSection(&this->m_critNpcData);

	if(!vtNpcData.empty())
	{
		for(std::vector<_CS_NPC_DATA>::iterator it = vtNpcData.begin();it != vtNpcData.end(); it++)
		{
			this->m_vtNpcData.push_back(*it);
		}
	}

	LeaveCriticalSection(&this->m_critNpcData);

	std::vector<_CS_SCHEDULE_DATA>::iterator it1 = this->m_vtScheduleData.begin();

	while(it1 != this->m_vtScheduleData.end())
	{
		_CS_SCHEDULE_DATA stTEMP = *it1;
		char szTEMP[256] = {0};
		wsprintf(szTEMP,"STT:%d, DAY:%d, HOUR:%d, MIN:%d, GAP:%d\n", stTEMP.m_iSTATE,stTEMP.m_iADD_DAY,stTEMP.m_iADD_HOUR,stTEMP.m_iADD_MIN,stTEMP.m_iGAP_SEC);
		OutputDebugString(szTEMP);
		it1++;
	}

	std::vector<_CS_NPC_DATA>::iterator it2 = this->m_vtNpcData.begin();

	while(it2 != this->m_vtNpcData.end())
	{
		_CS_NPC_DATA stTEMP = *it2;
		char szTEMP[256] = {0};
		wsprintf(szTEMP,"NUM:%d, IDX:%d, DBSAVE:%s, SIDE:%d, DFLEVEL:%d, RGLEVEL:%d, MXHP:%d, (%d,%d)-(%d,%d), DIR:%d\n", stTEMP.m_iNPC_NUM,stTEMP.m_iNPC_INDEX,(stTEMP.m_bNPC_DBSAVE != 0) ? "YES" : "NO",stTEMP.m_iNPC_SIDE,stTEMP.m_iNPC_DF_LEVEL, stTEMP.m_iNPC_RG_LEVEL,stTEMP.m_iNPC_MAXHP,stTEMP.m_iNPC_SX,stTEMP.m_iNPC_SY,stTEMP.m_iNPC_DX,stTEMP.m_iNPC_DY,stTEMP.m_iNPC_DIR);
		OutputDebugString(szTEMP);
		it2++;
	}

	this->m_bFileDataLoadOK = TRUE;
	return true;
}

int CCastleSiege::DataRequest()
{
	if(this->m_bFileDataLoadOK == FALSE)
	{
		LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::DataRequest() - m_bFileDataLoadOK == FALSE");
		return false;
	}

	if(this->m_iMapSvrGroup < 0)
	{
		LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::DataRequest() - m_iMapSvrGroup < 0");
		return false;
	}

	if(this->m_iCastleDataLoadState != CASTLESIEGE_DATALOAD_2)
	{
		LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::DataRequest() - m_iCastleDataLoadState != CASTLESIEGE_DATALOAD_2 (%d)",m_iCastleDataLoadState);
		return false;
	}

	BOOL bDbRequestData = FALSE;

	if(this->m_dwDbDataLoadTickCount == 0)
	{
		bDbRequestData = TRUE;
		m_dwDbDataLoadTickCount = GetTickCount();
		LogAdd(LOG_GREEN,"[CastleSiege] CCastleSiege::DataRequest() - REQUEST DATA FIRST");
	}
	else if(GetTickCount() - this->m_dwDbDataLoadTickCount > 3000)
	{
		bDbRequestData = TRUE;
		m_dwDbDataLoadTickCount = GetTickCount();
		LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::DataRequest() - REQUEST DATA AGAIN");
	}

	if(bDbRequestData != FALSE)
	{
		GS_GDReqCastleInitData(m_iMapSvrGroup,m_iCastleSiegeCycle);
	}

	return true;
}

int CCastleSiege::Init()
{
	if(this->m_iCastleDataLoadState != CASTLESIEGE_DATALOAD_4)
	{
		LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::Init() - m_iCastleDataLoadState != CASTLESIEGE_DATALOAD_4 (%d)",this->m_iCastleDataLoadState);
		ErrorMessageBox("[CastleSiege] CCastleSiege::Init() - m_iCastleDataLoadState != CASTLESIEGE_DATALOAD_4 (%d)",this->m_iCastleDataLoadState);
		return false;
	}

	if(this->m_bFileDataLoadOK == FALSE || this->m_bDbDataLoadOK == FALSE)
	{
		LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::Init() - Data Load Fail (FILE:%d, DB:%d)", this->m_bFileDataLoadOK, this->m_bDbDataLoadOK);
		ErrorMessageBox("[CastleSiege] CCastleSiege::Init() - Data Load Fail (FILE:%d, DB:%d)", this->m_bFileDataLoadOK, this->m_bDbDataLoadOK);
		return false;
	}

	LogAdd(LOG_BLACK,"[CastleSiege] CCastleSiege::Init() - START", this->m_bFileDataLoadOK, this->m_bDbDataLoadOK);

	this->m_iCastleDataLoadState = 0;

	this->CreateAllCsGateLever();

	if(this->CheckSync() == FALSE)
	{
		LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::Init() - CheckSync() == FALSE");
		//ErrorMessageBox("[CastleSiege] CCastleSiege::Init() - CheckSync() == FALSE");
		return false;
	}

	this->m_bDoRun = TRUE;

	this->m_dwNPC_DBSAVE_TICK_COUNT = GetTickCount();
	this->m_dwCALC_LEFTSIEGE_TICK_COUNT = 0;
	this->m_dwEVENT_MSG_TICK_COUNT = 0;
	this->m_dwCHECK_GATE_ALIVE_COUNT = 0;
	this->m_dwCS_STARTTIME_TICK_COUNT = 0;
	this->m_dwCS_LEFTTIME_TICK_COUNT = 0;
	this->m_dwCS_JOINSIDE_REFRESH_TICK_COUNT = 0;
	this->m_dwCS_JOINSIDE_REFRESH_TOTAL_COUNT = 0;
	this->m_dwCS_MINIMAP_SEND_TICK_COUNT = 0;
	
	return true;
}

void CCastleSiege::SetState(int iCastleSiegeState,BOOL bSetRemainMsec)
{
	if(iCastleSiegeState < CASTLESIEGE_STATE_NONE || iCastleSiegeState > CASTLESIEGE_STATE_ENDCYCLE)
	{
		return;
	}

	this->m_iCastleSiegeState = iCastleSiegeState;

	switch(this->m_iCastleSiegeState)
	{
	case CASTLESIEGE_STATE_NONE:			this->SetState_NONE(bSetRemainMsec); break;
	case CASTLESIEGE_STATE_IDLE_1:			this->SetState_IDLE_1(bSetRemainMsec); break;
	case CASTLESIEGE_STATE_REGSIEGE:		this->SetState_REGSIEGE(bSetRemainMsec); break;
	case CASTLESIEGE_STATE_IDLE_2:			this->SetState_IDLE_2(bSetRemainMsec); break;
	case CASTLESIEGE_STATE_REGMARK:			this->SetState_REGMARK(bSetRemainMsec); break;
	case CASTLESIEGE_STATE_IDLE_3:			this->SetState_IDLE_3(bSetRemainMsec); break;
	case CASTLESIEGE_STATE_NOTIFY:			this->SetState_NOTIFY(bSetRemainMsec); break;
	case CASTLESIEGE_STATE_READYSIEGE:		this->SetState_READYSIEGE(bSetRemainMsec); break;
	case CASTLESIEGE_STATE_STARTSIEGE:		this->SetState_STARTSIEGE(bSetRemainMsec); break;
	case CASTLESIEGE_STATE_ENDSIEGE:		this->SetState_ENDSIEGE(bSetRemainMsec); break;
	case CASTLESIEGE_STATE_ENDCYCLE:		this->SetState_ENDCYCLE(bSetRemainMsec); break;
	}

	if(this->m_bDbNpcCreated == FALSE)
	{
		this->m_bDbNpcCreated = TRUE;
		this->CreateDbNPC();
	}

	GetLocalTime(&this->m_tmStateStartDate);

	this->m_dwEVENT_MSG_TICK_COUNT = 0;
	this->m_dwCHECK_GATE_ALIVE_COUNT = 0;
}

void CCastleSiege::MainProc()
{
	if( this->m_bDoRun == FALSE )
	{
		return;
	}
	
	if( this->m_bCastleSiegeEnable == FALSE )
	{
		return;
	}

	switch( this->m_iCastleSiegeState )
	{
	case CASTLESIEGE_STATE_NONE:
		this->ProcState_NONE();
		break;
	case CASTLESIEGE_STATE_IDLE_1:
		this->ProcState_IDLE_1();
		break;
	case CASTLESIEGE_STATE_REGSIEGE:
		this->ProcState_REGSIEGE();
		break;
	case CASTLESIEGE_STATE_IDLE_2:
		this->ProcState_IDLE_2();
		break;
	case CASTLESIEGE_STATE_REGMARK:
		this->ProcState_REGMARK();
		break;
	case CASTLESIEGE_STATE_IDLE_3:
		this->ProcState_IDLE_3();
		break;
	case CASTLESIEGE_STATE_NOTIFY:
		this->ProcState_NOTIFY();
		break;
	case CASTLESIEGE_STATE_READYSIEGE:
		this->ProcState_READYSIEGE();
		break;
	case CASTLESIEGE_STATE_STARTSIEGE:
		this->ProcState_STARTSIEGE();
		break;
	case CASTLESIEGE_STATE_ENDSIEGE:
		this->ProcState_ENDSIEGE();
		break;
	case CASTLESIEGE_STATE_ENDCYCLE:
		this->ProcState_ENDCYCLE();
		break;
	}

	if(gServerInfo.m_CastleSiegeEvent == 0)
	{
		if (gServerDisplayer.EventCs != -1)
		{
			gServerDisplayer.EventCs = -1;
		}
	}
	else 
	{
			gServerDisplayer.EventCs = this->m_iCS_REMAIN_MSEC/1000;
			gServerDisplayer.EventCsState = this->m_iCastleSiegeState;
	}

	if( (GetTickCount() - this->m_dwNPC_DBSAVE_TICK_COUNT) > 300000 )
	{
		if( this->m_iCastleSiegeState != CASTLESIEGE_STATE_STARTSIEGE )
		{
			this->StoreDbNpc();
		}
		this->m_dwNPC_DBSAVE_TICK_COUNT = GetTickCount();
	}

	
	if( (GetTickCount() - this->m_dwCALC_LEFTSIEGE_TICK_COUNT) > 10000 )
	{
		this->m_dwCALC_LEFTSIEGE_TICK_COUNT = GetTickCount();
		this->CalcCastleLeftSiegeDate();
	}

	static int g_iLastSiegeUpdate = 0;

	if( (GetTickCount() - g_iLastSiegeUpdate) > 60000 )
	{
		g_iLastSiegeUpdate = GetTickCount();

		int iLeftSiegeDate = this->CalcCastleLeftSiegeDate();

		if( iLeftSiegeDate >= 0 )
		{
			SYSTEMTIME tmLeftDate = this->GetCastleLeftSiegeDate();
			LogAdd(LOG_BLUE, "LEFT : SEC(%d), SIEGE-DAY(%04d-%02d-%02d %02d:%02d:%02d)", iLeftSiegeDate, tmLeftDate.wYear, tmLeftDate.wMonth, tmLeftDate.wDay, tmLeftDate.wHour, tmLeftDate.wMinute, tmLeftDate.wSecond);
		}
		else
		{
			switch( iLeftSiegeDate )
			{
			case CASTLESIEGE_LEFTTIME_ONSIGE:
				LogAdd(LOG_BLUE, "[CastleSiege] LEFT-TIME : CASTLESIEGE_LEFTTIME_ONSIGE");
				break;
			case CASTLESIEGE_LEFTTIME_ENDSIEGE:
				LogAdd(LOG_BLUE, "[CastleSiege] LEFT-TIME : CASTLESIEGE_LEFTTIME_ENDSIEGE");
				break;
			case CASTLESIEGE_LEFTTIME_NOTRUNNING:
				LogAdd(LOG_BLUE, "[CastleSiege] LEFT-TIME : CASTLESIEGE_LEFTTIME_NOTRUNNING");
				break;
			}
		}
	}
}

void CCastleSiege::SetState_NONE(BOOL bSetRemainMsec)
{

}

void CCastleSiege::SetState_IDLE_1(BOOL bSetRemainMsec)
{
	LogAdd(LOG_BLACK,"SET SetState_IDLE_1()");

	if(bSetRemainMsec != FALSE)
	{
		time_t ttSTime;
		time_t ttETime;
		tm tmETime;
		double dResultSecond;

		memset(&tmETime,0,sizeof(tmETime));

		tmETime.tm_year = m_tmStartDate.wYear - 1900;
		tmETime.tm_mon = m_tmStartDate.wMonth - 1;
		tmETime.tm_mday = m_tmStartDate.wDay;

		ttETime = mktime(&tmETime);
		time(&ttSTime);

		dResultSecond = difftime(ttETime,ttSTime);
		this->m_iCS_REMAIN_MSEC = (int)(dResultSecond * (double)1000.0);
		this->m_iCS_TARGET_MSEC = (int)(time(0)+(this->m_iCS_REMAIN_MSEC/1000));
	}
}

void CCastleSiege::SetState_REGSIEGE(BOOL bSetRemainMsec)
{
	LogAdd(LOG_BLACK,"SET SetState_REGSIEGE()");

	if(bSetRemainMsec != FALSE)
	{
		int iGAP_SEC = GetStateGapSec(CASTLESIEGE_STATE_REGSIEGE);

		if(iGAP_SEC < 0)
		{
			this->m_bDoRun = FALSE;
			ErrorMessageBox("[CastleSiege] CCastleSiege::SetState_REGSIEGE() - iGAP_SEC < 0");
		}

		this->m_iCS_REMAIN_MSEC = iGAP_SEC * 1000;
		this->m_iCS_TARGET_MSEC = (int)(time(0)+(this->m_iCS_REMAIN_MSEC/1000));
	}

	GS_GDReqRestartCastleState(m_iMapSvrGroup);
}

void CCastleSiege::SetState_IDLE_2(BOOL bSetRemainMsec)
{
	LogAdd(LOG_BLACK,"SET SetState_IDLE_2()");

	if(bSetRemainMsec != FALSE)
	{
		int iGAP_SEC = GetStateGapSec(CASTLESIEGE_STATE_IDLE_2);

		if(iGAP_SEC < 0)
		{
			this->m_bDoRun = FALSE;
			ErrorMessageBox("[CastleSiege] CCastleSiege::SetState_IDLE_2() - iGAP_SEC < 0");
		}

		this->m_iCS_REMAIN_MSEC = iGAP_SEC * 1000;
		this->m_iCS_TARGET_MSEC = (int)(time(0)+(this->m_iCS_REMAIN_MSEC/1000));
	}
}

void CCastleSiege::SetState_REGMARK(BOOL bSetRemainMsec)
{
	LogAdd(LOG_BLACK,"SET SetState_REGMARK()");

	if(bSetRemainMsec != FALSE)
	{
		int iGAP_SEC = GetStateGapSec(CASTLESIEGE_STATE_REGMARK);

		if(iGAP_SEC < 0)
		{
			this->m_bDoRun = FALSE;
			ErrorMessageBox("[CastleSiege] CCastleSiege::SetState_REGMARK() - iGAP_SEC < 0");
		}

		this->m_iCS_REMAIN_MSEC = iGAP_SEC * 1000;
		this->m_iCS_TARGET_MSEC = (int)(time(0)+(this->m_iCS_REMAIN_MSEC/1000));
	}
}

void CCastleSiege::SetState_IDLE_3(BOOL bSetRemainMsec)
{
	LogAdd(LOG_BLACK,"SET SetState_IDLE_3()");

	if(bSetRemainMsec != FALSE)
	{
		int iGAP_SEC = GetStateGapSec(CASTLESIEGE_STATE_IDLE_3);

		if(iGAP_SEC < 0)
		{
			this->m_bDoRun = FALSE;
			ErrorMessageBox("[CastleSiege] CCastleSiege::SetState_IDLE_3() - iGAP_SEC < 0");
		}

		this->m_iCS_REMAIN_MSEC = iGAP_SEC * 1000;
		this->m_iCS_TARGET_MSEC = (int)(time(0)+(this->m_iCS_REMAIN_MSEC/1000));
	}
}

void CCastleSiege::SetState_NOTIFY(BOOL bSetRemainMsec)
{
	LogAdd(LOG_BLACK,"SET SetState_NOTIFY()");

	if(bSetRemainMsec != FALSE)
	{
		int iGAP_SEC = GetStateGapSec(CASTLESIEGE_STATE_NOTIFY);

		if(iGAP_SEC < 0)
		{
			this->m_bDoRun = FALSE;
			ErrorMessageBox("[CastleSiege] CCastleSiege::SetState_NOTIFY() - iGAP_SEC < 0");
		}

		this->m_iCS_REMAIN_MSEC = iGAP_SEC * 1000;
		this->m_iCS_TARGET_MSEC = (int)(time(0)+(this->m_iCS_REMAIN_MSEC/1000));
	}

	this->CheckBuildCsGuildInfo();
}

void CCastleSiege::SetState_READYSIEGE(BOOL bSetRemainMsec)
{
	LogAdd(LOG_BLACK,"SET SetState_READYSIEGE()");

	if(bSetRemainMsec != FALSE)
	{
		int iGAP_SEC = GetStateGapSec(CASTLESIEGE_STATE_READYSIEGE);

		if(iGAP_SEC < 0)
		{
			this->m_bDoRun = 0;
			ErrorMessageBox("[CastleSiege] CCastleSiege::SetState_READYSIEGE() - iGAP_SEC < 0");
		}

		this->m_iCS_REMAIN_MSEC = iGAP_SEC * 1000;
		this->m_iCS_TARGET_MSEC = (int)(time(0)+(this->m_iCS_REMAIN_MSEC/1000));
	}

	this->CheckBuildCsGuildInfo();
}

void CCastleSiege::SetState_STARTSIEGE(BOOL bSetRemainMsec)
{
	LogAdd(LOG_BLACK,"SET SetState_STARTSIEGE()");

	if(bSetRemainMsec != FALSE)
	{
		int iGAP_SEC = GetStateGapSec(CASTLESIEGE_STATE_STARTSIEGE);

		if(iGAP_SEC < 0)
		{
			this->m_bDoRun = FALSE;
			ErrorMessageBox("[CastleSiege] CCastleSiege::SetState_STARTSIEGE() - iGAP_SEC < 0");
		}

		this->m_iCS_REMAIN_MSEC = iGAP_SEC * 1000;
		this->m_iCS_TARGET_MSEC = (int)(time(0)+(this->m_iCS_REMAIN_MSEC/1000));
	}

	this->m_bCastleTowerAccessable = FALSE;
	this->m_bRegCrownAvailable = FALSE;

	this->m_iCastleCrownAccessUser = -1;

	this->m_btCastleCrownAccessUserX = 0;
	this->m_btCastleCrownAccessUserY = 0;

	this->m_iCastleSwitchAccessUser1 = -1;
	this->m_iCastleSwitchAccessUser2 = -1;

	this->m_dwCrownAccessTime = 0;

	this->m_dwCHECK_GATE_ALIVE_COUNT = 0;
	this->m_dwCS_STARTTIME_TICK_COUNT = 0;
	this->m_dwCS_LEFTTIME_TICK_COUNT = GetTickCount();
	this->m_dwCS_JOINSIDE_REFRESH_TICK_COUNT = GetTickCount();
	this->m_dwCS_JOINSIDE_REFRESH_TOTAL_COUNT = 0;
	this->m_dwCS_MINIMAP_SEND_TICK_COUNT = GetTickCount();

	memset(this->m_szMiddleWinnerGuild,0,sizeof(this->m_szMiddleWinnerGuild));

	if(this->CheckAttackGuildExist() == FALSE)
	{
		this->CheckCastleSiegeResult();
		this->SetState(CASTLESIEGE_STATE_ENDSIEGE,TRUE);
		LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::SetState_STARTSIEGE() - CheckAttackGuildExist() == FALSE");
	}
	else
	{
		this->StoreDbNpc();
		this->ClearNonDbNPC();
		this->CheckReviveNonDbNPC();

		if( this->CheckGuardianStatueExist() == FALSE )
		{
			this->ClearCastleTowerBarrier();
			this->SetCastleTowerAccessable(TRUE);
		}

		this->AdjustDbNpcLevel();
		this->SetAllCastleGateState(FALSE);
		this->SetAllUserCsJoinSide();
		this->SavePcRoomUserList();
		this->ReSpawnEnemyUser(TRUE);
		this->NotifyAllUserCsStartState(1);
	}
}

void CCastleSiege::SetState_ENDSIEGE(BOOL bSetRemainMsec)
{
	LogAdd(LOG_BLACK,"SET SetState_ENDSIEGE()");

	if(bSetRemainMsec != FALSE)
	{
		time_t ttSTime;
		time_t ttETime;
		tm tmETime;
		double dResultSecond;

		memset(&tmETime,0,sizeof(tmETime));

		tmETime.tm_year = m_tmEndDate.wYear - 1900;
		tmETime.tm_mon = m_tmEndDate.wMonth - 1;
		tmETime.tm_mday = m_tmEndDate.wDay;

		time(&ttSTime);
		ttETime = mktime(&tmETime);

		dResultSecond = difftime(ttETime,ttSTime);
		this->m_iCS_REMAIN_MSEC = (int)(dResultSecond * (double)1000.0);
		this->m_iCS_TARGET_MSEC = (int)(time(0)+(this->m_iCS_REMAIN_MSEC/1000));
	}

	this->ClearNonDbNPC();
	this->ResetAllUserCsJoinSide();
	this->SendAllUserAnyMsg(gMessage.GetMessage(416),2);
}

void CCastleSiege::SetState_ENDCYCLE(BOOL bSetRemainMsec)
{
	LogAdd(LOG_BLACK,"SET SetState_ENDCYCLE()");

	SYSTEMTIME tmNowDate;

	GetLocalTime(&tmNowDate);

	tmNowDate.wHour = 0;
	tmNowDate.wMinute = 0;
	tmNowDate.wSecond = 0;
	tmNowDate.wMilliseconds = 0;

	tmNowDate = this->m_tmSiegeEndSchedule;

	this->m_tmStartDate = this->m_tmSiegeEndSchedule;

	GetNextDay(&tmNowDate, this->m_iCastleSiegeCycle, 0, 0, 0);

	GetNextDay(&this->m_tmSiegeEndSchedule, this->m_iCastleSiegeCycle, 0, 0, 0);

	this->m_tmEndDate = tmNowDate;

	LogAdd(LOG_BLACK,"[CastleSiege] State - End Cycle Schedule : (%d-%d-%d(%d:%d:%d)) (%d-%d-%d(%d:%d:%d))", this->m_tmStartDate.wYear, this->m_tmStartDate.wMonth, this->m_tmStartDate.wDay, this->m_tmStartDate.wHour, this->m_tmStartDate.wMinute, this->m_tmStartDate.wSecond, this->m_tmEndDate.wYear, this->m_tmEndDate.wMonth, this->m_tmEndDate.wDay, this->m_tmEndDate.wHour, this->m_tmEndDate.wMinute, this->m_tmEndDate.wSecond);
	LogAdd(LOG_BLACK,"[CastleSiege] State - End Cycle : Date-Changing Info (%d-%d-%d) (%d-%d-%d)", this->m_tmStartDate.wYear, this->m_tmStartDate.wMonth, this->m_tmStartDate.wDay, this->m_tmEndDate.wYear, this->m_tmEndDate.wMonth, this->m_tmEndDate.wDay);

	GS_GDReqSiegeDateChange(this->m_iMapSvrGroup, -1, this->m_tmStartDate.wYear, (BYTE)this->m_tmStartDate.wMonth, (BYTE)this->m_tmStartDate.wDay, this->m_tmEndDate.wYear, (BYTE)this->m_tmEndDate.wMonth, (BYTE)this->m_tmEndDate.wDay);
	GS_GDReqResetRegSiegeInfo(this->m_iMapSvrGroup);
	GS_GDReqResetSiegeGuildInfo(this->m_iMapSvrGroup);

	this->SetState(CASTLESIEGE_STATE_REGSIEGE, TRUE);
}

void CCastleSiege::ProcState_NONE()
{

}

void CCastleSiege::ProcState_IDLE_1()
{
	int iTICK_MSEC = ( GetTickCount() - this->m_iCS_TICK_COUNT );

	if( iTICK_MSEC >= 1000 )
	{
		this->m_iCS_REMAIN_MSEC = (int)(difftime(this->m_iCS_TARGET_MSEC,time(0))*1000);
		this->m_iCS_TICK_COUNT = GetTickCount();
		//LogAdd(LOG_GREEN, "RUN ProcState_IDLE_1()\tLEFT-MSEC:%d", this->m_iCS_REMAIN_MSEC);
	}

	if( this->m_iCS_REMAIN_MSEC <= 0 )
	{
		this->SetState(CASTLESIEGE_STATE_REGSIEGE, TRUE);
	}
}

void CCastleSiege::ProcState_REGSIEGE()
{
	int iTICK_MSEC = ( GetTickCount() - this->m_iCS_TICK_COUNT );

	if( iTICK_MSEC >= 1000 )
	{
		this->m_iCS_REMAIN_MSEC = (int)(difftime(this->m_iCS_TARGET_MSEC,time(0))*1000);
		this->m_iCS_TICK_COUNT = GetTickCount();
		
		if( (GetTickCount() - this->m_dwEVENT_MSG_TICK_COUNT) > 1800000 )
		{
			this->SendAllUserAnyMsg(gMessage.GetMessage(417), 2);
			this->m_dwEVENT_MSG_TICK_COUNT = GetTickCount();
		}
		//LogAdd(LOG_GREEN, "RUN ProcState_REGSIEGE()\tLEFT-MSEC:%d", this->m_iCS_REMAIN_MSEC);
	}

	if( this->m_iCS_REMAIN_MSEC <= 0 )
	{
		this->SetState(CASTLESIEGE_STATE_IDLE_2, TRUE);
	}
}

void CCastleSiege::ProcState_IDLE_2()
{
	int iTICK_MSEC = ( GetTickCount() - this->m_iCS_TICK_COUNT );

	if( iTICK_MSEC >= 1000 )
	{
		this->m_iCS_REMAIN_MSEC = (int)(difftime(this->m_iCS_TARGET_MSEC,time(0))*1000);
		this->m_iCS_TICK_COUNT = GetTickCount();
		//LogAdd(LOG_GREEN, "RUN ProcState_IDLE_2()\tLEFT-MSEC:%d", this->m_iCS_REMAIN_MSEC);
	}

	if( this->m_iCS_REMAIN_MSEC <= 0 )
	{
		this->SetState(CASTLESIEGE_STATE_REGMARK, TRUE);
	}
}

void CCastleSiege::ProcState_REGMARK()
{
	int iTICK_MSEC = ( GetTickCount() - this->m_iCS_TICK_COUNT );

	if( iTICK_MSEC >= 1000 )
	{
		this->m_iCS_REMAIN_MSEC = (int)(difftime(this->m_iCS_TARGET_MSEC,time(0))*1000);
		this->m_iCS_TICK_COUNT = GetTickCount();
		
		if( (GetTickCount() - this->m_dwEVENT_MSG_TICK_COUNT) > 1800000 )
		{
			this->SendAllUserAnyMsg(gMessage.GetMessage(418), 2);
			this->m_dwEVENT_MSG_TICK_COUNT = GetTickCount();
		}
		//LogAdd(LOG_GREEN, "RUN ProcState_REGMARK()\tLEFT-MSEC:%d", this->m_iCS_REMAIN_MSEC);
	}

	if( this->m_iCS_REMAIN_MSEC <= 0 )
	{
		this->SetState(CASTLESIEGE_STATE_IDLE_3, TRUE);
	}
}

void CCastleSiege::ProcState_IDLE_3()
{
	int iTICK_MSEC = ( GetTickCount() - this->m_iCS_TICK_COUNT );

	if( iTICK_MSEC >= 1000 )
	{
		this->m_iCS_REMAIN_MSEC = (int)(difftime(this->m_iCS_TARGET_MSEC,time(0))*1000);
		this->m_iCS_TICK_COUNT = GetTickCount();
		
		if( (GetTickCount() - this->m_dwEVENT_MSG_TICK_COUNT) > 7200000 )
		{
			this->SendAllUserAnyMsg(gMessage.GetMessage(419), 2);
			this->m_dwEVENT_MSG_TICK_COUNT = GetTickCount();
		}
		//LogAdd(LOG_GREEN, "RUN ProcState_IDLE_3()\tLEFT-MSEC:%d", this->m_iCS_REMAIN_MSEC);
	}

	if( this->m_iCS_REMAIN_MSEC <= 0 )
	{
		this->SetState(CASTLESIEGE_STATE_NOTIFY, TRUE);
	}
}

void CCastleSiege::ProcState_NOTIFY()
{
	int iTICK_MSEC = ( GetTickCount() - this->m_iCS_TICK_COUNT );

	if( iTICK_MSEC >= 1000 )
	{
		this->m_iCS_REMAIN_MSEC = (int)(difftime(this->m_iCS_TARGET_MSEC,time(0))*1000);
		this->m_iCS_TICK_COUNT = GetTickCount();
		
		if( (GetTickCount() - this->m_dwEVENT_MSG_TICK_COUNT) > 1800000 )
		{
			this->SendAllUserAnyMsg(gMessage.GetMessage(420), 2);
			this->m_dwEVENT_MSG_TICK_COUNT = GetTickCount();
		}
		//LogAdd(LOG_GREEN, "RUN ProcState_NOTIFY()\tLEFT-MSEC:%d", this->m_iCS_REMAIN_MSEC);
	}

	if( this->m_iCS_REMAIN_MSEC <= 0 )
	{
		this->SetState(CASTLESIEGE_STATE_READYSIEGE, TRUE);
	}
}

void CCastleSiege::ProcState_READYSIEGE()
{
	int iTICK_MSEC = ( GetTickCount() - this->m_iCS_TICK_COUNT );

	if( iTICK_MSEC >= 1000 )
	{
		this->m_iCS_REMAIN_MSEC = (int)(difftime(this->m_iCS_TARGET_MSEC,time(0))*1000);
		this->m_iCS_TICK_COUNT = GetTickCount();
		
		if( (GetTickCount() - this->m_dwEVENT_MSG_TICK_COUNT) > 1800000 )
		{
			this->SendAllUserAnyMsg(gMessage.GetMessage(421), 2);
			this->m_dwEVENT_MSG_TICK_COUNT = GetTickCount();
		}
		if( this->m_iCS_REMAIN_MSEC <= 300000 )
		{
			if( ( GetTickCount() - this->m_dwCS_STARTTIME_TICK_COUNT ) > 60000 )
			{
				char szBuff[256]={0};
				wsprintf(szBuff, gMessage.GetMessage(422), (this->m_iCS_REMAIN_MSEC/60000)+1);
				this->SendAllUserAnyMsg(szBuff, 2);
				this->m_dwCS_STARTTIME_TICK_COUNT = GetTickCount();
			}
		}
		//LogAdd(LOG_GREEN, "RUN ProcState_READYSIEGE()\tLEFT-MSEC:%d", this->m_iCS_REMAIN_MSEC);
	}

	if( this->m_iCS_REMAIN_MSEC <= 0 )
	{
		this->SetState(CASTLESIEGE_STATE_STARTSIEGE, TRUE);
	}
}

void CCastleSiege::ProcState_STARTSIEGE()
{
	this->CheckMiddleWinnerGuild();

	int iTICK_MSEC = ( GetTickCount() - this->m_iCS_TICK_COUNT );

	if( iTICK_MSEC >= 1000 )
	{
		this->m_iCS_REMAIN_MSEC = (int)(difftime(this->m_iCS_TARGET_MSEC,time(0))*1000);
		this->m_iCS_TICK_COUNT = GetTickCount();
		
		if( (GetTickCount() - this->m_dwCHECK_GATE_ALIVE_COUNT) > 5000 )
		{
			this->CheckCsDbNpcAlive();
			this->m_dwCHECK_GATE_ALIVE_COUNT = GetTickCount();
		}

		if( (GetTickCount() - this->m_dwCS_LEFTTIME_TICK_COUNT) > 180000 )
		{
			if( this->m_iCS_REMAIN_MSEC > 180000 )
			{
				GCAnsCsLeftTimeAlarm( (this->m_iCS_REMAIN_MSEC/1000)/3600, ((this->m_iCS_REMAIN_MSEC/1000)%3600)/60);
				this->m_dwCS_LEFTTIME_TICK_COUNT = GetTickCount();
			}
		}

		if( this->m_iCS_REMAIN_MSEC <= 180000 )
		{
			if( (GetTickCount() - this->m_dwCS_LEFTTIME_TICK_COUNT) > 60000 )
			{
				GCAnsCsLeftTimeAlarm( ((this->m_iCS_REMAIN_MSEC/1000)/3600), ((this->m_iCS_REMAIN_MSEC/1000)%3600)/60);
				this->m_dwCS_LEFTTIME_TICK_COUNT = GetTickCount();
			}
		}

		if( this->m_dwCS_JOINSIDE_REFRESH_TOTAL_COUNT < 3 )
		{
			if( (GetTickCount() - this->m_dwCS_JOINSIDE_REFRESH_TICK_COUNT) > 10000 )
			{
				this->SetAllUserCsJoinSide();
				this->m_dwCS_JOINSIDE_REFRESH_TICK_COUNT = GetTickCount();
				this->m_dwCS_JOINSIDE_REFRESH_TOTAL_COUNT++;
			}
		}

		if( (GetTickCount() - this->m_dwCS_MINIMAP_SEND_TICK_COUNT) > 3000 )
		{
			this->OperateMiniMapWork();
			this->m_dwCS_MINIMAP_SEND_TICK_COUNT = GetTickCount();
		}

		if( (GetTickCount() - this->m_dwEVENT_MSG_TICK_COUNT) > 1800000 )
		{
			this->SendAllUserAnyMsg(gMessage.GetMessage(423), 2);
			this->m_dwEVENT_MSG_TICK_COUNT = GetTickCount();
		}

		//LogAdd(LOG_GREEN, "RUN ProcState_STARTSIEGE()\tLEFT-MSEC:%d", this->m_iCS_REMAIN_MSEC);
	}
	
	if( this->m_iCS_REMAIN_MSEC <= 0 )
	{
		if( this->CheckCastleSiegeResult() == TRUE )
		{
			GS_GDReqResetCastleTaxInfo(this->m_iMapSvrGroup);
		}

		this->ReSpawnEnemyUser(FALSE);

		if( this->m_btIsCastleOccupied == 1)
		{
			GS_GDReqCastleOwnerChange(this->m_iMapSvrGroup, this->m_btIsCastleOccupied, this->m_szCastleOwnerGuild);
		}
		
		GS_GDReqSiegeEndedChange(this->m_iMapSvrGroup, TRUE);
		
		this->StoreDbNpc();
		this->NotifyAllUserCsStartState(0);
		
		GS_GDReqResetSiegeGuildInfo(this->m_iMapSvrGroup);
		
		this->SetState(CASTLESIEGE_STATE_ENDSIEGE, TRUE);
	}
}

void CCastleSiege::ProcState_ENDSIEGE()
{
	int iTICK_MSEC = (GetTickCount() - this->m_iCS_TICK_COUNT);

	if( iTICK_MSEC >= 1000 )
	{
		this->m_iCS_REMAIN_MSEC = (int)(difftime(this->m_iCS_TARGET_MSEC,time(0))*1000);
		this->m_iCS_TICK_COUNT = GetTickCount();

		//LogAdd(LOG_GREEN, "RUN ProcState_ENDSIEGE()\tLEFT-MSEC:%d", this->m_iCS_REMAIN_MSEC);
	}

	if( this->m_iCS_REMAIN_MSEC <= 0 )
	{
		this->SetState(CASTLESIEGE_STATE_ENDCYCLE, TRUE);
	}
}

void CCastleSiege::ProcState_ENDCYCLE()
{
	int iTICK_MSEC = (GetTickCount() - this->m_iCS_TICK_COUNT);

	if( iTICK_MSEC >= 1000 )
	{
		this->m_iCS_REMAIN_MSEC = (int)(difftime(this->m_iCS_TARGET_MSEC,time(0))*1000);
		this->m_iCS_TICK_COUNT = GetTickCount();

		//LogAdd(LOG_GREEN, "RUN ProcState_ENDCYCLE()\tLEFT-MSEC:%d", this->m_iCS_REMAIN_MSEC);
	}

	if( this->m_iCS_REMAIN_MSEC <= 0 )
	{
		this->SetState(CASTLESIEGE_STATE_IDLE_1, TRUE);
	}
}

BOOL CCastleSiege::FirstCreateDbNPC()
{
	char cBUFFER[4240];

	CSP_REQ_NPCSAVEDATA* lpMsg;
	CSP_NPCSAVEDATA* lpMsgBody;
	
	if( this->m_iCastleDataLoadState != CASTLESIEGE_DATALOAD_4 )
	{
		LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::FirstCreateDbNPC() - m_iCastleDataLoadState != CASTLESIEGE_DATALOAD_4");
		return FALSE;
	}
	
	lpMsg = (CSP_REQ_NPCSAVEDATA*)cBUFFER;
	lpMsgBody = (CSP_NPCSAVEDATA*)&cBUFFER[sizeof(CSP_REQ_NPCSAVEDATA)];
	int iCOUNT = 0;

	EnterCriticalSection(&this->m_critNpcData);
	
	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();
	BOOL bExist = FALSE;

	while( it != this->m_vtNpcData.end() )
	{
		_CS_NPC_DATA & pNpcData = _CS_NPC_DATA(*it);

		if( pNpcData.m_bNPC_DBSAVE == TRUE )
		{
			it->SetBaseValue();
			it->m_iNPC_LIVE = 1;
			it->m_iNPC_SIDE = 1;

			lpMsgBody[iCOUNT].iNpcNumber = pNpcData.m_iNPC_NUM;
			lpMsgBody[iCOUNT].iNpcIndex = pNpcData.m_iNPC_INDEX;
			lpMsgBody[iCOUNT].iNpcDfLevel = pNpcData.m_iNPC_BASE_DF_LEVEL;
			lpMsgBody[iCOUNT].iNpcRgLevel = pNpcData.m_iNPC_BASE_RG_LEVEL;
			lpMsgBody[iCOUNT].iNpcMaxHp = pNpcData.m_iNPC_BASE_MAXHP;
			lpMsgBody[iCOUNT].iNpcHp = pNpcData.m_iNPC_BASE_HP;
			lpMsgBody[iCOUNT].btNpcX = pNpcData.m_iNPC_BASE_SX;
			lpMsgBody[iCOUNT].btNpcY = pNpcData.m_iNPC_BASE_SY;
			lpMsgBody[iCOUNT].btNpcDIR = pNpcData.m_iNPC_BASE_DIR;
			iCOUNT++;
		}
		it++;
	}

	LeaveCriticalSection(&this->m_critNpcData);

	lpMsg->wMapSvrNum = this->m_iMapSvrGroup;
	lpMsg->iCount = iCOUNT;
	lpMsg->h.set(0x84, (iCOUNT * sizeof(CSP_NPCSAVEDATA)) + sizeof(CSP_REQ_NPCSAVEDATA));
	gDataServerConnection.DataSend((BYTE*)lpMsg, (iCOUNT * sizeof(CSP_NPCSAVEDATA)) + sizeof(CSP_REQ_NPCSAVEDATA));

	return TRUE;
}

BOOL CCastleSiege::SetCastleInitData(CSP_ANS_CSINITDATA* lpMsg)
{
	if( this->m_iCastleDataLoadState != CASTLESIEGE_DATALOAD_3 )
	{
		LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::SetCastleInitData() - m_iCastleDataLoadState != CASTLESIEGE_DATALOAD_3");
		return FALSE;
	}

	if( lpMsg == NULL )
	{
		return FALSE;
	}

	memset(&this->m_tmStartDate, 0, sizeof(this->m_tmStartDate));
	memset(&this->m_tmEndDate, 0, sizeof(this->m_tmEndDate));
	
	this->m_tmStartDate.wYear = lpMsg->wStartYear;
	this->m_tmStartDate.wMonth = lpMsg->btStartMonth;
	this->m_tmStartDate.wDay = lpMsg->btStartDay;
	this->m_tmEndDate.wYear = lpMsg->wEndYear;
	this->m_tmEndDate.wMonth = lpMsg->btEndMonth;
	this->m_tmEndDate.wDay = lpMsg->btEndDay;
	this->m_btIsSiegeGuildList = lpMsg->btIsSiegeGuildList;
	this->m_btIsSiegeEnded = lpMsg->btIsSiegeEnded;
	this->m_btIsCastleOccupied = lpMsg->btIsCastleOccupied;

	memset(&this->m_szCastleOwnerGuild, 0, sizeof(this->m_szCastleOwnerGuild));
	memcpy(&this->m_szCastleOwnerGuild, lpMsg->szCastleOwnGuild, sizeof(lpMsg->szCastleOwnGuild));
	
	this->m_i64CastleMoney = lpMsg->i64CastleMoney;
	this->m_iTaxRateChaos = lpMsg->iTaxRateChaos;
	this->m_iTaxRateStore = lpMsg->iTaxRateStore;
	this->m_iTaxHuntZone = lpMsg->iTaxHuntZone;

	int iSTART_DATE_NUM = MACRO2(( MACRO1(this->m_tmStartDate.wDay) | MACRO1(this->m_tmStartDate.wMonth) << 8 ) & 0xFFFF ) | MACRO2(this->m_tmStartDate.wYear) << 16;
	int iEND_DATE_NUM = MACRO2(( MACRO1(this->m_tmEndDate.wDay) | MACRO1(this->m_tmEndDate.wMonth) << 8 ) & 0xFFFF ) | MACRO2(this->m_tmEndDate.wYear) << 16;
	
	this->m_tmSiegeEndSchedule = this->m_tmStartDate;

	SYSTEMTIME EndTime;
	GetStateDate(CASTLESIEGE_STATE_ENDCYCLE, &EndTime);

	LogAdd(LOG_BLACK,"[CastleSiege] CCastleSiege::LoadData() - Siege Start Date (%d-%d-%d)", m_tmStartDate.wYear, m_tmStartDate.wMonth, m_tmStartDate.wDay);
	
	LogAdd(LOG_BLACK,"[CastleSiege] CCastleSiege::LoadData() - Siege Date (%d-%d-%d)", EndTime.wDay, EndTime.wHour, EndTime.wMinute);

	GetNextDay(&m_tmSiegeEndSchedule, EndTime.wDay, EndTime.wHour, EndTime.wMinute, 0);

	LogAdd(LOG_BLACK,"[CastleSiege] CCastleSiege::LoadData() - Siege End Date (%d-%d-%d(%d:%d:%d))", this->m_tmSiegeEndSchedule.wYear, this->m_tmSiegeEndSchedule.wMonth, this->m_tmSiegeEndSchedule.wDay, this->m_tmSiegeEndSchedule.wHour, this->m_tmSiegeEndSchedule.wMinute, this->m_tmSiegeEndSchedule.wSecond);

	if( iSTART_DATE_NUM > iEND_DATE_NUM )
	{
		LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::SetCastleInitData() - iSTART_DATE_NUM > iEND_DATE_NUM");
		return FALSE;
	}
	return TRUE;
}

BOOL CCastleSiege::SetCastleNpcData(CSP_CSINITDATA* lpMsg, int iCOUNT)
{
	if( this->m_iCastleDataLoadState != CASTLESIEGE_DATALOAD_3 )
	{
		LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::SetCastleNpcData() - m_iCastleDataLoadState != CASTLESIEGE_DATALOAD_3");
		return FALSE;
	}

	if( lpMsg == NULL )
	{
		return FALSE;
	}

	if( iCOUNT < 0 )
	{
		return FALSE;
	}

	std::vector<_CS_NPC_DATA>::iterator it;

	for( int iNPC_COUNT = 0; iNPC_COUNT < iCOUNT; iNPC_COUNT++ )
	{
		EnterCriticalSection(&this->m_critNpcData);

		it = this->m_vtNpcData.begin();

		BOOL bExist = FALSE;

		while( it != this->m_vtNpcData.end() )
		{
			_CS_NPC_DATA & NpcData = _CS_NPC_DATA(*it);

			if( NpcData.m_iNPC_NUM == lpMsg[iNPC_COUNT].iNpcNumber )
			{
				if( NpcData.m_iNPC_INDEX == lpMsg[iNPC_COUNT].iNpcIndex )
				{
					it->m_iNPC_LIVE	= 1;
					it->m_iNPC_DF_LEVEL = lpMsg[iNPC_COUNT].iNpcDfLevel;
					it->m_iNPC_RG_LEVEL = lpMsg[iNPC_COUNT].iNpcRgLevel;
					it->m_iNPC_MAXHP = lpMsg[iNPC_COUNT].iNpcMaxHp;
					it->m_iNPC_HP = lpMsg[iNPC_COUNT].iNpcHp;
					it->m_iNPC_SX = lpMsg[iNPC_COUNT].btNpcX;
					it->m_iNPC_SY = lpMsg[iNPC_COUNT].btNpcY;
					it->m_iNPC_DX = -1;
					it->m_iNPC_DY = -1;
					it->m_iNPC_DIR = lpMsg[iNPC_COUNT].btNpcDIR;
					it->m_iNPC_SIDE = 1;
					bExist = TRUE;
					break;
				}
			}
			it++;
		}
		
		LeaveCriticalSection(&this->m_critNpcData);

		if( bExist == FALSE )
		{
			_CS_NPC_DATA pNpcData2;
			pNpcData2.m_bIN_USE	= TRUE;
			pNpcData2.m_iNPC_NUM = lpMsg[iNPC_COUNT].iNpcNumber;
			pNpcData2.m_iNPC_INDEX = lpMsg[iNPC_COUNT].iNpcIndex;
			pNpcData2.m_iNPC_LIVE = 1;
			pNpcData2.m_iNPC_SIDE = 1;
			pNpcData2.m_iNPC_BASE_DF_LEVEL = lpMsg[iNPC_COUNT].iNpcDfLevel;
			pNpcData2.m_iNPC_DF_LEVEL = pNpcData2.m_iNPC_BASE_DF_LEVEL;
			pNpcData2.m_iNPC_BASE_RG_LEVEL = lpMsg[iNPC_COUNT].iNpcRgLevel;
			pNpcData2.m_iNPC_RG_LEVEL = pNpcData2.m_iNPC_BASE_RG_LEVEL;

			pNpcData2.m_iNPC_BASE_MAXHP = lpMsg[iNPC_COUNT].iNpcMaxHp;
			pNpcData2.m_iNPC_MAXHP = pNpcData2.m_iNPC_BASE_MAXHP;
			pNpcData2.m_iNPC_BASE_HP = lpMsg[iNPC_COUNT].iNpcHp;
			pNpcData2.m_iNPC_HP = pNpcData2.m_iNPC_BASE_HP;
			pNpcData2.m_iNPC_BASE_SX = lpMsg[iNPC_COUNT].btNpcX;
			pNpcData2.m_iNPC_SX	= pNpcData2.m_iNPC_BASE_SX;
			pNpcData2.m_iNPC_BASE_SY = lpMsg[iNPC_COUNT].btNpcY;
			pNpcData2.m_iNPC_SY	= pNpcData2.m_iNPC_BASE_SY;
			pNpcData2.m_iNPC_BASE_DX = -1;
			pNpcData2.m_iNPC_DX	= pNpcData2.m_iNPC_BASE_DX;
			pNpcData2.m_iNPC_BASE_DY = -1;
			pNpcData2.m_iNPC_DY	= pNpcData2.m_iNPC_BASE_DY;
			pNpcData2.m_iNPC_BASE_DIR = lpMsg[iNPC_COUNT].btNpcDIR;
			pNpcData2.m_iNPC_DIR = pNpcData2.m_iNPC_BASE_DIR;

			EnterCriticalSection(&this->m_critNpcData);
			this->m_vtNpcData.push_back(pNpcData2);
			LeaveCriticalSection(&this->m_critNpcData);
		}
	}
	return TRUE;
}

int CCastleSiege::CheckSync()
{
	if(this->m_bFileDataLoadOK == FALSE)
	{
		LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::CheckSync() - m_bFileDataLoadOK == FALSE");
		return false;
	}

	this->m_iCS_TICK_COUNT = GetTickCount();

	
	_SYSTEMTIME tmToDay;

	GetLocalTime(&tmToDay);
	int iTODAY_DATE_NUM = MACRO2(( MACRO1(tmToDay.wDay) | MACRO1(tmToDay.wMonth) << 8 ) & 0xFFFF ) | MACRO2(tmToDay.wYear) << 16;
	int iCastleInitState = 0;

	if(this->m_bFixCastleCycleStartDate != FALSE)
	{
		this->m_tmStartDate.wYear = this->m_tmFixCastleCycleStartDate.wYear;
		this->m_tmStartDate.wMonth = this->m_tmFixCastleCycleStartDate.wMonth;
		this->m_tmStartDate.wDay = this->m_tmFixCastleCycleStartDate.wDay;

		this->m_tmEndDate = this->m_tmStartDate;
		GetNextDay(&this->m_tmEndDate, this->m_iCastleSiegeCycle, 0, 0, 0);
	}

	int iEVENT_END_DATE_NUM = MACRO2(( MACRO1(m_tmEndDate.wDay) | MACRO1(m_tmEndDate.wMonth) << 8 ) & 0xFFFF ) | MACRO2(m_tmEndDate.wYear) << 16;

	if(iEVENT_END_DATE_NUM <= iTODAY_DATE_NUM)
	{
		this->m_bFixCastleCycleStartDate = FALSE;

		//MessageBox(0,"teste","Error",MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
		//ErrorMessageBox("[CastleSiege] CCastleSiege::CheckSync() - iEVENT_END_DATE_NUM (%04d-%02d-%02d) <= iTODAY_DATE_NUM (%04d-%02d-%02d)",this->m_tmEndDate.wYear,this->m_tmEndDate.wMonth,this->m_tmEndDate.wDay,tmToDay.wYear,tmToDay.wMonth,tmToDay.wDay);
		return false;
	}

	 if(this->m_bFixCastleCycleStartDate != FALSE)
	 {
		 GS_GDReqSiegeDateChange(this->m_iMapSvrGroup, -1, this->m_tmStartDate.wYear, (BYTE)this->m_tmStartDate.wMonth, (BYTE)this->m_tmStartDate.wDay, this->m_tmEndDate.wYear, (BYTE)this->m_tmEndDate.wMonth, (BYTE)this->m_tmEndDate.wDay);
	 }

	 if(this->m_bFixCastleStateStartDate != FALSE)
	 {
		 BOOL bSpecificStateExist = FALSE;

		 EnterCriticalSection(&this->m_critScheduleData);

		 std::vector<_CS_SCHEDULE_DATA>::iterator it = this->m_vtScheduleData.begin();

		 while(it != this->m_vtScheduleData.end())
		 {
			 _CS_SCHEDULE_DATA & pScheduleData = (_CS_SCHEDULE_DATA &)*it;

			 if(pScheduleData.m_iSTATE == this->m_iFixCastleSpecificState)
			 {
				 bSpecificStateExist = TRUE;
				 break;
			 }

			 it++;
		 }

		 LeaveCriticalSection(&this->m_critScheduleData);

		 if(bSpecificStateExist != FALSE)
		 {
			 time_t ttSTime;
			 time_t ttETime;
			 tm tmETime;
			 double dResultSecond;
			 memset(&tmETime,0,sizeof(tmETime));

			 tmETime.tm_year = this->m_tmFixCastleStateStartDate.wYear - 1900;
			 tmETime.tm_mon = this->m_tmFixCastleStateStartDate.wMonth - 1;
			 tmETime.tm_mday = this->m_tmFixCastleStateStartDate.wDay;
			 tmETime.tm_hour = this->m_tmFixCastleStateStartDate.wHour;
			 tmETime.tm_min = this->m_tmFixCastleStateStartDate.wMinute;

			 time(&ttSTime);
			 ttETime = mktime(&tmETime);

			 dResultSecond = difftime(ttETime,ttSTime);

			 this->m_iCS_REMAIN_MSEC = (int)(dResultSecond * (double)1000.0);
			 this->m_iCS_TARGET_MSEC = (int)(time(0)+(this->m_iCS_REMAIN_MSEC/1000));
			 this->m_iCastleSiegeState = this->m_iFixCastleSpecificState-1;
			 this->SetState(this->m_iCastleSiegeState, FALSE);
			 return true;
		 }
		 else
		 {
			 ErrorMessageBox("[CastleSiege] CCastleSiege::CheckSync() - FIXED STATE:%d NOT FOUND IN SCHEDULE (P.S.> Check File 'MuCastleData.dat', 'commonserver.cfg')", this->m_iFixCastleSpecificState);
			 this->m_bFixCastleStateStartDate = FALSE;
		 }
	 }

	 if(this->m_btIsSiegeEnded != 0)
	 {
		 this->SetState(CASTLESIEGE_STATE_ENDSIEGE,TRUE);
		 return true;
	 }

	 int iEVENT_START_DATE_NUM = MACRO2(( MACRO1(this->m_tmStartDate.wDay) | MACRO1(this->m_tmStartDate.wMonth) << 8 ) & 0xFFFF ) | MACRO2(this->m_tmStartDate.wYear) << 16;

	 if(iEVENT_START_DATE_NUM > iTODAY_DATE_NUM)
	 {
		 this->SetState(CASTLESIEGE_STATE_IDLE_1,TRUE);
		 return true;
	 }
	 else
	 {
		 BOOL bStateSetted = FALSE;
		_CS_SCHEDULE_DATA pScheData_FR;
		_CS_SCHEDULE_DATA pScheData_RR;
		std::vector<_CS_SCHEDULE_DATA>::iterator it;

		 EnterCriticalSection(&this->m_critScheduleData);

		 it = this->m_vtScheduleData.begin();

		 while(it != this->m_vtScheduleData.end())
		 {
			_CS_SCHEDULE_DATA & pScheduleData = (_CS_SCHEDULE_DATA &)*it;;
			_SYSTEMTIME tmSchduleDate;

			tmSchduleDate = this->m_tmStartDate;
			GetNextDay(&tmSchduleDate,pScheduleData.m_iADD_DAY,0,0,0);
			tmSchduleDate.wHour = pScheduleData.m_iADD_HOUR;
			tmSchduleDate.wMinute = pScheduleData.m_iADD_MIN;

			__int64 i64ScheduleDateNum;
			__int64 i64ToDayDateNum;

			i64ScheduleDateNum = (MACRO2(tmSchduleDate.wMinute) | MACRO2(tmSchduleDate.wHour) << 16) | ((__int64)(MACRO2(( MACRO1(tmSchduleDate.wDay) | MACRO1(tmSchduleDate.wMonth) << 8 ) & 0xFFFF ) | MACRO2(tmSchduleDate.wYear) << 16) << 0x20);
			i64ToDayDateNum = (MACRO2(tmToDay.wMinute) | MACRO2(tmToDay.wHour) << 16) | ((__int64)(MACRO2(( MACRO1(tmToDay.wDay) | MACRO1(tmToDay.wMonth) << 8 ) & 0xFFFF ) | MACRO2(tmToDay.wYear) << 16) << 0x20);

			 if(i64ScheduleDateNum > i64ToDayDateNum)
			 {
				 bStateSetted = TRUE;
				 pScheData_RR = pScheduleData;
				 this->m_iCastleSiegeState = iCastleInitState;
				 break;
			 }

			 iCastleInitState = pScheduleData.m_iSTATE;
			 pScheData_FR = pScheduleData;

			 it++;
		 }

		 LeaveCriticalSection(&this->m_critScheduleData);

		 _SYSTEMTIME tmSchduleDate;
	
		time_t ttSTime;
		time_t ttETime;
		struct tm tmETime;
		double dResultSecond;

		if(bStateSetted == FALSE)
		{
			ErrorMessageBox("[CastleSiege] CCastleSiege::CheckSync() - bStateSetted == FALSE  START_DATE (%04d-%02d-%02d), END_DATE (%04d-%02d-%02d)", this->m_tmStartDate.wYear, this->m_tmStartDate.wMonth, this->m_tmStartDate.wDay, this->m_tmEndDate.wYear, this->m_tmEndDate.wMonth, this->m_tmEndDate.wDay);
			return false;
		}

		tmSchduleDate = this->m_tmStartDate;
		GetNextDay(&tmSchduleDate,pScheData_RR.m_iADD_DAY,0,0,0);
		tmSchduleDate.wHour = pScheData_RR.m_iADD_HOUR;
		tmSchduleDate.wMinute = pScheData_RR.m_iADD_MIN;

		memset(&tmETime,0,sizeof(tmETime));

		tmETime.tm_year = tmSchduleDate.wYear - 1900;
		tmETime.tm_mon = tmSchduleDate.wMonth - 1;
		tmETime.tm_mday = tmSchduleDate.wDay;
		tmETime.tm_hour = tmSchduleDate.wHour;
		tmETime.tm_min = tmSchduleDate.wMinute;

		time(&ttSTime);
		ttETime = mktime(&tmETime);

		dResultSecond = difftime(ttETime,ttSTime);
		this->m_iCS_REMAIN_MSEC = (int)(dResultSecond * (double)1000.0);
		this->m_iCS_TARGET_MSEC = (int)(time(0)+(this->m_iCS_REMAIN_MSEC/1000));

		if(this->m_iCastleSiegeState == CASTLESIEGE_STATE_STARTSIEGE)
		{
			this->m_iCastleSiegeState = CASTLESIEGE_STATE_READYSIEGE;
			this->m_iCS_REMAIN_MSEC = 900000;
			this->m_iCS_TARGET_MSEC = (int)(time(0)+(this->m_iCS_REMAIN_MSEC/1000));
		}
		else if(this->m_iCastleSiegeState == CASTLESIEGE_STATE_READYSIEGE)
		{
			if(m_iCS_REMAIN_MSEC < 900000)
			{
				this->m_iCastleSiegeState = CASTLESIEGE_STATE_READYSIEGE;
				this->m_iCS_REMAIN_MSEC = 900000;
				this->m_iCS_TARGET_MSEC = (int)(time(0)+(this->m_iCS_REMAIN_MSEC/1000));
			}
		}

		this->SetState(this->m_iCastleSiegeState,FALSE);
	 }
	return true;
}

void CCastleSiege::GetNextDay(_SYSTEMTIME * st,int iAfterDay,int iAfterHour,int iAfterMin,int iAfterSec)
{
	__int64 i64;
	_FILETIME fst;
	SystemTimeToFileTime(st,&fst);
	i64 = ((__int64)fst.dwHighDateTime << 0x20) + fst.dwLowDateTime;
	i64 = i64 + (__int64)iAfterDay * 864000000000 + (__int64)iAfterHour * 36000000000 +(__int64) iAfterMin * 600000000 + (__int64)iAfterSec * 10000000;
	fst.dwHighDateTime = i64 >> 0x20;
	fst.dwLowDateTime = i64 & 0xFFFFFFFF;
	FileTimeToSystemTime(&fst,st);
}

void CCastleSiege::GetStateDate(int iCastleSiegeState, SYSTEMTIME* st)
{
	EnterCriticalSection(&this->m_critScheduleData);

	for( std::vector<_CS_SCHEDULE_DATA>::iterator it = this->m_vtScheduleData.begin(); it != this->m_vtScheduleData.end(); it++ )
	{
		_CS_SCHEDULE_DATA & pScheduleData = *it;

		if( pScheduleData.m_iSTATE == iCastleSiegeState )
		{
			st->wDay = pScheduleData.m_iADD_DAY;
			st->wHour = pScheduleData.m_iADD_HOUR;
			st->wMinute = pScheduleData.m_iADD_MIN;
			st->wSecond = 0;
			break;
		}
	}
	LeaveCriticalSection(&this->m_critScheduleData);
}

int CCastleSiege::GetStateGapSec(int iCastleSiegeState)
{
	int iGAP_SEC = -1;

	EnterCriticalSection(&this->m_critScheduleData);

	for(std::vector<_CS_SCHEDULE_DATA>::iterator it = this->m_vtScheduleData.begin(); it != this->m_vtScheduleData.end(); it++)
	{
		_CS_SCHEDULE_DATA & pScheduleData = *it;

		if(pScheduleData.m_iSTATE == iCastleSiegeState)
		{
			iGAP_SEC = pScheduleData.m_iGAP_SEC;
			break;
		}
	}

	LeaveCriticalSection(&this->m_critScheduleData);

	return iGAP_SEC;
}

int CCastleSiege::CalcCastleLeftSiegeDate()
{
	memset(&this->m_tmLeftCastleSiegeDate, 0, sizeof(this->m_tmLeftCastleSiegeDate));

	if( this->m_iCastleSiegeState < CASTLESIEGE_STATE_IDLE_1 || CASTLESIEGE_STATE_ENDCYCLE < this->m_iCastleSiegeState )
	{
		return CASTLESIEGE_LEFTTIME_NOTRUNNING;
	}

	if( this->m_bDoRun == FALSE )
	{
		return CASTLESIEGE_LEFTTIME_NOTRUNNING;
	}

	if( this->m_iCastleSiegeState == CASTLESIEGE_STATE_STARTSIEGE )
	{
		return CASTLESIEGE_LEFTTIME_ONSIGE;
	}

	if( this->m_btIsSiegeEnded == 1 || this->m_iCastleSiegeState >= CASTLESIEGE_STATE_ENDSIEGE )
	{
		return CASTLESIEGE_LEFTTIME_ENDSIEGE;
	}

	int iGAP_SEC = CASTLESIEGE_LEFTTIME_ONSIGE;
	BOOL bAddStart = FALSE;

	EnterCriticalSection(&this->m_critScheduleData);
	
	for( std::vector<_CS_SCHEDULE_DATA>::iterator it = this->m_vtScheduleData.begin(); it != this->m_vtScheduleData.end(); it++ )
	{
		_CS_SCHEDULE_DATA& lpScheduleData = *it;

		if( lpScheduleData.m_iSTATE == 7 )
		{
			break;
		}

		if( bAddStart )
		{
			iGAP_SEC += lpScheduleData.m_iGAP_SEC;
		}

		if( lpScheduleData.m_iSTATE == this->m_iCastleSiegeState )
		{
			iGAP_SEC = (this->m_iCS_REMAIN_MSEC / 1000);
			bAddStart = TRUE;
		}
	}

	LeaveCriticalSection(&this->m_critScheduleData);

	if( iGAP_SEC >= 0 )
	{
		SYSTEMTIME tmLocalTime;
		GetLocalTime(&tmLocalTime);

		this->GetNextDay(&tmLocalTime, 0, 0, 0, iGAP_SEC);

		this->m_tmLeftCastleSiegeDate = tmLocalTime;

		if( iGAP_SEC == 0 )
		{
			LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::CalcCastleLeftSiegeDate() - iGAP_SEC == 0");
		}

		return iGAP_SEC;
	}

	return CASTLESIEGE_LEFTTIME_NOTRUNNING;
}

BOOL CCastleSiege::GetCastleStateTerm(SYSTEMTIME * tmStateStartDate, SYSTEMTIME * tmStateEndDate)
{
	if( this->m_iCastleSiegeState < CASTLESIEGE_STATE_IDLE_1 || CASTLESIEGE_STATE_ENDCYCLE < this->m_iCastleSiegeState )
	{
		return FALSE;
	}

	if( this->m_bDoRun == FALSE )
	{
		return FALSE;
	}

	*tmStateStartDate = this->m_tmStateStartDate;

	GetLocalTime(tmStateEndDate);

	GetNextDay(tmStateEndDate, 0, 0, 0, this->m_iCS_REMAIN_MSEC / 1000);

	return TRUE;
}

int CCastleSiege::GetCurRemainSec()
{
	if( this->m_iCS_REMAIN_MSEC < 0 )
	{
		return -1;
	}

	return (this->m_iCS_REMAIN_MSEC / 1000);
}

void CCastleSiege::ClearDbNPC()
{
	for( int n = 0 ; n < MAX_OBJECT_MONSTER; n++ )
	{
		if( gObj[n].Map == MAP_CASTLE_SIEGE )
		{
			if( gObj[n].Class == 277 || gObj[n].Class == 283 || gObj[n].Class == 219 )
			{
				if( gObj[n].Class == 277 )
				{
					this->SetGateBlockState(gObj[n].X, gObj[n].Y, 1);
					gEffectManager.AddEffect(&gObj[n],0,EFFECT_CASTLE_GATE_STATE,0,0,0,0,0);
				}
				gObjDel(n);
			}
		}
	}
}

void CCastleSiege::ClearNonDbNPC()
{
	for(int n = 0; n < MAX_OBJECT_MONSTER; n++)
	{
		if(gObjIsConnected(n) && gObj[n].Map == MAP_CASTLE_SIEGE && gObj[n].CsNpcType != NULL && gObj[n].CsNpcType != 1)
		{
			if(gObj[n].Class == 278)
			{
				gLifeStone.DeleteLifeStone(n);
			}

			if(gObj[n].Class == 287 || gObj[n].Class == 286)
			{
				gMercenary.DeleteMercenary(n);
			}

			if(gObj[n].Class == 219)
			{
				continue;
			}

			gObjDel(n);
		}
	}
}

void CCastleSiege::ClearNonDbNPC_MidWin()
{
	for( int n = 0 ; n < MAX_OBJECT_MONSTER; n++ )
	{
		if( gObjIsConnected(n) )
		{
			if( gObj[n].Map == MAP_CASTLE_SIEGE )
			{
				if( gObj[n].CsNpcType && gObj[n].CsNpcType != 1 )
				{
					if( gObj[n].Class == 278 )
					{
						gLifeStone.DeleteLifeStone(n);
					}
					
					if( gObj[n].Class == 278 )
					{
						gObjDel(n);
					}
				}
			}
		}
	}
}

void CCastleSiege::ClearAllNPC()
{
	for(int n = 0; n < MAX_OBJECT_MONSTER; n++)
	{
		if(gObj[n].CsNpcType)
		{
			gObjDel(n);
		}
	}
}

void CCastleSiege::CreateDbNPC()
{
	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();
	BOOL bExist = FALSE;

	while(it != this->m_vtNpcData.end())
	{
		_CS_NPC_DATA & pNpcData = _CS_NPC_DATA(*it);

		if(pNpcData.m_bIN_USE == 1 && pNpcData.m_iNPC_LIVE == 1)
		{
			int iNPC_INDEX = gObjAddMonster(MAP_CASTLE_SIEGE);

			if(iNPC_INDEX >= 0)
			{
				gObjSetMonster(iNPC_INDEX, pNpcData.m_iNPC_NUM);
				gObj[iNPC_INDEX].PosNum = -1;
				gObj[iNPC_INDEX].X = pNpcData.m_iNPC_SX;
				gObj[iNPC_INDEX].Y = pNpcData.m_iNPC_SY;
				gObj[iNPC_INDEX].Map = MAP_CASTLE_SIEGE;
				gObj[iNPC_INDEX].TX = gObj[iNPC_INDEX].X;
				gObj[iNPC_INDEX].TY = gObj[iNPC_INDEX].Y;
				gObj[iNPC_INDEX].OldX = gObj[iNPC_INDEX].X;
				gObj[iNPC_INDEX].OldY = gObj[iNPC_INDEX].Y;
				gObj[iNPC_INDEX].Dir = pNpcData.m_iNPC_DIR;
				gObj[iNPC_INDEX].StartX = (BYTE)gObj[iNPC_INDEX].X;
				gObj[iNPC_INDEX].StartY = (BYTE)gObj[iNPC_INDEX].Y;
				gObj[iNPC_INDEX].DieRegen = 0;
				gObj[iNPC_INDEX].MaxRegenTime = 0;
				gObj[iNPC_INDEX].Life = (float)pNpcData.m_iNPC_HP;
				gObj[iNPC_INDEX].MaxLife = (float)pNpcData.m_iNPC_MAXHP;
				gObj[iNPC_INDEX].CsNpcType = 1;

				int iNPC_DF_LEVEL = pNpcData.m_iNPC_DF_LEVEL;

				if(iNPC_DF_LEVEL < 0)
				{
					iNPC_DF_LEVEL = 0;
				}

				if(iNPC_DF_LEVEL > CS_MAX_UPGRADE)
				{
					iNPC_DF_LEVEL = CS_MAX_UPGRADE;
				}

				int iNPC_RG_LEVEL = pNpcData.m_iNPC_RG_LEVEL;

				if(iNPC_RG_LEVEL < 0)
				{
					iNPC_RG_LEVEL = 0;
				}

				if(iNPC_RG_LEVEL > CS_MAX_UPGRADE)
				{
					iNPC_RG_LEVEL = CS_MAX_UPGRADE;
				}

				switch(pNpcData.m_iNPC_NUM)
				{
				case 277:
					gObj[iNPC_INDEX].Defense = g_iNpcDefense_CGATE[iNPC_DF_LEVEL];
					gObj[iNPC_INDEX].MagicDefense = g_iNpcDefense_CGATE[iNPC_DF_LEVEL];
					gObj[iNPC_INDEX].CsNpcDfLevel = iNPC_DF_LEVEL;
					break;
				case 283:
					gObj[iNPC_INDEX].Defense = g_iNpcDefense_CSTATUE[iNPC_DF_LEVEL];
					gObj[iNPC_INDEX].MagicDefense = g_iNpcDefense_CSTATUE[iNPC_DF_LEVEL];
					gObj[iNPC_INDEX].CsNpcDfLevel = iNPC_DF_LEVEL;
					gObj[iNPC_INDEX].CsNpcRgLevel = iNPC_RG_LEVEL;
					break;
				}

				it->m_btCsNpcExistVal1 = GetLargeRand()%255+1;
				gObj[iNPC_INDEX].CsNpcExistVal1 = it->m_btCsNpcExistVal1;

				it->m_btCsNpcExistVal2 = GetLargeRand()%256;
				gObj[iNPC_INDEX].CsNpcExistVal2 = it->m_btCsNpcExistVal2;

				it->m_btCsNpcExistVal3 = GetLargeRand()%256;
				gObj[iNPC_INDEX].CsNpcExistVal3 = it->m_btCsNpcExistVal3;

				it->m_btCsNpcExistVal4 = GetLargeRand()%256;
				gObj[iNPC_INDEX].CsNpcExistVal4 = it->m_btCsNpcExistVal4;

				if(pNpcData.m_iNPC_NUM == 277)
				{
					this->SetGateBlockState(pNpcData.m_iNPC_SX,pNpcData.m_iNPC_SY,1);
					gEffectManager.AddEffect(&gObj[iNPC_INDEX],0,EFFECT_CASTLE_GATE_STATE,0,0,0,0,0);

					gObj[iNPC_INDEX].CsGateOpen = 1;
					BOOL bCreateLever = FALSE;

					if(pNpcData.m_iCS_GATE_LEVER_INDEX == -1)
					{
						bCreateLever = TRUE;
					}
					else if(this->CheckLeverAlive(pNpcData.m_iCS_GATE_LEVER_INDEX) == FALSE)
					{
						bCreateLever = TRUE;
					}

					if(bCreateLever != FALSE)
					{
						int iLeverIndex = this->CreateCsGateLever(pNpcData.m_iNPC_SX,pNpcData.m_iNPC_SY+4);

						if(OBJECT_RANGE(iLeverIndex))
						{
							it->m_iCS_GATE_LEVER_INDEX = iLeverIndex;

							BOOL bRETVAL = this->LinkCsGateLever(iLeverIndex,iNPC_INDEX);

							if(bRETVAL == FALSE)
							{
								LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::CreateDbNPC() ERROR - Failed to Create Gate Lever : GateIndex:%d",pNpcData.m_iNPC_INDEX);
							}
						}
						else
						{
							it->m_iCS_GATE_LEVER_INDEX = -1;
							LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::CreateDbNPC() ERROR - Failed to Create Gate Lever : GateIndex:%d",pNpcData.m_iNPC_INDEX);
						}
					}
					else
					{
						BOOL bRETVAL = this->LinkCsGateLever(pNpcData.m_iCS_GATE_LEVER_INDEX,iNPC_INDEX);

						if(bRETVAL == FALSE)
						{
							LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::CreateDbNPC() ERROR - Failed to Create Gate Lever : GateIndex:%d",pNpcData.m_iNPC_INDEX);
						}
					}
				}

				it->m_iNPC_OBJINDEX = iNPC_INDEX;
				it->m_iNPC_LIVE = 2;
			}
		}

		it++;
	}
	LeaveCriticalSection(&this->m_critNpcData);
}

void CCastleSiege::CreateNonDbNPC(BOOL bDelFirst)
{
	if( bDelFirst )
	{
		this->ClearNonDbNPC();
	}

	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();

	while(it != this->m_vtNpcData.end())
	{
		_CS_NPC_DATA & pNpcData = _CS_NPC_DATA(*it);

		if(pNpcData.m_bIN_USE == TRUE && pNpcData.m_bNPC_DBSAVE == FALSE)
		{
			int iNPC_INDEX = gObjAddMonster(MAP_CASTLE_SIEGE);

			if(iNPC_INDEX >= 0)
			{
				gObjSetMonster(iNPC_INDEX, pNpcData.m_iNPC_NUM);
				gObj[iNPC_INDEX].PosNum = -1;
				gObj[iNPC_INDEX].X = pNpcData.m_iNPC_SX;
				gObj[iNPC_INDEX].Y = pNpcData.m_iNPC_SY;
				gObj[iNPC_INDEX].Map = MAP_CASTLE_SIEGE;
				gObj[iNPC_INDEX].TX = gObj[iNPC_INDEX].X;
				gObj[iNPC_INDEX].TY = gObj[iNPC_INDEX].Y;
				gObj[iNPC_INDEX].OldX = gObj[iNPC_INDEX].X;
				gObj[iNPC_INDEX].OldY = gObj[iNPC_INDEX].Y;
				gObj[iNPC_INDEX].Dir = pNpcData.m_iNPC_DIR;
				gObj[iNPC_INDEX].StartX = (BYTE)gObj[iNPC_INDEX].X;
				gObj[iNPC_INDEX].StartY = (BYTE)gObj[iNPC_INDEX].Y;
				gObj[iNPC_INDEX].DieRegen = 0;
				gObj[iNPC_INDEX].MaxRegenTime = 0;
				gObj[iNPC_INDEX].Life = (float)pNpcData.m_iNPC_HP;
				gObj[iNPC_INDEX].MaxLife = (float)pNpcData.m_iNPC_MAXHP;


				switch( pNpcData.m_iNPC_SIDE )
				{
				case 1:
					gObj[iNPC_INDEX].CsNpcType = 2;
					break;
				case 2:
					gObj[iNPC_INDEX].CsNpcType = 3;
					break;
				default:
					gObj[iNPC_INDEX].CsNpcType = 3;
					break;
				}

				it->m_btCsNpcExistVal1 = GetLargeRand()%255+1;
				gObj[iNPC_INDEX].CsNpcExistVal1 = it->m_btCsNpcExistVal1;

				it->m_btCsNpcExistVal2 = GetLargeRand()%256;
				gObj[iNPC_INDEX].CsNpcExistVal2 = it->m_btCsNpcExistVal2;

				it->m_btCsNpcExistVal3 = GetLargeRand()%256;
				gObj[iNPC_INDEX].CsNpcExistVal3 = it->m_btCsNpcExistVal3;

				it->m_btCsNpcExistVal4 = GetLargeRand()%256;
				gObj[iNPC_INDEX].CsNpcExistVal4 = it->m_btCsNpcExistVal4;

				it->m_iNPC_OBJINDEX = iNPC_INDEX;
				it->m_iNPC_LIVE = 2;
			}
		}
		it++;
	}
	LeaveCriticalSection(&this->m_critNpcData);
}

int CCastleSiege::CheckAddDbNPC(int iIndex, int iNpcType, int iNpcIndex, BYTE & btResult)
{
	int iRETVAL = 0;

	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();

	while( it != this->m_vtNpcData.end() )
	{
		_CS_NPC_DATA & NpcData = _CS_NPC_DATA(*it);

		if( NpcData.m_iNPC_NUM == iNpcType && NpcData.m_iNPC_INDEX == iNpcIndex )
		{
			if( NpcData.m_bIN_USE == TRUE && NpcData.m_iNPC_LIVE > 0 && gObjIsConnected(NpcData.m_iNPC_OBJINDEX) && gObj[NpcData.m_iNPC_OBJINDEX].Class == iNpcType )
			{
				if( gObj[NpcData.m_iNPC_OBJINDEX].Live == 1 && gObj[NpcData.m_iNPC_OBJINDEX].CsNpcExistVal == NpcData.m_iCsNpcExistVal && gObj[NpcData.m_iNPC_OBJINDEX].CsNpcExistVal != 0 )
				{
					btResult = 0;
					iRETVAL = 0;
					break;
				}
			}
			
			int iNpcBuyCost = 0;

			switch( NpcData.m_iNPC_NUM )
			{
			case 277:
				iNpcBuyCost = NpcData.m_iNPC_BASE_MAXHP * 5;
				break;
			case 283:
				iNpcBuyCost = NpcData.m_iNPC_BASE_MAXHP * 3;
				break;
			}

			if( gObj[iIndex].Money < ((DWORD)iNpcBuyCost) )
			{
				btResult = 3;
				iRETVAL = 0;
				break;
			}
			
			GS_GDReqCastleNpcBuy(m_iMapSvrGroup, iIndex, iNpcType, iNpcIndex, NpcData.m_iNPC_DF_LEVEL, NpcData.m_iNPC_RG_LEVEL, NpcData.m_iNPC_MAXHP, NpcData.m_iNPC_HP, NpcData.m_iNPC_SX, NpcData.m_iNPC_SY, NpcData.m_iNPC_DIR, iNpcBuyCost);
			btResult = 1;
			iRETVAL = 1;
			break;
		}
		it++;
	}

	LeaveCriticalSection(&this->m_critNpcData);

	return iRETVAL;
}

int CCastleSiege::AddDbNPC(int iNpcType, int iNpcIndex)
{
	int iRETVAL = 0;
	
	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();

	while( it != this->m_vtNpcData.end() )
	{
		_CS_NPC_DATA & NpcData = _CS_NPC_DATA(*it);

		if( (NpcData.m_iNPC_NUM == iNpcType) && (NpcData.m_iNPC_INDEX == iNpcIndex) )
		{
			if( gObjIsConnected(NpcData.m_iNPC_OBJINDEX) )
			{
				if( gObj[NpcData.m_iNPC_OBJINDEX].Class == iNpcType )
				{
					if( gObj[NpcData.m_iNPC_OBJINDEX].Live == TRUE )
					{
						if( gObj[NpcData.m_iNPC_OBJINDEX].CsNpcExistVal == NpcData.m_iCsNpcExistVal )
						{
							if( gObj[NpcData.m_iNPC_OBJINDEX].CsNpcExistVal )
							{
								LogAdd(LOG_BLACK,"[CastleSiege] CCastleSiege::AddDbNPC() ERROR - DB NPC EXIST (CLS:%d, IDX:%d(%d), MAXHP:%d, HP:%d, DF:%d, RG:%d)",	NpcData.m_iNPC_NUM, NpcData.m_iNPC_INDEX, NpcData.m_iNPC_OBJINDEX, gObj[NpcData.m_iNPC_OBJINDEX].MaxLife, gObj[NpcData.m_iNPC_OBJINDEX].Life, gObj[NpcData.m_iNPC_OBJINDEX].CsNpcDfLevel, gObj[NpcData.m_iNPC_OBJINDEX].CsNpcRgLevel);
								break;
							}
						}
					}
				}
			}

			int iObjIndex = gObjAddMonster(MAP_CASTLE_SIEGE);

			if( iObjIndex >= 0 )
			{
				gObjSetMonster(iObjIndex, NpcData.m_iNPC_NUM);

				gObj[iObjIndex].PosNum = -1;
				gObj[iObjIndex].X = NpcData.m_iNPC_SX;
				gObj[iObjIndex].Y = NpcData.m_iNPC_SY;
				gObj[iObjIndex].Map = MAP_CASTLE_SIEGE;
				gObj[iObjIndex].TX = gObj[iObjIndex].X;
				gObj[iObjIndex].TY = gObj[iObjIndex].Y;
				gObj[iObjIndex].OldX = gObj[iObjIndex].X;
				gObj[iObjIndex].OldY = gObj[iObjIndex].Y;
				gObj[iObjIndex].Dir = NpcData.m_iNPC_DIR;
				gObj[iObjIndex].StartX = (BYTE)gObj[iObjIndex].X;
				gObj[iObjIndex].StartY = (BYTE)gObj[iObjIndex].Y;
				gObj[iObjIndex].DieRegen = 0;
				gObj[iObjIndex].MaxRegenTime = 0;
				gObj[iObjIndex].Life = (float)NpcData.m_iNPC_BASE_HP;
				gObj[iObjIndex].MaxLife = (float)NpcData.m_iNPC_BASE_MAXHP;
				gObj[iObjIndex].CsNpcType = 1;

				int iBaseDFLevel = NpcData.m_iNPC_BASE_DF_LEVEL;

				if( iBaseDFLevel < 0 )
				{
					iBaseDFLevel = 0;
				}

				if( iBaseDFLevel > 3 )
				{
					iBaseDFLevel = 3;
				}

				int iBaseRGLevel = NpcData.m_iNPC_BASE_RG_LEVEL;
					
				if( iBaseRGLevel < 0 )
				{
					iBaseRGLevel = 0;
				}

				if( iBaseRGLevel > 3 )
				{
					iBaseRGLevel = 3;
				}
				

				switch( NpcData.m_iNPC_NUM )
				{
				case 277:
					{
						gObj[iObjIndex].Defense = g_iNpcDefense_CGATE[iBaseDFLevel];
						gObj[iObjIndex].CsNpcDfLevel = iBaseDFLevel;
					}
					break;
				case 283:
					{
						gObj[iObjIndex].Defense = g_iNpcDefense_CSTATUE[iBaseDFLevel];
						gObj[iObjIndex].CsNpcDfLevel = iBaseDFLevel;
						gObj[iObjIndex].CsNpcRgLevel = iBaseRGLevel;
					}
					break;
				}

				it->m_btCsNpcExistVal1 = ( GetLargeRand() % 255) + 1;
				gObj[iObjIndex].CsNpcExistVal1 = it->m_btCsNpcExistVal1;

				it->m_btCsNpcExistVal2 = ( GetLargeRand() % 256);
				gObj[iObjIndex].CsNpcExistVal2 = it->m_btCsNpcExistVal2;

				it->m_btCsNpcExistVal3 = ( GetLargeRand() % 256);
				gObj[iObjIndex].CsNpcExistVal3 = it->m_btCsNpcExistVal3;

				it->m_btCsNpcExistVal4 = ( GetLargeRand() % 256);
				gObj[iObjIndex].CsNpcExistVal4 = it->m_btCsNpcExistVal4;

				if( NpcData.m_iNPC_NUM == 277 )
				{
					this->SetGateBlockState(NpcData.m_iNPC_SX, NpcData.m_iNPC_SY, 1);
					gEffectManager.AddEffect(&gObj[iObjIndex],0,EFFECT_CASTLE_GATE_STATE,0,0,0,0,0);

					gObj[iObjIndex].CsGateOpen = 1;

					BOOL bCreateLever = FALSE;

					if( NpcData.m_iCS_GATE_LEVER_INDEX == -1 )
					{
						bCreateLever = TRUE;
					}
					else if( this->CheckLeverAlive(NpcData.m_iCS_GATE_LEVER_INDEX) == FALSE )
					{
						bCreateLever = TRUE;
					}

					if( bCreateLever != 0 )
					{
						int iLevelerIndex = this->CreateCsGateLever(NpcData.m_iNPC_SX, NpcData.m_iNPC_SY+4);

						if( OBJECT_RANGE(iLevelerIndex) )
						{
							it->m_iCS_GATE_LEVER_INDEX = iLevelerIndex;

							int iLinkIndex = this->LinkCsGateLever(iLevelerIndex, iObjIndex);

							if( iLinkIndex == 0 )
							{
								LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::CreateDbNPC() ERROR - Failed to Create Gate Lever : GateIndex:%d", NpcData.m_iNPC_INDEX);
							}
						}
						else
						{
							it->m_iCS_GATE_LEVER_INDEX = -1;
							LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::CreateDbNPC() ERROR - Failed to Create Gate Lever : GateIndex:%d", NpcData.m_iNPC_INDEX);
						}
					}
					else
					{
						int iLinkIndex = this->LinkCsGateLever(NpcData.m_iCS_GATE_LEVER_INDEX, iObjIndex);

						if( iLinkIndex == 0 )
						{
							LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::CreateDbNPC() ERROR - Failed to Create Gate Lever : GateIndex:%d", NpcData.m_iNPC_INDEX);
						}
					}
				}

				it->m_iNPC_OBJINDEX = iObjIndex;
				it->m_iNPC_LIVE = 2;
				it->m_bIN_USE = TRUE;

				iRETVAL = 1;

				if( NpcData.m_iNPC_NUM == 277 || NpcData.m_iNPC_NUM == 283 )
				{
					LogAdd(LOG_BLACK,"[CastleSiege][CreateDBNPC] (Index:%d, Num:%d, DF:%d, RG:%d, MaxHP:%d, HP:%d )", NpcData.m_iNPC_INDEX, NpcData.m_iNPC_NUM, NpcData.m_iNPC_DF_LEVEL, NpcData.m_iNPC_RG_LEVEL, NpcData.m_iNPC_MAXHP, NpcData.m_iNPC_HP);

				}
				break;
			}
		}

		it++;
	}
	LeaveCriticalSection(&this->m_critNpcData);

	return iRETVAL;
}

BOOL CCastleSiege::DelNPC(int iIndex, int iNpcType, int iMonsterExistVal, int bDbSave)
{
	if( !OBJECT_RANGE(iIndex) )
	{
		return FALSE;
	}

	BOOL bEXIST = FALSE;
	BOOL bNPC_DB = FALSE;

	_CS_NPC_DATA pTempNpcData;

	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();

	while( it != this->m_vtNpcData.end() )
	{
		_CS_NPC_DATA* pNpcData;
		_CS_NPC_DATA NpcData( *it );
		pNpcData = &NpcData;

		if( pNpcData->m_iNPC_NUM == iNpcType && pNpcData->m_iNPC_OBJINDEX == iIndex)
		{
			if( pNpcData->m_iCsNpcExistVal == iMonsterExistVal && pNpcData->m_bIN_USE == TRUE )
			{
				if( pNpcData->m_iNPC_LIVE > 0 )
				{
					it->SetBaseValue();

					it->m_iNPC_OBJINDEX = -1;
					it->m_iNPC_LIVE = 0;

					if( pNpcData->m_iNPC_NUM == 277 || pNpcData->m_iNPC_NUM == 283 )
					{
						pNpcData->m_iNPC_HP = pNpcData->m_iNPC_BASE_HP;
						pNpcData->m_iNPC_MAXHP = pNpcData->m_iNPC_BASE_MAXHP;
						pNpcData->m_iNPC_RG_LEVEL = pNpcData->m_iNPC_BASE_RG_LEVEL;
						pNpcData->m_iNPC_DF_LEVEL = pNpcData->m_iNPC_BASE_DF_LEVEL;
					}
					
					bEXIST = TRUE;
					pTempNpcData = *pNpcData;

					if( pNpcData->m_iNPC_NUM == 277 )
					{
						this->SetGateBlockState(pNpcData->m_iNPC_SX, pNpcData->m_iNPC_SY, 1);
						gEffectManager.AddEffect(&gObj[pNpcData->m_iNPC_OBJINDEX],0,EFFECT_CASTLE_GATE_STATE,0,0,0,0,0);
						LogAdd(LOG_BLACK,"[CastleSiege][ State:%d ] Delete Castle Gate ( CLS:%d, IDX:%d, X:%d. Y:%d )", this->m_iCastleSiegeState, pNpcData->m_iNPC_NUM, pNpcData->m_iNPC_INDEX, pNpcData->m_iNPC_SX, pNpcData->m_iNPC_SY);
					}

					if( pNpcData->m_iNPC_NUM == 283 )
					{
						if( this->m_iCastleSiegeState == CASTLESIEGE_STATE_STARTSIEGE )
						{
							if( this->CheckGuardianStatueExist() == FALSE )
							{
								this->ClearCastleTowerBarrier();
								this->SetCastleTowerAccessable(TRUE);
							}
							LogAdd(LOG_BLACK,"[CastleSiege] Defend Statue is Broken (CLS:%d, IDX:%d, X:%d, Y:%d)", pNpcData->m_iNPC_NUM, pNpcData->m_iNPC_INDEX, pNpcData->m_iNPC_SX, pNpcData->m_iNPC_SY);
						}
					}
					break;
				}
			}
		}
		it++;
	}
	LeaveCriticalSection(&this->m_critNpcData);

	if( bEXIST == TRUE && bDbSave == TRUE && this->m_iCastleSiegeState != CASTLESIEGE_STATE_STARTSIEGE )
	{
		GS_GDReqNpcRemove(this->m_iMapSvrGroup, pTempNpcData.m_iNPC_NUM, pTempNpcData.m_iNPC_INDEX);
	}

	return TRUE;
}

BOOL CCastleSiege::RepairDbNPC(int iNpcType, int iNpcIndex, int iNpcHP, int iNpcMaxHP)
{
	BOOL bEXIST = FALSE;

	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();

	while( it != this->m_vtNpcData.end() )
	{
		_CS_NPC_DATA & NpcData = _CS_NPC_DATA(*it);

		if( NpcData.m_iNPC_NUM == iNpcType && NpcData.m_iNPC_INDEX == iNpcIndex && NpcData.m_bIN_USE == 1 && NpcData.m_iNPC_LIVE > 0 )
		{
			it->m_iNPC_HP = iNpcHP;
			it->m_iNPC_MAXHP = iNpcMaxHP;

			int iObjIndex = NpcData.m_iNPC_OBJINDEX;

			if(gObjIsConnected(iObjIndex) != FALSE && gObj[iObjIndex].CsNpcType != 0 && gObj[iObjIndex].Class == iNpcType && gObj[iObjIndex].CsNpcExistVal == NpcData.m_iCsNpcExistVal && gObj[iObjIndex].CsNpcExistVal != 0 )
			{
				gObj[iObjIndex].Life = gObj[iObjIndex].MaxLife;
				bEXIST = TRUE;
			}
			break;
		}
		it++;
	}

	LeaveCriticalSection(&this->m_critNpcData);

	return bEXIST;
}

int CCastleSiege::PayForUpgradeDbNpc(int iIndex, int iNpcType, int iNpcIndex, int iNpcUpType, int iNpcUpValue, int iNpcUpIndex)
{
	int iNEED_GEMOFDEFEND = 0;
	int iNEED_MONEY = 0;

	if( CS_CHECK_UPGRADE(iNpcUpIndex) == FALSE )
	{
		LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::PayForUpgradeDbNpc() ERROR - lpMsg->iNpcUpIndex is Out of Bound : %d", iNpcUpIndex);
		return FALSE;
	}

	_CS_NPC_DATA pNpcData;

	BOOL bIsLive = this->GetNpcData(iNpcType, iNpcIndex, pNpcData);

	if( bIsLive == FALSE )
	{
		LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::PayForUpgradeDbNpc() ERROR - bIsLive == FALSE", iNpcUpIndex);
		return FALSE;
	}

	switch( iNpcType )
	{
	case 277:
		{
			switch( iNpcUpType )
			{
			case 1:
				{
					iNEED_GEMOFDEFEND = g_iNpcUpDfLevel_CGATE[iNpcUpIndex][1];
					iNEED_MONEY = g_iNpcUpDfLevel_CGATE[iNpcUpIndex][2];
					
					if( gObj[iIndex].Money < ((DWORD)iNEED_MONEY) )
					{
						GCAnsNpcUpgrade(iIndex, 3, iNpcType, iNpcIndex, iNpcUpType, iNpcUpValue);
						return FALSE;
					}

					int iDelResult = DelGemOfDefend(iIndex, iNEED_GEMOFDEFEND);

					if( iDelResult == FALSE )
					{
						GCAnsNpcUpgrade(iIndex, 4, iNpcType, iNpcIndex, iNpcUpType, iNpcUpValue);
						return FALSE;	
					}

					gObj[iIndex].Money -= iNEED_MONEY;
				}
				break;
			case 3:
				{
					iNEED_GEMOFDEFEND = g_iNpcUpMaxHP_CGATE[iNpcUpIndex][1];
					iNEED_MONEY = g_iNpcUpMaxHP_CGATE[iNpcUpIndex][2];

					if( gObj[iIndex].Money < ((DWORD)iNEED_MONEY) )
					{
						GCAnsNpcUpgrade(iIndex, 3, iNpcType, iNpcIndex, iNpcUpType, iNpcUpValue);
						return FALSE;
					}

					int iDelResult = DelGemOfDefend(iIndex, iNEED_GEMOFDEFEND);

					if( iDelResult == FALSE )
					{
						GCAnsNpcUpgrade(iIndex, 4, iNpcType, iNpcIndex, iNpcUpType, iNpcUpValue);
						return FALSE;	
					}

					gObj[iIndex].Money -= iNEED_MONEY;
				}
				break;
			default:
				{
					GCAnsNpcUpgrade(iIndex, 5, iNpcType, iNpcIndex, iNpcUpType, iNpcUpValue);
					return FALSE;	
				}
				break;
			}
		}
		break;
	case 283:
		{
			switch( iNpcUpType )
			{
			case 1:
				{
					iNEED_GEMOFDEFEND = g_iNpcUpDfLevel_CSTATUE[iNpcUpIndex][1];
					iNEED_MONEY = g_iNpcUpDfLevel_CSTATUE[iNpcUpIndex][2];
					
					if( gObj[iIndex].Money < ((DWORD)iNEED_MONEY) )
					{
						GCAnsNpcUpgrade(iIndex, 3, iNpcType, iNpcIndex, iNpcUpType, iNpcUpValue);
						return FALSE;
					}

					int iDelResult = DelGemOfDefend(iIndex, iNEED_GEMOFDEFEND);

					if( iDelResult == FALSE )
					{
						GCAnsNpcUpgrade(iIndex, 4, iNpcType, iNpcIndex, iNpcUpType, iNpcUpValue);
						return FALSE;	
					}

					gObj[iIndex].Money -= iNEED_MONEY;
				}
				break;
			case 2:
				{
					iNEED_GEMOFDEFEND = g_iNpcUpRgLevel_CSTATUE[iNpcUpIndex][1];
					iNEED_MONEY = g_iNpcUpRgLevel_CSTATUE[iNpcUpIndex][2];

					if( gObj[iIndex].Money < ((DWORD)iNEED_MONEY) )
					{
						GCAnsNpcUpgrade(iIndex, 3, iNpcType, iNpcIndex, iNpcUpType, iNpcUpValue);
						return FALSE;
					}

					int iDelResult = DelGemOfDefend(iIndex, iNEED_GEMOFDEFEND);

					if( iDelResult == FALSE )
					{
						GCAnsNpcUpgrade(iIndex, 4, iNpcType, iNpcIndex, iNpcUpType, iNpcUpValue);
						return FALSE;	
					}

					gObj[iIndex].Money -= iNEED_MONEY;
				}
				break;
			case 3:
				{
					iNEED_GEMOFDEFEND = g_iNpcUpMaxHP_CSTATUE[iNpcUpIndex][1];
					iNEED_MONEY = g_iNpcUpMaxHP_CSTATUE[iNpcUpIndex][2];

					if( gObj[iIndex].Money < ((DWORD)iNEED_MONEY) )
					{
						GCAnsNpcUpgrade(iIndex, 3, iNpcType, iNpcIndex, iNpcUpType, iNpcUpValue);
						return FALSE;
					}

					int iDelResult = DelGemOfDefend(iIndex, iNEED_GEMOFDEFEND);

					if( iDelResult == FALSE )
					{
						GCAnsNpcUpgrade(iIndex, 4, iNpcType, iNpcIndex, iNpcUpType, iNpcUpValue);
						return FALSE;	
					}

					gObj[iIndex].Money -= iNEED_MONEY;
				}
				break;
			default:
				{
					GCAnsNpcUpgrade(iIndex, 5, iNpcType, iNpcIndex, iNpcUpType, iNpcUpValue);
					return FALSE;	
				}
				break;
			}
		}
		break;
	}

	GCMoneySend(iIndex, gObj[iIndex].Money);

	return TRUE;
}

BOOL CCastleSiege::UpgradeDbNPC(int iIndex, int iNpcType, int iNpcIndex, int iNpcUpType, int iNpcUpValue, int iNpcUpIndex)
{
	if( CS_CHECK_UPGRADE(iNpcUpIndex) == FALSE )
	{
		LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::UpgradeDbNPC() ERROR - lpMsg->iNpcUpIndex is Out of Bound : %d", iNpcUpIndex);
		return FALSE;
	}

	_CS_NPC_DATA pNpcData;
	BOOL bIsLive;

	bIsLive = GetNpcData(iNpcType, iNpcIndex, pNpcData);

	if( bIsLive == FALSE )
	{
		LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::UpgradeDbNPC() ERROR - bIsLive == FALSE", iNpcUpIndex);
		return FALSE;
	}

	switch( iNpcType )
	{
		case 277:
			{
				switch( iNpcUpType )
				{
					case 1:
						{
							this->UpgradeDbNPC_DFLEVEL(iNpcType, iNpcIndex, iNpcUpValue );	
						}
						break;

					case 3:
						{
							this->UpgradeDbNPC_MAXHP(iNpcType, iNpcIndex, iNpcUpValue );
						}
						break;

					default:
						{
							::GCAnsNpcUpgrade(iIndex, 5, iNpcType, iNpcIndex, iNpcUpType, iNpcUpValue);
							return FALSE;
						}
						break;
				}
			}
			break;

		case 283:
			{
				switch( iNpcUpType )
				{
					case 1:
						{
							this->UpgradeDbNPC_DFLEVEL(iNpcType, iNpcIndex, iNpcUpValue );
						}
						break;

					case 2:
						{
							this->UpgradeDbNPC_RGLEVEL(iNpcType, iNpcIndex, iNpcUpValue );
						}
						break;

					case 3:
						{
							this->UpgradeDbNPC_MAXHP(iNpcType, iNpcIndex, iNpcUpValue );
						}
						break;

					default:
						{
							::GCAnsNpcUpgrade(iIndex, 5, iNpcType, iNpcIndex, iNpcUpType, iNpcUpValue);
							return FALSE;
						}
						break;
				}
			}
			break;
	}

	return TRUE;
}

int CCastleSiege::UpgradeDbNPC_DFLEVEL(int iNpcType, int iNpcIndex, int iNpcDfLevel)
{
	BOOL bEXIST = FALSE;

	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();
	
	while( it != this->m_vtNpcData.end() )
	{
		_CS_NPC_DATA & NpcData = _CS_NPC_DATA(*it);

		if( NpcData.m_iNPC_NUM == iNpcType && NpcData.m_iNPC_INDEX == iNpcIndex && NpcData.m_bIN_USE == 1 && NpcData.m_iNPC_LIVE > 0 )
		{
			int iObjIndex = NpcData.m_iNPC_OBJINDEX;

			if( gObjIsConnected(iObjIndex) && gObj[iObjIndex].CsNpcType != 0 &&	gObj[iObjIndex].CsNpcExistVal == NpcData.m_iCsNpcExistVal && gObj[iObjIndex].CsNpcExistVal != 0 )
			{
				it->m_iNPC_DF_LEVEL = iNpcDfLevel;
				
				bEXIST = TRUE;
	
				gObj[iObjIndex].Defense = g_iNpcDefense_CGATE[iNpcDfLevel];
				gObj[iObjIndex].MagicDefense = g_iNpcDefense_CGATE[iNpcDfLevel];
				gObj[iObjIndex].CsNpcDfLevel = iNpcDfLevel;
			}
			break;
		}
		it++;
	}

	LeaveCriticalSection(&this->m_critNpcData);
	
	return bEXIST;
}

int CCastleSiege::UpgradeDbNPC_RGLEVEL(int iNpcType, int iNpcIndex, int iNpcRgLevel)
{
	BOOL bEXIST = FALSE;

	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();
	
	while( it != this->m_vtNpcData.end() )
	{
		_CS_NPC_DATA & NpcData = _CS_NPC_DATA(*it);

		if( NpcData.m_iNPC_NUM == iNpcType && NpcData.m_iNPC_INDEX == iNpcIndex && NpcData.m_bIN_USE == 1 && NpcData.m_iNPC_LIVE > 0 )
		{
			int iObjIndex = NpcData.m_iNPC_OBJINDEX;

			if( gObjIsConnected(iObjIndex) && gObj[iObjIndex].CsNpcType != 0 &&	gObj[iObjIndex].CsNpcExistVal == NpcData.m_iCsNpcExistVal && gObj[iObjIndex].CsNpcExistVal != 0 )
			{
				it->m_iNPC_RG_LEVEL = iNpcRgLevel;
				bEXIST = TRUE;
				gObj[iObjIndex].CsNpcRgLevel = iNpcRgLevel;
			}
			break;
		}
		it++;
	}

	LeaveCriticalSection(&this->m_critNpcData);
	
	return bEXIST;
}

int CCastleSiege::UpgradeDbNPC_MAXHP(int iNpcType, int iNpcIndex, int iNpcMaxHP)
{
	BOOL bEXIST = FALSE;

	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();
	
	while( it != this->m_vtNpcData.end() )
	{
		_CS_NPC_DATA & NpcData = _CS_NPC_DATA(*it);

		if( NpcData.m_iNPC_NUM == iNpcType && NpcData.m_iNPC_INDEX == iNpcIndex && NpcData.m_bIN_USE == 1 && NpcData.m_iNPC_LIVE > 0 )
		{
			int iObjIndex = NpcData.m_iNPC_OBJINDEX;

			if( gObjIsConnected(iObjIndex) && gObj[iObjIndex].CsNpcType != 0 &&	gObj[iObjIndex].CsNpcExistVal == NpcData.m_iCsNpcExistVal && gObj[iObjIndex].CsNpcExistVal != 0 )
			{
				it->m_iNPC_MAXHP = iNpcMaxHP;
				it->m_iNPC_HP = iNpcMaxHP;
				bEXIST = TRUE;
				gObj[iObjIndex].Life = (float)iNpcMaxHP;
				gObj[iObjIndex].MaxLife = (float)iNpcMaxHP;
			}
			break;
		}
		it++;
	}

	LeaveCriticalSection(&this->m_critNpcData);
	
	return bEXIST;
}

void CCastleSiege::StoreDbNpc()
{
	LogAdd(LOG_BLACK,"[CastleSiege] CCastleSiege::StoreDbNpc() - << START >>");

	char cBUFFER[0x1090];

	CSP_REQ_NPCUPDATEDATA * lpMsg = (CSP_REQ_NPCUPDATEDATA *)cBUFFER;
	CSP_NPCUPDATEDATA * lpMsgBody = (CSP_NPCUPDATEDATA *)&cBUFFER[sizeof(CSP_REQ_NPCUPDATEDATA)];

	int iCOUNT = 0;

	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();
	BOOL bExist = FALSE;

	while(it != this->m_vtNpcData.end())
	{
		if(iCOUNT >= 150)
		{
			break;
		}

		_CS_NPC_DATA & pNpcData = _CS_NPC_DATA(*it);

		if(pNpcData.m_bNPC_DBSAVE == TRUE  && gObjIsConnected(pNpcData.m_iNPC_OBJINDEX) && gObj[pNpcData.m_iNPC_OBJINDEX].Class == pNpcData.m_iNPC_NUM && gObj[pNpcData.m_iNPC_OBJINDEX].CsNpcExistVal == pNpcData.m_iCsNpcExistVal && gObj[pNpcData.m_iNPC_OBJINDEX].CsNpcExistVal != NULL && gObj[pNpcData.m_iNPC_OBJINDEX].Live == TRUE && gObj[pNpcData.m_iNPC_OBJINDEX].Life > 0)
		{
			lpMsgBody[iCOUNT].iNpcNumber = pNpcData.m_iNPC_NUM;
			lpMsgBody[iCOUNT].iNpcIndex = pNpcData.m_iNPC_INDEX;
			lpMsgBody[iCOUNT].iNpcDfLevel = pNpcData.m_iNPC_DF_LEVEL;
			lpMsgBody[iCOUNT].iNpcRgLevel = pNpcData.m_iNPC_RG_LEVEL;
			lpMsgBody[iCOUNT].iNpcMaxHp = (int)gObj[pNpcData.m_iNPC_OBJINDEX].MaxLife;
			lpMsgBody[iCOUNT].iNpcHp = (int)gObj[pNpcData.m_iNPC_OBJINDEX].Life;
			lpMsgBody[iCOUNT].btNpcX = pNpcData.m_iNPC_SX;
			lpMsgBody[iCOUNT].btNpcY = pNpcData.m_iNPC_SY;
			lpMsgBody[iCOUNT].btNpcDIR = pNpcData.m_iNPC_DIR;

			LogAdd(LOG_BLACK,"[CastleSiege] CCastleSiege::StoreDbNpc() NPC INFO (CLS:%d, IDX:%d(%d), MAXHP:%d, HP:%d, DF:%d, RG:%d)", pNpcData.m_iNPC_NUM,pNpcData.m_iNPC_INDEX,pNpcData.m_iNPC_OBJINDEX,lpMsgBody[iCOUNT].iNpcMaxHp,lpMsgBody[iCOUNT].iNpcHp,lpMsgBody[iCOUNT].iNpcDfLevel,lpMsgBody[iCOUNT].iNpcRgLevel);
			iCOUNT++;
		}
		it++;
	}

	LeaveCriticalSection(&this->m_critNpcData);

	if(iCOUNT < 0)
	{
		iCOUNT = 0;
	}

	lpMsg->wMapSvrNum = this->m_iMapSvrGroup;
	lpMsg->iCount = iCOUNT;
	lpMsg->h.set(0x89,iCOUNT * sizeof(CSP_NPCUPDATEDATA) + sizeof(CSP_REQ_NPCUPDATEDATA));
	gDataServerConnection.DataSend((BYTE*)lpMsg,iCOUNT * sizeof(CSP_NPCUPDATEDATA) + sizeof(CSP_REQ_NPCUPDATEDATA));

	LogAdd(LOG_BLACK,"[CastleSiege] CCastleSiege::StoreDbNpc() - << END >>");
}

void CCastleSiege::SendNpcStateList(int iIndex, int iNpcType)
{
	if( gObjIsConnected(iIndex) == FALSE )
	{
		return;
	}

	char cNPC_LIST[(sizeof(PMSG_NPCDBLIST) * 50)+sizeof(PMSG_ANS_NPCDBLIST)] = {0};

	int iNPC_COUNT = 0;

	PMSG_ANS_NPCDBLIST* lpMsgSend = (PMSG_ANS_NPCDBLIST*)&cNPC_LIST[0];
	PMSG_NPCDBLIST* lpMsgSendBody = (PMSG_NPCDBLIST*)&cNPC_LIST[sizeof(PMSG_ANS_NPCDBLIST)];

	EnterCriticalSection(&this->m_critNpcData);
	
	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();

	while( it != this->m_vtNpcData.end() )
	{
		if( iNPC_COUNT >= 50 )
		{
			break;
		}
	
		_CS_NPC_DATA & NpcData = _CS_NPC_DATA(*it);

		if( NpcData.m_iNPC_NUM == iNpcType && NpcData.m_bIN_USE == TRUE )
		{
			int iLife = 0;
			int iMaxLife = 0;
			BYTE btNpcLive = 0;
			int iObjIndex = NpcData.m_iNPC_OBJINDEX;

			if( gObjIsConnected(iObjIndex) )
			{
				if( gObj[iObjIndex].Class == iNpcType && gObj[iObjIndex].Live == 1 && gObj[iObjIndex].Life > 0 && gObj[iObjIndex].CsNpcType != 0 && gObj[iObjIndex].CsNpcExistVal == NpcData.m_iCsNpcExistVal && gObj[iObjIndex].CsNpcExistVal != 0 )
				{
					iLife = (int)gObj[iObjIndex].Life;
					iMaxLife = (int)gObj[iObjIndex].MaxLife;
					btNpcLive = 1;
				}
				else
				{
					iLife = 0;
					iMaxLife = NpcData.m_iNPC_BASE_MAXHP;
					btNpcLive = 0;
				}
			}
			else
			{
				iLife = 0;
				iMaxLife = NpcData.m_iNPC_BASE_MAXHP;
				btNpcLive = 0;
			}

			lpMsgSendBody[iNPC_COUNT].iNpcNumber = NpcData.m_iNPC_NUM;
			lpMsgSendBody[iNPC_COUNT].iNpcIndex = NpcData.m_iNPC_INDEX;
			lpMsgSendBody[iNPC_COUNT].iNpcDfLevel = NpcData.m_iNPC_DF_LEVEL;
			lpMsgSendBody[iNPC_COUNT].iNpcRgLevel = NpcData.m_iNPC_RG_LEVEL;
			lpMsgSendBody[iNPC_COUNT].iNpcHp = iLife;
			lpMsgSendBody[iNPC_COUNT].iNpcMaxHp = iMaxLife;
			lpMsgSendBody[iNPC_COUNT].btNpcX = NpcData.m_iNPC_SX;
			lpMsgSendBody[iNPC_COUNT].btNpcY = NpcData.m_iNPC_SY;
			lpMsgSendBody[iNPC_COUNT].btNpcLive = btNpcLive;

			iNPC_COUNT++;
		}

		it++;
	}
	LeaveCriticalSection(&this->m_critNpcData);

	lpMsgSend->btResult = 1;
	lpMsgSend->h.set(0xB3, (sizeof(PMSG_NPCDBLIST) * iNPC_COUNT)+sizeof(PMSG_ANS_NPCDBLIST));
	lpMsgSend->iCount = iNPC_COUNT;

	DataSend(iIndex, (LPBYTE)&cNPC_LIST, (sizeof(PMSG_NPCDBLIST) * iNPC_COUNT)+sizeof(PMSG_ANS_NPCDBLIST));
}

int CCastleSiege::GetNpcData(int iNpcType, int iNpcIndex, _CS_NPC_DATA & pRetNpcData)
{
	pRetNpcData.Clear();

	BOOL bEXIST = FALSE;
	
	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();

	while( it != this->m_vtNpcData.end() )
	{
		_CS_NPC_DATA & NpcData = _CS_NPC_DATA(*it);

		if( NpcData.m_iNPC_NUM == iNpcType && NpcData.m_iNPC_INDEX == iNpcIndex && NpcData.m_bIN_USE == 1 && NpcData.m_iNPC_LIVE > 0 )
		{
			int iObjIndex = NpcData.m_iNPC_OBJINDEX;

			if( gObjIsConnected(iObjIndex) && gObj[iObjIndex].CsNpcType != 0 && gObj[iObjIndex].CsNpcExistVal == NpcData.m_iCsNpcExistVal && gObj[iObjIndex].CsNpcExistVal != 0 )
			{
				pRetNpcData = NpcData;
				bEXIST = TRUE;
			}
			break;
		}
		it++;
	}

	LeaveCriticalSection(&this->m_critNpcData);
	return bEXIST;
}

void CCastleSiege::AdjustDbNpcLevel()
{
	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();

	while(it != this->m_vtNpcData.end())
	{
		_CS_NPC_DATA & pNpcData = _CS_NPC_DATA(*it);

		if(pNpcData.m_bIN_USE == 1 && pNpcData.m_iNPC_LIVE > 0)
		{
			int iNPC_INDEX = pNpcData.m_iNPC_OBJINDEX;

			if(gObjIsConnected(iNPC_INDEX) && gObj[iNPC_INDEX].Class == pNpcData.m_iNPC_NUM	&& gObj[iNPC_INDEX].CsNpcType != FALSE && gObj[iNPC_INDEX].CsNpcExistVal == pNpcData.m_iCsNpcExistVal && gObj[iNPC_INDEX].CsNpcExistVal != NULL)
			{
				switch(gObj[iNPC_INDEX].Class)
				{
				case 277:
					{
						int iDF_LEVEL = pNpcData.m_iNPC_DF_LEVEL;

						if(iDF_LEVEL < 0)
						{
							iDF_LEVEL = 0;
						}

						if(iDF_LEVEL > CS_MAX_UPGRADE)
						{
							iDF_LEVEL = CS_MAX_UPGRADE;
						}

						gObj[iNPC_INDEX].CsNpcDfLevel = iDF_LEVEL;
						gObj[iNPC_INDEX].Defense = g_iNpcDefense_CGATE[iDF_LEVEL];
					}
					break;
				case 283:
					{
						int iDF_LEVEL = pNpcData.m_iNPC_DF_LEVEL;

						if(iDF_LEVEL < 0)
						{
							iDF_LEVEL = 0;
						}

						if(iDF_LEVEL > CS_MAX_UPGRADE)
						{
							iDF_LEVEL = CS_MAX_UPGRADE;
						}

						gObj[iNPC_INDEX].CsNpcDfLevel = iDF_LEVEL;
						gObj[iNPC_INDEX].Defense = g_iNpcDefense_CSTATUE[iDF_LEVEL];
						gObj[iNPC_INDEX].CsNpcRgLevel = pNpcData.m_iNPC_RG_LEVEL;
					}
					break;
				}
			}

			break;
		}
		it++;
	}

	LeaveCriticalSection(&m_critNpcData);
}

void CCastleSiege::SendCsGateStateViewPort(int iGateIndex, BYTE btOperate)
{
	if( gObjIsConnected(iGateIndex) == FALSE )
	{
		return;
	}

	LPOBJ lpObj = &gObj[iGateIndex];

	if( lpObj->VPCount2 < 1 )
	{
		return;
	}

	int tObjNum = -1;

	for( int i = 0; i < MAX_VIEWPORT; i++ )
	{
		tObjNum = lpObj->VpPlayer2[i].index;

		if( tObjNum >= 0 )
		{
			if( gObj[tObjNum].Type == OBJECT_USER && gObjIsConnected(tObjNum) )
			{
				GCAnsCsGateCurState(tObjNum, iGateIndex, btOperate);
			}
		}
	}
}

void CCastleSiege::CreateAllCsGateLever()
{
	EnterCriticalSection(&this->m_critNpcData);

	if(!this->m_vtNpcData.empty())
	{
		for(std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin(); it != this->m_vtNpcData.end(); it++)
		{
			_CS_NPC_DATA & pNpcData = _CS_NPC_DATA(*it);

			if(pNpcData.m_iNPC_NUM == 277)
			{
				if(pNpcData.m_iCS_GATE_LEVER_INDEX != -1 && CheckLeverAlive(pNpcData.m_iCS_GATE_LEVER_INDEX) == 1)
				{
					continue;
				}

				int iLeverIndex = this->CreateCsGateLever(pNpcData.m_iNPC_SX,pNpcData.m_iNPC_SY + 4);

				if(OBJECT_RANGE(iLeverIndex))
				{
					it->m_iCS_GATE_LEVER_INDEX = iLeverIndex;
				}
				else
				{
					it->m_iCS_GATE_LEVER_INDEX = -1;
					LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::CreateAllCsGateLever() ERROR - Failed to Create Gate Lever : GateIndex:%d",pNpcData.m_iNPC_INDEX);
				}
			}
		}
	}

	LeaveCriticalSection(&this->m_critNpcData);
}

int CCastleSiege::CreateCsGateLever(int iX, int iY)
{
	if(XY_MACRO(iX) == FALSE || XY_MACRO(iY) == FALSE)
	{
		return -1;
	}

	int iNPC_INDEX = gObjAddMonster(MAP_CASTLE_SIEGE);

	if(iNPC_INDEX >= 0)
	{
		gObjSetMonster(iNPC_INDEX, 219);

		gObj[iNPC_INDEX].PosNum = -1;
		gObj[iNPC_INDEX].X = iX;
		gObj[iNPC_INDEX].Y = iY;
		gObj[iNPC_INDEX].Map = MAP_CASTLE_SIEGE;
		gObj[iNPC_INDEX].TX = gObj[iNPC_INDEX].X;
		gObj[iNPC_INDEX].TY = gObj[iNPC_INDEX].Y;
		gObj[iNPC_INDEX].OldX = gObj[iNPC_INDEX].X;
		gObj[iNPC_INDEX].OldY = gObj[iNPC_INDEX].Y;
		gObj[iNPC_INDEX].Dir = 1;
		gObj[iNPC_INDEX].StartX = (BYTE)gObj[iNPC_INDEX].X;
		gObj[iNPC_INDEX].StartY = (BYTE)gObj[iNPC_INDEX].Y;
		gObj[iNPC_INDEX].DieRegen = 0;
		gObj[iNPC_INDEX].MaxRegenTime = 0;
		gObj[iNPC_INDEX].Life = 10000.0f;
		gObj[iNPC_INDEX].MaxLife = 10000.0f;
		gObj[iNPC_INDEX].CsNpcType = 2;
		gObj[iNPC_INDEX].CsGateLeverLinkIndex = -1;
	
		return iNPC_INDEX;
	}	

	return -1;
}

int CCastleSiege::LinkCsGateLever(int iLeverIndex,int iGateIndex)
{
	if(!gObjIsConnected(iLeverIndex) || gObj[iLeverIndex].Class != 219)
	{
		return false;
	}

	if(!gObjIsConnected(iGateIndex) || gObj[iGateIndex].Class != 277)
	{
		return false;
	}

	gObj[iLeverIndex].CsGateLeverLinkIndex = iGateIndex;
	gObj[iGateIndex].CsGateLeverLinkIndex = iLeverIndex;
	return true;
}

int CCastleSiege::CheckLeverAlive(int iLeverIndex)
{
	if(gObjIsConnected(iLeverIndex) == FALSE || gObj[iLeverIndex].Class != 219 )
	{
		return false;
	}

	return true;
}

void CCastleSiege::CheckCsDbNpcAlive()
{
	if( m_iCastleSiegeState != CASTLESIEGE_STATE_STARTSIEGE )
	{
		return;
	}

	EnterCriticalSection(&this->m_critNpcData);

	if( this->m_vtNpcData.empty() == false )
	{
		for( std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin(); it != this->m_vtNpcData.end(); it++ )
		{
			_CS_NPC_DATA & NpcData = _CS_NPC_DATA(*it);

			if( (NpcData.m_iNPC_NUM == 277 || NpcData.m_iNPC_NUM == 283) && NpcData.m_iNPC_OBJINDEX != -1 )
			{
				if( !gObjIsConnected(NpcData.m_iNPC_OBJINDEX) || gObj[NpcData.m_iNPC_OBJINDEX].CsNpcExistVal != NpcData.m_iCsNpcExistVal || gObj[NpcData.m_iNPC_OBJINDEX].CsNpcExistVal == 0 || (gObj[NpcData.m_iNPC_OBJINDEX].Class != 277 && gObj[NpcData.m_iNPC_OBJINDEX].Class != 283) || gObj[NpcData.m_iNPC_OBJINDEX].Live == FALSE)
				{
					NpcData.m_iNPC_HP = NpcData.m_iNPC_BASE_HP;
					NpcData.m_iNPC_MAXHP = NpcData.m_iNPC_BASE_MAXHP;
					NpcData.m_iNPC_RG_LEVEL = NpcData.m_iNPC_BASE_RG_LEVEL;
					NpcData.m_iNPC_DF_LEVEL = NpcData.m_iNPC_BASE_DF_LEVEL;

					switch( NpcData.m_iNPC_NUM )
					{
						case 277:
							this->SetGateBlockState(NpcData.m_iNPC_SX, NpcData.m_iNPC_SY, 1);
							gEffectManager.AddEffect(&gObj[NpcData.m_iNPC_OBJINDEX],0,EFFECT_CASTLE_GATE_STATE,0,0,0,0,0);
							LogAdd(LOG_BLACK,"[CastleSiege] Castle Gate is Broken (CLS:%d, IDX:%d, X:%d, Y:%d)", NpcData.m_iNPC_NUM, NpcData.m_iNPC_INDEX, NpcData.m_iNPC_SX, NpcData.m_iNPC_SY);
							break;
						case 283:
							if( this->CheckGuardianStatueExist() == FALSE )
							{
								this->ClearCastleTowerBarrier();
								this->SetCastleTowerAccessable(TRUE);
							}
							LogAdd(LOG_BLACK,"[CastleSiege] Defend Statue is Broken (CLS:%d, IDX:%d, X:%d, Y:%d)", NpcData.m_iNPC_NUM, NpcData.m_iNPC_INDEX, NpcData.m_iNPC_SX, NpcData.m_iNPC_SY);
							break;
					}
					it->m_iNPC_OBJINDEX = -1;
				}
			}
		}
	}
	LeaveCriticalSection(&this->m_critNpcData);
}

BOOL CCastleSiege::CheckCsGateAlive(int iGateIndex)
{
	if( !gObjIsConnected(iGateIndex) || gObj[iGateIndex].Class != 277 ) 
	{
		return FALSE;
	}

	return TRUE;
}

int CCastleSiege::DelGemOfDefend(int iIndex, int iNeedGemOfDefend)
{
	if( gObjIsConnected(iIndex) == FALSE )
	{
		return FALSE;
	}

	if( iNeedGemOfDefend <= 0 )
	{
		return FALSE;
	}

	int iGemOfDefendCount = 0;

	for( int x = 0; x < INVENTORY_MAIN_SIZE; x++ )
	{
		if( gObj[iIndex].Inventory[x].IsItem() == TRUE )
		{
			if( gObj[iIndex].Inventory[x].m_Index == GET_ITEM(14,31) )
			{
				iGemOfDefendCount++;
			}
		}
	}

	if( iGemOfDefendCount < iNeedGemOfDefend )
	{
		return FALSE;
	}

	int iGemOfDefendDeletCount = 0;

	for(int x = 0; x < INVENTORY_MAIN_SIZE; x++ )
	{
		if( gObj[iIndex].Inventory[x].IsItem() == TRUE )
		{
			if( gObj[iIndex].Inventory[x].m_Index == GET_ITEM(14,31) )
			{
				LogAdd(LOG_BLACK,"[CastleSiege] [%s][%s] Deleting Gem of Defend (GUILD:%s) - Serial:%u", gObj[iIndex].Account, gObj[iIndex].Name, gObj[iIndex].GuildName, gObj[iIndex].Inventory[x].m_Serial);

				gItemManager.InventoryItemSet(iIndex,x,0xFF);
				gItemManager.InventoryDelItem(iIndex,x);
				gItemManager.GCItemDeleteSend(iIndex,x,1);

				iGemOfDefendDeletCount++;

				if( iGemOfDefendDeletCount >= iNeedGemOfDefend )
				{
					break;
				}
			}
		}
	}

	return TRUE;
}

void CCastleSiege::SetTaxRate(int iTaxType, int iTaxRate)
{
	int iMaxTaxRate = 0;
	
	switch( iTaxType )
	{
	case 1:
		iMaxTaxRate = 3;
		break;
	case 2:
		iMaxTaxRate = 3;
		break;
	case 3:
		iMaxTaxRate = 300000;
		break;
	}

	if( iTaxRate < 0 || iTaxRate > iMaxTaxRate )
	{
		return;
	}
	
	switch( iTaxType )
	{
	case 1:
		this->m_iTaxRateChaos = iTaxRate;
		break;
	case 2:
		this->m_iTaxRateStore = iTaxRate;
		break;
	case 3:
		this->m_iTaxHuntZone = iTaxRate;
		break;
	}
	
	LogAdd(LOG_BLACK,"[CastleSiege] Tax Rate Setted - Type:%d, Rate:%d", iTaxType, iTaxRate);
}

BOOL CCastleSiege::CheckCastleHasMoney(__int64 i64CastleMoney)
{
	if( this->m_i64CastleMoney >= i64CastleMoney )
	{
		return TRUE;
	}

	return FALSE;
}

void CCastleSiege::SetCastleMoney(__int64 i64CastleMoney)
{
	this->m_i64CastleMoney = i64CastleMoney;
}

void CCastleSiege::ResetCastleTaxInfo()
{
	this->m_i64CastleMoney = 0;
	this->m_iTaxRateChaos = 0;
	this->m_iTaxRateStore = 0;
	this->m_iTaxHuntZone = 0;
}

void CCastleSiege::SendCastleStateSync()
{
	GS_GDReqCastleStateSync(this->m_iMapSvrGroup, this->m_iCastleSiegeState, this->m_iTaxRateChaos, this->m_iTaxRateStore, this->m_iTaxHuntZone, this->m_szCastleOwnerGuild);
}

BOOL CCastleSiege::CheckGuildOwnCastle(char* lpszGuildName)
{
	if( lpszGuildName == NULL )
	{
		return FALSE;
	}

	if( strcmp(this->m_szCastleOwnerGuild, "") == 0 )
	{
		return FALSE;
	}

	if( strcmp(this->m_szCastleOwnerGuild, lpszGuildName) == 0 )
	{
		return TRUE;
	}

	return FALSE;
}

void CCastleSiege::ResetCastleCycle()
{
	this->m_bCsBasicGuildInfoLoadOK = FALSE;
	this->m_bCsTotalGuildInfoLoadOK = FALSE;

	EnterCriticalSection(&this->m_critCsBasicGuildInfo);
	this->m_mapCsBasicGuildInfo.clear();
	LeaveCriticalSection(&this->m_critCsBasicGuildInfo);

	EnterCriticalSection(&this->m_critCsTotalGuildInfo);
	this->m_mapCsTotalGuildInfo.clear();
	LeaveCriticalSection(&this->m_critCsTotalGuildInfo);
	
	this->m_bCastleTowerAccessable = FALSE;
	this->m_bRegCrownAvailable = FALSE;
	this->m_btIsSiegeGuildList = 0;
	this->m_btIsSiegeEnded = 0;
}

void CCastleSiege::SetGateBlockState(int iX,int iY,int iGateState)
{
	if(XY_MACRO(iX) == FALSE || XY_MACRO(iY) == FALSE)
	{
		return;
	}

	switch(iGateState)
	{
	case 0:
		{
			for(int i = iX -2; i <= iX + 3; i++)
			{
				for(int j = iY; j <= iY + 2;j++)
				{
					gMap[MAP_CASTLE_SIEGE].m_MapAttr[j * 256 + i] |= 16;
				}
			}
		}
		break;
	case 1:
		{
			for(int i = iX -2; i <= iX + 3; i++)
			{
				for(int j = iY; j <= iY + 2;j++)
				{
					gMap[MAP_CASTLE_SIEGE].m_MapAttr[j * 256 + i] &= ~16;
				}
			}
		}
		break;
	}
}

void CCastleSiege::CheckBuildCsGuildInfo()
{
	this->m_bCsBasicGuildInfoLoadOK = FALSE;
	this->m_bCsTotalGuildInfoLoadOK = FALSE;

	EnterCriticalSection(&this->m_critCsBasicGuildInfo);
	this->m_mapCsBasicGuildInfo.clear();
	LeaveCriticalSection(&this->m_critCsBasicGuildInfo);

	EnterCriticalSection(&this->m_critCsTotalGuildInfo);
	this->m_mapCsTotalGuildInfo.clear();
	LeaveCriticalSection(&this->m_critCsTotalGuildInfo);

	if(this->m_btIsSiegeGuildList == TRUE)
	{
		GS_GDReqCsLoadTotalGuildInfo(this->m_iMapSvrGroup);
	}
	else
	{
		GS_GDReqCalcRegGuildList(this->m_iMapSvrGroup);
	}
}

void CCastleSiege::SetCalcRegGuildList(CSP_CALCREGGUILDLIST* lpMsg, int iCOUNT)
{
	if( lpMsg == NULL )
	{
		return;
	}

	if( iCOUNT > 100 ) 
	{
		iCOUNT = 100;
	}

	if( iCOUNT <= 0 )
	{
		return;
	}

	this->m_bCsBasicGuildInfoLoadOK = FALSE;
	this->m_bCsTotalGuildInfoLoadOK = FALSE;

	std::vector<_CS_REG_GUILD_DATA> vtRegGuildScore;

	for( int iGCNT = 0; iGCNT < iCOUNT; iGCNT++ )
	{
		_CS_REG_GUILD_DATA GuildData;

		char szGuildName[16] = {0};

		memcpy(szGuildName, lpMsg[iGCNT].szGuildName, 8);
		
		GuildData.m_bIN_USE = TRUE;
		GuildData.m_strGuildName = szGuildName;
		GuildData.m_iRegMarkCount = lpMsg[iGCNT].iRegMarkCount;
		GuildData.m_iGuildMasterLevel = lpMsg[iGCNT].iGuildMasterLevel;
		GuildData.m_iGuildMemberCount = lpMsg[iGCNT].iGuildMemberCount;
		GuildData.m_iSeqNum = 0x100000 - lpMsg[iGCNT].iSeqNum; 

		int dwHighScore = ( ( lpMsg[iGCNT].iRegMarkCount * 5 ) + lpMsg[iGCNT].iGuildMemberCount + (lpMsg[iGCNT].iGuildMasterLevel / 4));

		#define MACRO3(value) (((__int64) value & 0xFFFFFFFF ))

		GuildData.m_i64TotolScore = ((__int64)(DWORD)MACRO3(lpMsg[iGCNT].iSeqNum)) | (((__int64)(DWORD)MACRO3(dwHighScore))<<32);

		vtRegGuildScore.push_back(GuildData);

		LogAdd(LOG_BLACK,"[CastleSiege][REG Mark Count] - [%s][%d]", szGuildName,lpMsg[iGCNT].iRegMarkCount);
	}
 
	if( vtRegGuildScore.empty() )
	{
		return;
	}

	sort(vtRegGuildScore.begin(), vtRegGuildScore.end(), CCastleSiege::RegGuildScoreCompFunc);

	int iGUILD_COUNT = vtRegGuildScore.size();

	if( iGUILD_COUNT > 3 ) 
	{
		iGUILD_COUNT = 3;
	}

	EnterCriticalSection(&this->m_critCsBasicGuildInfo);

	this->m_mapCsBasicGuildInfo.clear();

	for(int iGCNT = 0; iGCNT < iGUILD_COUNT; iGCNT++ )
	{
		_CS_TOTAL_GUILD_DATA GuildData;
		
		GuildData.m_bIN_USE = TRUE;
		GuildData.m_iCsGuildID = (iGCNT + 2);
		GuildData.m_bGuildInvolved = TRUE;
		GuildData.m_iGuildScore = (vtRegGuildScore[iGCNT].m_iRegMarkCount*5) + (vtRegGuildScore[iGCNT].m_iGuildMemberCount) + (vtRegGuildScore[iGCNT].m_iGuildMasterLevel/4);

		this->m_mapCsBasicGuildInfo.insert(std::pair<std::string,_CS_TOTAL_GUILD_DATA>(vtRegGuildScore[iGCNT].m_strGuildName, GuildData));
	}

	if( this->m_btIsCastleOccupied == TRUE )
	{
		if( strcmp(this->m_szCastleOwnerGuild, "") != 0 )
		{
			_CS_TOTAL_GUILD_DATA GuildData;

			GuildData.m_bIN_USE = TRUE;
			GuildData.m_iCsGuildID = 1;
			GuildData.m_bGuildInvolved = TRUE;
			GuildData.m_iGuildScore = 0;
			
			this->m_mapCsBasicGuildInfo.insert(std::pair<std::string,_CS_TOTAL_GUILD_DATA>(m_szCastleOwnerGuild, GuildData));
		}
	}

	LeaveCriticalSection(&this->m_critCsBasicGuildInfo);

	char cBUFFER[2080];

	CSP_REQ_CSGUILDUNIONINFO* lpMsgSend = (CSP_REQ_CSGUILDUNIONINFO*)cBUFFER;
	CSP_CSGUILDUNIONINFO* lpMsgSendBody = (CSP_CSGUILDUNIONINFO*)&cBUFFER[sizeof(CSP_REQ_CSGUILDUNIONINFO)];

	lpMsgSend->wMapSvrNum = this->m_iMapSvrGroup;
	lpMsgSend->iCount = 0;

	EnterCriticalSection(&this->m_critCsTotalGuildInfo);

	m_mapCsTotalGuildInfo.clear();

	std::map<std::string,_CS_TOTAL_GUILD_DATA>::iterator it = this->m_mapCsBasicGuildInfo.begin();

	for(; it != this->m_mapCsBasicGuildInfo.end(); it++)
	{
		if( lpMsgSend->iCount > 150 )
		{
			break;
		}

		this->m_mapCsTotalGuildInfo.insert(std::pair<std::string,_CS_TOTAL_GUILD_DATA>(it->first, it->second));

		std::string GuildName = it->first;

		memcpy(&lpMsgSendBody[lpMsgSend->iCount].szGuildName,  &GuildName[0], 8);
		lpMsgSendBody[lpMsgSend->iCount].iCsGuildID = it->second.m_iCsGuildID;

		lpMsgSend->iCount++;
	}

	LeaveCriticalSection(&this->m_critCsTotalGuildInfo);

	this->m_bCsBasicGuildInfoLoadOK = TRUE;

	lpMsgSend->h.set(0x86, (lpMsgSend->iCount * sizeof(CSP_CSGUILDUNIONINFO))+ sizeof(CSP_REQ_CSGUILDUNIONINFO));
	gDataServerConnection.DataSend((BYTE*)lpMsgSend, (lpMsgSend->iCount * sizeof(CSP_CSGUILDUNIONINFO))+ sizeof(CSP_REQ_CSGUILDUNIONINFO));
}

BYTE CCastleSiege::RegGuildScoreCompFunc(_CS_REG_GUILD_DATA & A, _CS_REG_GUILD_DATA & B)
{
	if(A.m_i64TotolScore < B.m_i64TotolScore)
	{
		return false;
	}
	return true;
}

void CCastleSiege::MakeCsTotalGuildInfo(CSP_CSGUILDUNIONINFO* lpMsg, int iCOUNT)
{
	if( lpMsg == NULL )
	{
		return;
	}

	if( this->m_bCsBasicGuildInfoLoadOK == FALSE )
	{
		LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::MakeCsTotalGuildInfo() ERROR - m_bCsBasicGuildInfoLoadOK == FALSE");
		return;
	}
	if( iCOUNT <= 0 )
	{
		LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::MakeCsTotalGuildInfo() ERROR - iCOUNT <= 0 : %d", iCOUNT);
		return;
	}

	EnterCriticalSection(&this->m_critCsTotalGuildInfo);

	for( int iGCNT = 0; iGCNT < iCOUNT; iGCNT++ )
	{
		char szGuildName[16] = {0};

		memcpy(szGuildName, lpMsg[iGCNT].szGuildName, 8);

		std::map<std::string,_CS_TOTAL_GUILD_DATA>::iterator it = this->m_mapCsTotalGuildInfo.find(szGuildName);

		if( it != this->m_mapCsTotalGuildInfo.end() )
		{
			continue;
		}

		_CS_TOTAL_GUILD_DATA GuildData;
		
		GuildData.m_bIN_USE = TRUE;
		GuildData.m_iCsGuildID = lpMsg[iGCNT].iCsGuildID;
		GuildData.m_bGuildInvolved = FALSE;

		this->m_mapCsTotalGuildInfo.insert(std::pair<std::string,_CS_TOTAL_GUILD_DATA>(szGuildName, GuildData));
	}

	LeaveCriticalSection(&this->m_critCsTotalGuildInfo);

	this->m_bCsTotalGuildInfoLoadOK = TRUE;
	this->SaveCsTotalGuildInfo();
	this->SetAllUserCsJoinSide();
}

void CCastleSiege::SaveCsTotalGuildInfo()
{
	if( this->m_bCsTotalGuildInfoLoadOK == FALSE )
	{
		LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::SaveCsTotalGuildInfo() ERROR - m_bCsTotalGuildInfoLoadOK == FALSE");
		return;
	}

	char cBUFFER[3288];

	CSP_REQ_CSSAVETOTALGUILDINFO* lpMsgSend = (CSP_REQ_CSSAVETOTALGUILDINFO*)&cBUFFER[0];
	CSP_CSSAVETOTALGUILDINFO* lpMsgSendBody =(CSP_CSSAVETOTALGUILDINFO*)&cBUFFER[sizeof(CSP_REQ_CSSAVETOTALGUILDINFO)];

	lpMsgSend->wMapSvrNum = this->m_iMapSvrGroup;
	lpMsgSend->iCount = 0;

	EnterCriticalSection(&this->m_critCsTotalGuildInfo);

	std::map<std::string,_CS_TOTAL_GUILD_DATA>::iterator it = this->m_mapCsTotalGuildInfo.begin();

	for(; it != this->m_mapCsTotalGuildInfo.end(); it++)
	{
		if( lpMsgSend->iCount > 150 )
		{
			break;
		}

		std::string szGuildName = it->first;

		memcpy(&lpMsgSendBody[lpMsgSend->iCount].szGuildName, &szGuildName[0], 8);
		
		lpMsgSendBody[lpMsgSend->iCount].iCsGuildID = it->second.m_iCsGuildID;
		lpMsgSendBody[lpMsgSend->iCount].iGuildInvolved = it->second.m_bGuildInvolved;
		lpMsgSendBody[lpMsgSend->iCount].iGuildScore = it->second.m_iGuildScore;

		lpMsgSend->iCount++;
	}

	LeaveCriticalSection(&this->m_critCsTotalGuildInfo);

	lpMsgSend->h.set(0x87, (lpMsgSend->iCount * sizeof(CSP_CSSAVETOTALGUILDINFO))+ sizeof(CSP_REQ_CSSAVETOTALGUILDINFO));
	gDataServerConnection.DataSend((BYTE*)lpMsgSend, (lpMsgSend->iCount * sizeof(CSP_CSSAVETOTALGUILDINFO))+ sizeof(CSP_REQ_CSSAVETOTALGUILDINFO));

	EnterCriticalSection(&this->m_critCsTotalGuildInfo);

	std::map<std::string,_CS_TOTAL_GUILD_DATA>::iterator it2 = this->m_mapCsTotalGuildInfo.begin();

	for( int iSize = this->m_mapCsTotalGuildInfo.size(); it2 != this->m_mapCsTotalGuildInfo.end(); it2++ )
	{
		std::string szGuildName = it2->first;

		_CS_TOTAL_GUILD_DATA GuildData = it2->second;

		LogAdd(LOG_BLACK,"[CastleSiege] Siege Guild List Save - GNAME:%s, CSGUID:%d, INVOLVED:%d, SCORE:%d", (char*)&szGuildName[0], GuildData.m_iCsGuildID, GuildData.m_bGuildInvolved, GuildData.m_iGuildScore);
	}

	LeaveCriticalSection(&this->m_critCsTotalGuildInfo);
}

void CCastleSiege::SetCsTotalGuildInfo(CSP_CSLOADTOTALGUILDINFO* lpMsg, int iCOUNT)
{
	if( lpMsg == NULL )
	{
		return;
	}

	if( iCOUNT <= 0 )
	{
		LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::SetCsTotalGuildInfo() ERROR - iCOUNT <= 0 : %d", iCOUNT);
		return;
	}

	for( int iGCNT = 0; iGCNT < iCOUNT; iGCNT++ )
	{
		char szGuildName[16] = {0};

		memcpy(szGuildName, lpMsg[iGCNT].szGuildName, 8);

		_CS_TOTAL_GUILD_DATA GuildData;
		
		GuildData.m_bIN_USE = TRUE;
		GuildData.m_iCsGuildID = lpMsg[iGCNT].iCsGuildID;
		GuildData.m_bGuildInvolved = lpMsg[iGCNT].iGuildInvolved;
		GuildData.m_iGuildScore = lpMsg[iGCNT].iGuildScore;

		EnterCriticalSection(&this->m_critCsTotalGuildInfo);
		this->m_mapCsTotalGuildInfo.insert(std::pair<std::string,_CS_TOTAL_GUILD_DATA>(szGuildName, GuildData));
		LeaveCriticalSection(&this->m_critCsTotalGuildInfo);
		
		if( lpMsg[iGCNT].iGuildInvolved == TRUE )
		{
			EnterCriticalSection(&this->m_critCsBasicGuildInfo);
			this->m_mapCsBasicGuildInfo.insert(std::pair<std::string,_CS_TOTAL_GUILD_DATA>(szGuildName, GuildData));
			LeaveCriticalSection(&this->m_critCsBasicGuildInfo);
		}
	}

	this->m_bCsBasicGuildInfoLoadOK = TRUE;
	this->m_bCsTotalGuildInfoLoadOK = TRUE;
	this->SetAllUserCsJoinSide();

	EnterCriticalSection(&this->m_critCsTotalGuildInfo);

	std::map<std::string,_CS_TOTAL_GUILD_DATA>::iterator it2 = this->m_mapCsTotalGuildInfo.begin();
	
	int iSize = this->m_mapCsTotalGuildInfo.size();

	for( ; it2 != this->m_mapCsTotalGuildInfo.end(); it2++ )
	{
		std::string GuildName = it2->first;

		_CS_TOTAL_GUILD_DATA GuildData = _CS_TOTAL_GUILD_DATA(it2->second);

		LogAdd(LOG_BLACK,"[CastleSiege] Siege Guild List Load - GNAME:%s, CSGUID:%d, INVOLVED:%d, SCORE:%d", &GuildName[0], GuildData.m_iCsGuildID, GuildData.m_bGuildInvolved, GuildData.m_iGuildScore);
	}

	LeaveCriticalSection(&m_critCsTotalGuildInfo);
}

int CCastleSiege::GetCsJoinSide(char* lpszGuildName, BYTE * btCsJoinSide, bool * bCsGuildInvolved)
{
	*btCsJoinSide = 0;
	*bCsGuildInvolved = 0;

	if( lpszGuildName == NULL )
	{
		return FALSE;
	}

	if( this->m_bDoRun == FALSE )
	{
		return FALSE;
	}

	if( this->m_iCastleSiegeState < CASTLESIEGE_STATE_NOTIFY || CASTLESIEGE_STATE_STARTSIEGE < this->m_iCastleSiegeState )
	{
		return FALSE;
	}

	if( this->m_bCsTotalGuildInfoLoadOK == FALSE )
	{
		return FALSE;
	}
	
	BOOL bRETVAL = FALSE;

	EnterCriticalSection(&this->m_critCsTotalGuildInfo);

	std::map<std::string,_CS_TOTAL_GUILD_DATA>::iterator it = this->m_mapCsTotalGuildInfo.find(lpszGuildName);

	if( it != this->m_mapCsTotalGuildInfo.end() )
	{
		*btCsJoinSide = it->second.m_iCsGuildID;
		*bCsGuildInvolved = (it->second.m_bGuildInvolved != 0);
		bRETVAL = TRUE;
	}
	
	LeaveCriticalSection(&this->m_critCsTotalGuildInfo);

	return bRETVAL;
}

void CCastleSiege::SetAllUserCsJoinSide()
{
	if(this->m_bCsTotalGuildInfoLoadOK == FALSE)
	{
		return;
	}

	EnterCriticalSection(&this->m_critCsTotalGuildInfo);

	for(int iUSER = OBJECT_START_USER; iUSER < MAX_OBJECT; iUSER++)
	{
		if(gObjIsConnected(iUSER) == FALSE)
		{
			continue;
		}

		char szGuildName[16] = {0};

		memcpy(szGuildName,gObj[iUSER].GuildName,8);

		std::map<std::string,_CS_TOTAL_GUILD_DATA>::iterator it = this->m_mapCsTotalGuildInfo.find(szGuildName);
		
		if( it != this->m_mapCsTotalGuildInfo.end() )
		{
			gObj[iUSER].CsJoinSide = it->second.m_iCsGuildID;
			gObj[iUSER].CsGuildInvolved = (it->second.m_bGuildInvolved != 0);
		}

		this->NotifySelfCsJoinSide(iUSER);

		gObj[iUSER].AccumulatedCrownAccessTime = 0;
	}

	LeaveCriticalSection(&this->m_critCsTotalGuildInfo);
}

void CCastleSiege::ResetAllUserCsJoinSide()
{
	EnterCriticalSection(&this->m_critCsTotalGuildInfo);

	for(int iUSER = OBJECT_START_USER; iUSER < MAX_OBJECT; iUSER++)
	{
		if(::gObjIsConnected(iUSER) == FALSE)
		{
			continue;
		}

		gObj[iUSER].CsJoinSide = 0;
		gObj[iUSER].CsGuildInvolved = 0;
		gObj[iUSER].LifeStoneCount = 0;

		this->NotifySelfCsJoinSide(iUSER);

		gObj[iUSER].AccumulatedCrownAccessTime = 0;
	}

	LeaveCriticalSection(&this->m_critCsTotalGuildInfo);
}

void CCastleSiege::NotifySelfCsJoinSide(int iIndex)
{
	if(gObjIsConnected(iIndex) == FALSE)
	{
		return;
	}

	int iState = 0;
	int iViewState = 0;

	BYTE btCsJoinSide = gObj[iIndex].CsJoinSide;

	PMSG_ANS_NOTIFYCSJOINSIDE pMsg;

	if(btCsJoinSide < 0)
	{
		btCsJoinSide = 0;
	}

	if(btCsJoinSide > 4)
	{
		btCsJoinSide = 0;
	}

	switch(btCsJoinSide)
	{
	case 1:
		iState = 1;
		iViewState = 0x40000;
		if(this->m_iCastleSiegeState == CASTLESIEGE_STATE_STARTSIEGE || this->m_iCastleSiegeState == CASTLESIEGE_STATE_ENDSIEGE)
		{
			gEffectManager.AddEffect(&gObj[iIndex],0,EFFECT_GUILD_STATE1,0,0,0,0,0);
		}
		else
		{
			gEffectManager.DelEffect(&gObj[iIndex],EFFECT_GUILD_STATE1);
		}
		break;
	case 2:
		iState = 1;
		iViewState = 0x80000;
		if(this->m_iCastleSiegeState == CASTLESIEGE_STATE_STARTSIEGE || this->m_iCastleSiegeState == CASTLESIEGE_STATE_ENDSIEGE)
		{
			gEffectManager.AddEffect(&gObj[iIndex],0,EFFECT_GUILD_STATE2,0,0,0,0,0);
		}
		else
		{
			gEffectManager.DelEffect(&gObj[iIndex],EFFECT_GUILD_STATE2);
		}
		break;
	case 3:
		iState = 1;
		iViewState = 0x400000;
		if(this->m_iCastleSiegeState == CASTLESIEGE_STATE_STARTSIEGE || this->m_iCastleSiegeState == CASTLESIEGE_STATE_ENDSIEGE)
		{
			gEffectManager.AddEffect(&gObj[iIndex],0,EFFECT_GUILD_STATE3,0,0,0,0,0);
		}
		else
		{
			gEffectManager.DelEffect(&gObj[iIndex],EFFECT_GUILD_STATE3);
		}
		break;
	case 4:
		iState = 1;
		iViewState = 0x800000;
		if(this->m_iCastleSiegeState == CASTLESIEGE_STATE_STARTSIEGE || this->m_iCastleSiegeState == CASTLESIEGE_STATE_ENDSIEGE)
		{
			gEffectManager.AddEffect(&gObj[iIndex],0,EFFECT_GUILD_STATE4,0,0,0,0,0);
		}
		else
		{
			gEffectManager.DelEffect(&gObj[iIndex],EFFECT_GUILD_STATE4);
		}
		break;
	case 0:
		iState = 0;
		iViewState = 0xC0000;
		gEffectManager.DelEffect(&gObj[iIndex],EFFECT_GUILD_STATE1);
		gEffectManager.DelEffect(&gObj[iIndex],EFFECT_GUILD_STATE2);
		gEffectManager.DelEffect(&gObj[iIndex],EFFECT_GUILD_STATE3);
		gEffectManager.DelEffect(&gObj[iIndex],EFFECT_GUILD_STATE4);
	}

	pMsg.h.set(0xB2,0x19,sizeof(pMsg));

	if(btCsJoinSide == 2)
	{
		pMsg.btCsJoinSide = gObj[iIndex].CsJoinSide;
	}
	else
	{
		pMsg.btCsJoinSide = btCsJoinSide;
	}

	DataSend(iIndex,(LPBYTE)&pMsg,pMsg.h.size);
}

int CCastleSiege::OperateGate(int iObjIndex, int iMonsterExistVal, int bOpenType)
{
	if( gObjIsConnected(iObjIndex) == FALSE )
	{
		return FALSE;
	}

	BOOL bRETVAL = FALSE;

	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();

	while( it != this->m_vtNpcData.end() )
	{
		_CS_NPC_DATA & NpcData = _CS_NPC_DATA(*it);

		if( NpcData.m_iNPC_NUM == 277 && NpcData.m_iNPC_OBJINDEX == iObjIndex && NpcData.m_bIN_USE == TRUE && NpcData.m_iNPC_LIVE == 2 )
		{
			int iObjNumber = NpcData.m_iNPC_OBJINDEX;

			if( gObj[iObjNumber].CsNpcType != 0 && iMonsterExistVal == NpcData.m_iCsNpcExistVal )
			{
				this->SetGateBlockState(NpcData.m_iNPC_SX, NpcData.m_iNPC_SY, bOpenType);
				gEffectManager.AddEffect(&gObj[iObjNumber],0,EFFECT_CASTLE_GATE_STATE,0,0,0,0,0);
				gObj[iObjNumber].CsGateOpen = bOpenType;
				bRETVAL = TRUE;
			}
			break;
		}
		it++;
	}

	LeaveCriticalSection(&this->m_critNpcData);

	return bRETVAL;
}

int CCastleSiege::OperateGate(int iNpcIndex, int bOpenType)
{
	BOOL bRETVAL = FALSE;

	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();

	while( it != this->m_vtNpcData.end() )
	{
		_CS_NPC_DATA & NpcData = _CS_NPC_DATA(*it);

		if( NpcData.m_iNPC_NUM == 277 && NpcData.m_iNPC_INDEX == iNpcIndex && NpcData.m_bIN_USE == TRUE && NpcData.m_iNPC_LIVE == 2 )
		{
			int iObjNumber = NpcData.m_iNPC_OBJINDEX;

			if( gObjIsConnected(iObjNumber) && gObj[iObjNumber].CsNpcType != 0 && gObj[iObjNumber].CsNpcExistVal == NpcData.m_iCsNpcExistVal && gObj[iObjNumber].CsNpcExistVal != 0 )
			{
					this->SetGateBlockState(NpcData.m_iNPC_SX, NpcData.m_iNPC_SY, bOpenType);
					
					if(bOpenType == 0)
					{
						gEffectManager.DelEffect(&gObj[iObjNumber],EFFECT_CASTLE_GATE_STATE);
					}
					else
					{
						gEffectManager.AddEffect(&gObj[iObjNumber],0,EFFECT_CASTLE_GATE_STATE,0,0,0,0,0);
					}
					
					gObj[iObjNumber].CsGateOpen = bOpenType;
					bRETVAL = TRUE;
			}
			break;
		}
		it++;
	}

	LeaveCriticalSection(&this->m_critNpcData);

	return bRETVAL;
}

int CCastleSiege::CheckAttackGuildExist()
{
	int iRETVAL = FALSE;

	EnterCriticalSection(&this->m_critCsTotalGuildInfo);

	if(!this->m_mapCsTotalGuildInfo.empty())
	{
		iRETVAL = TRUE;
	}

	LeaveCriticalSection(&this->m_critCsTotalGuildInfo);
	return iRETVAL;
}

BOOL CCastleSiege::CheckCastleOwnerMember(int iIndex)
{
	if( gObjIsConnected(iIndex) == FALSE )
	{
		return FALSE;
	}

	if( strcmp(this->m_szCastleOwnerGuild, "") == 0 )
	{
		return FALSE;
	}

	if( strcmp(gObj[iIndex].GuildName, "") == 0 )
	{
		return FALSE;
	}

	if( strcmp(gObj[iIndex].GuildName, this->m_szCastleOwnerGuild) != 0 )
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CCastleSiege::CheckCastleOwnerUnionMember(int iIndex)
{
	GUILD_INFO_STRUCT* lpGuildInfo;
	CUnionInfo * pUnionInfo;

	if( ::gObjIsConnected(iIndex) == FALSE )
	{
		return FALSE;
	}

	if( strcmp(this->m_szCastleOwnerGuild, "") == 0 )
	{
		return FALSE;
	}

	if( strcmp(gObj[iIndex].GuildName, "") == 0 )
	{
		return FALSE;
	}

	lpGuildInfo = gObj[iIndex].Guild;

	if( lpGuildInfo == NULL )
	{
		return FALSE;
	}

	pUnionInfo = gUnionManager.SearchUnion(lpGuildInfo->GuildUnion);

	if( pUnionInfo == NULL )
	{
		return FALSE;
	}

	if( strcmp(pUnionInfo->m_szMasterGuild, this->m_szCastleOwnerGuild) == 0 )
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CCastleSiege::CheckGuardianStatueExist()
{
	for( int iIDX = 0; iIDX < MAX_OBJECT_MONSTER; iIDX++ )
	{
		if( gObj[iIDX].Class == 283 )
		{
			if( gObj[iIDX].Connected > OBJECT_LOGGED && gObj[iIDX].Live == 1 )
			{
				if( gObj[iIDX].Map == MAP_CASTLE_SIEGE )
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

void CCastleSiege::ChangeWinnerGuild(int iCsJoinSide)
{
	char szGuildTo[16] = {0};
	char szGuildFrom[16] = {0};

	EnterCriticalSection(&this->m_critCsTotalGuildInfo);

	std::map<std::string,_CS_TOTAL_GUILD_DATA>::iterator it = this->m_mapCsTotalGuildInfo.begin();
	
	for(; it != this->m_mapCsTotalGuildInfo.end(); it++)
	{
		if(it->second.m_iCsGuildID == iCsJoinSide)
		{
			it->second.m_iCsGuildID = 1;
		
			if(it->second.m_bGuildInvolved == TRUE)
			{
				memcpy(&szGuildTo,&it->first[0], 8);
			}
		}
		else if(it->second.m_iCsGuildID == 1)
		{
			it->second.m_iCsGuildID = iCsJoinSide;
		
			if(it->second.m_bGuildInvolved == TRUE)
			{
				memcpy(&szGuildFrom,&it->first[0], 8);
			}
		}
	}

	for( int iUSER = OBJECT_START_USER; iUSER < MAX_OBJECT; iUSER++ )
	{
		if(!gObjIsConnected(iUSER))
		{
			continue;
		}

		char szNewGuild[16] = {0};

		memcpy(&szNewGuild,gObj[iUSER].GuildName,8);

		std::map<std::string,_CS_TOTAL_GUILD_DATA>::iterator it = this->m_mapCsTotalGuildInfo.find(szNewGuild);
	
		if( it != this->m_mapCsTotalGuildInfo.end() )
		{
			gObj[iUSER].CsJoinSide = it->second.m_iCsGuildID;
			gObj[iUSER].CsGuildInvolved = (it->second.m_bGuildInvolved != 0);
		}
		gObj[iUSER].AccumulatedCrownAccessTime = 0;
	}

	LeaveCriticalSection(&this->m_critCsTotalGuildInfo);

	LogAdd(LOG_BLACK,"[CastleSiege] Castle Winner Guild Change From '%s' To '%s'", szGuildFrom, szGuildTo);
}

BOOL CCastleSiege::CheckMiddleWinnerGuild()
{
	if( this->m_dwCrownAccessTime == 0 )
	{
		return FALSE;
	}

	if( (gObjIsConnected(this->m_iCastleCrownAccessUser) == FALSE) || (gObjIsConnected(this->m_iCastleSwitchAccessUser1) == FALSE) || (gObjIsConnected(this->m_iCastleSwitchAccessUser2) == FALSE) )
	{
		return FALSE;
	}

	if( (strcmp(gObj[this->m_iCastleCrownAccessUser].GuildName, "") == 0) || (strcmp(gObj[this->m_iCastleSwitchAccessUser1].GuildName, "") == 0) || (strcmp(gObj[this->m_iCastleSwitchAccessUser2].GuildName, "") == 0) )
	{
		return FALSE;
	}

	if( (gObj[this->m_iCastleCrownAccessUser].CsJoinSide >= 2 ) && (gObj[this->m_iCastleSwitchAccessUser1].CsJoinSide >= 2 ) && (gObj[this->m_iCastleSwitchAccessUser2].CsJoinSide >= 2 ) )
	{
		if( gObj[this->m_iCastleCrownAccessUser].CsGuildInvolved == 1 )
		{
			if( (gObj[this->m_iCastleCrownAccessUser].CsJoinSide == gObj[this->m_iCastleSwitchAccessUser1].CsJoinSide) && (gObj[this->m_iCastleCrownAccessUser].CsJoinSide == gObj[this->m_iCastleSwitchAccessUser2].CsJoinSide) )
			{
				if( ( ( GetTickCount() - this->m_dwCrownAccessTime ) + gObj[this->m_iCastleCrownAccessUser].AccumulatedCrownAccessTime ) >= 60000 )
				{
					LogAdd(LOG_BLACK,"[CastleSiege] Castle Winner Has been Changed (GUILD:%s)", gObj[this->m_iCastleCrownAccessUser].GuildName);

					GCAnsCsAccessCrownState(this->m_iCastleCrownAccessUser, 1);

					this->NotifyCrownState(2);

					memset(&this->m_szMiddleWinnerGuild, 0, sizeof(this->m_szMiddleWinnerGuild));
					memcpy(&this->m_szMiddleWinnerGuild, gObj[this->m_iCastleCrownAccessUser].GuildName, 8);

					this->ClearNonDbNPC_MidWin();
					this->ChangeWinnerGuild(gObj[this->m_iCastleCrownAccessUser].CsJoinSide);
					this->NotifyAllUserCsProgState(1, gObj[this->m_iCastleCrownAccessUser].GuildName);
					this->ResetCrownAccessTickCount();
					this->ResetCrownUserIndex();
					this->SetCrownAccessUserX(0);
					this->SetCrownAccessUserY(0);
					this->ResetCrownSwitchUserIndex(217);
					this->ResetCrownSwitchUserIndex(218);
					this->m_bRegCrownAvailable = FALSE;
					this->ReSpawnEnemyUser(FALSE);
	
					GCAnsCsLeftTimeAlarm( (this->m_iCS_REMAIN_MSEC/1000)/3600, ((this->m_iCS_REMAIN_MSEC/1000)%3600)/60);

					this->m_dwCS_LEFTTIME_TICK_COUNT = GetTickCount();

					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

int CCastleSiege::CheckCastleSiegeResult()
{
	char szMsg[256] = {0};
	int bRETVAL = FALSE;

	if(this->m_btIsCastleOccupied == TRUE && strcmp(this->m_szCastleOwnerGuild,"") != FALSE)
	{
		if(strcmp(this->m_szMiddleWinnerGuild,"") == FALSE || strcmp(this->m_szMiddleWinnerGuild,this->m_szCastleOwnerGuild) == FALSE)
		{
			this->m_btIsCastleOccupied = 1;
			wsprintf(szMsg,gMessage.GetMessage(424),this->m_szCastleOwnerGuild);
			bRETVAL = FALSE;
		}
		else
		{
			this->m_btIsCastleOccupied = 1;
			memset(this->m_szCastleOwnerGuild,0,sizeof(this->m_szCastleOwnerGuild));
			memcpy(this->m_szCastleOwnerGuild,this->m_szMiddleWinnerGuild,8);
			wsprintf(szMsg,gMessage.GetMessage(425),this->m_szCastleOwnerGuild);
			bRETVAL = TRUE;
		}
	}
	else
	{
		if(strcmp(this->m_szMiddleWinnerGuild,"") == FALSE)
		{
			this->m_btIsCastleOccupied = 0;
			wsprintf(szMsg,gMessage.GetMessage(426));
			bRETVAL = FALSE;
		}
		else
		{
			this->m_btIsCastleOccupied = 1;
			memset(this->m_szCastleOwnerGuild,0,sizeof(this->m_szCastleOwnerGuild));
			memcpy(this->m_szCastleOwnerGuild,this->m_szMiddleWinnerGuild,8);
			wsprintf(szMsg,gMessage.GetMessage(425),this->m_szCastleOwnerGuild);
			bRETVAL = TRUE;
		}
	}

	this->m_btIsSiegeEnded = 1;

	this->SendAllUserAnyMsg(szMsg,2);

	return bRETVAL;
}

BOOL CCastleSiege::CheckUnionGuildMaster(int iIndex)
{
	GUILD_INFO_STRUCT* lpGuildInfo;

	if( gObjIsConnected(iIndex) == FALSE )
	{
		return FALSE;
	}

	lpGuildInfo = gObj[iIndex].Guild;

	if( lpGuildInfo == NULL )
	{
		return FALSE;
	}

	if( gObj[iIndex].GuildStatus >= 32 ) // Testar CS
	{
		if( (lpGuildInfo->Number == lpGuildInfo->GuildUnion) || (lpGuildInfo->GuildUnion == 0) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CCastleSiege::SetCrownSwitchUserIndex(int iMonsterClass, int iIndex)
{
	switch( iMonsterClass )
	{
	case 217:
			this->m_iCastleSwitchAccessUser1 = iIndex;
		break;
	case 218:
			this->m_iCastleSwitchAccessUser2 = iIndex;
		break;
	}
}

int CCastleSiege::GetCrownSwitchUserIndex(int iMonsterClass)
{
	int iUserIndex = -1;

	switch( iMonsterClass )
	{
	case 217:
			iUserIndex = this->m_iCastleSwitchAccessUser1;
		break;
	case 218:
			iUserIndex = this->m_iCastleSwitchAccessUser2;
		break;
	}

	return iUserIndex;
}

void CCastleSiege::ResetCrownSwitchUserIndex(int iMonsterClass)
{
	switch( iMonsterClass )
	{
	case 217:
			this->m_iCastleSwitchAccessUser1 = -1;
		break;
	case 218:
			this->m_iCastleSwitchAccessUser2 = -1;
		break;
	}
}

void CCastleSiege::SetCrownAccessTickCount()
{
	this->m_dwCrownAccessTime = GetTickCount();
}

void CCastleSiege::ResetCrownAccessTickCount()
{
	this->m_dwCrownAccessTime = 0;
}

int CCastleSiege::CheckOverlapCsMarks(int iIndex)
{
	for( int x = INVENTORY_WEAR_SIZE; x < INVENTORY_MAIN_SIZE; x++ )
	{
		if( gObj[iIndex].Inventory[x].IsItem() == TRUE )
		{
			if( (gObj[iIndex].Inventory[x].m_Index == GET_ITEM(14,21)) && (gObj[iIndex].Inventory[x].m_Level == 3) )
			{
				int dur = (int)gObj[iIndex].Inventory[x].m_Durability;

				if(dur >= 0 && dur <= 255)
				{
					return x;
				}
			}
		}
	}
	return -1;
}

int CCastleSiege::GetCsAttkGuildList(PMSG_CSATTKGUILDLIST* lpMsgBody, int & iCount)
{
	iCount = 0;

	if( lpMsgBody == NULL ) return 0;

	if( (this->m_iCastleSiegeState != CASTLESIEGE_STATE_NOTIFY) && (this->m_iCastleSiegeState != CASTLESIEGE_STATE_READYSIEGE) )
	{
		return 2;
	}

	if( (this->m_bCsBasicGuildInfoLoadOK == FALSE) || (this->m_bCsTotalGuildInfoLoadOK == FALSE ) )
	{
		return 3;
	}

	EnterCriticalSection(&this->m_critCsTotalGuildInfo);

	std::map<std::string,_CS_TOTAL_GUILD_DATA>::iterator it = this->m_mapCsTotalGuildInfo.begin();

	for(;  it != this->m_mapCsTotalGuildInfo.end(); it++)
	{
		lpMsgBody[iCount].btCsJoinSide = it->second.m_iCsGuildID;
		lpMsgBody[iCount].btGuildInvolved = it->second.m_bGuildInvolved;
		
		memset(&lpMsgBody[iCount].szGuildName, 0, sizeof(lpMsgBody[iCount].szGuildName));
		memcpy(&lpMsgBody[iCount].szGuildName, &it->first[0], sizeof(lpMsgBody[iCount].szGuildName));

		lpMsgBody[iCount].iGuildScore = it->second.m_iGuildScore;

		iCount++;
	}

	LeaveCriticalSection(&this->m_critCsTotalGuildInfo);

	return 1;
}

BOOL CCastleSiege::CheckTeleportMagicAxisY(int iStartY, int iTargetX, int iTargetY)
{
	for( int iSTEP = 0; iSTEP < 3; iSTEP++ )
	{
		if( (g_iCastleWallStepAxisY[iSTEP] >= iStartY) && (g_iCastleWallStepAxisY[iSTEP] <= iTargetY) && (iTargetX < 144) )
		{
			return FALSE;
		}
	}
	return TRUE;
}

void CCastleSiege::NotifyCsSelfLeftTime(int iIndex)
{
	GCAnsSelfCsLeftTimeAlarm(iIndex, (this->m_iCS_REMAIN_MSEC/1000)/3600, ((this->m_iCS_REMAIN_MSEC/1000)%3600)/60);
}

void CCastleSiege::CheckReviveNonDbNPC()
{
	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();

	while(it != this->m_vtNpcData.end())
	{
		_CS_NPC_DATA & pNpcData = _CS_NPC_DATA(*it);

		if(pNpcData.m_bNPC_DBSAVE == 0)
		{
			int iNPC_INDEX = gObjAddMonster(MAP_CASTLE_SIEGE);

			if(iNPC_INDEX >= 0)
			{
				gObjSetMonster(iNPC_INDEX,pNpcData.m_iNPC_NUM);

				gObj[iNPC_INDEX].PosNum = -1;
				gObj[iNPC_INDEX].X = pNpcData.m_iNPC_SX;
				gObj[iNPC_INDEX].Y = pNpcData.m_iNPC_SY;
				gObj[iNPC_INDEX].Map = MAP_CASTLE_SIEGE;
				gObj[iNPC_INDEX].TX = gObj[iNPC_INDEX].X;
				gObj[iNPC_INDEX].TY = gObj[iNPC_INDEX].Y;
				gObj[iNPC_INDEX].OldX = gObj[iNPC_INDEX].X;
				gObj[iNPC_INDEX].OldY = gObj[iNPC_INDEX].Y;
				gObj[iNPC_INDEX].Dir = pNpcData.m_iNPC_DIR;
				gObj[iNPC_INDEX].StartX = (BYTE)gObj[iNPC_INDEX].X;
				gObj[iNPC_INDEX].StartY = (BYTE)gObj[iNPC_INDEX].Y;
				gObj[iNPC_INDEX].DieRegen = 0;
				gObj[iNPC_INDEX].MaxRegenTime = 0;
				gObj[iNPC_INDEX].Life = (float)pNpcData.m_iNPC_HP;
				gObj[iNPC_INDEX].MaxLife = (float)pNpcData.m_iNPC_MAXHP;

				switch(pNpcData.m_iNPC_NUM)
				{
				case 215:
					gObj[iNPC_INDEX].CsNpcType = 2;
					break;
				case 221:
					gObj[iNPC_INDEX].CsNpcType = 3;
					break;
				case 222:
					gObj[iNPC_INDEX].CsNpcType = 2;
					break;
				case 288:
					gObj[iNPC_INDEX].CsNpcType = 2;
					break;
				default:
					gObj[iNPC_INDEX].CsNpcType = 2;
					break;
				}

				it->m_bIN_USE = TRUE;
			}
		}

		it++;
	}

	LeaveCriticalSection(&this->m_critNpcData);
}

void CCastleSiege::CheckReviveGuardianStatue()
{
	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();

	while(it != this->m_vtNpcData.end())
	{
		_CS_NPC_DATA & pNpcData = _CS_NPC_DATA(*it);

		if(pNpcData.m_iNPC_NUM == 283 && pNpcData.m_iNPC_LIVE == 0 )
		{
			int iNPC_INDEX = gObjAddMonster(MAP_CASTLE_SIEGE);

			if(iNPC_INDEX >= 0)
			{
				gObjSetMonster(iNPC_INDEX,pNpcData.m_iNPC_NUM);

				gObj[iNPC_INDEX].PosNum = -1;
				gObj[iNPC_INDEX].X = pNpcData.m_iNPC_SX;
				gObj[iNPC_INDEX].Y = pNpcData.m_iNPC_SY;
				gObj[iNPC_INDEX].Map = MAP_CASTLE_SIEGE;
				gObj[iNPC_INDEX].TX = gObj[iNPC_INDEX].X;
				gObj[iNPC_INDEX].TY = gObj[iNPC_INDEX].Y;
				gObj[iNPC_INDEX].OldX = gObj[iNPC_INDEX].X;
				gObj[iNPC_INDEX].OldY = gObj[iNPC_INDEX].Y;
				gObj[iNPC_INDEX].Dir = pNpcData.m_iNPC_DIR;
				gObj[iNPC_INDEX].StartX = (BYTE)gObj[iNPC_INDEX].X;
				gObj[iNPC_INDEX].StartY = (BYTE)gObj[iNPC_INDEX].Y;
				gObj[iNPC_INDEX].DieRegen = 0;
				gObj[iNPC_INDEX].MaxRegenTime = 0;
				gObj[iNPC_INDEX].Life = (float)pNpcData.m_iNPC_HP;
				gObj[iNPC_INDEX].MaxLife = (float)pNpcData.m_iNPC_MAXHP;
				gObj[iNPC_INDEX].CsNpcType = 1;

				int iNPC_DF_LEVEL = pNpcData.m_iNPC_BASE_DF_LEVEL;

				if(iNPC_DF_LEVEL < 0)
				{
					iNPC_DF_LEVEL = 0;
				}

				if(iNPC_DF_LEVEL > CS_MAX_UPGRADE)
				{
					iNPC_DF_LEVEL = CS_MAX_UPGRADE;
				}

				int iNPC_RG_LEVEL = pNpcData.m_iNPC_BASE_RG_LEVEL;

				if(iNPC_RG_LEVEL < 0)
				{
					iNPC_RG_LEVEL = 0;
				}

				if(iNPC_RG_LEVEL > CS_MAX_UPGRADE)
				{
					iNPC_RG_LEVEL = CS_MAX_UPGRADE;
				}

				gObj[iNPC_INDEX].Defense = g_iNpcDefense_CSTATUE[iNPC_DF_LEVEL];
				gObj[iNPC_INDEX].CsNpcDfLevel = iNPC_DF_LEVEL;
				gObj[iNPC_INDEX].CsNpcRgLevel = iNPC_RG_LEVEL;

				it->m_btCsNpcExistVal1 = GetLargeRand()%255+1;
				gObj[iNPC_INDEX].CsNpcExistVal1 = it->m_btCsNpcExistVal1;

				it->m_btCsNpcExistVal2 = GetLargeRand()%256;
				gObj[iNPC_INDEX].CsNpcExistVal2 = it->m_btCsNpcExistVal2;

				it->m_btCsNpcExistVal3 = GetLargeRand()%256;
				gObj[iNPC_INDEX].CsNpcExistVal3 = it->m_btCsNpcExistVal3;

				it->m_btCsNpcExistVal4 = GetLargeRand()%256;
				gObj[iNPC_INDEX].CsNpcExistVal4 = it->m_btCsNpcExistVal4;

				it->m_iNPC_OBJINDEX = iNPC_INDEX;
				it->m_iNPC_LIVE = 2;
				it->m_bIN_USE = TRUE;
			}
		}
		it++;
	}

	LeaveCriticalSection(&this->m_critNpcData);
}

void CCastleSiege::ReSpawnAllUser()
{
	int iTX, iTY;
	BYTE btMapAttr;

	for( int iUSER = OBJECT_START_USER; iUSER < MAX_OBJECT; iUSER++ )
	{
		if( gObjIsConnected(iUSER) == FALSE )
		{
			continue;
		}

		if( gObj[iUSER].Map != MAP_CASTLE_SIEGE )
		{
			continue;
		}

		BYTE btJoinSide = gObj[iUSER].CsJoinSide;

		if( btJoinSide < 0 ) 
		{
			btJoinSide = 0;
		}

		if( btJoinSide > 2 )
		{
			btJoinSide = 2;
		}

		BOOL bMoveOK = FALSE;

		switch( btJoinSide )
		{
		case 1:
			{
				for(int iCNT = 0; iCNT < 100; iCNT++)
				{
					iTX = (GetLargeRand() % (g_iCsUserReSpawnArea[1][2] - g_iCsUserReSpawnArea[1][0])) + g_iCsUserReSpawnArea[1][0];
					iTY = (GetLargeRand() % (g_iCsUserReSpawnArea[1][3] - g_iCsUserReSpawnArea[1][1])) + g_iCsUserReSpawnArea[1][1];

					btMapAttr = gMap[MAP_CASTLE_SIEGE].m_MapAttr[iTY * 256 + iTX];

					if((btMapAttr & 4) != 4)
					{
						bMoveOK = TRUE;
						break;
					}
				}
			}
			break;
		case 2:
			{
				for(int iCNT = 0; iCNT < 100; iCNT++)
				{
					iTX = (GetLargeRand() % (g_iCsUserReSpawnArea[2][2] - g_iCsUserReSpawnArea[2][0])) + g_iCsUserReSpawnArea[2][0];
					iTY = (GetLargeRand() % (g_iCsUserReSpawnArea[2][3] - g_iCsUserReSpawnArea[2][1])) + g_iCsUserReSpawnArea[2][1];

					btMapAttr = gMap[MAP_CASTLE_SIEGE].m_MapAttr[iTY * 256 + iTX];

					if((btMapAttr & 4) != 4)
					{
						bMoveOK = TRUE;
						break;
					}
				}
			}
			break;
		default:
			{
				for(int iCNT = 0; iCNT < 100; iCNT++)
				{
					iTX = (GetLargeRand() % (g_iCsUserReSpawnArea[0][2] - g_iCsUserReSpawnArea[0][0])) + g_iCsUserReSpawnArea[0][0];
					iTY = (GetLargeRand() % (g_iCsUserReSpawnArea[0][3] - g_iCsUserReSpawnArea[0][1])) + g_iCsUserReSpawnArea[0][1];

					btMapAttr = gMap[MAP_CASTLE_SIEGE].m_MapAttr[iTY * 256 + iTX];

					if((btMapAttr & 4) != 4)
					{
						bMoveOK = TRUE;
						break;
					}
				}
			}
			break;
		}

		if( bMoveOK )
		{
			gObj[iUSER].State = 32;
			gObj[iUSER].X = iTX;
			gObj[iUSER].Y = iTY;
			gObj[iUSER].TX = iTX;
			gObj[iUSER].TX = iTY;
			gObj[iUSER].Map = MAP_CASTLE_SIEGE;
			gObj[iUSER].PathCount = 0;
			gObj[iUSER].Teleport = 0;

			gObjClearViewport(&gObj[iUSER]);

			gMove.GCTeleportSend(iUSER, -1, MAP_CASTLE_SIEGE, (BYTE)gObj[iUSER].X, (BYTE)gObj[iUSER].Y, gObj[iUSER].Dir);

			gObjViewportListProtocolCreate(&gObj[iUSER]);

			gObjectManager.CharacterUpdateMapEffect(&gObj[iUSER]);

			gObj[iUSER].RegenMapNumber = MAP_CASTLE_SIEGE;
			gObj[iUSER].RegenMapX = iTX;
			gObj[iUSER].RegenMapY = iTY;
			gObj[iUSER].RegenOk = TRUE;
		}
	}
}

void CCastleSiege::ReSpawnEnemyUser(int bRefreshOwnerUser)
{
	int iTX;
	int iTY;
	BYTE btMapAttr;

	for(int iUSER = OBJECT_START_USER; iUSER < MAX_OBJECT; iUSER++)
	{
		if(gObjIsConnected(iUSER) == FALSE)
		{
			continue;
		}

		if(gObj[iUSER].Map != MAP_CASTLE_SIEGE && gObj[iUSER].Map != MAP_LAND_OF_TRIALS)
		{
			continue;
		}

		BYTE btCsJoinSide = gObj[iUSER].CsJoinSide;

		if(btCsJoinSide < 0)
		{
			btCsJoinSide = 0;
		}

		if(btCsJoinSide > 2)
		{
			btCsJoinSide = 2;
		}

		int bMoveOK = FALSE;

		switch(btCsJoinSide)
		{
		case 1:
			if(bRefreshOwnerUser != FALSE)
			{
				gObj[iUSER].RegenMapNumber = gObj[iUSER].Map;
				gObj[iUSER].RegenMapX = (BYTE)gObj[iUSER].X;
				gObj[iUSER].RegenMapY = (BYTE)gObj[iUSER].Y;

				if(gObj[iUSER].Map == MAP_LAND_OF_TRIALS)
				{
					for(int iCNT = 0; iCNT < 100; iCNT++)
					{
						iTX = (GetLargeRand() % (g_iCsUserReSpawnArea[2][2] - g_iCsUserReSpawnArea[2][0])) + g_iCsUserReSpawnArea[2][0];
						iTY = (GetLargeRand() % (g_iCsUserReSpawnArea[2][3] - g_iCsUserReSpawnArea[2][1])) + g_iCsUserReSpawnArea[2][1];

						btMapAttr = gMap[MAP_CASTLE_SIEGE].m_MapAttr[iTY * 256 + iTX];

						if((btMapAttr & 4) != 4)
						{
							bMoveOK = TRUE;
							break;
						}
					}
					break;
				}

				gObjClearViewport(&gObj[iUSER]);

				gMove.GCTeleportSend(iUSER,-1,gObj[iUSER].Map,(BYTE)gObj[iUSER].X,(BYTE)gObj[iUSER].Y,gObj[iUSER].Dir);

				gObjViewportListProtocolCreate(&gObj[iUSER]);

				gObjectManager.CharacterUpdateMapEffect(&gObj[iUSER]);

				gObj[iUSER].RegenOk = TRUE;
			}
			bMoveOK = FALSE;
			break;
		case 2:
			{
				for(int iCNT = 0; iCNT < 100; iCNT++)
				{
					iTX = (GetLargeRand() % (g_iCsUserReSpawnArea[2][2] - g_iCsUserReSpawnArea[2][0])) + g_iCsUserReSpawnArea[2][0];
					iTY = (GetLargeRand() % (g_iCsUserReSpawnArea[2][3] - g_iCsUserReSpawnArea[2][1])) + g_iCsUserReSpawnArea[2][1];

					btMapAttr = gMap[MAP_CASTLE_SIEGE].m_MapAttr[iTY * 256 + iTX];

					if((btMapAttr & 4) != 4)
					{
						bMoveOK = TRUE;
						break;
					}
				}
			}
			break;
		case 0:
			{
				for(int iCNT = 0; iCNT < 100; iCNT++)
				{
					iTX = (GetLargeRand()% (g_iCsUserReSpawnArea[0][2] - g_iCsUserReSpawnArea[0][0])) + g_iCsUserReSpawnArea[0][0];
					iTY = (GetLargeRand()% (g_iCsUserReSpawnArea[0][3] - g_iCsUserReSpawnArea[0][1])) + g_iCsUserReSpawnArea[0][1];

					btMapAttr = gMap[MAP_CASTLE_SIEGE].m_MapAttr[iTY * 256 + iTX];

					if((btMapAttr & 4) != 4)
					{
						bMoveOK = TRUE;
						break;
					}
				}
			}
			break;
		}

		if(bMoveOK != FALSE)
		{
			gObj[iUSER].State = 32;
			gObj[iUSER].X = iTX;
			gObj[iUSER].Y = iTY;
			gObj[iUSER].TX = iTX;
			gObj[iUSER].TX = iTY;
			gObj[iUSER].Map = MAP_CASTLE_SIEGE;
			gObj[iUSER].PathCount = 0;
			gObj[iUSER].Teleport = 0;

			gObjClearViewport(&gObj[iUSER]);

			gMove.GCTeleportSend(iUSER,-1,MAP_CASTLE_SIEGE,(BYTE)gObj[iUSER].X,(BYTE)gObj[iUSER].Y,gObj[iUSER].Dir);

			gObjViewportListProtocolCreate(&gObj[iUSER]);

			gObjectManager.CharacterUpdateMapEffect(&gObj[iUSER]);

			gObj[iUSER].RegenMapNumber = MAP_CASTLE_SIEGE;
			gObj[iUSER].RegenMapX = iTX;
			gObj[iUSER].RegenMapY = iTY;
			gObj[iUSER].RegenOk = TRUE;
		}

		gObj[iUSER].AccumulatedCrownAccessTime = 0;
	}
}

void CCastleSiege::NotifyAllUserCsStartState(BYTE btStartState)
{
	PMSG_ANS_NOTIFYCSSTART pMsg;

	pMsg.h.set(0xB2,0x17,sizeof(pMsg));
	pMsg.btStartState = btStartState;

	for(int i = OBJECT_START_USER; i < MAX_OBJECT; i++)
	{
		if(gObj[i].Connected == OBJECT_ONLINE && gObj[i].Type == OBJECT_USER)
		{
			DataSend(i,(LPBYTE)&pMsg,pMsg.h.size);
		}
	}
}

void CCastleSiege::NotifyAllUserCsProgState(BYTE btProgState, char* lpszGuildName)
{
	PMSG_ANS_NOTIFYCSPROGRESS pMsg;
	
	pMsg.h.set(0xB2, 0x18, sizeof(pMsg));

	pMsg.btCastleSiegeState = btProgState;

	memset(&pMsg.szGuildName, 0, sizeof(pMsg.szGuildName));
	memcpy(&pMsg.szGuildName, lpszGuildName, sizeof(pMsg.szGuildName));

	for( int i = OBJECT_START_USER; i < MAX_OBJECT; i++ )
	{
		if( (gObj[i].Connected == OBJECT_ONLINE) && (gObj[i].Type == OBJECT_USER) )
		{
			if( gObj[i].Map == MAP_CASTLE_SIEGE )
			{
				DataSend(i, (LPBYTE)&pMsg, pMsg.h.size);
			}
		}
	}
}

void CCastleSiege::ClearCastleTowerBarrier()
{
	for( int iIDX = 0; iIDX < MAX_OBJECT_MONSTER; iIDX++ )
	{
		if( ::gObjIsConnected(iIDX) == FALSE )
		{
			continue;
		}

		if( gObj[iIDX].Class == 215 )
		{
			gObjDel(iIDX);
		}
	}
}

void CCastleSiege::SetAllCastleGateState(BOOL bOpenType)
{
	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();

	BOOL bExist = FALSE;

	while(it != m_vtNpcData.end())
	{
		_CS_NPC_DATA & pNpcData = _CS_NPC_DATA(*it);

		if(pNpcData.m_iNPC_NUM == 277 && gObjIsConnected(pNpcData.m_iNPC_OBJINDEX) && gObj[pNpcData.m_iNPC_OBJINDEX].Class == 277)
		{
			switch(bOpenType)
			{
			case 0:
				this->SetGateBlockState(pNpcData.m_iNPC_SX,pNpcData.m_iNPC_SY,0);
				gEffectManager.DelEffect(&gObj[pNpcData.m_iNPC_OBJINDEX],EFFECT_CASTLE_GATE_STATE);
				gObj[pNpcData.m_iNPC_OBJINDEX].CsGateOpen = 0;
				break;
			case 1:
				this->SetGateBlockState(pNpcData.m_iNPC_SX,pNpcData.m_iNPC_SY,1);
				gEffectManager.AddEffect(&gObj[pNpcData.m_iNPC_OBJINDEX],0,EFFECT_CASTLE_GATE_STATE,0,0,0,0,0);
				gObj[pNpcData.m_iNPC_OBJINDEX].CsGateOpen = 1;
				break;
			}
		}

		it++;
	}

	LeaveCriticalSection(&m_critNpcData);
}

void CCastleSiege::AddMiniMapDataReqUser(int iIndex)
{
	EnterCriticalSection(&this->m_critCsMiniMap);
		
	if( std::find(this->m_vtMiniMapReqUser.begin(), this->m_vtMiniMapReqUser.end(), iIndex) == this->m_vtMiniMapReqUser.end() )
	{
		this->m_vtMiniMapReqUser.push_back(iIndex);
	}

	LeaveCriticalSection(&this->m_critCsMiniMap);
}

void CCastleSiege::DelMiniMapDataReqUser(int iIndex)
{
	EnterCriticalSection(&this->m_critCsMiniMap);
	
	std::vector<int>::iterator it;
	
	if( ( it = std::find(this->m_vtMiniMapReqUser.begin(), this->m_vtMiniMapReqUser.end(), iIndex)) != this->m_vtMiniMapReqUser.end() )
	{
		this->m_vtMiniMapReqUser.erase(it);
	}

	LeaveCriticalSection(&this->m_critCsMiniMap);
}

void CCastleSiege::OperateMiniMapWork()
{
	if( this->m_bCsBasicGuildInfoLoadOK == FALSE )
	{
		return;
	}

	std::map<int, _CS_MINIMAP_DATA> mapMiniMapData;

	BYTE cBUFFER1[2264] = {0};

	PMSG_ANS_SENDMINIMAPDATA* lpMsgSend1 = (PMSG_ANS_SENDMINIMAPDATA*)cBUFFER1;
	PMSG_SENDMINIMAPDATA* lpMsgSendBody1 = (PMSG_SENDMINIMAPDATA*)&cBUFFER1[sizeof(PMSG_ANS_SENDMINIMAPDATA)];

	BYTE cBUFFER2[714] = {0};

	PMSG_ANS_SENDNPCMINIMAPDATA* lpMsgSend2 = (PMSG_ANS_SENDNPCMINIMAPDATA*)cBUFFER2;
	
	PMSG_SENDNPCMINIMAPDATA* lpMsgSendBody2 = (PMSG_SENDNPCMINIMAPDATA*)&cBUFFER2[sizeof(PMSG_ANS_SENDNPCMINIMAPDATA)];

	EnterCriticalSection(&this->m_critCsBasicGuildInfo);

	for(std::map<std::string,_CS_TOTAL_GUILD_DATA>::iterator it = this->m_mapCsBasicGuildInfo.begin();  it != this->m_mapCsBasicGuildInfo.end(); it++)
	{
		_CS_TOTAL_GUILD_DATA* GuildData = &it->second;
		_CS_MINIMAP_DATA MiniMapData;

		memcpy(MiniMapData.m_szGuildName, &it->first[0], 8);

		mapMiniMapData.insert(std::pair<int,_CS_MINIMAP_DATA>(GuildData->m_iCsGuildID, MiniMapData) );
	}

	LeaveCriticalSection(&this->m_critCsBasicGuildInfo);

	if( mapMiniMapData.empty() )
	{
		return;
	}

	EnterCriticalSection(&this->m_critNpcData);

	lpMsgSend2->iCount = 0;

	if( this->m_vtNpcData.empty() == false )
	{
		for(std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin(); it != this->m_vtNpcData.end(); it++ )
		{
			if( lpMsgSend2->iCount >= 150 )
			{
				break;
			}

			_CS_NPC_DATA & NpcData = _CS_NPC_DATA(*it);

			if( (NpcData.m_iNPC_NUM == 277 || NpcData.m_iNPC_NUM == 283) && NpcData.m_iNPC_OBJINDEX != -1 )
			{
				if( !gObjIsConnected(NpcData.m_iNPC_OBJINDEX) || gObj[NpcData.m_iNPC_OBJINDEX].CsNpcExistVal != NpcData.m_iCsNpcExistVal || (gObj[NpcData.m_iNPC_OBJINDEX].Class != 277 && gObj[NpcData.m_iNPC_OBJINDEX].Class != 283) )
				{
					BYTE btNpcType = 0;

					switch( NpcData.m_iNPC_NUM )
					{
					case 277:
						btNpcType = 0;
						break;
					case 283:
						btNpcType = 1;
						break;
					default:
						continue;
						break;
					}

					lpMsgSendBody2[lpMsgSend2->iCount].btNpcType = btNpcType;
					lpMsgSendBody2[lpMsgSend2->iCount].btX = (BYTE)gObj[NpcData.m_iNPC_OBJINDEX].X;
					lpMsgSendBody2[lpMsgSend2->iCount].btY = (BYTE)gObj[NpcData.m_iNPC_OBJINDEX].Y;

					lpMsgSend2->iCount++;
				}
			}
		}
	}

	lpMsgSend2->h.set(0xBB, (lpMsgSend2->iCount * sizeof(PMSG_SENDNPCMINIMAPDATA)) + sizeof(PMSG_ANS_SENDNPCMINIMAPDATA));

	LeaveCriticalSection(&this->m_critNpcData);

	EnterCriticalSection(&this->m_critCsMiniMap);

	this->m_mapMiniMapData.clear();

	std::map<int, _CS_MINIMAP_DATA>::iterator it2( mapMiniMapData.begin() );
	
	for(; it2 != mapMiniMapData.end(); it2++ )
	{
		this->m_mapMiniMapData.insert(std::pair<int, _CS_MINIMAP_DATA>(it2->first, it2->second) );
	}

	for( int iIDX = OBJECT_START_USER; iIDX < MAX_OBJECT; iIDX++ )
	{
		if( gObjIsConnected(iIDX) == FALSE )
		{
			continue;
		}

		if( gObj[iIDX].Map != MAP_CASTLE_SIEGE )
		{
			continue;
		}

		if( gObj[iIDX].CsJoinSide == 0 )
		{
			continue;
		}

		std::map<int, _CS_MINIMAP_DATA>::iterator it3(this->m_mapMiniMapData.find(gObj[iIDX].CsJoinSide));

		if( it3 != this->m_mapMiniMapData.end() )
		{
			if( it3->second.m_iMiniMapPointCount >= 1000 )
			{
				continue;
			}
		
			if( gObj[iIDX].GuildStatus == GUILD_MASTER && strcmp(gObj[iIDX].GuildName, "") != 0 && strcmp(gObj[iIDX].GuildName, it3->second.m_szGuildName) == 0)
			{
				it3->second.m_iGuildMasterIndex = iIDX;
				continue;
			}

			it3->second.m_stMiniMapPoint[it3->second.m_iMiniMapPointCount].btX = (BYTE)gObj[iIDX].X;
			it3->second.m_stMiniMapPoint[it3->second.m_iMiniMapPointCount].btY = (BYTE)gObj[iIDX].Y;
			it3->second.m_iMiniMapPointCount++;
		}
	}

	std::map<int, _CS_MINIMAP_DATA>::iterator it4(this->m_mapMiniMapData.begin());

	for( ; it4 != this->m_mapMiniMapData.end(); it4++ )
	{
		lpMsgSend1->iCount = 0;

		if( gObjIsConnected(it4->second.m_iGuildMasterIndex) == FALSE )
		{
			continue;
		}

		lpMsgSend1->iCount = it4->second.m_iMiniMapPointCount;

		if( lpMsgSend1->iCount > 1000 )
		{
			lpMsgSend1->iCount = 1000;
		}

		memcpy(lpMsgSendBody1, it4->second.m_stMiniMapPoint, (lpMsgSend1->iCount * sizeof(PMSG_SENDMINIMAPDATA)));

		lpMsgSend1->h.set(0xB6, (lpMsgSend1->iCount * sizeof(PMSG_SENDMINIMAPDATA)) + sizeof(PMSG_ANS_SENDMINIMAPDATA));

		DataSend(it4->second.m_iGuildMasterIndex, cBUFFER1, (lpMsgSend1->iCount * sizeof(PMSG_SENDMINIMAPDATA)) + sizeof(PMSG_ANS_SENDMINIMAPDATA));
		DataSend(it4->second.m_iGuildMasterIndex, cBUFFER2, (lpMsgSend2->iCount * sizeof(PMSG_SENDNPCMINIMAPDATA)) + sizeof(PMSG_ANS_SENDNPCMINIMAPDATA));

	}

	LeaveCriticalSection(&this->m_critCsMiniMap);
}

void CCastleSiege::NotifyCrownState(BYTE btState)
{
	if( gObjIsConnected(m_iCrownIndex) == FALSE )
	{
		LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::NotifyCrownState() - m_iCrownIndex isn't Valid");
		return;
	}

	if( gObj[m_iCrownIndex].Class != 216 )
	{
		LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::NotifyCrownState() - m_iCrownIndex isn't Valid");
		return;
	}

	LPOBJ lpObj = &gObj[m_iCrownIndex];

	if(btState == 0)
	{
		gEffectManager.AddEffect(lpObj,0,EFFECT_CASTLE_CROWN_STATE,0,0,0,0,0);
	}
	else
	{
		gEffectManager.DelEffect(lpObj,EFFECT_CASTLE_CROWN_STATE);
	}

	if( lpObj->VPCount2 < 1 )
	{
		return;
	}

	PMSG_ANS_NOTIFYCROWNSTATE pMsg;

	pMsg.h.set(0xB2, 0x16, sizeof(pMsg));
	pMsg.btCrownState = btState;

	int tObjNum = -1;

	for( int i = 0; i < MAX_VIEWPORT; i++ )
	{
		tObjNum = lpObj->VpPlayer2[i].index;

		if( tObjNum >= 0 )
		{
			if( gObj[tObjNum].Type == OBJECT_USER && gObj[tObjNum].Live )
			{
				DataSend(tObjNum, (LPBYTE)&pMsg, pMsg.h.size);
			}
		}
	}

	LogAdd(LOG_BLACK,"[CastleSiege] Crown State Changed (%d)", btState);
}

void CCastleSiege::NotifyCrownSwitchInfo(int iCrownSwitchIndex)
{
	if( gObjIsConnected(iCrownSwitchIndex) == FALSE )
	{
		LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::NotifyCrownSwitchInfo() - iCrownSwitchIndex is Invalid");
		return;
	}

	if( (gObj[iCrownSwitchIndex].Class != 217) && (gObj[iCrownSwitchIndex].Class != 218) )
	{
		LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::NotifyCrownSwitchInfo() - iCrownSwitchIndex is Invalid");
		return;
	}

	LPOBJ lpObj = &gObj[iCrownSwitchIndex];

	if( lpObj->VPCount2 < 1 )
	{
		return;
	}

	PMSG_ANS_NOTIFY_CROWNSWITCH_INFO pMsg;
	pMsg.h.set(0xB2, 0x20, sizeof(pMsg));

	pMsg.btIndex1 = SET_NUMBERHB(LOWORD(iCrownSwitchIndex));
	pMsg.btIndex2 = SET_NUMBERLB(LOWORD(iCrownSwitchIndex));

	pMsg.btSwitchState = 0;
	pMsg.btJoinSide = 0;

	memset(&pMsg.szGuildName, 0, 8);
	memset(&pMsg.szUserName, 0, 10);

	int iCrownSwitchUserIndex = gCastleSiege.GetCrownSwitchUserIndex(lpObj->Class);

	if( gObjIsConnected(iCrownSwitchUserIndex) )
	{
		LPOBJ lpSwitchObj = &gObj[iCrownSwitchUserIndex];
		char szMasterGuildName[8];

		pMsg.btSwitchState = TRUE;
		pMsg.btJoinSide = lpSwitchObj->CsJoinSide;

		memcpy(&pMsg.szUserName, lpSwitchObj->Name, 10);
		memcpy(&pMsg.szGuildName, lpSwitchObj->GuildName, 8);
		memset(&szMasterGuildName, 0, 8);

		if( lpSwitchObj->Guild )
		{
			if( gUnionManager.GetUnionName(lpSwitchObj->Guild->GuildUnion, szMasterGuildName) != -1 )
			{
				memcpy(&pMsg.szGuildName, szMasterGuildName, 8);
			}
		}
		DataSend(iCrownSwitchUserIndex, (LPBYTE)&pMsg, pMsg.h.size);
	}
	
	int tObjNum = -1;

	for( int i = 0; i < MAX_VIEWPORT; i++ )
	{
		tObjNum = lpObj->VpPlayer2[i].index;

		if( tObjNum >= 0 )
		{
			if( gObj[tObjNum].Type == OBJECT_USER && gObj[tObjNum].Live )
			{
				DataSend(tObjNum, (LPBYTE)&pMsg, pMsg.h.size);
			}
		}
	}
}

void CCastleSiege::SendMapServerGroupMsg(char * lpszMsg)
{
	GS_GDReqMapSvrMsgMultiCast(this->m_iMapSvrGroup, lpszMsg);
}

void CCastleSiege::SendAllUserAnyData(LPBYTE lpMsg, int iSize)
{
	for( int i = OBJECT_START_USER; i < MAX_OBJECT; i++ )
	{
		if( (gObj[i].Connected == OBJECT_ONLINE) && (gObj[i].Type == OBJECT_USER) )
		{
			DataSend(i, lpMsg, iSize);
		}
	}
}

void CCastleSiege::SendAllUserAnyMsg(char * lpszMsg, int iType)
{
	if( lpszMsg == NULL )
	{
		return;
	}

	switch( iType )
	{
		case 1:
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,lpszMsg);
		break;

		case 2:
			GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(),0,0,0,0,0,0,lpszMsg);
		break;

		default:
			break;
	}
}

void CCastleSiege::SendCsUserAnyData(LPBYTE lpMsg, int iSize)
{
	for( int i = OBJECT_START_USER; i < MAX_OBJECT; i++ )
	{
		if( (gObj[i].Connected == OBJECT_ONLINE) && (gObj[i].Type == OBJECT_USER) )
		{
			if( gObj[i].Map == MAP_CASTLE_SIEGE )
			{
				DataSend(i, lpMsg, iSize);
			}
		}
	}
}

void CCastleSiege::SendCsUserAnyMsg(char* lpszMsg)
{
	if( lpszMsg == NULL )
	{
		return;
	}

	for( int i = OBJECT_START_USER; i < MAX_OBJECT; i++ )
	{
		if( (gObj[i].Connected == OBJECT_ONLINE) && (gObj[i].Type == OBJECT_USER) )
		{
			if( gObj[i].Map == MAP_CASTLE_SIEGE )
			{
				gNotice.GCNoticeSend(i,0,0,0,0,0,0,lpszMsg);
			}
		}
	}
}

void CCastleSiege::CreateDbNPC_INS()
{
	this->ClearDbNPC();

	EnterCriticalSection(&this->m_critNpcData);

	std::vector<_CS_NPC_DATA>::iterator it = this->m_vtNpcData.begin();

	while( it != this->m_vtNpcData.end() )
	{
		_CS_NPC_DATA* pNpcData;
		_CS_NPC_DATA NpcData( *it );
		pNpcData = &NpcData;

		if( pNpcData->m_bNPC_DBSAVE == TRUE )
		{
			it->SetBaseValue();
			it->m_iNPC_LIVE = 1;
			it->m_iNPC_SIDE = 1;
			it->m_iNPC_OBJINDEX = -1;
			it->m_iCS_GATE_LEVER_INDEX = -1;
		}
		it++;
	}

	LeaveCriticalSection(&this->m_critNpcData);

	this->m_bDbNpcCreated = FALSE;
}

void CCastleSiege::SavePcRoomUserList()
{

}

void CCastleSiege::StartCS()
{

	CTime CurrentTime = CTime::GetTickCount();

	this->m_tmStartDate.wYear = CurrentTime.GetYear();
	this->m_tmStartDate.wMonth = CurrentTime.GetMonth();
	this->m_tmStartDate.wDay = CurrentTime.GetDay();
	this->m_tmEndDate.wYear = CurrentTime.GetYear();
	this->m_tmEndDate.wMonth = CurrentTime.GetMonth();
	this->m_tmEndDate.wDay = CurrentTime.GetDay()+1;

	//this->m_btIsSiegeGuildList = lpMsg->btIsSiegeGuildList;
	//this->m_btIsSiegeEnded = lpMsg->btIsSiegeEnded;
	//this->m_btIsCastleOccupied = lpMsg->btIsCastleOccupied;

	//memset(&this->m_szCastleOwnerGuild, 0, sizeof(this->m_szCastleOwnerGuild));
	//memcpy(&this->m_szCastleOwnerGuild, lpMsg->szCastleOwnGuild, sizeof(lpMsg->szCastleOwnGuild));
	//
	//this->m_i64CastleMoney = lpMsg->i64CastleMoney;
	//this->m_iTaxRateChaos = lpMsg->iTaxRateChaos;
	//this->m_iTaxRateStore = lpMsg->iTaxRateStore;
	//this->m_iTaxHuntZone = lpMsg->iTaxHuntZone;

				std::vector<_CS_SCHEDULE_DATA> vtScheduleData;

				int iSTATE = -1;
				int iSTART_DAY = -1;
				int iSTART_HOUR = -1;
				int iSTART_MIN = -1;

				EnterCriticalSection(&this->m_critScheduleData);
				this->m_vtScheduleData.clear();
				LeaveCriticalSection(&this->m_critScheduleData);

				int value = 0;
	
				for(int i=1; i<10; i++)
				{
				iSTATE = i;
				iSTART_DAY = 0;

				if (i == 2)
				{
					value+=5;
				}

				if (i == 4)
				{
					value+=5;
				}

				if (i == 8)
				{
					value+=30;
				}

				//if (i == 9)
				//{
				//	iSTART_DAY=1;
				//	iSTART_HOUR=0;
				//	iSTART_MIN=0;
				//}

				iSTART_HOUR = CurrentTime.GetHour();
				iSTART_MIN = CurrentTime.GetMinute()+(i+value);

				if (iSTART_MIN >= 60)
				{
					iSTART_HOUR++;
					iSTART_MIN = iSTART_MIN-60;
				}

				LogAdd(LOG_RED,"[CastleSiege] Stage %d : %d : %d",iSTATE,iSTART_HOUR,iSTART_MIN);

				if(!vtScheduleData.empty())
				{
					for(std::vector<_CS_SCHEDULE_DATA>::iterator it = vtScheduleData.begin(); it != vtScheduleData.end(); it++)
					{
						_CS_SCHEDULE_DATA pScheduleData = _CS_SCHEDULE_DATA(*it);

						if(pScheduleData.m_bIN_USE != 0)
						{
							if(pScheduleData.m_iSTATE == iSTATE)
							{
								LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::LoadData() - Same State Exist : %d",iSTATE);
								return;
							}
							
							if(pScheduleData.m_iSTATE > iSTATE)
							{
								int iSTATE1_TIME = pScheduleData.m_iADD_DAY * 24 * 60 + pScheduleData.m_iADD_HOUR * 60 + pScheduleData.m_iADD_MIN;
								int iSTATE2_TIME = iSTART_DAY * 24 * 60 + iSTART_HOUR * 60 + iSTART_MIN;
	
								if(iSTATE1_TIME <= iSTATE2_TIME)
								{
									LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::LoadData() - Date Order is wrong : %d",iSTATE2_TIME);
									return;
								}
							}
							else
							{
								int iSTATE1_TIME = pScheduleData.m_iADD_DAY * 24 * 60 + pScheduleData.m_iADD_HOUR * 60 + pScheduleData.m_iADD_MIN;
								int iSTATE2_TIME = iSTART_DAY * 24 * 60 + iSTART_HOUR * 60 + iSTART_MIN;
								if(iSTATE1_TIME >= iSTATE2_TIME)
								{
									LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::LoadData() - Date Order is wrong : %d",iSTATE2_TIME);
									return;
								}
							}
						}
					}
				}

				_CS_SCHEDULE_DATA pScheduleDataInsert;

				pScheduleDataInsert.m_bIN_USE = TRUE;
				pScheduleDataInsert.m_iSTATE = iSTATE;
				pScheduleDataInsert.m_iADD_DAY = iSTART_DAY;
				pScheduleDataInsert.m_iADD_HOUR = iSTART_HOUR;
				pScheduleDataInsert.m_iADD_MIN = iSTART_MIN;

				vtScheduleData.push_back(pScheduleDataInsert);
				}

	EnterCriticalSection(&this->m_critScheduleData);

	if(!vtScheduleData.empty())
	{
		for(std::vector<_CS_SCHEDULE_DATA>::iterator it = vtScheduleData.begin(); it != vtScheduleData.end(); it++)
		{
			this->m_vtScheduleData.push_back(*it);
		}
	}

	sort(this->m_vtScheduleData.begin(),this->m_vtScheduleData.end(),this->ScheduleStateCompFunc);

	if(!m_vtScheduleData.empty())
	{
		for(int iIDX = 0; (DWORD)iIDX < this->m_vtScheduleData.size(); iIDX++)
		{
			if((DWORD)(iIDX+1) < this->m_vtScheduleData.size())
			{
				int iIDX_SEC1 = this->m_vtScheduleData[iIDX].m_iADD_DAY * 24 * 60 * 60 + this->m_vtScheduleData[iIDX].m_iADD_HOUR * 60 * 60 + this->m_vtScheduleData[iIDX].m_iADD_MIN * 60;
				int iIDX_SEC2 = this->m_vtScheduleData[iIDX+1].m_iADD_DAY * 24 * 60 * 60 + this->m_vtScheduleData[iIDX+1].m_iADD_HOUR * 60 * 60 + this->m_vtScheduleData[iIDX+1].m_iADD_MIN * 60;

				int iIDX_RESULT = iIDX_SEC2 - iIDX_SEC1;

				if(iIDX_RESULT < 0)
				{
					LogAdd(LOG_RED,"[CastleSiege] CCastleSiege::LoadData() - Date Order is wrong (sort fail) : %d-%d", this->m_vtScheduleData[iIDX].m_iSTATE, this->m_vtScheduleData[iIDX+1].m_iSTATE);
				}
				else
				{
					this->m_vtScheduleData[iIDX].m_iGAP_SEC = iIDX_RESULT;
				}
			}
			else
			{
				this->m_vtScheduleData[iIDX].m_iGAP_SEC = 0;
			}
		}
	}

	LeaveCriticalSection(&this->m_critScheduleData);

	this->m_btIsSiegeGuildList = 0;
	this->m_btIsSiegeEnded = 0;

	int iSTART_DATE_NUM = MACRO2(( MACRO1(this->m_tmStartDate.wDay) | MACRO1(this->m_tmStartDate.wMonth) << 8 ) & 0xFFFF ) | MACRO2(this->m_tmStartDate.wYear) << 16;
	int iEND_DATE_NUM = MACRO2(( MACRO1(this->m_tmEndDate.wDay) | MACRO1(this->m_tmEndDate.wMonth) << 8 ) & 0xFFFF ) | MACRO2(this->m_tmEndDate.wYear) << 16;
	
	this->m_tmSiegeEndSchedule = this->m_tmStartDate;

	SYSTEMTIME EndTime;
	GetStateDate(CASTLESIEGE_STATE_ENDCYCLE, &EndTime);

	LogAdd(LOG_BLACK,"[CastleSiege] CCastleSiege::LoadData() - Siege Start Date (%d-%d-%d)", m_tmStartDate.wYear, m_tmStartDate.wMonth, m_tmStartDate.wDay);
	
	LogAdd(LOG_BLACK,"[CastleSiege] CCastleSiege::LoadData() - Siege Date (%d-%d-%d)", EndTime.wDay, EndTime.wHour, EndTime.wMinute);

	GetNextDay(&m_tmSiegeEndSchedule, EndTime.wDay, EndTime.wHour, EndTime.wMinute, 0);

	LogAdd(LOG_BLACK,"[CastleSiege] CCastleSiege::LoadData() - Siege End Date (%d-%d-%d(%d:%d:%d))", this->m_tmSiegeEndSchedule.wYear, this->m_tmSiegeEndSchedule.wMonth, this->m_tmSiegeEndSchedule.wDay, this->m_tmSiegeEndSchedule.wHour, this->m_tmSiegeEndSchedule.wMinute, this->m_tmSiegeEndSchedule.wSecond);

	if( iSTART_DATE_NUM > iEND_DATE_NUM )
	{
		LogAdd(LOG_RED, "[CastleSiege] CCastleSiege::SetCastleInitData() - iSTART_DATE_NUM > iEND_DATE_NUM");
		return;
	}

	gCastleSiege.SetDbDataLoadOK(TRUE);

	gCastleSiege.SetDataLoadState(4);

	gCastleSiege.Init();

	return;
}
#endif

