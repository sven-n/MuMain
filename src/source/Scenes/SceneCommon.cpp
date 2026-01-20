///////////////////////////////////////////////////////////////////////////////
// SceneCommon.cpp - Shared utilities used by multiple scenes
// Extracted from ZzzScene.cpp as part of scene refactoring
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SceneCommon.h"
#include "../ZzzScene.h"

//=============================================================================
// Character Selection State Implementation
//=============================================================================

// Global instance
CharacterSelectionState g_characterSelection;

// Legacy global reference (for backward compatibility)
int& SelectedHero = g_characterSelection.GetLegacyReference();

//=============================================================================
// Scene Common Utilities
//=============================================================================
#include "../ZzzInterface.h"
#include "../ZzzInventory.h"
#include "../ZzzTexture.h"
#include "../ZzzOpenglUtil.h"
#include "../ZzzObject.h"
#include "../ZzzInfomation.h"
#include "../ZzzOpenData.h"
#include "../UIManager.h"
#include "../DSPlaySound.h"
#include "../Local.h"
#include "../GlobalText.h"
#include "../PersonalShopTitleImp.h"
#include "../CComGem.h"
#include "../UIMng.h"

// External variable declarations (defined in ZzzScene.cpp or other files)
extern wchar_t AbuseFilter[][20];
extern wchar_t AbuseNameFilter[][20];
extern int AbuseFilterNumber;
extern int AbuseNameFilterNumber;
extern wchar_t InputText[12][256];
extern int InputGold;
extern int AskYesOrNo;
extern ITEM PickItem;
extern ITEM TargetItem;
extern int ErrorMessage;
extern bool MouseLButtonPush;
extern int MouseUpdateTime;
extern int MouseUpdateTimeMax;
extern int MouseWheel;
extern short g_shCameraLevel;
extern SKILL_ATTRIBUTE* SkillAttribute;
extern bool g_bEnterPressed;

// External variables from ZzzScene.cpp
extern int g_iMessageTextStart;
extern char g_cMessageTextCurrNum;
extern char g_cMessageTextNum;
extern int g_iNumLineMessageBoxCustom;
extern wchar_t g_lpszMessageBoxCustom[NUM_LINE_CMB][MAX_LENGTH_CMB];
extern int g_iCustomMessageBoxButton[NUM_BUTTON_CMB][NUM_PAR_BUTTON_CMB];
extern int g_iCustomMessageBoxButton_Cancel[NUM_PAR_BUTTON_CMB];
extern int g_iCancelSkillTarget;
extern bool g_bTimeCheck;
extern int g_iBackupTime;
extern EGameScene SceneFlag;

///////////////////////////////////////////////////////////////////////////////
// Abuse Filter Functions
///////////////////////////////////////////////////////////////////////////////

bool CheckAbuseFilter(wchar_t* Text, bool bCheckSlash)
{
    if (bCheckSlash == true)
    {
        if (Text[0] == '/')
        {
            return false;
        }
    }

    int icntText = 0;
    wchar_t TmpText[2048];
    int textLength = wcslen(Text);
    for (int i = 0; i < textLength; ++i)
    {
        if (Text[i] != 32)
        {
            TmpText[icntText] = Text[i];
            icntText++;
        }
    }
    TmpText[icntText] = 0;

    for (int i = 0; i < AbuseFilterNumber; i++)
    {
        if (FindText(TmpText, AbuseFilter[i]))
        {
            return true;
        }
    }
    return false;
}

bool CheckAbuseNameFilter(wchar_t* Text)
{
    int icntText = 0;
    wchar_t TmpText[256];
    int textLength = wcslen(Text);
    for (int i = 0; i < textLength; ++i)
    {
        if (Text[i] != 32)
        {
            TmpText[icntText] = Text[i];
            icntText++;
        }
    }
    TmpText[icntText] = 0;

    for (int i = 0; i < AbuseNameFilterNumber; i++)
    {
        if (FindText(TmpText, AbuseNameFilter[i]))
        {
            return true;
        }
    }
    return false;
}

