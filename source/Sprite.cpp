//*****************************************************************************
// File: Sprite.cpp
//
// Desc: implementation of the CSprite class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "Sprite.h"

#include "Input.h"

#include "ZzzOpenglUtil.h"

#include <crtdbg.h>		// _ASSERT() 사용.

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSprite::CSprite()
{
	m_aFrameTexCoord = NULL;
	m_pTexture = NULL;
}

CSprite::~CSprite()
{
	Release();
}

//*****************************************************************************
// 함수 이름 : Release()
// 함수 설명 : 동적 할당한 메모리 해제.
//*****************************************************************************
void CSprite::Release()
{
	m_pTexture = NULL;
	SAFE_DELETE_ARRAY(m_aFrameTexCoord);
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 스프라이트를 생성.
//			   nTexID 값이 음수이면 텍스처가 없는 스프라이트를 생성함.
//			   애니메이션이 없고 텍스처의 특정 부분으로 생성하려면 nMaxFrame을
//			  1로 넣고 aAniTexCoord에 텍스처 위치를 주소를 넣으면 됨.
//			   fScaleX, fScaleY가 1.0f면 해상도 마다 픽셀 고정.
//			   800 * 600 가상 좌표일 경우 fScaleX엔 '윈도우 가로 크기 / 800'를
//			  fScaleY엔 '윈도우 세로 크기 / 600'을 넣어주면 됨. 이때 버택스의
//			  스크린 좌표(m_aScrCoord[])는 800 * 600 가상 좌표가 됨.
// 매개 변수 : nOrgWidth	: 텍스처 내 프레임의 픽셀 너비.
//			   nOrgHeight	: 텍스처 내 프레임의 픽셀 높비.
//			   nTexID		: 텍스처 ID.(기본값 -1)
//			   nMaxFrame	: 총 프레임 수.(기본값 0)
//							(aFrameCoord이 NULL일 경우 nMaxFrame는 무시됨.)
//			   aFrameCoord	: 각 프레임의 텍스처 내의 픽셀 좌표 배열.(기본값
//							 NULL)
//			   nDatumX		: 기준점X. (마우스 커서 Hot spot개념.기본값 0)
//			   nDatumY		: 기준점Y. (마우스 커서 Hot spot개념.기본값 0)
//			   bTile		: 타일링 여부.(기본값 false)
//			   nSizingDatums: 크기 조절 시 기준.(기본값 SPR_SIZING_DATUMS_LT)
//							(예 : 우하 기준 -> SPR_SIZING_DATUMS_RB)
//			   fScaleX		: X 확대, 축소 비율.
//							(내부 좌표는 변화 없음. 기본값 1.0f)
//			   fScaleY		: Y 확대, 축소 비율.
//							(내부 좌표는 변화 없음. 기본값 1.0f)
//*****************************************************************************
void CSprite::Create(int nOrgWidth, int nOrgHeight, int nTexID, int nMaxFrame,
					 SFrameCoord* aFrameCoord, int nDatumX, int nDatumY,
					 bool bTile, int nSizingDatums, float fScaleX,
					 float fScaleY)
{
	// 전에 Release()를 호출하지 않고 Create()를 또 호출할 경우 메모리가 낭비되
	//는 것을 방지하기 위해.
	Release();

	m_fOrgWidth = (float)nOrgWidth;
	m_fOrgHeight = (float)nOrgHeight;
	m_nTexID = nTexID;
	m_pTexture = Bitmaps.FindTexture(m_nTexID);

	m_fScrHeight = (float)WindowHeight / fScaleY;

	// 버택스의 스크린 좌표 초기화.
	m_aScrCoord[LT].fX = 0.0f;
	m_aScrCoord[LT].fY = m_fScrHeight;
	m_aScrCoord[LB].fX = 0.0f;
	m_aScrCoord[LB].fY = m_fScrHeight - m_fOrgHeight;
	m_aScrCoord[RB].fX = m_fOrgWidth;
	m_aScrCoord[RB].fY = m_fScrHeight - m_fOrgHeight;
	m_aScrCoord[RT].fX = m_fOrgWidth;
	m_aScrCoord[RT].fY = m_fScrHeight;

	m_nNowFrame = -1;	// 아래 SetNowFrame(0) 때문에 이 위치에서 초기화.

	if (-1 < m_nTexID)	// 텍스처가 있을 때.
	{
		// 텍스처 좌표 초기화.
		// 0.5f는 필터링시 경계선이 나오는 것을 방지해준다.
		m_aTexCoord[LT].fTU = 0.5f / m_pTexture->Width;
		m_aTexCoord[LT].fTV = 0.5f / m_pTexture->Height;
		m_aTexCoord[LB].fTU = 0.5f / m_pTexture->Width;
		m_aTexCoord[LB].fTV = (m_fOrgHeight - 0.5f) / m_pTexture->Height;
		m_aTexCoord[RB].fTU = (m_fOrgWidth - 0.5f) / m_pTexture->Width;
		m_aTexCoord[RB].fTV = (m_fOrgHeight - 0.5f) / m_pTexture->Height;
		m_aTexCoord[RT].fTU = (m_fOrgWidth - 0.5f) / m_pTexture->Width;
		m_aTexCoord[RT].fTV = 0.5f / m_pTexture->Height;

		// 애니메이션일 경우나 텍스처 특정 위치 지정일때.
		if (NULL != aFrameCoord)
		{
			_ASSERT(0 < nMaxFrame);

			m_nMaxFrame = nMaxFrame;

			m_aFrameTexCoord = new STexCoord[m_nMaxFrame];

			for (int i = 0; i < m_nMaxFrame; ++i)
			{
				// 0.5f는 필터링시 경계선이 나오는 것을 방지해준다.
				m_aFrameTexCoord[i].fTU = ((float)aFrameCoord[i].nX + 0.5f)
					/ m_pTexture->Width;
				m_aFrameTexCoord[i].fTV = ((float)aFrameCoord[i].nY + 0.5f)
					/ m_pTexture->Height;
			}

			m_nStartFrame = m_nEndFrame = 0;
			SetNowFrame(0);		// 0 번째 프레임으로 초기화함.

			// 애니메이션일 경우나 텍스처 특정 위치 지정일 경우는 타일링을 할
			//수 없음.
			m_bTile = false;
		}
		else	// 텍스처 좌표 0, 0 위치일때.
		{
			m_nMaxFrame = 0;
			m_nStartFrame = m_nEndFrame = -1;
			m_bTile = bTile;
		}
	}
	else				// 텍스처가 없을 때.
	{
		::memset(m_aTexCoord, 0, sizeof(STexCoord) * POS_MAX);

		m_nMaxFrame = 0;
		m_nStartFrame = m_nEndFrame = -1;
		m_bTile = false;
	}

	m_byAlpha = m_byRed = m_byGreen = m_byBlue = 255;

	m_fDatumX = (float)nDatumX;
	m_fDatumY = (float)nDatumY;

	m_bRepeat = false;
	m_dDelayTime = m_dDeltaTickSum = 0.0;
	m_nSizingDatums = nSizingDatums;
	m_fScaleX = fScaleX;
	m_fScaleY = fScaleY;
	m_bShow = false;
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 스프라이트를 생성.
//			   이미지 여러개 일 경우 사용 불가능.(애니메이션 불가)
//			   pImgInfo->nTexID 값이 음수이면 텍스처가 없는 스프라이트를 생성함.
//			  (이때 pImgInfo->nX와 pImgInfo->nY 값은 0, 0이어야 함)
//			   애니메이션이 없고 텍스처의 특정 부분으로 생성하려면 pImgInfo->nX
//			  와 pImgInfo->nY에 텍스처 내 이미지 시작 픽셀 좌표를 넣으면 됨.
//			   fScaleX, fScaleY가 1.0f면 해상도 마다 픽셀 고정.
//			   800 * 600 가상 좌표일 경우 fScaleX엔 '윈도우 가로 크기 / 800'를
//			  fScaleY엔 '윈도우 세로 크기 / 600'을 넣어주면 됨. 이때 버택스의
//			  스크린 좌표(m_aScrCoord[])는 800 * 600 가상 좌표가 됨.
// 매개 변수 : pImgInfo		: 텍스처 내 이미지 정보.
//			   nDatumX		: 기준점X. (마우스 커서 Hot spot개념.기본값 0)
//			   nDatumY		: 기준점Y. (마우스 커서 Hot spot개념.기본값 0)
//			   bTile		: 타일링 여부.(기본값 false)
//			   nSizingDatums: 크기 조절 시 기준.(기본값 SPR_SIZING_DATUMS_LT)
//							(예 : 우하 기준 -> SPR_SIZING_DATUMS_RB)
//			   fScaleX		: X 확대, 축소 비율.
//							(내부 좌표는 변화 없음. 기본값 1.0f)
//			   fScaleY		: Y 확대, 축소 비율.
//							(내부 좌표는 변화 없음. 기본값 1.0f)
//*****************************************************************************
void CSprite::Create(SImgInfo* pImgInfo, int nDatumX, int nDatumY, bool bTile,
					 int nSizingDatums, float fScaleX, float fScaleY)
{
	// 텍스처 내 이미지 좌표가 0, 0일 경우.
	if (pImgInfo->nX == 0 && pImgInfo->nY == 0)
		Create(pImgInfo->nWidth, pImgInfo->nHeight, pImgInfo->nTexID, 0, NULL,
			nDatumX, nDatumY, bTile, nSizingDatums, fScaleX, fScaleY);
	// 텍스처 내 이미지 좌표가 0, 0가 아닌 특정 위치일 경우.
	else
	{
		SFrameCoord frameCoord = { pImgInfo->nX, pImgInfo->nY };
		Create(pImgInfo->nWidth, pImgInfo->nHeight, pImgInfo->nTexID, 1,
			&frameCoord, nDatumX, nDatumY, bTile, nSizingDatums, fScaleX,
			fScaleY);
	}
}

//*****************************************************************************
// 함수 이름 : SetPosition()
// 함수 설명 : 스프라이트의 이동.
// 매개 변수 : nXCoord		: X좌표.(스크린 좌표.)
//			   nYCoord		: Y좌표.(스크린 좌표.)
//			   eChangedPram	: 변경할 파라미터.
//							  예를 들자면 X는 X좌표만, Y는 Y좌표만, XY는 XY 좌
//							 표만 변경.(기본값 XY)
//*****************************************************************************
void CSprite::SetPosition(int nXCoord, int nYCoord, CHANGE_PRAM eChangedPram)
{
	// 버텍스 좌표 세팅. eChangedPram 값이 XY라면 아래의 2개 if문 모두 참.
	if (eChangedPram & X)	// X좌표를 바꾸는가? 
	{
		// 미리 이전의 너비를 얻어옴.
		float fWidth = m_aScrCoord[RT].fX - m_aScrCoord[LT].fX;

		// 크기조절 기준이 오른쪽인가?
		if (IS_SIZING_DATUMS_R(m_nSizingDatums))
		{
			// 오른쪽 기준으로 왼쪽으로 늘어 난다면, 스프라이트 생성시 기준점
			//nDatumX을 스프라이트 오른쪽 끝으로 잡는것이 좋다.
			// 이때 nXCoord 값은 스프라이트 오른쪽 끝이 된다.
			m_aScrCoord[RT].fX = m_aScrCoord[RB].fX
				= (float)nXCoord + m_fOrgWidth - m_fDatumX;
			m_aScrCoord[LT].fX = m_aScrCoord[LB].fX
				= m_aScrCoord[RT].fX - fWidth;
		}
		else
		{
			m_aScrCoord[LT].fX = m_aScrCoord[LB].fX
				= (float)nXCoord - m_fDatumX;
			m_aScrCoord[RT].fX = m_aScrCoord[RB].fX
				= m_aScrCoord[LT].fX + fWidth;
		}
	}	

	if (eChangedPram & Y)	// Y좌표를 바꾸는가?
	{
		// 미리 이전의 높이를 얻어옴.
		float fHeight = m_aScrCoord[LT].fY - m_aScrCoord[LB].fY;

		// 크기조절 기준이 아래인가?
		if (IS_SIZING_DATUMS_B(m_nSizingDatums))
		{
			m_aScrCoord[LB].fY = m_aScrCoord[RB].fY =
				m_fScrHeight - (float)nYCoord - m_fOrgHeight + m_fDatumY;

			m_aScrCoord[LT].fY = m_aScrCoord[RT].fY
				= m_aScrCoord[LB].fY + fHeight;
		}
		else
		{
			m_aScrCoord[LT].fY = m_aScrCoord[RT].fY
				= m_fScrHeight - (float)nYCoord + m_fDatumY;
			m_aScrCoord[LB].fY = m_aScrCoord[RB].fY
				= m_aScrCoord[LT].fY - fHeight;
		}
	}
}

//*****************************************************************************
// 함수 이름 : SetSize()
// 함수 설명 : 크기조절 기준으로 스프라이트의 크기 변경.
// 매개 변수 : nWidth		: 너비.(픽셀 단위.)
//			   nHeight		: 높이.(픽셀 단위.)
//			   eChangedPram	: 변경할 파라미터. X는 너비만, Y는 높이만, XY는 모
//							 두 변경.(기본값 XY)
//*****************************************************************************
void CSprite::SetSize(int nWidth, int nHeight, CHANGE_PRAM eChangedPram)
{
	// eChangedPram 값이 XY라면 아래의 2개 if문 모두 참.
	if (eChangedPram & X)	// 가로 크기 변경인가?
	{
		// 크기조절 기준이 오른쪽인가?
		if (IS_SIZING_DATUMS_R(m_nSizingDatums))
		{
			m_aScrCoord[LT].fX = m_aScrCoord[LB].fX
				= m_aScrCoord[RT].fX - (float)nWidth;
			if (m_bTile)	// 타일링.
				m_aTexCoord[LT].fTU = m_aTexCoord[LB].fTU
					= m_aTexCoord[RT].fTU - nWidth / m_pTexture->Width;
		}
		else
		{
			m_aScrCoord[RT].fX = m_aScrCoord[RB].fX
				= m_aScrCoord[LT].fX + (float)nWidth;
			if (m_bTile)	// 타일링.
				m_aTexCoord[RT].fTU = m_aTexCoord[RB].fTU
					= nWidth / m_pTexture->Width;
		}
	}
	if (eChangedPram & Y)	// 세로 크기 변경인가?
	{
		// 크기조절 기준이 아래쪽인가?
		if (IS_SIZING_DATUMS_B(m_nSizingDatums))
		{
			m_aScrCoord[LT].fY = m_aScrCoord[RT].fY
				= m_aScrCoord[LB].fY + (float)nHeight;
			if (m_bTile)	// 타일링.
				m_aTexCoord[LT].fTV = m_aTexCoord[RT].fTV
					= m_aTexCoord[LB].fTV - nHeight / m_pTexture->Height;
		}
		else
		{
			m_aScrCoord[LB].fY = m_aScrCoord[RB].fY
				= m_aScrCoord[LT].fY - (float)nHeight;
			if (m_bTile)	// 타일링.
				m_aTexCoord[LB].fTV = m_aTexCoord[RB].fTV
					= nHeight / m_pTexture->Height;
		}
	}
}

//*****************************************************************************
// 함수 이름 : PtInSprite()
// 함수 설명 : 스프라이트 안에 검색할 점이 있으면 TRUE, 없으면 FALSE를 리턴.
// 매개 변수 : lXPos	: 검색할 점의 X좌표.
//			   lYPos	: 검색할 점의 Y좌표.
//*****************************************************************************
BOOL CSprite::PtInSprite(long lXPos, long lYPos)
{
	if (!m_bShow)	// 보여주지 않는다면 판단하지 않고 무조건 FALSE 리턴.
		return FALSE;

	// 검색할 점 구조체(pt) 초기화.
	POINT pt = { lXPos, lYPos };

	RECT rc = {
		long(m_aScrCoord[LT].fX * m_fScaleX),
		long((m_fScrHeight - m_aScrCoord[LT].fY) * m_fScaleY),
		long(m_aScrCoord[RB].fX * m_fScaleX),
		long((m_fScrHeight - m_aScrCoord[RB].fY) * m_fScaleY)
	};

	// rc안에 pt가 있는가?
	return ::PtInRect(&rc, pt);
}

//*****************************************************************************
// 함수 이름 : CursorInObject()
// 함수 설명 : 스프라이트 안에 마우스 커서가 있으면 TRUE, 없으면 FALSE를 리턴.
//*****************************************************************************
BOOL CSprite::CursorInObject()
{
	CInput& rInput = CInput::Instance();

	return PtInSprite(rInput.GetCursorX(), rInput.GetCursorY());
}

//*****************************************************************************
// 함수 이름 : SetAction()
// 함수 설명 : 시작 프레임 번호와 마지막 프레임 번호를 세팅하여 차례로 애니메이
//			  션 시킴. 시작 프레임 번호와 마지막 프레임 번호가 같다면 그 프레임
//			  만 보여줌.(이때 이후에 Update()가 호출되어야 함.)
// 매개 변수 : nStartFrame	: 시작 프레임 번호.
//			   nEndFrame	: 마지막 프레임 번호.
//			   dDelayTime	: 프레임 간 지연 시간.(기본값 0.0)
//			   bRepeat		: Animation 반복 여부.(기본값 true)
//*****************************************************************************
void CSprite::SetAction(int nStartFrame, int nEndFrame, double dDelayTime,
						bool bRepeat)
{
	if (1 >= m_nMaxFrame)	// 애니메이션이 없을 경우.
		return;

	_ASSERT(nStartFrame <= nEndFrame && nStartFrame >= 0
		&& nEndFrame < m_nMaxFrame);

	m_nStartFrame = m_nNowFrame = nStartFrame;
	m_nEndFrame = nEndFrame;
	m_bRepeat = bRepeat;
	m_dDelayTime = dDelayTime;
}

//*****************************************************************************
// 함수 이름 : SetNowFrame()
// 함수 설명 : 현재 프레임을 nFrame으로 바꿈.
// 매개 변수 : nFrame	: 바꿀 프레임 번호.(0 이상의 값)
//*****************************************************************************
void CSprite::SetNowFrame(int nFrame)
{
	// 애니메이션이 없거나 프레임번호가 같으면.
	if (NULL == m_aFrameTexCoord || nFrame == m_nNowFrame)
		return;

//	_ASSERT(nFrame >= m_nStartFrame && nFrame <= m_nEndFrame);

	// 애니메이션 범위를 벗어나면.
	if (nFrame < m_nStartFrame || nFrame > m_nEndFrame)
		return;

	m_nNowFrame = nFrame;

	float fTUWidth = m_aTexCoord[RT].fTU - m_aTexCoord[LT].fTU;
	float fTVHeight = m_aTexCoord[LB].fTV - m_aTexCoord[LT].fTV;

	m_aTexCoord[LT] = m_aFrameTexCoord[m_nNowFrame];

	m_aTexCoord[RT].fTU = m_aFrameTexCoord[m_nNowFrame].fTU + fTUWidth;
	m_aTexCoord[RT].fTV = m_aFrameTexCoord[m_nNowFrame].fTV;

	m_aTexCoord[LB].fTU = m_aFrameTexCoord[m_nNowFrame].fTU;
	m_aTexCoord[LB].fTV = m_aFrameTexCoord[m_nNowFrame].fTV + fTVHeight;

	m_aTexCoord[RB].fTU = m_aTexCoord[RT].fTU;
	m_aTexCoord[RB].fTV = m_aTexCoord[LB].fTV;
}

//*****************************************************************************
// 함수 이름 : Update()
// 함수 설명 : m_dDelayTime만큼 시간이 지나면 다음 프레임으로 바꿈.
// 매개 변수 : dDeltaTick	: 이전 Update()호출 후부터 지금 Update()까지 시간.
//							  (기본값: 0.0)
//*****************************************************************************
void CSprite::Update(double dDeltaTick)
{
	if (!m_bShow)	// 보여주지 않을 경우 return.
		return;

	if (1 >= m_nMaxFrame)	// 애니메이션이 없을 경우.
		return;

	m_dDeltaTickSum += dDeltaTick;

	// m_dDelayTime만큼 시간이 지났는가?
	if (m_dDeltaTickSum >= m_dDelayTime)
	{
		int nFrame = m_nNowFrame;

		if (m_bRepeat)	// 반복인가?
			// 현재 프레임을 증가시키고 이것이 마지막 프레임 보다 크면 시작 프
			//레임으로 전환.
			nFrame = ++nFrame > m_nEndFrame ? m_nStartFrame : nFrame;
		else			// 반복이 아니면.
			// 현재 프레임을 증가시키고 이것이 마지막 프레임 보다 크면 마지막
			//프레임으로 유지.
			nFrame = ++nFrame > m_nEndFrame ? m_nEndFrame : nFrame;

		SetNowFrame(nFrame);

		m_dDeltaTickSum = 0.0;
	}
	// 이 클래스에서는 매 프래임 마다 기준점 보정을 하지 않음. 파생 클래스인
	//CCursor 클래스에서는 SetPosition()을 호출해서 기준점 보정을 함.
}

//*****************************************************************************
// 함수 이름 : Render()
// 함수 설명 : 랜더.
//*****************************************************************************
void CSprite::Render()
{
	if (!m_bShow)
		return;

	if (-1 < m_nTexID)	// 텍스처가 있는 경우.
	{
		// TextureEnable은 어쩔 수 없이 사용함 ㅜㅜ.
		if (!TextureEnable) 
		{
			TextureEnable = true;
			::glEnable(GL_TEXTURE_2D);
		}

		BindTexture(m_nTexID);

		::glBegin(GL_TRIANGLE_FAN);

		::glColor4ub(m_byRed, m_byGreen, m_byBlue, m_byAlpha);

		for (int i = LT; i < POS_MAX; ++i)
		{
			::glTexCoord2f(m_aTexCoord[i].fTU, m_aTexCoord[i].fTV);
			::glVertex2f(m_aScrCoord[i].fX * m_fScaleX,
				m_aScrCoord[i].fY * m_fScaleY);
		}

		::glEnd();
	}
	else				// 텍스처가 없는 경우.
	{
//		::DisableTexture();	// ㅜㅜ 사용하기 싫었음.
		if (TextureEnable) 
		{
			TextureEnable = false;
			::glDisable(GL_TEXTURE_2D);
		}

		::glBegin(GL_TRIANGLE_FAN);

		::glColor4ub(m_byRed, m_byGreen, m_byBlue, m_byAlpha);
		for (int i = LT; i < POS_MAX; ++i)
			::glVertex2f(m_aScrCoord[i].fX * m_fScaleX,
				m_aScrCoord[i].fY * m_fScaleY);

		::glEnd();
	}
}