#include "stdafx.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Textures/ZzzTexture.h"
#include "UI/Core/UIManager.h"
#include "UIJewelHarmony.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Core/Utilities/StringUtils.h"

JewelHarmonyInfo* JewelHarmonyInfo::MakeInfo()
{
    auto* info = new JewelHarmonyInfo;
    return info;
}

JewelHarmonyInfo::JewelHarmonyInfo()
{
    const std::wstring filename =
        L"Data\\Local\\" + g_strSelectedML + L"\\JewelOfHarmonyOption_" + g_strSelectedML + L".bmd";
    if (OpenJewelHarmonyInfoFile(filename))
        return;

    mu::log::Get("ui")->error("{} file not found.", StringUtils::WideToNarrow(filename.c_str()));
    const std::wstring message = filename + L" file not found.\r\n";
    ::MessageBox(g_hWnd, message.c_str(), NULL, MB_OK);
    ::PostMessage(g_hWnd, WM_DESTROY, 0, 0);
}

JewelHarmonyInfo::~JewelHarmonyInfo() {}

const bool JewelHarmonyInfo::OpenJewelHarmonyInfoFile(const std::wstring& filename)
{
    constexpr size_t NAME_SIZE = 60;
    constexpr size_t LEVEL_COUNT = 14;
    constexpr size_t ENTRY_SIZE = sizeof(int) +               // OptionType
                                  NAME_SIZE +                 // Name
                                  sizeof(int) +               // Minlevel
                                  LEVEL_COUNT * sizeof(int) + // HarmonyJewelLevel
                                  LEVEL_COUNT * sizeof(int);  // Zen

    FILE* fp = ::_wfopen(filename.c_str(), L"rb");
    if (fp != NULL)
    {
        int nEntries = MAXHARMONYJEWELOPTIONTYPE * MAXHARMONYJEWELOPTIONINDEX;
        size_t nSize = ENTRY_SIZE * nEntries;

        std::vector<BYTE> tempBuffer(nSize);

        ::fread(tempBuffer.data(), nSize, 1, fp);
        ::BuxConvert((BYTE*)tempBuffer.data(), nSize);
        ::fclose(fp);

        // Copy to the new HarmonyJewelOption array that uses wchar_t[] for the Name
        for (int i = 0; i < nEntries; ++i)
        {
            int type = i / MAXHARMONYJEWELOPTIONINDEX;
            int option = i % MAXHARMONYJEWELOPTIONINDEX;

            // Calculate base offset for the current entry
            BYTE* entry = tempBuffer.data() + i * ENTRY_SIZE;

            // Read OptionType
            m_OptionData[type][option].OptionType = *reinterpret_cast<int*>(entry);

            // Read Name and convert to wchar_t
            char* name = reinterpret_cast<char*>(entry + sizeof(int));
            CMultiLanguage::GetSingletonPtr()->ConvertFromUtf8(m_OptionData[type][option].Name, name, NAME_SIZE);

            // Read Minlevel
            m_OptionData[type][option].Minlevel = *reinterpret_cast<int*>(entry + sizeof(int) + NAME_SIZE);

            // Read HarmonyJewelLevel[14]
            memcpy(m_OptionData[type][option].HarmonyJewelLevel, entry + sizeof(int) + NAME_SIZE + sizeof(int),
                   LEVEL_COUNT * sizeof(int));

            // Read Zen[14]
            memcpy(m_OptionData[type][option].Zen,
                   entry + sizeof(int) + NAME_SIZE + sizeof(int) + LEVEL_COUNT * sizeof(int),
                   LEVEL_COUNT * sizeof(int));
        }

        return true;
    }
    return false;
}

const StrengthenItem JewelHarmonyInfo::GetItemType(int type)
{
    StrengthenItem itemtype = SI_None;

    if (ITEM_SWORD <= type && ITEM_STAFF > type)
    {
        itemtype = SI_Weapon;
        return itemtype;
    }
    else if (ITEM_STAFF <= type && ITEM_SHIELD > type)
    {
        itemtype = SI_Staff;
        return itemtype;
    }
    else if (ITEM_SHIELD <= type && ITEM_WING > type)
    {
        itemtype = SI_Defense;
        return itemtype;
    }

    return itemtype;
}

