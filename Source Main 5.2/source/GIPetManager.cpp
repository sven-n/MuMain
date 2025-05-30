//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PersonalShopTitleImp.h"
#include "CSPetSystem.h"
#include "UIManager.h"
#include "ZzzOpenglUtil.h"
#include "zzzInfomation.h"
#include "zzzBmd.h"
#include "zzztexture.h"
#include "zzzCharacter.h"
#include "zzzInterface.h"
#include "zzzinventory.h"

#include "GIPetManager.h"
#include "MapManager.h"
#include "ZzzEffect.h"
#include "CharacterManager.h"
#include "DSPlaySound.h"
#include "NewUISystem.h"

extern  bool    SkillEnable;
extern	wchar_t TextList[50][100];
extern	int     TextListColor[50];
extern	int     TextBold[50];
extern  int     CheckX;
extern  int     CheckY;
extern  int     CheckSkill;

namespace giPetManager
{
    static  BYTE    g_byTabBar = 0;
    static  DWORD   g_iRenderItemIndexBackup = 0;
    static  ITEM    g_RenderItemInfoBackup;// = { 0, };

    static  const   BYTE    g_byCommandNum = 4;

    void InitPetManager(void)
    {
        g_byTabBar = 0;
        gs_PetInfo.m_dwPetType = PET_TYPE_NONE;
    }

    void CreatePetDarkSpirit(CHARACTER* c)
    {
        DeletePet(c);

        if (gMapManager.InChaosCastle() == true) return;

        CSPetSystem* pPet = new CSPetDarkSpirit(c);
        c->m_pPet = (void*)pPet;
    }

    void CreatePetDarkSpirit_Now(CHARACTER* c)
    {
        if (c->Weapon[1].Type == MODEL_DARK_RAVEN_ITEM)
        {
            DeletePet(c);
            CSPetSystem* pPet = new CSPetDarkSpirit(c);
            c->m_pPet = (void*)pPet;
        }
    }

    void MovePet(CHARACTER* c)
    {
        if (c->m_pPet != NULL)
        {
            auto* pPet = (CSPetSystem*)c->m_pPet;
            pPet->MovePet();
        }
    }

    void RenderPet(CHARACTER* c)
    {
        OBJECT* o = &c->Object;
        if (c->m_pPet != NULL)
        {
            auto* pPet = (CSPetSystem*)c->m_pPet;

            if (g_isCharacterBuff(o, eBuff_Cloaking))
            {
                pPet->RenderPet(10);
            }
            else
            {
                ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
                PET_INFO* pPetInfo = giPetManager::GetPetInfo(pEquipmentItemSlot);
                pPet->SetPetInfo(pPetInfo);
                //pPet->SetPetInfo((PET_INFO*)c->m_pPet);
                pPet->RenderPet();
            }
        }
    }

    bool SelectPetCommand(void)
    {
        if (gCharacterManager.GetBaseClass(Hero->Class) != CLASS_DARK_LORD)
            return false;
        if (HIBYTE(GetAsyncKeyState(VK_SHIFT)) == 128)
        {
            for (int i = 0; i < (AT_PET_COMMAND_END - AT_PET_COMMAND_DEFAULT); i++)
            {
                if (HIBYTE(GetAsyncKeyState('1' + i)))
                {
                    Hero->CurrentSkill = AT_PET_COMMAND_DEFAULT + i;
                    return true;
                }
            }
        }
        return false;
    }
    void MovePetCommand(CHARACTER* c)
    {
        if (c->m_pPet != NULL)
        {
            int SkillCount = 0;
            int Width, Height, x, y;
            for (int i = AT_PET_COMMAND_DEFAULT; i < AT_PET_COMMAND_END; i++)
            {
                Width = 32; Height = 36; x = 320 - (AT_PET_COMMAND_END - AT_PET_COMMAND_DEFAULT) * Width / 2 + SkillCount * Width; y = 330;
                SkillCount++;

                if (MouseX >= x && MouseX < x + Width && MouseY >= y && MouseY < y + Height)
                {
                    CheckSkill = i;
                    CheckX = x + Width / 2;
                    CheckY = y;
                    MouseOnWindow = true;
                    if (MouseLButtonPush)
                    {
                        MouseLButtonPush = false;
                        Hero->CurrentSkill = i;
                        SkillEnable = false;
                        PlayBuffer(SOUND_CLICK01);
                        MouseUpdateTime = 0;
                        MouseUpdateTimeMax = 6;
                    }
                }
            }
        }
    }

