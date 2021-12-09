// HackSkillCheck.h: implementation of the CHackSkillCheck class.
//
//////////////////////////////////////////////////////////////////////

struct HACKSKILL_INFO
{
	int Index;
	int Skill;
	int MinSpeed;
	int MaxSpeed;
	int Delay;
	int MaxCount;
};

class CHackSkillCheck
{
public:
	CHackSkillCheck();
	virtual ~CHackSkillCheck();
	void Load(char* path);
    bool GetInfo(int index,HACKSKILL_INFO* lpInfo);
	bool GetInfoBySpeed(int Skill,int Speed, HACKSKILL_INFO* lpInfo);
	bool CheckSpeedHack(LPOBJ lpObj,int skill);
private:
	std::map<int,HACKSKILL_INFO> m_HackSkillInfo;
	int ShowMsg;
};

extern CHackSkillCheck gHackSkillCheck;