// NewUIInGameShop.cpp: implementation of the NewUIInGameShop class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef PBG_ADD_INGAMESHOP_UI_ITEMSHOP
#include "iexplorer.h"
#include "NewUISystem.h"
#include "NewUIInGameShop.h"
#include "MsgBoxIGSBuyPackageItem.h"
#include "MsgBoxIGSBuySelectItem.h"
#include "MsgBoxIGSCommon.h"
#include "MsgBoxIGSStorageItemInfo.h"
#include "MsgBoxIGSGiftStorageItemInfo.h"
#include "MapManager.h"
#include "DSPlaySound.h"

using namespace SEASON3B;

CNewUIInGameShop::CNewUIInGameShop()
{
    Init();
}

CNewUIInGameShop::~CNewUIInGameShop()
{
    Release();
}

void CNewUIInGameShop::Init()
{
    m_ItemAngle = false;
    m_bLoadBanner = false;
    m_bBannerLink = false;
    m_iStorageTotalItemCnt = 0;
    m_iStorageCurrentPageItemCnt = 0;
    m_iStorageTotalPage = 0;
    m_iStorageCurrentPage = 0;
    m_iSelectedStorageItemIndex = 0;
    m_iStorageCurrentPageReceiveItemCnt = 0;
    m_bRequestCurrentPage = false;
}

void CNewUIInGameShop::Release()
{
    UnloadImages();

    ReleaseBanner();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    ClearAllStorageItem();
}

bool CNewUIInGameShop::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (pNewUIMng == NULL)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_INGAMESHOP, this);

    SetPos(x, y);
    LoadImages();
    SetBtnInfo();
    Show(false);	//visible()을 flase로

    return true;
}

void CNewUIInGameShop::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CNewUIInGameShop::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderButtons();
    RenderTexts();
    RenderBanner();
    RenderListBox();
    RenderDisplayItems();
    DisableAlphaBlend();
    return true;
}

void CNewUIInGameShop::RenderFrame()
{
    RenderImage(IMAGE_IGS_BACK, m_Pos.x, m_Pos.y, IMAGE_IGS_BACK_WIDTH, IMAGE_IGS_BACK_HEIGHT);

    int iSizeCategory = g_InGameShopSystem->GetSizeCategoriesAsSelectedZone();

    if (iSizeCategory < 0)
        return;

    // Category Deco Middle Render
    POINT CategoryDecoMiddlePos;
    CategoryDecoMiddlePos.x = m_CategoryButton.GetPos(0).x + (IMAGE_IGS_CATEGORY_BTN_WIDTH / 2) - (IMAGE_IGS_CATEGORY_DECO_MIDDLE_WIDTH / 2);

    for (int i = 0; i < iSizeCategory - 1; i++)
    {
        CategoryDecoMiddlePos.y = m_CategoryButton.GetPos(i).y + IMAGE_IGS_CATEGORY_BTN_HEIGHT - 1;

        RenderImage(IMAGE_IGS_CATEGORY_DECO_MIDDLE, CategoryDecoMiddlePos.x, CategoryDecoMiddlePos.y, IMAGE_IGS_CATEGORY_DECO_MIDDLE_WIDTH, IMAGE_IGS_CATEGORY_DECO_MIDDLE_HEIGHT);
    }

    // Category Deco Down Render
    RenderImage(IMAGE_IGS_CATEGORY_DECO_DOWN, m_Pos.x, m_CategoryButton.GetPos(iSizeCategory - 1).y - 10, IMAGE_IGS_CATEGORY_DECO_DOWN_WIDTH, IMAGE_IGS_CATEGORY_DECO_DOWN_HEIGHT);

    for (int cnt = g_InGameShopSystem->GetSizePackageAsDisplayPackage(); cnt < INGAMESHOP_DISPLAY_ITEMLIST_SIZE; cnt++)
    {
        RenderImage(IMAGE_IGS_ITEMBOX_LOGO, m_Pos.x + IMAGE_IGS_ITEMBOX_LOGO_POS_X + ((cnt % IGS_NUM_ITEMS_WIDTH) * IMAGE_IGS_VIEWDETAIL_BTN_DISTANCE_X), m_Pos.y + IMAGE_IGS_ITEMBOX_LOGO_POS_Y + ((cnt / IGS_NUM_ITEMS_HEIGHT) * IMAGE_IGS_VIEWDETAIL_BTN_DISTANCE_Y), IMAGE_IGS_ITEMBOX_LOGO_SIZE, IMAGE_IGS_ITEMBOX_LOGO_SIZE);
    }

    RenderImage(IMAGE_IGS_STORAGE_PAGE, m_Pos.x + IMAGE_IGS_STORAGE_PAGE_POS_X, m_Pos.y + IMAGE_IGS_STORAGE_PAGE_POS_Y, IMGAE_IGS_STORAGE_PAGE_WIDTH, IMGAE_IGS_STORAGE_PAGE_HEIGHT);
}

