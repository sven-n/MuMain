// UIDefaultBase.cpp: implementation of the CUIDefaultBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIDefaultBase.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef UIDEFAULTBASE

using namespace SEASON3A;

CUIDefaultBase::CUIDefaultBase(const string& uiname) : m_IsOpen(false), m_UIName(uiname)
{
}

CUIDefaultBase::~CUIDefaultBase()
{
}

#endif //UIDEFAULTBASE