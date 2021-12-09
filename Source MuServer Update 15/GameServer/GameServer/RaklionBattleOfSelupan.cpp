// RaklionBattleOfSelupan.cpp: implementation of the CRaklionBattleOfSelupan class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RaklionBattleOfSelupan.h"
#include "Message.h"
#include "Notice.h"
#include "RaklionBattleUserMng.h"
#include "RaklionUtil.h"
#include "User.h"
#include "Util.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRaklionBattleOfSelupan::CRaklionBattleOfSelupan() // OK
{
	this->ResetAllData();
}

CRaklionBattleOfSelupan::~CRaklionBattleOfSelupan() // OK
{

}

void CRaklionBattleOfSelupan::ResetAllData() // OK
{
	this->m_IsUsed = 0;
	this->m_SkillDelay = 0;
	this->m_SkillDelayTick = 0;
	this->m_IsUseFirstSkill = 0;

	memset(this->m_PatternSkill,0,sizeof(this->m_PatternSkill));

	this->m_PatternSkill[STATE_PATTERN1].IsSkillEnabled[SELUPAN_SKILL_POISON_ATTACK] = 1;
	this->m_PatternSkill[STATE_PATTERN1].IsSkillEnabled[SELUPAN_SKILL_ICESTORM] = 1;
	this->m_PatternSkill[STATE_PATTERN1].IsSkillEnabled[SELUPAN_SKILL_ICESTRIKE] = 1;
	this->m_PatternSkill[STATE_PATTERN1].IsSkillEnabled[SELUPAN_SKILL_TELEPORT] = 1;

	this->m_PatternSkill[STATE_PATTERN2].IsSkillEnabled[SELUPAN_SKILL_POISON_ATTACK] = 1;
	this->m_PatternSkill[STATE_PATTERN2].IsSkillEnabled[SELUPAN_SKILL_ICESTORM] = 1;
	this->m_PatternSkill[STATE_PATTERN2].IsSkillEnabled[SELUPAN_SKILL_ICESTRIKE] = 1;
	this->m_PatternSkill[STATE_PATTERN2].IsSkillEnabled[SELUPAN_SKILL_FREEZE] = 1;
	this->m_PatternSkill[STATE_PATTERN2].IsSkillEnabled[SELUPAN_SKILL_TELEPORT] = 1;

	this->m_PatternSkill[STATE_PATTERN3].IsSkillEnabled[SELUPAN_SKILL_POISON_ATTACK] = 1;
	this->m_PatternSkill[STATE_PATTERN3].IsSkillEnabled[SELUPAN_SKILL_ICESTORM] = 1;
	this->m_PatternSkill[STATE_PATTERN3].IsSkillEnabled[SELUPAN_SKILL_ICESTRIKE] = 1;
	this->m_PatternSkill[STATE_PATTERN3].IsSkillEnabled[SELUPAN_SKILL_FREEZE] = 1;
	this->m_PatternSkill[STATE_PATTERN3].IsSkillEnabled[SELUPAN_SKILL_TELEPORT] = 1;

	this->m_PatternSkill[STATE_PATTERN4].IsSkillEnabled[SELUPAN_SKILL_POISON_ATTACK] = 1;
	this->m_PatternSkill[STATE_PATTERN4].IsSkillEnabled[SELUPAN_SKILL_ICESTORM] = 1;
	this->m_PatternSkill[STATE_PATTERN4].IsSkillEnabled[SELUPAN_SKILL_ICESTRIKE] = 1;
	this->m_PatternSkill[STATE_PATTERN4].IsSkillEnabled[SELUPAN_SKILL_HEAL] = 1;
	this->m_PatternSkill[STATE_PATTERN4].IsSkillEnabled[SELUPAN_SKILL_FREEZE] = 1;
	this->m_PatternSkill[STATE_PATTERN4].IsSkillEnabled[SELUPAN_SKILL_TELEPORT] = 1;

	this->m_PatternSkill[STATE_PATTERN5].IsSkillEnabled[SELUPAN_SKILL_POISON_ATTACK] = 1;
	this->m_PatternSkill[STATE_PATTERN5].IsSkillEnabled[SELUPAN_SKILL_ICESTORM] = 1;
	this->m_PatternSkill[STATE_PATTERN5].IsSkillEnabled[SELUPAN_SKILL_ICESTRIKE] = 1;
	this->m_PatternSkill[STATE_PATTERN5].IsSkillEnabled[SELUPAN_SKILL_SUMMON] = 1;
	this->m_PatternSkill[STATE_PATTERN5].IsSkillEnabled[SELUPAN_SKILL_HEAL] = 1;
	this->m_PatternSkill[STATE_PATTERN5].IsSkillEnabled[SELUPAN_SKILL_FREEZE] = 1;
	this->m_PatternSkill[STATE_PATTERN5].IsSkillEnabled[SELUPAN_SKILL_TELEPORT] = 1;

	this->m_PatternSkill[STATE_PATTERN6].IsSkillEnabled[SELUPAN_SKILL_POISON_ATTACK] = 1;
	this->m_PatternSkill[STATE_PATTERN6].IsSkillEnabled[SELUPAN_SKILL_ICESTORM] = 1;
	this->m_PatternSkill[STATE_PATTERN6].IsSkillEnabled[SELUPAN_SKILL_ICESTRIKE] = 1;
	this->m_PatternSkill[STATE_PATTERN6].IsSkillEnabled[SELUPAN_SKILL_SUMMON] = 1;
	this->m_PatternSkill[STATE_PATTERN6].IsSkillEnabled[SELUPAN_SKILL_HEAL] = 1;
	this->m_PatternSkill[STATE_PATTERN6].IsSkillEnabled[SELUPAN_SKILL_FREEZE] = 1;
	this->m_PatternSkill[STATE_PATTERN6].IsSkillEnabled[SELUPAN_SKILL_TELEPORT] = 1;

	this->m_PatternSkill[STATE_PATTERN7].IsSkillEnabled[SELUPAN_SKILL_POISON_ATTACK] = 1;
	this->m_PatternSkill[STATE_PATTERN7].IsSkillEnabled[SELUPAN_SKILL_ICESTORM] = 1;
	this->m_PatternSkill[STATE_PATTERN7].IsSkillEnabled[SELUPAN_SKILL_ICESTRIKE] = 1;
	this->m_PatternSkill[STATE_PATTERN7].IsSkillEnabled[SELUPAN_SKILL_SUMMON] = 1;
	this->m_PatternSkill[STATE_PATTERN7].IsSkillEnabled[SELUPAN_SKILL_HEAL] = 1;
	this->m_PatternSkill[STATE_PATTERN7].IsSkillEnabled[SELUPAN_SKILL_FREEZE] = 1;
	this->m_PatternSkill[STATE_PATTERN7].IsSkillEnabled[SELUPAN_SKILL_TELEPORT] = 1;
	this->m_PatternSkill[STATE_PATTERN7].IsSkillEnabled[SELUPAN_SKILL_INVINCIBILITY] = 1;

	this->SetSuccessValue(0);

	this->SetBattleOfSelupanState(SELUPAN_STATE_NONE);
}

