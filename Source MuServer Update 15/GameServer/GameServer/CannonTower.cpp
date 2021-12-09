// CannonTower.cpp: implementation of the CCannonTower class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CannonTower.h"
#include "SkillManager.h"
#include "Util.h"

CCannonTower gCannonTower;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCannonTower::CCannonTower() // OK
{

}

CCannonTower::~CCannonTower() // OK
{

}

void CCannonTower::CannonTowerAct(int aIndex) // OK
{
	#if(GAMESERVER_TYPE==1)

	int iIndex = aIndex;
	if( ( GetLargeRand() % 2 ) != 0 )
	{
		return;
	}

	LPOBJ lpObj = &gObj[iIndex];
	int tObjNum;
	int count = 0;
	PMSG_MULTI_SKILL_ATTACK_RECV pCount;
	PMSG_MULTI_SKILL_ATTACK pAttack;
	BYTE AttackSendBuff[256];
	int ASBOfs;

	ASBOfs = 0;

	pCount.header.set(PROTOCOL_CODE4,0);

	#if(GAMESERVER_UPDATE>=701)

	pCount.skillH = 0;

	pCount.skillL = 0;

	#else

	pCount.skill[0] = 0;

	pCount.skill[1] = 0;

	#endif

	pCount.x = (BYTE)lpObj->X;

	pCount.y = (BYTE)lpObj->Y;

	pCount.serial = 0;

	pCount.count = 0;

	ASBOfs = sizeof(PMSG_MULTI_SKILL_ATTACK_RECV);

	while( true ) 
	{
		if( lpObj->VpPlayer2[count].state ) 
		{
			if( lpObj->VpPlayer2[count].type == OBJECT_USER ) 
			{
				tObjNum = lpObj->VpPlayer2[count].index;

				if( tObjNum >= 0 ) 
				{
					if( gObj[tObjNum].CsJoinSide != 1 )
					{
						if( gObjCalcDistance(lpObj, &gObj[tObjNum]) < 7 ) 
						{
							#if(GAMESERVER_UPDATE>=701)
							pAttack.indexH = SET_NUMBERHB(tObjNum);
							pAttack.indexL = SET_NUMBERLB(tObjNum);
							#else
							pAttack.index[0] = SET_NUMBERHB(tObjNum);
							pAttack.index[1] = SET_NUMBERLB(tObjNum);
							#endif
							pAttack.MagicKey = 0;
							memcpy( (AttackSendBuff+ASBOfs), (LPBYTE)&pAttack, sizeof(PMSG_MULTI_SKILL_ATTACK));
							ASBOfs += sizeof(PMSG_MULTI_SKILL_ATTACK);
							pCount.count++;
						}
					}
				}
			}
		}
		count++;

		if( count > MAX_VIEWPORT-1 ) break;
	}
	if( pCount.count > 0 ) 
	{
		pCount.header.size = ASBOfs;

		memcpy(AttackSendBuff, (LPBYTE)&pCount, sizeof(PMSG_MULTI_SKILL_ATTACK_RECV));

		gSkillManager.CGMultiSkillAttackRecv((PMSG_MULTI_SKILL_ATTACK_RECV*)AttackSendBuff, lpObj->Index, 1);

		gSkillManager.GCDurationSkillAttackSend(lpObj,SKILL_MONSTER_AREA_ATTACK,(BYTE)lpObj->X,(BYTE)lpObj->Y,0);
	}

	#endif
}
