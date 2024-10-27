#include "stdafx.h"
#include "NewUISystem.h"
#include "NewUIMuHelper.h"
#include "CharacterManager.h"

SEASON3B::CNewUIMuHelper::CNewUIMuHelper()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_ButtonList.clear();
    m_iNumCurOpenTab = 0;
}

SEASON3B::CNewUIMuHelper::~CNewUIMuHelper()
{
    Release();
}

bool SEASON3B::CNewUIMuHelper::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MUHELPER, this);

    SetPos(x, y);

    LoadImages();

    InitButtons();

    InitCheckBox();

    InitImage();

    InitText();

    Show(false);

    return true;
}

void SEASON3B::CNewUIMuHelper::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUIMuHelper::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

void SEASON3B::CNewUIMuHelper::InitButtons()
{
    std::list<std::wstring> ltext;
    ltext.push_back(GlobalText[3500]);
    ltext.push_back(GlobalText[3501]);
    ltext.push_back(GlobalText[3590]);

    m_TabBtn.CreateRadioGroup(3, IMAGE_WINDOW_TAB_BTN, TRUE);
    m_TabBtn.ChangeRadioText(ltext);
    m_TabBtn.ChangeRadioButtonInfo(true, m_Pos.x + 10.f, m_Pos.y + 48.f, 56, 22);
    m_TabBtn.ChangeFrame(m_iNumCurOpenTab);

    InsertButton(IMAGE_CHAINFO_BTN_STAT, m_Pos.x + 56, m_Pos.y + 78, 16, 15, 0, 0, 0, 0, L"", L"", 0, 0);
    InsertButton(IMAGE_MACROUI_HELPER_RAGEMINUS, m_Pos.x + 56, m_Pos.y + 97, 16, 15, 0, 0, 0, 0, L"", L"", 1, 0);
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 191, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", 2, 0); //-- skill 2
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 243, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", 3, 0); //-- skill 3
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 84, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", 4, 0); //-- Buff
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 79, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", 5, 0); //-- potion
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 84, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", 6, 0); //-- potion
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 17, m_Pos.y + 234, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", 7, 0); //-- potion
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 17, m_Pos.y + 234, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", 8, 0); //-- potion
    InsertButton(IMAGE_CHAINFO_BTN_STAT, m_Pos.x + 56, m_Pos.y + 78, 16, 15, 0, 0, 0, 0, L"", L"", 9, 1);
    InsertButton(IMAGE_MACROUI_HELPER_RAGEMINUS, m_Pos.x + 56, m_Pos.y + 97, 16, 15, 0, 0, 0, 0, L"", L"", 10, 1);
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 208, 38, 24, 1, 0, 1, 1, GlobalText[3505], L"", 11, 1); //-- Buff
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 309, 38, 24, 1, 0, 1, 1, GlobalText[3506], L"", 12, 1); //-- Buff
    //--
    InsertButton(IMAGE_IGS_BUTTON, m_Pos.x + 120, m_Pos.y + 388, 52, 26, 1, 0, 1, 1, GlobalText[3503], L"", 13, -1);
    InsertButton(IMAGE_IGS_BUTTON, m_Pos.x + 65, m_Pos.y + 388, 52, 26, 1, 0, 1, 1, GlobalText[3504], L"", 14, -1);
    InsertButton(IMAGE_BASE_WINDOW_BTN_EXIT, m_Pos.x + 20, m_Pos.y + 388, 36, 29, 0, 0, 0, 0, L"", GlobalText[388], 15, -1);

    RegisterBtnCharacter(0xFF, 0);
    RegisterBtnCharacter(0xFF, 1);
    RegisterBtnCharacter(0xFF, 2);
    RegisterBtnCharacter(0xFF, 9);
    RegisterBtnCharacter(0xFF, 10);
    RegisterBtnCharacter(0xFF, 11);
    RegisterBtnCharacter(0xFF, 12);
    RegisterBtnCharacter(0xFF, 13);
    RegisterBtnCharacter(0xFF, 15);
    RegisterBtnCharacter(0xFF, 14);

    RegisterBtnCharacter(Dark_Knight, 3);
    RegisterBtnCharacter(Dark_Knight, 6);

    RegisterBtnCharacter(Dark_Wizard, 3);
    RegisterBtnCharacter(Dark_Wizard, 6);
    RegisterBtnCharacter(Dark_Wizard, 7);

    RegisterBtnCharacter(Magic_Gladiator, 3);
    RegisterBtnCharacter(Magic_Gladiator, 6);
    RegisterBtnCharacter(Dark_Lord, 6);

    RegisterBtnCharacter(Rage_Fighter, 3);
    RegisterBtnCharacter(Rage_Fighter, 6);

    RegisterBtnCharacter(Fairy_Elf, 3);
    RegisterBtnCharacter(Fairy_Elf, 4);
    RegisterBtnCharacter(Fairy_Elf, 8);

    RegisterBtnCharacter(Summoner, 3);
    RegisterBtnCharacter(Summoner, 5);
}

