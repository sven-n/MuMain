//*****************************************************************************
// File: Text.cpp
//
// Desc: implementation of the CText class.
//
// producer: Ahn Sang-Kyu (2007. 2. 6)
//*****************************************************************************

#include "stdafx.h"
#include "Text.h"
#include <crtdbg.h>  // _ASSERT() 사용.

#ifdef NEW_USER_INTERFACE_FONT

extern bool TextureEnable;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CText::CText() : m_pwTexImg(NULL)
{
}

CText::~CText()
{
	Release();
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 텍스처 및 버텍스 생성 등 데이터 멤버 초기화.
//			   가능하면 nFontHeight + eStyle 값이 16을 넘지 않게 하자. 넘는다면
//			  텍스처 높이가 32가 되므로 2배가 되어버린다.
// 매개 변수 : lpszFont		: 폰트 이름 문자열.
//			   nFontHeight	: 폰트 높이.(픽셀 단위)
//							  반드시 이 높이로 나타나지 않는다는 것을 유념하자.
//							이 높이 보다 클리는 없지만 작게 보일 수 있다.
//			   nTextMaxLen	: Text 최대 문자 길이.
//							  최소로 정해주는 것이 좋다. 이 값에 따라 텍스처 크
//							기가 결정된다.
//			   eStyle		: TS_NORMAL, TS_SHADOW, TS_OUTLINE 중 하나.
//							(기본값은 TS_SHADOW).
//							  Text의 너비, 높이가 TS_SHADOW는 1픽셀, TS_OUTLINE
//							은 2픽셀 늘어남.
//			   eType		: TT_NORMAL, TT_MONEY, TT_PASSWORD 중 하나.
//							(기본값 TT_NORMAL)
//							  TT_MONEY는 3자리 마다 ','가 찍히며, TT_PASSWORD는
//							어떤 문자열을 집어 넣든 '*'로 보여진다. m_szText의
//							내용은 변화없다.
//*****************************************************************************
void CText::Create(LPCTSTR lpszFont, int nFontHeight, int nTextMaxLen,
				   TEXT_STYLE eStyle, TEXT_TYPE eType)
{
	// 텍스처의 높이는 최대 TB_HEIGHT이므로 nFontHeight값 제한.
	//eStyle에 대한 설명은 위 함수 설명 참조.
	_ASSERT(0 < nFontHeight && TB_HEIGHT >= nFontHeight + eStyle);

	// 텍스트 비트맵 오브젝트에 폰트를 추가하고, 문자 최대 너비를 얻음.
	//동일 폰트, 크기라면 문자 최대 너비만 얻어옴.
	CTextBitmap& rTextBitmap = CTextBitmap::Instance();
	m_nCharMaxWidth = rTextBitmap.AddFont(lpszFont, nFontHeight);

// 텍스처 너비 구하기.
	// 최대 글자 수 만큼 표현하기 위한 최소한의 텍스처 너비.
	int nTempWidth = nTextMaxLen * m_nCharMaxWidth + eStyle;
	// 텍스처 너비를 8부터 배로 증가(우로 쉬프트)하여 최대 비트맵 크기
	//(TB_WIDTH)까지 최소한의 텍스처 너비와 비교하여 결정.
	for (m_nTexWidth = 8; m_nTexWidth < TB_WIDTH; m_nTexWidth <<= 1)
		if (m_nTexWidth >= nTempWidth)
			break;
// 텍스처 높이 구하기.
	m_nTexHeight = 16 >= nFontHeight + eStyle ? 16 : 32;

	Release();		// 텍스처가 있으면 지움.

// 텍스처 생성.
	m_pwTexImg = new WORD[m_nTexWidth * m_nTexHeight];
	::memset(m_pwTexImg, 0, m_nTexWidth * m_nTexHeight * sizeof(WORD));

	::glGenTextures(1, &m_uiTexID);				// 텍스처 ID 얻음.
	::glBindTexture(GL_TEXTURE_2D, m_uiTexID);	// 텍스처 바인드.
	// 텍스처 이미지 정의. R(5bit)G(5bit)B(5bit)A(1bit)로 16비트 컬러임.
	// 처리속도로 보나 메모리 크기로 보나 텍스트는 32비트 컬러까지 사용할 필요
	//는 없다는 판단.
	::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, m_nTexWidth, m_nTexHeight, 0,
		GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, m_pwTexImg);
// 텍스처 효과 정의.
//	::glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	// 텍스처 필터링.
//	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	::strcpy(m_szFont, lpszFont);		// 폰트 이름 저장.
	m_nFontHeight = nFontHeight;		// 폰트 높이 저장.
	// 텍스트 최대 길이는 텍스처 범위를 벗어나서는 안됨.
	int nTempLen = m_nTexWidth / m_nCharMaxWidth;
	m_nTextMaxLen = nTempLen > nTextMaxLen ? nTextMaxLen : nTempLen;
	SetTextColor(RGB(255, 255, 255));	// 텍스트 컬러.
	m_eStyle = eStyle;					// 텍스트 스타일.
	m_eType = eType;					// 텍스트 타잎.
	m_byAlign = TA_LEFT;				// 텍스트 정렬 방식.
	m_bShow = false;
	::memset(m_szText, 0, TEXT_MAX_LEN + 1);// 랜더할 문자열.
	m_nTextWidth = 0;
	m_nDatumX = 0;

// 버텍스 설정.
	// 버택스의 스크린 좌표 초기화.
	m_aScrCoord[LT].fX = 0.0f;
	m_aScrCoord[LT].fY = (float)WindowHeight;
	m_aScrCoord[LB].fX = 0.0f;
	m_aScrCoord[LB].fY = float(WindowHeight - m_nTexHeight);
	m_aScrCoord[RB].fX = (float)m_nTexWidth;
	m_aScrCoord[RB].fY = float(WindowHeight - m_nTexHeight);
	m_aScrCoord[RT].fX = (float)m_nTexWidth;
	m_aScrCoord[RT].fY = (float)WindowHeight;

	// 텍스처 좌표 초기화.
	m_aTexCoord[LT].fTU = 0.0f;
	m_aTexCoord[LT].fTV = 0.0f;
	m_aTexCoord[LB].fTU = 0.0f;
	m_aTexCoord[LB].fTV = 1.0f;
	m_aTexCoord[RB].fTU = 1.0f;
	m_aTexCoord[RB].fTV = 1.0f;
	m_aTexCoord[RT].fTU = 1.0f;
	m_aTexCoord[RT].fTV = 0.0f;

/*	m_aTexCoord[LT].fTU = 0.5f / float(m_nTexWidth);
	m_aTexCoord[LT].fTV = 0.5f / float(TB_HEIGHT);
	m_aTexCoord[LB].fTU = 0.5f / float(m_nTexWidth);
	m_aTexCoord[LB].fTV = 1.0f - 0.5f / float(TB_HEIGHT);
	m_aTexCoord[RB].fTU = 1.0f - 0.5f / float(m_nTexWidth);
	m_aTexCoord[RB].fTV = 1.0f - 0.5f / float(TB_HEIGHT);
	m_aTexCoord[RT].fTU = 1.0f - 0.5f / float(m_nTexWidth);
	m_aTexCoord[RT].fTV = 0.5f / float(TB_HEIGHT);
*/
	m_byAlpha = m_byRed = m_byGreen = m_byBlue = 255;
}

