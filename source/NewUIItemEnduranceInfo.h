// NewUIItemEnduranceInfo.h: interface for the CNewUIItemEnduranceInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIITEMENDURANCEINFO_H__ADB04FC1_C3E3_47B5_8026_C78C5800500C__INCLUDED_)
#define AFX_NEWUIITEMENDURANCEINFO_H__ADB04FC1_C3E3_47B5_8026_C78C5800500C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NewUIBase.h"
#include "NewUIManager.h"

namespace SEASON3B
{
	class CNewUIItemEnduranceInfo : public CNewUIObj
	{
	protected:
		enum IMAGE_LIST
		{		
			IMAGE_PETHP_FRAME = BITMAP_ITEM_ENDURANCE_INFO_BEGIN,
			IMAGE_PETHP_BAR,
			IMAGE_ITEM_DUR_BOOTS,
			IMAGE_ITEM_DUR_CAP,
			IMAGE_ITEM_DUR_GLOVES,
			IMAGE_ITEM_DUR_LOWER,
			IMAGE_ITEM_DUR_NECKLACE,
			IMAGE_ITEM_DUR_RING,
			IMAGE_ITEM_DUR_SHIELD,
			IMAGE_ITEM_DUR_UPPER,
			IMAGE_ITEM_DUR_WEAPON,
			IMAGE_ITEM_DUR_WING,
#ifdef PBG_ADD_PCROOM_NEWUI
			IMAGE_PCROOM_UI = BITMAP_PCROOM_NEWUI,
#endif //PBG_ADD_PCROOM_NEWUI
		};
		
		enum IMAGE_SIZE
		{
			PETHP_FRAME_WIDTH = 57,
			PETHP_FRAME_HEIGHT = 23,
			PETHP_BAR_WIDTH = 49,
			PETHP_BAR_HEIGHT = 3,
			ITEM_DUR_WIDTH = 23,
			ITEM_DUR_HEIGHT = 23,
#ifdef PBG_ADD_PCROOM_NEWUI
			PCROOM_WIDTH = 57,
			PCROOM_HEIGHT = 23,
#endif //PBG_ADD_PCROOM_NEWUI
		};

		enum ITEM_DUR_ARROW_TYPE
		{
			ARROWTYPE_NONE = -1,	
			ARROWTYPE_BOW = ITEM_BOW + 15,
			ARROWTYPE_CROSSBOW = ITEM_BOW + 7,
		};

		enum 
		{
			UI_INTERVAL_HEIGHT = 1,
			UI_INTERVAL_WIDTH = 2,
		};

	private:
		CNewUIManager*				m_pNewUIMng;
		POINT						m_UIStartPos;				// UI 시작위치
		int							m_iTextEndPosX;				// Text끝 X의 위치
		POINT						m_ItemDurUIStartPos;		// 아이템 내구도UI 시작위치

		int							m_iCurArrowType;			// 현재 화살타입
		int							m_iItemDurImageIndex[MAX_EQUIPMENT];	// 아이템 내구도 이미지 인덱스
		int							m_iTooltipIndex;

	public:
		CNewUIItemEnduranceInfo();
		virtual ~CNewUIItemEnduranceInfo();

		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos( int x, int y);
		void SetPos( int x );
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();

		bool BtnProcess();
		
		float GetLayerDepth();	//. 3.5f
		
		void OpenningProcess();
		void ClosingProcess();
		
	private:
		void LoadImages();
		void UnloadImages();

		void InitImageIndex();

		void RenderLeft();
		void RenderRight();

		void RenderHPUI( int iX, int iY, unicode::t_char* pszName, int iLife, int iMaxLife = 255, bool bWarning = false );
		void RenderTooltip( int iX, int iY, const ITEM* pItem, const DWORD& dwTextColor);
		//void RenderItemDurIcon( int iImageIndex, int iX, int iY, int iWidth, int iHeight, DWORD dwColor, unicode::t_char* pszName );
															// 순위
		bool RenderEquipedHelperLife( int iX, int iY );		// 1. 수호천사, 사탄, 다크호스, 디노란트, 펜릴
		bool RenderEquipedPetLife( int iX, int iY );		// 2. 다크스피릿
		bool RenderSummonMonsterLife( int iX, int iY );		// 3. 소환몬스터
		bool RenderNumArrow( int iX, int iY );				// 4. 남은화살
#ifndef KJH_DEL_PC_ROOM_SYSTEM		// #ifndef
 		bool RenderPCRoomPoint( int iX, int iY );			// 5. PC방포인트표시
#endif // KJH_DEL_PC_ROOM_SYSTEM
#ifdef PBG_ADD_PCROOM_NEWUI
		bool RenderPCRoomUI(int _PosX, int _PosY);			// pc방 UI
		bool RenderPCRoomToolTip(int _PosX, int _PosY);
#endif //PBG_ADD_PCROOM_NEWUI
 		bool RenderItemEndurance( int ix, int iY );			// 6. 아이템 내구도
	};
}

#endif // !defined(AFX_NEWUIITEMENDURANCEINFO_H__ADB04FC1_C3E3_47B5_8026_C78C5800500C__INCLUDED_)