void CNewUIInGameShop::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    wchar_t szValue[256] = { 0, };

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    swprintf(szText, Hero->ID);
    g_pRenderText->RenderText(m_Pos.x + TEXT_IGS_CHAR_NAME_POS_X, m_Pos.y + TEXT_IGS_CHAR_NAME_POS_Y, szText, TEXT_IGS_CHAR_NAME_WIDTH, 0, RT3_SORT_CENTER);
    g_pRenderText->SetFont(g_hFont);

    // Display Item
    for (int i = 0; i < g_InGameShopSystem->GetSizePackageAsDisplayPackage(); i++)
    {
        CShopPackage* pPackage = g_InGameShopSystem->GetDisplayPackage(i);
        // Package
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->RenderText(m_Pos.x + IGS_PACKAGE_NAME_POS_X + ((i % IGS_NUM_ITEMS_WIDTH) * IMAGE_IGS_VIEWDETAIL_BTN_DISTANCE_X), m_Pos.y + IGS_PACKAGE_NAME_POS_Y + ((i / IGS_NUM_ITEMS_HEIGHT) * IMAGE_IGS_VIEWDETAIL_BTN_DISTANCE_Y), pPackage->PackageProductName, IGS_PACKAGE_NAME_WIDTH, 0, RT3_SORT_CENTER);
        // Package
        ConvertGold(pPackage->Price, szValue);
        swprintf(szText, L"%s %s", szValue, pPackage->PricUnitName);
        g_pRenderText->SetTextColor(255, 238, 161, 255);
        g_pRenderText->RenderText(m_Pos.x + IGS_PACKAGE_NAME_POS_X + ((i % IGS_NUM_ITEMS_WIDTH) * IMAGE_IGS_VIEWDETAIL_BTN_DISTANCE_X), m_Pos.y + IGS_PACKAGE_PRICE_POS_Y + 53 + ((i / IGS_NUM_ITEMS_HEIGHT) * IMAGE_IGS_VIEWDETAIL_BTN_DISTANCE_Y), szText, IGS_PACKAGE_NAME_WIDTH, 0, RT3_SORT_CENTER);
    }
    g_pRenderText->SetTextColor(255, 238, 161, 255);

    //CreditCard
    ConvertGold(g_InGameShopSystem->GetCashCreditCard(), szValue);
    swprintf(szText, GlobalText[2883], L"");
    g_pRenderText->RenderText(m_Pos.x + TEXT_IGS_CASH_POS_X, m_Pos.y + TEXT_IGS_CASH_POS_Y, szText, TEXT_IGS_CASH_WIDTH, 0, RT3_SORT_LEFT);
    g_pRenderText->RenderText(m_Pos.x + TEXT_IGS_CASH_POS_X + 50, m_Pos.y + TEXT_IGS_CASH_POS_Y, szValue, TEXT_IGS_CASH_WIDTH - 56, 0, RT3_SORT_RIGHT);

    //Prepaid
    ConvertGold(g_InGameShopSystem->GetCashPrepaid(), szValue);
    swprintf(szText, GlobalText[3145], L"");
    g_pRenderText->RenderText(m_Pos.x + TEXT_IGS_CASH_POS_X, m_Pos.y + TEXT_IGS_MILEAGE_POS_Y, szText, TEXT_IGS_CASH_WIDTH, 0, RT3_SORT_LEFT);
    g_pRenderText->RenderText(m_Pos.x + TEXT_IGS_CASH_POS_X + 50, m_Pos.y + TEXT_IGS_MILEAGE_POS_Y, szValue, TEXT_IGS_CASH_WIDTH - 56, 0, RT3_SORT_RIGHT);

    ConvertGold(g_InGameShopSystem->GetTotalMileage(), szValue, 1);
    swprintf(szText, GlobalText[2884], L"");
    g_pRenderText->RenderText(m_Pos.x + TEXT_IGS_CASH_POS_X, m_Pos.y + TEXT_IGS_POINT_POS_Y, szText, TEXT_IGS_CASH_WIDTH, 0, RT3_SORT_LEFT);
    g_pRenderText->RenderText(m_Pos.x + TEXT_IGS_CASH_POS_X + 50, m_Pos.y + TEXT_IGS_POINT_POS_Y, szValue, TEXT_IGS_CASH_WIDTH - 56, 0, RT3_SORT_RIGHT);

    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);

    g_pRenderText->RenderText(m_Pos.x + TEXT_IGS_STORAGE_NAME_POS_X, m_Pos.y + TEXT_IGS_STORAGE_NAME_POS_Y, GlobalText[2951], TEXT_IGS_STORAGE_NAME_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->RenderText(m_Pos.x + TEXT_IGS_STORAGE_TIME_POS_X, m_Pos.y + TEXT_IGS_STORAGE_NAME_POS_Y, GlobalText[2952], TEXT_IGS_STORAGE_TIME_WIDTH, 0, RT3_SORT_CENTER);

    // Page Info
    g_pRenderText->RenderText(m_Pos.x + TEXT_IGS_PAGE_POS_X + 23, m_Pos.y + TEXT_IGS_PAGE_POS_Y, L"/", 10, 0, RT3_SORT_CENTER);

    swprintf(szText, L"%d", g_InGameShopSystem->GetSelectPage());
    g_pRenderText->RenderText(m_Pos.x + TEXT_IGS_PAGE_POS_X + 5, m_Pos.y + TEXT_IGS_PAGE_POS_Y, szText, 15, 0, RT3_SORT_RIGHT);

    swprintf(szText, L"%d", g_InGameShopSystem->GetTotalPages());
    g_pRenderText->RenderText(m_Pos.x + TEXT_IGS_PAGE_POS_X + 36, m_Pos.y + TEXT_IGS_PAGE_POS_Y, szText, 15, 0, RT3_SORT_LEFT);

    // Storage Page Info
    g_pRenderText->RenderText(m_Pos.x + TEXT_IGS_STORAGE_PAGE_INFO_POS_X + 35, m_Pos.y + TEXT_IGS_STORAGE_PAGE_INFO_POS_Y, L"/", 10, 0, RT3_SORT_CENTER);
    swprintf(szText, L"%d", m_iStorageCurrentPage);
    g_pRenderText->RenderText(m_Pos.x + TEXT_IGS_STORAGE_PAGE_INFO_POS_X + 12, m_Pos.y + TEXT_IGS_STORAGE_PAGE_INFO_POS_Y, szText, 20, 0, RT3_SORT_RIGHT);
    swprintf(szText, L"%d", m_iStorageTotalPage);
    g_pRenderText->RenderText(m_Pos.x + TEXT_IGS_STORAGE_PAGE_INFO_POS_X + 48, m_Pos.y + TEXT_IGS_STORAGE_PAGE_INFO_POS_Y, szText, 20, 0, RT3_SORT_LEFT);

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
#ifdef FOR_WORK
    g_pRenderText->SetTextColor(210, 180, 230, 255);
    g_pRenderText->SetFont(g_hFont);

    // Script Version Info
    CListVersionInfo ScriptVer;
    ScriptVer = g_InGameShopSystem->GetCurrentScriptVer();
    swprintf(szText, L"Script Ver. %d.%d.%d", ScriptVer.Zone, ScriptVer.year, ScriptVer.yearId);
    g_pRenderText->RenderText(m_Pos.x + 12, m_Pos.y + 396, szText, 150, 0, RT3_SORT_LEFT);

    ScriptVer = g_InGameShopSystem->GetCurrentBannerVer();
    swprintf(szText, L"Banner Ver. %d.%d.%d", ScriptVer.Zone, ScriptVer.year, ScriptVer.yearId);
    g_pRenderText->RenderText(m_Pos.x + 12, m_Pos.y + 408, szText, 150, 0, RT3_SORT_LEFT);
#endif // FOR_WORK
#endif //KJH_MOD_SHOP_SCRIPT_DOWNLOAD
}

