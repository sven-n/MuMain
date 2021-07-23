// FatigueTimeSystem.cpp: implementation of the FatigueTimeSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef PBG_ADD_SECRETBUFF

#include "FatigueTimeSystem.h"

CFatiguePercentage::CFatiguePercentage()
{
	m_bIsFatigue = false;
	m_btFatiguePercentage = 0;
}
CFatiguePercentage::~CFatiguePercentage()
{

}
CFatiguePercentage* CFatiguePercentage::GetInstance()
{
	static CFatiguePercentage sInstance;
	return &sInstance;
}

bool CFatiguePercentage::SetFatiguePercentage(const BYTE& _Value)
{
	// 피로도의 퍼센테이지
	if(_Value >= 0 && _Value <= 100)
	{
		m_btFatiguePercentage = _Value;
		return true;
	}
	else
		return false;
}

const BYTE& CFatiguePercentage::GetFatiguePercentage() const
{
	return m_btFatiguePercentage;
}

// 피로도 시스템을 적용하는가? 툴팁사용여부사용
const bool& CFatiguePercentage::IsFatigueSystem() const
{
	return m_bIsFatigue;
}

// 서버로 부터 ReceiveFatigueTime받을 경우 true
void CFatiguePercentage::SetIsFatigueSystem(bool _Value)
{
	m_bIsFatigue = _Value;
}
#endif //PBG_ADD_SECRETBUFF

#ifdef PBG_MOD_STAMINA_UI
#include "NewUISystem.h"
namespace SEASON3B
{
	CNewUIStamina::CNewUIStamina() : m_fImageScale(0.85f)
	{
		Init(m_fImageScale);
	}
	CNewUIStamina::~CNewUIStamina()
	{
		Destroy();
	}
	void CNewUIStamina::Init(float _fImageScale)
	{
		LoadImages(_fImageScale);

		m_FrameSize.x = IMAGE_FRAME_WIDTH*_fImageScale;
		m_FrameSize.y = IMAGE_FRAME_HEIGHT*_fImageScale;
		m_GaugeSize.x = IMAGE_GAUGE_WIDTH*_fImageScale;
		m_GaugeSize.y = IMAGE_GAUGE_HEIGHT*_fImageScale;

		m_nCaution = IMAGE_GAUGE_START;
		m_fGaugeBar = 100;
	}
	void CNewUIStamina::Destroy()
	{
		UnloadImages();

#ifdef LDS_FIX_MEMORYLEAK_WHERE_NEWUI_DEINITIALIZE
		if (m_pNewUIMng)
		{
			m_pNewUIMng->RemoveUIObj(this);
			m_pNewUIMng = NULL;
		}
#endif // LDS_FIX_MEMORYLEAK_WHERE_NEWUI_DEINITIALIZE
	}
	void CNewUIStamina::LoadImages(float _fImageScale)
	{
		LoadBitmap("Interface\\fautigue_frame.tga", IMAGE_STAMINA_FRAME, GL_LINEAR);
		LoadBitmap("Interface\\fautigue_gauge.tga", IMAGE_STAMINA_GAUGE, GL_LINEAR);
		LoadBitmap("Interface\\fautigue_frame_lamp.tga", IMAGE_STAMINA_CAUTION, GL_LINEAR);

		for(int i=IMAGE_STAMINA_FRAME; i<IMAGE_STAMINA_END; ++i)
		{
			BITMAP_t *pImage = &Bitmaps[i];
			pImage->Width *= _fImageScale;
			pImage->Height *= _fImageScale;
		}
	}
	void CNewUIStamina::UnloadImages()
	{
		DeleteBitmap(IMAGE_STAMINA_FRAME);
		DeleteBitmap(IMAGE_STAMINA_GAUGE);
		DeleteBitmap(IMAGE_STAMINA_CAUTION);
	}
	void CNewUIStamina::SetPos(int x, int y)
	{
		m_Pos.x = x; 
		m_Pos.y = y;
	}
	bool CNewUIStamina::Create(CNewUIManager* pNewUIMng, int x, int y)
	{
		if(pNewUIMng  == NULL)
			return false;
		
		m_pNewUIMng = pNewUIMng;
		m_pNewUIMng->AddUIObj(INTERFACE_STAMINA_GAUGE, this);
		
		SetPos(x, y);
		LoadImages();
		Show(false);
		
		return true;
	}
	void CNewUIStamina::SetCaution(float _caution)
	{
		m_fGaugeBar = _caution;
	}
	float CNewUIStamina::GetCaution()
	{
		// 컨셉팀의 요구사항으로 구간마다 다른 비율로 증가한다
		if(m_fGaugeBar == 100)
		{
			m_nCaution = IMAGE_GAUGE_START;
		}
		else if(m_fGaugeBar < 100 && m_fGaugeBar >= 5)
		{
			m_nCaution = IMAGE_GAUGE_START + (100 - m_fGaugeBar) * ((float)(IMAGE_GAUGE_95-IMAGE_GAUGE_START) / 94);
		}
		else
		{
			m_nCaution = IMAGE_GAUGE_95 + (5 - m_fGaugeBar) * ((float)(IMAGE_GAUGE_END - IMAGE_GAUGE_95) / 5);
		}

		return m_nCaution;
	}
	bool CNewUIStamina::Render()
	{
		if(!IsStaminaUI())
			return false;

		EnableAlphaTest();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		
		// 게이지의 기본 중심좌표 7,3
 		ImageRotation(IMAGE_STAMINA_GAUGE,m_Pos.x-6,m_Pos.y-51-3, m_GaugeSize.x, m_GaugeSize.y,GetCaution());

		if(m_fGaugeBar >= 5)
			RenderImage(IMAGE_STAMINA_FRAME, m_Pos.x-m_FrameSize.x, m_Pos.y-m_FrameSize.y-51, m_FrameSize.x, m_FrameSize.y);
		else
			RenderImage(IMAGE_STAMINA_CAUTION, m_Pos.x-m_FrameSize.x, m_Pos.y-m_FrameSize.y-51, m_FrameSize.x, m_FrameSize.y);

		RenderTexts();

		DisableAlphaBlend();
		
		return true;
	}
	
