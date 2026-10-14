//////////////////////////////////////////////////////////////////////////
//  npcBreeder.cpp
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#ifdef _WIN32
#include <process.h>
#endif
#include "UI/Core/UIManager.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInventory.h"
#include "Render/Textures/ZzzTexture.h"
#include "I18N/All.h"

#include "GameLogic/NPCs/npcBreeder.h"
#include "GameLogic/Pets/GIPetManager.h"
#include "UI/Core/WindowSystem.h"

extern  int SrcInventoryIndex;

namespace npcBreeder
{
    // Array reference keeps the bound: a plain pointer would make mu_swprintf assume 1024
    // characters, which glibc's _FORTIFY_SOURCE aborts on once the call is inlined and the
    // real (smaller) buffer is visible. All callers pass 100-wide buffers.
    int CalcRecoveryZen(BYTE type, wchar_t (&Text)[100])
    {
        ITEM* ip;

        g_pMyInventory->SetRepairEnableLevel(false);

        switch (type)
        {
        case REVIVAL_DARKHORSE:
            ip = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
            if (ip->Type != ITEM_DARK_HORSE_ITEM)
            {
                mu_swprintf(Text, I18N::Game::PetIsNotEquipped);
                return -1;
            }
            break;

        case REVIVAL_DARKSPIRIT:
            ip = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
            if (ip->Type != ITEM_DARK_RAVEN_ITEM)
            {
                mu_swprintf(Text, I18N::Game::PetIsNotEquipped);
                return -1;
            }
            break;
        default:
            mu_swprintf(Text, I18N::Game::PetIsNotEquipped);
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
            mu_swprintf(Text, I18N::Game::LifeHasBeenRecovered);
            break;

        default:
        {
            wchar_t Text2[100] = {};

            if ((int)CharacterMachine->Gold < Gold)
            {
                ConvertGold((double)Gold - CharacterMachine->Gold, Text);
                mu_swprintf(Text2, I18N::Game::SZenIsLackingToRecoverLife, Text);
            }
            else
            {
                mu_swprintf(Text2, I18N::Game::SZenIsRequiredToRecoverLife, Text);
            }

            int Length = wcslen(Text2);
            memcpy(Text, Text2, sizeof(wchar_t) * Length);
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