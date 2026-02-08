//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "GIPetManager.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <string>

#include "ZzzCharacter.h"
#include "ZzzInfomation.h"
#include "CharacterManager.h"
#include "CSPetSystem.h"
#include "DSPlaySound.h"
#include "Input.h"
#include "MapManager.h"
#include "NewUISystem.h"
#include "PersonalShopTitleImp.h"
#include "UIManager.h"
#include "ZzzBMD.h"
#include "ZzzEffect.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"

extern  bool    SkillEnable;
extern	wchar_t TextList[50][100];
extern	int     TextListColor[50];
extern	int     TextBold[50];
extern  int     CheckX;
extern  int     CheckY;
extern  int     CheckSkill;

namespace
{
    constexpr int kShiftKeyCode = 0x10;
    constexpr std::uint16_t kInvalidTargetKey = 0xFFFF;
    constexpr std::size_t kTooltipBufferCapacity = 100;
    constexpr int kPetCommandCount = AT_PET_COMMAND_END - AT_PET_COMMAND_DEFAULT;
    constexpr int kTooltipLineLimit = 50;

    std::wstring SanitizeWideStringFormat(const wchar_t* format)
    {
        if (format == nullptr)
        {
            return L"";
        }

        std::wstring sanitized;
        sanitized.reserve(std::wcslen(format) + 1);

        const wchar_t* ptr = format;
        while (*ptr != L'\0')
        {
            if (*ptr != L'%')
            {
                sanitized.push_back(*ptr++);
                continue;
            }

            sanitized.push_back(*ptr++);

            if (*ptr == L'%')
            {
                sanitized.push_back(*ptr++);
                continue;
            }

            bool hasLengthModifier = false;

            while (*ptr && std::wcschr(L"-+ #0", *ptr))
            {
                sanitized.push_back(*ptr++);
            }

            while (*ptr && std::iswdigit(*ptr))
            {
                sanitized.push_back(*ptr++);
            }

            if (*ptr == L'*')
            {
                sanitized.push_back(*ptr++);
            }

            if (*ptr == L'.')
            {
                sanitized.push_back(*ptr++);
                while (*ptr && std::iswdigit(*ptr))
                {
                    sanitized.push_back(*ptr++);
                }
                if (*ptr == L'*')
                {
                    sanitized.push_back(*ptr++);
                }
            }

            if (*ptr && std::wcschr(L"hljztL", *ptr))
            {
                hasLengthModifier = true;
                wchar_t lengthChar = *ptr;
                sanitized.push_back(lengthChar);
                ++ptr;

                if ((lengthChar == L'h' && *ptr == L'h') || (lengthChar == L'l' && *ptr == L'l'))
                {
                    sanitized.push_back(*ptr);
                    ++ptr;
                }
            }

            if ((*ptr == L's' || *ptr == L'S') && !hasLengthModifier)
            {
                sanitized.push_back(L'l');
            }

            if (*ptr != L'\0')
            {
                sanitized.push_back(*ptr++);
            }
        }

        return sanitized;
    }

    std::uint32_t ComposeItemIndex(int sx, int sy)
    {
        const auto clampedX = std::clamp(sx, 0, 0xFFFF);
        const auto clampedY = std::clamp(sy, 0, 0xFFFF);
        return static_cast<std::uint32_t>((static_cast<std::uint32_t>(clampedY) << 16) | static_cast<std::uint32_t>(clampedX));
    }

    bool HasActivePet(const CHARACTER* character)
    {
        return character != nullptr && character->m_pPet != nullptr;
    }

    bool IsVirtualKeyPressed(int virtualKey)
    {
        return CInput::Instance().IsKeyDown(virtualKey);
    }

    CSPetSystem* ResolvePetSystem(CHARACTER* character)
    {
        return HasActivePet(character) ? static_cast<CSPetSystem*>(character->m_pPet) : nullptr;
    }

    void ClearRightMouseInputState()
    {
        MouseRButtonPop = false;
        MouseRButtonPush = false;
        MouseRButton = false;
        MouseRButtonPress = 0;
    }
}

namespace giPetManager
{
    static std::uint8_t g_tabBar = 0;
    static std::uint32_t g_renderItemIndexBackup = 0;
    static ITEM g_renderItemInfoBackup {};

