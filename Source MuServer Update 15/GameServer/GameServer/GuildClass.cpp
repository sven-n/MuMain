// GuildClass.cpp: implementation of the CGuildClass class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GuildClass.h"
#include "Guild.h"
#include "Viewport.h"
#include "Util.h"

CGuildClass gGuildClass;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGuildClass::CGuildClass()
{
	this->head = NULL;
	this->tail = NULL;
}

CGuildClass::~CGuildClass()
{
	this->AllDelete();
}

void CGuildClass::Init()
{
	this->m_GuildMap.clear();
	this->m_GuildNumberMap.clear();
}

GUILD_INFO_STRUCT * CGuildClass::AddGuild(int number, char* guildname, GUILDMARK mark, char * mastername, int score)
{
	GUILD_INFO_STRUCT * pNewNode;
	GUILD_INFO_STRUCT * pSearchGuild = this->SearchGuild(guildname);

	if ( pSearchGuild != NULL )
	{
		return pSearchGuild;
	}

	pNewNode = (GUILD_INFO_STRUCT*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(GUILD_INFO_STRUCT));

	if ( pNewNode != NULL )
	{
		memset(pNewNode->Name, 0, sizeof(pNewNode->Name));
		strcpy_s(pNewNode->Name, guildname);
		strcpy_s(pNewNode->Names[0], mastername);

		if ( mark != NULL )
		{
			memcpy(pNewNode->Mark, mark, sizeof(GUILDMARK));
		}

		pNewNode->Number = number;
		pNewNode->Count = 0;
		pNewNode->TotalCount = 0;
		pNewNode->PlayScore = 0;
		pNewNode->TotalScore = score;
		pNewNode->next = NULL;
		pNewNode->WarDeclareState = 0;
		pNewNode->WarState = 0;
		memset(pNewNode->Notice, 0, sizeof(pNewNode->Notice));
		pNewNode->GuildUnion = 0;
		pNewNode->GuildRival = 0;

		for ( int i=0;i<MAX_GUILD_USER;i++)
		{
			pNewNode->Use[i] = FALSE;
			pNewNode->Index[i] = -1;
			pNewNode->Server[i] = -1;
			pNewNode->GuildStatus[i] = -1;
		}

#if(GAMESERVER_TYPE == 1) 
		pNewNode->LifeStone = NULL; //@24/04/2010
#endif

		this->BuildMemberTotal(pNewNode);
		this->AddTail(pNewNode);
		return pNewNode;
	}
	return NULL;
}

void CGuildClass::AddTail(GUILD_INFO_STRUCT * pNewNode)
{
	pNewNode->back = this->tail;
	pNewNode->next = NULL;

	if ( this->head == NULL )
	{
		this->head = pNewNode;
	}
	else
	{
		this->tail->next = pNewNode;
	}

	this->tail = pNewNode;
	this->m_GuildMap[pNewNode->Name] = pNewNode;
	this->m_GuildNumberMap[pNewNode->Number] = pNewNode;
}

void CGuildClass::Print()
{
	GUILD_INFO_STRUCT * pNode = this->head;

	while ( pNode != NULL )
	{
		pNode = pNode->next;
	}
}

void CGuildClass::PrintGuild(char* guildname)
{
	GUILD_INFO_STRUCT * pNode = this->SearchGuild(guildname);
}

void CGuildClass::AllDelete()
{
	GUILD_INFO_STRUCT * pNode = this->head;
	GUILD_INFO_STRUCT * pTemp;

	while ( pNode != NULL )
	{
		pTemp = pNode;
		pNode = pNode->next;
		HeapFree(GetProcessHeap(), 0, pTemp);
	}

	this->head = NULL;
	this->tail = NULL;
	this->m_GuildMap.clear();
	this->m_GuildNumberMap.clear();
}

