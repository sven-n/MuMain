// NewUIMixInventory.h: interface for the CNewUIMixInventory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIMIXINVENTORY_H__EBAA98CA_D9A0_423E_AFF8_08079E2637F7__INCLUDED_)
#define AFX_NEWUIMIXINVENTORY_H__EBAA98CA_D9A0_423E_AFF8_08079E2637F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NewUIBase.h"
#include "NewUIInventoryCtrl.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#ifdef ADD_SOCKET_MIX
#include "UIControls.h"
#endif	// ADD_SOCKET_MIX

namespace SEASON3B
{
	class CNewUIMixInventory : public CNewUIObj
	{
	public:
		enum IMAGE_LIST
		{
			IMAGE_MIXINVENTORY_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
			IMAGE_MIXINVENTORY_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2,	
			IMAGE_MIXINVENTORY_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
			IMAGE_MIXINVENTORY_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
			IMAGE_MIXINVENTORY_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
			IMAGE_MIXINVENTORY_MIXBTN = BITMAP_INTERFACE_NEW_MIXINVENTORY_BEGIN
		};
		enum MIX_STATE
		{
			MIX_READY = 0,
			MIX_REQUESTED,
			MIX_FINISHED
		};
		
	private:
		enum
		{
			INVENTORY_WIDTH = 190,
			INVENTORY_HEIGHT = 429,
		};
		
		CNewUIManager*			m_pNewUIMng;
		CNewUIInventoryCtrl*	m_pNewInventoryCtrl;
		POINT m_Pos;
		
		CNewUIButton m_BtnMix;

		int m_iMixState;		// 현재 조합창 상태
		int m_iMixEffectTimer;	// 믹스 이펙트 타이머
		float m_fInventoryColor[3];	// 픽킹 기본 배경색 백업용
		float m_fInventoryWarningColor[3];	// 픽킹 기본 배경색 백업용

#ifdef ADD_SOCKET_MIX
		CUISocketListBox m_SocketListBox;	// 소켓 아이템 장착/파괴시 목록 표시용
#endif	// ADD_SOCKET_MIX

	public:
		CNewUIMixInventory();
		virtual ~CNewUIMixInventory();
		
		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();

		bool InsertItem(int iIndex, BYTE* pbyItemPacket);
		void DeleteItem(int iIndex);
		void DeleteAllItems();
		
		void OpeningProcess();
		bool ClosingProcess();	// 닫을수 있는지 검사하고 hide를 제외한 조합 종료 처리

		void SetMixState(int iMixState);
		int GetMixState() { return m_iMixState; }
		
		int GetPointedItemIndex();

		void SetPos(int x, int y);

		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();

		float GetLayerDepth();	//. 3.4f

		CNewUIInventoryCtrl* GetInventoryCtrl() const;
		
		static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);

	private:
		void LoadImages();
		void UnloadImages();

		void RenderFrame();
		bool InventoryProcess();
		bool BtnProcess();

		void RenderMixDescriptions(float fPos_x, float fPos_y);	// 조합에 강제로 지정된 텍스트 출력
		
		void CheckMixInventory();
		bool Mix();
		void RenderMixEffect();

	#ifdef LEM_FIX_MIXREQUIREZEN
		int Rtn_MixRequireZen( int _nMixZen, int _nTax );	// 함수헤더 [lem.2010.7.29]
	#endif	// LEM_FIX_MIXREQUIREZEN
		
	};
}

#endif // !defined(AFX_NEWUIMIXINVENTORY_H__EBAA98CA_D9A0_423E_AFF8_08079E2637F7__INCLUDED_)
