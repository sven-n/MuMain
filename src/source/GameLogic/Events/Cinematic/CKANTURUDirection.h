#ifndef _KANTURUDIRECTION_H_
#define _KANTURUDIRECTION_H_

#include <cstdint>

class CKanturuDirection
{
public:
    int m_iKanturuState;
    int m_iMayaState;
    int m_iNightmareState;
    bool m_bKanturuDirection;

    CKanturuDirection();
    virtual ~CKanturuDirection();

    void Init();
    bool IsKanturuDirection() const;
    bool IsKanturu3rdTimer() const;
    bool IsMayaScene() const;
    void GetKanturuAllState(std::uint8_t State, std::uint8_t DetailState);
    void KanturuAllDirection();
    bool GetMayaExplotion() const;
    void SetMayaExplotion(bool MayaDie);
    bool GetMayaAppear() const;
    void SetMayaAppear(bool MayaDie);

private:
    struct DirectionTarget
    {
        int x;
        int y;
        int z;
        float distance;
    };

    bool m_bMayaDie;
    bool m_bMayaAppear;
    bool m_bDirectionEnd;

    void GetKanturuMayaState(std::uint8_t DetailState);
    void GetKanturuNightmareState(std::uint8_t DetailState);

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

    bool IsKanturuWorldActive() const;
    void PrepareCameraFocus(bool adjustViewDistance = true);
    void ActivateDirectionSequence();
    void ResetDirectionState();
    void ApplyDirectionTarget(const DirectionTarget& target);
};

#endif //_KANTURUDIRECTION_H_