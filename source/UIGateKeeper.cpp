//////////////////////////////////////////////////////////////////////////
//  UIGateKeeper.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIGateKeeper.h"
#include "wsclientinline.h"

CUIGateKeeper::CUIGateKeeper()
{
	m_bPublic		= false;
    m_byType        = TOUCH_TYPE_NONE;
	m_nEntranceFee	= 0;
    m_iAddEntranceFee = 0;
    m_iMaxEnteranceFee = 0;

    m_iViewEntranceFee = m_nEntranceFee;
}

CUIGateKeeper::~CUIGateKeeper()
{
}

void CUIGateKeeper::SendPublicSetting()
{
    SendRequestHuntZoneEnter ( (BYTE)(m_bPublic^true) );
}

void CUIGateKeeper::SendEnteranceFee()
{
    SendRequestBCChangeTaxRate ( 3, (m_iViewEntranceFee>>24), ((m_iViewEntranceFee>>16)&0xff), ((m_iViewEntranceFee>>8)&0xff), (m_iViewEntranceFee&0xff) );
}

void CUIGateKeeper::EnteranceFeeUp()
{
    m_iViewEntranceFee += m_iAddEntranceFee;
    if ( m_iViewEntranceFee>m_iMaxEnteranceFee )
    {
        m_iViewEntranceFee = m_iMaxEnteranceFee;
    }
}

void CUIGateKeeper::EnteranceFeeDown()
{
    m_iViewEntranceFee -= m_iAddEntranceFee;
    if ( m_iViewEntranceFee<0 )
    {
        m_iViewEntranceFee = 0;
    }
}

void CUIGateKeeper::SendEnter()
{
	SendCastleHuntZoneEnter ( m_nEntranceFee );
}
