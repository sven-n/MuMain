//////////////////////////////////////////////////////////////////////////
//  
//  npcGateSwitch.cpp
//  
//  공성 성문 스위치 처리
//
//  날  짜 : 2004/12/09
//
//  작성자 : 조규하.
//  
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//  INCLUDE.
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "UIManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "zzzinfomation.h"
#include "zzzobject.h"
#include "zzzcharacter.h"
#include "zzzinterface.h"
#include "zzzinventory.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzTexture.h"
#include "zzzAi.h"
#include "wsclientinline.h"

#include "zzzlodterrain.h"
#include "zzzeffect.h"

#include "GMBattleCastle.h"
#include "npcGateSwitch.h"


//////////////////////////////////////////////////////////////////////////
//  EXTERN.
//////////////////////////////////////////////////////////////////////////


namespace   npcGateSwitch
{
    //////////////////////////////////////////////////////////////////////////
    //  GLOBAL Variable.
    //////////////////////////////////////////////////////////////////////////
	static  const   DWORD   GATE_OPEN  = static_cast<DWORD>(eBuff_CastleGateIsOpen);
    static  const   DWORD   GATE_CLOSE = 0;
	static          bool   g_isCurrentGateopen = false;
    static          int     g_iNpcCharacterKey = 0;
    
    //////////////////////////////////////////////////////////////////////////
    //  성문 오픈.
    //////////////////////////////////////////////////////////////////////////
    void    GateOpen ( CHARACTER* c, OBJECT* o )
    {
        SetAction ( o, 1 );

		g_CharacterRegisterBuff( o, eBuff_CastleGateIsOpen);
        
        battleCastle::SetCastleGate_Attribute ( (c->PositionX), (c->PositionY), 0 );

        PlayBuffer ( SOUND_BC_GATE_OPEN );
    }
    

    //////////////////////////////////////////////////////////////////////////
    //  성문 클로즈.
    //////////////////////////////////////////////////////////////////////////
    void    GateClose ( CHARACTER* c, OBJECT* o )
    {
        SetAction ( o, 0 );

		g_CharacterClearBuff(o);
        
        battleCastle::SetCastleGate_Attribute ( (c->PositionX), (c->PositionY), 1 );

        PlayBuffer ( SOUND_BC_GATE_OPEN );
    }


    //////////////////////////////////////////////////////////////////////////
    //  성문의 정보를 설정한다.
    //////////////////////////////////////////////////////////////////////////
    void    DoInterfaceOpen ( int Key )
    {
        g_iNpcCharacterKey = Key;

        int        Index = FindCharacterIndex ( Key );
        CHARACTER* c     = &CharactersClient[Index];
        OBJECT* o = &c->Object;

		g_isCurrentGateopen = g_isCharacterBuff(o, eBuff_CastleGateIsOpen);
    }


    //////////////////////////////////////////////////////////////////////////
    //  성문의 상태를 변경시킨다.
    //////////////////////////////////////////////////////////////////////////
    void    ProcessState ( int Key, BYTE GateOnOff, BYTE State )
    {
        switch ( State )
        {
        case 0: //  실패.
            break;

        case 1: //  성공.
            {
        	    int        Index = FindCharacterIndex ( Key );
                CHARACTER* c     = &CharactersClient[Index];

                if ( c==NULL ) return;
                if ( c->MonsterIndex!=277 ) return;

                OBJECT* o = &c->Object;
                //  성문 개/폐
                if ( GateOnOff )
                {
                    GateOpen ( c, o );
                }
                else 
                {
                    GateClose ( c, o );
                }
            }
            break;

        case 2: //  성문이 없음.
            break;

        case 3: //  레버 없음.
            break;

        case 4: //  권한 없음.
            break;
        }
    }

	void	SendToggleGate()
	{
		int State = 0;
		if (!IsGateOpened())
		{
			State = 1;
		}
		SendGateOparator ( State, g_iNpcCharacterKey );
        g_iNpcCharacterKey = 0;
	}

	bool	IsGateOpened()
	{
		return g_isCurrentGateopen;
	}
};

