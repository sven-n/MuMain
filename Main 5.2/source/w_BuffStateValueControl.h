// w_BuffStateValueControl.h: interface for the BuffStateValueControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_W_BUFFSTATEVALUECONTROL_H__E3E0DAB0_313B_471B_B631_5B6F588846AA__INCLUDED_)
#define AFX_W_BUFFSTATEVALUECONTROL_H__E3E0DAB0_313B_471B_B631_5B6F588846AA__INCLUDED_

#pragma once

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
	void GetBuffInfoString(std::list<std::string>& outstr, eBuffState bufftype );
	void GetBuffValueString(std::string& outstr, eBuffState bufftype );
	eBuffValueLoadType CheckValue( eBuffState bufftype );

private:
	void SetValue( eBuffState bufftype, BuffStateValueInfo& valueinfo );
	void Initialize();
	void Destroy();
	BuffStateValueControl();

private:
	typedef std::map<eBuffState, BuffStateValueInfo>  BuffStateValueMap;

private:
	BuffStateValueMap				m_BuffStateValue;
};

#endif // !defined(AFX_W_BUFFSTATEVALUECONTROL_H__E3E0DAB0_313B_471B_B631_5B6F588846AA__INCLUDED_)