void SEASON3B::CNewUIMuHelper::InitCheckBox()
{
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 79, m_Pos.y + 80, 15, 15, 0, GlobalText[3507], 0, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 122, 15, 15, 0, GlobalText[3508], 1, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 137, 15, 15, 0, GlobalText[3509], 2, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 94, m_Pos.y + 174, 15, 15, 0, GlobalText[3510], 3, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 94, m_Pos.y + 191, 15, 15, 0, GlobalText[3511], 4, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 94, m_Pos.y + 226, 15, 15, 0, GlobalText[3510], 5, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 94, m_Pos.y + 243, 15, 15, 0, GlobalText[3511], 6, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 226, 15, 15, 0, GlobalText[3512], 7, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 25, m_Pos.y + 276, 15, 15, 0, GlobalText[3513], 8, 0);

    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 60, m_Pos.y + 218, 15, 15, 0, GlobalText[3514], 9, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 15, m_Pos.y + 218, 15, 15, 0, GlobalText[3515], 10, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 79, m_Pos.y + 97, 15, 15, 0, GlobalText[3516], 11, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 79, m_Pos.y + 97, 15, 15, 0, GlobalText[3517], 12, 0);

    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 79, m_Pos.y + 80, 15, 15, 0, GlobalText[3518], 13, 1);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 17, m_Pos.y + 125, 15, 15, 0, GlobalText[3519], 14, 1);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 17, m_Pos.y + 152, 15, 15, 0, GlobalText[3520], 15, 1);

    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 22, m_Pos.y + 170, 15, 15, 0, GlobalText[3521], 16, 1);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 85, m_Pos.y + 170, 15, 15, 0, GlobalText[3522], 17, 1);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 22, m_Pos.y + 185, 15, 15, 0, GlobalText[3523], 18, 1);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 85, m_Pos.y + 185, 15, 15, 0, GlobalText[3524], 19, 1);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 22, m_Pos.y + 200, 15, 15, 0, GlobalText[3525], 20, 1);
    //--
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 80, 15, 15, 0, GlobalText[3591], 21, 2);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 97, 15, 15, 0, GlobalText[3592], 23, 2);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 125, 15, 15, 0, GlobalText[3594], 22, 2);

    RegisterBoxCharacter(0xFF, 0);
    RegisterBoxCharacter(0xFF, 1);
    RegisterBoxCharacter(0xFF, 2);
    RegisterBoxCharacter(0xFF, 3);
    RegisterBoxCharacter(0xFF, 4);
    RegisterBoxCharacter(0xFF, 8);
    RegisterBoxCharacter(0xFF, 13);
    RegisterBoxCharacter(0xFF, 14);
    RegisterBoxCharacter(0xFF, 15);
    RegisterBoxCharacter(0xFF, 16);
    RegisterBoxCharacter(0xFF, 17);
    RegisterBoxCharacter(0xFF, 19);
    RegisterBoxCharacter(0xFF, 18);
    RegisterBoxCharacter(0xFF, 20);
    RegisterBoxCharacter(0xFF, 21);
    RegisterBoxCharacter(0xFF, 23);
    RegisterBoxCharacter(0xFF, 22);

    RegisterBoxCharacter(Dark_Knight, 5);
    RegisterBoxCharacter(Dark_Knight, 6);
    RegisterBoxCharacter(Dark_Knight, 7);

    RegisterBoxCharacter(Dark_Wizard, 5);
    RegisterBoxCharacter(Dark_Wizard, 6);
    RegisterBoxCharacter(Dark_Wizard, 10);

    RegisterBoxCharacter(Magic_Gladiator, 5);
    RegisterBoxCharacter(Magic_Gladiator, 6);

    RegisterBoxCharacter(Dark_Lord, 9);

    RegisterBoxCharacter(Fairy_Elf, 11);
    RegisterBoxCharacter(Fairy_Elf, 5);
    RegisterBoxCharacter(Fairy_Elf, 6);
    RegisterBoxCharacter(Fairy_Elf, 10);

    RegisterBoxCharacter(Summoner, 5);
    RegisterBoxCharacter(Summoner, 6);
    RegisterBoxCharacter(Summoner, 12);

    RegisterBoxCharacter(Rage_Fighter, 5);
    RegisterBoxCharacter(Rage_Fighter, 6);
}