    void InitPetManager(void)
    {
        g_tabBar = 0;
        gs_PetInfo.m_dwPetType = PET_TYPE_NONE;
    }

    void CreatePetDarkSpirit(CHARACTER* c)
    {
        DeletePet(c);

        if (gMapManager.InChaosCastle())
        {
            return;
        }

        c->m_pPet = new CSPetDarkSpirit(c);
    }

    void CreatePetDarkSpirit_Now(CHARACTER* c)
    {
        if (c->Weapon[1].Type == MODEL_DARK_RAVEN_ITEM)
        {
            DeletePet(c);
            c->m_pPet = new CSPetDarkSpirit(c);
        }
    }

    void MovePet(CHARACTER* c)
    {
        if (auto* petSystem = ResolvePetSystem(c))
        {
            petSystem->MovePet();
        }
    }

    void RenderPet(CHARACTER* c)
    {
        OBJECT* o = &c->Object;
        if (auto* petSystem = ResolvePetSystem(c))
        {
            if (g_isCharacterBuff(o, eBuff_Cloaking))
            {
                petSystem->RenderPet(10);
            }
            else
            {
                ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
                PET_INFO* pPetInfo = giPetManager::GetPetInfo(pEquipmentItemSlot);
                petSystem->SetPetInfo(pPetInfo);
                petSystem->RenderPet();
            }
        }
    }

    bool SelectPetCommand(void)
    {
        if (gCharacterManager.GetBaseClass(Hero->Class) != CLASS_DARK_LORD)
        {
            return false;
        }

        if (!IsVirtualKeyPressed(kShiftKeyCode))
        {
            return false;
        }

        for (int commandOffset = 0; commandOffset < kPetCommandCount; ++commandOffset)
        {
            const int commandVirtualKey = '1' + commandOffset;
            if (IsVirtualKeyPressed(commandVirtualKey))
            {
                Hero->CurrentSkill = AT_PET_COMMAND_DEFAULT + commandOffset;
                return true;
            }
        }

        return false;
    }
    void MovePetCommand(CHARACTER* c)
    {
        if (!ResolvePetSystem(c))
        {
            return;
        }

        constexpr int kCommandIconWidth = 32;
        constexpr int kCommandIconHeight = 36;
        constexpr int kCommandBarY = 330;

        int skillCount = 0;
        for (int command = AT_PET_COMMAND_DEFAULT; command < AT_PET_COMMAND_END; ++command)
        {
            const int commandBarWidth = (AT_PET_COMMAND_END - AT_PET_COMMAND_DEFAULT) * kCommandIconWidth;
            const int x = 320 - commandBarWidth / 2 + skillCount * kCommandIconWidth;
            const int y = kCommandBarY;
            ++skillCount;

            if (MouseX >= x && MouseX < x + kCommandIconWidth && MouseY >= y && MouseY < y + kCommandIconHeight)
            {
                CheckSkill = command;
                CheckX = x + kCommandIconWidth / 2;
                CheckY = y;
                MouseOnWindow = true;
                if (MouseLButtonPush)
                {
                    MouseLButtonPush = false;
                    Hero->CurrentSkill = command;
                    SkillEnable = false;
                    PlayBuffer(SOUND_CLICK01);
                    MouseUpdateTime = 0;
                    MouseUpdateTimeMax = 6;
                }
            }
        }
    }

    bool SendPetCommand(CHARACTER* c, int Index)
    {
        auto* petSystem = ResolvePetSystem(c);
        if (petSystem == nullptr)
        {
            return false;
        }

        if (!(MouseRButtonPush || MouseRButton))
        {
            return false;
        }

        if (Index < AT_PET_COMMAND_DEFAULT || Index >= AT_PET_COMMAND_END)
        {
            return false;
        }

        const auto petCommand = Index - AT_PET_COMMAND_DEFAULT;
        if (Index == AT_PET_COMMAND_TARGET)
        {
            if (CheckAttack() && SelectedCharacter != -1)
            {
                CHARACTER* targetCharacter = &CharactersClient[SelectedCharacter];
                if (targetCharacter->Object.Kind == KIND_MONSTER || targetCharacter->Object.Kind == KIND_PLAYER)
                {
                    SocketClient->ToGameServer()->SendPetCommandRequest(petSystem->GetPetType(), petCommand, targetCharacter->Key);
                }
            }
        }
        else
        {
            SocketClient->ToGameServer()->SendPetCommandRequest(petSystem->GetPetType(), petCommand, kInvalidTargetKey);
        }

        ClearRightMouseInputState();
        return true;
    }

