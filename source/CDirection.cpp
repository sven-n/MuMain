#include "stdafx.h"
#include "CComGem.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzOpenData.h"
#include "ZzzEffect.h"
#include "ZzzAI.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzInterface.h"
#include "CDirection.h"
#include "wsclientinline.h"

static CDirection Direction;

CDirection::CDirection()
{
	Init();
}

CDirection::~CDirection()
{

}

void CDirection::Init()
{
	Vector(0.0f, 0.0f, 0.0f, m_vCameraPosition);
	Vector(0.0f, 0.0f, 0.0f, m_v1stPosition);
	Vector(0.0f, 0.0f, 0.0f, m_v2ndPosition);
	Vector(0.0f, 0.0f, 0.0f, m_vResult);

	m_bStateCheck = true;
	m_bCameraCheck = false;
	m_bAction = true;
	m_bTimeCheck = true;
	m_bOrderExit = false;
		
	m_bDownHero = false;

	m_fCount = 0.0f;
	m_fLength = 0.0f;
	m_fCameraSpeed = 100.0f;
	m_fCameraViewFar = 1000.0f;

	m_iCheckTime = 0;
	m_iTimeSchedule = 0;
	m_iBackupTime = 0;
	m_CameraLevel = 0;

	m_AngleY = 0.0f;

	m_CMVP.Init();
	m_CKanturu.Init();
}

void CDirection::CloseAllWindows()
{
	SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
	g_pNewUISystem->HideAll();
}

bool CDirection::IsDirection()
{
	if(World == WD_34CRYWOLF_1ST)
		return m_CMVP.IsCryWolfDirection();			// LHJ - 크라이울프 연출 상태인가 체크
	else if(World == WD_39KANTURU_3RD)
		return m_CKanturu.IsKanturuDirection();		// LHJ - 칸투르 연출 상태인가 체크

	return false;
}

void CDirection::CheckDirection()
{
	m_CMVP.CryWolfDirection();			// LHJ - 크라이울프 연출 상태이면 연출 수행
	m_CKanturu.KanturuAllDirection();	// LHJ - 칸투르 연출 상태이면 연출 수행
}

void CDirection::SetCameraPosition()
{
	if(m_bStateCheck)	// LHJ - 카메라가 이동 해야할 구간에서 위치를 설정
	{
		if(m_iTimeSchedule == 0)	// LHJ - 연출 순서가 처음 이라면
		{
			// LHJ - 자기 케릭터의 위치값을 카메라 처음 위치 값에 넘겨준다
			VectorCopy(Hero->Object.Position, m_v1stPosition);
			m_iTimeSchedule++;		// LHJ - 다음 연출 순서로 넘긴다
		}
		
		// LHJ - 카메라가 이동할 두 위치의 벡터를 구한다
		VectorSubtract(m_v2ndPosition, m_v1stPosition, m_vResult);
		// LHJ - 이동할 벡터의 길이를 구한다
		m_fLength = VectorLength(m_vResult);
	}
}

int CDirection::GetCameraPosition(vec3_t GetPosition)
{
	// LHJ - 연출을 위한 카메라 위치값 지정
	VectorCopy( m_vCameraPosition, GetPosition );
	// LHJ - 카메라가 땅을 보는 범위 레벨을 넘겨줌
	return m_CameraLevel;
}

bool CDirection::DirectionCameraMove()
{
	if(m_bCameraCheck)	// LHJ - 카메라가 이동 해야 할 경우를 판단
	{
		SetCameraPosition();	// LHJ - 위치 세팅
		m_bStateCheck = false;  // LHJ - 더이상 위치 세팅 안함

		VectorNormalize(m_vResult);	// LHJ - 이동할 벡터를 단위 벡터로 만듬
		vec3_t vTemp;

		VectorScale(m_vResult, m_fCount, vTemp);	// LHJ - 이동할 만큼 벡터를 늘인다 
		VectorAdd(m_v1stPosition, vTemp, m_vCameraPosition); // LHJ - 최종 카메라 위치
		m_fCount += m_fCameraSpeed;	// LHJ - 카메라 이동 속도를 더한다

		if(m_fLength <= VectorLength(vTemp))	// LHJ - 다 이동 했으면 그만 이동 시킴
		{
			VectorCopy(m_v2ndPosition, m_vCameraPosition);
			m_bAction = true;
			m_bCameraCheck = false;
			m_fCount = 0.0f;
			VectorCopy(m_v2ndPosition, m_v1stPosition);
		}
		return true;
	}

	return false;
}