void SEASON3B::CNewUIMuHelper::InitImage()
{
    InsertTexture(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 17, m_Pos.y + 171, 32, 38, 0, 0);
    InsertTexture(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 61, m_Pos.y + 171, 32, 38, 1, 0);
    InsertTexture(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 61, m_Pos.y + 222, 32, 38, 2, 0);
    InsertTexture(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 21, m_Pos.y + 293, 32, 38, 3, 0);
    InsertTexture(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 55, m_Pos.y + 293, 32, 38, 4, 0);
    InsertTexture(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 89, m_Pos.y + 293, 32, 38, 5, 0);

    InsertTexture(IMAGE_MACROUI_HELPER_INPUTNUMBER, m_Pos.x + 135, m_Pos.y + 137, 28, 15, 6, 0);
    InsertTexture(IMAGE_MACROUI_HELPER_INPUTNUMBER, m_Pos.x + 135, m_Pos.y + 174, 28, 15, 7, 0);
    InsertTexture(IMAGE_MACROUI_HELPER_INPUTNUMBER, m_Pos.x + 135, m_Pos.y + 226, 28, 15, 8, 0);
    InsertTexture(IMAGE_MACROUI_HELPER_INPUTSTRING, m_Pos.x + 34, m_Pos.y + 216, 93, 15, 9, 1);

    RegisterTextureCharacter(0xFF, 0);
    RegisterTextureCharacter(0xFF, 1);
    RegisterTextureCharacter(0xFF, 3);
    RegisterTextureCharacter(0xFF, 4);
    RegisterTextureCharacter(0xFF, 5);
    RegisterTextureCharacter(0xFF, 6);
    RegisterTextureCharacter(0xFF, 7);
    RegisterTextureCharacter(0xFF, 9);

    RegisterTextureCharacter(Dark_Knight, 2);
    RegisterTextureCharacter(Dark_Knight, 8);
    RegisterTextureCharacter(Dark_Wizard, 2);
    RegisterTextureCharacter(Dark_Wizard, 8);
    RegisterTextureCharacter(Fairy_Elf, 2);
    RegisterTextureCharacter(Fairy_Elf, 8);
    RegisterTextureCharacter(Magic_Gladiator, 2);
    RegisterTextureCharacter(Magic_Gladiator, 8);
    RegisterTextureCharacter(Summoner, 2);
    RegisterTextureCharacter(Summoner, 8);
    RegisterTextureCharacter(Rage_Fighter, 2);
    RegisterTextureCharacter(Rage_Fighter, 8);
}

