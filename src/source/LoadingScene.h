//*****************************************************************************
// File: LoadingScene.h
//*****************************************************************************

#if !defined(AFX_LOADINGSCENE_H__D5107C47_C7D8_49B8_8056_B21DDC7DACE0__INCLUDED_)
#define AFX_LOADINGSCENE_H__D5107C47_C7D8_49B8_8056_B21DDC7DACE0__INCLUDED_

#pragma once

#include "Sprite.h"

#define LDS_BACK_MAX	4

class CLoadingScene
{
protected:
    CSprite	m_asprBack[LDS_BACK_MAX];

public:
    CLoadingScene();
    virtual ~CLoadingScene();

    void Create();
    void Release();
    void Render();
};

#endif // !defined(AFX_LOADINGSCENE_H__D5107C47_C7D8_49B8_8056_B21DDC7DACE0__INCLUDED_)
