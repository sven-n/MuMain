//////////////////////////////////////////////////////////////////////////
//  
//  UIGateKeeper.h
//  
//  내  용 : 문지기 인터페이스
//  
//  날  짜 : 2004년 12월 07일
//  
//  작성자 : 강 병 국
//  
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIGATEKEEPER_H__1EA5EC0A_1A31_4614_98D6_BC982443141B__INCLUDED_)
#define AFX_UIGATEKEEPER_H__1EA5EC0A_1A31_4614_98D6_BC982443141B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum
{
    TOUCH_TYPE_NONE = 0,            //  
    TOUCH_TYPE_PERSON,              //  일반인.
    TOUCH_TYPE_GUILD_STAFF,         //  길드원.
    TOUCH_TYPE_GUILD_MASTER         //  연합(길드)마스터.
};

class CUIGateKeeper  
{
public:
	CUIGateKeeper();
	virtual ~CUIGateKeeper();

protected:
	bool		m_bPublic;              //  연합 길드외 캐릭터 입장 가능 여부.
    BYTE        m_byType;               //  문지기에 접근한 캐릭터의 타입 ( 0:없음, 1:일반인, 2:같은 길드원, 3:연합기드마스터... ).
	int			m_nEntranceFee;         //  입장 요금 ( 서버에서 확정된 )
    int         m_iViewEntranceFee;     //  화면에 바로 갱신된 입장 요금.
    int         m_iAddEntranceFee;      //  입장요금 증감값.
    int         m_iMaxEnteranceFee;     //  설정할수 있는 최대 요금.

public:
	void SetPublic( bool bPublic )			{ m_bPublic = bPublic; }
	void SetEntranceFee( int nEntranceFee )	{ m_nEntranceFee = nEntranceFee; }
    void SetInfo ( BYTE byType, bool bPublic, int iEntranceFee, int iAddEntranceFee, int iMaxEntranceFee )
    {
        m_byType  = byType;
        m_bPublic = bPublic;
        m_nEntranceFee = iEntranceFee;
        m_iViewEntranceFee = m_nEntranceFee;
        m_iAddEntranceFee = iAddEntranceFee;
        m_iMaxEnteranceFee= iMaxEntranceFee;
    }
	BYTE GetType() { return m_byType; }
	BOOL IsPublic() { return m_bPublic; }
	int GetEnteranceFee() { return m_nEntranceFee; }
	int GetViewEnteranceFee() { return m_iViewEntranceFee; }
	int GetAddEnteranceFee() { return m_iAddEntranceFee; }
	int GetMaxEnteranceFee() { return m_iMaxEnteranceFee; }

	void SendPublicSetting();
	void SendEnteranceFee();
	void SendEnter();
	void EnteranceFeeUp();
	void EnteranceFeeDown();
};

#endif // !defined(AFX_UIGATEKEEPER_H__1EA5EC0A_1A31_4614_98D6_BC982443141B__INCLUDED_)
