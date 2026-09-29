
#include "stdafx.h"
#include "UI/Inventory/ItemExplanationWindow.h"
#include "UI/Core/WindowSystem.h"
#include "Audio/DSPlaySound.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Items/CSItemOption.h"
#include "I18N/All.h"
#include "UI/RmlBridge/RmlTheme.h"
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
    m_Pos.x = 0;
    m_Pos.y = 0;
}

mu::ui::window::CItemExplanationWindow::~CItemExplanationWindow()
{
    Release();
}

bool mu::ui::window::CItemExplanationWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_ITEM_EXPLANATION, this);

    SetPos(x, y);

    m_View.Build();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { m_View.ReloadTheme(); });

    Show(false);

    return true;
}

void mu::ui::window::CItemExplanationWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CItemExplanationWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
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
    // The original drew the table in Render(); it is laid out here (UI::TipTextList) for the
    // document.
    TipTextListRecord record;
    if (IsVisible())
        RecordTable(record);
    m_View.Sync(IsVisible(), record);
    return true;
}

bool mu::ui::window::CItemExplanationWindow::Render()
{
    // Nothing native left: the table is RmlUi. Kept because CObject requires the override.
    return true;
}

void mu::ui::window::CItemExplanationWindow::RecordTable(TipTextListRecord& record)
{
    // The globals, not block-scope externs: inside mu::ui::window those redeclared UIManager.cpp's
    // same-named references as plain objects, so the original read ItemHelp as garbage and hid the
    // window on its first frame (and would have written the table through the references).

    int iInfoWidth = 0;
    int iLabelHeight = 0;
    int iDataHeight = 0;

    // The original knew only these four window widths; any other left iInfoWidth 0 and divided
    // by it below (never reached there: the window hid itself first). Other widths take the
    // nearest smaller one's values.
    const int layoutWidth = WindowWidth >= 1280   ? 1280
                            : WindowWidth >= 1024 ? 1024
                            : WindowWidth >= 800  ? 800
                                                  : REFERENCE_WIDTH;
    switch (layoutWidth)
    {
    case REFERENCE_WIDTH:
        iInfoWidth = 90;
        iLabelHeight = 38;
        iDataHeight = 52;
        break;
    case 800:
        iInfoWidth = 90;
        iLabelHeight = 33;
        iDataHeight = 47;
        break;
    case 1024:
        iInfoWidth = 103;
        iLabelHeight = 28;
        iDataHeight = 40;
        break;
    case 1280:
        iInfoWidth = 123;
        iLabelHeight = 22;
        iDataHeight = 32;
        break;
    }

    int iType = 0;
    int TabSpace = 0;

    if (ItemHelp == ITEM_BOLT || ItemHelp == ITEM_ARROWS)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_ITEM_EXPLANATION);
        return;
    }
    else if (ItemHelp >= ITEM_SWORD && ItemHelp < ITEM_BOW + MAX_ITEM_INDEX)
    {
        iType = 1;
        TabSpace += int(2160 / iInfoWidth);
    }
    else if (ItemHelp >= ITEM_STAFF && ItemHelp < ITEM_STAFF + MAX_ITEM_INDEX)
    {
        iType = 2;
        TabSpace += int(1800 / iInfoWidth);
    }
    else if (ItemHelp >= ITEM_SHIELD && ItemHelp < ITEM_SHIELD + MAX_ITEM_INDEX)
    {
        iType = 3;
        TabSpace += int(1800 / iInfoWidth);
    }
    else if (ItemHelp >= ITEM_HELM && ItemHelp < ITEM_BOOTS + MAX_ITEM_INDEX)
    {
        iType = 4;
        TabSpace += int(1800 / iInfoWidth);
    }
    else if (ItemHelp >= ITEM_ETC && ItemHelp < ITEM_ETC + MAX_ITEM_INDEX)
    {
        iType = 5;

        if (layoutWidth == REFERENCE_WIDTH || layoutWidth == 1280)
            TabSpace += int(5940 / iInfoWidth);
        else if (layoutWidth == 800 || layoutWidth == 1024)
            TabSpace += int(5200 / iInfoWidth);
    }
    else
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_ITEM_EXPLANATION);
        return;
    }

    if (ItemHelp >= ITEM_BOOK_OF_SAHAMUTT && ItemHelp <= ITEM_STAFF + 29)
    {
        iType = 6;
        TabSpace += int(800 / iInfoWidth);//20
    }

    int iCurrMaxLevel = iMaxLevel;

    if (iType == 5)
    {
        iCurrMaxLevel = 0;
    }

    ITEM_ATTRIBUTE* p = &ItemAttribute[ItemHelp];
    ComputeItemInfo(ItemHelp);

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
    mu_swprintf(TextList[TextNum], L" ");
    TextNum++;
    mu_swprintf(TextList[TextNum], L"\n");
    TextNum++;

    float fNumAdd = 1.0f;
    if (!(g_iItemInfo[0][_COLUMN_TYPE_ATTMIN] <= 0 || (ItemHelp >= ITEM_ETC && ItemHelp < ITEM_ETC + MAX_ITEM_INDEX)))
        ++fNumAdd;
    if (!(g_iItemInfo[0][_COLUMN_TYPE_ATTMAX] <= 0 || (ItemHelp >= ITEM_ETC && ItemHelp < ITEM_ETC + MAX_ITEM_INDEX)))
        ++fNumAdd;
    if (g_iItemInfo[0][_COLUMN_TYPE_MAGIC] > 0)
        ++fNumAdd;
    if (g_iItemInfo[0][_COLUMN_TYPE_CURSE] > 0)
        ++fNumAdd;
    if (g_iItemInfo[0][_COLUMN_TYPE_PET_ATTACK] > 0)
        ++fNumAdd;
    if (g_iItemInfo[0][_COLUMN_TYPE_DEFENCE] > 0)
        fNumAdd += 1.1f;
    if (g_iItemInfo[0][_COLUMN_TYPE_DEFRATE] > 0)
        fNumAdd += 1.1f;
    if (g_iItemInfo[0][_COLUMN_TYPE_REQSTR] > 0)
        ++fNumAdd;
    if (g_iItemInfo[0][_COLUMN_TYPE_REQDEX] > 0 || ItemHelp < ITEM_ETC)
        ++fNumAdd;
    if (g_iItemInfo[0][_COLUMN_TYPE_REQENG] > 0)
        fNumAdd += 1.1f;
    if (g_iItemInfo[0][_COLUMN_TYPE_REQNLV] > 0)
        fNumAdd += 1.1f;
    if (g_iItemInfo[0][_COLUMN_TYPE_REQVIT] > 0)
        fNumAdd += 1.1f;
    if (g_iItemInfo[0][_COLUMN_TYPE_REQCHA] > 0)
        fNumAdd += 1.1f;

    int iAddWidth = float(17 * iInfoWidth / 90) * fNumAdd + 0.5f;
    if (iInfoWidth < iAddWidth)
        iInfoWidth = iAddWidth;

    if (iType == 5 && fNumAdd < 3.f)
    {
        TabSpace += 20;
    }

    int iInfoNum = (WindowWidth <= 800 ? 46 : 51);
    memset(TextList[TextNum], ' ', iInfoNum);
    TextList[TextNum][iInfoNum] = '\0';
    TextListColor[TextNum] = TEXT_COLOR_WHITE;
    TextBold[TextNum] = false;
    TextNum++;

    for (int Level = 0; Level <= iCurrMaxLevel - 1; ++Level)
    {
        TextList[TextNum][0] = ' '; TextList[TextNum][1] = '\0';
        TextBold[TextNum] = false;
        TextNum++;
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
    }

    /*
    WORD mixLevel = g_csItemOption.GetMixItemLevel ( ItemHelp );
    if ( HIBYTE( mixLevel)<=3 )
    {
        wchar_t Text[100];
        if ( g_csItemOption.GetSetItemName( Text, ItemHelp, 1 ) )
        {
            TextListColor[TextNum] = TEXT_COLOR_GREEN;
            mu_swprintf(TextList[TextNum],"%ls %ls %ls:(%ls+%d)", Text, I18N::Game::Set, I18N::Game::Combining, I18N::Game::AncientMetal, HIBYTE( mixLevel ) );TextNum++;
        }
    }
    if ( LOBYTE( mixLevel)<=3 )
    {
        wchar_t Text[100];
        if ( g_csItemOption.GetSetItemName( Text, ItemHelp, 2 ) )
        {
            TextListColor[TextNum] = TEXT_COLOR_GREEN;
            mu_swprintf(TextList[TextNum],"%ls %ls %ls:(%ls+%d)", Text, I18N::Game::Set, I18N::Game::Combining, I18N::Game::AncientMetal, LOBYTE( mixLevel ) );TextNum++;
        }
    }
    */

    RequireClass(p);
    mu_swprintf(TextList[TextNum], L"\n");
    TextNum++;
    UI::TipTextList::Record(record, 1, 1, TextNum, iInfoWidth, RT3_SORT_CENTER, STRP_NONE, true);

    TextNum = 0;

    if (iType != 5)
    {
        UI::TipTextList::RecordHelpCategory(record, _COLUMN_TYPE_LEVEL, TabSpace, iLabelHeight);
        UI::TipTextList::RecordHelpLine(record, _COLUMN_TYPE_LEVEL, L"+%d", TabSpace, L"000000", iDataHeight, iType);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_REQNLV] > 0)
    {
        TabSpace += 2;
        UI::TipTextList::RecordHelpCategory(record, _COLUMN_TYPE_REQNLV, TabSpace, iLabelHeight);
        UI::TipTextList::RecordHelpLine(record, _COLUMN_TYPE_REQNLV, L"%3d", TabSpace, L"00000", iDataHeight, iType);
        TabSpace += 14;
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_ATTMIN] <= 0 || (ItemHelp >= ITEM_ETC && ItemHelp < ITEM_ETC + MAX_ITEM_INDEX))
    {
    }
    else
    {
        UI::TipTextList::RecordHelpLine(record, _COLUMN_TYPE_ATTMIN, L"%3d", TabSpace, L"00 ", iDataHeight);
        UI::TipTextList::RecordHelpCategory(record, _COLUMN_TYPE_ATTMIN, TabSpace, iLabelHeight);
        TabSpace += 2;
        UI::TipTextList::RecordHelpLine(record, _COLUMN_TYPE_LEVEL, L"~", TabSpace, L" 00", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_ATTMAX] <= 0 || (ItemHelp >= ITEM_ETC && ItemHelp < ITEM_ETC + MAX_ITEM_INDEX))
    {
    }
    else
    {
        UI::TipTextList::RecordHelpLine(record, _COLUMN_TYPE_ATTMAX, L"%3d", TabSpace, L"00000", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_MAGIC] > 0)
    {
        UI::TipTextList::RecordHelpCategory(record, _COLUMN_TYPE_MAGIC, TabSpace, iLabelHeight);
        UI::TipTextList::RecordHelpLine(record, _COLUMN_TYPE_MAGIC, L"%2d%%", TabSpace, L"00000", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_CURSE] > 0)
    {
        UI::TipTextList::RecordHelpCategory(record, _COLUMN_TYPE_CURSE, TabSpace, iLabelHeight);
        UI::TipTextList::RecordHelpLine(record, _COLUMN_TYPE_CURSE, L"%2d", TabSpace, L"00000", iDataHeight, iType);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_PET_ATTACK] > 0)
    {
        UI::TipTextList::RecordHelpCategory(record, _COLUMN_TYPE_PET_ATTACK, TabSpace, iLabelHeight);
        UI::TipTextList::RecordHelpLine(record, _COLUMN_TYPE_PET_ATTACK, L"%2d%%", TabSpace, L"00000", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_DEFENCE] > 0)
    {
        UI::TipTextList::RecordHelpCategory(record, _COLUMN_TYPE_DEFENCE, TabSpace, iLabelHeight);
        UI::TipTextList::RecordHelpLine(record, _COLUMN_TYPE_DEFENCE, L"%3d", TabSpace, L"000000", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_DEFRATE] > 0)
    {
        UI::TipTextList::RecordHelpCategory(record, _COLUMN_TYPE_DEFRATE, TabSpace, iLabelHeight);
        UI::TipTextList::RecordHelpLine(record, _COLUMN_TYPE_DEFRATE, L"%3d", TabSpace, L"000000", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_REQSTR] > 0)
    {
        UI::TipTextList::RecordHelpCategory(record, _COLUMN_TYPE_REQSTR, TabSpace, iLabelHeight);
        UI::TipTextList::RecordHelpLine(record, _COLUMN_TYPE_REQSTR, L"%3d", TabSpace, L"00000", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_REQDEX] > 0 || ItemHelp < ITEM_ETC)
    {
        UI::TipTextList::RecordHelpCategory(record, _COLUMN_TYPE_REQDEX, TabSpace, iLabelHeight);
        UI::TipTextList::RecordHelpLine(record, _COLUMN_TYPE_REQDEX, L"%3d", TabSpace, L"00000", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_REQVIT] > 0)
    {
        UI::TipTextList::RecordHelpCategory(record, _COLUMN_TYPE_REQVIT, TabSpace, iLabelHeight);
        UI::TipTextList::RecordHelpLine(record, _COLUMN_TYPE_REQVIT, L"%3d", TabSpace, L"00000", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_REQENG] > 0)
    {
        UI::TipTextList::RecordHelpCategory(record, _COLUMN_TYPE_REQENG, TabSpace, iLabelHeight);
        UI::TipTextList::RecordHelpLine(record, _COLUMN_TYPE_REQENG, L"%3d", TabSpace, L"00000", iDataHeight, iType);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_REQCHA] > 0)
    {
        UI::TipTextList::RecordHelpCategory(record, _COLUMN_TYPE_REQCHA, TabSpace, iLabelHeight);
        UI::TipTextList::RecordHelpLine(record, _COLUMN_TYPE_REQCHA, L"%3d", TabSpace, L"00000", iDataHeight);
    }
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