void CNewUIInGameShop::RenderButtons()
{
    m_ZoneButton.Render();
    m_CategoryButton.Render();
    m_ListBoxTabButton.Render();

    for (int i = 0; i < g_InGameShopSystem->GetSizePackageAsDisplayPackage(); i++)
    {
        m_ViewDetailButton[i].Render();
    }

    m_CashGiftButton.Render();
    m_CashChargeButton.Render();
    m_CashRefreshButton.Render();
    m_UseButton.Render();
    m_PrevButton.Render();
    m_NextButton.Render();
    m_StoragePrevButton.Render();
    m_StorageNextButton.Render();
    m_CloseButton.Render();
}

void CNewUIInGameShop::RenderListBox()
{
    m_StorageItemListBox.Render();
}

bool CNewUIInGameShop::IsInGameShopRect(float _x, float _y)
{
    if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INGAMESHOP))
        return false;

    RECT _TempRT;

    _TempRT.top = 0;
    _TempRT.bottom = IMAGE_IGS_BACK_HEIGHT;
    _TempRT.left = 0;
    _TempRT.right = IMAGE_IGS_BACK_WIDTH;

    if (_x >= _TempRT.left && _x < _TempRT.right && _y < _TempRT.bottom && _y >= _TempRT.top)
        return true;
    else
        return false;

    return false;
}

void CNewUIInGameShop::SetConvertInvenCoord(WORD _ItemType, float _Width, float _Height)
{
    ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[_ItemType];
    float _TempWidth = pItemAttr->Width * 20.0f;
    float _TempHeight = pItemAttr->Height * 20.0f;
    float _fCoodX = 0, _fCoodY = 0;

    if (_ItemType == ITEM_WING_OF_STORM)
    {
        _fCoodY = 5.0f;
    }

    else if (pItemAttr->Height >= 4)
    {
        _fCoodY = -10.0f;
    }

    m_fRePos.x = (_Width / 2) - (_TempWidth / 2) + _fCoodX;
    m_fRePos.y = (_Height / 2) - (_TempHeight / 2) + _fCoodY;
    m_fReSize.x = _TempWidth;
    m_fReSize.y = _TempHeight;
}
void CNewUIInGameShop::SetRateScale(int _ItemType)
{
    const float _fRate_Value = 0.703f;
    ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[_ItemType];

    if (_ItemType == ITEM_WING_OF_STORM)
    {
        m_fRate_Scale = _fRate_Value * 0.7f;
    }
    else if (_ItemType == ITEM_DIVINE_STAFF_OF_ARCHANGEL)
    {
        m_fRate_Scale = _fRate_Value * 0.7f;
    }
    else if (_ItemType >= ITEM_HELPER + 117 && _ItemType <= ITEM_HELPER + 120)
    {
        m_fRate_Scale = _fRate_Value * 1.6f;
    }
    else if (pItemAttr->Height >= 4)
    {
        m_fRate_Scale = _fRate_Value * 0.7f;
    }
    else
    {
        m_fRate_Scale = _fRate_Value;
    }
}