//*****************************************************************************
// 함수 이름 : Release()
// 함수 설명 : 텍스처가 있으면 지움.
//*****************************************************************************
void CText::Release()
{
	if (m_pwTexImg)
	{
		::glDeleteTextures(1, &m_uiTexID);
		delete [] m_pwTexImg;
		m_pwTexImg = NULL;
	}
}

//*****************************************************************************
// 함수 이름 : SetSize()
// 함수 설명 : 버텍스 사각형 크기 조절.
// 매개 변수 : nWidth	: 버텍스 사각형 너비.
//			   nHeight	: 버텍스 사각형 높이.
//*****************************************************************************
/*void CText::SetSize(int nWidth, int nHeight)
{
	m_nWidth = m_nTexWidth > nWidth ? nWidth : m_nTexWidth;
	m_aTLVertex[1].x = m_aTLVertex[3].x = m_aTLVertex[0].x + m_nWidth;
	m_aTLVertex[1].tu = m_aTLVertex[3].tu
		= (float)m_nWidth / (float)m_nTexWidth;

	m_nHeight = TB_HEIGHT > nHeight ? nHeight : TB_HEIGHT;
	m_aTLVertex[2].y = m_aTLVertex[3].y = m_aTLVertex[0].y + m_nHeight;
	m_aTLVertex[2].tv = m_aTLVertex[3].tv
		= (float)m_nHeight / (float)TB_HEIGHT;
}
*/
//*****************************************************************************
// 함수 이름 : SetPosition()
// 함수 설명 : 버텍스 사각형 위치 조절.
// 매개 변수 : nXCoord		: X좌표.
//			   nYCoord		: Y좌표.
//			   eChangedPram	: 변경할 파라미터.
//							  예를 들자면 X는 X좌표만, Y는 Y좌표만, XY는 XY 좌
//							 표만 변경.(기본값 XY)
//*****************************************************************************
void CText::SetPosition(int nXCoord, int nYCoord, CHANGE_PRAM eChangedPram)
{
	// 버텍스 좌표 세팅. eChangedPram 값이 XY라면 아래의 2개 if문 모두 참.
	if (eChangedPram & X)	// X좌표를 바꾸는가? 
	{
		// 미리 이전의 너비를 얻어옴.
		float fWidth = m_aScrCoord[RT].fX - m_aScrCoord[LT].fX;

		m_nDatumX = nXCoord;

		switch (m_byAlign)
		{
		case TA_CENTER:
			m_aScrCoord[LT].fX = m_aScrCoord[LB].fX
				= (float)(m_nDatumX - m_nTextWidth / 2);
			break;

		case TA_RIGHT:
			m_aScrCoord[LT].fX = m_aScrCoord[LB].fX
				= (float)(m_nDatumX - m_nTextWidth);
			break;

		default:
			m_aScrCoord[LT].fX = m_aScrCoord[LB].fX = (float)m_nDatumX;
		}

		m_aScrCoord[RT].fX = m_aScrCoord[RB].fX = m_aScrCoord[LT].fX + fWidth;
	}

	if (eChangedPram & Y)	// Y좌표를 바꾸는가?
	{
		// 미리 이전의 높이를 얻어옴.
		float fHeight = m_aScrCoord[LT].fY - m_aScrCoord[LB].fY;

		m_aScrCoord[LT].fY = m_aScrCoord[RT].fY = float(WindowHeight - nYCoord);
		m_aScrCoord[LB].fY = m_aScrCoord[RB].fY = m_aScrCoord[LT].fY - fHeight;
	}
}