    void SetPetCommand(CHARACTER* c, int Key, std::uint8_t Cmd)
    {
        if (auto* petSystem = ResolvePetSystem(c))
        {
            petSystem->SetCommand(Key, Cmd);
        }
    }

    void SetAttack(CHARACTER* c, int Key, int attackType)
    {
        if (auto* petSystem = ResolvePetSystem(c))
        {
            petSystem->SetAttack(Key, attackType);
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
            mu_swprintf(TextList[TextNum], GlobalText[1219 + cmdType]); TextNum++; SkipNum++;

            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
            mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
            switch (cmdType)
            {
            case PET_CMD_DEFAULT: mu_swprintf(TextList[TextNum], GlobalText[1223]); TextNum++; SkipNum++; break;
            case PET_CMD_RANDOM: mu_swprintf(TextList[TextNum], GlobalText[1224]); TextNum++; SkipNum++; break;
            case PET_CMD_OWNER: mu_swprintf(TextList[TextNum], GlobalText[1225]); TextNum++; SkipNum++; break;
            case PET_CMD_TARGET: mu_swprintf(TextList[TextNum], GlobalText[1226]); TextNum++; SkipNum++; break;
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
        if (auto* petSystem = ResolvePetSystem(c))
        {
            const int objectType = petSystem->GetObjectType();
            TerminateOwnerEffectObject(objectType);
            delete petSystem;
            c->m_pPet = nullptr;
        }
    }

    void InitItemBackup(void)
    {
        g_renderItemInfoBackup = ITEM {};
        gs_PetInfo.m_dwPetType = PET_TYPE_NONE;
    }

    bool RequestPetInfo(int sx, int sy, ITEM* pItem)
    {
        const auto itemIndex = ComposeItemIndex(sx, sy);
        if (gs_PetInfo.m_dwPetType == PET_TYPE_NONE || g_renderItemIndexBackup != itemIndex
            || g_renderItemInfoBackup.Type != pItem->Type || g_renderItemInfoBackup.Level != pItem->Level)
        {
            g_renderItemIndexBackup = itemIndex;
            g_renderItemInfoBackup.Type = pItem->Type;
            g_renderItemInfoBackup.Level = pItem->Level;

            std::uint8_t PetType = PET_TYPE_DARK_SPIRIT;
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

    void SetPetInfo(std::uint8_t InvType, std::uint8_t InvPos, PET_INFO* pPetinfo)
    {
        CalcPetInfo(pPetinfo);

        if ((InvType == 0) || (InvType == 254) || (InvType == 255))
        {
            if ((InvPos == EQUIPMENT_HELPER) || (InvPos == EQUIPMENT_WEAPON_LEFT))
            {
                PET_INFO* pHeroPetInfo = Hero->GetEquipedPetInfo(pPetinfo->m_dwPetType);
                std::memcpy(pHeroPetInfo, pPetinfo, sizeof(PET_INFO));

                if (pPetinfo->m_dwPetType == PET_TYPE_DARK_SPIRIT)
                {
                    if (auto* heroPetSystem = ResolvePetSystem(Hero))
                    {
                        heroPetSystem->SetPetInfo(pHeroPetInfo);

                        if (InvType == 254)
                        {
                            heroPetSystem->Eff_LevelUp();
                        }
                        else if (InvType == 255)
                        {
                            heroPetSystem->Eff_LevelDown();
                        }
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

        std::memcpy(&gs_PetInfo, pPetinfo, sizeof(PET_INFO));
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
                ip->SpecialValue[ip->SpecialNum] = static_cast<std::uint8_t>((5 + (CharacterAttribute->Dexterity / 20) + pPetInfo->m_wLevel * 2) & 0xFF);
                ip->Special[ip->SpecialNum] = AT_SET_OPTION_IMPROVE_DEFENCE;
                ip->SpecialNum++;
            }
            else
            {
                ip->SpecialValue[Index] = static_cast<std::uint8_t>((5 + (CharacterAttribute->Dexterity / 20) + pPetInfo->m_wLevel * 2) & 0xFF);
                ip->Special[Index] = AT_SET_OPTION_IMPROVE_DEFENCE;
            }
        }
    }

    std::uint32_t GetPetItemValue(PET_INFO* pPetInfo)
    {
        std::uint32_t gold = 0;

        if (pPetInfo->m_dwPetType == PET_TYPE_NONE)
        {
            return gold;
        }

        switch (pPetInfo->m_dwPetType)
        {
        case PET_TYPE_DARK_HORSE:
            gold = static_cast<std::uint32_t>(pPetInfo->m_wLevel) * 2000000u;
            break;

        case PET_TYPE_DARK_SPIRIT:
            gold = static_cast<std::uint32_t>(pPetInfo->m_wLevel) * 1000000u;
            break;
        }

        return gold;
    }

    bool RenderPetItemInfo(int sx, int sy, ITEM* pItem, int iInvenType)
    {
        PET_INFO* pPetInfo = GetPetInfo(pItem);

        if (pPetInfo->m_dwPetType == PET_TYPE_NONE)
        {
            return false;
        }

        int TextNum = 0;
        int SkipNum = 0;
        int RequireLevel = 0;
        int RequireCharisma = 0;
        const std::wstring priceFormat = SanitizeWideStringFormat(GlobalText[63]);
        const std::wstring ownershipFormat = SanitizeWideStringFormat(GlobalText[61]);

        auto appendLine = [&](int color, bool bold, bool countForHeight, const wchar_t* format, auto... args)
        {
            if (TextNum >= kTooltipLineLimit)
            {
                return;
            }

            TextListColor[TextNum] = color;
            TextBold[TextNum] = bold;
            std::swprintf(TextList[TextNum], kTooltipBufferCapacity, format, args...);
            ++TextNum;

            if (countForHeight)
            {
                ++SkipNum;
            }
        };

        auto appendEmptyLine = [&]() {
            appendLine(TEXT_COLOR_WHITE, false, true, L"\n");
        };

        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP))
        {
            wchar_t textBuffer[kTooltipBufferCapacity] {};
            std::uint32_t gold = GetPetItemValue(&giPetManager::gs_PetInfo) / 3u;
            gold = (gold / 100u) * 100u;

            ConvertGold(gold, textBuffer);
            appendLine(TEXT_COLOR_WHITE, true, false, priceFormat.c_str(), textBuffer);
            appendEmptyLine();
        }
        else if ((iInvenType == SEASON3B::TOOLTIP_TYPE_MY_SHOP) || (iInvenType == SEASON3B::TOOLTIP_TYPE_PURCHASE_SHOP))
        {
            int price = 0;
            const int indexInv = g_pMyShopInventory->GetInventoryCtrl()->GetIndexByItem(pItem);
            wchar_t textBuffer[kTooltipBufferCapacity] {};

            if (GetPersonalItemPrice(indexInv, price, g_IsPurchaseShop))
            {
                ConvertGold(price, textBuffer);

                int priceColor = TEXT_COLOR_WHITE;
                if (price >= 10000000)
                {
                    priceColor = TEXT_COLOR_RED;
                }
                else if (price >= 1000000)
                {
                    priceColor = TEXT_COLOR_YELLOW;
                }
                else if (price >= 100000)
                {
                    priceColor = TEXT_COLOR_GREEN;
                }

                appendLine(priceColor, true, false, priceFormat.c_str(), textBuffer);
                appendEmptyLine();

                const auto heroGold = CharacterMachine->Gold;
                if ((static_cast<std::int64_t>(heroGold) < static_cast<std::int64_t>(price)) && (g_IsPurchaseShop == PSHOPWNDTYPE_PURCHASE))
                {
                    appendLine(TEXT_COLOR_RED, true, false, GlobalText[423]);
                    appendEmptyLine();
                }
            }
            else if (g_IsPurchaseShop == PSHOPWNDTYPE_SALE)
            {
                appendLine(TEXT_COLOR_RED, true, false, GlobalText[1101]);
                appendEmptyLine();
            }
        }

        if (pItem->Type == ITEM_DARK_HORSE_ITEM)
        {
            RequireLevel = (218 + (pPetInfo->m_wLevel * 2));
            appendLine(TEXT_COLOR_BLUE, true, true, GlobalText[1187]);
        }
        else if (pItem->Type == ITEM_DARK_RAVEN_ITEM)
        {
            RequireCharisma = (185 + (pPetInfo->m_wLevel * 15));
            appendLine(TEXT_COLOR_BLUE, true, true, GlobalText[1214]);
        }

        appendEmptyLine();
        appendEmptyLine();

        appendLine(TEXT_COLOR_WHITE, false, true, GlobalText[201], pPetInfo->m_dwExp1, pPetInfo->m_dwExp2);
        appendLine(TEXT_COLOR_WHITE, false, true, L"%ls : %d", GlobalText[368], pPetInfo->m_wLevel);

        if (pItem->Type == ITEM_DARK_RAVEN_ITEM)
        {
            appendLine(TEXT_COLOR_WHITE, false, true, GlobalText[203], pPetInfo->m_wDamageMin, pPetInfo->m_wDamageMax, pPetInfo->m_wAttackSuccess);
            appendLine(TEXT_COLOR_WHITE, false, true, GlobalText[64], pPetInfo->m_wAttackSpeed);
        }
        appendLine(TEXT_COLOR_WHITE, false, true, GlobalText[70], pPetInfo->m_wLife);

        if (pItem->Type == ITEM_DARK_HORSE_ITEM)
        {
            const bool hasLevelRequirement = CharacterAttribute->Level >= RequireLevel;
            const int requirementColor = hasLevelRequirement ? TEXT_COLOR_WHITE : TEXT_COLOR_RED;
            appendLine(requirementColor, false, false, GlobalText[76], RequireLevel);

            if (!hasLevelRequirement)
            {
                appendLine(TEXT_COLOR_RED, false, false, GlobalText[74], RequireLevel - CharacterAttribute->Level);
            }
        }
        else if (pItem->Type == ITEM_DARK_RAVEN_ITEM)
        {
            const int charismaTotal = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
            const bool hasCharisma = charismaTotal >= RequireCharisma;
            const int charismaColor = hasCharisma ? TEXT_COLOR_WHITE : TEXT_COLOR_RED;

            appendLine(charismaColor, false, false, GlobalText[698], RequireCharisma);

            if (!hasCharisma)
            {
                appendLine(TEXT_COLOR_RED, false, false, GlobalText[74], RequireCharisma - charismaTotal);
            }
        }

        appendEmptyLine();

        const int ownershipColor = (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD) ? TEXT_COLOR_WHITE : TEXT_COLOR_DARKRED;
        appendLine(ownershipColor, false, true, ownershipFormat.c_str(), GlobalText[24]);

        for (int i = 0; i < pItem->SpecialNum; ++i)
        {
            SetPetItemConvert(pItem, &gs_PetInfo);
            GetSpecialOptionText(pItem->Type, TextList[TextNum], pItem->Special[i], pItem->SpecialValue[i], 0);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            ++TextNum;
            ++SkipNum;

            if (TextNum >= kTooltipLineLimit)
            {
                break;
            }
        }

        if (pItem->Type == ITEM_DARK_HORSE_ITEM)
        {
            appendLine(TEXT_COLOR_BLUE, false, true, GlobalText[744], (30 + pPetInfo->m_wLevel) / 2);
            appendLine(TEXT_COLOR_BLUE, false, false, GlobalText[1188], 2);
        }

        SIZE TextSize = { 0, 0 };
        GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);
        int Height = static_cast<int>(((TextNum - SkipNum) * TextSize.cy + SkipNum * TextSize.cy / 2) / g_fScreenRate_y);
        if (sy - Height >= 0)
        {
            sy -= Height;
        }

        RenderTipTextList(sx, sy, TextNum, 0);
        return true;
    }
}