//////////////////////////////////////////////////////////////////////
// NewUIMainFrameWindow.cpp: implementation of the CNewUIMainFrameWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUIMainFrameWindow.h"	// self
#include "NewUIOptionWindow.h"
#include "NewUISystem.h"
#include "UIBaseDef.h"
#include "DSPlaySound.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"

#include "CSItemOption.h"
#include "CSChaosCastle.h"
#include "MapManager.h"
#include "CharacterManager.h"
#include "SkillManager.h"
#include "./Time/CTimCheck.h"
#include "MonkSystem.h"

#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
#include "GameShop/InGameShopSystem.h"
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME

SEASON3B::CNewUIMainFrameWindow::CNewUIMainFrameWindow()
{
    m_bExpEffect = false;
    m_dwExpEffectTime = 0;
    m_dwPreExp = 0;
    m_dwGetExp = 0;
    m_bButtonBlink = false;
}

SEASON3B::CNewUIMainFrameWindow::~CNewUIMainFrameWindow()
{
    Release();
}

void SEASON3B::CNewUIMainFrameWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_menu01.jpg", IMAGE_MENU_1, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_menu02.jpg", IMAGE_MENU_2, GL_LINEAR);
    LoadBitmap(L"Interface\\partCharge1\\newui_menu03.jpg", IMAGE_MENU_3, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_menu02-03.jpg", IMAGE_MENU_2_1, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_menu_blue.jpg", IMAGE_GAUGE_BLUE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_menu_green.jpg", IMAGE_GAUGE_GREEN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_menu_red.jpg", IMAGE_GAUGE_RED, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_menu_ag.jpg", IMAGE_GAUGE_AG, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_menu_sd.jpg", IMAGE_GAUGE_SD, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exbar.jpg", IMAGE_GAUGE_EXBAR, GL_LINEAR);
    LoadBitmap(L"Interface\\Exbar_Master.jpg", IMAGE_MASTER_GAUGE_BAR, GL_LINEAR);
    LoadBitmap(L"Interface\\partCharge1\\newui_menu_Bt05.jpg", IMAGE_MENU_BTN_CSHOP, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\partCharge1\\newui_menu_Bt01.jpg", IMAGE_MENU_BTN_CHAINFO, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\partCharge1\\newui_menu_Bt02.jpg", IMAGE_MENU_BTN_MYINVEN, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\partCharge1\\newui_menu_Bt03.jpg", IMAGE_MENU_BTN_FRIEND, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\partCharge1\\newui_menu_Bt04.jpg", IMAGE_MENU_BTN_WINDOW, GL_LINEAR, GL_CLAMP_TO_EDGE);
}

void SEASON3B::CNewUIMainFrameWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_MENU_1);
    DeleteBitmap(IMAGE_MENU_2);
    DeleteBitmap(IMAGE_MENU_3);
    DeleteBitmap(IMAGE_MENU_2_1);
    DeleteBitmap(IMAGE_GAUGE_BLUE);
    DeleteBitmap(IMAGE_GAUGE_GREEN);
    DeleteBitmap(IMAGE_GAUGE_RED);
    DeleteBitmap(IMAGE_GAUGE_AG);
    DeleteBitmap(IMAGE_GAUGE_SD);
    DeleteBitmap(IMAGE_GAUGE_EXBAR);
    DeleteBitmap(IMAGE_MENU_BTN_CHAINFO);
    DeleteBitmap(IMAGE_MENU_BTN_MYINVEN);
    DeleteBitmap(IMAGE_MENU_BTN_FRIEND);
    DeleteBitmap(IMAGE_MENU_BTN_WINDOW);
}

bool SEASON3B::CNewUIMainFrameWindow::Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng)
{
    if (NULL == pNewUIMng || NULL == pNewUI3DRenderMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MAINFRAME, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;
    m_pNewUI3DRenderMng->Add3DRenderObj(this, ITEMHOTKEYNUMBER_CAMERA_Z_ORDER);

    LoadImages();

    SetButtonInfo();

    Show(true);

    return true;
}

void SEASON3B::CNewUIMainFrameWindow::SetButtonInfo()
{
    int x_Next = 489;
    int y_Next = 480 - 51;
    int x_Add = 30;
    int y_Add = 41;
    m_BtnCShop.ChangeTextBackColor(RGBA(255, 255, 255, 0));
    m_BtnCShop.ChangeButtonImgState(true, IMAGE_MENU_BTN_CSHOP, true);
    m_BtnCShop.ChangeButtonInfo(x_Next, y_Next, x_Add, y_Add);
    x_Next += x_Add;
    m_BtnCShop.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    m_BtnCShop.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
    m_BtnCShop.ChangeToolTipText(GlobalText[2277], true);

    m_BtnChaInfo.ChangeTextBackColor(RGBA(255, 255, 255, 0));
    m_BtnChaInfo.ChangeButtonImgState(true, IMAGE_MENU_BTN_CHAINFO, true);
    m_BtnChaInfo.ChangeButtonInfo(x_Next, y_Next, x_Add, y_Add);
    x_Next += x_Add;
    m_BtnChaInfo.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    m_BtnChaInfo.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
    m_BtnChaInfo.ChangeToolTipText(GlobalText[362], true);

    m_BtnMyInven.ChangeTextBackColor(RGBA(255, 255, 255, 0));
    m_BtnMyInven.ChangeButtonImgState(true, IMAGE_MENU_BTN_MYINVEN, true);
    m_BtnMyInven.ChangeButtonInfo(x_Next, y_Next, x_Add, y_Add);
    x_Next += x_Add;
    m_BtnMyInven.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    m_BtnMyInven.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
    m_BtnMyInven.ChangeToolTipText(GlobalText[363], true);

    m_BtnFriend.ChangeTextBackColor(RGBA(255, 255, 255, 0));
    m_BtnFriend.ChangeButtonImgState(true, IMAGE_MENU_BTN_FRIEND, true);
    m_BtnFriend.ChangeButtonInfo(x_Next, y_Next, x_Add, y_Add);
    x_Next += x_Add;
    m_BtnFriend.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    m_BtnFriend.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
    m_BtnFriend.ChangeToolTipText(GlobalText[1043], true);

    m_BtnWindow.ChangeTextBackColor(RGBA(255, 255, 255, 0));
    m_BtnWindow.ChangeButtonImgState(true, IMAGE_MENU_BTN_WINDOW, true);
    m_BtnWindow.ChangeButtonInfo(x_Next, y_Next, x_Add, y_Add);
    m_BtnWindow.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    m_BtnWindow.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
    m_BtnWindow.ChangeToolTipText(GlobalText[1744], true);
}

void SEASON3B::CNewUIMainFrameWindow::Release()
{
    UnloadImages();

    if (m_pNewUI3DRenderMng)
    {
        m_pNewUI3DRenderMng->Remove3DRenderObj(this);
        m_pNewUI3DRenderMng = NULL;
    }

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

bool SEASON3B::CNewUIMainFrameWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderFrame();

    m_pNewUI3DRenderMng->RenderUI2DEffect(ITEMHOTKEYNUMBER_CAMERA_Z_ORDER, UI2DEffectCallback, this, 0, 0);

    g_pSkillList->RenderCurrentSkillAndHotSkillList();

    EnableAlphaTest();
    RenderLifeMana();
    RenderGuageSD();
    RenderGuageAG();
    RenderButtons();
    RenderExperience();
    DisableAlphaBlend();

    return true;
}

void SEASON3B::CNewUIMainFrameWindow::Render3D()
{
    m_ItemHotKey.RenderItems();
}

void SEASON3B::CNewUIMainFrameWindow::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
    g_pMainFrame->RenderHotKeyItemCount();
}

bool SEASON3B::CNewUIMainFrameWindow::IsVisible() const
{
    return CNewUIObj::IsVisible();
}

void SEASON3B::CNewUIMainFrameWindow::RenderFrame()
{
    float width, height;
    float x, y;

    width = 256.f; height = 51.f;
    x = 0.f; y = 480.f - height;
    SEASON3B::RenderImage(IMAGE_MENU_1, x, y, width, height);
    width = 128.f;
    x = 256.f;
    SEASON3B::RenderImage(IMAGE_MENU_2, x, y, width, height);
    width = 256.f;
    x = 256.f + 128.f;
    SEASON3B::RenderImage(IMAGE_MENU_3, x, y, width, height);

    if (g_pSkillList->IsSkillListUp() == true)
    {
        width = 160.f; height = 40.f;
        x = 222.f;
        SEASON3B::RenderImage(IMAGE_MENU_2_1, x, y, width, height);
    }
}

void SEASON3B::CNewUIMainFrameWindow::RenderLifeMana()
{
    DWORD wLifeMax, wLife, wManaMax, wMana;

    if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
    {
        wLifeMax = Master_Level_Data.wMaxLife;
        wLife = min(max(0, CharacterAttribute->Life), wLifeMax);
        wManaMax = Master_Level_Data.wMaxMana;
        wMana = min(max(0, CharacterAttribute->Mana), wManaMax);
    }
    else
    {
        wLifeMax = CharacterAttribute->LifeMax;
        wLife = min(max(0, CharacterAttribute->Life), wLifeMax);
        wManaMax = CharacterAttribute->ManaMax;
        wMana = min(max(0, CharacterAttribute->Mana), wManaMax);
    }

    if (wLifeMax > 0)
    {
        if (wLife > 0 && (wLife / (float)wLifeMax) < 0.2f)
        {
            PlayBuffer(SOUND_HEART);
        }
    }

    float fLife = 0.f;
    float fMana = 0.f;

    if (wLifeMax > 0)
    {
        fLife = (wLifeMax - wLife) / (float)wLifeMax;
    }
    if (wManaMax > 0)
    {
        fMana = (wManaMax - wMana) / (float)wManaMax;
    }

    float width, height;
    float x, y;
    float fY, fH, fV;

    // life
    width = 45.f;
    x = 158;
    height = 39.f;
    y = 480.f - 48.f;

    fY = y + (fLife * height);
    fH = height - (fLife * height);
    fV = fLife;
    if (g_isCharacterBuff((&Hero->Object), eDeBuff_Poison))
    {
        RenderBitmap(IMAGE_GAUGE_GREEN, x, fY, width, fH, 0.f, fV * height / 64.f, width / 64.f, (1.0f - fV) * height / 64.f);
    }
    else
    {
        RenderBitmap(IMAGE_GAUGE_RED, x, fY, width, fH, 0.f, fV * height / 64.f, width / 64.f, (1.0f - fV) * height / 64.f);
    }

    SEASON3B::RenderNumber(x + 25, 480 - 18, wLife);

    wchar_t strTipText[256];
    if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
    {
        swprintf(strTipText, GlobalText[358], wLife, wLifeMax);
        RenderTipText((int)x, (int)418, strTipText);
    }

    // mana
    width = 45.f;
    x = 256.f + 128.f + 53.f;
    height = 39.f;
    y = 480.f - 48.f;

    fY = y + (fMana * height);
    fH = height - (fMana * height);
    fV = fMana;
    RenderBitmap(IMAGE_GAUGE_BLUE, x, fY, width, fH, 0.f, fV * height / 64.f, width / 64.f, (1.0f - fV) * height / 64.f);

    SEASON3B::RenderNumber(x + 30, 480 - 18, wMana);

    // mana
    if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
    {
        swprintf(strTipText, GlobalText[359], wMana, wManaMax);
        RenderTipText((int)x, (int)418, strTipText);
    }
}

