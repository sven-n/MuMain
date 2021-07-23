//*****************************************************************************
// File: LoadingScene.h
//
// Desc: interface for the CLoadingScene class.
//		 로딩씬 class.(본 게임 시작전 로딩임)
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_LOADINGSCENE_H__D5107C47_C7D8_49B8_8056_B21DDC7DACE0__INCLUDED_)
#define AFX_LOADINGSCENE_H__D5107C47_C7D8_49B8_8056_B21DDC7DACE0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Sprite.h"

#define LDS_BACK_MAX	4

class CLoadingScene
{
protected:
	CSprite	m_asprBack[LDS_BACK_MAX];	// 배경 그림이 4개로 나뉘어 있음.

public:
	CLoadingScene();
	virtual ~CLoadingScene();

	void Create();
	void Release();
	void Render();
};

#endif // !defined(AFX_LOADINGSCENE_H__D5107C47_C7D8_49B8_8056_B21DDC7DACE0__INCLUDED_)
