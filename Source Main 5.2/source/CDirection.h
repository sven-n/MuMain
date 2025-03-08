#ifndef _DIRECTION_H_
#define _DIRECTION_H_

#include <VECTOR>
#include "Singleton.h"
#include "CMVP1stDirection.h"
#include "CKANTURUDirection.h"

class CDirection : public Singleton <CDirection>
{
private:
    vec3_t	m_vCameraPosition;
    vec3_t	m_v1stPosition;
    vec3_t	m_v2ndPosition;
    vec3_t	m_vResult;

    bool	m_bCameraCheck;
    float	m_fCount;
    float	m_fLength;
    float	m_fCameraSpeed;

    bool	m_bTimeCheck;
    int		m_iBackupTime;
    bool	m_bStateCheck;

public:
    std::vector<DirectionMonster> stl_Monster;

    bool	m_bOrderExit;
    int		m_iTimeSchedule;
    int		m_CameraLevel;
    float	m_fCameraViewFar;
    int		m_iCheckTime;
    bool	m_bAction;
    bool	m_bDownHero;
    float	m_AngleY;

    CKanturuDirection m_CKanturu;
    CMVP1STDirection  m_CMVP;

    CDirection();
    virtual ~CDirection();

    void Init();
    void CloseAllWindows();
    bool IsDirection();
    void CheckDirection();
    void SetCameraPosition();
    int GetCameraPosition(vec3_t GetPosition);
    bool DirectionCameraMove();
    void DeleteMonster();

    float CalculateAngle(CHARACTER* c, int x, int y, float Angle);
    void CameraLevelUp();
    void SetNextDirectionPosition(int x, int y, int z, float Speed);
    bool GetTimeCheck(int DelayTime);
    void HeroFallingDownDirection();
    void HeroFallingDownInit();
    void FaillingEffect();

    void SummonCreateMonster(EMonsterType Type, int x, int y, float Angle, bool NextCheck = true, bool SummonAni = true, float AniSpeed = -1.0f);
    bool MoveCreatedMonster(int Index, int x, int y, float Angle, int Speed);
    bool ActionCreatedMonster(int Index, int Action, int Count, bool TankerAttack = false, bool NextCheck = false);
};

#define g_Direction CDirection::GetSingleton()

#endif _DIRECTION_H_