void SEASON3B::CNewUIMainFrameWindow::RenderGuageAG()
{
    float x, y, width, height;
    float fY, fH, fV;

    DWORD dwMaxSkillMana, dwSkillMana;

    if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
    {
        dwMaxSkillMana = max(1, Master_Level_Data.wMaxBP);
        dwSkillMana = min(dwMaxSkillMana, CharacterAttribute->SkillMana);
    }
    else
    {
        dwMaxSkillMana = max(1, CharacterAttribute->SkillManaMax);
        dwSkillMana = min(dwMaxSkillMana, CharacterAttribute->SkillMana);
    }

    float fSkillMana = 0.0f;

    if (dwMaxSkillMana > 0)
    {
        fSkillMana = (dwMaxSkillMana - dwSkillMana) / (float)dwMaxSkillMana;
    }

    width = 16.f, height = 39.f;
    x = 256 + 128 + 36; y = 480.f - 49.f;
    fY = y + (fSkillMana * height);
    fH = height - (fSkillMana * height);
    fV = fSkillMana;

    RenderBitmap(IMAGE_GAUGE_AG, x, fY, width, fH, 0.f, fV * height / 64.f, width / 16.f, (1.0f - fV) * height / 64.f);
    SEASON3B::RenderNumber(x + 10, 480 - 18, (int)dwSkillMana);

    if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
    {
        wchar_t strTipText[256];

        swprintf(strTipText, GlobalText[214], dwSkillMana, dwMaxSkillMana);
        RenderTipText((int)x - 20, (int)418, strTipText);
    }
}

void SEASON3B::CNewUIMainFrameWindow::RenderGuageSD()
{
    float x, y, width, height;
    float fY, fH, fV;
    DWORD wMaxShield, wShield;

    //Master_Level_Data.wMaxShield
    if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
    {
        wMaxShield = max(1, Master_Level_Data.wMaxShield);
        wShield = min(wMaxShield, CharacterAttribute->Shield);
    }
    else
    {
        wMaxShield = max(1, CharacterAttribute->ShieldMax);
        wShield = min(wMaxShield, CharacterAttribute->Shield);
    }

    float fShield = 0.0f;

    if (wMaxShield > 0)
    {
        fShield = (wMaxShield - wShield) / (float)wMaxShield;
    }

    width = 16.f, height = 39.f;
    x = 204; y = 480.f - 49.f;
    fY = y + (fShield * height);
    fH = height - (fShield * height);
    fV = fShield;

    RenderBitmap(IMAGE_GAUGE_SD, x, fY, width, fH, 0.f, fV * height / 64.f, width / 16.f, (1.0f - fV) * height / 64.f);
    SEASON3B::RenderNumber(x + 15, 480 - 18, (int)wShield);

    height = 39.f;
    y = 480.f - 10.f - 39.f;
    if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
    {
        wchar_t strTipText[256];

        swprintf(strTipText, GlobalText[2037], wShield, wMaxShield);
        RenderTipText((int)x - 20, (int)418, strTipText);
    }
}

void SEASON3B::CNewUIMainFrameWindow::RenderExperience()
{
    __int64 wLevel;
    __int64 dwNexExperience;
    __int64 dwExperience;
    double x, y, width, height;

    if (gCharacterManager.IsMasterLevel(CharacterAttribute->Class) == true)
    {
        wLevel = (__int64)Master_Level_Data.nMLevel;
        dwNexExperience = (__int64)Master_Level_Data.lNext_MasterLevel_Experince;
        dwExperience = (__int64)Master_Level_Data.lMasterLevel_Experince;
    }
    else
    {
        wLevel = CharacterAttribute->Level;
        dwNexExperience = CharacterAttribute->NextExperience;
        dwExperience = CharacterAttribute->Experience;
    }

    if (gCharacterManager.IsMasterLevel(CharacterAttribute->Class) == true)
    {
        x = 0; y = 470; width = 6; height = 4;

        __int64 iTotalLevel = wLevel + 400;
        __int64 iTOverLevel = iTotalLevel - 255;
        __int64 iBaseExperience = 0;

        __int64 iData_Master =	// A
            (
                (
                    (__int64)9 + (__int64)iTotalLevel
                    )
                * (__int64)iTotalLevel
                * (__int64)iTotalLevel
                * (__int64)10
                )
            +
            (
                (
                    (__int64)9 + (__int64)iTOverLevel
                    )
                * (__int64)iTOverLevel
                * (__int64)iTOverLevel
                * (__int64)1000
                );
        iBaseExperience = (iData_Master - (__int64)3892250000) / (__int64)2;	// B

        double fNeedExp = (double)dwNexExperience - (double)iBaseExperience;
        double fExp = (double)dwExperience - (double)iBaseExperience;

        if (dwExperience < iBaseExperience)
        {
            fExp = 0.f;
        }

        double fExpBarNum = 0.f;
        if (fExp > 0.f && fNeedExp > 0)
        {
            fExpBarNum = ((double)fExp / (double)fNeedExp) * (double)10.f;
        }

        double fProgress = fExpBarNum - __int64(fExpBarNum);

        if (m_bExpEffect == true)
        {
            double fPreProgress = 0.f;
            double fPreExp = (double)m_loPreExp - (double)iBaseExperience;
            if (m_loPreExp < iBaseExperience)
            {
                x = 2.f; y = 473.f; width = fProgress * 629.f; height = 4.f;
                RenderBitmap(IMAGE_MASTER_GAUGE_BAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
                glColor4f(1.f, 1.f, 1.f, 0.6f);
                RenderColor(x, y, width, height);
                EndRenderColor();
            }
            else
            {
                int iPreExpBarNum = 0;
                int iExpBarNum = 0;
                if (fPreExp > 0.f && fNeedExp > 0.f)
                {
                    fPreProgress = ((double)fPreExp / (double)fNeedExp) * (double)10.f;
                    iPreExpBarNum = (int)fPreProgress;
                    fPreProgress = (double)fPreProgress - __int64(fPreProgress);
                }
                iExpBarNum = (int)fExpBarNum;

                if (iExpBarNum > iPreExpBarNum)
                {
                    x = 2.f; y = 473.f; width = fProgress * 629.f; height = 4.f;
                    RenderBitmap(IMAGE_MASTER_GAUGE_BAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
                    glColor4f(1.f, 1.f, 1.f, 0.6f);
                    RenderColor(x, y, width, height);
                    EndRenderColor();
                }
                else
                {
                    double fGapProgress = 0.f;
                    fGapProgress = (double)fProgress - (double)fPreProgress;
                    x = 2.f; y = 473.f; width = (double)fPreProgress * (double)629.f; height = 4.f;
                    RenderBitmap(IMAGE_MASTER_GAUGE_BAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);

                    x += width; width = (double)fGapProgress * (double)629.f;
                    RenderBitmap(IMAGE_MASTER_GAUGE_BAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
                    glColor4f(1.f, 1.f, 1.f, 0.6f);
                    RenderColor(x, y, width, height);
                    EndRenderColor();
                }
            }
        }
        else
        {
            x = 2.f; y = 473.f; width = fProgress * 629.f; height = 4.f;
            RenderBitmap(IMAGE_MASTER_GAUGE_BAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
        }

        int iExp = (int)fExpBarNum;
        x = 635.f; y = 469.f;
        SEASON3B::RenderNumber(x, y, iExp);

        x = 2.f; y = 473.f; width = 629.f; height = 4.f;
        if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
        {
            wchar_t strTipText[256];

            swprintf(strTipText, GlobalText[1748], dwExperience, dwNexExperience);
            RenderTipText(280, 418, strTipText);
        }
    }
    else
    {
        x = 0; y = 470; width = 6; height = 4;

        WORD wPriorLevel = wLevel - 1;
        DWORD dwPriorExperience = 0;

        if (wPriorLevel > 0)
        {
            dwPriorExperience = (9 + wPriorLevel) * wPriorLevel * wPriorLevel * 10;

            if (wPriorLevel > 255)
            {
                int iLevelOverN = wPriorLevel - 255;
                dwPriorExperience += (9 + iLevelOverN) * iLevelOverN * iLevelOverN * 1000;
            }
        }

        float fNeedExp = dwNexExperience - dwPriorExperience;
        float fExp = dwExperience - dwPriorExperience;

        if (dwExperience < dwPriorExperience)
        {
            fExp = 0.f;
        }

        float fExpBarNum = 0.f;
        if (fExp > 0.f && fNeedExp > 0)
        {
            fExpBarNum = (fExp / fNeedExp) * 10.f;
        }

        float fProgress = fExpBarNum;
        fProgress = fProgress - (int)fProgress;

        if (m_bExpEffect == true)
        {
            float fPreProgress = 0.f;
            fExp = m_dwPreExp - dwPriorExperience;
            if (m_dwPreExp < dwPriorExperience)
            {
                x = 2.f; y = 473.f; width = fProgress * 629.f; height = 4.f;
                RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
                glColor4f(1.f, 1.f, 1.f, 0.4f);
                RenderColor(x, y, width, height);
                EndRenderColor();
            }
            else
            {
                int iPreExpBarNum = 0;
                int iExpBarNum = 0;
                if (fExp > 0.f && fNeedExp > 0.f)
                {
                    fPreProgress = (fExp / fNeedExp) * 10.f;
                    iPreExpBarNum = (int)fPreProgress;
                    fPreProgress = fPreProgress - (int)fPreProgress;
                }

                iExpBarNum = (int)fExpBarNum;

                if (iExpBarNum > iPreExpBarNum)
                {
                    x = 2.f; y = 473.f; width = fProgress * 629.f; height = 4.f;
                    RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
                    glColor4f(1.f, 1.f, 1.f, 0.4f);
                    RenderColor(x, y, width, height);
                    EndRenderColor();
                }
                else
                {
                    float fGapProgress = 0.f;
                    fGapProgress = fProgress - fPreProgress;
                    x = 2.f; y = 473.f; width = fPreProgress * 629.f; height = 4.f;
                    RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
                    x += width; width = fGapProgress * 629.f;
                    RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
                    glColor4f(1.f, 1.f, 1.f, 0.4f);
                    RenderColor(x, y, width, height);
                    EndRenderColor();
                }
            }
        }
        else
        {
            x = 2.f; y = 473.f; width = fProgress * 629.f; height = 4.f;
            RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
        }

        int iExp = (int)fExpBarNum;
        x = 635.f; y = 469.f;
        SEASON3B::RenderNumber(x, y, iExp);

        x = 2.f; y = 473.f; width = 629.f; height = 4.f;
        if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
        {
            wchar_t strTipText[256];

            swprintf(strTipText, GlobalText[1748], dwExperience, dwNexExperience);
            RenderTipText(280, 418, strTipText);
        }
    }
}

void SEASON3B::CNewUIMainFrameWindow::RenderHotKeyItemCount()
{
    m_ItemHotKey.RenderItemCount();
}

void SEASON3B::CNewUIMainFrameWindow::RenderButtons()
{
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
    m_BtnCShop.Render();
#endif //defined PBG_ADD_INGAMESHOP_UI_MAINFRAME

    RenderCharInfoButton();
    m_BtnMyInven.Render();

    RenderFriendButton();

    m_BtnWindow.Render();
}

void SEASON3B::CNewUIMainFrameWindow::RenderCharInfoButton()
{
    m_BtnChaInfo.Render();

    if (g_QuestMng.IsQuestIndexByEtcListEmpty())
        return;

    if (g_Time.GetTimeCheck(5, 500))
        m_bButtonBlink = !m_bButtonBlink;

    if (m_bButtonBlink)
    {
        if (!(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC)
            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER)))
            RenderImage(IMAGE_MENU_BTN_CHAINFO, 489 + 30, 480 - 51, 30, 41, 0.0f, 41.f);
    }
}

void SEASON3B::CNewUIMainFrameWindow::RenderFriendButton()
{
    m_BtnFriend.Render();

    int iBlinkTemp = g_pFriendMenu->GetBlinkTemp();
    BOOL bIsAlertTime = (iBlinkTemp % 24 < 12);

    if (g_pFriendMenu->IsNewChatAlert() && bIsAlertTime)
    {
        RenderFriendButtonState();
    }
    if (g_pFriendMenu->IsNewMailAlert())
    {
        if (bIsAlertTime)
        {
            RenderFriendButtonState();

            if (iBlinkTemp % 24 == 11)
            {
                g_pFriendMenu->IncreaseLetterBlink();
            }
        }
    }
    else if (g_pLetterList->CheckNoReadLetter())
    {
        RenderFriendButtonState();
    }

    g_pFriendMenu->IncreaseBlinkTemp();
}

void SEASON3B::CNewUIMainFrameWindow::RenderFriendButtonState()
{
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_FRIEND) == true)
    {
        RenderImage(IMAGE_MENU_BTN_FRIEND, 489 + (30 * 3), 480 - 51, 30, 41, 0.0f, 123.f);
    }
    else
    {
        RenderImage(IMAGE_MENU_BTN_FRIEND, 489 + (30 * 3), 480 - 51, 30, 41, 0.0f, 41.f);
    }
#else //defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_FRIEND) == true)
    {
        RenderImage(IMAGE_MENU_BTN_FRIEND, 488 + 76, 480 - 51, 38, 42, 0.0f, 126.f);
    }
    else
    {
        RenderImage(IMAGE_MENU_BTN_FRIEND, 488 + 76, 480 - 51, 38, 42, 0.0f, 42.f);
    }
#endif//defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
}

