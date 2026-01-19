// NewUISetItemExplanation.cpp: implementation of the CNewUISetItemExplanation class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUISetItemExplanation.h"
#include "NewUISystem.h"
#include "CSItemOption.h"
#include "DSPlaySound.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUISetItemExplanation::CNewUISetItemExplanation()
{
    m_pNewUIMng = NULL;
    m_Pos.x = 0;
    m_Pos.y = 0;
}

SEASON3B::CNewUISetItemExplanation::~CNewUISetItemExplanation()
{
    Release();
}

bool SEASON3B::CNewUISetItemExplanation::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_SETITEM_EXPLANATION, this);

    SetPos(x, y);

    Show(false);

    return true;
}

void SEASON3B::CNewUISetItemExplanation::Release()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUISetItemExplanation::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool SEASON3B::CNewUISetItemExplanation::UpdateMouseEvent()
{
    return true;
}

bool SEASON3B::CNewUISetItemExplanation::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_SETITEM_EXPLANATION))
    {
        if (IsPress(VK_ESCAPE) == true || IsPress(VK_F1) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_SETITEM_EXPLANATION);
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }

    return true;
}

bool SEASON3B::CNewUISetItemExplanation::Update()
{
    return true;
}

bool SEASON3B::CNewUISetItemExplanation::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    g_csItemOption.RenderOptionHelper();

    DisableAlphaBlend();

    return true;
}

float SEASON3B::CNewUISetItemExplanation::GetLayerDepth()
{
    return 6.6f;
}

float SEASON3B::CNewUISetItemExplanation::GetKeyEventOrder()
{
    return 10.f;
}

void SEASON3B::CNewUISetItemExplanation::OpenningProcess()
{
}

void SEASON3B::CNewUISetItemExplanation::ClosingProcess()
{
}