void SEASON3B::CNewUIMuHelper::InitText()
{
    InsertText(m_Pos.x + 18, m_Pos.y + 78, GlobalText[3526], 1, 0);
    InsertText(m_Pos.x + 18, m_Pos.y + 83, L"________", 2, 0);
    InsertText(m_Pos.x + 113, m_Pos.y + 141, GlobalText[3527], 3, 0);
    InsertText(m_Pos.x + 162, m_Pos.y + 141, GlobalText[3528], 4, 0);
    InsertText(m_Pos.x + 18, m_Pos.y + 160, GlobalText[3529], 5, 0);
    InsertText(m_Pos.x + 59, m_Pos.y + 160, GlobalText[3530], 7, 0);
    InsertText(m_Pos.x + 162, m_Pos.y + 178, GlobalText[3528], 8, 0);
    InsertText(m_Pos.x + 59, m_Pos.y + 212, GlobalText[3531], 9, 0);

    InsertText(m_Pos.x + 162, m_Pos.y + 230, GlobalText[3528], 10, 0);
    InsertText(m_Pos.x + 18, m_Pos.y + 78, GlobalText[3532], 11, 1);
    InsertText(m_Pos.x + 18, m_Pos.y + 83, L"________", 12, 1);

    RegisterTextCharacter(0xFF, 1);
    RegisterTextCharacter(0xFF, 2);
    RegisterTextCharacter(0xFF, 3);
    RegisterTextCharacter(0xFF, 4);
    RegisterTextCharacter(0xFF, 5);
    RegisterTextCharacter(0xFF, 7);
    RegisterTextCharacter(0xFF, 8);
    RegisterTextCharacter(0xFF, 11);
    RegisterTextCharacter(0xFF, 12);

    RegisterTextCharacter(Dark_Knight, 9);
    RegisterTextCharacter(Dark_Knight, 10);
    RegisterTextCharacter(Dark_Wizard, 9);
    RegisterTextCharacter(Dark_Wizard, 10);
    RegisterTextCharacter(Fairy_Elf, 9);
    RegisterTextCharacter(Fairy_Elf, 10);
    RegisterTextCharacter(Magic_Gladiator, 9);
    RegisterTextCharacter(Magic_Gladiator, 10);
    RegisterTextCharacter(Summoner, 9);
    RegisterTextCharacter(Summoner, 10);
    RegisterTextCharacter(Rage_Fighter, 9);
    RegisterTextCharacter(Rage_Fighter, 10);
}

bool SEASON3B::CNewUIMuHelper::Update()
{
    if (IsVisible())
    {
        int iNumCurOpenTab = m_TabBtn.UpdateMouseEvent();

        if (iNumCurOpenTab == RADIOGROUPEVENT_NONE)
            return true;

        m_iNumCurOpenTab = iNumCurOpenTab;
    }
    return true;
}

bool SEASON3B::CNewUIMuHelper::UpdateMouseEvent()
{
    // Ignore events outside MU Helper window
    if (!CheckMouseIn(m_Pos.x, m_Pos.y, WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        return true;
    }

    int buttonId = UpdateMouseBtnList();
    if (buttonId != -1)
    {
        g_ConsoleDebug->Write(MCD_NORMAL, L"Clicked button [%d]", buttonId);

        // Exit Button
        if (buttonId == 15)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_MUHELPER);
        }

        return false;
    }

    int checkboxId = UpdateMouseBoxList();
    if (checkboxId != -1)
    {
        g_ConsoleDebug->Write(MCD_NORMAL, L"Clicked checkbox [%d]", checkboxId);
        return false;
    }

    int imgId = UpdateTextureList();
    if (imgId != -1 && IsRelease(VK_LBUTTON))
    {
        g_ConsoleDebug->Write(MCD_NORMAL, L"Clicked skill icon [%d]", imgId);
        return false;
    }

    return false;
}