    bool SendPetCommand(CHARACTER* c, int Index)
    {
        if (c->m_pPet != NULL)
        {
            if (MouseRButtonPush || MouseRButton)
            {
                if (Index >= AT_PET_COMMAND_DEFAULT && Index < AT_PET_COMMAND_END)
                {
                    auto* pPet = (CSPetSystem*)c->m_pPet;

                    if (Index == AT_PET_COMMAND_TARGET)
                    {
                        if (CheckAttack())
                        {
                            CHARACTER* tc = &CharactersClient[SelectedCharacter];
                            if (SelectedCharacter != -1 && (tc->Object.Kind == KIND_MONSTER || tc->Object.Kind == KIND_PLAYER))
                            {
                                SocketClient->ToGameServer()->SendPetCommandRequest(pPet->GetPetType(), Index - AT_PET_COMMAND_DEFAULT, tc->Key);
                            }
                        }
                    }
                    else
                    {
                        SocketClient->ToGameServer()->SendPetCommandRequest(pPet->GetPetType(), Index - AT_PET_COMMAND_DEFAULT, 0xFFFF);
                    }
                    MouseRButtonPop = false;
                    MouseRButtonPush = false;
                    MouseRButton = false;

                    MouseRButtonPress = 0;
                    return true;
                }
            }
        }
        return false;
    }

    void SetPetCommand(CHARACTER* c, int Key, BYTE Cmd)
    {
        if (c->m_pPet != NULL)
        {
            auto* pPet = (CSPetSystem*)c->m_pPet;
            pPet->SetCommand(Key, Cmd);
        }
    }

    void SetAttack(CHARACTER* c, int Key, int attackType)
    {
        if (c->m_pPet != NULL)
        {
            auto* pPet = (CSPetSystem*)c->m_pPet;
            pPet->SetAttack(Key, attackType);
        }
    }

    bool RenderPetCmdInfo(int sx, int sy, int Type)
    {
        if (Type < AT_PET_COMMAND_DEFAULT || Type >= AT_PET_COMMAND_END) return false;

        int  TextNum = 0;
        int  SkipNum = 0;

        if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD)
        {
            int cmdType = Type - AT_PET_COMMAND_DEFAULT;

            TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = true;
            swprintf(TextList[TextNum], GlobalText[1219 + cmdType]); TextNum++; SkipNum++;

            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
            swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
            switch (cmdType)
            {
            case PET_CMD_DEFAULT: swprintf(TextList[TextNum], GlobalText[1223]); TextNum++; SkipNum++; break;
            case PET_CMD_RANDOM: swprintf(TextList[TextNum], GlobalText[1224]); TextNum++; SkipNum++; break;
            case PET_CMD_OWNER: swprintf(TextList[TextNum], GlobalText[1225]); TextNum++; SkipNum++; break;
            case PET_CMD_TARGET: swprintf(TextList[TextNum], GlobalText[1226]); TextNum++; SkipNum++; break;
            }

            SIZE TextSize = { 0, 0 };
            GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);
            int Height = (int)(((TextNum - SkipNum) * TextSize.cy + SkipNum * TextSize.cy / 2) / g_fScreenRate_y);
            sy -= Height;

