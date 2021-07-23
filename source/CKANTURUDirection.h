#ifndef _KANTURUDIRECTION_H_
#define _KANTURUDIRECTION_H_

//////////////////////////////////////////////////////////////////////////
// 
//  내  용 : 칸투르3차 연출
//  
//  날  짜 : 2006년 7월 3일
//  
//  작성자 : 이 혁 재
//  
//////////////////////////////////////////////////////////////////////////

class CKanturuDirection
{
public:
	int		m_iKanturuState;		// LHJ - 칸투르 상태 값
	int		m_iMayaState;			// LHJ - 칸투르 마야 전투 진행 상태 값
	int		m_iNightmareState;		// LHJ - 칸투르 나이트메어 전투진행 상태 값
	bool	m_bKanturuDirection;	// LHJ - 칸투르 연출 인지
	

	CKanturuDirection();
	virtual ~CKanturuDirection(); 

	void Init();											// LHJ - 칸투르 초기화
	bool IsKanturuDirection();								// LHJ - 칸투르 연출 상태 인지
	bool IsKanturu3rdTimer();								// LHJ - 칸투르 진행시 남시시간 표시 체크
	bool IsMayaScene();										// LHJ - 마야 관련 맵 상태 인지 체크
	void GetKanturuAllState(BYTE State, BYTE DetailState);	// LHJ - 칸투르 전체 상태
	void KanturuAllDirection();								// LHJ - 칸투르 전체 연출
	bool GetMayaExplotion();								// LHJ - 칸투르에서 마야 죽는 동작인지 아닌지 넘겨줌
	void SetMayaExplotion(bool MayaDie);					// LHJ - 칸투르에서 마야 죽는 연출이 끝난건지 상태 값 가져줌
	bool GetMayaAppear();									// LHJ - 칸투르에서 마야 등장 동작인지 아닌지 넘겨줌
	void SetMayaAppear(bool MayaDie);						// LHJ - 칸투르에서 마야 등장 연출이 끝난건지 상태 값 가져줌

private:
	bool	m_bMayaDie;				// LHJ - 칸투르 마야가 죽는 상태
	bool	m_bMayaAppear;			// LHJ - 칸투르 마야가 올라 오는 상태
	bool	m_bDirectionEnd;		// LHJ - 칸투르 연출이 끝난건지 상태 

	void GetKanturuMayaState(BYTE DetailState);				// LHJ - 서버로 부터 칸투루 상태 값을 받는다
	void GetKanturuNightmareState(BYTE DetailState);		// LHJ - 서버로 부터 칸투루 상태 값을 받는다
	
	void KanturuMayaDirection();		// LHJ - 칸투르 마야 연출
		void Move1stDirection();
			void Direction1st0();
			void Direction1st1();
		void Move2ndDirection();
			void Direction2nd0();
			void Direction2nd1();
			void Direction2nd2();

	void KanturuNightmareDirection();	// LHJ - 칸투르 나이트메어 연출
		void Move3rdDirection();
			void Direction3rd0();
			void Direction3rd1();
		void Move4thDirection();
			void Direction4th0();
			void Direction4th1();
};

#endif //_KANTURUDIRECTION_H_