#ifndef _MVP1STDIRECTION_H_
#define _MVP1STDIRECTION_H_

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <cstdint>
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

    bool IsCrywolfWorldActive() const;
    bool IsSequenceReady() const;
    void ResetSequence();
    void QueueCameraMove(int x, int y, int z, float speed);

public:
    int		m_iCryWolfState;

    CMVP1STDirection();
    virtual ~CMVP1STDirection();

    void Init();
    void GetCryWolfState(std::uint8_t CryWolfState);
    bool IsCryWolfDirection() const;
    void CryWolfDirection();
};

#endif //_MVP1STINTERFACE_H_