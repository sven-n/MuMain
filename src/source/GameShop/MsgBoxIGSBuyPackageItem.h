// MsgBoxIGSBuyPackageItem.h: interface for the CMsgBoxIGSBuyPackageItem class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MSGBOXIGSBUYPACKAGEITEM_H__42A6E746_9439_4E71_9C86_7CDF5F96AFE3__INCLUDED_)
#define AFX_MSGBOXIGSBUYPACKAGEITEM_H__42A6E746_9439_4E71_9C86_7CDF5F96AFE3__INCLUDED_

#pragma once

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "UIControls.h"
#include "NewUIMessageBox.h"
#include "NewUICommonMessageBox.h"
#include "NewUIButton.h"
#include "./GameShop/ShopListManager/ShopPackage.h"

using namespace SEASON3B;

class CMsgBoxIGSBuyPackageItem : public CNewUIMessageBoxBase, public INewUI3DRenderObj
{
public:
    enum IMAGE_IGS_BUY_PACKAGE_ITEM
    {
        IMAGE_IGS_FRAME = BITMAP_IGS_MSGBOX_BUY_PACKAGE_ITEM,
        IMAGE_IGS_BUTTON = BITMAP_IGS_MSGBOX_BUTTON,
    };

    enum IMAGESIZE_IGS_BUY_PACKAGE_ITEM
    {
        IMAGE_IGS_WINDOW_WIDTH = 640,
        IMAGE_IGS_WINDOW_HEIGHT = 429,
        IMAGE_IGS_FRAME_WIDTH = 198,
        IMAGE_IGS_FRAME_HEIGHT = 291,
        IMAGE_IGS_BTN_WIDTH = 52,
        IMAGE_IGS_BTN_HEIGHT = 26,
    };

    enum IGS_BUY_PACKAGE_ITEM_POS
    {
        IGS_BTN_BUY_POS_X = 18,
        IGS_BTN_PRESENT_POS_X = 74,
        IGS_BTN_CANCEL_POS_X = 130,
        IGS_BTN_POS_Y = 253,
        IGS_TEXT_TITLE_POS_Y = 10,
        IGS_TEXT_NAME_POS_X = 5,
        IGS_TEXT_NAME_POS_Y = 100,
        IGS_TEXT_NAME_WIDTH = 196,
        IGS_TEXT_PRICE_POS_X = 118,
        IGS_TEXT_PRICE_POX_Y = 229,
        IGS_TEXT_PRICE_WIDTH = 66,
        IGS_LISTBOX_POS_X = 14,
        IGS_LISTBOX_POS_Y = 216,
        IGS_LISTBOX_WIDTH = 158,
        IGS_3DITEM_POS_X = 50,
        IGS_3DITEM_POS_Y = 34,
        IGS_3DITEM_WIDTH = 96,
        IGS_3DITEM_HEIGHT = 60,
    };

public:
    CMsgBoxIGSBuyPackageItem();
    virtual ~CMsgBoxIGSBuyPackageItem();

    bool Create(float fPriority = 3.f);
    void Release();
    bool Update();
    bool Render();

    bool IsVisible() const;

    void Render3D();

    void Initialize(CShopPackage* pPackage);

    static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    static CALLBACK_RESULT BuyBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    static CALLBACK_RESULT PresentBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

private:
    void SetAddCallbackFunc();
    void SetButtonInfo();
    void RenderFrame();
    void RenderTexts();
    void RenderButtons();

    void CreateListBox();
    void RenderListBox();
    void ListBoxDoAction();
    void ReleaseListBox();

#ifdef LEM_FIX_WARNINGMSG_BUYITEM
    bool Add_WarningMsgBuyItem(int _nItemIndex);
#endif	// LEM_FIX_WARNINGMSG_BUYITEM

    void LoadImages();
    void UnloadImages();

private:
    CNewUIMessageBoxButton m_BtnBuy;
    CNewUIMessageBoxButton m_BtnPresent;
    CNewUIMessageBoxButton m_BtnCancel;
    CUIBuyingListBox	m_PackageInfo;

    int		m_iPackageSeq;
    int		m_iDisplaySeq;
    WORD	m_wItemCode;
    int		m_iCashType;

    wchar_t m_szPackageName[MAX_TEXT_LENGTH];
    wchar_t m_szPrice[MAX_TEXT_LENGTH];
    wchar_t m_szPeriod[MAX_TEXT_LENGTH];
    wchar_t m_szDescription[UIMAX_TEXT_LINE][MAX_TEXT_LENGTH];
};

class CMsgBoxBuyPackageItemLayout : public TMsgBoxLayout<CMsgBoxIGSBuyPackageItem>
{
public:
    bool SetLayout();
};

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
#endif // !defined(AFX_MSGBOXIGSBUYPACKAGEITEM_H__42A6E746_9439_4E71_9C86_7CDF5F96AFE3__INCLUDED_)
