// DefaultClassInfo.h: interface for the CDefaultClassInfo class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#if(GAMESERVER_UPDATE>=601)
#define MAX_CLASS 7
#else
#define MAX_CLASS 6
#endif

enum eClassNumber
{
	CLASS_DW = 0,
	CLASS_DK = 1,
	CLASS_FE = 2,
	CLASS_MG = 3,
	CLASS_DL = 4,
	CLASS_SU = 5,
	CLASS_RF = 6,
};

enum eDBClassNumber
{
	DB_CLASS_DW = 0,
	DB_CLASS_SM = 1,
	DB_CLASS_GM = 2,
	DB_CLASS_DK = 16,
	DB_CLASS_BK = 17,
	DB_CLASS_BM = 18,
	DB_CLASS_FE = 32,
	DB_CLASS_ME = 33,
	DB_CLASS_HE = 34,
	DB_CLASS_MG = 48,
	DB_CLASS_DM = 50,
	DB_CLASS_DL = 64,
	DB_CLASS_LE = 66,
	DB_CLASS_SU = 80,
	DB_CLASS_BS = 81,
	DB_CLASS_DS = 82,
	DB_CLASS_RF = 96,
	DB_CLASS_FM = 98,
};

struct DEFAULT_CLASS_INFO
{
	int Class;
	int Strength;
	int Dexterity;
	int Vitality;
	int Energy;
	int Leadership;
	float MaxLife;
	float MaxMana;
	float LevelLife;
	float LevelMana;
	float VitalityToLife;
	float EnergyToMana;
};

class CDefaultClassInfo
{
public:
	CDefaultClassInfo();
	virtual ~CDefaultClassInfo();
	void Init();
	void Load(char* path);
	void SetInfo(DEFAULT_CLASS_INFO info);
	int GetCharacterDefaultStat(int Class,int stat);
public:
	DEFAULT_CLASS_INFO m_DefaultClassInfo[MAX_CLASS];
};

extern CDefaultClassInfo gDefaultClassInfo;
