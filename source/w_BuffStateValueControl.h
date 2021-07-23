// w_BuffStateValueControl.h: interface for the BuffStateValueControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_W_BUFFSTATEVALUECONTROL_H__E3E0DAB0_313B_471B_B631_5B6F588846AA__INCLUDED_)
#define AFX_W_BUFFSTATEVALUECONTROL_H__E3E0DAB0_313B_471B_B631_5B6F588846AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE
#include "_types.h"
#endif // KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE

BoostSmartPointer( BuffStateValueControl );

class BuffStateValueControl  
{
private:
	struct BuffStateValueInfo 
	{
		DWORD    s_Value1;
		DWORD    s_Value2;
		DWORD    s_Time;

		BuffStateValueInfo() : s_Value1(0), s_Value2(0), s_Time(0){}
	};

public:
	static BuffStateValueControlPtr Make();
	virtual ~BuffStateValueControl();

public:
	const BuffStateValueInfo GetValue( eBuffState bufftype );
	void GetBuffInfoString( list<string>& outstr, eBuffState bufftype );
	void GetBuffValueString( string& outstr, eBuffState bufftype );
	eBuffValueLoadType CheckValue( eBuffState bufftype );

private:
	void SetValue( eBuffState bufftype, BuffStateValueInfo& valueinfo );
	void Initialize();
	void Destroy();
	BuffStateValueControl();

private:
	typedef map<eBuffState, BuffStateValueInfo>  BuffStateValueMap;

private:
	BuffStateValueMap				m_BuffStateValue;
};

#endif // !defined(AFX_W_BUFFSTATEVALUECONTROL_H__E3E0DAB0_313B_471B_B631_5B6F588846AA__INCLUDED_)
