// MsgBoxIGSBuySelectItem.h: interface for the CMsgBoxIGSBuySelectItem class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MSGBOXIGSBUYSELECTITEM_H__96137D00_144C_4E10_B335_383E5DAB5D50__INCLUDED_)
#define AFX_MSGBOXIGSBUYSELECTITEM_H__96137D00_144C_4E10_B335_383E5DAB5D50__INCLUDED_

#pragma once

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "UIControls.h"
#include "NewUICommonMessageBox.h"
#include "./GameShop/ShopListManager/ShopPackage.h"

using namespace SEASON3B;

class CMsgBoxIGSBuySelectItem : public CNewUIMessageBoxBase, public INewUI3DRenderObj
{
public:
    CMsgBoxIGSBuySelectItem();
    virtual ~CMsgBoxIGSBuySelectItem();

private:
    enum IMAGE_IGS_BUY_SELECT_ITEM
    {
        IMAGE_IGS_MGSBOX_BACK = BITMAP_IGS_MSGBOX_BUY_SELECT_ITEM,
        IMAGE_IGS_BUTTON = BITMAP_IGS_MSGBOX_BUTTON,
    };

    enum IMAGESIZE_IGS_BUY_SELECT_ITEM
    {
        IMAGE_IGS_WINDOW_WIDTH = 640,
        IMAGE_IGS_WINDOW_HEIGHT = 429,
        IMAGE_IGS_FRAME_WIDTH = 215,
        IMAGE_IGS_FRAME_HEIGHT = 346,
        IMAGE_IGS_BTN_WIDTH = 52,
        IMAGE_IGS_BTN_HEIGHT = 26,
    };

    enum IGS_BUY_SELECT_ITEM
    {
        IGS_BTN_BUY_POS_X = 26,
        IGS_BTN_PRESENT_POS_X = 82,
        IGS_BTN_CANCEL_POS_X = 138,
        IGS_BTN_POS_Y = 309,
        IGS_TEXT_TITLE_POS_Y = 10,
        IGS_TEXT_NAME_POS_X = 5,
        IGS_TEXT_NAME_POS_Y = 100,
        IGS_TEXT_ATTR_POS_X = 15,
        IGS_TEXT_ATTR_POS_Y = 118,
        IGS_TEXT_ATTR_WIDTH = 194,
        IGS_TEXT_PRICE_POS_X = 129,
        IGS_TEXT_PRICE_POX_Y = 288,
        IGS_TEXT_PRICE_WIDTH = 66,
        IGS_TEXT_DISCRIPTION_WIDTH = 185,
        IGS_LISTBOX_POS_X = 17,
        IGS_LISTBOX_POS_Y = 277,
        IGS_3DITEM_POS_X = 60,
        IGS_3DITEM_POS_Y = 33,
        IGS_3DITEM_WIDTH = 96,
        IGS_3DITEM_HEIGHT = 60,
    };

public:
    bool Create(float fPriority = 3.f);
    void Release();
    bool Update();
    bool Render();

    void Initialize(CShopPackage* pPackage);

    void Render3D();
    bool IsVisible() const;

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
    void LoadImages();
    void UnloadImages();
    void CreateListBox();
    void RenderListBox();
    void ReleaseListBox();
    void ListBoxDoAction();
    void CMsgBoxIGSBuySelectItem::AddData(int iPackageSeq, int iDisplaySeq, int iPriceSeq, int iProductSeq, wchar_t* pszPriceUnit, int iCashType);

private:

    CNewUIMessageBoxButton m_BtnBuy;
    CNewUIMessageBoxButton m_BtnPresent;
    CNewUIMessageBoxButton m_BtnCancel;
    CUIPackCheckBuyingListBox m_SelectBuyListBox;

    int		m_iPackageSeq;
    int		m_iDisplaySeq;
    WORD	m_wItemCode;
    int		m_iDescriptionLine;

    wchar_t m_szPackageName[MAX_TEXT_LENGTH];
    wchar_t m_szPrice[MAX_TEXT_LENGTH];
    wchar_t m_szDescription[UIMAX_TEXT_LINE][MAX_TEXT_LENGTH];
};

class CMsgBoxIGSBuySelectItemLayout : public TMsgBoxLayout<CMsgBoxIGSBuySelectItem>
{
public:
    bool SetLayout();
};

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

#endif // !defined(AFX_MSGBOXIGSBUYSELECTITEM_H__96137D00_144C_4E10_B335_383E5DAB5D50__INCLUDED_)
