// GuardianStatue.cpp: implementation of the CGuardianStatue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GuardianStatue.h"
#include "Protocol.h"

#if(GAMESERVER_TYPE==1)

CGuardianStatue gGuardianStatue;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGuardianStatue::CGuardianStatue() // OK
{

}

CGuardianStatue::~CGuardianStatue() // OK
{

}

void CGuardianStatue::GuardianStatueAct(int aIndex) // OK
{
	int iIndex = aIndex;
	if( ::gObjIsConnected(iIndex) == FALSE )
	{
		return;
	}

	LPOBJ lpObj = &gObj[iIndex];

	if( lpObj->VPCount < 1 )
	{
		return;
	}

	int tObjNum = -1;

	for( int i = 0; i < MAX_VIEWPORT; i++ )
	{
		tObjNum = lpObj->VpPlayer[i].index;

		if( tObjNum >= 0 )
		{
			if( gObj[tObjNum].Type == OBJECT_USER )
			{
				if( gObj[tObjNum].Live != FALSE )
				{
					if( gObj[tObjNum].CsJoinSide == 1 )
					{
						if( abs(lpObj->Y - gObj[tObjNum].Y) <= 3 && abs(lpObj->X - gObj[tObjNum].X) <= 3 )
						{
							BOOL bIsMaxLife = FALSE;
							BOOL bIsMaxMana = FALSE;
							BOOL bIsMaxBP	= FALSE;

							if( gObj[tObjNum].Life < (gObj[tObjNum].MaxLife + gObj[tObjNum].AddLife) )
							{
								gObj[tObjNum].Life += ( (gObj[tObjNum].MaxLife + gObj[tObjNum].AddLife) * (lpObj->CsNpcRgLevel+1) / 100 );

								if( gObj[tObjNum].Life > (gObj[tObjNum].MaxLife + gObj[tObjNum].AddLife) )
								{
									gObj[tObjNum].Life = gObj[tObjNum].MaxLife + gObj[tObjNum].AddLife;
								}

								bIsMaxLife = TRUE;
							}
							
							if( gObj[tObjNum].Mana < (gObj[tObjNum].MaxMana + gObj[tObjNum].AddMana) )
							{
								gObj[tObjNum].Mana += ( (gObj[tObjNum].MaxMana + gObj[tObjNum].AddMana) * (lpObj->CsNpcRgLevel+1)) / 100;

								if( gObj[tObjNum].Mana > (gObj[tObjNum].MaxMana + gObj[tObjNum].AddMana) )
								{
									gObj[tObjNum].Mana = gObj[tObjNum].MaxMana + gObj[tObjNum].AddMana;
								}

								bIsMaxMana = TRUE;
							}

							if( gObj[tObjNum].BP < (gObj[tObjNum].MaxBP + gObj[tObjNum].AddBP) )
							{
								gObj[tObjNum].BP += ( (gObj[tObjNum].MaxBP + gObj[tObjNum].AddBP) * (lpObj->CsNpcRgLevel+1) / 100);

								if( gObj[tObjNum].BP > (gObj[tObjNum].MaxBP + gObj[tObjNum].AddBP) )
								{
									gObj[tObjNum].BP = gObj[tObjNum].MaxBP + gObj[tObjNum].AddBP;
								}

								bIsMaxBP = TRUE;
							}

							if( bIsMaxLife != FALSE )
							{
								GCLifeSend(tObjNum, 0xFF, (int)gObj[tObjNum].Life, gObj[tObjNum].Shield);
							}

							if( bIsMaxMana != FALSE || bIsMaxBP != FALSE )
							{
								GCManaSend(tObjNum, 0xFF, (int)gObj[tObjNum].Mana, gObj[tObjNum].BP);
							}
						}
					}
				}
			}
		}
	}
}

#endif