void CNewUIInGameShop::RenderDisplayItems()
{
    EndBitmap();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glViewport2(0, 0, WindowWidth, WindowHeight);
    gluPerspective2(2.0f, (float)(WindowWidth) / (float)(WindowHeight), RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    GetOpenGLMatrix(CameraMatrix);
    EnableDepthTest();
    EnableDepthMask();

    glClear(GL_DEPTH_BUFFER_BIT);

    for (int i = 0; i < g_InGameShopSystem->GetSizePackageAsDisplayPackage(); i++)
    {
        int iPosX = IGS_ITEMRENDER_POS_X_STANDAD + (IMAGE_IGS_VIEWDETAIL_BTN_DISTANCE_X * (i % IGS_NUM_ITEMS_WIDTH));
        int iPosY = IGS_ITEMRENDER_POS_Y_STANDAD + (IMAGE_IGS_VIEWDETAIL_BTN_DISTANCE_Y * (i / IGS_NUM_ITEMS_HEIGHT));
        RenderItem3D(iPosX, iPosY, IGS_ITEMRENDER_POS_WIDTH, IGS_ITEMRENDER_POS_HEIGHT, g_InGameShopSystem->GetPackageItemCode(i), 0, 0, 0, true);
    }

    UpdateMousePositionn();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    BeginBitmap();
}

bool CNewUIInGameShop::BtnProcess()
{
    if (g_InGameShopSystem->IsRequestEventPackge() == true)
    {
        if (m_ZoneButton.UpdateMouseEvent() != -1)
        {
            g_InGameShopSystem->SelectZone(m_ZoneButton.GetCurButtonIndex());
            InitCategoryBtn();
            g_InGameShopSystem->SelectCategory(m_CategoryButton.GetCurButtonIndex());
            return true;
        }

        if (m_CategoryButton.UpdateMouseEvent() != -1)
        {
            g_InGameShopSystem->SelectCategory(m_CategoryButton.GetCurButtonIndex());
            return true;
        }
    }

    if (m_ListBoxTabButton.UpdateMouseEvent() != -1)
    {
        char szCode = GetCurrentStorageCode();
        m_iSelectedStorageItemIndex = 0;
        m_bRequestCurrentPage = true;
        SocketClient->ToGameServer()->SendCashShopStorageListRequest(1, szCode);
        return true;
    }

    for (int i = 0; i < g_InGameShopSystem->GetSizePackageAsDisplayPackage(); i++)
    {
        if (m_ViewDetailButton[i].UpdateMouseEvent())
        {
            CShopPackage* pPackage = g_InGameShopSystem->GetDisplayPackage(i);

            if (pPackage->PriceCount == 1)
            {
                CMsgBoxIGSBuyPackageItem* pMsgBox = NULL;
                CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxBuyPackageItemLayout), &pMsgBox);
                pMsgBox->Initialize(pPackage);
            }
            else if (pPackage->PriceCount > 1)
            {
                CMsgBoxIGSBuySelectItem* pMsgBox = NULL;
                CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSBuySelectItemLayout), &pMsgBox);
                pMsgBox->Initialize(pPackage);
            }

            return true;
        }
    }

    if (m_CashGiftButton.UpdateMouseEvent() == true)
    {
        CMsgBoxIGSCommon* pMsgBox = NULL;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2937], GlobalText[2938]);
        return true;
    }

    if (m_CashChargeButton.UpdateMouseEvent() == true)
    {
        CMsgBoxIGSCommon* pMsgBox = NULL;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2937], GlobalText[2938]);
        return true;
    }

    if (m_CashRefreshButton.UpdateMouseEvent() == true)
    {
        SocketClient->ToGameServer()->SendCashShopPointInfoRequest();

        return true;
    }

    if (m_UseButton.UpdateMouseEvent() == true)
    {
        if (m_StorageItemListBox.GetLineNum() <= 0)
        {
            CMsgBoxIGSCommon* pMsgBox = NULL;
            CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
            pMsgBox->Initialize(GlobalText[3028], GlobalText[3033]);
            return true;
        }

        int iStorageIndex = m_ListBoxTabButton.GetCurButtonIndex();

        IGS_StorageItem* pSelectItem = m_StorageItemListBox.GetSelectedText();

        if (iStorageIndex == IGS_SAFEKEEPING_LISTBOX)					// 보관함
        {
            CMsgBoxIGSStorageItemInfo* pMsgBox = NULL;
            CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSStorageItemInfoLayout), &pMsgBox);
            pMsgBox->Initialize(pSelectItem->m_iStorageSeq, pSelectItem->m_iStorageItemSeq, pSelectItem->m_wItemCode, pSelectItem->m_szType,
                pSelectItem->m_szName, pSelectItem->m_szNum, pSelectItem->m_szPeriod);
        }
        else if (iStorageIndex == IGS_PRESENTBOX_LISTBOX)				// 선물 보관함
        {
            CMsgBoxIGSGiftStorageItemInfo* pMsgBox = NULL;
            CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSGiftStorageItemInfoLayout), &pMsgBox);
            pMsgBox->Initialize(pSelectItem->m_iStorageSeq, pSelectItem->m_iStorageItemSeq, pSelectItem->m_wItemCode,
                pSelectItem->m_szType, pSelectItem->m_szSendUserName, pSelectItem->m_szMessage,
                pSelectItem->m_szName, pSelectItem->m_szNum, pSelectItem->m_szPeriod);
        }
        return true;
    }

    // Prev Button
    if (m_PrevButton.UpdateMouseEvent())
    {
        g_InGameShopSystem->PrePage();
        return true;
    }

    // Next Button
    if (m_NextButton.UpdateMouseEvent())
    {
        g_InGameShopSystem->NextPage();
        return true;
    }

    // Storage Prev Button
    if (m_StoragePrevButton.UpdateMouseEvent())
    {
        StoragePrevPage();
        return true;
    }

    // Next Button
    if (m_StorageNextButton.UpdateMouseEvent())
    {
        StorageNextPage();
        return true;
    }

    if (m_CloseButton.UpdateMouseEvent() == true)
    {
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INGAMESHOP) == true)
        {
            SocketClient->ToGameServer()->SendCashShopOpenState(1);
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_INGAMESHOP);

            return true;
        }
        return false;
    }

    return false;
}

