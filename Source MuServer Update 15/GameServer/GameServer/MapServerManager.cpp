// MapServerManager.cpp: implementation of the CMapServerManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MapServerManager.h"
#include "Map.h"
#include "MemScript.h"
#include "ServerInfo.h"
#include "Util.h"

CMapServerManager gMapServerManager;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMapServerManager::CMapServerManager() // OK
{
	this->m_MapDataLoadOk=FALSE;
	this->m_MapServerData = NULL;

	InitializeCriticalSection(&this->m_critical);
}

CMapServerManager::~CMapServerManager() // OK
{
	DeleteCriticalSection(&this->m_critical);
}

void CMapServerManager::Clear()
{
	this->m_MapDataLoadOk = FALSE;

	for ( int iGROUP_COUNT=0;iGROUP_COUNT<MAX_MAP_GROUPS;iGROUP_COUNT++)
	{
		this->m_MapServerGroup[iGROUP_COUNT] = 0;

		for ( int iSUB_GROUP_COUNT=0;iSUB_GROUP_COUNT<MAX_MAP_SUBGROUPS;iSUB_GROUP_COUNT++)
		{
			this->m_MapServer[iGROUP_COUNT][iSUB_GROUP_COUNT].Clear(1);
		}
	}

	this->m_MapServerCode.clear();
}

