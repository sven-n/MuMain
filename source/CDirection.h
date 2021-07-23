#ifndef _DIRECTION_H_
#define _DIRECTION_H_

//////////////////////////////////////////////////////////////////////////
// 
//  내  용 : 뮤 연출
//  
//  날  짜 : 2006년 7월 10일
//  
//  작성자 : 이 혁 재
//  
//////////////////////////////////////////////////////////////////////////

#include <VECTOR>
#include "Singleton.h"

#include "CMVP1stDirection.h"
#include "CKANTURUDirection.h"


using namespace std;

class CDirection : public Singleton <CDirection>
{
private:
	// LHJ - 카메라 관련 변수
	vec3_t	m_vCameraPosition;				// LHJ - 연출을 위한 카메라 위치
	vec3_t	m_v1stPosition;					// LHJ - 연출을 위한 처음 위치 좌표 저장
	vec3_t	m_v2ndPosition;					// LHJ - 연출을 위한 마지막 위치 좌표 저장
	vec3_t	m_vResult;						// LHJ - 시작 카메라 위치에서 연출 마지막 카메라 위치 결과 벡터; 

	bool	m_bCameraCheck;					// LHJ - 연출중 카메라 이동 가능 체크
	float	m_fCount;						// LHJ - 연출중 카메라 이동 범위
	float	m_fLength;						// LHJ - 연출중 카메라 이동 길이 저장
	float	m_fCameraSpeed;					// LHJ - 연출중 카메라 이동 속도

	// LHJ - 시간 관련 변수
	bool	m_bTimeCheck;					// LHJ - 시간 관련 체크	
	int		m_iBackupTime;					// LHJ - 시간 관련 백업
	bool	m_bStateCheck;					// LHJ - 시작,끝 연출을 위해 카메라 처음 위치값을 한번만 넘겨주기 위해

public:
	vector<DirectionMonster> stl_Monster;

	bool	m_bOrderExit;					// LHJ - 강제 종료 체크
	int		m_iTimeSchedule;				// LHJ - 연출 순서
	int		m_CameraLevel;					// LHJ - 연출시 카메라 더 뒤로 빼서 볼 수 있는 영역을 넓힘
	float	m_fCameraViewFar;				// LHJ - 카메라 멀리 보이는 거리 조절
	int		m_iCheckTime;					// LHJ - 연출중 진행 순서
	bool	m_bAction;						// LHJ - 연출중 소환이나 행동중인지 체크
	bool	m_bDownHero;					// LHJ - 케릭터 아래로 떨어지는지는 연출인지 체크
	float	m_AngleY;						// LHJ - 케릭터 떨어질때 바라보는 방향의 각도 저장함

	CKanturuDirection m_CKanturu;			// LHJ - 칸투르 연출
	CMVP1STDirection  m_CMVP;				// LHJ - 크라이울프 연출


	CDirection();
	virtual ~CDirection(); 

	void Init();								// LHJ - 초기화
	void CloseAllWindows();						// LHJ - 모든 열려진 창을 닫는다
	bool IsDirection();							// LHJ - 연출 상태 인지 체크	
	void CheckDirection();						// LHJ - 상태를 서버로 부터 받아와 연출일때 연출
	void SetCameraPosition();					// LHJ - 연출을 위한 처음 카메라 위치 세팅
	int GetCameraPosition(vec3_t GetPosition);	// LHJ - 연출을 위한 카메라 위치 값 넘겨줌
	bool DirectionCameraMove();					// LHJ - 연출중 카메라 이동
	void DeleteMonster();						// LHJ - 연출로 보여준 몬스터들 삭제


	float CalculateAngle(CHARACTER* c, int x, int y, float Angle);		// LHJ - 몬스터 다음 포지션으로 이동시 각도 계산
	void CameraLevelUp();												// LHJ - 카메라 뒤로 땡겨서 더 멀리 봄
	void SetNextDirectionPosition(int x, int y, int z, float Speed);	// LHJ - 클라이울프 연출시 다음 카메라 위치
	bool GetTimeCheck(int DelayTime);									// LHJ - 시간 관련 함수
	void HeroFallingDownDirection();									// LHJ - 케릭터가 떨어지는 연출
	void HeroFallingDownInit();											// LHJ - 케릭터가 떨어지는거 초기화
	void FaillingEffect();												// LHJ - 케릭터가 떨어질때 연기 효과

	// LHJ - 클라이울프 연출로 보여질 몬스터 생성
	void SummonCreateMonster(int Type, int x, int y, float Angle, bool NextCheck = true, bool SummonAni = true, float AniSpeed = -1.0f);
	// LHJ - 클라이울프 연출로 보여지기 위해 생성된 몬스터 이동
	bool MoveCreatedMonster(int Type, int x, int y, float Angle, int Speed);
	// LHJ - 클라이울프 연출로 보여지기 위해 생성된 몬스터 행동(에니메이션)
	bool ActionCreatedMonster(int Type, int Action, int Count, bool TankerAttack = false, bool NextCheck = false);

};

#define g_Direction CDirection::GetSingleton()

#endif _DIRECTION_H_