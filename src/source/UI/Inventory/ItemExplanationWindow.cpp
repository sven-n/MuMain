
#include "stdafx.h"
#include "UI/Inventory/ItemExplanationWindow.h"
#include "UI/Core/WindowSystem.h"
#include "Audio/DSPlaySound.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Items/CSItemOption.h"
#include "I18N/All.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Core/Utilities/StringUtils.h"
#include "Engine/Object/ZzzInterface.h"

extern int TextNum;
extern int g_iItemInfo[16][17];

using namespace SEASON3B;
using namespace mu::ui::window;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

mu::ui::window::CItemExplanationWindow::CItemExplanationWindow()
{
    m_pNewUIMng = NULL;
}

mu::ui::window::CItemExplanationWindow::~CItemExplanationWindow()
{
    Release();
}

bool mu::ui::window::CItemExplanationWindow::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_ITEM_EXPLANATION, this);

    m_View.Build();

    Show(false);

    return true;
}

void mu::ui::window::CItemExplanationWindow::Release()
{
    m_View.Release();
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

bool mu::ui::window::CItemExplanationWindow::UpdateMouseEvent()
{
    return true;
}

bool mu::ui::window::CItemExplanationWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_ITEM_EXPLANATION))
    {
        if (IsPress(VK_ESCAPE) == true || IsPress(VK_F1) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_ITEM_EXPLANATION);
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }

    return true;
}

bool mu::ui::window::CItemExplanationWindow::Update()
{
    // The original drew the table in Render(); its content is collected here for the document.
    if (IsVisible())
        SyncContent();
    else
        m_View.Sync(false, {});
    return true;
}

bool mu::ui::window::CItemExplanationWindow::Render()
{
    // Nothing native left: the table is RmlUi. Kept because CObject requires the override.
    return true;
}