void CDirection::DeleteMonster()
{
	// LHJ - 연출로 만들어진 몬스터 지우기
	int Count = stl_Monster.size();
	
	if(Count == 0)
		return;

	for(int i = 0; i < Count; i++)
	{
		DeleteCharacter(i+NUMOFMON);
		stl_Monster.erase(stl_Monster.begin());
	}
}

float CDirection::CalculateAngle(CHARACTER* c, int x, int y, float Angle)
{
	vec3_t vTemp, vTemp2, vResult;
	float fx = (float)(x*TERRAIN_SCALE) + 0.5f*TERRAIN_SCALE;	// LHJ - 실제 x 좌표로 변환
	float fy = (float)(y*TERRAIN_SCALE) + 0.5f*TERRAIN_SCALE;	// LHJ - 실제 y 좌표로 변환

	Vector(fx, fy, 0.0f, vTemp);	// LHJ - 이동할 위치
	Vector(c->Object.Position[0], c->Object.Position[1], 0.0f, vTemp2);	// LHJ - 현재 몬스터 위치
	
	VectorSubtract(vTemp2, vTemp, vResult);	// LHJ - 이동할 벡터
	Vector(0.0f, 1.0f, 0.0f, vTemp2);

	VectorNormalize(vResult);	// LHJ - 단위벡터로 만듬

	float fAngle = acos(DotProduct(vResult, vTemp2))/3.14159*180;	// LHJ - 현재 위치와 이동할 위치의 각도를 내적으로 계산

	if(vResult[0] > 0) fAngle = 360 - fAngle;	// LHJ - 180도를 안 넘어가는것에 대한 예외 처리

	// LHJ - 계산된 각도를 넘긴다
	return fAngle;
}