void CNewUIInGameShop::SetBtnInfo()
{
    m_CloseButton.ChangeButtonImgState(true, IMAGE_IGS_EXIT_BTN, false);
    m_CloseButton.ChangeButtonInfo(m_Pos.x + IMAGE_IGS_EXIT_BTN_POS_X, m_Pos.y + IMAGE_IGS_EXIT_BTN_POS_Y, IMAGE_IGS_EXIT_BTN_WIDTH, IMAGE_IGS_EXIT_BTN_HEIGHT);
    m_CloseButton.ChangeToolTipText(GlobalText[1002], true);
    m_ListBoxTabButton.CreateRadioGroup(IGS_TOTAL_LISTBOX, IMAGE_IGS_LEFT_TAB);
    m_ListBoxTabButton.ChangeRadioButtonInfo(true, m_Pos.x + IMAGE_IGS_TAB_BTN_POS_X, m_Pos.y + IMAGE_IGS_TAB_BTN_POS_Y, IMAGE_IGS_TAB_BTN_WIDTH, IMAGE_IGS_TAB_BTN_HEIGHT, IMAGE_IGS_TAB_BTN_DISTANCE);
    m_ListBoxTabButton.ChangeButtonState(SEASON3B::BUTTON_STATE_DOWN, 0);
    m_ListBoxTabButton.ChangeButtonState(IGS_SAFEKEEPING_LISTBOX, BITMAP_UNKNOWN, SEASON3B::BUTTON_STATE_UP, 0);
    m_ListBoxTabButton.ChangeButtonState(IGS_PRESENTBOX_LISTBOX, BITMAP_UNKNOWN, SEASON3B::BUTTON_STATE_UP, 0);
    m_ListBoxTabButton.ChangeButtonState(IGS_PRESENTBOX_LISTBOX, IMAGE_IGS_RIGHT_TAB, SEASON3B::BUTTON_STATE_DOWN, 0);

   std::wstring strText;
    std::list<std::wstring> TextList;
    strText = GlobalText[2888];
    TextList.push_back(strText);
    strText = GlobalText[2889];
    TextList.push_back(strText);

    m_ListBoxTabButton.ChangeRadioText(TextList);
    m_ListBoxTabButton.ChangeFrame(IGS_SAFEKEEPING_LISTBOX);

    for (int i = 0; i < INGAMESHOP_DISPLAY_ITEMLIST_SIZE; i++)
    {
        m_ViewDetailButton[i].ChangeButtonImgState(true, IMAGE_IGS_VIEWDETAIL_BTN, true, false, true);
        m_ViewDetailButton[i].ChangeButtonInfo(IMAGE_IGS_VIEWDETAIL_BTN_POS_X + ((i % IGS_NUM_ITEMS_WIDTH) * IMAGE_IGS_VIEWDETAIL_BTN_DISTANCE_X), IMAGE_IGS_VIEWDETAIL_BTN_POS_Y + ((i / IGS_NUM_ITEMS_HEIGHT) * IMAGE_IGS_VIEWDETAIL_BTN_DISTANCE_Y), IMAGE_IGS_VIEWDETAIL_BTN_WIDTH, IMAGE_IGS_VIEWDETAIL_BTN_HEIGHT);
        m_ViewDetailButton[i].MoveTextPos(0, -1);
        m_ViewDetailButton[i].ChangeText(GlobalText[2886]);
    }

    m_CashGiftButton.ChangeButtonImgState(true, IMAGE_IGS_ITEMGIFT_BTN, true);
    m_CashGiftButton.ChangeButtonInfo(m_Pos.x + IMAGE_IGS_ITEMGIFT_BTN_POS_X, m_Pos.y + IMAGE_IGS_ICON_BTN_POS_Y, IMAGE_IGS_ICON_BTN_WIDTH, IMAGE_IGS_ICON_BTN_HEIGHT);
    m_CashGiftButton.ChangeToolTipText(GlobalText[2939]);
    m_CashChargeButton.ChangeButtonImgState(true, IMAGE_IGS_CASHGIFT_BTN, true);
    m_CashChargeButton.ChangeButtonInfo(m_Pos.x + IMAGE_IGS_CASHGIFT_BTN_POS_X, m_Pos.y + IMAGE_IGS_ICON_BTN_POS_Y, IMAGE_IGS_ICON_BTN_WIDTH, IMAGE_IGS_ICON_BTN_HEIGHT);
    m_CashChargeButton.ChangeToolTipText(GlobalText[2940]);

    m_CashRefreshButton.ChangeButtonImgState(true, IMAGE_IGS_REFRESH_BTN, true);
    m_CashRefreshButton.ChangeButtonInfo(m_Pos.x + IMAGE_IGS_REFRESH_BTN_POS_X, m_Pos.y + IMAGE_IGS_ICON_BTN_POS_Y, IMAGE_IGS_ICON_BTN_WIDTH, IMAGE_IGS_ICON_BTN_HEIGHT);
    m_CashRefreshButton.ChangeToolTipText(GlobalText[2941]);

    m_UseButton.ChangeButtonImgState(true, IMAGE_IGS_VIEWDETAIL_BTN, true, false, true);
    m_UseButton.ChangeButtonInfo(m_Pos.x + IMAGE_IGS_USE_BTN_POS_X, m_Pos.y + IMAGE_IGS_USE_BTN_POS_Y, IMAGE_IGS_VIEWDETAIL_BTN_WIDTH, IMAGE_IGS_VIEWDETAIL_BTN_HEIGHT);
    m_UseButton.MoveTextPos(0, -1);
    m_UseButton.ChangeText(GlobalText[2887]);

    m_PrevButton.ChangeButtonImgState(true, IMAGE_IGS_PAGE_LEFT, true);
    m_PrevButton.ChangeButtonInfo(m_Pos.x + IMAGE_IGS_PAGE_LEFT_POS_X, m_Pos.y + IMAGE_IGS_PAGE_BUTTON_POS_Y, IMAGE_IGS_PAGE_BTN_WIDTH, IMAGE_IGS_PAGE_BTN_HEIGHT);

    // next
    m_NextButton.ChangeButtonImgState(true, IMAGE_IGS_PAGE_RIGHT, true);
    m_NextButton.ChangeButtonInfo(m_Pos.x + IMAGE_IGS_PAGE_RIGHT_POS_X, m_Pos.y + IMAGE_IGS_PAGE_BUTTON_POS_Y, IMAGE_IGS_PAGE_BTN_WIDTH, IMAGE_IGS_PAGE_BTN_HEIGHT);

    // Storage Page prev
    m_StoragePrevButton.ChangeButtonImgState(true, IMAGE_IGS_STORAGE_PAGE_LEFT, true);
    m_StoragePrevButton.ChangeButtonInfo(m_Pos.x + IMAGE_IGS_STORAGE_PAGE_LEFT_POS_X - 12, m_Pos.y + IMAGE_IGS_STORAGE_PAGE_BTN_POS_Y - 3, IMGAE_IGS_STORAGE_PAGE_BTN_WIDTH, IMGAE_IGS_STORAGE_PAGE_BTN_HEIGHT);

    // Storage Page next
    m_StorageNextButton.ChangeButtonImgState(true, IMAGE_IGS_STORAGE_PAGE_RIGHT, true);
    m_StorageNextButton.ChangeButtonInfo(m_Pos.x + IMAGE_IGS_STORAGE_PAGE_RIGHT_POS_X + 10, m_Pos.y + IMAGE_IGS_STORAGE_PAGE_BTN_POS_Y - 3, IMGAE_IGS_STORAGE_PAGE_BTN_WIDTH, IMGAE_IGS_STORAGE_PAGE_BTN_HEIGHT);
}

bool CNewUIInGameShop::Update()
{
    if (IsVisible() == false)
        return true;

    return true;
}