void CMapServerManager::Load(char* path)
{
	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,path);
		return;
	}

	if(lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	EnterCriticalSection(&this->m_critical);

	this->Clear();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}
		
			int section = lpMemScript->GetNumber();

			while(true)
			{
				if(section == 0)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					short sSVR_CODE = -1;
					short sMAPSVR_GROUP = -1;
					int iInitSetVal = 1;
					char szIpAddr[16]={0};
					WORD wPortNum = 0;

					sSVR_CODE = lpMemScript->GetNumber();

					sMAPSVR_GROUP = lpMemScript->GetAsNumber();

					iInitSetVal = lpMemScript->GetAsNumber();

					memcpy(szIpAddr, &lpMemScript->GetAsString()[1], 16);
					szIpAddr[15] =0;

					wPortNum = lpMemScript->GetAsNumber();

					if ( sSVR_CODE < 0 )
					{
						LeaveCriticalSection(&this->m_critical);
						ErrorMessageBox("[MapServerMng] CMapServerManager::LoadData() - file load error : sSVR_CODE < 0 (SVR:%d) - 1",
							sSVR_CODE);
						return;
					}

					if ( iInitSetVal != -1 && iInitSetVal != 0 && iInitSetVal != 1 )
					{
						LeaveCriticalSection(&this->m_critical);
						ErrorMessageBox("[MapServerMng] CMapServerManager::LoadData() - file load error : iInitSetting Value:%d (SVR:%d) - 1",
							iInitSetVal, sSVR_CODE);
						return;
					}

					if ( !strcmp(szIpAddr, ""))
					{
						LeaveCriticalSection(&this->m_critical);
						ErrorMessageBox("[MapServerMng] CMapServerManager::LoadData() - file load error : No IpAddress (SVR:%d)",
							sSVR_CODE);
						return;
					}

					if ( sMAPSVR_GROUP < 0 || sMAPSVR_GROUP >= MAX_MAP_GROUPS)
					{
						LeaveCriticalSection(&this->m_critical);
						ErrorMessageBox("[MapServerMng] CMapServerManager::LoadData() - file load error : Map Server Group Index (IDX:%d)",
							sMAPSVR_GROUP);
						return;
					}

					if ( this->m_MapServerGroup[sMAPSVR_GROUP] >= MAX_MAP_SUBGROUPS )
					{
						LeaveCriticalSection(&this->m_critical);
						ErrorMessageBox("[MapServerMng] CMapServerManager::LoadData() - file load error : No Space to Save SvrInfo (SVR:%d)",
							sSVR_CODE);
						return;
					}

					CMapServerData * lpMapSvrData = NULL;

					lpMapSvrData= &this->m_MapServer[sMAPSVR_GROUP][this->m_MapServerGroup[sMAPSVR_GROUP]];

					lpMapSvrData->Clear(iInitSetVal);
					lpMapSvrData->m_used = TRUE;
					lpMapSvrData->m_ServerCode = sSVR_CODE;
					lpMapSvrData->m_MapServerGroup = (BYTE)sMAPSVR_GROUP;
					lpMapSvrData->m_port = wPortNum;
					memcpy(lpMapSvrData->m_IpAddr, szIpAddr, 16);
					lpMapSvrData->m_IpAddr[15] = 0;
					
					this->m_MapServerCode.insert(std::pair<int,CMapServerData *>(sSVR_CODE, lpMapSvrData));
					this->m_MapServerGroup[sMAPSVR_GROUP]++;
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					short sSVR_CODE = -1;
					BYTE btNotMoveOption = 0;
					WORD wMapNum = 0;
					short sDEST_SVR_CODE = -1;
					CMapServerData * lpMapSvrData = NULL;

					sSVR_CODE = lpMemScript->GetNumber();

					btNotMoveOption = lpMemScript->GetAsNumber();

					wMapNum = lpMemScript->GetAsNumber();

					sDEST_SVR_CODE = lpMemScript->GetAsNumber();

					if ( sSVR_CODE < 0 )
					{
						LeaveCriticalSection(&this->m_critical);
						ErrorMessageBox("[MapServerMng] CMapServerManager::LoadData() - file load error : sSVR_CODE < 0 (SVR:%d) - 2",
							sSVR_CODE);
						return;
					}

					if ( sDEST_SVR_CODE < -2 )
					{
						LeaveCriticalSection(&this->m_critical);
						ErrorMessageBox("[MapServerMng] CMapServerManager::LoadData() - file load error : sDEST_SVR_CODE < -1 (SVR:%d, DEST_SVR:%d) - 2",
							sSVR_CODE, sDEST_SVR_CODE);
						return;
					}

					std::map<int  ,CMapServerData *>::iterator it = this->m_MapServerCode.find(sSVR_CODE);

					if ( it == this->m_MapServerCode.end() )
					{
						LeaveCriticalSection(&this->m_critical);
						ErrorMessageBox("[MapServerMng] CMapServerManager::LoadData() - file load error : sSVR_CODE wasn't registered (SVR:%d)",
							sSVR_CODE);
						return;
					}

					lpMapSvrData = it->second;

					if ( lpMapSvrData == NULL )
					{
						LeaveCriticalSection(&this->m_critical);
						ErrorMessageBox("[MapServerMng] CMapServerManager::LoadData() - file load error : lpMapSvrData == NULL (SVR:%d)",
							sSVR_CODE);
						return;
					}

					if ( lpMapSvrData->m_used == FALSE )
					{
						LeaveCriticalSection(&this->m_critical);
						ErrorMessageBox("[MapServerMng] CMapServerManager::LoadData() - file load error : lpMapSvrData->m_bIN_USE == FALSE (SVR:%d)",
							sSVR_CODE);
						return;
					}

					if ( lpMapSvrData->m_ServerCode != sSVR_CODE )
					{
						LeaveCriticalSection(&this->m_critical);
						ErrorMessageBox("[MapServerMng] CMapServerManager::LoadData() - file load error : lpMapSvrData->m_sSVR_CODE != sSVR_CODE (SVR:%d)",
							sSVR_CODE);
						return;
					}

					if ( btNotMoveOption != 0 && btNotMoveOption != 1 )
					{
						LeaveCriticalSection(&this->m_critical);
						ErrorMessageBox("[MapServerMng] CMapServerManager::LoadData() - file load error : lpMapSvrData->m_sSVR_CODE != sSVR_CODE (SVR:%d, OPT:%d)",
							sSVR_CODE, btNotMoveOption);
						return;
					}

					if ( MAP_RANGE(wMapNum) == FALSE )
					{
						LeaveCriticalSection(&this->m_critical);
						ErrorMessageBox("[MapServerMng] CMapServerManager::LoadData() - file load error : Map Number is out of bound (SVR:%d, MAP:%d)",
							sSVR_CODE, wMapNum);
						return;
					}

					switch ( btNotMoveOption )
					{
						case 0:
							lpMapSvrData->m_MapMove[wMapNum] = sDEST_SVR_CODE;
							break;
						case 1:
							lpMapSvrData->m_MapMove[wMapNum] = -3;
							break;
						default:
							LeaveCriticalSection(&this->m_critical);
							ErrorMessageBox("[MapServerMng] CMapServerManager::LoadData() - file load error : lpMapSvrData->m_sSVR_CODE != sSVR_CODE (SVR:%d, OPT:%d)",
								sSVR_CODE, btNotMoveOption);
							return;
					}
				}
				else
				{
					break;
				}
			}
		}
	}
	catch(...)
	{
		LeaveCriticalSection(&this->m_critical);

		ErrorMessageBox(lpMemScript->GetLastError());
	}

	std::map<int  ,CMapServerData *>::iterator it = this->m_MapServerCode.find(gServerInfo.m_ServerCode);

	if ( it != this->m_MapServerCode.end() )
	{
		this->m_MapServerData = it->second;
	}
	else
	{
		this->m_MapServerData = NULL;
	}

	if ( this->m_MapServerData == NULL )
	{
		LeaveCriticalSection(&this->m_critical);
		ErrorMessageBox("[MapServerMng] CMapServerManager::LoadData() - file load error : This GameServerCode (%d) doesn't Exist at file '%s' != sSVR_CODE",
			gServerInfo.m_ServerCode, path);

		return;
	}

	this->m_MapDataLoadOk = TRUE;

	LeaveCriticalSection(&this->m_critical);

	delete lpMemScript;
}

