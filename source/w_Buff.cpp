// w_Buff.cpp: implementation of the Buff class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_Buff.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace
{
	//등록후 사용 하세요.
	void GetTokenBufflist( list<eBuffState>& outtokenbufflist, const eBuffState curbufftype )
	{
		if(curbufftype >= eBuff_CastleRegimentDefense && curbufftype <= eBuff_CastleRegimentAttack3)
		{
			outtokenbufflist.push_back( eBuff_CastleRegimentDefense ); outtokenbufflist.push_back( eBuff_CastleRegimentAttack1 ); 
			outtokenbufflist.push_back( eBuff_CastleRegimentAttack2 ); outtokenbufflist.push_back( eBuff_CastleRegimentAttack3 );
		}
		if(curbufftype >= eBuff_CrywolfAltarEnable && curbufftype <= eBuff_CrywolfNPCHide)
		{
			outtokenbufflist.push_back( eBuff_CrywolfAltarEnable ); outtokenbufflist.push_back( eBuff_CrywolfAltarDisable ); 
			outtokenbufflist.push_back( eBuff_CrywolfAltarContracted ); outtokenbufflist.push_back( eBuff_CrywolfAltarAttempt );
			outtokenbufflist.push_back( eBuff_CrywolfAltarOccufied ); outtokenbufflist.push_back( eBuff_CrywolfHeroContracted );
			outtokenbufflist.push_back( eBuff_CrywolfNPCHide );
		}
#ifdef LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
		if( (curbufftype >= eBuff_PcRoomSeal1 && curbufftype <= eBuff_PcRoomSeal3) || curbufftype == eBuff_NewWealthSeal )
		{
			outtokenbufflist.push_back( eBuff_NewWealthSeal );
#else //LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
		if(curbufftype >= eBuff_PcRoomSeal1 && curbufftype <= eBuff_PcRoomSeal3)
		{
			list<eBuffState> tokenbufflist;	//<<-- 필요없는 것인듯??
#endif //LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
			outtokenbufflist.push_back( eBuff_PcRoomSeal1 ); outtokenbufflist.push_back( eBuff_PcRoomSeal2 ); 
			outtokenbufflist.push_back( eBuff_PcRoomSeal3 );
		}
#if defined(ASG_ADD_CS6_ASCENSION_SEAL_MASTER) && defined(ASG_ADD_CS6_WEALTH_SEAL_MASTER)
		// 아래 if문에서 왜 eBuff_Seal_HpRecovery, eBuff_Seal_MpRecovery를 뺏는지 모르겠음.
		if ((curbufftype >= eBuff_Seal1 && curbufftype <= eBuff_Seal4)
			|| curbufftype == eBuff_AscensionSealMaster || curbufftype == eBuff_WealthSealMaster)
		{
			outtokenbufflist.push_back(eBuff_Seal1);
			outtokenbufflist.push_back(eBuff_Seal2); 
			outtokenbufflist.push_back(eBuff_Seal3);
			outtokenbufflist.push_back(eBuff_Seal4);
#ifdef PSW_ADD_PC4_SEALITEM
			outtokenbufflist.push_back(eBuff_Seal_HpRecovery); 
			outtokenbufflist.push_back(eBuff_Seal_MpRecovery);
#endif //PSW_ADD_PC4_SEALITEM
			outtokenbufflist.push_back(eBuff_AscensionSealMaster); 
			outtokenbufflist.push_back(eBuff_WealthSealMaster);
		}
#else	// defined(ASG_ADD_CS6_ASCENSION_SEAL_MASTER) && defined(ASG_ADD_CS6_WEALTH_SEAL_MASTER)
		if(curbufftype >= eBuff_Seal1 && curbufftype <= eBuff_Seal4)
		{
			outtokenbufflist.push_back( eBuff_Seal1 ); outtokenbufflist.push_back( eBuff_Seal2 ); 
			outtokenbufflist.push_back( eBuff_Seal3 ); outtokenbufflist.push_back( eBuff_Seal4 );
#ifdef PSW_ADD_PC4_SEALITEM
			outtokenbufflist.push_back( eBuff_Seal_HpRecovery ); 
			outtokenbufflist.push_back( eBuff_Seal_MpRecovery );
#endif //PSW_ADD_PC4_SEALITEM
		}
#endif	// defined(ASG_ADD_CS6_ASCENSION_SEAL_MASTER) && defined(ASG_ADD_CS6_WEALTH_SEAL_MASTER)
		if(curbufftype >= eBuff_EliteScroll1 && curbufftype <= eBuff_EliteScroll6)
		{
			outtokenbufflist.push_back( eBuff_EliteScroll1 ); outtokenbufflist.push_back( eBuff_EliteScroll2 ); 
			outtokenbufflist.push_back( eBuff_EliteScroll3 ); outtokenbufflist.push_back( eBuff_EliteScroll4 );
			outtokenbufflist.push_back( eBuff_EliteScroll5 ); outtokenbufflist.push_back( eBuff_EliteScroll6 );
#ifdef PSW_ADD_PC4_SCROLLITEM
			outtokenbufflist.push_back( eBuff_Scroll_Battle );
			outtokenbufflist.push_back( eBuff_Scroll_Strengthen );
#endif //PSW_ADD_PC4_SCROLLITEM
		}
		if(curbufftype >= eBuff_SecretPotion1 && curbufftype <= eBuff_SecretPotion5)
		{
			outtokenbufflist.push_back( eBuff_SecretPotion1 ); outtokenbufflist.push_back( eBuff_SecretPotion2 ); 
			outtokenbufflist.push_back( eBuff_SecretPotion3 ); outtokenbufflist.push_back( eBuff_SecretPotion4 );
			outtokenbufflist.push_back( eBuff_SecretPotion5 );
		}
	}
}

BuffPtr Buff::Make()
{
	BuffPtr buff( new Buff() );
	return buff;
}

Buff::Buff()
{
	
}

Buff::~Buff()
{
	ClearBuff();
}

bool Buff::isBuff()
{
	if( m_Buff.size() != 0 ) return true;
	return false;
}

bool Buff::isBuff( eBuffState buffstate )
{
	if( !isBuff() ) return false;
	
	BuffStateMap::iterator iter = m_Buff.find( buffstate );
	
	if( iter != m_Buff.end() )
	{
		return true;
	}
	
	return false;
}

const eBuffState Buff::isBuff( list<eBuffState> buffstatelist )
{
	if( !isBuff() ) return eBuffNone;

	for ( list<eBuffState>::iterator iter = buffstatelist.begin(); 
	iter != buffstatelist.end(); )
	{
		list<eBuffState>::iterator Tempiter = iter;
		++iter;
		eBuffState tempbufftype = (*Tempiter);

		if( isBuff( tempbufftype ) ) return tempbufftype;
	}

	return eBuffNone;
}

void Buff::TokenBuff( eBuffState curbufftype )
{
	list<eBuffState> tokenbufflist;
	// 토큰될 버프 리스트를 얻어 온다.
	GetTokenBufflist( tokenbufflist, curbufftype );
	// 리스트에 담겨 있는 버프 리스트를 지운다.
	UnRegisterBuff( tokenbufflist );
	// 바뀔 버프를 등록 한다.
	RegisterBuff( curbufftype );
}

const DWORD Buff::GetBuffCount( eBuffState buffstate )
{
	DWORD tempcount = 0;
	
	if( !isBuff() ) return tempcount;
	
	BuffStateMap::iterator iter = m_Buff.find( buffstate );
	
	if( iter != m_Buff.end() )
	{
		tempcount = (*iter).second;
		return tempcount;
	}
	
	return tempcount;
}

const DWORD Buff::GetBuffSize()
{
	return m_Buff.size();
}

const eBuffState Buff::GetBuff( int iterindex )
{
#ifdef _VS2008PORTING
	if( iterindex >= (int)GetBuffSize() ) return eBuffNone;
#else // _VS2008PORTING
	if( iterindex >= GetBuffSize() ) return eBuffNone;
#endif // _VS2008PORTING
	
	int i = 0;
	
	for ( BuffStateMap::iterator iter = m_Buff.begin(); iter != m_Buff.end(); )
	{
		BuffStateMap::iterator tempiter = iter;
		++iter;
		
		if( i == iterindex ) 
		{
			return (*tempiter).first;
		}
		
		i += 1; 
	}
	
	return eBuffNone;
}

#ifdef KJH_PBG_ADD_INGAMESHOP_SYSTEM
bool Buff::IsEqualBuffType(IN int iBuffType, OUT unicode::t_char* szBuffName)
{
	BuffStateMap::iterator iter = m_Buff.begin();
	BuffInfo buffinfo;

	while(iter != m_Buff.end())
	{
		buffinfo = TheBuffInfo().GetBuffinfo(iter->first);
		if( buffinfo.s_BuffEffectType == iBuffType )
		{
			strcpy(szBuffName, buffinfo.s_BuffName);
			return true;
		}

		iter++;
	}

	return false;
}
#endif // KJH_PBG_ADD_INGAMESHOP_SYSTEM

// 버프 등록
void Buff::RegisterBuff( eBuffState buffstate )
{
	BuffStateMap::iterator iter = m_Buff.find( buffstate );
	
	if( iter == m_Buff.end() )
	{
		m_Buff.insert( make_pair( buffstate, 1 ) );
	}
}

void Buff::RegisterBuff( list<eBuffState> buffstate )
{
	for( list<eBuffState>::iterator iter = buffstate.begin(); iter != buffstate.end(); )
	{
		list<eBuffState>::iterator tempiter = iter;
		++iter;
		eBuffState& tempdata = (*tempiter);
		
		RegisterBuff( tempdata );
	}
}

// 버프 헤제
void Buff::UnRegisterBuff( eBuffState buffstate )
{
	if( !isBuff() )
	{
		// 예외처리를 해줘야 한다.
		// 없는 버프를 서버에서 지우라고 했을 경우
		// 클라이언트에서 버프를 절대 지워서는 안된다.
		//assert(0);
		return;
	}

	BuffStateMap::iterator iter = m_Buff.find( buffstate );
	
	if( iter != m_Buff.end() )
	{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
		DWORD& tempcount = (*iter).second;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
		{
			m_Buff.erase(iter);
		}
	}
	else
	{
		// 예외처리를 해줘야 한다.
		// 없는 버프를 서버에서 지우라고 했을 경우
		// 클라이언트에서 버프를 절대 지워서는 안된다.
		//assert(0);
		return;
	}
}

void Buff::UnRegisterBuff( list<eBuffState> buffstate )
{
	for( list<eBuffState>::iterator iter = buffstate.begin(); iter != buffstate.end(); )
	{
		list<eBuffState>::iterator tempiter = iter;
		++iter;
		eBuffState& tempdata = (*tempiter);
		
		UnRegisterBuff( tempdata );
	}
}

void Buff::ClearBuff()
{
	m_Buff.clear();
}
