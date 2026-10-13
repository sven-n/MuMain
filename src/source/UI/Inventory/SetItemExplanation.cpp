
#include "stdafx.h"

#include "UI/Inventory/SetItemExplanation.h"
#include "UI/Core/WindowSystem.h"
#include "GameLogic/Items/CSItemOption.h"
#include "Audio/DSPlaySound.h"
#include "Engine/Object/ZzzInventory.h"
#include "UI/RmlBridge/RmlTheme.h"

using namespace SEASON3B;
using namespace mu::ui::window;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

mu::ui::window::CSetItemExplanation::CSetItemExplanation()
{
    m_pNewUIMng = NULL;
}

mu::ui::window::CSetItemExplanation::~CSetItemExplanation()
{
    Release();
}

bool mu::ui::window::CSetItemExplanation::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_SETITEM_EXPLANATION, this);

    m_View.Build();

    Show(false);

    return true;
}

void mu::ui::window::CSetItemExplanation::Release()
{
    m_View.Release();
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
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
    // RenderOptionHelper(): the set's options, as the inventory tooltip lists them.
    if (IsVisible())
        m_View.Sync(true, ItemHelpView::TextListLines(g_csItemOption.BuildOptionHelperTextList()));
    else
        m_View.Sync(false, {});
    return true;
}

bool mu::ui::window::CSetItemExplanation::Render()
{
    // Nothing native left: the table is RmlUi. Kept because CObject requires the override.
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