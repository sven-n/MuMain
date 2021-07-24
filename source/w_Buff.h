//////////////////////////////////////////////////////////////////////
// w_Buff.h: interface for the Buff class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_W_BUFF_H__E827A2BA_5AD5_4576_A530_C14C1472F89A__INCLUDED_)
#define AFX_W_BUFF_H__E827A2BA_5AD5_4576_A530_C14C1472F89A__INCLUDED_

#pragma once

#ifdef KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE
#include "_types.h"
#endif // KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE

BoostSmartPointer(Buff); 

//boost::shared_ptr<Buff> BuffPtr;

//class Buff;

//typedef std::tr1::shared_ptr<Buff> BuffPtr;

class Buff  
{
public:
	static BuffPtr Make();
	Buff();
	virtual ~Buff();

public:
	Buff&       operator =  ( const Buff& buff );

public:
	void RegisterBuff( eBuffState buffstate );
	void RegisterBuff( std::list<eBuffState> buffstate );
	void UnRegisterBuff( eBuffState buffstate );
	void UnRegisterBuff( std::list<eBuffState> buffstate );

	bool isBuff();
	bool isBuff( eBuffState buffstate );
	const eBuffState isBuff( std::list<eBuffState> buffstatelist );
	void TokenBuff( eBuffState curbufftype );

public:
	const DWORD GetBuffSize();
	const eBuffState GetBuff( int iterindex );
	const DWORD GetBuffCount( eBuffState buffstate );
	void ClearBuff();
#ifdef KJH_PBG_ADD_INGAMESHOP_SYSTEM
	bool IsEqualBuffType( IN int iBuffType, OUT unicode::t_char* szBuffName );
#endif // KJH_PBG_ADD_INGAMESHOP_SYSTEM

public:
	BuffStateMap			m_Buff;
};

inline
Buff& Buff::operator =  ( const Buff& buff )
{
    m_Buff = buff.m_Buff;
    return *this;
}

#endif // !defined(AFX_W_BUFF_H__E827A2BA_5AD5_4576_A530_C14C1472F89A__INCLUDED_)
