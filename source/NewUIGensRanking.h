// NewUIGensRanking.h: interface for the CNewUIGensRanking class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIGENSRANKING_H__67F934E4_256C_4A48_AE15_A13D3D9475DA__INCLUDED_)
#define AFX_NEWUIGENSRANKING_H__67F934E4_256C_4A48_AE15_A13D3D9475DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef PBG_ADD_GENSRANKING

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIButton.h"
#ifdef KJW_FIX_GENS_WINDOW_OPEN
#include "NewUIScrollBar.h"
#include "NewUITextBox.h"
#endif // KJW_FIX_GENS_WINDOW_OPEN
#define MAX_TITLELENGTH		32

namespace SEASON3B
{
	class CNewUIGensRanking : public CNewUIObj
	{
		enum IMAGE_LIST
		{
			IMAGE_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
			IMAGE_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2,	
			IMAGE_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
			IMAGE_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
			IMAGE_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
			IMAGE_CLOSE_REGIST = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL,
			IMAGE_GENS_EXIT_BTN = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
			IMAGE_NEWMARK_DUPRIAN = BITMAP_GENS_MARK_DUPRIAN,
			IMAGE_NEWMARK_BARNERT = BITMAP_GENS_MARK_BARNERT,

			IMAGE_GENSINFO_TOP_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,
			IMAGE_GENSINFO_TOP_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,
			IMAGE_GENSINFO_BOTTOM_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,
			IMAGE_GENSINFO_BOTTOM_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,
			IMAGE_GENSINFO_TOP_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,
			IMAGE_GENSINFO_BOTTOM_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,
			IMAGE_GENSINFO_LEFT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,
			IMAGE_GENSINFO_RIGHT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,

			IMAGE_RANKBACK = BITMAP_GENS_RANKBACK,
			IMAGE_RANKBACK_TEXTBOX = BITMAP_INTERFACE_NEW_CHAINFO_WINDOW_BEGIN,
		};

		enum IMAGE_SIZE
		{
			GENSRANKING_WIDTH = 190,
			GENSRANKING_HEIGHT = 429,
#ifdef LJH_FIX_DEFINE_GENS_TEAMNAME_LENTH_TO_16
			TEAMNAME_LENTH = 16,
#else  //LJH_FIX_DEFINE_GENS_TEAMNAME_LENTH_TO_16
			TEAMNAME_LENTH = 10,
#endif //LJH_FIX_DEFINE_GENS_TEAMNAME_LENTH_TO_16
			GENSMARK_WIDTH = 50,
			GENSMARK_HEIGHT = 69,
			GENSRANKBACK_WIDTH = 170,
			GENSRANKBACK_HEIGHT = 88,
			GENSRANKTEXTBACK_WIDTH = 170,
			GENSRANKTEXTBACK_HEIGHT = 21,
		};

		enum GENS_TYPE
		{
			GENSTYPE_NONE=0,
			GENSTYPE_DUPRIAN,
			GENSTYPE_BARNERT,
		};

		enum IMAGE_INDEX
		{
			// 계급이 늘어날 경우 순서의 의해 추가 할것
			// 1.대공 2.공작 3.후작 4.백작 5.자작 6.남작 7.기사단장 
			// 8.상급기사 9.기사 10.근위대장 11.사관 12.백인장 13.십인장 14 사병
			TITLENAME_NONE =	0,
			TITLENAME_START =	1,
			TITLENAME_END =		14,
		};
	
	public:
		enum IMAGE_AREA
		{
			MARK_UIINFO =0,
			MARK_BOOLEAN,
			MARK_RANKINFOWIN,
		};

	private:
		void Init();
		void Destroy();
		void LoadImages();
		void UnloadImages();

		POINT m_Pos;

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
		FLOAT m_fBooleanSize;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
		const float m_fBooleanSize;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
		int m_nContribution;
		unicode::t_char m_szRanking[TEAMNAME_LENTH];
		unicode::t_char m_szGensTeam[TEAMNAME_LENTH];

		BYTE m_byGensInfluence;
		POINT m_ptRenderMarkPos;
		
		CNewUIButton m_BtnExit;

		unicode::t_char m_szTitleName[TITLENAME_END][MAX_TITLELENGTH];

		int m_nNextContribution;

	public:
		CNewUIManager* m_pNewUIMng;
#ifdef KJW_FIX_GENS_WINDOW_OPEN
		CNewUIScrollBar* m_pScrollBar;
		CNewUITextBox* m_pTextBox;
#endif // KJW_FIX_GENS_WINDOW_OPEN
		CNewUIGensRanking();
		virtual ~CNewUIGensRanking();
		
		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void SetPos(int x, int y);
		const POINT& GetPos() { return m_Pos; }
		
		bool Render();
		void RenderFrame();
		void RenderTexts();
		void RenderButtons();
		void RenderMark(int x, int y, BYTE _GensInfluence);
		void RenderInfoFrame(int iPosX, int iPosY, int iWidth, int iHeight, int iTitleWidth = 60, int iTitleHeight = 20);

		bool Update();
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();

		bool BtnProcess();
		void SetBtnInfo(float _PosX, float _PosY);

		void OpenningProcess();
		void ClosingProcess();
	
		float GetLayerDepth() { return 4.2f; }

		// 기여도 관련
		void SetContribution(int _Contribution);
		int GetContribution();

		// 다음 기여도까지 포인트
		void SetNextContribution(int _NextContribution);
		int GetNextContribution();

		// 랭킹
		bool SetRanking(int _Ranking);
		unicode::t_char* GetRanking();

		// 가입겐스
		bool SetGensInfo();
		bool SetGensTeamName(const char* _pTeamName);
		unicode::t_char* GetGensTeamName();

		// 계급관련
		void SetTitleName();
		const unicode::t_char* GetTitleName(BYTE _index);

		// 계급장 랜더
		void RanderMark(float _x, float _y, BYTE _GensInfluence, BYTE _GensRankInfo, int _ImageArea = MARK_RANKINFOWIN, float _RenderY = 0);
		int GetImageIndex(BYTE _index);
	};
}

#endif //PBG_ADD_GENSRANKING

#endif // !defined(AFX_NEWUIGENSRANKING_H__67F934E4_256C_4A48_AE15_A13D3D9475DA__INCLUDED_)