bool SEASON3B::CNewUIMainFrameWindow::UpdateMouseEvent()
{
    if (g_pNewUIHotKey->IsStateGameOver() == true)
    {
        return true;
    }

    if (BtnProcess() == true)
    {
        return false;
    }

    return true;
}

bool SEASON3B::CNewUIMainFrameWindow::BtnProcess()
{
    if (g_pNewUIHotKey->CanUpdateKeyEventRelatedMyInventory() == true)
    {
        if (m_BtnMyInven.UpdateMouseEvent() == true)
        {
            g_pNewUISystem->Toggle(SEASON3B::INTERFACE_INVENTORY);
            PlayBuffer(SOUND_CLICK01);
            return true;
        }
    }
    else if (g_pNewUIHotKey->CanUpdateKeyEvent() == true)
    {
        if (m_BtnMyInven.UpdateMouseEvent() == true)
        {
            g_pNewUISystem->Toggle(SEASON3B::INTERFACE_INVENTORY);
            PlayBuffer(SOUND_CLICK01);
            return true;
        }
        else if (m_BtnChaInfo.UpdateMouseEvent() == true)
        {
            g_pNewUISystem->Toggle(SEASON3B::INTERFACE_CHARACTER);

            PlayBuffer(SOUND_CLICK01);

            if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER))
                g_QuestMng.SendQuestIndexByEtcSelection();

            return true;
        }
        else if (m_BtnFriend.UpdateMouseEvent() == true)
        {
            if (gMapManager.InChaosCastle() == true)
            {
                PlayBuffer(SOUND_CLICK01);
                return true;
            }

            int iLevel = CharacterAttribute->Level;

            if (iLevel < 6)
            {
                if (g_pSystemLogBox->CheckChatRedundancy(GlobalText[1067]) == FALSE)
                {
                    g_pSystemLogBox->AddText(GlobalText[1067], SEASON3B::TYPE_SYSTEM_MESSAGE);
                }
            }
            else
            {
                g_pNewUISystem->Toggle(SEASON3B::INTERFACE_FRIEND);
            }
            PlayBuffer(SOUND_CLICK01);
            return true;
        }
        else if (m_BtnWindow.UpdateMouseEvent() == true)
        {
            g_pNewUISystem->Toggle(SEASON3B::INTERFACE_WINDOW_MENU);
            PlayBuffer(SOUND_CLICK01);
            return true;
        }

#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
        else if (m_BtnCShop.UpdateMouseEvent() == true)
        {
            if (g_pInGameShop->IsInGameShopOpen() == false)
                return false;

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
            if (g_InGameShopSystem->IsScriptDownload() == true)
            {
                if (g_InGameShopSystem->ScriptDownload() == false)
                    return false;
            }

            if (g_InGameShopSystem->IsBannerDownload() == true)
            {
                g_InGameShopSystem->BannerDownload();
            }
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

            if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INGAMESHOP) == false)
            {
                if (g_InGameShopSystem->GetIsRequestShopOpenning() == false)
                {
                    SocketClient->ToGameServer()->SendCashShopOpenState(0);
                    g_InGameShopSystem->SetIsRequestShopOpenning(true);

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
                    g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, true);
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
                }
            }
            else
            {
                SocketClient->ToGameServer()->SendCashShopOpenState(1);
                g_pNewUISystem->Hide(SEASON3B::INTERFACE_INGAMESHOP);
            }

            return true;
        }
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
    }

    return false;
}

bool SEASON3B::CNewUIMainFrameWindow::UpdateKeyEvent()
{
    if (m_ItemHotKey.UpdateKeyEvent() == false)
    {
        return false;
    }
    return true;
}

bool SEASON3B::CNewUIMainFrameWindow::Update()
{
    if (m_bExpEffect == true)
    {
        if (timeGetTime() - m_dwExpEffectTime > 2000)
        {
            m_bExpEffect = false;
            m_dwExpEffectTime = 0;
            m_dwGetExp = 0;
        }
    }

    return true;
}

float SEASON3B::CNewUIMainFrameWindow::GetLayerDepth()
{
    return 10.6f;
}

float SEASON3B::CNewUIMainFrameWindow::GetKeyEventOrder()
{
    return 2.9f;
}

void SEASON3B::CNewUIMainFrameWindow::SetItemHotKey(int iHotKey, int iItemType, int iItemLevel)
{
    m_ItemHotKey.SetHotKey(iHotKey, iItemType, iItemLevel);
}

int SEASON3B::CNewUIMainFrameWindow::GetItemHotKey(int iHotKey)
{
    return m_ItemHotKey.GetHotKey(iHotKey);
}

int SEASON3B::CNewUIMainFrameWindow::GetItemHotKeyLevel(int iHotKey)
{
    return m_ItemHotKey.GetHotKeyLevel(iHotKey);
}

void SEASON3B::CNewUIMainFrameWindow::UseHotKeyItemRButton()
{
    m_ItemHotKey.UseItemRButton();
}

void SEASON3B::CNewUIMainFrameWindow::UpdateItemHotKey()
{
    m_ItemHotKey.UpdateKeyEvent();
}

void SEASON3B::CNewUIMainFrameWindow::ResetSkillHotKey()
{
    g_pSkillList->Reset();
}

void SEASON3B::CNewUIMainFrameWindow::SetSkillHotKey(int iHotKey, int iSkillType)
{
    g_pSkillList->SetHotKey(iHotKey, iSkillType);
}

int SEASON3B::CNewUIMainFrameWindow::GetSkillHotKey(int iHotKey)
{
    return g_pSkillList->GetHotKey(iHotKey);
}

int SEASON3B::CNewUIMainFrameWindow::GetSkillHotKeyIndex(int iSkillType)
{
    return g_pSkillList->GetSkillIndex(iSkillType);
}

SEASON3B::CNewUIItemHotKey::CNewUIItemHotKey()
{
    for (int i = 0; i < HOTKEY_COUNT; ++i)
    {
        m_iHotKeyItemType[i] = -1;
        m_iHotKeyItemLevel[i] = 0;
    }
}

SEASON3B::CNewUIItemHotKey::~CNewUIItemHotKey()
{
}