//*****************************************************************************
// 함수 이름 : SetText()
// 함수 설명 : 문자열을 비트맵에 쓰고 텍스처로 복사.
// 매개 변수 : lpszText	: 표현할 문자열.
//*****************************************************************************
void CText::SetText(LPCTSTR lpszText)
{
//	_ASSERT(::strlen(lpszText) <= TEXT_MAX_LEN);

	if (TT_MONEY == m_eType)	// 출력 형태가 돈 타잎이라면.
	{
		char szMoney[TEXT_MAX_LEN + 1];
		*szMoney = 0;	// 초기화.
		// 매개변수를 돈 타잎(3자리마다','로 구분)으로 변형함.
		NUMBERFMT nFmt = { 0, 0, 3, ".", ",", 1 };
		::GetNumberFormat(NULL, NULL, lpszText, &nFmt, szMoney,
			TEXT_MAX_LEN + 1);

		// 최대 글자 수 만큼만 비교하여 텍스트가 변하지 않았다면 그냥 리턴.
		if (0 == strncmp(m_szText, szMoney, m_nTextMaxLen))
			return;
		// 최대 글자 수 만큼만 m_szText에 보관.
		::strncpy(m_szText, szMoney, m_nTextMaxLen);
	}
	else
	{
		// 최대 글자 수 만큼만 비교하여 텍스트가 변하지 않았다면 그냥 리턴.
		if (0 == strncmp(m_szText, lpszText, m_nTextMaxLen))
			return;
		// 최대 글자 수 만큼만 m_szText에 보관.
		::strncpy(m_szText, lpszText, m_nTextMaxLen);
	}

	// 저장 해 놓은 폰트 이름과 크기로 폰트 선택.
	CTextBitmap& rTextBitmap = CTextBitmap::Instance();
	rTextBitmap.SelFont(m_szFont, m_nFontHeight);

	if (TT_PASSWORD == m_eType)	// 암호 입력인가?
	{
		// 비트맵에 '*'을 글자 수 만큼 찍고 픽셀 너비를 얻음.
		char szAsterisk[TEXT_MAX_LEN + 1];
		::strcpy(szAsterisk, m_szText);
		::_strset(szAsterisk, '*');
		m_nTextWidth = rTextBitmap.TextOut(
			szAsterisk, m_crTextColor, m_eStyle, m_crStyleColor);
	}
	else
		// 비트맵에 문자열을 쓰고 문자열 픽셀 너비를 얻음.
		m_nTextWidth = rTextBitmap.TextOut(
			m_szText, m_crTextColor, m_eStyle, m_crStyleColor);

// 비트맵을 텍스처로 카피.
	// 비트맵에서 복사할 시작 위치.
	WORD* pwSrc = rTextBitmap.GetBitmap();
	WORD* pwDst = m_pwTexImg;			// 텍스처 이미지에 복사될 시작 위치.
	// 아래 코드가 nLockWidth = m_nTextWidth 이라면 표시될 문자열이 이전 보다
	//너비가 작은 경우에는 그 차이 만큼 이전 텍스트가 보이게 되므로 안됨.
	int nLockWidth = m_nTextMaxLen * m_nCharMaxWidth;	// 복사할 너비.
	// 아래 glTexSubImage2D()함수는 내부적으로 텍스처 이미지로 32bit씩 카피하는
	//듯 하다. 그런 이유로 여기서 사용된 텍스처 이미지는 16bit컬러이므로
	//nLockWidth가 홀수일 때는 한 픽셀씩 밀리게 된다. 따라서 nLockWidth가 홀수
	//일 때는 nLockWidth를 1증가 시켜주어 짝수로 만들어 그런 현상을 막는다. 텍
	//스처 이미지가 32bit이라면 이런 작업은 불필요할 듯하다.
	int nSrcNextLineGap
		= nLockWidth % 2 ? TB_WIDTH - ++nLockWidth : TB_WIDTH - nLockWidth;
	int nLockHeight = m_nFontHeight + m_eStyle;

	::memset(pwDst, 0, nLockWidth * nLockHeight * sizeof(WORD));
	int x, y;
	for (y = 0; y < nLockHeight; ++y)	// 텍스트 높이 만큼 반복.
    {
		for (x = 0; x < nLockWidth; ++x)// 복사할 너비 만큼 반복.
        {
			if (*pwSrc)				// 색이 있으면.
			{
				 *pwSrc <<= 1;		// 알파값 자리가 맨 오른쪽 bit이므로.
				 *pwSrc |= 0x0001;	// 알파값 세팅.(알파값이 0이면 안보이므로)
				 *pwDst = *pwSrc;	// 복사.
			}
			++pwDst;
			++pwSrc;
        }
		pwSrc += nSrcNextLineGap;				// 다음 픽셀 라인으로.
		// 텍스처 이미지인 pwDst는 아래 glTexSubImage2D()함수에서 nLockWidth만
		//알면 알아서 이미지를 변경(복사)시켜주기 때문에 다음 픽셀 라인으로 계
		//산하는 코드(예 : pwDst += nDstNextLineGap;)는 필요가 없다.
    }

	::glBindTexture(GL_TEXTURE_2D, m_uiTexID);	// 텍스처 바인드.
	// 바뀐 텍스처 이미지를 다시 정의.
	::glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nLockWidth, nLockHeight,
		GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, m_pwTexImg);

