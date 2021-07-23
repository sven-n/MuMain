// NewUIHeroPositionInfo.h: interface for the CNewUIHeroPositionInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIHEROPOSITIONINFO_H__5D452B62_B315_41EE_A862_5929286925F7__INCLUDED_)
#define AFX_NEWUIHEROPOSITIONINFO_H__5D452B62_B315_41EE_A862_5929286925F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NewUIBase.h"
#include "NewUIManager.h"

namespace SEASON3B
{
	class CNewUIHeroPositionInfo : public CNewUIObj
	{
	public:
		enum IMAGE_LIST
		{	
			// 기타 이미지
			IMAGE_HERO_POSITION_INFO_BASE_WINDOW = BITMAP_HERO_POSITION_INFO_BEGIN,		// newui_position02.tga			(70, 25)	
		};
	private:
		enum HERO_POSITION_INFO_BASE_WINDOW_SIZE
		{
#ifdef PJH_ADD_MINIMAP
			HERO_POSITION_INFO_BASEA_WINDOW_WIDTH = 22,
			HERO_POSITION_INFO_BASE_WINDOW_HEIGHT = 25,
			HERO_POSITION_INFO_BASEB_WINDOW_WIDTH = 80,		//이수치만 조정하면 인터페이스의 전체 길이가 달라진다.
			HERO_POSITION_INFO_BASEC_WINDOW_WIDTH = 22,
#else
			HERO_POSITION_INFO_BASE_WINDOW_WIDTH = 70,
			HERO_POSITION_INFO_BASE_WINDOW_HEIGHT = 25,
#endif //PJH_ADD_MINIMAP

		};
		
	private:
#ifdef PJH_ADD_MINIMAP
		int							WidenX;
#endif //PJH_ADD_MINIMAP

		CNewUIManager*				m_pNewUIMng;
		POINT						m_Pos;

		POINT						m_CurHeroPosition;			// 지도상 주인공의 현재 위치
		
	public:
		CNewUIHeroPositionInfo();
		virtual ~CNewUIHeroPositionInfo();
		
		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();

		bool BtnProcess();
		
		float GetLayerDepth();	//. 4.3f

		void OpenningProcess();
		void ClosingProcess();

		void SetCurHeroPosition( int x, int y );
		
	private:
		void LoadImages();
		void UnloadImages();	
		
	};
}

#endif // !defined(AFX_NEWUIHEROPOSITIONINFO_H__5D452B62_B315_41EE_A862_5929286925F7__INCLUDED_)
