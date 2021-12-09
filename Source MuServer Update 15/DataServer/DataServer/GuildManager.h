// GuildManager.h: interface for the CGuildManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_GUILD_UNION 5
#define MAX_GUILD_RIVAL 2
#define MAX_GUILD_MEMBER 80

struct GUILD_MEMBER_INFO
{
	void Clear()
	{
		memset(this->szGuildMember,0,sizeof(this->szGuildMember));

		this->btServer = 0xFFFF;

		this->btStatus = 0;
	}

	bool IsEmpty()
	{
		return ((this->szGuildMember[0]==0)?1:0);
	}

	char szGuildMember[11];
	BYTE btStatus;
	WORD btServer;
};

struct GUILD_INFO
{
	void Clear()
	{
		this->dwNumber = 0;

		this->dwUnionNumber = 0;

		this->dwRivalNumber = 0;

		this->dwScore = 0;

		this->btType = 0;

		memset(this->szName,0,sizeof(this->szName));

		memset(this->szMaster,0,sizeof(this->szMaster));

		memset(this->szNotice,0,sizeof(this->szNotice));

		memset(this->arMark,0,sizeof(this->arMark));

		for(int n=0;n < MAX_GUILD_MEMBER;n++){this->arGuildMember[n].Clear();}
	}

	BYTE GetMemberCount()
	{
		BYTE count = 0;

		for(int n=0;n < MAX_GUILD_MEMBER;n++)
		{
			if(this->arGuildMember[n].IsEmpty() == 0)
			{
				count++;
			}
		}

		return count;
	}

	DWORD dwNumber;
	DWORD dwUnionNumber;
	DWORD dwRivalNumber;
	DWORD dwScore;
	BYTE btType;
	char szName[9];
	char szMaster[11];
	char szNotice[60];
	BYTE arMark[32];
	GUILD_MEMBER_INFO arGuildMember[MAX_GUILD_MEMBER];
};

class CGuildManager
{
public:
	CGuildManager();
	virtual ~CGuildManager();
	void Init();
	GUILD_INFO* GetGuildInfo(char* szName);
	GUILD_INFO* GetGuildInfo(DWORD dwNumber);
	GUILD_INFO* GetMemberGuildInfo(char* szGuildMember);
	GUILD_MEMBER_INFO* GetGuildMemberInfo(char* szGuildMember);
	BOOL CheckGuildOnCS(char* szGuildName);
	void ConnectMember(char* szGuildMember,WORD btServer);
	void DisconnectMember(char* szGuildMember);
	BYTE AddGuild(int index,char* szGuildName,char* szMasterName,BYTE* lpMark,BYTE btType);
	BYTE DelGuild(int index,char* szGuildName);
	BYTE AddGuildMember(int index,char* szGuildName,char* szGuildMember,BYTE btStatus,WORD btServer);
	BYTE DelGuildMember(int index,char* szGuildMember);
	BYTE AddGuildRelationship(int index,DWORD dwSourceGuild,DWORD dwTargetGuild,BYTE btRelationType);
	BYTE DelGuildRelationship(int index,DWORD dwSourceGuild,BYTE btRelationType);
	BYTE SetGuildRelationship(int index,char* szGuildMember,char* szGuildMaster);
	BYTE SetGuildScore(char* szGuildName,DWORD dwScore);
	BYTE SetGuildNotice(char* szGuildName,char* szNotice);
	BYTE SetGuildType(char* szGuildName,BYTE btType);
	BYTE SetGuildMemberStatus(char* szGuildMember,BYTE btStatus);
	long GetUnionList(DWORD dwUnionNumber,DWORD* lpUnionList);
	long GetRivalList(DWORD dwRivalNumber,DWORD* lpRivalList);
private:
	std::vector<GUILD_INFO> vGuildList;
};

extern CGuildManager gGuildManager;