void CRaklionBattleOfSelupan::Run() // OK
{
	if((OBJECT_RANGE(this->m_RaklionSelupan.GetSelupanObjIndex()) == 0 || this->m_RaklionSelupan.GetSelupanLife() <= 0) && this->GetBattleOfSelupanState() != SELUPAN_STATE_DIE)
	{
		this->SetState(SELUPAN_STATE_DIE);
		return;
	}

	switch(this->m_BattleOfSelupanState)
	{
		case SELUPAN_STATE_NONE:
			this->ProcState_NONE();
			break;
		case SELUPAN_STATE_STANDBY:
			this->ProcState_STANDBY();
			break;
		case SELUPAN_STATE_PATTERN1:
			this->ProcState_PATTERN1();
			break;
		case SELUPAN_STATE_PATTERN2:
			this->ProcState_PATTERN2();
			break;
		case SELUPAN_STATE_PATTERN3:
			this->ProcState_PATTERN3();
			break;
		case SELUPAN_STATE_PATTERN4:
			this->ProcState_PATTERN4();
			break;
		case SELUPAN_STATE_PATTERN5:
			this->ProcState_PATTERN5();
			break;
		case SELUPAN_STATE_PATTERN6:
			this->ProcState_PATTERN6();
			break;
		case SELUPAN_STATE_PATTERN7:
			this->ProcState_PATTERN7();
			break;
		case SELUPAN_STATE_DIE:
			this->ProcState_DIE();
			break;
	}
}

void CRaklionBattleOfSelupan::ProcState_NONE() // OK
{

}

void CRaklionBattleOfSelupan::ProcState_STANDBY() // OK
{

}

