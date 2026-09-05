// NewUISetItemExplanation.cpp: implementation of the CSetItemExplanation class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "UI/Inventory/NewUISetItemExplanation.h"
#include "UI/Core/NewUISystem.h"
#include "GameLogic/Items/CSItemOption.h"
#include "Audio/DSPlaySound.h"

using namespace SEASON3B;
using namespace mu::ui::window;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

mu::ui::window::CSetItemExplanation::CSetItemExplanation()
{
    m_pNewUIMng = NULL;
    m_Pos.x = 0;
    m_Pos.y = 0;
}

mu::ui::window::CSetItemExplanation::~CSetItemExplanation()
{
    Release();
}

bool mu::ui::window::CSetItemExplanation::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_SETITEM_EXPLANATION, this);

    SetPos(x, y);

    Show(false);

    return true;
}

void mu::ui::window::CSetItemExplanation::Release()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CSetItemExplanation::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool mu::ui::window::CSetItemExplanation::UpdateMouseEvent()
{
    return true;
}

bool mu::ui::window::CSetItemExplanation::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_SETITEM_EXPLANATION))
    {
        if (IsPress(VK_ESCAPE) == true || IsPress(VK_F1) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_SETITEM_EXPLANATION);
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }

    return true;
}

bool mu::ui::window::CSetItemExplanation::Update()
{
    return true;
}

bool mu::ui::window::CSetItemExplanation::Render()
{
    EnableAlphaTest();

    g_csItemOption.RenderOptionHelper();

    DisableAlphaBlend();

    return true;
}

float mu::ui::window::CSetItemExplanation::GetLayerDepth()
{
    return 6.6f;
}

float mu::ui::window::CSetItemExplanation::GetKeyEventOrder()
{
    return 10.f;
}

void mu::ui::window::CSetItemExplanation::OpenningProcess() {}

void mu::ui::window::CSetItemExplanation::ClosingProcess() {}