int CMapServerManager::GetMapServerGroup() // OK
{
	if(this->m_MapServerData == 0)
	{
		return -1;
	}

	return this->m_MapServerData->m_MapServerGroup;
}

bool CMapServerManager::GetMapServerData(WORD ServerCode,char* IpAddress,WORD* ServerPort) // OK
{
	if ( !IpAddress || !ServerPort )
		return FALSE;

	CMapServerData * lpMapSvrData = NULL;

	EnterCriticalSection(&this->m_critical);

	std::map<int, CMapServerData *>::iterator it = this->m_MapServerCode.find(ServerCode);

	if ( it != this->m_MapServerCode.end() )
	{
		lpMapSvrData = it->second;
	}

	LeaveCriticalSection(&this->m_critical);

	if ( lpMapSvrData == NULL )
		return FALSE;

	strcpy_s(IpAddress, sizeof(lpMapSvrData->m_IpAddr), lpMapSvrData->m_IpAddr);
	(*ServerPort) = lpMapSvrData->m_port;

	return TRUE;
}

bool CMapServerManager::CheckMapServer(int map) // OK
{
	if(MAP_RANGE(map) == 0)
	{
		return 0;
	}

	if(this->m_MapServerData == 0)
	{
		return 0;
	}

	if(this->m_MapServerData->m_used == 0)
	{
		return 0;
	}

	if(this->m_MapServerData->m_MapMove[map] != -3)
	{
		return 0;
	}

	return 1;
}

