//////////////////////////////////////////////////////////////////////
// NewUIQuickCommandWindow.cpp: implementation of the CQuickCommandWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/HUD/NewUIQuickCommandWindow.h"
#include "UI/Core/NewUISystem.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"

using namespace SEASON3B;
using namespace mu::ui::window;

// cppcheck-suppress uninitMemberVar
mu::ui::window::CQuickCommandWindow::CQuickCommandWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = 0;
    m_Pos.y = 0;

    m_iSelectedIndex = -1;
    m_iSelectedCharacterIndex = -1;
}

mu::ui::window::CQuickCommandWindow::~CQuickCommandWindow()
{
    Release();
}

bool mu::ui::window::CQuickCommandWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_QUICK_COMMAND, this);

    LoadImages();

    SetPos(x, y);

    Show(false);

    return true;
}

void mu::ui::window::CQuickCommandWindow::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CQuickCommandWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool mu::ui::window::CQuickCommandWindow::UpdateMouseEvent()
{
    if (m_iSelectedCharacterIndex < 0)
    {
        return true;
    }

    POINT pt = {m_Pos.x, m_Pos.y + 38};

    for (int i = 0; i < 5; ++i)
    {
        if (CheckMouseIn(pt.x, pt.y, 112, 19) == true)
        {
            m_iSelectedIndex = i;
            break;
        }

        pt.y += 20.f;
    }

    if (m_iSelectedIndex > -1 && mu::ui::window::IsRelease(VK_LBUTTON))
    {
        switch (m_iSelectedIndex)
        {
        case 0:
        {
            CHARACTER* pCha = &CharactersClient[m_iSelectedCharacterIndex];
            g_pCommandWindow->CommandTrade(pCha);
            CloseQuickCommand();

            return false;
        }
        break;
        case 1:
        {
            CHARACTER* pCha = &CharactersClient[m_iSelectedCharacterIndex];
            g_pCommandWindow->CommandPurchase(pCha);
            CloseQuickCommand();

            return false;
        }
        break;
        case 2:
        {
            CHARACTER* pCha = &CharactersClient[m_iSelectedCharacterIndex];
            g_pCommandWindow->CommandParty(pCha->Key);
            CloseQuickCommand();

            return false;
        }
        break;
        case 3:
        {
            g_pCommandWindow->CommandFollow(m_iSelectedCharacterIndex);
            CloseQuickCommand();

            return false;
        }
        break;
        case 4:
        {
            CHARACTER* pCha = &CharactersClient[m_iSelectedCharacterIndex];
            g_pCommandWindow->CommandDual(pCha);
            CloseQuickCommand();

            return false;
        }
        break;
        }
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y + 30, 112, 110) == false)
    {
        m_iSelectedIndex = -1;

        if (mu::ui::window::IsRelease(VK_LBUTTON))
        {
            CloseQuickCommand();
            return false;
        }
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, 112, 140))
    {
        return false;
    }

    return true;
}

bool mu::ui::window::CQuickCommandWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_QUICK_COMMAND) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            CloseQuickCommand();
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }

    return true;
}

bool mu::ui::window::CQuickCommandWindow::Update()
{
    if (m_iSelectedCharacterIndex >= 0)
    {
        CHARACTER* pCha = &CharactersClient[m_iSelectedCharacterIndex];

        if (wcscmp(pCha->ID, m_strID) != 0 || pCha->Object.Live == false || pCha->Object.Kind != KIND_PLAYER)
        {
            CloseQuickCommand();
        }

        float fPos_x = pCha->Object.Position[0] - Hero->Object.Position[0];
        float fPos_y = pCha->Object.Position[1] - Hero->Object.Position[1];
        float fDistance = sqrtf((fPos_x * fPos_x) + (fPos_y * fPos_y));

        if (fDistance > 300.f)
        {
            CloseQuickCommand();
        }
    }
    else
    {
        CloseQuickCommand();
    }

    return true;
}

bool mu::ui::window::CQuickCommandWindow::Render()
{
    EnableAlphaTest();

    if (m_iSelectedCharacterIndex < 0)
    {
        return true;
    }

    RenderFrame();
    RenderContents();
    RenderArrow();

    DisableAlphaBlend();

    return true;
}