bool SEASON3B::CNewUIItemHotKey::UpdateKeyEvent()
{
    int iIndex = -1;

    if (SEASON3B::IsPress('Q') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_Q);
    }
    else if (SEASON3B::IsPress('W') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_W);
    }
    else if (SEASON3B::IsPress('E') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_E);
    }
    else if (SEASON3B::IsPress('R') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_R);
    }

    if (iIndex != -1)
    {
        ITEM* pItem = NULL;
        pItem = g_pMyInventory->FindItem(iIndex);
        if ((pItem->Type >= ITEM_POTION + 78 && pItem->Type <= ITEM_POTION + 82))
        {
            std::list<eBuffState> secretPotionbufflist;
            secretPotionbufflist.push_back(eBuff_SecretPotion1);
            secretPotionbufflist.push_back(eBuff_SecretPotion2);
            secretPotionbufflist.push_back(eBuff_SecretPotion3);
            secretPotionbufflist.push_back(eBuff_SecretPotion4);
            secretPotionbufflist.push_back(eBuff_SecretPotion5);

            if (g_isCharacterBufflist((&Hero->Object), secretPotionbufflist) != eBuffNone) {
                SEASON3B::CreateOkMessageBox(GlobalText[2530], RGBA(255, 30, 0, 255));
            }
            else {
                SendRequestUse(iIndex, 0);
            }
        }
        else

        {
            SendRequestUse(iIndex, 0);
        }
        return false;
    }

    return true;
}

int SEASON3B::CNewUIItemHotKey::GetHotKeyItemIndex(int iType, bool bItemCount)
{
    int iStartItemType = 0, iEndItemType = 0;
    int i, j;

    switch (iType)
    {
    case HOTKEY_Q:
        if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
        {
            if (m_iHotKeyItemType[iType] >= ITEM_SMALL_MANA_POTION && m_iHotKeyItemType[iType] <= ITEM_LARGE_MANA_POTION)
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
            else
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
        }
        break;
    case HOTKEY_W:
        if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
        {
            if (m_iHotKeyItemType[iType] >= ITEM_APPLE && m_iHotKeyItemType[iType] <= ITEM_LARGE_HEALING_POTION)
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
            else
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
        }
        break;
    case HOTKEY_E:
        if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
        {
            if (m_iHotKeyItemType[iType] >= ITEM_APPLE && m_iHotKeyItemType[iType] <= ITEM_LARGE_HEALING_POTION)
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
            else if (m_iHotKeyItemType[iType] >= ITEM_SMALL_MANA_POTION && m_iHotKeyItemType[iType] <= ITEM_LARGE_MANA_POTION)
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
            else
            {
                iStartItemType = ITEM_ANTIDOTE; iEndItemType = ITEM_ANTIDOTE;
            }
        }
        break;
    case HOTKEY_R:
        if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
        {
            if (m_iHotKeyItemType[iType] >= ITEM_APPLE && m_iHotKeyItemType[iType] <= ITEM_LARGE_HEALING_POTION)
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
            else if (m_iHotKeyItemType[iType] >= ITEM_SMALL_MANA_POTION && m_iHotKeyItemType[iType] <= ITEM_LARGE_MANA_POTION)
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
            else
            {
                iStartItemType = ITEM_LARGE_SHIELD_POTION; iEndItemType = ITEM_SMALL_SHIELD_POTION;
            }
        }
        break;
    }

    int iItemCount = 0;
    ITEM* pItem = NULL;

    int iNumberofItems = g_pMyInventory->GetInventoryCtrl()->GetNumberOfItems();
    for (i = iStartItemType; i >= iEndItemType; --i)
    {
        if (bItemCount)
        {
            for (j = 0; j < iNumberofItems; ++j)
            {
                pItem = g_pMyInventory->GetInventoryCtrl()->GetItem(j);
                if (pItem == NULL)
                {
                    continue;
                }

                if (
                    (pItem->Type == i && pItem->Level == m_iHotKeyItemLevel[iType])
                    || (pItem->Type == i && (pItem->Type >= ITEM_APPLE && pItem->Type <= ITEM_LARGE_HEALING_POTION))
                    )
                {
                    if (pItem->Type == ITEM_ALE
                        || pItem->Type == ITEM_TOWN_PORTAL_SCROLL
                        || pItem->Type == ITEM_POTION + 20
                        )
                    {
                        iItemCount++;
                    }
                    else
                    {
                        iItemCount += pItem->Durability;
                    }
                }
            }
        }
        else
        {
            int iIndex = -1;
            if (i >= ITEM_APPLE && i <= ITEM_LARGE_HEALING_POTION)
            {
                iIndex = g_pMyInventory->FindItemReverseIndex(i);
            }
            else
            {
                iIndex = g_pMyInventory->FindItemReverseIndex(i, m_iHotKeyItemLevel[iType]);
            }

            if (-1 != iIndex)
            {
                pItem = g_pMyInventory->FindItem(iIndex);
                if ((pItem->Type != ITEM_SIEGE_POTION
                    && pItem->Type != ITEM_TOWN_PORTAL_SCROLL
                    && pItem->Type != ITEM_POTION + 20)
                    || pItem->Level == m_iHotKeyItemLevel[iType]
                    )
                {
                    return iIndex;
                }
            }
        }
    }

    if (bItemCount == true)
    {
        return iItemCount;
    }

    return -1;
}

bool SEASON3B::CNewUIItemHotKey::GetHotKeyCommonItem(IN int iHotKey, OUT int& iStart, OUT int& iEnd)
{
    switch (m_iHotKeyItemType[iHotKey])
    {
    case ITEM_SIEGE_POTION:
    case ITEM_ANTIDOTE:
    case ITEM_ALE:
    case ITEM_TOWN_PORTAL_SCROLL:
    case ITEM_POTION + 20:
    case ITEM_JACK_OLANTERN_BLESSINGS:
    case ITEM_JACK_OLANTERN_WRATH:
    case ITEM_JACK_OLANTERN_CRY:
    case ITEM_JACK_OLANTERN_FOOD:
    case ITEM_JACK_OLANTERN_DRINK:
    case ITEM_POTION + 70:
    case ITEM_POTION + 71:
    case ITEM_POTION + 78:
    case ITEM_POTION + 79:
    case ITEM_POTION + 80:
    case ITEM_POTION + 81:
    case ITEM_POTION + 82:
    case ITEM_POTION + 94:
    case ITEM_CHERRY_BLOSSOM_WINE:
    case ITEM_CHERRY_BLOSSOM_RICE_CAKE:
    case ITEM_CHERRY_BLOSSOM_FLOWER_PETAL:
    case ITEM_POTION + 133:
        if (m_iHotKeyItemType[iHotKey] != ITEM_POTION + 20 || m_iHotKeyItemLevel[iHotKey] == 0)
        {
            iStart = iEnd = m_iHotKeyItemType[iHotKey];
            return true;
        }
        break;
    default:
        if (m_iHotKeyItemType[iHotKey] >= ITEM_SMALL_SHIELD_POTION && m_iHotKeyItemType[iHotKey] <= ITEM_LARGE_SHIELD_POTION)
        {
            iStart = ITEM_LARGE_SHIELD_POTION; iEnd = ITEM_SMALL_SHIELD_POTION;
            return true;
        }
        else if (m_iHotKeyItemType[iHotKey] >= ITEM_SMALL_COMPLEX_POTION && m_iHotKeyItemType[iHotKey] <= ITEM_LARGE_COMPLEX_POTION)
        {
            iStart = ITEM_LARGE_COMPLEX_POTION; iEnd = ITEM_SMALL_COMPLEX_POTION;
            return true;
        }
        break;
    }
    return false;
}

int SEASON3B::CNewUIItemHotKey::GetHotKeyItemCount(int iType)
{
    return 0;
}

void SEASON3B::CNewUIItemHotKey::SetHotKey(int iHotKey, int iItemType, int iItemLevel)
{
    if (iHotKey != -1 && CNewUIMyInventory::CanRegisterItemHotKey(iItemType) == true
        )
    {
        m_iHotKeyItemType[iHotKey] = iItemType;
        m_iHotKeyItemLevel[iHotKey] = iItemLevel;
    }
    else
    {
        m_iHotKeyItemType[iHotKey] = -1;
        m_iHotKeyItemLevel[iHotKey] = 0;
    }
}

int SEASON3B::CNewUIItemHotKey::GetHotKey(int iHotKey)
{
    if (iHotKey != -1)
    {
        return m_iHotKeyItemType[iHotKey];
    }

    return -1;
}

int SEASON3B::CNewUIItemHotKey::GetHotKeyLevel(int iHotKey)
{
    if (iHotKey != -1)
    {
        return m_iHotKeyItemLevel[iHotKey];
    }

    return 0;
}

void SEASON3B::CNewUIItemHotKey::RenderItems()
{
    float x, y, width, height;

    for (int i = 0; i < HOTKEY_COUNT; ++i)
    {
        int iIndex = GetHotKeyItemIndex(i);
        if (iIndex != -1)
        {
            ITEM* pItem = g_pMyInventory->FindItem(iIndex);
            if (pItem)
            {
                x = 10 + (i * 38); y = 443; width = 20; height = 20;
                RenderItem3D(x, y, width, height, pItem->Type, pItem->Level, 0, 0);
            }
        }
    }
}

void SEASON3B::CNewUIItemHotKey::RenderItemCount()
{
    float x, y, width, height;

    glColor4f(1.f, 1.f, 1.f, 1.f);

    for (int i = 0; i < HOTKEY_COUNT; ++i)
    {
        int iCount = GetHotKeyItemIndex(i, true);
        if (iCount > 0)
        {
            x = 30 + (i * 38); y = 457; width = 8; height = 9;
            SEASON3B::RenderNumber(x, y, iCount);
        }
    }
}

void SEASON3B::CNewUIItemHotKey::UseItemRButton()
{
    int x, y, width, height;

    for (int i = 0; i < HOTKEY_COUNT; ++i)
    {
        x = 10 + (i * 38); y = 445; width = 20; height = 20;
        if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
        {
            if (MouseRButtonPush)
            {
                MouseRButtonPush = false;
                int iIndex = GetHotKeyItemIndex(i);
                if (iIndex != -1)
                {
                    SendRequestUse(iIndex, 0);
                    break;
                }
            }
        }
    }
}

SEASON3B::CNewUISkillList::CNewUISkillList()
{
    m_pNewUIMng = NULL;
    Reset();
}

SEASON3B::CNewUISkillList::~CNewUISkillList()
{
    Release();
}

bool SEASON3B::CNewUISkillList::Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_SKILL_LIST, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;

    LoadImages();

    Show(true);

    return true;
}

void SEASON3B::CNewUISkillList::Release()
{
    if (m_pNewUI3DRenderMng)
    {
        m_pNewUI3DRenderMng->DeleteUI2DEffectObject(UI2DEffectCallback);
    }

    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUISkillList::Reset()
{
    m_bSkillList = false;
    m_bHotKeySkillListUp = false;

    m_bRenderSkillInfo = false;
    m_iRenderSkillInfoType = 0;
    m_iRenderSkillInfoPosX = 0;
    m_iRenderSkillInfoPosY = 0;

    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        m_iHotKeySkillType[i] = -1;
    }

    m_EventState = EVENT_NONE;
}

