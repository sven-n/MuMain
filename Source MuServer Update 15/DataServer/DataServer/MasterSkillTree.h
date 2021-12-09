// MasterSkillTree.h: interface for the CMasterSkillTree class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DataServerProtocol.h"

#if(DATASERVER_UPDATE>=701)
#define MAX_MASTER_SKILL_LIST 120
#else
#define MAX_MASTER_SKILL_LIST 60
#endif

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_MASTER_SKILL_TREE_RECV
{
	PSBMSG_HEAD header; // C1:0D:00
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_MASTER_SKILL_TREE_SAVE_RECV
{
	PSWMSG_HEAD header; // C2:0D:30
	WORD index;
	char account[11];
	char name[11];
	DWORD MasterLevel;
	DWORD MasterPoint;
	QWORD MasterExperience;
	#if(DATASERVER_UPDATE>=602)
	BYTE MasterSkill[MAX_MASTER_SKILL_LIST][3];
	#endif
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_MASTER_SKILL_TREE_SEND
{
	PSWMSG_HEAD header; // C2:0D:00
	WORD index;
	char account[11];
	char name[11];
	DWORD MasterLevel;
	DWORD MasterPoint;
	QWORD MasterExperience;
	#if(DATASERVER_UPDATE>=602)
	BYTE MasterSkill[MAX_MASTER_SKILL_LIST][3];
	#endif
};

//**********************************************//
//**********************************************//
//**********************************************//

class CMasterSkillTree
{
public:
	CMasterSkillTree();
	virtual ~CMasterSkillTree();
	void GDMasterSkillTreeRecv(SDHP_MASTER_SKILL_TREE_RECV* lpMsg,int index);
	void GDMasterSkillTreeSaveRecv(SDHP_MASTER_SKILL_TREE_SAVE_RECV* lpMsg);
};

extern CMasterSkillTree gMasterSkillTree;