BOOL CGuildClass::DeleteGuild(char* guildname, char* commander)
{
	GUILD_INFO_STRUCT * pNode = this->SearchGuild(guildname);
	GUILD_INFO_STRUCT * pprev;
	GUILD_INFO_STRUCT * pnext;

	if ( pNode == NULL )
	{
		return false;
	}

	std::map<std::string, GUILD_INFO_STRUCT *>::iterator Itor = this->m_GuildMap.find(guildname);

	if( Itor != this->m_GuildMap.end() )
	{
		this->m_GuildMap.erase(Itor);
	}

	std::map<int , GUILD_INFO_STRUCT *>::iterator _Itor = this->m_GuildNumberMap.find(pNode->Number);

	if ( _Itor != this->m_GuildNumberMap.end() )
	{
		this->m_GuildNumberMap.erase(_Itor);
	}

	pprev = pNode->back;
	pnext = pNode->next;

	if ( pprev == NULL && pnext == NULL )
	{

	}
	else if ( pprev == NULL && pnext != NULL )
	{
		pnext->back = NULL;
		this->head = pnext;
	}
	else if ( pprev != NULL && pnext == NULL )
	{
		pprev->next = NULL;
		this->tail = pprev;
	}
	else if ( pprev != NULL && pnext != NULL )
	{
		pprev->next = pnext;
		pnext->back = pprev;
	}

	HeapFree(GetProcessHeap(), 0, pNode);

	if ( pprev == NULL && pnext == NULL )
	{
		this->head=NULL;
		this->tail = NULL;
	}

	return true;
}

GUILD_INFO_STRUCT * CGuildClass::SearchGuild(char* guildname)
{
	std::map<std::string, GUILD_INFO_STRUCT *>::iterator Itor = this->m_GuildMap.find(guildname);

	if( Itor == this->m_GuildMap.end() )
	{
		return NULL;
	}

	return (*(Itor)).second;
}

GUILD_INFO_STRUCT * CGuildClass::SearchGuild_Number(int number)
{
	std::map<int , GUILD_INFO_STRUCT *>::iterator Itor = this->m_GuildNumberMap.find(number);

	if ( Itor == this->m_GuildNumberMap.end() )
	{
		return NULL;
	}

	return (*(Itor)).second;
}

GUILD_INFO_STRUCT * CGuildClass::SearchGuild_NumberAndId(int number, char* name)
{
	GUILD_INFO_STRUCT * pNode = this->SearchGuild_Number(number);

	if ( pNode != NULL )
	{
		for ( int i=0;i<MAX_GUILD_USER;i++)
		{
			if ( strcmp(pNode->Names[i], name) == 0 )
			{
				return pNode;
			}
		}
	}

	return NULL;
}

BOOL CGuildClass::ConnectUser(GUILD_INFO_STRUCT * lpNode, char* guild_name, char* player_name, int aIndex, int pServer)
{
	if ( lpNode == NULL )
	{
		return false;
	}

	if ( strcmp(guild_name, lpNode->Name) != 0 )
	{
		return false;
	}

	for ( int n = 0; n<MAX_GUILD_USER;n++)
	{
		if ( lpNode->Use[n] == TRUE )
		{
			if ( strcmp(lpNode->Names[n], player_name) == 0 )
			{
				lpNode->Index[n] = aIndex;
				lpNode->Server[n] = pServer;

				if ( n == 0 )	// Case if is GuildMaster
				{
					lpNode->PlayScore = 0;
					lpNode->WarDeclareState = 0;
					lpNode->WarState = 0;
				}
				
				return TRUE;
			}
		}
	}
	
	this->AddMember(lpNode, player_name, aIndex, -1, pServer);
	return false;
}