void SEASON3B::CNewUISkillList::LoadImages()
{
    LoadBitmap(L"Interface\\newui_skill.jpg", IMAGE_SKILL1, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skill2.jpg", IMAGE_SKILL2, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_command.jpg", IMAGE_COMMAND, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skillbox.jpg", IMAGE_SKILLBOX, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skillbox2.jpg", IMAGE_SKILLBOX_USE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_skill.jpg", IMAGE_NON_SKILL1, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_skill2.jpg", IMAGE_NON_SKILL2, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_command.jpg", IMAGE_NON_COMMAND, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skill3.jpg", IMAGE_SKILL3, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_skill3.jpg", IMAGE_NON_SKILL3, GL_LINEAR);
}

void SEASON3B::CNewUISkillList::UnloadImages()
{
    DeleteBitmap(IMAGE_SKILL1);
    DeleteBitmap(IMAGE_SKILL2);
    DeleteBitmap(IMAGE_COMMAND);
    DeleteBitmap(IMAGE_SKILLBOX);
    DeleteBitmap(IMAGE_SKILLBOX_USE);
    DeleteBitmap(IMAGE_NON_SKILL1);
    DeleteBitmap(IMAGE_NON_SKILL2);
    DeleteBitmap(IMAGE_NON_COMMAND);
    DeleteBitmap(IMAGE_SKILL3);
    DeleteBitmap(IMAGE_NON_SKILL3);
}

bool SEASON3B::CNewUISkillList::UpdateMouseEvent()
{
#ifdef MOD_SKILLLIST_UPDATEMOUSE_BLOCK
    if (GFxProcess::GetInstancePtr()->GetUISelect() == 1)
    {
        return true;
    }
#endif //MOD_SKILLLIST_UPDATEMOUSE_BLOCK

    if (g_isCharacterBuff((&Hero->Object), eBuff_DuelWatch))
    {
        m_bSkillList = false;
        return true;
    }

    BYTE bySkillNumber = CharacterAttribute->SkillNumber;
    BYTE bySkillMasterNumber = CharacterAttribute->SkillMasterNumber;

    float x, y, width, height;

    m_bRenderSkillInfo = false;

    if (bySkillNumber <= 0)
    {
        return true;
    }

    x = 385.f; y = 431.f; width = 32.f; height = 38.f;
    if (m_EventState == EVENT_NONE && MouseLButtonPush == false
        && SEASON3B::CheckMouseIn(x, y, width, height) == true)
    {
        m_EventState = EVENT_BTN_HOVER_CURRENTSKILL;
        return true;
    }
    if (m_EventState == EVENT_BTN_HOVER_CURRENTSKILL && MouseLButtonPush == false
        && SEASON3B::CheckMouseIn(x, y, width, height) == false)
    {
        m_EventState = EVENT_NONE;
        return true;
    }
    if (m_EventState == EVENT_BTN_HOVER_CURRENTSKILL && (MouseLButtonPush == true || MouseLButtonDBClick == true)
        && SEASON3B::CheckMouseIn(x, y, width, height) == true)
    {
        m_EventState = EVENT_BTN_DOWN_CURRENTSKILL;
        return false;
    }
    if (m_EventState == EVENT_BTN_DOWN_CURRENTSKILL)
    {
        if (MouseLButtonPush == false && MouseLButtonDBClick == false)
        {
            if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
            {
                m_bSkillList = !m_bSkillList;
                PlayBuffer(SOUND_CLICK01);
                m_EventState = EVENT_NONE;
                return false;
            }
            m_EventState = EVENT_NONE;
            return true;
        }
    }

    if (m_EventState == EVENT_BTN_HOVER_CURRENTSKILL)
    {
        m_bRenderSkillInfo = true;
        m_iRenderSkillInfoType = Hero->CurrentSkill;
        m_iRenderSkillInfoPosX = x - 5;
        m_iRenderSkillInfoPosY = y;

        return false;
    }
    else if (m_EventState == EVENT_BTN_DOWN_CURRENTSKILL)
    {
        return false;
    }

    x = 222.f; y = 431.f; width = 32.f * 5.f; height = 38.f;
    if (m_EventState == EVENT_NONE && MouseLButtonPush == false
        && SEASON3B::CheckMouseIn(x, y, width, height) == true)
    {
        m_EventState = EVENT_BTN_HOVER_SKILLHOTKEY;
        return true;
    }
    if (m_EventState == EVENT_BTN_HOVER_SKILLHOTKEY && MouseLButtonPush == false
        && SEASON3B::CheckMouseIn(x, y, width, height) == false)
    {
        m_EventState = EVENT_NONE;
        return true;
    }
    if (m_EventState == EVENT_BTN_HOVER_SKILLHOTKEY && MouseLButtonPush == true
        && SEASON3B::CheckMouseIn(x, y, width, height) == true)
    {
        m_EventState = EVENT_BTN_DOWN_SKILLHOTKEY;
        return false;
    }

    x = 190.f; y = 431.f; width = 32.f; height = 38.f;
    int iStartIndex = (m_bHotKeySkillListUp == true) ? 6 : 1;
    for (int i = 0, iIndex = iStartIndex; i < 5; ++i, iIndex++)
    {
        x += width;

        if (iIndex == 10)
        {
            iIndex = 0;
        }
        if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
        {
            if (m_iHotKeySkillType[iIndex] == -1)
            {
                if (m_EventState == EVENT_BTN_HOVER_SKILLHOTKEY)
                {
                    m_bRenderSkillInfo = false;
                    m_iRenderSkillInfoType = -1;
                }
                if (m_EventState == EVENT_BTN_DOWN_SKILLHOTKEY && MouseLButtonPush == false)
                {
                    m_EventState = EVENT_NONE;
                }
                continue;
            }

            WORD bySkillType = CharacterAttribute->Skill[m_iHotKeySkillType[iIndex]];

            if (bySkillType == 0 || (bySkillType >= AT_SKILL_STUN && bySkillType <= AT_SKILL_REMOVAL_BUFF))
                continue;

            BYTE bySkillUseType = SkillAttribute[bySkillType].SkillUseType;

            if (bySkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
            {
                continue;
            }

            if (m_EventState == EVENT_BTN_HOVER_SKILLHOTKEY)
            {
                m_bRenderSkillInfo = true;
                m_iRenderSkillInfoType = m_iHotKeySkillType[iIndex];
                m_iRenderSkillInfoPosX = x - 5;
                m_iRenderSkillInfoPosY = y;
                return true;
            }
            if (m_EventState == EVENT_BTN_DOWN_SKILLHOTKEY)
            {
                if (MouseLButtonPush == false)
                {
                    if (m_iRenderSkillInfoType == m_iHotKeySkillType[iIndex])
                    {
                        m_EventState = EVENT_NONE;
                        m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
                        Hero->CurrentSkill = m_iHotKeySkillType[iIndex];
                        PlayBuffer(SOUND_CLICK01);
                        return false;
                    }
                    else
                    {
                        m_EventState = EVENT_NONE;
                    }
                }
            }
        }
    }

    x = 222.f; y = 431.f; width = 32.f * 5.f; height = 38.f;
    if (m_EventState == EVENT_BTN_DOWN_SKILLHOTKEY)
    {
        if (MouseLButtonPush == false && SEASON3B::CheckMouseIn(x, y, width, height) == false)
        {
            m_EventState = EVENT_NONE;
            return true;
        }
        return false;
    }

    if (m_bSkillList == false)
        return true;

    WORD bySkillType = 0;

    int iSkillCount = 0;
    bool bMouseOnSkillList = false;

    x = 385.f; y = 390; width = 32; height = 38;
    float fOrigX = 385.f;

    EVENT_STATE PrevEventState = m_EventState;

    for (int i = 0; i < MAX_MAGIC; ++i)
    {
        bySkillType = CharacterAttribute->Skill[i];

        if (bySkillType == 0 || (bySkillType >= AT_SKILL_STUN && bySkillType <= AT_SKILL_REMOVAL_BUFF))
            continue;

        BYTE bySkillUseType = SkillAttribute[bySkillType].SkillUseType;

        if (bySkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
        {
            continue;
        }

        if (iSkillCount == 18)
        {
            y -= height;
        }

        if (iSkillCount < 14)
        {
            int iRemainder = iSkillCount % 2;
            int iQuotient = iSkillCount / 2;

            if (iRemainder == 0)
            {
                x = fOrigX + iQuotient * width;
            }
            else
            {
                x = fOrigX - (iQuotient + 1) * width;
            }
        }
        else if (iSkillCount >= 14 && iSkillCount < 18)
        {
            x = fOrigX - (8 * width) - ((iSkillCount - 14) * width);
        }
        else
        {
            x = fOrigX - (12 * width) + ((iSkillCount - 17) * width);
        }

        iSkillCount++;

        if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
        {
            bMouseOnSkillList = true;
            if (m_EventState == EVENT_NONE && MouseLButtonPush == false)
            {
                m_EventState = EVENT_BTN_HOVER_SKILLLIST;
                break;
            }
        }

        if (m_EventState == EVENT_BTN_HOVER_SKILLLIST && MouseLButtonPush == true
            && SEASON3B::CheckMouseIn(x, y, width, height) == true)
        {
            m_EventState = EVENT_BTN_DOWN_SKILLLIST;
            break;
        }

        if (m_EventState == EVENT_BTN_HOVER_SKILLLIST && MouseLButtonPush == false
            && SEASON3B::CheckMouseIn(x, y, width, height) == true)
        {
            m_bRenderSkillInfo = true;
            m_iRenderSkillInfoType = i;
            m_iRenderSkillInfoPosX = x;
            m_iRenderSkillInfoPosY = y;
        }

        if (m_EventState == EVENT_BTN_DOWN_SKILLLIST && MouseLButtonPush == false
            && m_iRenderSkillInfoType == i && SEASON3B::CheckMouseIn(x, y, width, height) == true)
        {
            m_EventState = EVENT_NONE;

            m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];

            Hero->CurrentSkill = i;
            m_bSkillList = false;

            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    if (PrevEventState != m_EventState)
    {
        if (m_EventState == EVENT_NONE || m_EventState == EVENT_BTN_HOVER_SKILLLIST)
            return true;
        return false;
    }

    if (Hero->m_pPet != NULL)
    {
        x = 353.f; y = 352; width = 32; height = 38;
        for (int i = AT_PET_COMMAND_DEFAULT; i < AT_PET_COMMAND_END; ++i)
        {
            if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
            {
                bMouseOnSkillList = true;

                if (m_EventState == EVENT_NONE && MouseLButtonPush == false)
                {
                    m_EventState = EVENT_BTN_HOVER_SKILLLIST;
                    return true;
                }
                if (m_EventState == EVENT_BTN_HOVER_SKILLLIST && MouseLButtonPush == true)
                {
                    m_EventState = EVENT_BTN_DOWN_SKILLLIST;
                    return false;
                }

                if (m_EventState == EVENT_BTN_HOVER_SKILLLIST)
                {
                    m_bRenderSkillInfo = true;
                    m_iRenderSkillInfoType = i;
                    m_iRenderSkillInfoPosX = x;
                    m_iRenderSkillInfoPosY = y;
                }
                if (m_EventState == EVENT_BTN_DOWN_SKILLLIST && MouseLButtonPush == false
                    && m_iRenderSkillInfoType == i)
                {
                    m_EventState = EVENT_NONE;

                    m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];

                    Hero->CurrentSkill = i;
                    m_bSkillList = false;
                    PlayBuffer(SOUND_CLICK01);
                    return false;
                }
            }
            x += width;
        }
    }

    if (bMouseOnSkillList == false && m_EventState == EVENT_BTN_HOVER_SKILLLIST)
    {
        m_EventState = EVENT_NONE;
        return true;
    }
    if (bMouseOnSkillList == false && MouseLButtonPush == false
        && m_EventState == EVENT_BTN_DOWN_SKILLLIST)
    {
        m_EventState = EVENT_NONE;
        return false;
    }
    if (m_EventState == EVENT_BTN_DOWN_SKILLLIST)
    {
        if (MouseLButtonPush == false)
        {
            m_EventState = EVENT_NONE;
            return true;
        }
        return false;
    }

    return true;
}

bool SEASON3B::CNewUISkillList::UpdateKeyEvent()
{
    for (int i = 0; i < 9; ++i)
    {
        if (SEASON3B::IsPress('1' + i))
        {
            UseHotKey(i + 1);
        }
    }

    if (SEASON3B::IsPress('0'))
    {
        UseHotKey(0);
    }

    if (m_EventState == EVENT_BTN_HOVER_SKILLLIST)
    {
        if (SEASON3B::IsRepeat(VK_CONTROL))
        {
            for (int i = 0; i < 9; ++i)
            {
                if (SEASON3B::IsPress('1' + i))
                {
                    SetHotKey(i + 1, m_iRenderSkillInfoType);

                    return false;
                }
            }

            if (SEASON3B::IsPress('0'))
            {
                SetHotKey(0, m_iRenderSkillInfoType);

                return false;
            }
        }
    }

    if (SEASON3B::IsRepeat(VK_SHIFT))
    {
        for (int i = 0; i < 4; ++i)
        {
            if (SEASON3B::IsPress('1' + i))
            {
                Hero->CurrentSkill = AT_PET_COMMAND_DEFAULT + i;
                return false;
            }
        }
    }

    return true;
}

bool SEASON3B::CNewUISkillList::IsArrayUp(BYTE bySkill)
{
    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        if (m_iHotKeySkillType[i] == bySkill)
        {
            if (i == 0 || i > 5)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    return false;
}

bool SEASON3B::CNewUISkillList::IsArrayIn(BYTE bySkill)
{
    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        if (m_iHotKeySkillType[i] == bySkill)
        {
            return true;
        }
    }

    return false;
}

void SEASON3B::CNewUISkillList::SetHotKey(int iHotKey, int iSkillType)
{
    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        if (m_iHotKeySkillType[i] == iSkillType)
        {
            m_iHotKeySkillType[i] = -1;
            break;
        }
    }

    m_iHotKeySkillType[iHotKey] = iSkillType;
}

int SEASON3B::CNewUISkillList::GetHotKey(int iHotKey)
{
    return m_iHotKeySkillType[iHotKey];
}

int SEASON3B::CNewUISkillList::GetSkillIndex(int iSkillType)
{
    // special handling for skills with different skill id for the trigger
    if (iSkillType == AT_SKILL_NOVA_BEGIN)
    {
        iSkillType = AT_SKILL_NOVA;
    }

    int iReturn = -1;
    for (int i = 0; i < MAX_MAGIC; ++i)
    {
        if (CharacterAttribute->Skill[i] == iSkillType)
        {
            iReturn = i;
            break;
        }
    }

    return iReturn;
}

void SEASON3B::CNewUISkillList::UseHotKey(int iHotKey)
{
    if (m_iHotKeySkillType[iHotKey] != -1)
    {
        if (m_iHotKeySkillType[iHotKey] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iHotKey] < AT_PET_COMMAND_END)
        {
            if (Hero->m_pPet == NULL)
            {
                return;
            }
        }

        auto wHotKeySkill = CharacterAttribute->Skill[m_iHotKeySkillType[iHotKey]];

        if (wHotKeySkill == 0)
        {
            return;
        }

        m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];

        Hero->CurrentSkill = m_iHotKeySkillType[iHotKey];

        auto bySkill = CharacterAttribute->Skill[Hero->CurrentSkill];

        if (
            g_pOption->IsAutoAttack() == true
            && gMapManager.WorldActive != WD_6STADIUM
            && gMapManager.InChaosCastle() == false
            && (bySkill == AT_SKILL_TELEPORT || bySkill == AT_SKILL_TELEPORT_ALLY))
        {
            SelectedCharacter = -1;
            Attacking = -1;
        }
    }
}

