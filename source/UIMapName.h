//*****************************************************************************
// File: UIMapName.h
//
// Desc: interface for the CUIMapName class.
//		 화면 상단중앙의 맵 이름 UI.
//
// producer: Ahn Sang-Kyu (2005. 6. 16)
//*****************************************************************************	

#if !defined(AFX_UIMAPNAME_H__6771C771_B81D_4D5C_8484_63D6961ED6C0__INCLUDED_)
#define AFX_UIMAPNAME_H__6771C771_B81D_4D5C_8484_63D6961ED6C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

using namespace std;

typedef map<int, string> ImgPathMap;

class CUIMapName  
{
	// (안보임, 서서히 나타남, 보임, 서서히 사라짐)
	enum SHOW_STATE { HIDE, FADEIN, SHOW, FADEOUT };

protected:
	ImgPathMap	m_mapImgPath;		// 이미지 경로 문자열 맵.(맵 번호가 키)
	short		m_nOldWorld;		// 이전 맵 번호.
	SHOW_STATE	m_eState;			// 보여주기 상태.
	DWORD		m_dwOldTime;		// 이전 프레임 시간.
	DWORD		m_dwDeltaTickSum;	// 프레임간 시간차.
	float		m_fAlpha;			// 알파값.
#ifdef ASG_ADD_GENS_SYSTEM
	bool		m_bStrife;			// 분쟁지역이면 true;
#endif	// ASG_ADD_GENS_SYSTEM

public:
	CUIMapName();
	virtual ~CUIMapName();

	void Init();
	void ShowMapName();
	void Update();
	void Render();

protected:
	void InitImgPathMap();
};

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
extern string g_strSelectedML;
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

#endif // !defined(AFX_UIMAPNAME_H__6771C771_B81D_4D5C_8484_63D6961ED6C0__INCLUDED_)