void CDirection::SummonCreateMonster(int Type, int x, int y, float Angle, bool NextCheck, bool SummonAni, float AniSpeed)
{
	CHARACTER* c = NULL;
	DirectionMonster DMonster = {0,	};

	DMonster.m_Index = stl_Monster.size();
	DMonster.m_bAngleCheck	= true;	// LHJ - 생성된 몹이 방향 틀 수 있도록 초기화
	DMonster.m_iActionCheck = 0;	// LHJ - 생성된 몹의 동작 초기화

	stl_Monster.push_back(DMonster);
	
	c = CreateMonster(Type,x,y,DMonster.m_Index+NUMOFMON);	// LHJ - 몬스터 생성
	c->Key = NUMOFMON + DMonster.m_Index++;
	c->Object.Angle[2] = Angle;	// LHJ - 처음 생성시 넘어온 각도로 몬스터를 튼다
	c->Weapon[0].Type = -1;
	c->Weapon[1].Type = -1;
	c->Object.Alpha = 0.0f;	// LHJ - 0.0f 이면 몹 서서히 보이기 1.0f면 바로 보이기

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	int Index = 0;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	int Index;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	
	switch(Type)
	{
	// LHJ - 이곳에 애니메이션 속도 조절을 위하여 연출로 생성된 몬스터의 인덱스를 추가한다
	case 344: Index = MODEL_MONSTER01+91;	break;	// LHJ - 발람
	case 341: Index = MODEL_MONSTER01+94;	break;	// LHJ - 소람
	case 440:
  	case 340: Index = MODEL_MONSTER01+92;	break;	// LHJ - 다크엘프
	case 345: Index = MODEL_MONSTER01+93;	break;	// LHJ - 데스스피릿
	case 348: Index = MODEL_MONSTER01+99;	break;	// LHJ - 탱커
	case 349: Index = MODEL_MONSTER01+89;	break;	// LHJ - 발가스
	case 361: Index = MODEL_MONSTER01+121;	break;	// LHJ - 나이트 메어
	}
	
  	BMD *b = &Models[Index];
	
	// LHJ - 생성된 몬스터의 애니메이션 속도 조절하는 부분
	if(AniSpeed >= 0.0f)
		b->Actions[MONSTER01_WALK].PlaySpeed = AniSpeed;

	if(SummonAni)	// LHJ - 넘어온 값에 소환 이펙트가 있으면
	{
		if(Type == 361)	// LHJ - 나이트 메어
		{
			// LHJ - 나이트 메어 소환 될때 회오리 효솨
			vec3_t Light, Angle;

			Vector ( 0.5f, 0.8f, 1.0f, Light );
			Vector(0.0f, 0.0f, 0.0f, Angle);
			CreateEffect(MODEL_STORM2,c->Object.Position, Angle, Light, 1, NULL, -1, 0, 0, 0, 1.6f);
			CreateEffect(MODEL_STORM2,c->Object.Position, Angle, Light, 1, NULL, -1, 0, 0, 0, 1.3f);
			CreateEffect(MODEL_STORM2,c->Object.Position, Angle, Light, 1, NULL, -1, 0, 0, 0, 0.7f);

			PlayBuffer ( SOUND_KANTURU_3RD_MAYA_END );
		}
		else
		{
			PlayBuffer ( SOUND_CRY1ST_SUMMON );

			vec3_t vPos;
			Vector(c->Object.Position[0]+20.0f, c->Object.Position[1]+20.0f, c->Object.Position[2], vPos);
			
			// LHJ - 몬스터 소환 효과 - 하늘에서 번개 떨어지기
			CreateJoint ( BITMAP_JOINT_THUNDER+1, vPos, vPos, c->Object.Angle, 7, NULL, 60.f+rand()%10 );
			CreateJoint ( BITMAP_JOINT_THUNDER+1, vPos, vPos, c->Object.Angle, 7, NULL, 50.f+rand()%10 );
			CreateJoint ( BITMAP_JOINT_THUNDER+1, vPos, vPos, c->Object.Angle, 7, NULL, 50.f+rand()%10 );
			CreateJoint ( BITMAP_JOINT_THUNDER+1, vPos, vPos, c->Object.Angle, 7, NULL, 60.f+rand()%10 );

			// LHJ - 몬스터 소환 효과 - 번개 떨어진 지점에 연기 나기
			CreateParticle ( BITMAP_SMOKE+4, c->Object.Position, c->Object.Angle, c->Object.Light, 1, 5.0f);
			CreateParticle ( BITMAP_SMOKE+4, c->Object.Position, c->Object.Angle, c->Object.Light, 1, 5.0f);
			CreateParticle ( BITMAP_SMOKE+4, c->Object.Position, c->Object.Angle, c->Object.Light, 1, 5.0f);

			// LHJ - 몬스터 소환 효과 - 몹 나타 나면서 주위에 번개 효과
			Vector(c->Object.Position[0], c->Object.Position[1], c->Object.Position[2]+120.0f, vPos);
			CreateJoint(BITMAP_JOINT_THUNDER, c->Object.Position, vPos, c->Object.Angle, 17);
			CreateJoint(BITMAP_JOINT_THUNDER, c->Object.Position, vPos, c->Object.Angle, 17);
			CreateJoint(BITMAP_JOINT_THUNDER, c->Object.Position, vPos, c->Object.Angle, 17);
			CreateJoint(BITMAP_JOINT_THUNDER, c->Object.Position, vPos, c->Object.Angle, 17);
		}
		
	}
	
	if(NextCheck) m_iCheckTime++;	// LHJ - NextCheck이 True면 다음 연출 스케줄로 넘김
}