bool SEASON3B::CNewUIMuHelper::UpdateKeyEvent()
{
    if (IsVisible())
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_MUHELPER);
            //PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }
    return true;
}

float SEASON3B::CNewUIMuHelper::GetLayerDepth()
{
    return 3.4;
}

float SEASON3B::CNewUIMuHelper::GetKeyEventOrder()
{
    return 3.4;
}

bool SEASON3B::CNewUIMuHelper::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    DWORD TextColor = g_pRenderText->GetTextColor();

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(0xFFFFFFFF);

    g_pRenderText->SetBgColor(0);

    RenderImage(IMAGE_BASE_WINDOW_BACK, m_Pos.x, m_Pos.y, float(WINDOW_WIDTH), float(WINDOW_HEIGHT));
    RenderImage(IMAGE_BASE_WINDOW_TOP, m_Pos.x, m_Pos.y, float(WINDOW_WIDTH), 64.f);
    RenderImage(IMAGE_BASE_WINDOW_LEFT, m_Pos.x, m_Pos.y + 64.f, 21.f, float(WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_BASE_WINDOW_RIGHT, m_Pos.x + float(WINDOW_WIDTH) - 21.f, m_Pos.y + 64.f, 21.f, float(WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_BASE_WINDOW_BOTTOM, m_Pos.x, m_Pos.y + float(WINDOW_HEIGHT) - 45.f, float(WINDOW_WIDTH), 45.f);

    g_pRenderText->SetFont(g_hFontBold);

    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 13, GlobalText[3536], 190, 0, RT3_SORT_CENTER);

    RenderBack(m_Pos.x + 12, m_Pos.y + 340, 165, 46);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + 347, GlobalText[3537], 0, 0, RT3_SORT_CENTER);

    g_pRenderText->SetTextColor(0xFF00B4FF);
    g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + 365, GlobalText[3538], 0, 0, RT3_SORT_CENTER);

    g_pRenderText->SetTextColor(TextColor);

    int sTapIndex = m_iNumCurOpenTab;

    m_TabBtn.Render();

    if (sTapIndex)
    {
        if (sTapIndex == 1)
        {
            RenderBack(m_Pos.x + 12, m_Pos.y + 73, 68, 50);
            RenderBack(m_Pos.x + 75, m_Pos.y + 73, 102, 50);
            RenderBack(m_Pos.x + 12, m_Pos.y + 120, 165, 30);
            RenderBack(m_Pos.x + 12, m_Pos.y + 147, 165, 195);
            RenderBack(m_Pos.x + 16, m_Pos.y + 235, 158, 74);

            RenderImage(BITMAP_INTERFACE_CRYWOLF_BEGIN + 34, m_Pos.x + 29, m_Pos.y + 92, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f);
        }
        else
        {
            RenderBack(m_Pos.x + 12, m_Pos.y + 73, 165, 50);
            RenderBack(m_Pos.x + 12, m_Pos.y + 120, 165, 222);
        }
    }
    else
    {
        RenderBack(m_Pos.x + 12, m_Pos.y + 73, 68, 50);
        RenderBack(m_Pos.x + 75, m_Pos.y + 73, 102, 50);
        RenderBack(m_Pos.x + 12, m_Pos.y + 120, 165, 39);
        RenderBack(m_Pos.x + 12, m_Pos.y + 156, 165, 120);
        RenderBack(m_Pos.x + 12, m_Pos.y + 273, 165, 69);

        RenderImage(BITMAP_INTERFACE_CRYWOLF_BEGIN + 33, m_Pos.x + 29, m_Pos.y + 92, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f);
    }

    RenderBoxList();
    RenderTextureList();
    RenderTextList();
    RenderBtnList();
    DisableAlphaBlend();

    return true;
}

