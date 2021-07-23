// FatigueTimeSystem.h: interface for the FatigueTimeSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FATIGUETIMESYSTEM_H__319C16C8_3B02_465B_9833_F1E171347591__INCLUDED_)
#define AFX_FATIGUETIMESYSTEM_H__319C16C8_3B02_465B_9833_F1E171347591__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef PBG_ADD_SECRETBUFF
// 피로도 적용 서버에서만 서버에서 직접 퍼센트를 넘겨준다
// 클라에서 퍼센트를 찍어주는 것외엔 처리하는것없음
class CFatiguePercentage
{
	BYTE m_btFatiguePercentage;
	bool m_bIsFatigue;
public:
	CFatiguePercentage();
	~CFatiguePercentage();

	static CFatiguePercentage* GetInstance();

	// 서버로부터 퍼센트를 받는다
	bool SetFatiguePercentage(const BYTE& _Value);
	const BYTE& GetFatiguePercentage() const;
	// 텍스트 출력을 위해 피로도적용서버임을 확인한다
 	const bool& IsFatigueSystem() const;
 	void SetIsFatigueSystem(bool _Value);
};

#define g_FatigueTimeSystem	CFatiguePercentage::GetInstance()
#endif //PBG_ADD_SECRETBUFF
#ifdef PBG_MOD_STAMINA_UI
#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUICommonMessageBox.h"
#include "ZzzInventory.h"
#include "ZzzInterface.h"

namespace SEASON3B
{
	class CNewUIStamina : public CNewUIObj 
	{
		enum IMAGE_LIST
		{
			IMAGE_STAMINA_FRAME	= BITMAP_BLUE_STAMINA_FRAME,
			IMAGE_STAMINA_GAUGE	= BITMAP_BLUE_STAMINA_GAUGE,
			IMAGE_STAMINA_CAUTION = BITMAP_BLUE_STAMINA_CAUTION,
			IMAGE_STAMINA_END,
		};
		enum IMAGE_SIZE
		{
			IMAGE_FRAME_WIDTH    = 96,
			IMAGE_FRAME_HEIGHT   = 86,
			IMAGE_GAUGE_WIDTH    = 77,
			IMAGE_GAUGE_HEIGHT   = 39,
		};
		enum IMAGE_ANGLE
		{
			IMAGE_GAUGE_START	= 10,
			IMAGE_GAUGE_95		= 77,
			IMAGE_GAUGE_END		= 90,
		};

		const float m_fImageScale;
		POINT m_Pos;
		POINT m_FrameSize;
		POINT m_GaugeSize;
		
		float m_nCaution;
		float m_fGaugeBar;


		void Init(float _fImageScale = 1.0f);
		void Destroy();
		void LoadImages(float _fImageScale = 1.0f);
		void UnloadImages();
		void ImageRotation(int Texture,float x,float y,float Width,float Height,float Rotate);
		float ConvertX(float x);
		float ConvertY(float y);
		float GetCaution();

	public:
		CNewUIManager* m_pNewUIMng;

		CNewUIStamina();
		virtual ~CNewUIStamina();

		bool Create(CNewUIManager* pNewUIMng, int x, int y);

		void SetPos(int x, int y);
		const POINT& GetPos() { return m_Pos; }
		bool Render();

		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		float GetLayerDepth();

		void SetCaution(float _caution);
		void OpeningProcess();
		void ClosingProcess();

		BOOL IsStaminaUI();
		void RenderTexts();
	};
}
#endif //PBG_MOD_STAMINA_UI

#endif // !defined(AFX_FATIGUETIMESYSTEM_H__319C16C8_3B02_465B_9833_F1E171347591__INCLUDED_)
