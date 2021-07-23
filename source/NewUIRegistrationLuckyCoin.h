// NewUIRegistrationLuckyCoin.h: interface for the CNewUIRegistrationLuckyCoin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIREGISTRATIONLUCKYCOIN_H__14342688_43E6_4EA9_BB5B_BBC4B0DAC7C9__INCLUDED_)
#define AFX_NEWUIREGISTRATIONLUCKYCOIN_H__14342688_43E6_4EA9_BB5B_BBC4B0DAC7C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUICommonMessageBox.h"
#include "ZzzInventory.h"

namespace SEASON3B
{
	class CNewUIRegistrationLuckyCoin : public CNewUIObj
	{
	public:
		enum IMAGE_LIST
		{
			//UI이미지
			IMAGE_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
			IMAGE_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2,	
			IMAGE_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
			IMAGE_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
			IMAGE_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
			//버튼이미지에 사용(닫기,등록)
			IMAGE_CLOSE_REGIST = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL,
		};
		
	private:
		enum
		{
			//전체창의 크기.
			LUCKYCOIN_REG_WIDTH = 190,
			LUCKYCOIN_REG_HEIGHT = 429,
		};
		
	public:
		CNewUIRegistrationLuckyCoin();
		virtual ~CNewUIRegistrationLuckyCoin();

		//UI메니져 등록 및 로드
		bool Create(CNewUIManager* pNewUIMng, int x, int y);

		void SetPos(int x, int y);
		const POINT& GetPos() { return m_Pos; }
		
		bool Render();
		bool Update();
		//마우스관련
		bool UpdateMouseEvent();
		//키관련
		bool UpdateKeyEvent();

		//버튼관련
		bool BtnProcess();
		void SetBtnInfo();

		float GetLayerDepth()
		{
			return 4.2f;
		}
		
		//등록갯수
		const int& GetRegistCount() { return m_RegistCount; }

		//등록 갯수입력
		void SetRegistCount(int nRegistCount) { m_RegistCount = nRegistCount; }

		//아템 회전하기.
		bool GetItemRotation() { return m_ItemAngle; }
		void SetItemRotation(bool _bInput) { m_ItemAngle = _bInput; }

		//등록버튼 unlock을위해
		void LockLuckyCoinRegBtn();
		void UnLockLuckyCoinRegBtn();

		//열고닫을때의 프로세스
		void OpeningProcess();
		void ClosingProcess();

		void Release();
		
	private:
		void LoadImages();
		void UnloadImages();

		//전체 프레임
		void RenderFrame();
		//텍스트 찍기
		void RenderTexts();
		//버튼그리기
		void RenderButtons();
		//아템그리기
		void RenderLuckyCoin();

	private:
		CNewUIManager* m_pNewUIMng;
		POINT m_Pos;
		ITEM* m_CoinItem;
		//인터페이스에 그려질 아이템
		bool m_ItemAngle;
		//버튼의 크기
		float m_width, m_height;
		//등록갯수
		int m_RegistCount;
		//닫기버튼
		CNewUIButton m_CloseButton;
		//등록버튼
		CNewUIButton m_RegistButton;
	};
}
#endif //KJH_PBG_ADD_SEVEN_EVENT_2008

#endif // !defined(AFX_NEWUIREGISTRATIONLUCKYCOIN_H__14342688_43E6_4EA9_BB5B_BBC4B0DAC7C9__INCLUDED_)