void SEASON3B::CNewUIMuHelper::RenderBack(int x, int y, int width, int height)
{
    EnableAlphaTest();
    glColor4f(0.0, 0.0, 0.0, 0.4f);
    RenderColor(x + 3.f, y + 2.f, width - 7.f, height - 7, 0.0, 0);
    EndRenderColor();

    RenderImage(IMAGE_TABLE_TOP_LEFT, x, y, 14.0, 14.0);
    RenderImage(IMAGE_TABLE_TOP_RIGHT, (x + width) - 14.f, y, 14.0, 14.0);
    RenderImage(IMAGE_TABLE_BOTTOM_LEFT, x, (y + height) - 14.f, 14.0, 14.0);
    RenderImage(IMAGE_TABLE_BOTTOM_RIGHT, (x + width) - 14.f, (y + height) - 14.f, 14.0, 14.0);
    RenderImage(IMAGE_TABLE_TOP_PIXEL, x + 6.f, y, (width - 12.f), 14.0);
    RenderImage(IMAGE_TABLE_RIGHT_PIXEL, (x + width) - 14.f, y + 6.f, 14.0, (height - 14.f));
    RenderImage(IMAGE_TABLE_BOTTOM_PIXEL, x + 6.f, (y + height) - 14.f, (width - 12.f), 14.0);
    RenderImage(IMAGE_TABLE_LEFT_PIXEL, x, (y + 6.f), 14.0, (height - 14.f));
}

void SEASON3B::CNewUIMuHelper::LoadImages()
{
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_RangeMinus.tga", IMAGE_MACROUI_HELPER_RAGEMINUS, GL_LINEAR, GL_CLAMP, 1, 0);
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_OptionButton.tga", IMAGE_MACROUI_HELPER_OPTIONBUTTON, GL_LINEAR, GL_CLAMP, 1, 0);
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_InputNumber.tga", IMAGE_MACROUI_HELPER_INPUTNUMBER, GL_LINEAR, GL_CLAMP, 1, 0);
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_InputString.tga", IMAGE_MACROUI_HELPER_INPUTSTRING, GL_LINEAR, GL_CLAMP, 1, 0);
    //--
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Bt03.tga", IMAGE_IGS_BUTTON, GL_LINEAR, GL_CLAMP, 1, 0);
}

void SEASON3B::CNewUIMuHelper::UnloadImages()
{
    DeleteBitmap(IMAGE_MACROUI_HELPER_RAGEMINUS);
    DeleteBitmap(IMAGE_MACROUI_HELPER_OPTIONBUTTON);
    DeleteBitmap(IMAGE_MACROUI_HELPER_INPUTNUMBER);
    DeleteBitmap(IMAGE_MACROUI_HELPER_INPUTSTRING);
    //--
    DeleteBitmap(IMAGE_IGS_BUTTON);
}

//===============================================================================================================
//===============================================================================================================

void SEASON3B::CNewUIMuHelper::RegisterButton(int Identifier, CButtonTap button)
{
    m_ButtonList.insert(std::pair<int, CButtonTap>(Identifier, button));
}

void SEASON3B::CNewUIMuHelper::RegisterBtnCharacter(BYTE class_character, int Identifier)
{
    auto li = m_ButtonList.find(Identifier);

    if (li != m_ButtonList.end())
    {
        CButtonTap* cBTN = &li->second;
        if (class_character >= 0 && class_character < MAX_CLASS)
        {
            cBTN->class_character[class_character] = TRUE;
        }
        else
        {
            memset(cBTN->class_character, 1, sizeof(cBTN->class_character));
        }
    }
}

void SEASON3B::CNewUIMuHelper::InsertButton(int imgindex, int x, int y, int sx, int sy, bool overflg, bool isimgwidth, bool bClickEffect, bool MoveTxt,std::wstring btname,std::wstring tooltiptext, int Identifier, int iNumTab)
{
    CButtonTap cBTN;
    auto* button = new CNewUIButton();

    button->ChangeButtonImgState(1, imgindex, overflg, isimgwidth, bClickEffect);
    button->ChangeButtonInfo(x, y, sx, sy);

    button->ChangeText(btname);
    button->ChangeToolTipText(tooltiptext, TRUE);

    if (MoveTxt)
    {
        button->MoveTextPos(0, -1);
    }

    cBTN.btn = button;
    cBTN.iNumTab = iNumTab;
    memset(cBTN.class_character, 0, sizeof(cBTN.class_character));

    RegisterButton(Identifier, cBTN);
}