// 택스트 정렬이 TA_RIGHT이거나 TA_CENTER일 때만 다시 위치를 잡아줌.
	if (TA_RIGHT == m_byAlign || TA_CENTER == m_byAlign)
		SetPosition(m_nDatumX, 0, X);
}

//*****************************************************************************
// 함수 이름 : Render()
// 함수 설명 : 랜더링 수행.
//*****************************************************************************
void CText::Render()
{
	if (m_bShow)	// 보여준다면 랜더링 함.
	{
		// TextureEnable은 어쩔 수 없이 사용함 ㅜㅜ.
		if (!TextureEnable) 
		{
			TextureEnable = true;
			::glEnable(GL_TEXTURE_2D);
		}

//		::EnableAlphaTest();

		::glBindTexture(GL_TEXTURE_2D, m_uiTexID);

		::glBegin(GL_TRIANGLE_FAN);
		::glColor4ub(m_byRed, m_byGreen, m_byBlue, m_byAlpha);
		for (int i = LT; i < POS_MAX; ++i)
		{
			::glTexCoord2f(m_aTexCoord[i].fTU, m_aTexCoord[i].fTV);
			::glVertex2f(m_aScrCoord[i].fX, m_aScrCoord[i].fY);
		}
		::glEnd();
	}
}

#endif //NEW_USER_INTERFACE_FONT