bool CDirection::MoveCreatedMonster(int Index, int x, int y, float Angle, int Speed)
{
	CHARACTER *c = NULL;
	bool bNext = false;	// LHJ - 다음 연출로 넘어가는지 체크

	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)	// LHJ - 이동시킬 Index 몬스터 검사
	{
		c = &CharactersClient[i];	

		if(c->Object.Live && c->Key == Index+NUMOFMON)
			break;
	}

	int PresentX = (int)(c->Object.Position[0])/TERRAIN_SCALE;	// LHJ - 타일 X 인덱스
	int PresentY = (int)(c->Object.Position[1])/TERRAIN_SCALE;	// LHJ - 타일 Y 인덱스

	if(PresentX == x && PresentY == y) bNext = true;	// LHJ - 몬스터가 지정된 값에 있으면 이동 완료로 판단
	
	if(!bNext)
	{
		int iResult = 0;

		if(stl_Monster[Index].m_bAngleCheck)
		{
			int	iAngle1 = (int)CalculateAngle(c, x, y, Angle);	// LHJ - 이동할 좌표로의 몬스터 바라보는 방향 각도 계산
			int iAngle2 = (int)c->Object.Angle[2];				// LHJ - 현재 케릭 각도

			if((iAngle1 - Angle) > 180)		// LHJ - 몬스터의 계산된 각도가 180도 보다 크면 반대 방향으로 회전하기 위해 음수로 만든다
				iAngle1 = iAngle1 - 360;
			
			iResult = iAngle1 - iAngle2;	// LHJ - 최종 틀어야할 각도
			c->Blood  = false;				// LHJ - 이 임시 값으로 이동할지 안할지 판단 한다
		}

		if(iResult <= 3 && iResult >= -3)	// LHJ - 각도 다 틀었는지 검사 후 틀었으면 이동함
		{
			c->Blood = true;	// LHJ - 이동함
			stl_Monster[Index].m_bAngleCheck = false;	// LHJ - 각도 계산 그만함
		}

		if(c->Blood)	// LHJ - 이동
		{
			c->MoveSpeed = Speed;	// LHJ - 이동 속도
			SetAction(&c->Object,MONSTER01_WALK);
			MoveCharacterPosition(c);
		}
		else			// LHJ - 방향 틀기
		{
			// LHJ - 몬스터 각도 틀때 방향
			if(iResult > 3 && iResult <= 180)
				c->Object.Angle[2] += 3.0f;
			else								
				c->Object.Angle[2] -= 3.0f;		// LHJ - 180도 초과면 반대 방향으로 튼다

			SetAction(&c->Object,MONSTER01_STOP1);
		}
	}
	else
	{
		c->Object.Position[0] = (float)(x*TERRAIN_SCALE) + 0.5f*TERRAIN_SCALE;	// LHJ - 실제 x 좌표로 변환
		c->Object.Position[1] = (float)(y*TERRAIN_SCALE) + 0.5f*TERRAIN_SCALE;	// LHJ - 실제 y 좌표로 변환

		stl_Monster[Index].m_bAngleCheck = true;	// LHJ - 다음에 각도 틀수 있도록 초기화
		SetAction(&c->Object,MONSTER01_STOP1);		// LHJ - 몬스터 행동 초기화
		return true;	// LHJ - 다음 행동으로 넘어감
	}

	return false;
}

bool CDirection::ActionCreatedMonster(int Index, int Action, int Count, bool TankerAttack, bool NextCheck)
{
	CHARACTER *c = NULL;
	bool bNext = false;	// LHJ - 다음 연출로 넘어가는지 체크

	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)	// LHJ - 지정된 애니메이션 시킬 Index 몬스터 검사
	{
		c = &CharactersClient[i];	

		if(c->Object.Live && c->Key == Index+NUMOFMON) break;
	}
	
	// LHJ - 지정된 애니메이션 반복 횟수를 채웠는지 확인
	if( stl_Monster[Index].m_iActionCheck == Count) bNext = true;

	if(!bNext)	// LHJ - 애니메이션 시킴
	{
		if(c->Object.CurrentAction != Action)	// LHJ - 현재 동작과 명령된 동작이 서로 다르면
		{
			// LHJ - 명령된 동작으로 지정
			c->Object.CurrentAction = Action;
			c->Object.AnimationFrame = 0.0f;	
			stl_Monster[Index].m_iActionCheck++;	// LHJ - 몬스터의 애니메이션 반복 횟수

			// LHJ - 만약 크라이울프의 탱커 몬스터의 경우면 이펙트 효과 뿌려줌
			if(TankerAttack)
				CreateEffect(MODEL_ARROW_TANKER,c->Object.Position,c->Object.Angle,c->Object.Light,1,&c->Object,c->Object.PKKey);
		}
	}
	else
	{
		if(NextCheck) 	// LHJ - 다음 연출 시케줄로 넘김
		{
			stl_Monster[Index].m_iActionCheck = 0;
			m_iCheckTime++;
		}

		// LHJ - 명령된 애니메이션의 동작이 거의 끝날 시점
		if(c->Object.AnimationFrame >= 8.0f) return true;
	}

	return false;
}