void SEASON3B::CNewUIMuHelper::RenderBtnList()
{
    auto li = m_ButtonList.begin();

    for (; li != m_ButtonList.end(); li++)
    {
        CButtonTap* cBTN = &li->second;

        if ((cBTN->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cBTN->iNumTab == m_iNumCurOpenTab || cBTN->iNumTab == -1))
        {
            cBTN->btn->Render();
        }
    }
}

int SEASON3B::CNewUIMuHelper::UpdateMouseBtnList()
{
    int Identifier = -1;

    auto li = m_ButtonList.begin();

    for (; li != m_ButtonList.end(); li++)
    {
        CButtonTap* cBTN = &li->second;

        if ((cBTN->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cBTN->iNumTab == m_iNumCurOpenTab || cBTN->iNumTab == -1))
        {
            if (cBTN->btn->UpdateMouseEvent())
            {
                return li->first;
            }
        }
    }
    return Identifier;
}

//===============================================================================================================
//===============================================================================================================

void SEASON3B::CNewUIMuHelper::RegisterBoxCharacter(BYTE class_character, int Identifier)
{
    auto li = m_CheckBoxList.find(Identifier);

    if (li != m_CheckBoxList.end())
    {
        CheckBoxTap* cBOX = &li->second;

        if (class_character >= 0 && class_character < MAX_CLASS)
        {
            cBOX->class_character[class_character] = TRUE;
        }
        else
        {
            memset(cBOX->class_character, 1, sizeof(cBOX->class_character));
        }
    }
}

void SEASON3B::CNewUIMuHelper::RegisterCheckBox(int Identifier, CheckBoxTap button)
{
    m_CheckBoxList.insert(std::pair<int, CheckBoxTap>(Identifier, button));
}

void SEASON3B::CNewUIMuHelper::InsertCheckBox(int imgindex, int x, int y, int sx, int sy, bool overflg,std::wstring btname, int Identifier, int iNumTab)
{
    CheckBoxTap cBOX;

    auto* cbox = new CNewUICheckBox;

    cbox->CheckBoxImgState(imgindex);
    cbox->CheckBoxInfo(x, y, sx, sy);

    cbox->ChangeText(btname);
    cbox->RegisterBoxState(overflg);

    cBOX.box = cbox;
    cBOX.iNumTab = iNumTab;
    memset(cBOX.class_character, 0, sizeof(cBOX.class_character));

    RegisterCheckBox(Identifier, cBOX);
}

void SEASON3B::CNewUIMuHelper::RenderBoxList()
{
    auto li = m_CheckBoxList.begin();

    for (; li != m_CheckBoxList.end(); li++)
    {
        CheckBoxTap* cBOX = &li->second;

        if ((cBOX->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cBOX->iNumTab == m_iNumCurOpenTab || cBOX->iNumTab == -1))
        {
            cBOX->box->Render();
        }
    }
}

int SEASON3B::CNewUIMuHelper::UpdateMouseBoxList()
{
    int Identifier = -1;

    auto li = m_CheckBoxList.begin();

    for (; li != m_CheckBoxList.end(); li++)
    {
        CheckBoxTap* cBOX = &li->second;

        if ((cBOX->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cBOX->iNumTab == m_iNumCurOpenTab || cBOX->iNumTab == -1))
        {
            if (cBOX->box->UpdateMouseEvent())
            {
                return li->first;
            }
        }
    }
    return Identifier;
}

//===============================================================================================================
//===============================================================================================================