            RenderTipTextList(sx, sy, TextNum, 0);
            return true;
        }
        return false;
    }

    void DeletePet(CHARACTER* c)
    {
        if (c->m_pPet != NULL)
        {
            if (c->m_pPet != NULL)
            {
                int iObjectType = ((CSPetSystem*)c->m_pPet)->GetObjectType();
                TerminateOwnerEffectObject(iObjectType);
            }

            delete ((CSPetSystem*)c->m_pPet);
            c->m_pPet = NULL;
        }
    }

    void InitItemBackup(void)
    {
        ZeroMemory(&g_RenderItemInfoBackup, sizeof(ITEM));
        gs_PetInfo.m_dwPetType = PET_TYPE_NONE;
    }

    bool RequestPetInfo(int sx, int sy, ITEM* pItem)
    {
        DWORD itemIndex = MAKELONG(sx, sy);
        if (gs_PetInfo.m_dwPetType == PET_TYPE_NONE || g_iRenderItemIndexBackup != itemIndex
            || g_RenderItemInfoBackup.Type != pItem->Type || g_RenderItemInfoBackup.Level != pItem->Level)
        {
            g_iRenderItemIndexBackup = itemIndex;
            g_RenderItemInfoBackup.Type = pItem->Type;
            g_RenderItemInfoBackup.Level = pItem->Level;

            BYTE PetType = PET_TYPE_DARK_SPIRIT;
            if (pItem->Type == ITEM_DARK_HORSE_ITEM)
            {
                PetType = PET_TYPE_DARK_HORSE;
            }

            int iInvenType = 0, iItemIndex = 0;

            if ((iItemIndex = g_pMyInventory->GetPointedItemIndex()) != -1)
            {
                iInvenType = 0;
            }
            else if ((iItemIndex = g_pMyShopInventory->GetPointedItemIndex()) != -1)
            {
                iInvenType = 0;
            }
            else if ((iItemIndex = g_pStorageInventory->GetPointedItemIndex()) != -1)
            {
                iInvenType = 1;
            }
            else if ((iItemIndex = g_pStorageInventoryExt->GetPointedItemIndex()) != -1)
            {
                iInvenType = 1;
            }
            else if ((iItemIndex = g_pTrade->GetPointedItemIndexMyInven()) != -1)
            {
                iInvenType = 2;
            }
            else if ((iItemIndex = g_pTrade->GetPointedItemIndexYourInven()) != -1)
            {
                iInvenType = 3;
            }
            else if ((iItemIndex = g_pMixInventory->GetPointedItemIndex()) != -1)
            {
                iInvenType = 4;
            }
            else if ((iItemIndex = g_pPurchaseShopInventory->GetPointedItemIndex()) != -1)
            {
                iInvenType = 5;
            }
            else if ((iItemIndex = g_pNPCShop->GetPointedItemIndex()) != -1)
            {
                iInvenType = 6;
            }

            SocketClient->ToGameServer()->SendPetInfoRequest(PetType, iInvenType, iItemIndex);

            return true;
        }
        return false;
    }

    void SetPetInfo(BYTE InvType, BYTE InvPos, PET_INFO* pPetinfo)
    {
        CalcPetInfo(pPetinfo);

        if ((InvType == 0) || (InvType == 254) || (InvType == 255))
        {
            if ((InvPos == EQUIPMENT_HELPER) || (InvPos == EQUIPMENT_WEAPON_LEFT))
            {
                PET_INFO* pHeroPetInfo = Hero->GetEquipedPetInfo(pPetinfo->m_dwPetType);
                memcpy(pHeroPetInfo, pPetinfo, sizeof(PET_INFO));

                if (pPetinfo->m_dwPetType == PET_TYPE_DARK_SPIRIT)
                {
                    if (Hero->m_pPet == NULL)
                        return;

                    ((CSPetSystem*)Hero->m_pPet)->SetPetInfo(pHeroPetInfo);

                    if (InvType == 254)
                    {
                        ((CSPetSystem*)Hero->m_pPet)->Eff_LevelUp();
                    }
                    else if (InvType == 255)
                    {
                        ((CSPetSystem*)Hero->m_pPet)->Eff_LevelDown();
                    }
                }
                else if (pPetinfo->m_dwPetType == PET_TYPE_DARK_HORSE)
                {
                    if (InvType == 254 || InvType == 255)
                    {
                        Hero->Object.ExtState = InvType - 253;
                    }

                    SetPetItemConvert(&CharacterMachine->Equipment[EQUIPMENT_HELPER], pHeroPetInfo);
                }

                CharacterMachine->CalculateAll();

                //return;
            }
        }

        memcpy(&gs_PetInfo, pPetinfo, sizeof(PET_INFO));
    }

    PET_INFO* GetPetInfo(ITEM* pItem)
    {
        if (pItem == &CharacterMachine->Equipment[EQUIPMENT_HELPER])
        {
            return Hero->GetEquipedPetInfo(PET_TYPE_DARK_HORSE);
        }
        else if (pItem == &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT])
        {
            return Hero->GetEquipedPetInfo(PET_TYPE_DARK_SPIRIT);
        }
        return &gs_PetInfo;
    }

    void CalcPetInfo(PET_INFO* pPetInfo)
    {
        int Charisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
        int Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;

        int Level = pPetInfo->m_wLevel + 1;

        switch (pPetInfo->m_dwPetType)
        {
        case PET_TYPE_DARK_SPIRIT:
            pPetInfo->m_dwExp2 = ((10 + Level) * Level * Level * Level * 100);
            pPetInfo->m_wDamageMin = (180 + (pPetInfo->m_wLevel * 15) + (Charisma / 8));
            pPetInfo->m_wDamageMax = (200 + (pPetInfo->m_wLevel * 15) + (Charisma / 4));
            pPetInfo->m_wAttackSpeed = (20 + (pPetInfo->m_wLevel * 4 / 5) + (Charisma / 50));
            pPetInfo->m_wAttackSuccess = (1000 + pPetInfo->m_wLevel) + (pPetInfo->m_wLevel * 15);
            break;

        case PET_TYPE_DARK_HORSE:
            pPetInfo->m_dwExp2 = ((10 + Level) * Level * Level * Level * 100);
            pPetInfo->m_wDamageMin = (Strength / 10) + (Charisma / 10) + (pPetInfo->m_wLevel * 5);
            pPetInfo->m_wDamageMax = pPetInfo->m_wDamageMin + (pPetInfo->m_wDamageMin / 2);
            pPetInfo->m_wAttackSpeed = (20 + (pPetInfo->m_wLevel * 4 / 5) + (Charisma / 50));
            pPetInfo->m_wAttackSuccess = (1000 + pPetInfo->m_wLevel) + (pPetInfo->m_wLevel * 15);
            break;
        }
    }

    void SetPetItemConvert(ITEM* ip, PET_INFO* pPetInfo)
    {
        if (ip->Type == ITEM_DARK_HORSE_ITEM)
        {
            int Index = 0;
            for (int i = 0; i < ip->SpecialNum; ++i)
            {
                if (ip->Special[i] == AT_SET_OPTION_IMPROVE_DEFENCE)
                {
                    Index = i;
                    break;
                }
            }

            if (Index == 0)
            {
                ip->SpecialValue[ip->SpecialNum] = static_cast<BYTE>((5 + (CharacterAttribute->Dexterity / 20) + pPetInfo->m_wLevel * 2) & 0xFF);
                ip->Special[ip->SpecialNum] = AT_SET_OPTION_IMPROVE_DEFENCE; ip->SpecialNum++;
            }
            else
            {
                ip->SpecialValue[Index] = static_cast<BYTE>((5 + (CharacterAttribute->Dexterity / 20) + pPetInfo->m_wLevel * 2 & 0xFF));
                ip->Special[Index] = AT_SET_OPTION_IMPROVE_DEFENCE;
            }
        }
    }

    DWORD GetPetItemValue(PET_INFO* pPetInfo)
    {
        DWORD dwGold = 0;

        if (pPetInfo->m_dwPetType == -1)
            return dwGold;

        switch (pPetInfo->m_dwPetType)
        {
        case PET_TYPE_DARK_HORSE:
            dwGold = pPetInfo->m_wLevel * 2000000;
            break;

        case PET_TYPE_DARK_SPIRIT:
            dwGold = pPetInfo->m_wLevel * 1000000;
            break;
        }

        return dwGold;
    }

    bool RenderPetItemInfo(int sx, int sy, ITEM* pItem, int iInvenType)
    {
        PET_INFO* pPetInfo = GetPetInfo(pItem);

        if (pPetInfo->m_dwPetType == PET_TYPE_NONE)
            return false;

        int TextNum = 0;
        int SkipNum = 0;
        int RequireLevel = 0;
        int RequireCharisma = 0;

        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP))
        {
            wchar_t  Text[100];
            DWORD Gold = (GetPetItemValue(&giPetManager::gs_PetInfo) / 3);
            Gold = Gold / 100 * 100;

            ConvertGold(Gold, Text);
            swprintf(TextList[TextNum], GlobalText[63], Text);

            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = true;
            TextNum++;
            swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        }
        else if ((iInvenType == SEASON3B::TOOLTIP_TYPE_MY_SHOP) || (iInvenType == SEASON3B::TOOLTIP_TYPE_PURCHASE_SHOP))
        {
            int price = 0;
            int indexInv = g_pMyShopInventory->GetInventoryCtrl()->GetIndexByItem(pItem);
            wchar_t Text[100];

            if (GetPersonalItemPrice(indexInv, price, g_IsPurchaseShop))
            {
                ConvertGold(price, Text);
                swprintf(TextList[TextNum], GlobalText[63], Text);

                if (price >= 10000000)
                    TextListColor[TextNum] = TEXT_COLOR_RED;
                else if (price >= 1000000)
                    TextListColor[TextNum] = TEXT_COLOR_YELLOW;
                else if (price >= 100000)
                    TextListColor[TextNum] = TEXT_COLOR_GREEN;
                else
                    TextListColor[TextNum] = TEXT_COLOR_WHITE;
                TextBold[TextNum] = true;
                TextNum++;
                swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

                DWORD gold = CharacterMachine->Gold;

                if (((int)gold < price) && (g_IsPurchaseShop == PSHOPWNDTYPE_PURCHASE))
                {
                    TextListColor[TextNum] = TEXT_COLOR_RED;
                    TextBold[TextNum] = true;
                    swprintf(TextList[TextNum], GlobalText[423]);
                    TextNum++;
                    swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
                }
            }
            else if (g_IsPurchaseShop == PSHOPWNDTYPE_SALE)
            {
                TextListColor[TextNum] = TEXT_COLOR_RED;
                TextBold[TextNum] = true;
                swprintf(TextList[TextNum], GlobalText[1101]);
                TextNum++;
                swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
            }
        }

        TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = true;
        if (pItem->Type == ITEM_DARK_HORSE_ITEM)
        {
            RequireLevel = (218 + (pPetInfo->m_wLevel * 2));
            RequireCharisma = 0;

            swprintf(TextList[TextNum], GlobalText[1187]); TextNum++; SkipNum++;
        }
        else if (pItem->Type == ITEM_DARK_RAVEN_ITEM)
        {
            RequireCharisma = (185 + (pPetInfo->m_wLevel * 15));

            swprintf(TextList[TextNum], GlobalText[1214]); TextNum++; SkipNum++;
        }

        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        swprintf(TextList[TextNum], GlobalText[201], pPetInfo->m_dwExp1, pPetInfo->m_dwExp2); TextNum++; SkipNum++;
        swprintf(TextList[TextNum], L"%s : %d", GlobalText[368], pPetInfo->m_wLevel); TextNum++; SkipNum++;

        if (pItem->Type == ITEM_DARK_RAVEN_ITEM)
        {
            swprintf(TextList[TextNum], GlobalText[203], pPetInfo->m_wDamageMin, pPetInfo->m_wDamageMax, pPetInfo->m_wAttackSuccess); TextNum++; SkipNum++;
            swprintf(TextList[TextNum], GlobalText[64], pPetInfo->m_wAttackSpeed); TextNum++; SkipNum++;
        }
        swprintf(TextList[TextNum], GlobalText[70], pPetInfo->m_wLife); TextNum++; SkipNum++;

        if (pItem->Type == ITEM_DARK_HORSE_ITEM)
        {
            swprintf(TextList[TextNum], GlobalText[76], RequireLevel);

            if (CharacterAttribute->Level < RequireLevel)
            {
                TextListColor[TextNum] = TEXT_COLOR_RED;
                TextBold[TextNum] = false;
                TextNum++;
                swprintf(TextList[TextNum], GlobalText[74], RequireLevel - CharacterAttribute->Level);
                TextListColor[TextNum] = TEXT_COLOR_RED;
                TextBold[TextNum] = false;
                TextNum++;
            }
            else
            {
                TextListColor[TextNum] = TEXT_COLOR_WHITE;
                TextBold[TextNum] = false;
                TextNum++;
            }
        }
        else if (pItem->Type == ITEM_DARK_RAVEN_ITEM)
        {
            swprintf(TextList[TextNum], GlobalText[698], RequireCharisma);

            WORD Charisma;
            Charisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;

            if (Charisma < RequireCharisma)
            {
                TextListColor[TextNum] = TEXT_COLOR_RED;
                TextBold[TextNum] = false;
                TextNum++;
                swprintf(TextList[TextNum], GlobalText[74], RequireCharisma - Charisma);
                TextListColor[TextNum] = TEXT_COLOR_RED;
                TextBold[TextNum] = false;
                TextNum++;
            }
            else
            {
                TextListColor[TextNum] = TEXT_COLOR_WHITE;
                TextBold[TextNum] = false;
                TextNum++;
            }
        }

        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

        if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD)
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
        else
            TextListColor[TextNum] = TEXT_COLOR_DARKRED;

        swprintf(TextList[TextNum], GlobalText[61], GlobalText[24]); TextNum++; SkipNum++;

        for (int i = 0; i < pItem->SpecialNum; ++i)
        {
            SetPetItemConvert(pItem, &gs_PetInfo);
            GetSpecialOptionText(pItem->Type, TextList[TextNum], pItem->Special[i], pItem->SpecialValue[i], 0);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false; TextNum++; SkipNum++;
        }

        if (pItem->Type == ITEM_DARK_HORSE_ITEM)
        {
            swprintf(TextList[TextNum], GlobalText[744], (30 + pPetInfo->m_wLevel) / 2);
            TextListColor[TextNum] = TEXT_COLOR_BLUE; TextNum++; SkipNum++;

            swprintf(TextList[TextNum], GlobalText[1188], 2);
            TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
        }

        SIZE TextSize = { 0, 0 };
        GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);
        int Height = (int)(((TextNum - SkipNum) * TextSize.cy + SkipNum * TextSize.cy / 2) / g_fScreenRate_y);
        if (sy - Height >= 0)
            sy -= Height;

        bool isrendertooltip = true;

        if (isrendertooltip)
        {
            RenderTipTextList(sx, sy, TextNum, 0);
        }

        return true;
    }
}