void CDirection::HeroFallingDownDirection()
{
	if(!m_bDownHero)	// LHJ - 케릭터가 떨어지는 연출이 아니면 리턴 시킴
		return;

	Hero->Object.Gravity		+= 1.5f;		// LHJ - 떨어지는 정도를 값과 같이 증가 시켜줌(가속도)
	Hero->Object.Angle[0]		-= 2.f;			// LHJ - 떨어질때 케릭터가 뒤로 넘어가도록 회전
	Hero->Object.m_bActionStart	 = true;		// LHJ - 
	Hero->Object.Direction[1]	+= Hero->Object.Direction[0];

	if(Hero->Object.Gravity <= 2.f)				// LHJ - 떨어지기 시작하면 케릭터 Y축 각도 고정
		m_AngleY = Hero->Object.Angle[2];

	if(Hero->Object.Angle[0] <= -90.0f)			// LHJ - 케릭터가 90도로 완전히 누우면 더이상 회전 안시킴
		Hero->Object.Angle[0] = -90.0f;
	
	Hero->Object.Angle[2] = m_AngleY;			// LHJ - Y축 고정 각도로 계속 갱신해줌

	if(Hero->Object.Gravity >= 150.0f)			// LHJ - 가속도를 150.0f이상으로 증가 안하도록 함
		Hero->Object.Gravity = 150.0f;

	Hero->Object.Position[2] -= Hero->Object.Gravity;	// LHJ - 케릭터의 높이 값을 가속도 만큼 빼줌

	FaillingEffect();	// LHJ - 케릭터가 떨어지는 연출일때 주위 연기 효과
}

void CDirection::FaillingEffect()
{
	vec3_t Pos, Light;
	float Scale = 1.3f + rand()%10/30.0f;
	Vector(0.05f, 0.05f, 0.08f, Light);

	Pos[0] = Hero->Object.Position[0]+(float)(rand()%20-10)*70.0f;
	Pos[1] = Hero->Object.Position[1]+(float)(rand()%20-10)*70.0f;
	Pos[2] = Hero->Object.Position[2]-rand()%200-500.0f;

	CreateParticle(BITMAP_CLOUD, Pos, Hero->Object.Angle, Light, 13, Scale);

	Pos[0] = Hero->Object.Position[0]+(float)(rand()%20-10)*70.0f;
	Pos[1] = Hero->Object.Position[1]+(float)(rand()%20-10)*70.0f;
	Pos[2] = Hero->Object.Position[2]-rand()%200-500.0f;

	Vector(0.05f, 0.05f, 0.05f, Light);
	CreateParticle(BITMAP_CLOUD, Pos, Hero->Object.Angle, Light, 13, Scale);
}

void CDirection::HeroFallingDownInit()
{
	if(!m_bDownHero)
	{
		Hero->Object.m_bActionStart	= false;
		Hero->Object.Gravity = 0.0f;
		Hero->Object.Angle[0] = 0.0f;
	}
}

void CDirection::CameraLevelUp()
{
	if(m_CameraLevel < 4) m_CameraLevel++;
}

void CDirection::SetNextDirectionPosition(int x, int y, int z, float Speed)
{
	m_iCheckTime = 0;
	m_bCameraCheck =true;
	m_bStateCheck = true;
	m_fCameraSpeed = Speed;		// LHJ - 넘어온 값으로 카메라 이동 속도 지정

	float fx = (float)(x*TERRAIN_SCALE) + 0.5f*TERRAIN_SCALE;	// LHJ - 실제 x 좌표로 변환
	float fy = (float)(y*TERRAIN_SCALE) + 0.5f*TERRAIN_SCALE;	// LHJ - 실제 y 좌표로 변환
	float fz = (float)(z*TERRAIN_SCALE) + 0.5f*TERRAIN_SCALE;	// LHJ - 실제 y 좌표로 변환

	Vector(fx, fy, fz, m_v2ndPosition);	// LHJ - 이동할 지점을 지정
	m_iTimeSchedule++;
}

bool CDirection::GetTimeCheck(int DelayTime)
{
	int PresentTime = timeGetTime();
	
	if(m_bTimeCheck)
	{
		m_iBackupTime = PresentTime;
		m_bTimeCheck = false;
	}

	if(m_iBackupTime+DelayTime <= PresentTime)
	{
		m_bTimeCheck = true;
		m_iCheckTime++;
		return true;
	}
	return false;
}
