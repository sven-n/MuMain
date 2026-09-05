// NewUISetItemExplanation.cpp: implementation of the CNewUISetItemExplanation class.
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

mu::ui::window::CNewUISetItemExplanation::CNewUISetItemExplanation()
{
    m_pNewUIMng = NULL;
    m_Pos.x = 0;
    m_Pos.y = 0;
}

mu::ui::window::CNewUISetItemExplanation::~CNewUISetItemExplanation()
{
    Release();
}

bool mu::ui::window::CNewUISetItemExplanation::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_SETITEM_EXPLANATION, this);

    SetPos(x, y);

    Show(false);

    return true;
}

void mu::ui::window::CNewUISetItemExplanation::Release()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CNewUISetItemExplanation::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool mu::ui::window::CNewUISetItemExplanation::UpdateMouseEvent()
{
    return true;
}

bool mu::ui::window::CNewUISetItemExplanation::UpdateKeyEvent()
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

bool mu::ui::window::CNewUISetItemExplanation::Update()
{
    return true;
}

bool mu::ui::window::CNewUISetItemExplanation::Render()
{
    EnableAlphaTest();

    g_csItemOption.RenderOptionHelper();

    DisableAlphaBlend();

    return true;
}

float mu::ui::window::CNewUISetItemExplanation::GetLayerDepth()
{
    return 6.6f;
}

float mu::ui::window::CNewUISetItemExplanation::GetKeyEventOrder()
{
    return 10.f;
}

void mu::ui::window::CNewUISetItemExplanation::OpenningProcess() {}

void mu::ui::window::CNewUISetItemExplanation::ClosingProcess() {}