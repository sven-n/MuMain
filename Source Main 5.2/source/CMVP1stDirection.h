#ifndef _MVP1STDIRECTION_H_
#define _MVP1STDIRECTION_H_

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "Singleton.h"

class CMVP1STDirection
{
private:
    bool	m_bTimerCheck;

    void IsCryWolfDirectionTimer();

    void MoveBeginDirection();
    void BeginDirection0();
    void BeginDirection1();
    void BeginDirection2();
    void BeginDirection3();
    void BeginDirection4();
    void BeginDirection5();
public:
    int		m_iCryWolfState;

    CMVP1STDirection();
    virtual ~CMVP1STDirection();

    void Init();
    void GetCryWolfState(BYTE CryWolfState);
    bool IsCryWolfDirection();
    void CryWolfDirection();
};

#endif //_MVP1STINTERFACE_H_