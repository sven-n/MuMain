// GuildManager.cpp: implementation of the CGuildManager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GuildManager.h"
#include "UIGuildInfo.h"
#include "./Utilities/Log/ErrorReport.h"

CGuildManager gGuildManager;

CGuildManager::CGuildManager() // OK
{
}

CGuildManager::~CGuildManager() // OK
{
}

bool CGuildManager::IsGuildMaster()
{
	return ( Hero->GuildStatus == G_MASTER );
}

bool CGuildManager::IsSubGuildMaster()
{
	return ( Hero->GuildStatus == G_SUB_MASTER );
}		

bool CGuildManager::IsBattleMaster()
{
	return ( Hero->GuildStatus == G_BATTLE_MASTER );
}		