bool CNewUIInGameShop::UpdateMouseEvent()
{
    if (IsVisible() == false)
        return true;

    if (BtnProcess())
        return false;

    if (UpdateBanner())
        return false;

    if (SEASON3B::CheckMouseIn(m_Pos.x, m_Pos.y, IMAGE_IGS_BACK_WIDTH, IMAGE_IGS_BACK_HEIGHT))
    {
        m_StorageItemListBox.DoAction();

        if (SEASON3B::IsPress(VK_RBUTTON))
        {
            MouseRButton = false;
            MouseRButtonPop = false;
            MouseRButtonPush = false;
            return false;
        }

        if (SEASON3B::IsNone(VK_LBUTTON) == false)
        {
            return false;
        }

        return false;
    }

    return true;
}

bool CNewUIInGameShop::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INGAMESHOP) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            SocketClient->ToGameServer()->SendCashShopOpenState(1);
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_INGAMESHOP);

            return false;
        }
    }
    return true;
}

bool CNewUIInGameShop::IsInGameShopOpen()
{
    g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt CallStack - CNewUIInGameShop::IsInGameShopOpen()");
    if (Hero->Movement)
        return false;

    if (!(Hero->SafeZone) && !(WD_0LORENCIA == gMapManager.WorldActive && WD_3NORIA == gMapManager.WorldActive && WD_2DEVIAS == gMapManager.WorldActive && WD_51HOME_6TH_CHAR == gMapManager.WorldActive))
    {
        CMsgBoxIGSCommon* pMsgBox = NULL;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[3028], GlobalText[3051]);
        g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt Return - false <%s>", GlobalText[3051]);
        return false;
    }

    if (g_InGameShopSystem->IsShopOpen() == false)
    {
        CMsgBoxIGSCommon* pMsgBox = NULL;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[3028], GlobalText[3035]);
        g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt Return - false <%s>", GlobalText[3035]);
        return false;
    }
    g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt Return - true");
    return true;
}

bool CNewUIInGameShop::IsInGameShop()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INGAMESHOP))
        return true;
    else
        return false;
}

void CNewUIInGameShop::InitBanner(wchar_t* pszFileName, wchar_t* pszBannerURL)
{
    ReleaseBanner();

    if (pszFileName == NULL)
        return;

    if (pszBannerURL[0] != '#')
    {
        m_bBannerLink = true;
    }

    if (Bitmaps.Convert_Format(pszFileName) == false)
        return;

    if (LoadBitmap(pszFileName, IMAGE_IGS_BANNER, GL_LINEAR, GL_CLAMP, true, true) == true)
    {
        m_bLoadBanner = true;

        wcscpy(m_szBannerURL, pszBannerURL);
    }
}

void CNewUIInGameShop::RenderBanner()
{
    if (m_bLoadBanner == false)
        return;

    RenderImage(IMAGE_IGS_BANNER, IMAGE_IGS_BANNER_POS_X, IMAGE_IGS_BANNER_POS_Y, IMAGE_IGS_BANNER_WIDTH, IMAGE_IGS_BANNER_HEIGHT);
}

bool CNewUIInGameShop::UpdateBanner()
{
    if (m_bLoadBanner == false || m_bBannerLink == false)
        return false;

    if ((SEASON3B::IsPress(VK_LBUTTON))
        && (SEASON3B::CheckMouseIn(IMAGE_IGS_BANNER_POS_X, IMAGE_IGS_BANNER_POS_Y, IMAGE_IGS_BANNER_WIDTH, IMAGE_IGS_BANNER_HEIGHT)))
    {
        leaf::OpenExplorer(m_szBannerURL);
        return true;
    }
    return false;
}

void CNewUIInGameShop::ReleaseBanner()
{
    if (m_bLoadBanner == false)
        return;

    DeleteBitmap(IMAGE_IGS_BANNER);

    m_bLoadBanner = false;
}

void CNewUIInGameShop::OpeningProcess()
{
    g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt CallStack - CNewUIInGameShop::OpeningProcess()");
    PlayBuffer(SOUND_CLICK01);
    g_InGameShopSystem->Initalize();
    g_InGameShopSystem->SelectZone(0);
    InitZoneBtn();
    g_InGameShopSystem->SelectCategory(0);
    InitCategoryBtn();
    g_InGameShopSystem->SetRequestEventPackge();
}

void CNewUIInGameShop::ClosingProcess()
{
    PlayBuffer(SOUND_CLICK01);
    m_ListBoxTabButton.ChangeFrame(IGS_SAFEKEEPING_LISTBOX);
    ClearAllStorageItem();
}

void CNewUIInGameShop::InitZoneBtn()
{
    m_ZoneButton.UnRegisterRadioButton();

    if (g_InGameShopSystem->GetSizeZones() == 0)
        return;

    m_ZoneButton.UnRegisterRadioButton();
    m_ZoneButton.CreateRadioGroup(g_InGameShopSystem->GetSizeZones(), IMAGE_IGS_ZONE_BTN);
    m_ZoneButton.ChangeRadioButtonInfo(true, m_Pos.x + IMAGE_IGS_ZONE_BTN_POS_X, m_Pos.y + IMAGE_IGS_ZONE_BTN_POS_Y, IMAGE_IGS_ZONE_BTN_WIDTH, IMAGE_IGS_ZONE_BTN_HEIGHT);
    m_ZoneButton.SetFont(g_hFontBold);
    m_ZoneButton.ChangeRadioText(g_InGameShopSystem->GetZoneName());
    m_ZoneButton.ChangeFrame(0);
}

