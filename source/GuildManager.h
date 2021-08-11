// GuildManager.h: interface for the CGuildManager class.
//////////////////////////////////////////////////////////////////////
#pragma once

class CGuildManager
{
public:
	CGuildManager();
	virtual ~CGuildManager();
	bool IsGuildMaster();
	bool IsSubGuildMaster();
	bool IsBattleMaster();
public:

};

extern CGuildManager gGuildManager;