GUILD_INFO_STRUCT * CGuildClass::AddMember(char* guild_name, char* player_name, int aIndex, int totalcount, int iGuildStatus, int pServer)
{
	GUILD_INFO_STRUCT * pNode = this->SearchGuild(guild_name);
	int blank = -1;

	if ( pNode == NULL )
	{
		return NULL;
	}

	this->BuildMemberTotal(pNode);

	if ( strcmp(player_name, pNode->Names[0] ) == 0 )
	{
		blank = 0;
	}
	else
	{
		for ( int i = 1;i<MAX_GUILD_USER ; i++)
		{
			if ( pNode->Use[i] == TRUE )
			{
				if ( strcmp(pNode->Names[i], player_name) == 0 )
				{
					if ( aIndex != -1 )
					{
						pNode->Use[i] = TRUE;
						pNode->Index[i] = aIndex;
						pNode->Server[i] = pServer;
						pNode->GuildStatus[i] = 0x80; //G_MASTER;
					}

					this->BuildMemberTotal(pNode);
					return FALSE;
				}
			}
			else if ( blank < 0 )
			{
				blank = i;
			}
		}
	}

	if (blank < 0 )
	{
		return NULL;
	}

	strcpy_s(pNode->Names[blank], player_name);
	pNode->Use[blank] = TRUE;
	pNode->Server[blank] = pServer;
	pNode->GuildStatus[blank] = iGuildStatus;

	if ( aIndex != -1 )
	{
		pNode->Index[blank] = aIndex;
		pNode->Count++;
	}

	if ( totalcount > 0 )
	{
		pNode->TotalCount = totalcount;
	}

	this->BuildMemberTotal(pNode);

	return pNode;
}

GUILD_INFO_STRUCT * CGuildClass::AddMember(GUILD_INFO_STRUCT * pNode, char* player_name, int aIndex, int totalcount, int pServer)
{
	int blank = -1;

	if ( pNode == NULL )
	{
		return NULL;
	}

	if ( strcmp(player_name, pNode->Names[0] ) == 0 )
	{
		blank = 0;
	}
	else
	{
		for ( int i = 1;i<MAX_GUILD_USER ; i++)
		{
			if ( pNode->Use[i] == TRUE )
			{
				if ( strcmp(pNode->Names[i], player_name) == 0 )
				{
					if ( aIndex != -1 )
					{
						pNode->Use[i] = TRUE;
						pNode->Index[i] = aIndex;
						pNode->Server[i] = pServer;
					}

					this->BuildMemberTotal(pNode);
					return FALSE;
				}
			}
			else if ( blank < 0 )
			{
				blank = i;
			}
		}
	}

	if (blank < 0 )
	{
		return NULL;
	}

	strcpy_s(pNode->Names[blank], player_name);
	pNode->Use[blank] = TRUE;
	pNode->Server[blank] = pServer;

	if ( aIndex != -1 )
	{
		pNode->Index[blank] = aIndex;
		pNode->Count++;
	}

	if ( totalcount > 0 )
	{
		pNode->TotalCount = totalcount;
	}

	this->BuildMemberTotal(pNode);

	return pNode;
}

int CGuildClass::GetGuildMemberStatus(char* szGuildName, char* szMemberName)
{
	int iGuildStatus = 0;
	GUILD_INFO_STRUCT * pNode = this->SearchGuild(szGuildName);
	int iKey = -1;

	if ( pNode == NULL )
	{
		return -1;
	}

	for ( int n=0;n<MAX_GUILD_USER;n++)
	{
		if ( pNode->Use[n] == TRUE )
		{
			if ( strcmp(pNode->Names[n], szMemberName) == 0 )
			{
				iKey = n;
			}
		}
	}

	if ( iKey < 0 )
	{
		return -1;
	}

	iGuildStatus = pNode->GuildStatus[iKey];
	return iGuildStatus;
}

BOOL CGuildClass::SetGuildMemberStatus(char* szGuildName, char* szMemberName, int iGuildStatus)
{
	GUILD_INFO_STRUCT * pNode = this->SearchGuild(szGuildName);
	int iKey = -1;

	if ( pNode == NULL )
	{
		return -1;
	}

	for ( int n=0;n<MAX_GUILD_USER;n++)
	{
		if ( pNode->Use[n] == TRUE )
		{
			if ( strcmp(pNode->Names[n], szMemberName) == 0 )
			{
				iKey = n;
			}
		}
	}

	if ( iKey < 0 )
	{
		return 0;
	}
	
	pNode->GuildStatus[iKey] = iGuildStatus;

	if ( pNode->Index[iKey] > 0 )
	{
		if ( gObjIsConnectedGP(pNode->Index[iKey]) == FALSE )
		{
			LogAdd(LOG_BLACK,"@SetGuildMemberStatus() error-L2 : Index %d", pNode->Index[iKey]);
			return false;
		}

		if ( !OBJECT_RANGE(pNode->Index[iKey]) )
		{
			return 0;
		}
		
		LPOBJ lpObj = &gObj[pNode->Index[iKey]];

		if ( strcmp(lpObj->Name, szMemberName) == 0 )
		{
			lpObj->GuildStatus = iGuildStatus;
			gViewport.GCViewportSimpleGuildSend(lpObj);
		}

	}

	return true;
}

