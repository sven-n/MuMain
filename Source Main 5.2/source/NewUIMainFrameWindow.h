// NewUIMainFrameWindow.h: interface for the CNewUIMainFrameWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_)
#define AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "ZzzTexture.h"
#include "NewUI3DRenderMng.h"
#include "NewUIButton.h"

namespace SEASON3B
{
    enum
    {
        HOTKEY_Q = 0,
        HOTKEY_W,
        HOTKEY_E,
        HOTKEY_R,
        HOTKEY_COUNT
    };

    enum
    {
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
        MAINFRAME_BTN_PARTCHARGE = 0,
#endif //defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
        MAINFRAME_BTN_CHAINFO,
        MAINFRAME_BTN_MYINVEN,
        MAINFRAME_BTN_FRIEND,
        MAINFRAME_BTN_WINDOW,
    };

    enum KINDOFSKILL
    {
        KOS_COMMAND = 1,
        KOS_SKILL1,
        KOS_SKILL2,
        KOS_SKILL3,
    };

    class CNewUIItemHotKey
    {
    public:
        CNewUIItemHotKey();
        virtual ~CNewUIItemHotKey();

        bool UpdateKeyEvent();

        void SetHotKey(int iHotKey, int iItemType, int iItemLevel);
        int GetHotKey(int iHotKey);
        int GetHotKeyLevel(int iHotKey);
        void UseItemRButton();
        void RenderItems();
        void RenderItemCount();

    private:
        int GetHotKeyItemIndex(int iType, bool bItemCount = false);
        bool GetHotKeyCommonItem(IN int iHotKey, OUT int& iStart, OUT int& iEnd);
        int GetHotKeyItemCount(int iType);

        int m_iHotKeyItemType[HOTKEY_COUNT];
        int m_iHotKeyItemLevel[HOTKEY_COUNT];
    };

    class CNewUISkillList : public CNewUIObj
    {
        enum
        {
            SKILLHOTKEY_COUNT = 10
        };
        enum EVENT_STATE
        {
            EVENT_NONE = 0,

            // currentskill
            EVENT_BTN_HOVER_CURRENTSKILL,
            EVENT_BTN_DOWN_CURRENTSKILL,

            // skillhotkey
            EVENT_BTN_HOVER_SKILLHOTKEY,
            EVENT_BTN_DOWN_SKILLHOTKEY,

            // skilllist
            EVENT_BTN_HOVER_SKILLLIST,
            EVENT_BTN_DOWN_SKILLLIST,
        };

    public:
        enum IMAGE_LIST
        {
            IMAGE_SKILL1 = BITMAP_INTERFACE_NEW_SKILLICON_BEGIN,
            IMAGE_SKILL2,
            IMAGE_COMMAND,
            IMAGE_SKILL3,
            IMAGE_SKILLBOX,
            IMAGE_SKILLBOX_USE,
            IMAGE_NON_SKILL1,
            IMAGE_NON_SKILL2,
            IMAGE_NON_COMMAND,
            IMAGE_NON_SKILL3,
        };

        CNewUISkillList();
        virtual ~CNewUISkillList();

        bool Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng);
        void Release();

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        void RenderSkillInfo();
        float GetLayerDepth();		// 10.6f

        WORD GetHeroPriorSkill();
        void SetHeroPriorSkill(BYTE bySkill);

        void Reset();

        void SetHotKey(int iHotKey, int iSkillType);
        int GetHotKey(int iHotKey);
        int GetSkillIndex(int iSkillType);
        void RenderCurrentSkillAndHotSkillList();

        bool IsSkillListUp();