	void CNewUIStamina::RenderTexts()
	{
		if(SEASON3B::CheckMouseIn(m_Pos.x-m_FrameSize.x, m_Pos.y-m_FrameSize.y-51, m_FrameSize.x, m_FrameSize.y))
		{
			char strfatigue[128]={0,};
			sprintf(strfatigue, GlobalText[2867], g_FatigueTimeSystem->GetFatiguePercentage());

			RenderTipText(m_Pos.x-m_FrameSize.x-10, m_Pos.y-m_FrameSize.y, strfatigue);
		}
	}

	float CNewUIStamina::ConvertX(float x)
	{
		return x*(float)WindowWidth/640.f;
	}

	float CNewUIStamina::ConvertY(float y)
	{
		return y*(float)WindowHeight/480.f;
	}

	void CNewUIStamina::ImageRotation(int Texture,float x,float y,float Width,float Height,float Rotate)
	{

		BITMAP_t *pImage = &Bitmaps[Texture];
		float u, v, uw, vh;

		u = 0.5f / ((float)pImage->Width);
		v = 0.5f / ((float)pImage->Height);
		uw = (Width - 0.5f) / ((float)pImage->Width);
		vh = (Height - 0.5f) / ((float)pImage->Height);

		float uWidth = uw - u;
		float vHeight = vh - v;

		x = ConvertX(x);
		y = ConvertY(y);
		Width = ConvertX(Width);
		Height = ConvertY(Height);
		
		BindTexture(Texture);

		vec3_t p[4],p2[4];
		y = WindowHeight - y;

		Vector(-Width*0.5f, Height,0.f,p[0]);
		Vector(-Width*0.5f,0,0.f,p[1]);
		Vector( Width*0.5f,0,0.f,p[2]);
		Vector( Width*0.5f, Height,0.f,p[3]);

		vec3_t Angle;
		Vector(0,0,Rotate,Angle);
		float Matrix[3][4];
		AngleMatrix(Angle,Matrix);

		float c[4][2];
		TEXCOORD(c[0],u,v);
		TEXCOORD(c[3],u+uWidth,v);
		TEXCOORD(c[2],u+uWidth,v+vHeight);
		TEXCOORD(c[1],u,v+vHeight);

		glBegin(GL_TRIANGLE_FAN);
		for(int i=0;i<4;i++)
		{
			glTexCoord2f(c[i][0],c[i][1]);
     		VectorRotate(p[i],Matrix,p2[i]);
			glVertex2f(p2[i][0]+x,p2[i][1]+y);
		}
		glEnd();

	}
	bool CNewUIStamina::UpdateMouseEvent()
	{
		if(!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STAMINA_GAUGE))
			return true;

		if(CheckMouseIn(m_Pos.x, m_Pos.y, IMAGE_FRAME_WIDTH, IMAGE_FRAME_HEIGHT))
		{
			if(SEASON3B::IsPress(VK_RBUTTON))
			{
				MouseRButton = false;
				MouseRButtonPop = false;
				MouseRButtonPush = false;
				return false;
			}
			
			if(SEASON3B::IsNone(VK_LBUTTON) == false)
			{
				return false;
			}
		}
		return true;
	}
	bool CNewUIStamina::UpdateKeyEvent()
	{
		return true;
	}
	bool CNewUIStamina::Update()
	{
		if(!IsStaminaUI())
			return true;
		
		g_pNewUIStamina->Show(SEASON3B::INTERFACE_STAMINA_GAUGE);
		return true;
	}
	float CNewUIStamina::GetLayerDepth()
	{
		return -100.0f;
	}
	void CNewUIStamina::OpeningProcess()
	{

	}
	void CNewUIStamina::ClosingProcess()
	{

	}

	BOOL CNewUIStamina::IsStaminaUI()
	{
		if(
		((World >= WD_18CHAOS_CASTLE && World <= WD_18CHAOS_CASTLE_END) || World == WD_53CAOSCASTLE_MASTER_LEVEL) ||
		((World >= WD_11BLOODCASTLE1 && World <= WD_11BLOODCASTLE_END) || World == WD_52BLOODCASTLE_MASTER_LEVEL) ||
		(World >= WD_45CURSEDTEMPLE_LV1 && World <= WD_45CURSEDTEMPLE_LV6) ||
		World == WD_6STADIUM ||
		(World == WD_65DOPPLEGANGER1) || (World == WD_66DOPPLEGANGER2) ||
		(World == WD_67DOPPLEGANGER3) || (World == WD_68DOPPLEGANGER4) ||
		(World == WD_69EMPIREGUARDIAN1) || (World == WD_70EMPIREGUARDIAN2) ||
		(World == WD_71EMPIREGUARDIAN3) || (World == WD_72EMPIREGUARDIAN4)

		|| (World == WD_30BATTLECASTLE)

		)
		{
			return false;
		}

		return true;
	}
}
#endif //PBG_MOD_STAMINA_UI