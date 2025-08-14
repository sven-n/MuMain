//////////////////////////////////////////////////////////////////////////
//  npcGateSwitch.cpp
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "UIManager.h"
#include "zzzcharacter.h"
#include "ZzzTexture.h"
#include "zzzAi.h"
#include "npcGateSwitch.h"

#include "DSPlaySound.h"

namespace npcGateSwitch
{
    static  const   DWORD   GATE_OPEN = static_cast<DWORD>(eBuff_CastleGateIsOpen);
    static  const   DWORD   GATE_CLOSE = 0;
    static          bool   g_isCurrentGateopen = false;
    static          int     g_iNpcCharacterKey = 0;

    void GateOpen(CHARACTER* c, OBJECT* o)
    {
        SetAction(o, 1);

        g_CharacterRegisterBuff(o, eBuff_CastleGateIsOpen);

        battleCastle::SetCastleGate_Attribute((c->PositionX), (c->PositionY), 0);

        PlayBuffer(SOUND_BC_GATE_OPEN);
    }

    void GateClose(CHARACTER* c, OBJECT* o)
    {
        SetAction(o, 0);

        g_CharacterClearBuff(o);

        battleCastle::SetCastleGate_Attribute((c->PositionX), (c->PositionY), 1);

        PlayBuffer(SOUND_BC_GATE_OPEN);
    }

    void DoInterfaceOpen(int Key)
    {
        g_iNpcCharacterKey = Key;

        int        Index = FindCharacterIndex(Key);
        CHARACTER* c = &CharactersClient[Index];
        OBJECT* o = &c->Object;

        g_isCurrentGateopen = g_isCharacterBuff(o, eBuff_CastleGateIsOpen);
    }

    void ProcessState(int Key, BYTE GateOnOff, BYTE State)
    {
        switch (State)
        {
        case 0:
            break;

        case 1:
        {
            int        Index = FindCharacterIndex(Key);
            CHARACTER* c = &CharactersClient[Index];

            if (c == NULL) return;
            if (c->MonsterIndex != MONSTER_CASTLE_GATE1) return;

            OBJECT* o = &c->Object;

            if (GateOnOff)
            {
                GateOpen(c, o);
            }
            else
            {
                GateClose(c, o);
            }
        }
        break;

        case 2:
            break;

        case 3:
            break;

        case 4:
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

        SocketClient->ToGameServer()->SendToggleCastleGateRequest(State, g_iNpcCharacterKey);
        g_iNpcCharacterKey = 0;
    }

    bool	IsGateOpened()
    {
        return g_isCurrentGateopen;
    }
};