const bool JewelHarmonyInfo::IsHarmonyJewelOption(int type, int option)
{
    bool isSuccess = false;

    switch (type)
    {
    case SI_Weapon:
    {
        if (0 <= option && (MAXHARMONYJEWELOPTIONINDEX + 1) > option)
        {
            isSuccess = true;
        }
    }
    break;
    case SI_Staff:
    {
        if (0 <= option && (MAXHARMONYJEWELOPTIONINDEX + 1) - 2 > option)
        {
            isSuccess = true;
        }
    }
    break;
    case SI_Defense:
    {
        if (0 <= option && (MAXHARMONYJEWELOPTIONINDEX + 1) - 2 > option)
        {
            isSuccess = true;
        }
    }
    break;
    }

    return isSuccess;
}

void JewelHarmonyInfo::GetStrengthenCapability(StrengthenCapability* pitemSC, const ITEM* pitem, const int index)
{
    if (pitem->Jewel_Of_Harmony_Option != 0)
    {
        StrengthenItem type = GetItemType(static_cast<int>(pitem->Type));

        if (type < SI_None)
        {
            if (IsHarmonyJewelOption(type, pitem->Jewel_Of_Harmony_Option))
            {
                HARMONYJEWELOPTION harmonyjewel = GetHarmonyJewelOptionInfo(type, pitem->Jewel_Of_Harmony_Option);

                if (index == 0)
                {
                    if (type == SI_Weapon)
                    {
                        pitemSC->SI_isNB = true;

                        if (pitem->Jewel_Of_Harmony_Option == 3)
                        {
                            pitemSC->SI_NB.SI_force =
                                harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 4)
                        {
                            pitemSC->SI_NB.SI_activity =
                                harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                    }
                    else if (type == SI_Staff)
                    {
                        pitemSC->SI_isNB = true;

                        if (pitem->Jewel_Of_Harmony_Option == 2)
                        {
                            pitemSC->SI_NB.SI_force =
                                harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 3)
                        {
                            pitemSC->SI_NB.SI_activity =
                                harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                    }
                }
                else if (index == 1)
                {
                    if (type == SI_Weapon)
                    {
                        pitemSC->SI_isSP = true;

                        if (pitem->Jewel_Of_Harmony_Option == 1)
                        {
                            pitemSC->SI_SP.SI_minattackpower =
                                harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 2)
                        {
                            pitemSC->SI_SP.SI_maxattackpower =
                                harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 5)
                        {
                            pitemSC->SI_SP.SI_minattackpower =
                                harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                            pitemSC->SI_SP.SI_maxattackpower =
                                harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 7)
                        {
                            pitemSC->SI_SP.SI_skillattackpower =
                                harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 8)
                        {
                            pitemSC->SI_SP.SI_attackpowerRate =
                                harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                    }
                    else if (type == SI_Staff)
                    {
                        pitemSC->SI_isSP = true;

                        if (pitem->Jewel_Of_Harmony_Option == 1)
                        {
                            pitemSC->SI_SP.SI_magicalpower =
                                harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 4)
                        {
                            pitemSC->SI_SP.SI_skillattackpower =
                                harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 7)
                        {
                            pitemSC->SI_SP.SI_attackpowerRate =
                                harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                    }
                }

                else if (index == 2)
                {
                    if (SI_Defense == type)
                    {
                        pitemSC->SI_isSD = true;

                        if (pitem->Jewel_Of_Harmony_Option == 1)
                        {
                            pitemSC->SI_SD.SI_defense =
                                harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 2)
                        {
                            pitemSC->SI_SD.SI_AG = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 3)
                        {
                            pitemSC->SI_SD.SI_HP = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 6)
                        {
                            pitemSC->SI_SD.SI_defenseRate =
                                harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                    }
                }
            }
        }
    }
}