bool CheckName()
{
    if (CheckAbuseNameFilter(InputText[0]) || CheckAbuseFilter(InputText[0]) ||
        FindText(InputText[0], L" ") || FindText(InputText[0], L"　") ||
        FindText(InputText[0], L".") || FindText(InputText[0], L"·") || FindText(InputText[0], L"∼") ||
        FindText(InputText[0], L"Webzen") || FindText(InputText[0], L"WebZen") || FindText(InputText[0], L"webzen") || FindText(InputText[0], L"WEBZEN") ||
        FindText(InputText[0], GlobalText[457]) || FindText(InputText[0], GlobalText[458]))
        return true;
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// UI Utility Functions
///////////////////////////////////////////////////////////////////////////////

BOOL CheckOptionMouseClick(int iOptionPos_y, BOOL bPlayClickSound)
{
    if (CheckMouseIn((640 - 120) / 2, 30 + iOptionPos_y, 120, 22) && MouseLButtonPush)
    {
        MouseLButtonPush = false;
        MouseUpdateTime = 0;
        MouseUpdateTimeMax = 6;
        if (bPlayClickSound == TRUE) PlayBuffer(SOUND_CLICK01);
        return TRUE;
    }
    return FALSE;
}

int SeparateTextIntoLines(const wchar_t* lpszText, wchar_t* lpszSeparated, int iMaxLine, int iLineSize)
{
    int iLine = 0;
    const wchar_t* lpLineStart = lpszText;
    wchar_t* lpDst = lpszSeparated;
    const wchar_t* lpSpace = NULL;
    int iMbclen = 0;
    for (const wchar_t* lpSeek = lpszText; *lpSeek; lpSeek += iMbclen, lpDst += iMbclen)
    {
        // For wide characters, always 1 character per unit
        iMbclen = 1;
        if (iMbclen + (lpSeek - lpLineStart) >= iLineSize)
        {
            if (lpSpace && (lpSeek - lpSpace) < std::min<int>(10, iLineSize / 2))
            {
                lpDst -= lpSeek - lpSpace - 1;
                lpSeek = lpSpace + 1;
            }

            lpLineStart = lpSeek;
            *lpDst = L'\0';
            if (iLine >= iMaxLine - 1)
            {
                break;
            }
            ++iLine;
            lpDst = lpszSeparated + iLine * iLineSize;
            lpSpace = NULL;
        }

        memcpy(lpDst, lpSeek, iMbclen);
        if (*lpSeek == L' ')
        {
            lpSpace = lpSeek;
        }
    }
    *lpDst = L'\0';

    return (iLine + 1);
}

///////////////////////////////////////////////////////////////////////////////
// Audio and Viewport Functions
///////////////////////////////////////////////////////////////////////////////

void SetEffectVolumeLevel(int level)
{
    if (level > 9)
        level = 9;
    if (level < 0)
        level = 0;

    if (level == 0)
    {
        SetMasterVolume(-10000);
    }
    else
    {
        long vol = -2000 * log10(10.f / float(level));
        SetMasterVolume(vol);
    }
}

void SetViewPortLevel(int Wheel)
{
    if ((HIBYTE(GetAsyncKeyState(VK_CONTROL)) == 128))
    {
        if (Wheel > 0)
            g_shCameraLevel--;
        else if (Wheel < 0)
            g_shCameraLevel++;

        MouseWheel = 0;

        if (g_shCameraLevel > 4)
            g_shCameraLevel = 4;
        if (g_shCameraLevel < 0)
            g_shCameraLevel = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Rendering Functions
///////////////////////////////////////////////////////////////////////////////

void RenderInfomation3D()
{
    bool Success = false;

    if (((ErrorMessage == MESSAGE_TRADE_CHECK || ErrorMessage == MESSAGE_CHECK) && AskYesOrNo == 1)
        || ErrorMessage == MESSAGE_USE_STATE
        || ErrorMessage == MESSAGE_USE_STATE2)
    {
        Success = true;
    }

    if (ErrorMessage == MESSAGE_TRADE_CHECK && AskYesOrNo == 5)
    {
        Success = true;
    }
    if (ErrorMessage == MESSAGE_PERSONALSHOP_WARNING)
    {
        Success = true;
    }

    if (Success)
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glViewport2(0, 0, WindowWidth, WindowHeight);
        gluPerspective2(1.f, (float)(WindowWidth) / (float)(WindowHeight), CameraViewNear, CameraViewFar);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        GetOpenGLMatrix(CameraMatrix);
        EnableDepthTest();
        EnableDepthMask();

        float Width, Height;
        float x = (640 - 150) / 2;
        float y;
        if (ErrorMessage == MESSAGE_TRADE_CHECK)
        {
            y = 60 + 55;
        }
        else
        {
            y = 60 + 55;
        }

        Width = 40.f; Height = 60.f;
        int iRenderType = ErrorMessage;
        if (AskYesOrNo == 5)
            iRenderType = MESSAGE_USE_STATE;
        switch (iRenderType)
        {
        case MESSAGE_USE_STATE:
        case MESSAGE_USE_STATE2:
        case MESSAGE_PERSONALSHOP_WARNING:
            RenderItem3D(x, y, Width, Height, TargetItem.Type, TargetItem.Level, TargetItem.ExcellentFlags, TargetItem.AncientDiscriminator, true);
            break;

        default:
            RenderItem3D(x, y, Width, Height, PickItem.Type, PickItem.Level, PickItem.ExcellentFlags, PickItem.AncientDiscriminator, true);
            break;
        }

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        UpdateMousePositionn();
    }
}

void RenderInfomation()
{
    RenderNotices();

    CUIMng::Instance().Render();

    if (SceneFlag == LOG_IN_SCENE || SceneFlag == CHARACTER_SCENE)
    {
        RenderCursor();
    }

    RenderInfomation3D();
}

BOOL ShowCheckBox(int num, int index, int message)
{
    if (message == MESSAGE_USE_STATE || message == MESSAGE_USE_STATE2)
    {
        wchar_t Name[50] = { 0, };
        if (TargetItem.Type == ITEM_FRUITS)
        {
            switch (TargetItem.Level)
            {
            case 0:swprintf_s(Name, 50, L"%ls", GlobalText[168]); break;
            case 1:swprintf_s(Name, 50, L"%ls", GlobalText[169]); break;
            case 2:swprintf_s(Name, 50, L"%ls", GlobalText[167]); break;
            case 3:swprintf_s(Name, 50, L"%ls", GlobalText[166]); break;
            case 4:swprintf_s(Name, 50, L"%ls", GlobalText[1900]); break;
            }
        }

        if (message == MESSAGE_USE_STATE2)
            swprintf_s(g_lpszMessageBoxCustom[0], MAX_LENGTH_CMB, L"( %ls%ls )", Name, GlobalText[1901]);
        else
            swprintf_s(g_lpszMessageBoxCustom[0], MAX_LENGTH_CMB, L"( %ls )", Name);

        num++;
        for (int i = 1; i < num; ++i)
        {
            swprintf_s(g_lpszMessageBoxCustom[i], MAX_LENGTH_CMB, GlobalText[index]);
        }
        g_iNumLineMessageBoxCustom = num;
    }
    else if (message == MESSAGE_PERSONALSHOP_WARNING)
    {
        wchar_t szGold[256];
        ConvertGold(InputGold, szGold);
        swprintf_s(g_lpszMessageBoxCustom[0], MAX_LENGTH_CMB, GlobalText[index], szGold);

        for (int i = 1; i < num; ++i)
        {
            swprintf_s(g_lpszMessageBoxCustom[i], MAX_LENGTH_CMB, GlobalText[index + i]);
        }
        g_iNumLineMessageBoxCustom = num;
    }
    else if (message == MESSAGE_CHAOS_CASTLE_CHECK)
    {
        g_iNumLineMessageBoxCustom = 0;
        for (int i = 0; i < num; ++i)
        {
            g_iNumLineMessageBoxCustom += SeparateTextIntoLines(GlobalText[index + i], g_lpszMessageBoxCustom[g_iNumLineMessageBoxCustom], NUM_LINE_CMB, MAX_LENGTH_CMB);
        }
    }
    else if (message == MESSAGE_GEM_INTEGRATION3)
    {
        wchar_t tBuf[MAX_GLOBAL_TEXT_STRING];
        wchar_t tLines[2][30];
        for (int t = 0; t < 2; ++t) memset(tLines[t], 0, 20);
        g_iNumLineMessageBoxCustom = 0;
        if (COMGEM::isComMode())
        {
            if (COMGEM::m_cGemType == 0) swprintf_s(tBuf, MAX_GLOBAL_TEXT_STRING, GlobalText[1809], GlobalText[1806], COMGEM::m_cCount);
            else swprintf_s(tBuf, MAX_GLOBAL_TEXT_STRING, GlobalText[1809], GlobalText[1807], COMGEM::m_cCount);

            g_iNumLineMessageBoxCustom += SeparateTextIntoLines(tBuf,
                tLines[g_iNumLineMessageBoxCustom], 2, 30);

            for (int t = 0; t < 2; ++t)
                wcscpy_s(g_lpszMessageBoxCustom[t], MAX_LENGTH_CMB, tLines[t]);

            swprintf_s(g_lpszMessageBoxCustom[g_iNumLineMessageBoxCustom], MAX_LENGTH_CMB, GlobalText[1810], COMGEM::m_iValue);
            ++g_iNumLineMessageBoxCustom;
        }
        else
        {
            int t_GemLevel = COMGEM::GetUnMixGemLevel() + 1;
            if (COMGEM::m_cGemType == 0) swprintf_s(tBuf, MAX_GLOBAL_TEXT_STRING, GlobalText[1813], GlobalText[1806], t_GemLevel);
            else swprintf_s(tBuf, MAX_GLOBAL_TEXT_STRING, GlobalText[1813], GlobalText[1807], t_GemLevel);

            g_iNumLineMessageBoxCustom += SeparateTextIntoLines(tBuf,
                tLines[g_iNumLineMessageBoxCustom], 2, 30);

            for (int t = 0; t < 2; ++t)
                wcscpy_s(g_lpszMessageBoxCustom[t], MAX_LENGTH_CMB, tLines[t]);

            swprintf_s(g_lpszMessageBoxCustom[g_iNumLineMessageBoxCustom], MAX_LENGTH_CMB, GlobalText[1814], COMGEM::m_iValue);
            ++g_iNumLineMessageBoxCustom;
        }
    }
    else if (message == MESSAGE_CANCEL_SKILL)
    {
        wchar_t tBuf[MAX_GLOBAL_TEXT_STRING];
        swprintf_s(tBuf, MAX_GLOBAL_TEXT_STRING, L"%ls%ls", SkillAttribute[index].Name, GlobalText[2046]);
        g_iNumLineMessageBoxCustom = SeparateTextIntoLines(tBuf, g_lpszMessageBoxCustom[0], 2, MAX_LENGTH_CMB);
        g_iCancelSkillTarget = index;
    }
    else
    {
        for (int i = 0; i < num; ++i)
        {
            wcscpy_s(g_lpszMessageBoxCustom[i], MAX_LENGTH_CMB, GlobalText[index + i]);
        }

        g_iNumLineMessageBoxCustom = num;
    }

    ZeroMemory(g_iCustomMessageBoxButton, NUM_BUTTON_CMB * NUM_PAR_BUTTON_CMB * sizeof(int));

    int iOkButton[5] = { 1,  21, 90, 70, 21 };
    int iCancelButton[5] = { 3, 120, 90, 70, 21 };

    if (message == MESSAGE_USE_STATE2)
    {
        iOkButton[1] = 22;
        iOkButton[2] = 92;	// y
        iOkButton[3] = 49;
        iOkButton[4] = 16;

        iCancelButton[1] = 82;
        iCancelButton[2] = 92;	// y
        iCancelButton[3] = 49;
        iCancelButton[4] = 16;

        g_iCustomMessageBoxButton_Cancel[0] = 5;
        g_iCustomMessageBoxButton_Cancel[1] = 142;	// x
        g_iCustomMessageBoxButton_Cancel[2] = 92;	// y
        g_iCustomMessageBoxButton_Cancel[3] = 49;	// width
        g_iCustomMessageBoxButton_Cancel[4] = 16;	// height
    }

    if (message == MESSAGE_CHAOS_CASTLE_CHECK)
    {
        iOkButton[2] = 120;
        iCancelButton[2] = 120;
    }

    memcpy(g_iCustomMessageBoxButton[0], iOkButton, 5 * sizeof(int));
    memcpy(g_iCustomMessageBoxButton[1], iCancelButton, 5 * sizeof(int));

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Time Utility Functions
///////////////////////////////////////////////////////////////////////////////

bool GetTimeCheck(int DelayTime)
{
    int PresentTime = timeGetTime();

    if (g_bTimeCheck)
    {
        g_iBackupTime = PresentTime;
        g_bTimeCheck = false;
    }

    if (g_iBackupTime + DelayTime <= PresentTime)
    {
        g_bTimeCheck = true;
        return true;
    }
    return false;
}

bool IsEnterPressed() {
    return g_bEnterPressed;
}

void SetEnterPressed(bool enterpressed) {
    g_bEnterPressed = enterpressed;
}
