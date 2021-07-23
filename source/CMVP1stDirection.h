#ifndef _MVP1STDIRECTION_H_
#define _MVP1STDIRECTION_H_

//////////////////////////////////////////////////////////////////////////
// 
//  내  용 : 크라이울프 1차 레이드 MVP 연출
//  
//  날  짜 : 2005년 11월 14일
//  
//  작성자 : 이 혁 재
//  
//////////////////////////////////////////////////////////////////////////

#include "Singleton.h"

class CMVP1STDirection
{
private:
	bool	m_bTimerCheck;					// 크라이울프 연출시작전 5초의 시간을 준다

	void IsCryWolfDirectionTimer();			// 크라이울프 연출 시작전 5초의 여유를 줌

	void MoveBeginDirection();				// 크라이울프 시작 연출
		void BeginDirection0();				// 크라이울프 시작시 연출 부분
		void BeginDirection1();
		void BeginDirection2();
		void BeginDirection3();
		void BeginDirection4();
		void BeginDirection5();
public:
	int		m_iCryWolfState;				// 크라이울프 상태

	CMVP1STDirection();
	virtual ~CMVP1STDirection(); 

	void Init();
	void GetCryWolfState(BYTE CryWolfState);	// 크라이울프 상태값 받아옴
	bool IsCryWolfDirection();					// 크라이울프 연출 상태인가 체크
	void CryWolfDirection();					// 크라이울프 상태를 서버로 부터 받아와 시작,끝 연출일때 연출

};

#endif //_MVP1STINTERFACE_H_