void mu::ui::window::CItemExplanationWindow::SyncContent()
{
    // The globals, not block-scope externs: inside mu::ui::window those redeclared UIManager.cpp's
    // same-named references as plain objects, so the original read ItemHelp as garbage and hid the
    // window on its first frame (and would have written the table through the references).
    const bool etc = ItemHelp >= ITEM_ETC && ItemHelp < ITEM_ETC + MAX_ITEM_INDEX;
    const bool known = (ItemHelp >= ITEM_SWORD && ItemHelp < ITEM_BOW + MAX_ITEM_INDEX) ||
                       (ItemHelp >= ITEM_STAFF && ItemHelp < ITEM_STAFF + MAX_ITEM_INDEX) ||
                       (ItemHelp >= ITEM_SHIELD && ItemHelp < ITEM_SHIELD + MAX_ITEM_INDEX) ||
                       (ItemHelp >= ITEM_HELM && ItemHelp < ITEM_BOOTS + MAX_ITEM_INDEX) || etc;
    if (ItemHelp == ITEM_BOLT || ItemHelp == ITEM_ARROWS || !known)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_ITEM_EXPLANATION);
        m_View.Sync(false, {});
        return;
    }

    ITEM_ATTRIBUTE* p = &ItemAttribute[ItemHelp];
    ComputeItemInfo(ItemHelp);

    // The original's heading: "Item info" and the item's name, between half lines.
    TextNum = 0;
    mu_swprintf(TextList[TextNum], L"\n");
    TextNum++;
    wcscpy(TextList[TextNum], I18N::Game::ItemInfo);
    TextListColor[TextNum] = TEXT_COLOR_BLUE;
    TextBold[TextNum] = true;
    TextNum++;
    mu_swprintf(TextList[TextNum], L"%ls", p->Name);
    TextListColor[TextNum] = TEXT_COLOR_WHITE;
    TextBold[TextNum] = true;
    TextNum++;
    mu_swprintf(TextList[TextNum], L"\n");
    TextNum++;
    std::vector<ItemHelpLineEntry> lines = ItemHelpView::TextListLines(TextNum);

    // RenderHelpCategory() / RenderHelpLine()'s columns, in the original's order and on its
    // conditions: a heading and a value per level, white where the hero can equip the item and red
    // where not, the attack damage as min~max. The theme lays them out as a table.
    const int maxLevel = etc ? 0 : iMaxLevel;
    ItemHelpView::Table table;
    const auto column = [&](const wchar_t* heading, auto value)
    {
        ItemHelpColumnEntry entry{StringUtils::WideToNarrow(heading), {}};
        for (int level = 0; level <= maxLevel; ++level)
        {
            wchar_t text[64] = {};
            value(level, text);
            entry.cells.push_back({StringUtils::WideToNarrow(text), g_iItemInfo[level][_COLUMN_TYPE_CAN_EQUIP] == TRUE});
        }
        table.push_back(std::move(entry));
    };
    const auto number = [](int columnType, const wchar_t* format)
    { return [=](int level, wchar_t (&text)[64]) { mu_swprintf(text, format, g_iItemInfo[level][columnType]); }; };

    if (!etc)
        column(I18N::Game::LV, number(_COLUMN_TYPE_LEVEL, L"+%d"));
    if (g_iItemInfo[0][_COLUMN_TYPE_REQNLV] > 0)
        column(I18N::Game::ReqLV, number(_COLUMN_TYPE_REQNLV, L"%d"));
    if (g_iItemInfo[0][_COLUMN_TYPE_ATTMIN] > 0 && !etc)
    {
        const bool withMax = g_iItemInfo[0][_COLUMN_TYPE_ATTMAX] > 0;
        column(I18N::Game::ATKDmg, [withMax](int level, wchar_t (&text)[64])
               {
                   if (withMax)
                       mu_swprintf(text, L"%d~%d", g_iItemInfo[level][_COLUMN_TYPE_ATTMIN],
                                   g_iItemInfo[level][_COLUMN_TYPE_ATTMAX]);
                   else
                       mu_swprintf(text, L"%d~", g_iItemInfo[level][_COLUMN_TYPE_ATTMIN]);
               });
    }
    if (g_iItemInfo[0][_COLUMN_TYPE_MAGIC] > 0)
        column(I18N::Game::WIZDmg, number(_COLUMN_TYPE_MAGIC, L"%d%%"));
    if (g_iItemInfo[0][_COLUMN_TYPE_CURSE] > 0)
        column(I18N::Game::Curse, number(_COLUMN_TYPE_CURSE, L"%d"));
    if (g_iItemInfo[0][_COLUMN_TYPE_PET_ATTACK] > 0)
        column(I18N::Game::Attack, number(_COLUMN_TYPE_PET_ATTACK, L"%d%%"));
    if (g_iItemInfo[0][_COLUMN_TYPE_DEFENCE] > 0)
        column(I18N::Game::DEF, number(_COLUMN_TYPE_DEFENCE, L"%d"));
    if (g_iItemInfo[0][_COLUMN_TYPE_DEFRATE] > 0)
        column(I18N::Game::DEFRate, number(_COLUMN_TYPE_DEFRATE, L"%d"));
    if (g_iItemInfo[0][_COLUMN_TYPE_REQSTR] > 0)
        column(I18N::Game::STR, number(_COLUMN_TYPE_REQSTR, L"%d"));
    if (g_iItemInfo[0][_COLUMN_TYPE_REQDEX] > 0 || ItemHelp < ITEM_ETC)
        column(I18N::Game::AGI, number(_COLUMN_TYPE_REQDEX, L"%d"));
    if (g_iItemInfo[0][_COLUMN_TYPE_REQVIT] > 0)
        column(I18N::Game::STA, number(_COLUMN_TYPE_REQVIT, L"%d"));
    if (g_iItemInfo[0][_COLUMN_TYPE_REQENG] > 0)
        column(I18N::Game::ENG, number(_COLUMN_TYPE_REQENG, L"%d"));
    if (g_iItemInfo[0][_COLUMN_TYPE_REQCHA] > 0)
        column(I18N::Game::Command, number(_COLUMN_TYPE_REQCHA, L"%d"));

    // Under the table: which classes can equip it, and the closing half line.
    TextNum = 0;
    RequireClass(p);
    mu_swprintf(TextList[TextNum], L"\n");
    TextNum++;
    std::vector<ItemHelpLineEntry> tail = ItemHelpView::TextListLines(TextNum);
    TextNum = 0;

    m_View.Sync(true, std::move(lines), std::move(table), std::move(tail));
}

float mu::ui::window::CItemExplanationWindow::GetLayerDepth()
{
    return 6.5f;
}

float mu::ui::window::CItemExplanationWindow::GetKeyEventOrder()
{
    return 10.f;
}

void mu::ui::window::CItemExplanationWindow::OpenningProcess()
{
}

void mu::ui::window::CItemExplanationWindow::ClosingProcess()
{
}