void mu::ui::window::CQuickCommandWindow::RenderFrame()
{
    float x, y, width, height;

    x = m_Pos.x;
    y = m_Pos.y;
    width = 112.f;
    height = 140;

    RenderImage(IMAGE_QUICKCOMMAND_BACK, x, y, width, height);

    y = m_Pos.y;
    RenderImage(IMAGE_QUICKCOMMAND_FRAME_UP, m_Pos.x, y, 112.f, 45.f);
    y += 45.f;

    for (int i = 0; i < 3; ++i)
    {
        RenderImage(IMAGE_QUICKCOMMAND_FRAME_MIDDLE, m_Pos.x, y, 112.f, 15.f);
        y += 15.f;
    }

    RenderImage(IMAGE_QUICKCOMMAND_FRAME_MIDDLE, m_Pos.x, y, 112.f, 5.f);
    y += 5.f;

    RenderImage(IMAGE_QUICKCOMMAND_FRAME_DOWN, m_Pos.x, y, 112.f, 45.f);

    y = m_Pos.y + 55.f;

    for (int i = 0; i < 4; ++i)
    {
        RenderImage(IMAGE_QUICKCOMMAND_LINE, m_Pos.x + 15.f, y, 82.f, 2.f);
        y += 19.f;
    }
}

void mu::ui::window::CQuickCommandWindow::RenderContents()
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(0, 255, 0, 255);
    g_pRenderText->SetBgColor(0);

    int y = m_Pos.y + 14;
    g_pRenderText->RenderText(m_Pos.x, y, m_strID, 112, 0, RT3_SORT_CENTER);
    y += 30;

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);

    int iGlobalText[] = {943, 1124, 944, 948, 949};
    for (int i = 0; i < 5; ++i)
    {
        if (m_iSelectedIndex == i)
        {
            g_pRenderText->SetTextColor(255, 255, 0, 255);
        }
        else
        {
            g_pRenderText->SetTextColor(255, 255, 255, 255);
        }
        g_pRenderText->RenderText(m_Pos.x, y, I18N::Game::Lookup(iGlobalText[i]), 112, 0, RT3_SORT_CENTER);
        y += 19.f;
    }
}

void mu::ui::window::CQuickCommandWindow::RenderArrow()
{
    if (m_iSelectedIndex < 0)
    {
        return;
    }

    float x, y;
    x = m_Pos.x + 16.f;
    y = m_Pos.y + 43.f + (m_iSelectedIndex * 19);
    RenderImage(IMAGE_QUICKCOMMAND_ARROWL, x, y, 6.f, 9.f);
    x = m_Pos.x + 90.f;
    RenderImage(IMAGE_QUICKCOMMAND_ARROWR, x, y, 6.f, 9.f);
}

float mu::ui::window::CQuickCommandWindow::GetLayerDepth()
{
    return 2.0f;
}

float mu::ui::window::CQuickCommandWindow::GetKeyEventOrder()
{
    return 10.f;
}

void mu::ui::window::CQuickCommandWindow::OpenningProcess()
{
    m_iSelectedIndex = -1;
    m_iSelectedCharacterIndex = -1;
}

void mu::ui::window::CQuickCommandWindow::ClosingProcess()
{
    m_iSelectedIndex = -1;
    m_iSelectedCharacterIndex = -1;
}

void mu::ui::window::CQuickCommandWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_QUICKCOMMAND_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_commamd04.tga", IMAGE_QUICKCOMMAND_FRAME_UP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_commamd02.tga", IMAGE_QUICKCOMMAND_FRAME_MIDDLE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_commamd03.tga", IMAGE_QUICKCOMMAND_FRAME_DOWN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_commamd_Line.jpg", IMAGE_QUICKCOMMAND_LINE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_arrow(L).tga", IMAGE_QUICKCOMMAND_ARROWL, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_arrow(R).tga", IMAGE_QUICKCOMMAND_ARROWR, GL_LINEAR);
}

void mu::ui::window::CQuickCommandWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_QUICKCOMMAND_BACK);
    DeleteBitmap(IMAGE_QUICKCOMMAND_FRAME_UP);
    DeleteBitmap(IMAGE_QUICKCOMMAND_FRAME_MIDDLE);
    DeleteBitmap(IMAGE_QUICKCOMMAND_FRAME_DOWN);
    DeleteBitmap(IMAGE_QUICKCOMMAND_LINE);
    DeleteBitmap(IMAGE_QUICKCOMMAND_ARROWL);
    DeleteBitmap(IMAGE_QUICKCOMMAND_ARROWR);
}

void mu::ui::window::CQuickCommandWindow::OpenQuickCommand(const wchar_t* strID, int iIndex, int x, int y)
{
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_QUICK_COMMAND);

    SetID(strID);
    SetSelectedCharacterIndex(iIndex);
    SetPos(x, y);
}

void mu::ui::window::CQuickCommandWindow::CloseQuickCommand()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_QUICK_COMMAND) == true)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_QUICK_COMMAND);
    }
}

void mu::ui::window::CQuickCommandWindow::SetID(const wchar_t* strID)
{
    wcscpy(m_strID, strID);
}

void mu::ui::window::CQuickCommandWindow::SetSelectedCharacterIndex(int iIndex)
{
    m_iSelectedCharacterIndex = iIndex;
}