void CRaklionBattleOfSelupan::ProcState_PATTERN1() // OK
{
	if(this->m_SkillDelay > (GetTickCount()-this->m_SkillDelayTick))
	{
		return;
	}

	int skill = 0;

	while(this->m_PatternSkill[STATE_PATTERN1].IsSkillEnabled[skill] == 0)
	{
		skill = GetLargeRand()%MAX_SELUPAN_SKILL;
	}

	this->PatternStateSkillUseProc(skill);
}

void CRaklionBattleOfSelupan::ProcState_PATTERN2() // OK
{
	if(this->m_SkillDelay > (GetTickCount()-this->m_SkillDelayTick))
	{
		return;
	}

	int skill = 0;

	while(this->m_PatternSkill[STATE_PATTERN2].IsSkillEnabled[skill] == 0)
	{
		skill = GetLargeRand()%MAX_SELUPAN_SKILL;
	}

	this->PatternStateSkillUseProc(skill);
}

void CRaklionBattleOfSelupan::ProcState_PATTERN3() // OK
{
	if(this->m_SkillDelay > (GetTickCount()-this->m_SkillDelayTick))
	{
		return;
	}

	int skill = 0;

	while(this->m_PatternSkill[STATE_PATTERN3].IsSkillEnabled[skill] == 0)
	{
		skill = GetLargeRand()%MAX_SELUPAN_SKILL;
	}

	this->PatternStateSkillUseProc(skill);
}

void CRaklionBattleOfSelupan::ProcState_PATTERN4() // OK
{
	if(this->m_SkillDelay > (GetTickCount()-this->m_SkillDelayTick))
	{
		return;
	}

	int skill = 0;

	while(this->m_PatternSkill[STATE_PATTERN4].IsSkillEnabled[skill] == 0)
	{
		skill = GetLargeRand()%MAX_SELUPAN_SKILL;
	}

	this->PatternStateSkillUseProc(skill);
}

void CRaklionBattleOfSelupan::ProcState_PATTERN5() // OK
{
	if(this->m_SkillDelay > (GetTickCount()-this->m_SkillDelayTick))
	{
		return;
	}

	int skill = 0;

	while(this->m_PatternSkill[STATE_PATTERN5].IsSkillEnabled[skill] == 0)
	{
		skill = GetLargeRand()%MAX_SELUPAN_SKILL;
	}

	this->PatternStateSkillUseProc(skill);
}

void CRaklionBattleOfSelupan::ProcState_PATTERN6() // OK
{
	if(this->m_SkillDelay > (GetTickCount()-this->m_SkillDelayTick))
	{
		return;
	}

	int skill = 0;

	while(this->m_PatternSkill[STATE_PATTERN6].IsSkillEnabled[skill] == 0)
	{
		skill = GetLargeRand()%MAX_SELUPAN_SKILL;
	}

	this->PatternStateSkillUseProc(skill);
}

void CRaklionBattleOfSelupan::ProcState_PATTERN7() // OK
{
	if(this->m_SkillDelay > (GetTickCount()-this->m_SkillDelayTick))
	{
		return;
	}

	int skill = 0;

	while(this->m_PatternSkill[STATE_PATTERN7].IsSkillEnabled[skill] == 0)
	{
		skill = GetLargeRand()%MAX_SELUPAN_SKILL;
	}

	this->PatternStateSkillUseProc(skill);
}

void CRaklionBattleOfSelupan::ProcState_DIE() // OK
{

}

void CRaklionBattleOfSelupan::SetState(int state) // OK
{
	switch(state)
	{
		case SELUPAN_STATE_NONE:
			this->SetState_NONE();
			break;
		case SELUPAN_STATE_STANDBY:
			this->SetState_STANDBY();
			break;
		case SELUPAN_STATE_PATTERN1:
			this->SetState_PATTERN1();
			break;
		case SELUPAN_STATE_PATTERN2:
			this->SetState_PATTERN2();
			break;
		case SELUPAN_STATE_PATTERN3:
			this->SetState_PATTERN3();
			break;
		case SELUPAN_STATE_PATTERN4:
			this->SetState_PATTERN4();
			break;
		case SELUPAN_STATE_PATTERN5:
			this->SetState_PATTERN5();
			break;
		case SELUPAN_STATE_PATTERN6:
			this->SetState_PATTERN6();
			break;
		case SELUPAN_STATE_PATTERN7:
			this->SetState_PATTERN7();
			break;
		case SELUPAN_STATE_DIE:
			this->SetState_DIE();
			break;
	}

	gRaklionUtil.NotifyRaklionChangeState(11,this->GetBattleOfSelupanState());
}