void CNewUIInGameShop::InitCategoryBtn()
{
    m_CategoryButton.UnRegisterRadioButton();

    if (g_InGameShopSystem->GetSizeCategoriesAsSelectedZone() == 0)
        return;

    m_CategoryButton.UnRegisterRadioButton();
    m_CategoryButton.CreateRadioGroup(g_InGameShopSystem->GetSizeCategoriesAsSelectedZone(), IMAGE_IGS_CATEGORY_BTN, true);
    m_CategoryButton.ChangeRadioButtonInfo(false, m_Pos.x + IMAGE_IGS_CATEGORY_BTN_POS_X, m_Pos.y + IMAGE_IGS_CATEGORY_BTN_POS_Y, IMAGE_IGS_CATEGORY_BTN_WIDTH, IMAGE_IGS_CATEGORY_BTN_HEIGHT, IMAGE_IGS_CATEGORY_BTN_DISTANCE);
    m_CategoryButton.ChangeButtonState(SEASON3B::BUTTON_STATE_DOWN, 2);
    m_CategoryButton.SetFont(g_hFontBold);
    m_CategoryButton.ChangeRadioText(g_InGameShopSystem->GetCategoryName());
    m_CategoryButton.ChangeFrame(0);
}

void CNewUIInGameShop::AddStorageItem(int iStorageSeq, int iStorageItemSeq, int iStorageGroupCode, int iProductSeq, int iPriceSeq, int iCashPoint, wchar_t chItemType, wchar_t* pszUserName /* = NULL */, wchar_t* pszMessage /* = NULL */)
{
    int iValue = -1;
    wchar_t szText[MAX_TEXT_LENGTH] = { '\0', };
    IGS_StorageItem Item;

    Item.m_bIsSelected = FALSE;
    Item.m_iStorageSeq = iStorageSeq;
    Item.m_iStorageItemSeq = iStorageItemSeq;
    Item.m_iStorageGroupCode = iStorageGroupCode;
    Item.m_iProductSeq = iProductSeq;
    Item.m_iPriceSeq = iPriceSeq;
    Item.m_iCashPoint = iCashPoint;
    Item.m_iNum = 1;
    Item.m_szType = chItemType;
    Item.m_wItemCode = -1;

    if (pszUserName == NULL)
    {
        Item.m_szSendUserName[0] = '\0';
    }
    else
    {
        wcscpy(Item.m_szSendUserName, pszUserName);
    }

    if (pszMessage == NULL)
    {
        Item.m_szMessage[0] = '\0';
    }
    else
    {
        wcscpy(Item.m_szMessage, pszMessage);
    }

    if (chItemType == 'C' || chItemType == 'c')
    {
        wchar_t szValue[MAX_TEXT_LENGTH] = { '\0', };
        ConvertGold(iCashPoint, szValue);
        // Name
        swprintf(Item.m_szName, GlobalText[3050], szValue);

        // Num
        swprintf(Item.m_szNum, GlobalText[3043], szValue);
        Item.m_iNum = iCashPoint;

        // Period
        swprintf(Item.m_szPeriod, L"-");
    }
    else if (chItemType == 'P' || chItemType == 'p')
    {
        if (iPriceSeq > 0)
        {
            // Name
            if (g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_ITEMNAME, iValue, Item.m_szName) == false)
            {
                swprintf(Item.m_szName, L"aaa");
            }

            g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_NUM, iValue, szText);
            if (iValue > 0)
            {
                swprintf(Item.m_szNum, L"%d %s", iValue, szText);
                Item.m_iNum = iValue;
            }
            else
            {
                swprintf(Item.m_szNum, L"-");
            }

            // Period
            g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_USE_LIMIT_PERIOD, iValue, szText);
            if (iValue > 0)
            {
                swprintf(Item.m_szPeriod, L"%d %s", iValue, szText);
            }
            else
            {
                swprintf(Item.m_szPeriod, L"-");
            }

            g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_ITEMCODE, iValue, szText);
            Item.m_wItemCode = iValue;
        }
        else
        {
            if (g_InGameShopSystem->GetProductInfoFromProductSeq(iProductSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_ITEMNAME, iValue, Item.m_szName) == false)
                return;

            // Num
            g_InGameShopSystem->GetProductInfoFromProductSeq(iProductSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_NUM, iValue, szText);
            if (iValue > 0)
            {
                swprintf(Item.m_szNum, L"%d %s", iValue, szText);
                Item.m_iNum = iValue;
            }
            else
            {
                swprintf(Item.m_szNum, L"-");
            }

            // Period
            g_InGameShopSystem->GetProductInfoFromProductSeq(iProductSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_USE_LIMIT_PERIOD, iValue, szText);
            if (iValue > 0)
            {
                swprintf(Item.m_szPeriod, L"%d %s", iValue, szText);
            }
            else
            {
                swprintf(Item.m_szPeriod, L"-");
            }

            g_InGameShopSystem->GetProductInfoFromProductSeq(iProductSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_ITEMCODE, iValue, szText);
            Item.m_wItemCode = iValue;
        }
    }
    else
    {
        return;
    }

    m_iStorageCurrentPageReceiveItemCnt++;

    m_StorageItemListBox.AddText(Item);

    if (m_iStorageCurrentPageReceiveItemCnt >= m_iStorageCurrentPageItemCnt)
    {
        if (m_iSelectedStorageItemIndex > m_iStorageCurrentPageItemCnt)
        {
            m_StorageItemListBox.SLSetSelectLine(m_iStorageCurrentPageItemCnt);
        }
        else
        {
            m_StorageItemListBox.SLSetSelectLine(m_iSelectedStorageItemIndex);
        }
    }
}

void CNewUIInGameShop::ClearAllStorageItem()
{
    m_iStorageTotalItemCnt = 0;
    m_iStorageCurrentPageItemCnt = 0;
    m_iStorageTotalPage = 0;
    m_iStorageCurrentPage = 0;
    m_iStorageCurrentPageReceiveItemCnt = 0;
    m_StorageItemListBox.Clear();
}

void CNewUIInGameShop::InitStorage(int iTotalItemCnt, int iCurrentPageItemCnt, int iTotalPage, int iCurrentPage)
{
    ClearAllStorageItem();

    m_iStorageTotalItemCnt = iTotalItemCnt;
    m_iStorageCurrentPageItemCnt = iCurrentPageItemCnt;
    m_iStorageTotalPage = iTotalPage;

    if (m_iStorageTotalPage > 0)
    {
        m_iStorageCurrentPage = iCurrentPage;
    }
    else
    {
        m_iStorageCurrentPage = 0;
    }

    if (m_iSelectedStorageItemIndex == 0 || m_bRequestCurrentPage == false)
    {
        m_iSelectedStorageItemIndex = iCurrentPageItemCnt;
    }

    m_bRequestCurrentPage = false;
}

