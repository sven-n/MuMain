// NewUIMainFrameWindow.h: interface for the CNewUIMainFrameWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_)
#define AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
#if defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
		MAINFRAME_BTN_PARTCHARGE = 0,
#endif //defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
		MAINFRAME_BTN_CHAINFO,
		MAINFRAME_BTN_MYINVEN,
		MAINFRAME_BTN_FRIEND,
		MAINFRAME_BTN_WINDOW,
	};

#ifdef KJH_ADD_SKILLICON_RENEWAL
	enum KINDOFSKILL
	{
		KOS_COMMAND = 1,
		KOS_SKILL1,
		KOS_SKILL2,
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
		KOS_SKILL3,
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	};
#endif // KJH_ADD_SKILLICON_RENEWAL

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
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			IMAGE_SKILL3,			// 이미지 인덱스로 non설정을 위함
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
			IMAGE_SKILLBOX,
			IMAGE_SKILLBOX_USE,
#ifdef KJH_ADD_SKILLICON_RENEWAL
			IMAGE_NON_SKILL1,
			IMAGE_NON_SKILL2,
			IMAGE_NON_COMMAND,
#endif // KJH_ADD_SKILLICON_RENEWAL
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			IMAGE_NON_SKILL3,
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
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
		
#ifdef CSK_FIX_WOPS_K29010_HELLBUST
#ifdef PBG_ADD_NEWCHAR_MONK
		WORD GetHeroPriorSkill();
#else //PBG_ADD_NEWCHAR_MONK
		BYTE GetHeroPriorSkill();
#endif //PBG_ADD_NEWCHAR_MONK
		void SetHeroPriorSkill(BYTE bySkill);
#endif // CSK_FIX_WOPS_K29010_HELLBUST

		void Reset();

		// 핫키 등록
		void SetHotKey(int iHotKey, int iSkillType);
		int GetHotKey(int iHotKey);
		int GetSkillIndex(int iSkillType);
#ifdef PBG_FIX_SKILLHOTKEY
		void SetHotKeyClear(int iHotKey);
#endif //PBG_FIX_SKILLHOTKEY
		void RenderCurrentSkillAndHotSkillList();	// 현재 사용하고 있는 스킬과 핫스킬 

		bool IsSkillListUp();

		static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);
		
	private:
		void LoadImages();
		void UnloadImages();

		// 현재 사용중인 스킬이 윗칸인가 아랫칸인가
		bool IsArrayUp(BYTE bySkill);
		// 현재 사용중인 스킬이 스킬핫킷 배열 안에 있는가?
		bool IsArrayIn(BYTE bySkill);

		// 핫키 사용
		void UseHotKey(int iHotKey);
		
		void RenderSkillIcon(int iIndex, float x, float y, float width, float height);
		void RenderSkillDelay(int iIndex, float x, float y, float width, float height);
		void RenderPetSkill();

		void ResetMouseLButton();
		
	private:
		CNewUIManager*		m_pNewUIMng;
		CNewUI3DRenderMng*	m_pNewUI3DRenderMng;

		// 스킬 핫키 관련
		bool m_bHotKeySkillListUp;	// 6 ~ 0인가
		int m_iHotKeySkillType[SKILLHOTKEY_COUNT];

		// 스킬 리스트
		bool m_bSkillList;

		// 스킬 정보 관련
		bool m_bRenderSkillInfo;
		int m_iRenderSkillInfoType;
		int m_iRenderSkillInfoPosX;
		int m_iRenderSkillInfoPosY;

		// 마우스 상태
		EVENT_STATE m_EventState;
		
#ifdef CSK_FIX_WOPS_K29010_HELLBUST
#ifdef PBG_ADD_NEWCHAR_MONK
		// 헬버스트에서만 사용으로 문제는 없으나 하위캐스팅문제
		WORD m_wHeroPriorSkill;
#else //PBG_ADD_NEWCHAR_MONK
		BYTE m_byHeroPriorSkill;
#endif //PBG_ADD_NEWCHAR_MONK
#endif // CSK_FIX_WOPS_K29010_HELLBUST
	};

	class CNewUIMainFrameWindow	: public CNewUIObj, public INewUI3DRenderObj
	{
	public:
		enum IMAGE_LIST
		{	//. max: 10개
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
#if defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
			IMAGE_MENU_BTN_CSHOP,
#endif //defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
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

		// ItemHotKey 관련 외부공개 함수들
		void SetItemHotKey(int iHotKey, int iItemType, int iItemLevel);
		int GetItemHotKey(int iHotKey);
		int GetItemHotKeyLevel(int iHotKey);
		void UseHotKeyItemRButton();
		//void RenderHotKeyItems();
		void UpdateItemHotKey();

		// SkillHotKey 관련 외부공개 함수들
		void ResetSkillHotKey();
		void SetSkillHotKey(int iHotKey, int iSkillType);
		int GetSkillHotKey(int iHotKey);
		int GetSkillHotKeyIndex(int iSkillType);

#ifdef PBG_FIX_SKILLHOTKEY
		void SetSkillHotKeyClear(int iHotKey);
#endif //PBG_FIX_SKILLHOTKEY

		void SetPreExp_Wide(__int64 dwPreExp);
		void SetGetExp_Wide(__int64 dwGetExp);

		void SetPreExp(DWORD dwPreExp);
		void SetGetExp(DWORD dwGetExp);

		// buttons
		void SetBtnState(int iBtnType, bool bStateDown);
		
		static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);
		
	private:
		void SetButtonInfo();

		void LoadImages();	// 이미지 로더 하는 함수
		void UnloadImages();

		bool BtnProcess();

		void RenderFrame();	// 메인프레임 틀 렌더링 함수
		void RenderLifeMana();	// HP, MP 렌더링
		void RenderGuageAG();	// AG 게이지 렌더링
		void RenderGuageSD();	// SD 게이지 렌더링
		void RenderExperience();	// 경험치 렌더링
		void RenderHotKeyItemCount();	// QWER 키에 등록된 아이템 갯수 렌더링
		void RenderButtons();
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
		void RenderCharInfoButton();
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
		void RenderFriendButton();
		void RenderFriendButtonState();

	public:
		__int64	m_loPreExp;
		__int64	m_loGetExp;
		
	private:
		CNewUIManager*		m_pNewUIMng;
		CNewUI3DRenderMng*	m_pNewUI3DRenderMng;

		CNewUIItemHotKey m_ItemHotKey;
		
		// 경험치바 관련 변수들
		bool m_bExpEffect;			// 경험치바 효과
		DWORD m_dwExpEffectTime;	// 경험치바 효과 시간

		DWORD m_dwPreExp;			// 경험치 이전 값
		DWORD m_dwGetExp;			// 경험치 획득 값

#if defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
		CNewUIButton m_BtnCShop;	// 부분 유료화 상점
#endif //defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
		CNewUIButton m_BtnChaInfo;	// 캐릭터 정보창
		CNewUIButton m_BtnMyInven;	// 인벤토리창
		CNewUIButton m_BtnFriend;	// 친구창
		CNewUIButton m_BtnWindow;	// 윈도우메뉴창

#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
		bool m_bButtonBlink;		// true이면 버튼이 밝아지고(마우스 오버 이미지) false이면 버튼이 원래 상태로 되는 용도로 사용.
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC

	};

}

#endif // !defined(AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_)
