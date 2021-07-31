//////////////////////////////////////////////////////////////////////
// w_Buff.h: interface for the Buff class.
//////////////////////////////////////////////////////////////////////

#pragma once

BoostSmartPointer(Buff); 

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
	bool IsEqualBuffType( IN int iBuffType, OUT unicode::t_char* szBuffName );

public:
	BuffStateMap			m_Buff;
};

inline
Buff& Buff::operator =  ( const Buff& buff )
{
    m_Buff = buff.m_Buff;
    return *this;
}

