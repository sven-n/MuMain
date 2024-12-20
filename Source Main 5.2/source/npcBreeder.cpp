//////////////////////////////////////////////////////////////////////////
//  npcBreeder.cpp
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <process.h>
#include "UIManager.h"
#include "ZzzBMD.h"
#include "zzzinfomation.h"
#include "zzzcharacter.h"
#include "zzzinventory.h"
#include "ZzzTexture.h"

#include "npcBreeder.h"
#include "GIPetManager.h"
#include "NewUISystem.h"

extern  int SrcInventoryIndex;

namespace npcBreeder
{
    int CalcRecoveryZen(BYTE type, wchar_t* Text)
    {
        ITEM* ip;

        g_pMyInventory->SetRepairEnableLevel(false);

        switch (type)
        {
        case REVIVAL_DARKHORSE:
            ip = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
            if (ip->Type != ITEM_DARK_HORSE_ITEM)
            {
                swprintf(Text, GlobalText[1229]);
                return -1;
            }
            break;

        case REVIVAL_DARKSPIRIT:
            ip = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
            if (ip->Type != ITEM_DARK_RAVEN_ITEM)
            {
                swprintf(Text, GlobalText[1229]);
                return -1;
            }
            break;
        default:
            swprintf(Text, GlobalText[1229]);
            return -1;
        }

        ITEM_ATTRIBUTE* p = &ItemAttribute[ip->Type];

        int maxDurability = CalcMaxDurability(ip, p, ip->Level);

        int Gold = 0;
        if (ip->Durability < maxDurability)
        {
            DWORD dwValue = 0;
            dwValue = giPetManager::GetPetItemValue(giPetManager::GetPetInfo(ip));

            Gold = ConvertRepairGold(dwValue, ip->Durability, maxDurability, ip->Type, Text);
        }

        switch (Gold)
        {
        case 0:
            swprintf(Text, GlobalText[1230]);
            break;

        default:
        {
            wchar_t  Text2[100];
            memset(Text2, 0, sizeof(char) * 100);

            if ((int)CharacterMachine->Gold < Gold)
            {
                ConvertGold((double)Gold - CharacterMachine->Gold, Text);
                swprintf(Text2, GlobalText[1231], Text);
            }
            else
            {
                swprintf(Text2, GlobalText[1232], Text);
            }

            int Length = wcslen(Text2);
            memcpy(Text, Text2, sizeof(char) * Length);
            Text[Length] = 0;
        }
        break;
        }

        return Gold;
    }

    void RecoverPet(BYTE type)
    {
        wchar_t Text[100];
        int Gold = CalcRecoveryZen(type, Text);

        if ((int)CharacterMachine->Gold >= Gold && Gold != -1)
        {
            switch (type)
            {
            case REVIVAL_DARKHORSE:
                SocketClient->ToGameServer()->SendRepairItemRequest(EQUIPMENT_HELPER, (BYTE)Gold);
                break;

            case REVIVAL_DARKSPIRIT:
                SocketClient->ToGameServer()->SendRepairItemRequest(EQUIPMENT_WEAPON_LEFT, (BYTE)Gold);
                break;
            }
            giPetManager::InitItemBackup();
        }
    }
}