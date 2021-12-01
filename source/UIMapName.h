//*****************************************************************************
// File: UIMapName.h
//
// producer: Ahn Sang-Kyu (2005. 6. 16)
//*****************************************************************************	

#if !defined(AFX_UIMAPNAME_H__6771C771_B81D_4D5C_8484_63D6961ED6C0__INCLUDED_)
#define AFX_UIMAPNAME_H__6771C771_B81D_4D5C_8484_63D6961ED6C0__INCLUDED_

#pragma once

typedef std::map<int, std::string> ImgPathMap;

class CUIMapName  
{
	enum SHOW_STATE { HIDE, FADEIN, SHOW, FADEOUT };

protected:
	ImgPathMap	m_mapImgPath;
	short		m_nOldWorld;
	SHOW_STATE	m_eState;
	DWORD		m_dwOldTime;
	DWORD		m_dwDeltaTickSum;
	float		m_fAlpha;
#ifdef ASG_ADD_GENS_SYSTEM
	bool		m_bStrife;
#endif

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

extern std::string g_strSelectedML;

#endif // !defined(AFX_UIMAPNAME_H__6771C771_B81D_4D5C_8484_63D6961ED6C0__INCLUDED_)