void CRaklionBattleOfSelupan::SetState_NONE() // OK
{
	//LogAdd(LOG_BLACK,"[ RAKLION ][ BattleOfSelupan ] State(%d) -> NONE",this->m_BattleOfSelupanState);

	this->m_RaklionSelupan.SelupanAct_BerserkCansel();

	this->SetBattleOfSelupanState(SELUPAN_STATE_NONE);
}

void CRaklionBattleOfSelupan::SetState_STANDBY() // OK
{
	//LogAdd(LOG_BLACK,"[ RAKLION ][ BattleOfSelupan ] State(%d) -> STANDBY",this->m_BattleOfSelupanState);

	this->m_RaklionSelupan.CreateSelupan();

	this->m_RaklionSelupan.SelupanAct_BerserkCansel();

	this->SetBattleOfSelupanState(SELUPAN_STATE_STANDBY);
}

void CRaklionBattleOfSelupan::SetState_PATTERN1() // OK
{
	//LogAdd(LOG_BLACK,"[ RAKLION ][ BattleOfSelupan ] State(%d) -> PATTERN1 // Life(%d/%d)",this->m_BattleOfSelupanState,this->m_RaklionSelupan.GetSelupanLife(),this->m_RaklionSelupan.GetSelupanMaxLife());

	this->m_RaklionSelupan.SelupanAct_BerserkCansel();

	this->SetBattleOfSelupanState(SELUPAN_STATE_PATTERN1);
}

void CRaklionBattleOfSelupan::SetState_PATTERN2() // OK
{
	//LogAdd(LOG_BLACK,"[ RAKLION ][ BattleOfSelupan ] State(%d) -> PATTERN2 // Life(%d/%d)",this->m_BattleOfSelupanState,this->m_RaklionSelupan.GetSelupanLife(),this->m_RaklionSelupan.GetSelupanMaxLife());

	this->m_RaklionSelupan.SelupanAct_Berserk1();

	this->SetBattleOfSelupanState(SELUPAN_STATE_PATTERN2);
}

void CRaklionBattleOfSelupan::SetState_PATTERN3() // OK
{
	//LogAdd(LOG_BLACK,"[ RAKLION ][ BattleOfSelupan ] State(%d) -> PATTERN3 // Life(%d/%d)",this->m_BattleOfSelupanState,this->m_RaklionSelupan.GetSelupanLife(),this->m_RaklionSelupan.GetSelupanMaxLife());

	this->m_RaklionSelupan.SelupanAct_Berserk2();

	this->SetBattleOfSelupanState(SELUPAN_STATE_PATTERN3);
}

void CRaklionBattleOfSelupan::SetState_PATTERN4() // OK
{
	//LogAdd(LOG_BLACK,"[ RAKLION ][ BattleOfSelupan ] State(%d) -> PATTERN4 // Life(%d/%d)",this->m_BattleOfSelupanState,this->m_RaklionSelupan.GetSelupanLife(),this->m_RaklionSelupan.GetSelupanMaxLife());

	this->m_RaklionSelupan.SelupanAct_Berserk2();

	this->SetBattleOfSelupanState(SELUPAN_STATE_PATTERN4);
}

void CRaklionBattleOfSelupan::SetState_PATTERN5() // OK
{
	//LogAdd(LOG_BLACK,"[ RAKLION ][ BattleOfSelupan ] State(%d) -> PATTERN5 // Life(%d/%d)",this->m_BattleOfSelupanState,this->m_RaklionSelupan.GetSelupanLife(),this->m_RaklionSelupan.GetSelupanMaxLife());

	this->m_RaklionSelupan.SelupanAct_Berserk3();

	this->SetBattleOfSelupanState(SELUPAN_STATE_PATTERN5);
}

void CRaklionBattleOfSelupan::SetState_PATTERN6() // OK
{
	//LogAdd(LOG_BLACK,"[ RAKLION ][ BattleOfSelupan ] State(%d) -> PATTERN6 // Life(%d/%d)",this->m_BattleOfSelupanState,this->m_RaklionSelupan.GetSelupanLife(),this->m_RaklionSelupan.GetSelupanMaxLife());

	this->m_RaklionSelupan.SelupanAct_Berserk4();

	this->SetBattleOfSelupanState(SELUPAN_STATE_PATTERN6);
}