void SEASON3B::CNewUIMuHelper::RenderTextureList()
{
    auto li = m_TextureList.begin();

    for (; li != m_TextureList.end(); li++)
    {
        cTexture* cImage = &li->second;

        if ((cImage->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cImage->iNumTab == m_iNumCurOpenTab || cImage->iNumTab == -1))
        {
            RenderImage(cImage->s_ImgIndex, cImage->m_Pos.x, cImage->m_Pos.y, cImage->m_Size.x, cImage->m_Size.y);
        }
    }
}

int SEASON3B::CNewUIMuHelper::UpdateTextureList()
{
    int Identifier = -1;

    auto li = m_TextureList.begin();

    for (; li != m_TextureList.end(); li++)
    {
        cTexture* cImage = &li->second;

        if ((cImage->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cImage->iNumTab == m_iNumCurOpenTab || cImage->iNumTab == -1))
        {
            if (CheckMouseIn(cImage->m_Pos.x, cImage->m_Pos.y, cImage->m_Size.x, cImage->m_Size.y))
            {
                return li->first;
            }
        }
    }

    return Identifier;
}

void SEASON3B::CNewUIMuHelper::RegisterTextureCharacter(BYTE class_character, int Identifier)
{
    auto li = m_TextureList.find(Identifier);

    if (li != m_TextureList.end())
    {
        cTexture* cImage = &li->second;

        if (class_character >= 0 && class_character < MAX_CLASS)
        {
            cImage->class_character[class_character] = TRUE;
        }
        else
        {
            memset(cImage->class_character, 1, sizeof(cImage->class_character));
        }
    }
}

void SEASON3B::CNewUIMuHelper::RegisterTexture(int Identifier, cTexture button)
{
    m_TextureList.insert(std::pair<int, cTexture>(Identifier, button));
}

void SEASON3B::CNewUIMuHelper::InsertTexture(int imgindex, int x, int y, int sx, int sy, int Identifier, int iNumTab)
{
    cTexture cImage;

    cImage.s_ImgIndex = imgindex;
    cImage.m_Pos.x = x;
    cImage.m_Pos.y = y;
    cImage.m_Size.x = sx;
    cImage.m_Size.y = sy;
    cImage.iNumTab = iNumTab;

    memset(cImage.class_character, 0, sizeof(cImage.class_character));

    RegisterTexture(Identifier, cImage);
}

//===============================================================================================================
//===============================================================================================================

void SEASON3B::CNewUIMuHelper::RenderTextList()
{
    auto li = m_TextNameList.begin();

    for (; li != m_TextNameList.end(); li++)
    {
        cTextName* cImage = &li->second;

        if ((cImage->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cImage->iNumTab == m_iNumCurOpenTab || cImage->iNumTab == -1))
        {
            g_pRenderText->RenderText(cImage->m_Pos.x, cImage->m_Pos.y, cImage->m_Name.c_str());
        }
    }
}

void SEASON3B::CNewUIMuHelper::RegisterTextCharacter(BYTE class_character, int Identifier)
{
    auto li = m_TextNameList.find(Identifier);

    if (li != m_TextNameList.end())
    {
        cTextName* cImage = &li->second;

        if (class_character >= 0 && class_character < MAX_CLASS)
        {
            cImage->class_character[class_character] = TRUE;
        }
        else
        {
            memset(cImage->class_character, 1, sizeof(cImage->class_character));
        }
    }
}

void SEASON3B::CNewUIMuHelper::RegisterTextur(int Identifier, cTextName button)
{
    m_TextNameList.insert(std::pair<int, cTextName>(Identifier, button));
}

void SEASON3B::CNewUIMuHelper::InsertText(int x, int y,std::wstring Name, int Identifier, int iNumTab)
{
    cTextName cText;

    cText.m_Pos.x = x;
    cText.m_Pos.y = y;
    cText.m_Name = Name;
    cText.iNumTab = iNumTab;

    memset(cText.class_character, 0, sizeof(cText.class_character));
    RegisterTextur(Identifier, cText);
}