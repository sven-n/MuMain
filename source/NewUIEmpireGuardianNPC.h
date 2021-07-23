// CNewUIEmpireGuardianNPC.h: interface for the CNewUIEmpireGuardianNPC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CNewUIEmpireGuardianNPC_H__C9BF91CC_CF17_459C_942F_51F6BBE2C11E__INCLUDED_)
#define AFX_CNewUIEmpireGuardianNPC_H__C9BF91CC_CF17_459C_942F_51F6BBE2C11E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef LDK_ADD_EMPIREGUARDIAN_UI

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIMyQuestInfoWindow.h"

namespace SEASON3B
{
	class CNewUIEmpireGuardianNPC : public CNewUIObj, public INewUI3DRenderObj  
	{
	public:	
		enum IMAGE_LIST
		{
			IMAGE_EMPIREGUARDIAN_NPC_TOP		= CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
			IMAGE_EMPIREGUARDIAN_NPC_LEFT		= CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
			IMAGE_EMPIREGUARDIAN_NPC_RIGHT		= CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
			IMAGE_EMPIREGUARDIAN_NPC_BOTTOM		= CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
			IMAGE_EMPIREGUARDIAN_NPC_BACK		= CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
 			IMAGE_EMPIREGUARDIAN_NPC_LINE		= CNewUIMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
			IMAGE_EMPIREGUARDIAN_NPC_BTN		= CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,
		};
		
	private:
		enum EMPIREGUARDIAN_TIME_WINDOW_SIZE
		{
			NPC_WINDOW_WIDTH = 190,
			NPC_WINDOW_HEIGHT = 429,
		};
		
		CNewUIManager*		m_pNewUIMng;
		CNewUI3DRenderMng*		m_pNewUI3DRenderMng;

		POINT				m_Pos;
		CNewUIButton		m_btPositive;	// ¿‘¿Â.
		CNewUIButton		m_btNegative;	// ¥›±‚.
		bool				m_bCanClick;
		
	public:
		CNewUIEmpireGuardianNPC();
		virtual ~CNewUIEmpireGuardianNPC();
		
		bool Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
		void Render3D();
		
		float GetLayerDepth();	//. 1.2f

		bool IsVisible() const;

		void OpenningProcess();
		void ClosingProcess();
		
	private:
		void InitButton(CNewUIButton * pNewUIButton, int iPos_x, int iPos_y, const unicode::t_char * pCaption);
		void LoadImages();
		void UnloadImages();

		bool BtnProcess();
		void RenderFrame();
		void RenderItem3D();

	};
}
#endif	// LDK_ADD_EMPIREGUARDIAN_UI

#endif // !defined(AFX_CNewUIEmpireGuardianNPC_H__C9BF91CC_CF17_459C_942F_51F6BBE2C11E__INCLUDED_)