short CMapServerManager::CheckMapServerMove(int aIndex,int map,short ServerCode) // OK
{
	if ( this->m_MapDataLoadOk == FALSE )
		return (short)gServerInfo.m_ServerCode;

	if ( !gObjIsConnected(aIndex))
		return (short)gServerInfo.m_ServerCode;

	if(MAP_RANGE(map) == 0)
	{
		LogAdd(LOG_RED, "[MapServerMng] CheckMoveMapSvr() - Map Index doesn't exist [%s][%s] : %d",
			gObj[aIndex].Account, gObj[aIndex].Name, map);

		return (short)gServerInfo.m_ServerCode;
	}

	CMapServerData * lpMapSvrData = this->m_MapServerData;

	if ( lpMapSvrData == NULL )
	{
		LogAdd(LOG_RED, "[MapServerMng] CheckMoveMapSvr() - m_lpThisMapSvrData == NULL [%s][%s] : %d",
			gObj[aIndex].Account, gObj[aIndex].Name, map);

		return (short)gServerInfo.m_ServerCode;
	}

	if ( lpMapSvrData->m_used == FALSE )
	{
		LogAdd(LOG_RED, "[MapServerMng] CheckMoveMapSvr() - lpMapSvrData->m_bIN_USE == FALSE [%s][%s] : %d",
			gObj[aIndex].Account, gObj[aIndex].Name, map);

		return (short)gServerInfo.m_ServerCode;
	}

	short sMAP_MOVE_INFO = lpMapSvrData->m_MapMove[map];

	switch ( sMAP_MOVE_INFO )
	{
		case -1:
			{
				CMapServerData * lpDestMapSvrData = NULL;

				if ( ServerCode != -1 )
				{
					EnterCriticalSection(&this->m_critical);

					std::map<int, CMapServerData *>::iterator it = this->m_MapServerCode.find(ServerCode);

					if ( it != this->m_MapServerCode.end() )
					{
						lpDestMapSvrData = it->second;
					}

					LeaveCriticalSection(&this->m_critical);

					if ( lpDestMapSvrData != NULL )
					{
						if ( lpDestMapSvrData->m_MapMove[map] == -3 )
						{
							return ServerCode;
						}
					}
				}

				std::vector<CMapServerData *> vtMapSvrData;

				EnterCriticalSection(&this->m_critical);

				for ( std::map<int ,CMapServerData *>::iterator it = this->m_MapServerCode.begin() ; it != this->m_MapServerCode.end() ;it++)
				{
					CMapServerData * lpTempMapSvrData = it->second;

					if ( lpTempMapSvrData != NULL &&
						lpTempMapSvrData->m_used == TRUE &&
						 lpTempMapSvrData->m_MapMove[map] == -3)
					{
						vtMapSvrData.push_back(it->second);
					}
				}

				LeaveCriticalSection(&this->m_critical);

				short sDestServerCode = -1;

				if ( vtMapSvrData.empty() == 0 )
				{
					sDestServerCode = vtMapSvrData[GetLargeRand()%vtMapSvrData.size()]->m_ServerCode;
				}

				if ( sDestServerCode != -1 )
				{
					//LogAdd(LOG_BLACK,"[MapServerMng] CheckMoveMapSvr() - MapServer Check OK [%s][%s] : MAP-%d / SVR-%d(State Map:%d X:%d Y:%d)",gObj[aIndex].Account, gObj[aIndex].Name, map, sDestServerCode,gObj[aIndex].Map, gObj[aIndex].X, gObj[aIndex].Y);

					return sDestServerCode;
				}
			}
			break;

		case -2:
			{
				CMapServerData * lpDestMapSvrData = NULL;

				if ( ServerCode != -1 )
				{
					EnterCriticalSection(&this->m_critical);

					std::map<int, CMapServerData *>::iterator it = this->m_MapServerCode.find(ServerCode);

					if ( it != this->m_MapServerCode.end() )
					{
						lpDestMapSvrData = it->second;
					}

					LeaveCriticalSection(&this->m_critical);

					if ( lpDestMapSvrData != NULL )
					{
						if ( lpDestMapSvrData->m_MapMove[map] == -3 )
						{
							return ServerCode;
						}
					}
				}
				
				short sDestServerCode = -1;

				EnterCriticalSection(&this->m_critical);

				for ( std::map<int ,CMapServerData *>::iterator it = this->m_MapServerCode.begin() ; it != this->m_MapServerCode.end() ;it++)
				{
					CMapServerData * lpTempMapSvrData = it->second;

					if ( lpTempMapSvrData != NULL &&
						lpTempMapSvrData->m_used == TRUE &&
						 lpTempMapSvrData->m_MapMove[map] == -3)
					{
						sDestServerCode = lpTempMapSvrData->m_ServerCode;
					}
				}

				LeaveCriticalSection(&this->m_critical);

				if ( sDestServerCode != -1 )
				{
					//LogAdd(LOG_BLACK,"[MapServerMng] CheckMoveMapSvr() - MapServer Check OK [%s][%s] : MAP-%d / SVR-%d(State Map:%d X:%d Y:%d)",gObj[aIndex].Account, gObj[aIndex].Name, map, sDestServerCode,gObj[aIndex].Map, gObj[aIndex].X, gObj[aIndex].Y);

					return sDestServerCode;
				}
			}
			break;

		case -3:
			//LogAdd(LOG_BLACK,"[MapServerMng] CheckMoveMapSvr() - MapServer Check OK [%s][%s] : MAP-%d / SVR-%d (State Map:%d X:%d Y:%d)",gObj[aIndex].Account, gObj[aIndex].Name, map, gServerInfo.m_ServerCode,gObj[aIndex].Map, gObj[aIndex].X, gObj[aIndex].Y);

			return (short)gServerInfo.m_ServerCode;
			break;

		default:
			if ( sMAP_MOVE_INFO > -1 )
			{
				CMapServerData * lpDestMapSvrData = NULL;

				EnterCriticalSection(&this->m_critical);

				std::map<int, CMapServerData *>::iterator it = this->m_MapServerCode.find(sMAP_MOVE_INFO);

				if ( it != this->m_MapServerCode.end() )
				{
					lpDestMapSvrData = it->second;
				}

				LeaveCriticalSection(&this->m_critical);

				if ( lpDestMapSvrData != NULL &&
					 lpDestMapSvrData->m_used == TRUE &&
					 lpDestMapSvrData->m_MapMove[map] == -3)
				{
					return sMAP_MOVE_INFO;
				}

				short sDestServerCode = -1;

				EnterCriticalSection(&this->m_critical);

				for ( it = this->m_MapServerCode.begin() ; it != this->m_MapServerCode.end() ;it++)
				{
					CMapServerData * lpTempMapSvrData = it->second;

					if ( lpTempMapSvrData != NULL &&
						 lpTempMapSvrData->m_used == TRUE &&
						 lpTempMapSvrData->m_MapMove[map] == -3)
					{
						sDestServerCode = lpTempMapSvrData->m_ServerCode;
					}
				}
				
				LeaveCriticalSection(&this->m_critical);

				if ( sDestServerCode != -1 )
				{
					//LogAdd(LOG_BLACK,"[MapServerMng] CheckMoveMapSvr() - MapServer Check OK [%s][%s] : MAP-%d / SVR-%d(State Map:%d X:%d Y:%d)",gObj[aIndex].Account, gObj[aIndex].Name, map, sDestServerCode,gObj[aIndex].Map, gObj[aIndex].X, gObj[aIndex].Y);

					return sDestServerCode;
				}
			}
			else
			{
				LogAdd(LOG_RED, "[MapServerMng] CheckMoveMapSvr() - Unknown MapMove Info [%s][%s] : MAP-%d / INFO-%d",gObj[aIndex].Account, gObj[aIndex].Name, map, sMAP_MOVE_INFO);

				return (short)gServerInfo.m_ServerCode;
			}
	}

	return (short)gServerInfo.m_ServerCode;
}
