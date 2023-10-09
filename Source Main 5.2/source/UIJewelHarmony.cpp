// UIRefinery.cpp: implementation of the UIRefinery class.
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "UIManager.h"
#include "UIJewelHarmony.h"


#define HARMONYJEWELOPTION_DATA_FILE std::wstring(L"Data\\Local\\"+g_strSelectedML+L"\\JewelOfHarmonyOption_"+g_strSelectedML+L".bmd").c_str()
#define NOTSMELTING_DATA_FILE std::wstring(L"Data\\Local\\"+g_strSelectedML+"L\\JewelOfHarmonySmelt_"+g_strSelectedML+L".bmd").c_str()

extern float		g_fScreenRate_x;
extern float		g_fScreenRate_y;

namespace
{
    int GetTextLines(const wchar_t* inText, wchar_t* outText, int maxLine, int lineSize)
    {
        int iLine = 0;
        const wchar_t* lpLineStart = inText;
        wchar_t* lpDst = outText;
        const wchar_t* lpSpace = NULL;
        int iMbclen = 0;
        for (const wchar_t* lpSeek = inText; *lpSeek; lpSeek += iMbclen, lpDst += iMbclen)
        {
            iMbclen = _mbclen((unsigned char*)lpSeek);
            if (iMbclen + (int)(lpSeek - lpLineStart) >= lineSize)
            {
                if (lpSpace && (int)(lpSeek - lpSpace) < min(10, lineSize / 2))
                {
                    lpDst -= (lpSeek - lpSpace - 1);
                    lpSeek = lpSpace + 1;
                }

                lpLineStart = lpSeek;
                *lpDst = '\0';
                if (iLine >= maxLine - 1)
                {
                    break;
                }
                ++iLine;
                lpDst = outText + iLine * lineSize;
                lpSpace = NULL;
            }

            memcpy(lpDst, lpSeek, iMbclen);
            if (*lpSeek == ' ')
            {
                lpSpace = lpSeek;
            }
        }
        *lpDst = '\0';

        return (iLine + 1);
    }
}

JewelHarmonyInfo* JewelHarmonyInfo::MakeInfo()
{
    auto* info = new JewelHarmonyInfo;
    return info;
}

JewelHarmonyInfo::JewelHarmonyInfo()
{
    bool Result = true;
    if (!OpenJewelHarmonyInfoFile(HARMONYJEWELOPTION_DATA_FILE))
    {
        Result = false;
    }

    if (!Result)
    {
        wchar_t szMessage[256];
        ::swprintf(szMessage, L"%s file not found.\r\n", L"JewelOfHarmonyOption.bmd && JewelOfHarmonySmelt.bmd");
        g_ErrorReport.Write(szMessage);
        ::MessageBox(g_hWnd, szMessage, NULL, MB_OK);
        ::PostMessage(g_hWnd, WM_DESTROY, 0, 0);
    }
}

JewelHarmonyInfo::~JewelHarmonyInfo()
{
}

const bool JewelHarmonyInfo::OpenJewelHarmonyInfoFile(const std::wstring& filename)
{
    FILE* fp = ::_wfopen(filename.c_str(), L"rb");
    if (fp != NULL)
    {
        int nSize = sizeof(HARMONYJEWELOPTION) * MAXHARMONYJEWELOPTIONTYPE * MAXHARMONYJEWELOPTIONINDEX;

        ::fread(m_OptionData, nSize, 1, fp);
        ::BuxConvert((BYTE*)m_OptionData, nSize);
        ::fclose(fp);

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
                            pitemSC->SI_NB.SI_force = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 4)
                        {
                            pitemSC->SI_NB.SI_activity = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                    }
                    else if (type == SI_Staff)
                    {
                        pitemSC->SI_isNB = true;

                        if (pitem->Jewel_Of_Harmony_Option == 2)
                        {
                            pitemSC->SI_NB.SI_force = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 3)
                        {
                            pitemSC->SI_NB.SI_activity = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
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
                            pitemSC->SI_SP.SI_minattackpower = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 2)
                        {
                            pitemSC->SI_SP.SI_maxattackpower = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 5)
                        {
                            pitemSC->SI_SP.SI_minattackpower = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                            pitemSC->SI_SP.SI_maxattackpower = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 7)
                        {
                            pitemSC->SI_SP.SI_skillattackpower = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 8)
                        {
                            pitemSC->SI_SP.SI_attackpowerRate = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                    }
                    else if (type == SI_Staff)
                    {
                        pitemSC->SI_isSP = true;

                        if (pitem->Jewel_Of_Harmony_Option == 1)
                        {
                            pitemSC->SI_SP.SI_magicalpower = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 4)
                        {
                            pitemSC->SI_SP.SI_skillattackpower = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                        else if (pitem->Jewel_Of_Harmony_Option == 7)
                        {
                            pitemSC->SI_SP.SI_attackpowerRate = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
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
                            pitemSC->SI_SD.SI_defense = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
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
                            pitemSC->SI_SD.SI_defenseRate = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
                        }
                    }
                }
            }
        }
    }
}