void CRaklionBattleOfSelupan::SetState_PATTERN7() // OK
{
	//LogAdd(LOG_BLACK,"[ RAKLION ][ BattleOfSelupan ] State(%d) -> PATTERN7 // Life(%d/%d)",this->m_BattleOfSelupanState,this->m_RaklionSelupan.GetSelupanLife(),this->m_RaklionSelupan.GetSelupanMaxLife());

	this->m_RaklionSelupan.SelupanAct_Berserk4();

	this->SetBattleOfSelupanState(SELUPAN_STATE_PATTERN7);
}

void CRaklionBattleOfSelupan::SetState_DIE() // OK
{
	//LogAdd(LOG_BLACK,"[ RAKLION ][ BattleOfSelupan ] State(%d) -> DIE",this->m_BattleOfSelupanState);

	this->DeleteSummonMonster();

	this->SetBattleOfSelupanState(SELUPAN_STATE_DIE);
}

void CRaklionBattleOfSelupan::PatternStateSkillUseProc(int skill) // OK
{
	if(OBJECT_RANGE(this->m_RaklionSelupan.GetSelupanObjIndex()) == 0)
	{
		return;
	}

	int aIndex = this->m_RaklionSelupan.GetSelupanObjIndex();

	if(gObj[aIndex].Teleport != 0 || gObj[aIndex].ActionState.Move != 0)
	{
		return;
	}

	switch(skill)
	{
		case SELUPAN_SKILL_FIRSTSKILL:
			this->m_RaklionSelupan.SelupanAct_FirstSkill();
			break;
		case SELUPAN_SKILL_POISON_ATTACK:
			this->m_RaklionSelupan.SelupanAct_PoisonAttack();
			break;
		case SELUPAN_SKILL_ICESTORM:
			this->m_RaklionSelupan.SelupanAct_IceStorm();
			break;
		case SELUPAN_SKILL_ICESTRIKE:
			this->m_RaklionSelupan.SelupanAct_IceStrike();
			break;
		case SELUPAN_SKILL_SUMMON:
			this->m_RaklionSelupan.SelupanAct_SummonMonster();
			break;
		case SELUPAN_SKILL_HEAL:
			this->m_RaklionSelupan.SelupanAct_Heal();
			break;
		case SELUPAN_SKILL_FREEZE:
			this->m_RaklionSelupan.SelupanAct_Freeze();
			break;
		case SELUPAN_SKILL_TELEPORT:
			this->m_RaklionSelupan.SelupanAct_Teleport();
			break;
		case SELUPAN_SKILL_INVINCIBILITY:
			this->m_RaklionSelupan.SelupanAct_Invincibility();
			break;
	}

	this->m_SkillDelay = this->m_RaklionSelupan.GetSelupanSkillDelay();

	this->m_SkillDelayTick = GetTickCount();
}

void CRaklionBattleOfSelupan::SetPatternCondition(int pattern,int condition) // OK
{
	if(pattern < 0 || condition < 0)
	{
		//LogAdd(LOG_RED,"[ RAKLION ][ SetPatternCondition ] error : PatternNum(%d), Condition(%d)",pattern,condition);
		return;
	}

	this->m_PatternCondition[pattern] = condition;
}