bool SEASON3B::CNewUISkillList::Update()
{
    if (IsArrayIn(Hero->CurrentSkill) == true)
    {
        if (IsArrayUp(Hero->CurrentSkill) == true)
        {
            m_bHotKeySkillListUp = true;
        }
        else
        {
            m_bHotKeySkillListUp = false;
        }
    }

    if (Hero->m_pPet == NULL)
    {
        if (Hero->CurrentSkill >= AT_PET_COMMAND_DEFAULT && Hero->CurrentSkill < AT_PET_COMMAND_END)
        {
            Hero->CurrentSkill = 0;
        }
    }

    return true;
}

void SEASON3B::CNewUISkillList::RenderCurrentSkillAndHotSkillList()
{
    int i;
    float x, y, width, height;

    BYTE bySkillNumber = CharacterAttribute->SkillNumber;

    if (bySkillNumber > 0)
    {
        int iStartSkillIndex = 1;
        if (m_bHotKeySkillListUp)
        {
            iStartSkillIndex = 6;
        }

        x = 190; y = 431; width = 32; height = 38;
        for (i = 0; i < 5; ++i)
        {
            x += width;

            int iIndex = iStartSkillIndex + i;
            if (iIndex == 10)
            {
                iIndex = 0;
            }

            if (m_iHotKeySkillType[iIndex] == -1)
            {
                continue;
            }

            if (m_iHotKeySkillType[iIndex] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iIndex] < AT_PET_COMMAND_END)
            {
                if (Hero->m_pPet == NULL)
                {
                    continue;
                }
            }

            if (Hero->CurrentSkill == m_iHotKeySkillType[iIndex])
            {
                SEASON3B::RenderImage(IMAGE_SKILLBOX_USE, x, y, width, height);
            }
            RenderSkillIcon(m_iHotKeySkillType[iIndex], x + 6, y + 6, 20, 28);
        }

        x = 392; y = 437; width = 20; height = 28;
        RenderSkillIcon(Hero->CurrentSkill, x, y, width, height);
    }
}

bool SEASON3B::CNewUISkillList::Render()
{
    int i;
    float x, y, width, height;

    BYTE bySkillNumber = CharacterAttribute->SkillNumber;

    if (bySkillNumber > 0)
    {
        if (m_bSkillList == true)
        {
            x = 385; y = 390; width = 32; height = 38;
            float fOrigX = 385.f;
            int iSkillType = 0;
            int iSkillCount = 0;

            for (i = 0; i < MAX_MAGIC; ++i)
            {
                iSkillType = CharacterAttribute->Skill[i];

                if (iSkillType != 0 && (iSkillType < AT_SKILL_STUN || iSkillType > AT_SKILL_REMOVAL_BUFF))
                {
                    BYTE bySkillUseType = SkillAttribute[iSkillType].SkillUseType;

                    if (bySkillUseType == SKILL_USE_TYPE_MASTER || bySkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
                    {
                        continue;
                    }

                    if (iSkillCount == 18)
                    {
                        y -= height;
                    }

                    if (iSkillCount < 14)
                    {
                        int iRemainder = iSkillCount % 2;
                        int iQuotient = iSkillCount / 2;

                        if (iRemainder == 0)
                        {
                            x = fOrigX + iQuotient * width;
                        }
                        else
                        {
                            x = fOrigX - (iQuotient + 1) * width;
                        }
                    }
                    else if (iSkillCount >= 14 && iSkillCount < 18)
                    {
                        x = fOrigX - (8 * width) - ((iSkillCount - 14) * width);
                    }
                    else
                    {
                        x = fOrigX - (12 * width) + ((iSkillCount - 17) * width);
                    }

                    iSkillCount++;

                    if (i == Hero->CurrentSkill)
                    {
                        SEASON3B::RenderImage(IMAGE_SKILLBOX_USE, x, y, width, height);
                    }
                    else
                    {
                        SEASON3B::RenderImage(IMAGE_SKILLBOX, x, y, width, height);
                    }

                    RenderSkillIcon(i, x + 6, y + 6, 20, 28);
                }
            }
            RenderPetSkill();
        }
    }

    if (m_bRenderSkillInfo == true && m_pNewUI3DRenderMng)
    {
        m_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, 0, 0);

        m_bRenderSkillInfo = false;
    }

    return true;
}

void SEASON3B::CNewUISkillList::RenderSkillInfo()
{
    ::RenderSkillInfo(m_iRenderSkillInfoPosX + 15, m_iRenderSkillInfoPosY - 10, m_iRenderSkillInfoType);
}

float SEASON3B::CNewUISkillList::GetLayerDepth()
{
    return 5.2f;
}

WORD SEASON3B::CNewUISkillList::GetHeroPriorSkill()
{
    return m_wHeroPriorSkill;
}

void SEASON3B::CNewUISkillList::SetHeroPriorSkill(BYTE bySkill)
{
    m_wHeroPriorSkill = bySkill;
}

