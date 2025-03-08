// NewUIItemExplanationWindow.cpp: implementation of the CNewUIItemExplanationWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIItemExplanationWindow.h"
#include "NewUISystem.h"
#include "DSPlaySound.h"
#include "ZzzInventory.h"
#include "CSItemOption.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIItemExplanationWindow::CNewUIItemExplanationWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = 0;
    m_Pos.y = 0;
}

SEASON3B::CNewUIItemExplanationWindow::~CNewUIItemExplanationWindow()
{
    Release();
}

bool SEASON3B::CNewUIItemExplanationWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_ITEM_EXPLANATION, this);

    SetPos(x, y);

    Show(false);

    return true;
}

void SEASON3B::CNewUIItemExplanationWindow::Release()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUIItemExplanationWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool SEASON3B::CNewUIItemExplanationWindow::UpdateMouseEvent()
{
    return true;
}

bool SEASON3B::CNewUIItemExplanationWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_ITEM_EXPLANATION))
    {
        if (IsPress(VK_ESCAPE) == true || IsPress(VK_F1) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_ITEM_EXPLANATION);
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }

    return true;
}

bool SEASON3B::CNewUIItemExplanationWindow::Update()
{
    return true;
}

bool SEASON3B::CNewUIItemExplanationWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    extern int ItemHelp;
    extern wchar_t TextList[50][100];
    extern int TextListColor[50];
    extern int TextBold[50];
    extern int TextNum;
    extern int g_iItemInfo[12][17];

    int iInfoWidth = 0;
    int iLabelHeight = 0;
    int iDataHeight = 0;

    switch (WindowWidth)
    {
    case 640:
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
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_ITEM_EXPLANATION);
        return true;
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

        if (WindowWidth == 640 || WindowWidth == 1280)
            TabSpace += int(5940 / iInfoWidth);
        else if (WindowWidth == 800 || WindowWidth == 1024)
            TabSpace += int(5200 / iInfoWidth);
    }
    else
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_ITEM_EXPLANATION);
        return true;
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
    swprintf(TextList[TextNum], L"\n");
    TextNum++;

    wcscpy(TextList[TextNum], GlobalText[160]);
    TextListColor[TextNum] = TEXT_COLOR_BLUE;
    TextBold[TextNum] = true;
    TextNum++;

    swprintf(TextList[TextNum], L"%s", p->Name);
    TextListColor[TextNum] = TEXT_COLOR_WHITE;
    TextBold[TextNum] = true;
    TextNum++;

    swprintf(TextList[TextNum], L"\n");
    TextNum++;
    swprintf(TextList[TextNum], L" ");
    TextNum++;
    swprintf(TextList[TextNum], L"\n");
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
            swprintf(TextList[TextNum],"%s %s %s:(%s+%d)", Text, GlobalText[1089], GlobalText[591], GlobalText[1090], HIBYTE( mixLevel ) );TextNum++;
        }
    }
    if ( LOBYTE( mixLevel)<=3 )
    {
        wchar_t Text[100];
        if ( g_csItemOption.GetSetItemName( Text, ItemHelp, 2 ) )
        {
            TextListColor[TextNum] = TEXT_COLOR_GREEN;
            swprintf(TextList[TextNum],"%s %s %s:(%s+%d)", Text, GlobalText[1089], GlobalText[591], GlobalText[1090], LOBYTE( mixLevel ) );TextNum++;
        }
    }
    */

    RequireClass(p);
    swprintf(TextList[TextNum], L"\n");
    TextNum++;
    RenderTipTextList(1, 1, TextNum, iInfoWidth, RT3_SORT_CENTER);
    EnableAlphaTest();

    TextNum = 0;

    if (iType != 5)
    {
        RenderHelpCategory(_COLUMN_TYPE_LEVEL, TabSpace, iLabelHeight);
        RenderHelpLine(_COLUMN_TYPE_LEVEL, L"+%d", TabSpace, L"000000", iDataHeight, iType);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_REQNLV] > 0)
    {
        TabSpace += 2;
        RenderHelpCategory(_COLUMN_TYPE_REQNLV, TabSpace, iLabelHeight);
        RenderHelpLine(_COLUMN_TYPE_REQNLV, L"%3d", TabSpace, L"00000", iDataHeight, iType);
        TabSpace += 14;
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_ATTMIN] <= 0 || (ItemHelp >= ITEM_ETC && ItemHelp < ITEM_ETC + MAX_ITEM_INDEX))
    {
    }
    else
    {
        RenderHelpLine(_COLUMN_TYPE_ATTMIN, L"%3d", TabSpace, L"00 ", iDataHeight);
        RenderHelpCategory(_COLUMN_TYPE_ATTMIN, TabSpace, iLabelHeight);
        TabSpace += 2;
        RenderHelpLine(_COLUMN_TYPE_LEVEL, L"~", TabSpace, L" 00", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_ATTMAX] <= 0 || (ItemHelp >= ITEM_ETC && ItemHelp < ITEM_ETC + MAX_ITEM_INDEX))
    {
    }
    else
    {
        RenderHelpLine(_COLUMN_TYPE_ATTMAX, L"%3d", TabSpace, L"00000", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_MAGIC] > 0)
    {
        RenderHelpCategory(_COLUMN_TYPE_MAGIC, TabSpace, iLabelHeight);
        RenderHelpLine(_COLUMN_TYPE_MAGIC, L"%2d%%", TabSpace, L"00000", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_CURSE] > 0)
    {
        RenderHelpCategory(_COLUMN_TYPE_CURSE, TabSpace, iLabelHeight);
        RenderHelpLine(_COLUMN_TYPE_CURSE, L"%2d", TabSpace, L"00000", iDataHeight, iType);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_PET_ATTACK] > 0)
    {
        RenderHelpCategory(_COLUMN_TYPE_PET_ATTACK, TabSpace, iLabelHeight);
        RenderHelpLine(_COLUMN_TYPE_PET_ATTACK, L"%2d%%", TabSpace, L"00000", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_DEFENCE] > 0)
    {
        RenderHelpCategory(_COLUMN_TYPE_DEFENCE, TabSpace, iLabelHeight);
        RenderHelpLine(_COLUMN_TYPE_DEFENCE, L"%3d", TabSpace, L"000000", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_DEFRATE] > 0)
    {
        RenderHelpCategory(_COLUMN_TYPE_DEFRATE, TabSpace, iLabelHeight);
        RenderHelpLine(_COLUMN_TYPE_DEFRATE, L"%3d", TabSpace, L"000000", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_REQSTR] > 0)
    {
        RenderHelpCategory(_COLUMN_TYPE_REQSTR, TabSpace, iLabelHeight);
        RenderHelpLine(_COLUMN_TYPE_REQSTR, L"%3d", TabSpace, L"00000", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_REQDEX] > 0 || ItemHelp < ITEM_ETC)
    {
        RenderHelpCategory(_COLUMN_TYPE_REQDEX, TabSpace, iLabelHeight);
        RenderHelpLine(_COLUMN_TYPE_REQDEX, L"%3d", TabSpace, L"00000", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_REQVIT] > 0)
    {
        RenderHelpCategory(_COLUMN_TYPE_REQVIT, TabSpace, iLabelHeight);
        RenderHelpLine(_COLUMN_TYPE_REQVIT, L"%3d", TabSpace, L"00000", iDataHeight);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_REQENG] > 0)
    {
        RenderHelpCategory(_COLUMN_TYPE_REQENG, TabSpace, iLabelHeight);
        RenderHelpLine(_COLUMN_TYPE_REQENG, L"%3d", TabSpace, L"00000", iDataHeight, iType);
    }

    if (g_iItemInfo[0][_COLUMN_TYPE_REQCHA] > 0)
    {
        RenderHelpCategory(_COLUMN_TYPE_REQCHA, TabSpace, iLabelHeight);
        RenderHelpLine(_COLUMN_TYPE_REQCHA, L"%3d", TabSpace, L"00000", iDataHeight);
    }

    DisableAlphaBlend();
    return true;
}

float SEASON3B::CNewUIItemExplanationWindow::GetLayerDepth()
{
    return 6.5f;
}

float SEASON3B::CNewUIItemExplanationWindow::GetKeyEventOrder()
{
    return 10.f;
}

void SEASON3B::CNewUIItemExplanationWindow::OpenningProcess()
{
}

void SEASON3B::CNewUIItemExplanationWindow::ClosingProcess()
{
}