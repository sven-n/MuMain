
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
    // The original drew the table in Render(); it is laid out here (UI::TipTextList) for the
    // document.
    TipTextListRecord record;
    if (IsVisible())
        RecordTable(record);
    m_View.Sync(IsVisible(), record);
    return true;
}

void mu::ui::window::CSetItemExplanation::RecordTable(TipTextListRecord& record)
{
    // RenderOptionHelper(): the set's table centred on x 0 at y 0.
    const int textNum = g_csItemOption.BuildOptionHelperTextList();
    if (textNum > 0)
        UI::TipTextList::Record(record, 0, 0, textNum, 0, RT3_SORT_CENTER, STRP_NONE, true);
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