void SEASON3B::CNewUISkillList::RenderPetSkill()
{
    if (Hero->m_pPet == NULL)
    {
        return;
    }

    float x, y, width, height;

    x = 353.f; y = 352; width = 32; height = 38;
    for (int i = AT_PET_COMMAND_DEFAULT; i < AT_PET_COMMAND_END; ++i)
    {
        if (i == Hero->CurrentSkill)
        {
            SEASON3B::RenderImage(IMAGE_SKILLBOX_USE, x, y, width, height);
        }
        else
        {
            SEASON3B::RenderImage(IMAGE_SKILLBOX, x, y, width, height);
        }

        RenderSkillIcon(i, x + 6, y + 6, 20, 28);
        x += width;
    }
}

void SEASON3B::CNewUISkillList::RenderSkillIcon(int iIndex, float x, float y, float width, float height)
{
    auto bySkillType = CharacterAttribute->Skill[iIndex];

    if (bySkillType == 0)
    {
        return;
    }

    if (iIndex >= AT_PET_COMMAND_DEFAULT)
    {
        bySkillType = (ActionSkillType)iIndex;
    }

    bool bCantSkill = false;

    BYTE bySkillUseType = SkillAttribute[bySkillType].SkillUseType;
    int Skill_Icon = SkillAttribute[bySkillType].Magic_Icon;

    if (!gSkillManager.AreSkillRequirementsFulfilled(bySkillType))
    {
        bCantSkill = true;
    }

    if (IsCanBCSkill(bySkillType) == false)
    {
        bCantSkill = true;
    }
    if (g_isCharacterBuff((&Hero->Object), eBuff_AddSkill) && bySkillUseType == SKILL_USE_TYPE_BRAND)
    {
        bCantSkill = true;
    }
    auto isSittingOnPet = (Hero->Helper.Type == MODEL_HORN_OF_UNIRIA || Hero->Helper.Type == MODEL_HORN_OF_DINORANT || Hero->Helper.Type == MODEL_HORN_OF_FENRIR);
    if (bySkillType == AT_SKILL_IMPALE && !isSittingOnPet)
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_IMPALE && isSittingOnPet)
    {
        int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
        int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
        if ((iTypeL < ITEM_SPEAR || iTypeL >= ITEM_BOW) && (iTypeR < ITEM_SPEAR || iTypeR >= ITEM_BOW))
        {
            bCantSkill = true;
        }
    }

    if (isSittingOnPet
        && ((bySkillType >= AT_SKILL_BLOCKING && bySkillType <= AT_SKILL_SLASH)
            || bySkillType == AT_SKILL_FALLING_SLASH_STR
            || bySkillType == AT_SKILL_LUNGE_STR
            || bySkillType == AT_SKILL_CYCLONE_STR
            || bySkillType == AT_SKILL_CYCLONE_STR_MG
            || bySkillType == AT_SKILL_SLASH_STR
            ))
    {
        bCantSkill = true;
    }

    if ((bySkillType == AT_SKILL_POWER_SLASH || bySkillType == AT_SKILL_POWER_SLASH_STR)
        && isSittingOnPet)
    {
        bCantSkill = true;
    }

    int iEnergy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;

    if (g_csItemOption.IsDisableSkill(bySkillType, iEnergy))
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_PARTY_TELEPORT && PartyNumber <= 0)
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_PARTY_TELEPORT && (IsDoppelGanger1() || IsDoppelGanger2() || IsDoppelGanger3() || IsDoppelGanger4()))
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_EARTHSHAKE || bySkillType == AT_SKILL_EARTHSHAKE_STR || bySkillType == AT_SKILL_EARTHSHAKE_MASTERY)
    {
        BYTE byDarkHorseLife = 0;
        byDarkHorseLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;
        if (byDarkHorseLife == 0 || Hero->Helper.Type != MODEL_DARK_HORSE_ITEM)
        {
            bCantSkill = true;
        }
    }
#ifdef PJH_FIX_SPRIT
    /*박종훈*/
    if (bySkillType >= AT_PET_COMMAND_DEFAULT && bySkillType < AT_PET_COMMAND_END)
    {
        int iCharisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
        PET_INFO PetInfo;
        giPetManager::GetPetInfo(PetInfo, 421 - PET_TYPE_DARK_SPIRIT);
        int RequireCharisma = (185 + (PetInfo.m_wLevel * 15));
        if (RequireCharisma > iCharisma)
        {
            bCantSkill = true;
        }
    }
