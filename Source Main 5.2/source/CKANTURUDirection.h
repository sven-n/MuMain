#ifndef _KANTURUDIRECTION_H_
#define _KANTURUDIRECTION_H_

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CKanturuDirection
{
public:
    int		m_iKanturuState;
    int		m_iMayaState;
    int		m_iNightmareState;
    bool	m_bKanturuDirection;

    CKanturuDirection();
    virtual ~CKanturuDirection();

    void Init();
    bool IsKanturuDirection();
    bool IsKanturu3rdTimer();
    bool IsMayaScene();
    void GetKanturuAllState(BYTE State, BYTE DetailState);
    void KanturuAllDirection();
    bool GetMayaExplotion();
    void SetMayaExplotion(bool MayaDie);
    bool GetMayaAppear();
    void SetMayaAppear(bool MayaDie);

private:
    bool	m_bMayaDie;
    bool	m_bMayaAppear;
    bool	m_bDirectionEnd;

    void GetKanturuMayaState(BYTE DetailState);
    void GetKanturuNightmareState(BYTE DetailState);

    void KanturuMayaDirection();
    void Move1stDirection();
    void Direction1st0();
    void Direction1st1();
    void Move2ndDirection();
    void Direction2nd0();
    void Direction2nd1();
    void Direction2nd2();

    void KanturuNightmareDirection();
    void Move3rdDirection();
    void Direction3rd0();
    void Direction3rd1();
    void Move4thDirection();
    void Direction4th0();
    void Direction4th1();
};

#endif //_KANTURUDIRECTION_H_