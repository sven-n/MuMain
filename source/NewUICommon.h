// NewUICommon.h: interface for the CNewUICommon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICOMMON_H__0668BCBC_7537_454B_82FD_9D6BBBBDBA84__INCLUDED_)
#define AFX_NEWUICOMMON_H__0668BCBC_7537_454B_82FD_9D6BBBBDBA84__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE
#include "_GlobalFunctions.h"
#endif // KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE

namespace SEASON3B
{
	// 아무 기능없고 텍스트와 OK버튼만 들어간 메세지박스 생성
	bool CreateOkMessageBox(const unicode::t_string& strMsg, DWORD dwColor = 0xffffffff, float fPriority = 3.f);

	// 개인상점 타입 리턴
	int IsPurchaseShop(); 
	#define g_IsPurchaseShop SEASON3B::IsPurchaseShop()

	// 마우스 영역 검사
	bool CheckMouseIn(int x, int y, int width, int height);
	
	// 원본 크기 이미지 그대로 찍을 때 사용
	void RenderImage(GLuint uiImageType, float x, float y, float width, float height);
	// 크기 변경 없고 시작UV 지점만 지정할 경우 사용 
	void RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv);
	void RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv, DWORD color);
	void RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv,float uw, float vh, DWORD color = RGBA(255,255,255,255));
	
	// 숫자 이미지 렌더링 함수
	float RenderNumber(float x, float y, int iNum, float fScale = 1.0f);
	
	// 키입력 처리
	
	//. CW32KeyInput와 연동
	bool IsNone(int iVirtKey);		//. 안눌렸는가?
	bool IsRelease(int iVirtKey);	//. 키를 땐 순간 한프레임만
	bool IsPress(int iVirtKey);		//. 눌린 순간의 첫프레임에만
	bool IsRepeat(int iVirtKey);	//. 누르고 있을 때
	
	class CNewKeyInput //. 심플 버젼
	{
		struct INPUTSTATEINFO 
		{
			BYTE byKeyState;
		} m_pInputInfo[256];

#ifndef ASG_FIX_ACTIVATE_APP_INPUT	// 정리시 삭제.
		void Init();
#endif	// ASG_FIX_ACTIVATE_APP_INPUT

	public:
		enum KEY_STATE 
		{ 
			KEY_NONE=0,			
			KEY_RELEASE,		
			KEY_PRESS,			
			KEY_REPEAT			
		};
		~CNewKeyInput();
		
		static CNewKeyInput* GetInstance();
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
		void Init();
#endif	// ASG_FIX_ACTIVATE_APP_INPUT
		void ScanAsyncKeyState();

		bool IsNone(int iVirtKey);
		bool IsRelease(int iVirtKey);
		bool IsPress(int iVirtKey); 
		bool IsRepeat(int iVirtKey);
		void SetKeyState(int iVirtKey, KEY_STATE KeyState);
		

	protected:
		CNewKeyInput();			//. ban to create a instance
	};
}

#define g_pNewKeyInput	SEASON3B::CNewKeyInput::GetInstance()

#endif // !defined(AFX_NEWUICOMMON_H__0668BCBC_7537_454B_82FD_9D6BBBBDBA84__INCLUDED_)