void CRaklionBattleOfSelupan::SelupanLifeCheck() // OK
{
	if(OBJECT_RANGE(this->m_RaklionSelupan.GetSelupanObjIndex()) == 0)
	{
		return;
	}

	int aIndex = this->m_RaklionSelupan.GetSelupanObjIndex();

	if(gObj[aIndex].DieRegen != 0)
	{
		return;
	}

	if(this->m_RaklionSelupan.GetSelupanMaxLife() == 0)
	{
		return;
	}

	if(this->m_IsUseFirstSkill == 0)
	{
		this->m_IsUseFirstSkill = 1;
		this->PatternStateSkillUseProc(SELUPAN_SKILL_FIRSTSKILL);
		return;
	}

	if(((this->m_RaklionSelupan.GetSelupanLife()*100)/this->m_RaklionSelupan.GetSelupanMaxLife()) > this->m_PatternCondition[0])
	{
		if(this->GetBattleOfSelupanState() == SELUPAN_STATE_PATTERN1)
		{
			this->SetBattleOfSelupanState(SELUPAN_STATE_PATTERN1);
			return;
		}
		else
		{
			this->SetState(SELUPAN_STATE_PATTERN1);
			return;
		}
	}

	if(((this->m_RaklionSelupan.GetSelupanLife()*100)/this->m_RaklionSelupan.GetSelupanMaxLife()) > this->m_PatternCondition[1])
	{
		if(this->GetBattleOfSelupanState() == SELUPAN_STATE_PATTERN2)
		{
			this->SetBattleOfSelupanState(SELUPAN_STATE_PATTERN2);
			return;
		}
		else
		{
			this->SetState(SELUPAN_STATE_PATTERN2);
			return;
		}
	}

	if(((this->m_RaklionSelupan.GetSelupanLife()*100)/this->m_RaklionSelupan.GetSelupanMaxLife()) > this->m_PatternCondition[2])
	{
		if(this->GetBattleOfSelupanState() == SELUPAN_STATE_PATTERN3)
		{
			this->SetBattleOfSelupanState(SELUPAN_STATE_PATTERN3);
			return;
		}
		else
		{
			this->SetState(SELUPAN_STATE_PATTERN3);
			return;
		}
	}

	if(((this->m_RaklionSelupan.GetSelupanLife()*100)/this->m_RaklionSelupan.GetSelupanMaxLife()) > this->m_PatternCondition[3])
	{
		if(this->GetBattleOfSelupanState() == SELUPAN_STATE_PATTERN4)
		{
			this->SetBattleOfSelupanState(SELUPAN_STATE_PATTERN4);
			return;
		}
		else
		{
			this->SetState(SELUPAN_STATE_PATTERN4);
			return;
		}
	}

	if(((this->m_RaklionSelupan.GetSelupanLife()*100)/this->m_RaklionSelupan.GetSelupanMaxLife()) > this->m_PatternCondition[4])
	{
		if(this->GetBattleOfSelupanState() == SELUPAN_STATE_PATTERN5)
		{
			this->SetBattleOfSelupanState(SELUPAN_STATE_PATTERN5);
			return;
		}
		else
		{
			this->SetState(SELUPAN_STATE_PATTERN5);
			return;
		}
	}

	if(((this->m_RaklionSelupan.GetSelupanLife()*100)/this->m_RaklionSelupan.GetSelupanMaxLife()) > this->m_PatternCondition[5])
	{
		if(this->GetBattleOfSelupanState() == SELUPAN_STATE_PATTERN6)
		{
			this->SetBattleOfSelupanState(SELUPAN_STATE_PATTERN6);
			return;
		}
		else
		{
			this->SetState(SELUPAN_STATE_PATTERN6);
			return;
		}
	}

	if(((this->m_RaklionSelupan.GetSelupanLife()*100)/this->m_RaklionSelupan.GetSelupanMaxLife()) < this->m_PatternCondition[5])
	{
		if(this->GetBattleOfSelupanState() == SELUPAN_STATE_PATTERN7)
		{
			this->SetBattleOfSelupanState(SELUPAN_STATE_PATTERN7);
			return;
		}
		else
		{
			this->SetState(SELUPAN_STATE_PATTERN7);
			return;
		}
	}
}

void CRaklionBattleOfSelupan::CreateSelupan() // OK
{
	this->m_RaklionSelupan.CreateSelupan();
}

void CRaklionBattleOfSelupan::DeleteSelupan() // OK
{
	this->m_RaklionSelupan.DeleteSelupan();
}

void CRaklionBattleOfSelupan::DeleteSummonMonster() // OK
{
	this->m_RaklionSelupan.DeleteSummonMonster();
}

void CRaklionBattleOfSelupan::SetSelupanObjIndex(int index) // OK
{
	this->m_RaklionSelupan.SetSelupanObjIndex(index);
}

void CRaklionBattleOfSelupan::SetSelupanSkillDelay(int delay) // OK
{
	this->m_RaklionSelupan.SetSelupanSkillDelay(delay);
}

void CRaklionBattleOfSelupan::SetBattleOfSelupanState(int state) // OK
{
	this->m_BattleOfSelupanState = state;
}

void CRaklionBattleOfSelupan::SetSuccessValue(int success) // OK
{
	this->m_IsSucccess = success;
}

int CRaklionBattleOfSelupan::GetSuccessValue() // OK
{
	return this->m_IsSucccess;
}

int CRaklionBattleOfSelupan::GetBattleUserCount() // OK
{
	return gRaklionBattleUserMng.GetBattleUserCount();
}

int CRaklionBattleOfSelupan::GetSelupanObjIndex() // OK
{
	return this->m_RaklionSelupan.GetSelupanObjIndex();
}

int CRaklionBattleOfSelupan::GetBattleOfSelupanState() // OK
{
	return this->m_BattleOfSelupanState;
}
