// NewUIBuffWindow.h: interface for the CNewUIBuffWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIBUFFWINDOW_H__FD9F3EB9_AC19_4AAD_8710_0A41D001483E__INCLUDED_)
#define AFX_NEWUIBUFFWINDOW_H__FD9F3EB9_AC19_4AAD_8710_0A41D001483E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NewUIManager.h"

namespace SEASON3B
{
	
	class CNewUIBuffWindow  : public CNewUIObj 
	{
	public:
		enum IMAGE_LIST
		{
			IMAGE_BUFF_STATUS = BITMAP_BUFFWINDOW_BEGIN,
#if defined ASG_ADD_SKILL_BERSERKER || defined PSW_PARTCHARGE_ITEM4 || defined PBG_ADD_SANTABUFF
			IMAGE_BUFF_STATUS2,
#endif	// defined ASG_ADD_SKILL_BERSERKER || defined PSW_PARTCHARGE_ITEM4
		};

		enum BUFF_RENDER
		{
			BUFF_RENDER_ICON = 0,
			BUFF_RENDER_TOOLTIP
		};
		
	public:
		CNewUIBuffWindow();
		virtual ~CNewUIBuffWindow();

		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		void SetPos(int iScreenWidth);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
		
		float GetLayerDepth();	//. 5.3f
		
		void OpenningProcess();
		void ClosingProcess();
		
	private:
		void LoadImages();
		void UnloadImages();

		void BuffSort( list<eBuffState>& buffstate );
		void RenderBuffStatus( BUFF_RENDER renderstate );
		void RenderBuffIcon(eBuffState& eBuffType, float x, float y, float width, float height);
		void RenderBuffTooltip(eBuffClass& eBuffClassType, eBuffState& eBuffType, float x, float y);
#ifdef PBG_ADD_DISABLERENDER_BUFF
		//버프중에 이미지 만 안띄울 버프 등록
		bool SetDisableRenderBuff(const eBuffState& _BuffState);
#endif //PBG_ADD_DISABLERENDER_BUFF

		CNewUIManager* m_pNewUIMng;
		POINT m_Pos;
	
	};
	
}

#endif // !defined(AFX_NEWUIBUFFWINDOW_H__FD9F3EB9_AC19_4AAD_8710_0A41D001483E__INCLUDED_)