int CGuildClass::GetGuildType(char* szGuildName)
{
	int iGuildType = 0;
	GUILD_INFO_STRUCT * pNode = this->SearchGuild(szGuildName);

	if (pNode == NULL )
	{
		return -1;
	}

	iGuildType = pNode->GuildType;
	return iGuildType;
}

BOOL CGuildClass::SetGuildType(char* szGuildName, int iGuildType)
{
	GUILD_INFO_STRUCT * pNode = this->SearchGuild(szGuildName);

	if (pNode == NULL )
	{
		return FALSE;
	}

	pNode->GuildType = iGuildType;

	for ( int n=0;n<MAX_GUILD_USER;n++)
	{
		if ( pNode->Use[n] == TRUE )
		{
			if ( pNode->Index[n] > 0 )
			{
				if ( gObjIsConnectedGP(pNode->Index[n]) == FALSE )
				{
					LogAdd(LOG_BLACK,"SetGuildType() error-L2 : Index %d", pNode->Index[n]);
					continue;
				}

				if ( !OBJECT_RANGE(pNode->Index[n]) )
				{
					continue;
				}

				LPOBJ lpObj = &gObj[pNode->Index[n]];
				gViewport.GCViewportSimpleGuildSend(lpObj);
			}
		}
	}

	return TRUE;
}

BOOL CGuildClass::BuildMemberTotal(GUILD_INFO_STRUCT * lpNode)
{
	if (lpNode == NULL )
	{
		return FALSE;
	}

	lpNode->TotalCount = 0;
	lpNode->Count = 0;

	for ( int n=0;n<MAX_GUILD_USER;n++)
	{
		if ( lpNode->Use[n] == TRUE )
		{
			lpNode->TotalCount++;

			if ( lpNode->Index[n] != 0 )
			{
				lpNode->Count++;
			}
		}
	}

	return TRUE;
}

BOOL CGuildClass::DelMember(char* guild_name, char* player_name)
{
	GUILD_INFO_STRUCT * pNode = this->SearchGuild(guild_name);

	if (pNode == NULL )
	{
		return FALSE;
	}

	for ( int n=0;n<MAX_GUILD_USER;n++)
	{
		if ( pNode->Use[n] == TRUE )
		{
			if ( strcmp(pNode->Names[n], player_name ) == 0 )
			{
				pNode->Use[n] = FALSE;
				pNode->Count--;
				this->BuildMemberTotal(pNode);
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CGuildClass::CloseMember(char* guild_name, char* player_name)
{
	GUILD_INFO_STRUCT * pNode = this->SearchGuild(guild_name);

	if (pNode == NULL )
	{
		return FALSE;
	}

	for ( int n=0;n<MAX_GUILD_USER;n++)
	{
		if ( pNode->Use[n] == TRUE )
		{
			if ( strcmp(pNode->Names[n], player_name ) == 0 )
			{
				pNode->Index[n] = -1;
				pNode->Count--;
				pNode->Server[n] = -1;
				this->BuildMemberTotal(pNode);
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CGuildClass::SetServer(char* guild_name, char* player_name, int pServer)
{
	GUILD_INFO_STRUCT * pNode = this->SearchGuild(guild_name);

	if (pNode == NULL )
	{
		return FALSE;
	}

	for ( int n=0;n<MAX_GUILD_USER;n++)
	{
		if ( pNode->Use[n] == TRUE )
		{
			if ( strcmp(pNode->Names[n], player_name ) == 0 )
			{
				pNode->Server[n] = pServer;
				return TRUE;
			}
		}
	}

	return FALSE;
}