        static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);

    private:
        void LoadImages();
        void UnloadImages();
        bool IsArrayUp(BYTE bySkill);
        bool IsArrayIn(BYTE bySkill);
        void UseHotKey(int iHotKey);

        void RenderSkillIcon(int iIndex, float x, float y, float width, float height);
        void RenderSkillDelay(int iIndex, float x, float y, float width, float height);
        void RenderPetSkill();

        void ResetMouseLButton();

    private:
        CNewUIManager* m_pNewUIMng;
        CNewUI3DRenderMng* m_pNewUI3DRenderMng;

        bool m_bHotKeySkillListUp;
        int m_iHotKeySkillType[SKILLHOTKEY_COUNT];

        bool m_bSkillList;

        bool m_bRenderSkillInfo;
        int m_iRenderSkillInfoType;
        int m_iRenderSkillInfoPosX;
        int m_iRenderSkillInfoPosY;

        EVENT_STATE m_EventState;
        WORD m_wHeroPriorSkill;
    };

    class CNewUIMainFrameWindow : public CNewUIObj, public INewUI3DRenderObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_MENU_1 = BITMAP_INTERFACE_NEW_MAINFRAME_BEGIN,	// newui_menu01.jpg
            IMAGE_MENU_2,		// newui_menu02.jpg
            IMAGE_MENU_3,		// newui_menu03.jpg
            IMAGE_MENU_2_1,
            IMAGE_GAUGE_BLUE,	// newui_menu_blue.tga
            IMAGE_GAUGE_GREEN,	// newui_menu_green.tga
            IMAGE_GAUGE_RED,	// newui_menu_red.tga
            IMAGE_GAUGE_AG,		// newui_menu_AG.tga
            IMAGE_GAUGE_SD,		// newui_menu_SD.tga
            IMAGE_GAUGE_EXBAR,	// newui_Exbar.jpg
            IMAGE_MASTER_GAUGE_BAR,	// Exbar_Master.jpg
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
            IMAGE_MENU_BTN_CSHOP,
#endif //defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
            IMAGE_MENU_BTN_CHAINFO,
            IMAGE_MENU_BTN_MYINVEN,
            IMAGE_MENU_BTN_FRIEND,
            IMAGE_MENU_BTN_WINDOW,
        };

        CNewUIMainFrameWindow();
        virtual ~CNewUIMainFrameWindow();

        bool Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng);
        void Release();

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        void Render3D();

        bool IsVisible() const;

        float GetLayerDepth();		// 10.2f
        float GetKeyEventOrder();	// 7.f

        void SetItemHotKey(int iHotKey, int iItemType, int iItemLevel);
        int GetItemHotKey(int iHotKey);
        int GetItemHotKeyLevel(int iHotKey);
        void UseHotKeyItemRButton();
        //void RenderHotKeyItems();
        void UpdateItemHotKey();

        void ResetSkillHotKey();
        void SetSkillHotKey(int iHotKey, int iSkillType);
        int GetSkillHotKey(int iHotKey);
        int GetSkillHotKeyIndex(int iSkillType);

        void SetPreExp_Wide(__int64 dwPreExp);
        void SetGetExp_Wide(__int64 dwGetExp);

        void SetPreExp(DWORD dwPreExp);
        void SetGetExp(DWORD dwGetExp);

        // buttons
        void SetBtnState(int iBtnType, bool bStateDown);

        static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);

    private:
        void SetButtonInfo();

        void LoadImages();
        void UnloadImages();

        bool BtnProcess();

        void RenderFrame();
        void RenderLifeMana();
        void RenderGuageAG();
        void RenderGuageSD();
        void RenderExperience();
        void RenderHotKeyItemCount();
        void RenderButtons();
        void RenderCharInfoButton();
        void RenderFriendButton();
        void RenderFriendButtonState();

    public:
        __int64	m_loPreExp;
        __int64	m_loGetExp;

    private:
        CNewUIManager* m_pNewUIMng;
        CNewUI3DRenderMng* m_pNewUI3DRenderMng;

        CNewUIItemHotKey m_ItemHotKey;

        bool m_bExpEffect;
        DWORD m_dwExpEffectTime;

        DWORD m_dwPreExp;
        DWORD m_dwGetExp;

#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
        CNewUIButton m_BtnCShop;
#endif //defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
        CNewUIButton m_BtnChaInfo;
        CNewUIButton m_BtnMyInven;
        CNewUIButton m_BtnFriend;
        CNewUIButton m_BtnWindow;

        bool m_bButtonBlink;
    };
}

#endif // !defined(AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_)
