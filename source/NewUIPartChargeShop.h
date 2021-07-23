// NewUIPartChargeShop.h: interface for the CNewUIPartChargeShop class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIPARTCHARGESHOP_H__577ED168_8B9B_4F17_B6BE_C4C44B44F877__INCLUDED_)
#define AFX_NEWUIPARTCHARGESHOP_H__577ED168_8B9B_4F17_B6BE_C4C44B44F877__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NewUIManager.h"
#include "NewUI3DRenderMng.h"

#ifdef NEW_USER_INTERFACE_SHELL

namespace SEASON3B
{
	class CNewUIPartChargeShop  : public CNewUIObj  
	{
	public:
		CNewUIPartChargeShop();
		virtual ~CNewUIPartChargeShop();

		bool Create(CNewUIManager* pNewUIMng, int x = 0, int y = 0);
		void Release();
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
		
		float GetLayerDepth();		// 10.08f

		bool IsVisible() const;
		
	public:
		void OpeningProcess();
		void ClosingProcess();

	private:
		CNewUIManager* m_pNewUIMng;		// UI ¸Å´ÏÀú.
	};
};

#endif //NEW_USER_INTERFACE_SHELL

#endif // !defined(AFX_NEWUIPARTCHARGESHOP_H__577ED168_8B9B_4F17_B6BE_C4C44B44F877__INCLUDED_)
