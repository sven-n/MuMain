// ServerInfo.h: interface for the CServerInfo class.
//////////////////////////////////////////////////////////////////////

#pragma once

class CServerInfo
{
public:
    CServerInfo();
    virtual ~CServerInfo();

    int			m_iSequence;
    int			m_iIndex;
    int			m_iConnectIndex;
    int			m_iPercent;
    BYTE		m_byNonPvP;
    wchar_t m_bName[MAX_TEXT_LENGTH];
};