#endif //PJH_FIX_SPRIT
    if ((bySkillType == AT_SKILL_INFINITY_ARROW)
        || (bySkillType == AT_SKILL_INFINITY_ARROW_STR)
        || (bySkillType == AT_SKILL_EXPANSION_OF_WIZARDRY)
        || (bySkillType == AT_SKILL_EXPANSION_OF_WIZARDRY_STR)
        || (bySkillType == AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY)
        )
    {
        if (g_csItemOption.IsDisableSkill(bySkillType, iEnergy))
        {
            bCantSkill = true;
        }
        if ((g_isCharacterBuff((&Hero->Object), eBuff_InfinityArrow)) || (g_isCharacterBuff((&Hero->Object), eBuff_SwellOfMagicPower)))
        {
            bCantSkill = true;
        }
    }

    if (bySkillType == AT_SKILL_FIRE_SLASH || bySkillType == AT_SKILL_FIRE_SLASH_STR)
    {
        WORD Strength;
        const WORD wRequireStrength = 596;
        Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
        if (Strength < wRequireStrength)
        {
            bCantSkill = true;
        }
        int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
        int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

        if (!(iTypeR != -1 && (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) && (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX)))
        {
            bCantSkill = true;
        }
    }

    switch (bySkillType)
    {
        //case AT_SKILL_PIERCING:
    case AT_SKILL_ICE_ARROW:
    case AT_SKILL_ICE_ARROW_STR:
    {
        WORD  Dexterity;
        const WORD wRequireDexterity = 646;
        Dexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
        if (Dexterity < wRequireDexterity)
        {
            bCantSkill = true;
        }
    }break;
    }

    if (bySkillType == AT_SKILL_TWISTING_SLASH
        || bySkillType == AT_SKILL_TWISTING_SLASH_STR
        || bySkillType == AT_SKILL_TWISTING_SLASH_STR_MG
        || bySkillType == AT_SKILL_TWISTING_SLASH_MASTERY
        || bySkillType == AT_SKILL_RAGEFUL_BLOW
        || bySkillType == AT_SKILL_RAGEFUL_BLOW_STR
        || bySkillType == AT_SKILL_RAGEFUL_BLOW_MASTERY
        || bySkillType == AT_SKILL_DEATHSTAB
        || bySkillType == AT_SKILL_DEATHSTAB_STR
        )
    {
        int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
        int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

        if (!(iTypeR != -1 && (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) && (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX)))
        {
            bCantSkill = true;
        }
    }

    if (gMapManager.InChaosCastle() == true)
    {
        if (bySkillType == AT_SKILL_EARTHSHAKE
            || bySkillType == AT_SKILL_EARTHSHAKE_STR
            || bySkillType == AT_SKILL_EARTHSHAKE_MASTERY
            || bySkillType == AT_SKILL_RIDER
            || (bySkillType >= AT_PET_COMMAND_DEFAULT && bySkillType <= AT_PET_COMMAND_TARGET)
            )
        {
            bCantSkill = true;
        }
    }
    else
    {
        if (bySkillType == AT_SKILL_EARTHSHAKE
            || bySkillType == AT_SKILL_EARTHSHAKE_STR
            || bySkillType == AT_SKILL_EARTHSHAKE_MASTERY)
        {
            BYTE byDarkHorseLife = 0;
            byDarkHorseLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;
            if (byDarkHorseLife == 0)
            {
                bCantSkill = true;
            }
        }
    }

    int iCharisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;

    if (g_csItemOption.IsDisableSkill(bySkillType, iEnergy, iCharisma))
    {
        bCantSkill = true;
    }

    if (!g_CMonkSystem.IsSwordformGlovesUseSkill(bySkillType))
    {
        bCantSkill = true;
    }
    if (g_CMonkSystem.IsRideNotUseSkill(bySkillType, Hero->Helper.Type))
    {
        bCantSkill = true;
    }

    ITEM* pLeftRing = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
    ITEM* pRightRing = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];

    if (g_CMonkSystem.IsChangeringNotUseSkill(pLeftRing->Type, pRightRing->Type, pLeftRing->Level, pRightRing->Level)
        && (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER))
    {
        bCantSkill = true;
    }

    float fU, fV;
    int iKindofSkill = 0;

    if (!g_csItemOption.IsNonWeaponSkillOrIsSkillEquipped(bySkillType))
    {
        bCantSkill = true;
    }

    if (bySkillType >= AT_PET_COMMAND_DEFAULT && bySkillType <= AT_PET_COMMAND_END)
    {
        fU = ((bySkillType - AT_PET_COMMAND_DEFAULT) % 8) * width / 256.f;
        fV = ((bySkillType - AT_PET_COMMAND_DEFAULT) / 8) * height / 256.f;
        iKindofSkill = KOS_COMMAND;
    }
    else if (bySkillType == AT_SKILL_PLASMA_STORM_FENRIR)
    {
        fU = 4 * width / 256.f;
        fV = 0.f;
        iKindofSkill = KOS_COMMAND;
    }
    else if ((bySkillType >= AT_SKILL_ALICE_DRAINLIFE && bySkillType <= AT_SKILL_ALICE_THORNS))
    {
        fU = ((bySkillType - AT_SKILL_ALICE_DRAINLIFE) % 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType >= AT_SKILL_ALICE_SLEEP && bySkillType <= AT_SKILL_ALICE_BLIND)
    {
        fU = ((bySkillType - AT_SKILL_ALICE_SLEEP + 4) % 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_ALICE_BERSERKER)
    {
        fU = 10 * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType >= AT_SKILL_ALICE_WEAKNESS && bySkillType <= AT_SKILL_ALICE_ENERVATION)
    {
        fU = (bySkillType - AT_SKILL_ALICE_WEAKNESS + 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType >= AT_SKILL_SUMMON_EXPLOSION && bySkillType <= AT_SKILL_SUMMON_REQUIEM)
    {
        fU = ((bySkillType - AT_SKILL_SUMMON_EXPLOSION + 6) % 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_SUMMON_POLLUTION)
    {
        fU = 11 * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_STRIKE_OF_DESTRUCTION)
    {
        fU = 7 * width / 256.f;
        fV = 2 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_GAOTIC)
    {
        fU = 3 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_RECOVER)
    {
        fU = 9 * width / 256.f;
        fV = 2 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_MULTI_SHOT)
    {
        if (gCharacterManager.GetEquipedBowType_Skill() == BOWTYPE_NONE)
        {
            bCantSkill = true;
        }

        fU = 0 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_FLAME_STRIKE)
    {
        int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
        int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

        if (!(iTypeR != -1 && (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) && (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX)))
        {
            bCantSkill = true;
        }

        fU = 1 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_GIGANTIC_STORM)
    {
        fU = 2 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_LIGHTNING_SHOCK)
    {
        fU = 2 * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_EXPANSION_OF_WIZARDRY)
    {
        fU = 8 * width / 256.f;
        fV = 2 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillUseType == 4)
    {
        fU = (width / 256.f) * (Skill_Icon % 12);
        fV = (height / 256.f) * ((Skill_Icon / 12) + 4);
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType >= AT_SKILL_KILLING_BLOW)
    {
        fU = ((bySkillType - AT_SKILL_KILLING_BLOW) % 12) * width / 256.f;
        fV = ((bySkillType - AT_SKILL_KILLING_BLOW) / 12) * height / 256.f;
        iKindofSkill = KOS_SKILL3;
    }
    else if (bySkillType >= AT_SKILL_SPIRAL_SLASH)
    {
        fU = ((bySkillType - AT_SKILL_SPIRAL_SLASH) % 8) * width / 256.f;
        fV = ((bySkillType - AT_SKILL_SPIRAL_SLASH) / 8) * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else
    {
        fU = ((bySkillType - 1) % 8) * width / 256.f;
        fV = ((bySkillType - 1) / 8) * height / 256.f;
        iKindofSkill = KOS_SKILL1;
    }
    int iSkillIndex = 0;
    switch (iKindofSkill)
    {
    case KOS_COMMAND:
    {
        iSkillIndex = IMAGE_COMMAND;
    }break;
    case KOS_SKILL1:
    {
        iSkillIndex = IMAGE_SKILL1;
    }break;
    case KOS_SKILL2:
    {
        iSkillIndex = IMAGE_SKILL2;
    }break;
    case KOS_SKILL3:
    {
        iSkillIndex = IMAGE_SKILL3;
    }break;
    }

    if (bySkillType >= AT_SKILL_MASTER_BEGIN)
    {
        if (bCantSkill)
        {
            RenderImage(BITMAP_INTERFACE_MASTER_BEGIN + 3, x, y, width, height, (20.f / 512.f) * (Skill_Icon % 25), ((28.f / 512.f) * ((Skill_Icon / 25))), 20.f / 512.f, 28.f / 512.f);
        }
        else
        {
            RenderImage(BITMAP_INTERFACE_MASTER_BEGIN + 2, x, y, width, height, (20.f / 512.f)* (Skill_Icon % 25), ((28.f / 512.f)* ((Skill_Icon / 25))), 20.f / 512.f, 28.f / 512.f);
        }
    }
    else
    {
        if (bCantSkill == true)
        {
            iSkillIndex += 6;
        }

        if (iSkillIndex != 0)
        {
            RenderBitmap(iSkillIndex, x, y, width, height, fU, fV, width / 256.f, height / 256.f);
        }
    }

    int iHotKey = -1;
    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        if (m_iHotKeySkillType[i] == iIndex)
        {
            iHotKey = i;
            break;
        }
    }

    if (iHotKey != -1)
    {
        glColor3f(1.f, 0.9f, 0.8f);
        SEASON3B::RenderNumber(x + 20, y + 20, iHotKey);
        glColor3f(1.f, 1.f, 1.f);
    }

    if ((bySkillType == AT_SKILL_CHAIN_DRIVE
        || bySkillType == AT_SKILL_CHAIN_DRIVE_STR
        || bySkillType == AT_SKILL_DRAGON_KICK
        || bySkillType == AT_SKILL_DRAGON_ROAR
        || bySkillType == AT_SKILL_DRAGON_ROAR_STR) && (bCantSkill))
        return;

    if ((bySkillType != AT_SKILL_INFINITY_ARROW)
        && (bySkillType != AT_SKILL_INFINITY_ARROW_STR)
        && (bySkillType != AT_SKILL_EXPANSION_OF_WIZARDRY)
        && (bySkillType != AT_SKILL_EXPANSION_OF_WIZARDRY_STR)
        && (bySkillType != AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY)
        )
    {
        RenderSkillDelay(iIndex, x, y, width, height);
    }
}

void SEASON3B::CNewUISkillList::RenderSkillDelay(int iIndex, float x, float y, float width, float height)
{
    int iSkillDelay = CharacterAttribute->SkillDelay[iIndex];
    if (iSkillDelay > 0)
    {
        int iSkillType = CharacterAttribute->Skill[iIndex];

        if (iSkillType == AT_SKILL_PLASMA_STORM_FENRIR)
        {
            if (!CheckAttack())
            {
                return;
            }
        }

        int iSkillMaxDelay = SkillAttribute[iSkillType].Delay;

        auto fPersent = (float)(iSkillDelay / (float)iSkillMaxDelay);

        EnableAlphaTest();
        glColor4f(1.f, 0.5f, 0.5f, 0.5f);
        float fdeltaH = height * fPersent;
        RenderColor(x, y + height - fdeltaH, width, fdeltaH);
        EndRenderColor();
    }
}

bool SEASON3B::CNewUISkillList::IsSkillListUp()
{
    return m_bHotKeySkillListUp;
}

void SEASON3B::CNewUISkillList::ResetMouseLButton()
{
    MouseLButton = false;
    MouseLButtonPop = false;
    MouseLButtonPush = false;
}

void SEASON3B::CNewUISkillList::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
    if (pClass)
    {
        auto* pSkillList = (CNewUISkillList*)(pClass);
        pSkillList->RenderSkillInfo();
    }
}

void SEASON3B::CNewUIMainFrameWindow::SetPreExp_Wide(__int64 dwPreExp)
{
    m_loPreExp = dwPreExp;
}

void SEASON3B::CNewUIMainFrameWindow::SetGetExp_Wide(__int64 dwGetExp)
{
    m_loGetExp = dwGetExp;

    if (m_loGetExp > 0)
    {
        m_bExpEffect = true;
        m_dwExpEffectTime = timeGetTime();
    }
}

void SEASON3B::CNewUIMainFrameWindow::SetPreExp(DWORD dwPreExp)
{
    m_dwPreExp = dwPreExp;
}

void SEASON3B::CNewUIMainFrameWindow::SetGetExp(DWORD dwGetExp)
{
    m_dwGetExp = dwGetExp;

    if (m_dwGetExp > 0)
    {
        m_bExpEffect = true;
        m_dwExpEffectTime = timeGetTime();
    }
}

void SEASON3B::CNewUIMainFrameWindow::SetBtnState(int iBtnType, bool bStateDown)
{
    switch (iBtnType)
    {
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
    case MAINFRAME_BTN_PARTCHARGE:
    {
        if (bStateDown)
        {
            m_BtnCShop.UnRegisterButtonState();
            m_BtnCShop.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_CSHOP, 2);
            m_BtnCShop.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_CSHOP, 3);
            m_BtnCShop.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_CSHOP, 2);
            m_BtnCShop.ChangeImgIndex(IMAGE_MENU_BTN_CSHOP, 2);
        }
        else
        {
            m_BtnCShop.UnRegisterButtonState();
            m_BtnCShop.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_CSHOP, 0);
            m_BtnCShop.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_CSHOP, 1);
            m_BtnCShop.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_CSHOP, 2);
            m_BtnCShop.ChangeImgIndex(IMAGE_MENU_BTN_CSHOP, 0);
        }
    }
    break;
#endif //defined defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
    case MAINFRAME_BTN_CHAINFO:
    {
        if (bStateDown)
        {
            m_BtnChaInfo.UnRegisterButtonState();
            m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_CHAINFO, 2);
            m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_CHAINFO, 3);
            m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_CHAINFO, 2);
            m_BtnChaInfo.ChangeImgIndex(IMAGE_MENU_BTN_CHAINFO, 2);
        }
        else
        {
            m_BtnChaInfo.UnRegisterButtonState();
            m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_CHAINFO, 0);
            m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_CHAINFO, 1);
            m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_CHAINFO, 2);
            m_BtnChaInfo.ChangeImgIndex(IMAGE_MENU_BTN_CHAINFO, 0);
        }
    }
    break;
    case MAINFRAME_BTN_MYINVEN:
    {
        if (bStateDown)
        {
            m_BtnMyInven.UnRegisterButtonState();
            m_BtnMyInven.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_MYINVEN, 2);
            m_BtnMyInven.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_MYINVEN, 3);
            m_BtnMyInven.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_MYINVEN, 2);
            m_BtnMyInven.ChangeImgIndex(IMAGE_MENU_BTN_MYINVEN, 2);
        }
        else
        {
            m_BtnMyInven.UnRegisterButtonState();
            m_BtnMyInven.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_MYINVEN, 0);
            m_BtnMyInven.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_MYINVEN, 1);
            m_BtnMyInven.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_MYINVEN, 2);
            m_BtnMyInven.ChangeImgIndex(IMAGE_MENU_BTN_MYINVEN, 0);
        }
    }
    break;
    case MAINFRAME_BTN_FRIEND:
    {
        if (bStateDown)
        {
            m_BtnFriend.UnRegisterButtonState();
            m_BtnFriend.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_FRIEND, 2);
            m_BtnFriend.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_FRIEND, 3);
            m_BtnFriend.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_FRIEND, 2);
            m_BtnFriend.ChangeImgIndex(IMAGE_MENU_BTN_FRIEND, 2);
        }
        else
        {
            m_BtnFriend.UnRegisterButtonState();
            m_BtnFriend.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_FRIEND, 0);
            m_BtnFriend.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_FRIEND, 1);
            m_BtnFriend.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_FRIEND, 2);
            m_BtnFriend.ChangeImgIndex(IMAGE_MENU_BTN_FRIEND, 0);
        }
    }
    break;
    case MAINFRAME_BTN_WINDOW:
    {
        if (bStateDown)
        {
            m_BtnWindow.UnRegisterButtonState();
            m_BtnWindow.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_WINDOW, 2);
            m_BtnWindow.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_WINDOW, 3);
            m_BtnWindow.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_WINDOW, 2);
            m_BtnWindow.ChangeImgIndex(IMAGE_MENU_BTN_WINDOW, 2);
        }
        else
        {
            m_BtnWindow.UnRegisterButtonState();
            m_BtnWindow.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_WINDOW, 0);
            m_BtnWindow.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_WINDOW, 1);
            m_BtnWindow.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_WINDOW, 2);
            m_BtnWindow.ChangeImgIndex(IMAGE_MENU_BTN_WINDOW, 0);
        }
    }
    break;
    }
}