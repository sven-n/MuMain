// ItemAddOptioninfo.h: interface for the ItemAddOptioninfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITEMADDOPTIONINFO_H__F0F0BF49_D23C_465F_BE31_4F4312C7D280__INCLUDED_)
#define AFX_ITEMADDOPTIONINFO_H__F0F0BF49_D23C_465F_BE31_4F4312C7D280__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct _ITEM_ADD_OPTION
{
	BYTE	m_byOption1;	// 옵션 첫번째 타입(음수 존재)
	WORD	m_byValue1;	    // 옵션 첫번째 값

	BYTE	m_byOption2;	// 옵션 두번째 타입(음수 존재)
	WORD	m_byValue2;	    // 옵션 두번째 값

	BYTE	m_Type;
	DWORD	m_Time;         // 버프 아이템만 필요함.

	_ITEM_ADD_OPTION()
	{
		m_byOption1	=	0;
		m_byValue1	=	0;

		m_byOption2	=	0;
		m_byValue2	=	0;

		m_Type      =   0;
		m_Time		=   0;
	}

} ITEM_ADD_OPTION;


typedef struct _ITEM_ADD_OPTION_RESULT
{
	int				m_Width;
	int				m_Height;
	int				m_Count;
	int				m_Type;

	_ITEM_ADD_OPTION_RESULT()
	{
		m_Width = 0;
		m_Height = 0;
		m_Count = 0;
		m_Type = 0;
	}
	bool IsSuccess()
	{
		if( m_Width * m_Height == m_Count && m_Count != 0 )
		{
			return true;
		}
		return false;
	}
} ITEM_ADD_OPTION_RESULT;

class ItemAddOptioninfo  
{
public:
	static ItemAddOptioninfo* MakeInfo();
	virtual ~ItemAddOptioninfo();

public:
	void GetItemAddOtioninfoText( std::vector<string>& outtextlist, int type );

public://inline
	const ITEM_ADD_OPTION& GetItemAddOtioninfo( int type );

private:
	const bool OpenItemAddOptionInfoFile( const string& filename );
	ItemAddOptioninfo();
	
private:
	ITEM_ADD_OPTION m_ItemAddOption[MAX_ITEM];
	ITEM_ADD_OPTION m_Temp;

};

inline
const ITEM_ADD_OPTION& ItemAddOptioninfo::GetItemAddOtioninfo( int type )
{
	return m_ItemAddOption[type];
}

#endif // !defined(AFX_ITEMADDOPTIONINFO_H__F0F0BF49_D23C_465F_BE31_4F4312C7D280__INCLUDED_)