char CNewUIInGameShop::GetCurrentStorageCode()
{
    char szCode;
    switch (m_ListBoxTabButton.GetCurButtonIndex())
    {
    case IGS_SAFEKEEPING_LISTBOX:
        szCode = 'S';
        break;
    case IGS_PRESENTBOX_LISTBOX:
        szCode = 'G';
        break;
    default:
        szCode = 'Z';
        break;
    }
    return szCode;
}

void CNewUIInGameShop::StoragePrevPage()
{
    if (m_iStorageCurrentPage > 1)
    {
        char szCode = GetCurrentStorageCode();
        m_iSelectedStorageItemIndex = 0;
        m_bRequestCurrentPage = true;
        SocketClient->ToGameServer()->SendCashShopStorageListRequest(m_iStorageCurrentPage - 1, szCode);
    }
}

void CNewUIInGameShop::StorageNextPage()
{
    if (m_iStorageCurrentPage < m_iStorageTotalPage)
    {
        char szCode = GetCurrentStorageCode();
        m_iSelectedStorageItemIndex = 0;
        m_bRequestCurrentPage = true;
        SocketClient->ToGameServer()->SendCashShopStorageListRequest(m_iStorageCurrentPage + 1, szCode);
    }
}

void CNewUIInGameShop::UpdateStorageItemList()
{
    char szCode = GetCurrentStorageCode();
    int iSelectLineIndex = m_StorageItemListBox.SLGetSelectLineNum();
    m_bRequestCurrentPage = true;

    if ((m_iStorageCurrentPageItemCnt == 1) && (m_iStorageTotalPage > 1))
    {
        m_iSelectedStorageItemIndex = 1;
        SocketClient->ToGameServer()->SendCashShopStorageListRequest(m_iStorageCurrentPage - 1, szCode);
    }
    else if (iSelectLineIndex == 1)
    {
        m_iSelectedStorageItemIndex = iSelectLineIndex;
        SocketClient->ToGameServer()->SendCashShopStorageListRequest(m_iStorageCurrentPage, szCode);
    }
    else if (m_iStorageCurrentPageItemCnt < IGS_STORAGE_TOTAL_ITEM_PER_PAGE)
    {
        m_iSelectedStorageItemIndex = (iSelectLineIndex - 1);
        SocketClient->ToGameServer()->SendCashShopStorageListRequest(m_iStorageCurrentPage, szCode);
    }
    else
    {
        m_iSelectedStorageItemIndex = iSelectLineIndex;
        SocketClient->ToGameServer()->SendCashShopStorageListRequest(m_iStorageCurrentPage, szCode);
    }
}

void CNewUIInGameShop::LoadImages()
{
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_IGS_EXIT_BTN, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\Ingame_shopback.jpg", IMAGE_IGS_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Bt01.tga", IMAGE_IGS_CATEGORY_BTN, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Deco_Center.tga", IMAGE_IGS_CATEGORY_DECO_MIDDLE, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Deco_Dn.tga", IMAGE_IGS_CATEGORY_DECO_DOWN, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\ingame_Tab01.tga", IMAGE_IGS_LEFT_TAB, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\ingame_Tab02.tga", IMAGE_IGS_RIGHT_TAB, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Tab_Up.tga", IMAGE_IGS_ZONE_BTN, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Bt_Gift.tga", IMAGE_IGS_ITEMGIFT_BTN, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Bt_Cash.tga", IMAGE_IGS_CASHGIFT_BTN, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Bt_Reset.tga", IMAGE_IGS_REFRESH_BTN, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Bt03.tga", IMAGE_IGS_VIEWDETAIL_BTN, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Itembox_logo.tga", IMAGE_IGS_ITEMBOX_LOGO, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\ingame_Bt_page_L.tga", IMAGE_IGS_PAGE_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\ingame_Bt_page_R.tga", IMAGE_IGS_PAGE_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\IGS_Storage_Page.tga", IMAGE_IGS_STORAGE_PAGE, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\IGS_Storage_Page_Left.tga", IMAGE_IGS_STORAGE_PAGE_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\IGS_Storage_Page_Right.tga", IMAGE_IGS_STORAGE_PAGE_RIGHT, GL_LINEAR);
}

void CNewUIInGameShop::UnloadImages()
{
    DeleteBitmap(IMAGE_IGS_EXIT_BTN);
    DeleteBitmap(IMAGE_IGS_BACK);
    DeleteBitmap(IMAGE_IGS_CATEGORY_BTN);
    DeleteBitmap(IMAGE_IGS_CATEGORY_DECO_MIDDLE);
    DeleteBitmap(IMAGE_IGS_CATEGORY_DECO_DOWN);
    DeleteBitmap(IMAGE_IGS_LEFT_TAB);
    DeleteBitmap(IMAGE_IGS_RIGHT_TAB);
    DeleteBitmap(IMAGE_IGS_ZONE_BTN);
    DeleteBitmap(IMAGE_IGS_ITEMGIFT_BTN);
    DeleteBitmap(IMAGE_IGS_CASHGIFT_BTN);
    DeleteBitmap(IMAGE_IGS_REFRESH_BTN);
    DeleteBitmap(IMAGE_IGS_VIEWDETAIL_BTN);
    DeleteBitmap(IMAGE_IGS_ITEMBOX_LOGO);
    DeleteBitmap(IMAGE_IGS_PAGE_LEFT);
    DeleteBitmap(IMAGE_IGS_PAGE_RIGHT);
    DeleteBitmap(IMAGE_IGS_STORAGE_PAGE);
    DeleteBitmap(IMAGE_IGS_STORAGE_PAGE_LEFT);
    DeleteBitmap(IMAGE_IGS_STORAGE_PAGE_RIGHT);
}

#endif //PBG_ADD_INGAMESHOP_UI_ITEMSHOP