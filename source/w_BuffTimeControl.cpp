// w_BuffTimeControl.cpp: implementation of the BuffTimeControl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzInfomation.h"
#include "UIManager.h"
#include "ItemAddOptioninfo.h"
#include "w_BuffTimeControl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BuffTimeControlPtr BuffTimeControl::Make()
{
	BuffTimeControlPtr bufftimecontrol( new BuffTimeControl );
	return bufftimecontrol;
}

BuffTimeControl::BuffTimeControl()
{
	
}

BuffTimeControl::~BuffTimeControl()
{
	for( BuffTimeInfoMap::iterator iter = m_BuffTimeList.begin(); iter != m_BuffTimeList.end(); )
	{
		 BuffTimeInfoMap::iterator tempiter = iter;
		 ++iter;

		 eBuffTimeType bufftimetype = static_cast<eBuffTimeType>((*tempiter).first);

		::KillTimer(g_hWnd, bufftimetype);
		m_BuffTimeList.erase(tempiter);
	}

	m_BuffTimeList.clear();
}

// 버프에 타임 콘트롤러가 들어가면 추가를 해줘야 한다...
// -=- 스크립트로 빼자..
// 상승, 풍요인장 같은 겹치지 않는(동시에 효과가 나타나지 않는) 버프들은 eBuffTimeType이 같음.	// rozy
eBuffTimeType BuffTimeControl::CheckBuffTimeType( eBuffState bufftype )
{
#ifdef LDK_MOD_BUFFTIMERTYPE_SCRIPT
	// 버프에 시간적용시 
	//eBuffTimeType에 등록안해도 됨
	//buffEffect.txt의 Viewport값을 가지고 중복판단
	if( g_IsBuffClass(bufftype) == eBuffClass_Count )
	{
		return eBuffTime_None;
	}

	BuffInfo bInfo = g_BuffInfo( bufftype );

	return eBuffTimeType(1005 + bInfo.s_BuffEffectType); // eBuffTimeType의 첫번째 enum값은 1006부터 시작
#else //LDK_MOD_BUFFTIMERTYPE_SCRIPT
	eBuffTimeType  bufftimetype = eBuffTime_None;
	
	if(bufftype >= eBuff_Hellowin1 && bufftype <= eBuff_Hellowin5)
	{
		bufftimetype = eBuffTime_Hellowin;
	}
	else if(bufftype >= eBuff_PcRoomSeal1 && bufftype <= eBuff_PcRoomSeal3)
	{
		bufftimetype = eBuffTime_PcRoomSeal;
	}
	else if(bufftype >= eBuff_Seal1 && bufftype <= eBuff_Seal4)
	{
		bufftimetype = eBuffTime_Seal;
	}
	else if(bufftype >= eBuff_EliteScroll1 && bufftype <= eBuff_EliteScroll6)
	{
		bufftimetype = eBuffTime_Scroll;
	}
	else if(bufftype >= eBuff_SecretPotion1 && bufftype <= eBuff_SecretPotion5)
	{
		bufftimetype = eBuffTime_Secret;
	}
#ifdef CSK_EVENT_CHERRYBLOSSOM
	else if(bufftype >= eBuff_CherryBlossom_Liguor && bufftype <= eBuff_CherryBlossom_Petal) // 벚꽃술, 벚꽃경단, 벚꽃꽃잎
	{
		bufftimetype = eBuffTime_CherryBlossom;
	}
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
	else if( bufftype == eBuff_SwellOfMagicPower )			// 마력증대
	{
		bufftimetype = eBuffTime_SwellOfMP;
	}
#endif //KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
#ifdef PSW_ADD_PC4_SEALITEM
	else if( bufftype == eBuff_Seal_HpRecovery || bufftype == eBuff_Seal_MpRecovery
#ifdef ASG_ADD_CS6_ASCENSION_SEAL_MASTER
		|| bufftype == eBuff_AscensionSealMaster	// 상승의인장마스터
#endif	// ASG_ADD_CS6_ASCENSION_SEAL_MASTER
#ifdef ASG_ADD_CS6_WEALTH_SEAL_MASTER
		|| bufftype == eBuff_WealthSealMaster		// 풍요의인장마스터
#endif	// ASG_ADD_CS6_WEALTH_SEAL_MASTER
		)
	{
		bufftimetype = eBuffTime_Seal;
	}
#endif //PSW_ADD_PC4_SEALITEM
#ifdef PSW_ADD_PC4_SCROLLITEM
	else if( bufftype == eBuff_Scroll_Battle || bufftype == eBuff_Scroll_Strengthen )
	{
		bufftimetype = eBuffTime_Scroll;
	}
#endif //PSW_ADD_PC4_SCROLLITEM
#ifdef PBG_ADD_SANTABUFF
	//pbg 산타관련 버프는 모두 30분.
	else if(bufftype >= eBuff_BlessingOfXmax && bufftype <= eBuff_LuckOfSanta)
	{
		bufftimetype = eBuffTime_Christmax;
	}
#endif //PBG_ADD_SANTABUFF
#ifdef PBG_ADD_HONOROFGLADIATORBUFFTIME
	else if(bufftype == eBuff_HonorOfGladiator)
	{
		bufftimetype = eBuffTime_HonorOfGladiator;
	}
#endif //PBG_ADD_HONOROFGLADIATORBUFFTIME
#ifdef ASG_ADD_CS6_GUARD_CHARM
	else if(bufftype == eBuff_GuardCharm)
	{
		bufftimetype = eBuffTime_GuardCharm;
	}
#endif	// ASG_ADD_CS6_GUARD_CHARM
#ifdef ASG_ADD_CS6_ITEM_GUARD_CHARM
	else if(bufftype == eBuff_ItemGuardCharm)
	{
		bufftimetype = eBuffTime_ItemGuardCharm;
	}
#endif	// ASG_ADD_CS6_ITEM_GUARD_CHARM
#ifdef LDS_ADD_PCROOM_ITEM_JPN_6TH
	else if(bufftype == eBuff_StrongCharm)
	{
		bufftimetype = eBuffTime_ItemStrongCharm;
	}
#endif // LDS_ADD_PCROOM_ITEM_JPN_6TH
#ifdef PBG_ADD_SECRETBUFF
	else if(bufftype >= eBuff_Vitality_Lowest && bufftype <= eBuff_Vitality_High)
	{
		bufftimetype = eBuffTime_Vitality;
	}
#endif //PBG_ADD_SECRETBUFF
#ifdef YDG_ADD_CS7_MAX_AG_AURA
	else if(bufftype == eBuff_AG_Addition)
	{
		bufftimetype = eBuffTime_AG_Addition;
	}
#endif	// YDG_ADD_CS7_MAX_AG_AURA
#ifdef YDG_ADD_CS7_MAX_SD_AURA
	else if(bufftype == eBuff_SD_Addition)
	{
		bufftimetype = eBuffTime_SD_Addition;
	}
#endif	// YDG_ADD_CS7_MAX_SD_AURA
#ifdef YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
	else if(bufftype == eBuff_PartyExpBonus)
	{
		bufftimetype = eBuffTime_PartyExpBonus;
	}
#endif	// YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
#ifdef LDK_FIX_NEWWEALTHSEAL_INFOTIMER_AND_MOVEWINDOW
	else if(bufftype == eBuff_NewWealthSeal)
	{
		bufftimetype = eBuffTime_Seal;
	}
#endif //LDK_FIX_NEWWEALTHSEAL_INFOTIMER_AND_MOVEWINDOW
#ifdef YDG_ADD_HEALING_SCROLL
	else if(bufftype == eBuff_Scroll_Healing)	// 치유의 스크롤
	{
		bufftimetype = eBuffTime_Scroll;
	}
#endif	// YDG_ADD_HEALING_SCROLL

	return bufftimetype;
#endif //LDK_MOD_BUFFTIMERTYPE_SCRIPT
}

DWORD BuffTimeControl::GetBuffEventTime( eBuffTimeType bufftimetype )
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	if(InBounds(bufftimetype, eBuffTime_Hellowin, eBuffTime_Count) == TRUE)
		return 1000;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	// 지금 까지는 거의다 1초에 한번씩 갱신다..
	// 다른 이벤트 타임이 있으면 추가를 해주자.
	return 1000;
}

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
int BuffTimeControl::GetBuffMaxTime( eBuffState bufftype, int curbufftime )
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
DWORD BuffTimeControl::GetBuffMaxTime( eBuffState bufftype, DWORD curbufftime )
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
{
	if( curbufftime == 0 )
	{
		const BuffInfo& buffinfo = g_BuffInfo( bufftype );

		if( buffinfo.s_ItemType == 255 )
		{
			return -1;
		}
		else
		{
			const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEMINDEX( buffinfo.s_ItemType, buffinfo.s_ItemIndex));

			if( Item_data.m_Time == 0 ) return -1;

			return Item_data.m_Time;
		}
	}

	return curbufftime;
}

bool BuffTimeControl::IsBuffTime( eBuffTimeType bufftype )
{
	BuffTimeInfoMap::iterator iter = m_BuffTimeList.find( bufftype );

	if( iter == m_BuffTimeList.end() )
	{
		return false;
	}
	else
	{
		return true;
	}
}

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
void BuffTimeControl::RegisterBuffTime( eBuffState bufftype, int curbufftime )
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
void BuffTimeControl::RegisterBuffTime( eBuffState bufftype, DWORD curbufftime )
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
{
	eBuffTimeType  bufftimetype = CheckBuffTimeType( bufftype );

	curbufftime = GetBuffMaxTime( bufftype, curbufftime );

	if( bufftimetype == eBuffTime_None || curbufftime == -1 ) 
	{
		return;
	}

	if( IsBuffTime( bufftimetype ) ) return;

	//UnRegisterBuffTime( bufftype );
	
#ifdef LDK_FIX_MODIFY_BUFFTIME_CHECK
	BuffTimeInfo  buffinfo;
	buffinfo.s_BuffType      = bufftype;
	buffinfo.s_CurBuffTime   = curbufftime * 1000;	// 정확한 표시를 위해 ms단위로 바꿨음
	buffinfo.s_EventBuffTime = GetTickCount(); // 버프 시작 시간을 설정한다.

	m_BuffTimeList.insert( make_pair( bufftimetype, buffinfo ) );
	
	//1초(1000ms)에 한번씩 이벤트 발생
	::SetTimer(g_hWnd, bufftimetype, 900, NULL);	// 타이머와 미묘한 동기화 때문에 조금 빠르게 설정했음;
#else //LDK_FIX_MODIFY_BUFFTIME_CHECK
	BuffTimeInfo  buffinfo;
	buffinfo.s_BuffType      = bufftype;
	buffinfo.s_CurBuffTime   = curbufftime;
	buffinfo.s_EventBuffTime = GetBuffEventTime(bufftimetype);

	m_BuffTimeList.insert( make_pair( bufftimetype, buffinfo ) );
	
	::SetTimer(g_hWnd, bufftimetype, buffinfo.s_EventBuffTime, NULL);
#endif //LDK_FIX_MODIFY_BUFFTIME_CHECK
}

bool BuffTimeControl::UnRegisterBuffTime( eBuffState bufftype )
{
	eBuffTimeType  bufftimetype = CheckBuffTimeType( bufftype );

	BuffTimeInfoMap::iterator iter = m_BuffTimeList.find( bufftimetype );

	if( iter != m_BuffTimeList.end() )
	{
#ifdef LDK_MOD_BUFF_END_LOG		// 버프 종료시 로그 추가
		::KillTimer(g_hWnd, bufftimetype);
		g_ErrorReport.Write("[Buff End] No. %d - 버프가 종료 되었습니다.  \r\n", bufftimetype);
#else //LDK_MOD_BUFF_END_LOG
		::KillTimer(g_hWnd, bufftimetype);
#endif //LDK_MOD_BUFF_END_LOG
		m_BuffTimeList.erase(iter);
		return true;
	}

	return false;
}

void BuffTimeControl::GetBuffStringTime( eBuffState bufftype, string& timeText )
{
	for ( BuffTimeInfoMap::iterator iter = m_BuffTimeList.begin(); iter != m_BuffTimeList.end(); ++iter )
	{
		BuffTimeInfo& bufftimeinfo = (*iter).second;

		if( bufftimeinfo.s_BuffType == bufftype )
		{
#ifdef LDK_FIX_MODIFY_BUFFTIME_CHECK
			float fTime = bufftimeinfo.s_CurBuffTime * 0.001f;			// 정확한 표시를 위해 ms단위로 바꿨음 -> sec로 변환
			GetStringTime( fTime, timeText, true );
#else	// LDK_FIX_MODIFY_BUFFTIME_CHECK
			GetStringTime( bufftimeinfo.s_CurBuffTime, timeText, true );
#endif	// LDK_FIX_MODIFY_BUFFTIME_CHECK
		}
	}
}

void BuffTimeControl::GetBuffStringTime( DWORD type, string& timeText, bool issecond )
{
	BuffTimeInfoMap::iterator iter = m_BuffTimeList.find( type );

	if( iter != m_BuffTimeList.end() )
	{
		BuffTimeInfo& bufftimeinfo = (*iter).second;
#ifdef LDK_FIX_MODIFY_BUFFTIME_CHECK
		float fTime = bufftimeinfo.s_CurBuffTime * 0.001f;			// 정확한 표시를 위해 ms단위로 바꿨음 -> sec로 변환
		GetStringTime( fTime, timeText, issecond );
#else	// LDK_FIX_MODIFY_BUFFTIME_CHECK
		GetStringTime( bufftimeinfo.s_CurBuffTime, timeText, issecond );
#endif	// LDK_FIX_MODIFY_BUFFTIME_CHECK
	}
}

const DWORD BuffTimeControl::GetBuffTime( DWORD type )
{
	BuffTimeInfoMap::iterator iter = m_BuffTimeList.find( type );

	if( iter != m_BuffTimeList.end() )
	{
		BuffTimeInfo& bufftimeinfo = (*iter).second;

		return bufftimeinfo.s_CurBuffTime;
	}
#ifdef KJH_FIX_WOPS_K22852_ABNORMAL_BUFFTIME
	return 0;
#else // KJH_FIX_WOPS_K22852_ABNORMAL_BUFFTIME
	return -1;
#endif // KJH_FIX_WOPS_K22852_ABNORMAL_BUFFTIME
}

void BuffTimeControl::GetStringTime( DWORD time, string& timeText, bool isSecond )
{
	if( isSecond )
	{
		DWORD day     = time/(1440*60);
		DWORD oClock  = (time-(day*(1440*60)))/3600;
		DWORD minutes = (time-((oClock*3600)+(day*(1440*60))))/60;
		DWORD second  = time%60;

		if( day != 0 )
		{
			timeText = ( format( "%1% %2% %3% %4% %5% %6% %7% %8%" ) 
								% day % GlobalText[2298] 
								% oClock % GlobalText[2299]
								% minutes % GlobalText[2300] 
								% second % GlobalText[2301] ).str();
		}
		else if( day == 0 && oClock != 0 )
		{
			timeText = ( format( "%1% %2% %3% %4% %5% %6%" ) 
								% oClock % GlobalText[2299]
								% minutes % GlobalText[2300]
								% second % GlobalText[2301] ).str();
		}
		else if( day == 0 && oClock == 0 && minutes != 0 )
		{
			timeText = ( format( "%1% %2% %3% %4%" ) 
								% minutes % GlobalText[2300]
								% second % GlobalText[2301] ).str();
		}
		else if( day == 0 && oClock == 0 && minutes == 0 )
		{
			timeText = ( format( "%1%" ) % GlobalText[2308] ).str();			
		}
	}
	else
	{
		DWORD day     = time/1440;
		DWORD oClock  = (time-(day*1440))/60;
		DWORD minutes = time%60;

		if( day != 0 )
		{
			timeText = ( format( "%1% %2% %3% %4% %5% %6%" ) 
								% day % GlobalText[2298] 
								% oClock % GlobalText[2299]
								% minutes % GlobalText[2300] ).str();
		}
		else if( day == 0 && oClock != 0 )
		{
			timeText = ( format( "%1% %2% %3% %4%" ) 
								% oClock % GlobalText[2299]
								% minutes % GlobalText[2300] ).str();
		}
		else if( day == 0 && oClock == 0 && minutes != 0 )
		{
			timeText = ( format( "%1% %2%" ) 
								% minutes
								% GlobalText[2300] ).str();
		}
	}
}

bool BuffTimeControl::CheckBuffTime( DWORD type )
{
	BuffTimeInfoMap::iterator iter = m_BuffTimeList.find( type );

	if( iter != m_BuffTimeList.end() )
	{
		BuffTimeInfo& bufftimeinfo = (*iter).second;

		// bufftimeinfo.s_CurBuffTime은 unsigned값이므로 
		// 0 일때 (-) 해주면 최대값으로 돌아가기 때문에,
		// int형 임시변수로 처리하였다.

#ifdef LDK_FIX_MODIFY_BUFFTIME_CHECK
		DWORD iCurBufftime = bufftimeinfo.s_CurBuffTime;
#ifdef YDG_FIX_BUFFTIME_OVERFLOW
		if (iCurBufftime > GetTickCount() - bufftimeinfo.s_EventBuffTime)
		{
			iCurBufftime -= GetTickCount() - bufftimeinfo.s_EventBuffTime;
		}
		else
		{
			iCurBufftime = 0;
		}
#else	// YDG_FIX_BUFFTIME_OVERFLOW
		iCurBufftime -= GetTickCount() - bufftimeinfo.s_EventBuffTime;
#endif	// YDG_FIX_BUFFTIME_OVERFLOW
		bufftimeinfo.s_EventBuffTime = GetTickCount();

		if (iCurBufftime <= 0)
		{
			bufftimeinfo.s_CurBuffTime = 0;
			return false;
		}
		else
		{
			bufftimeinfo.s_CurBuffTime = iCurBufftime;
			return true;
		}
#else //LDK_FIX_MODIFY_BUFFTIME_CHECK

#ifdef KJH_FIX_WOPS_K22852_ABNORMAL_BUFFTIME
		int iCurBufftime = bufftimeinfo.s_CurBuffTime;
		iCurBufftime -= (bufftimeinfo.s_EventBuffTime/1000); //1초

		if (iCurBufftime <= 0)
		{
			bufftimeinfo.s_CurBuffTime = 0;
			return false;
		}
		else
		{
			bufftimeinfo.s_CurBuffTime = iCurBufftime;
			return true;
		}
#else // KJH_FIX_WOPS_K22852_ABNORMAL_BUFFTIME
		bufftimeinfo.s_CurBuffTime -= (bufftimeinfo.s_EventBuffTime/1000); //1초

		if (bufftimeinfo.s_CurBuffTime < 0)
		{
			bufftimeinfo.s_CurBuffTime = 0;
			return false;
		}
		return true;
#endif // KJH_FIX_WOPS_K22852_ABNORMAL_BUFFTIME

#endif //LDK_FIX_MODIFY_BUFFTIME_CHECK
	}
	return false;
}

bool BuffTimeControl::HandleWindowMessage( UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result )
{
	if( message == WM_TIMER ) {

#ifdef LDK_MOD_BUFFTIMERTYPE_SCRIPT
		if( wParam != eBuffTime_None && wParam >= eBuffTime_Hellowin )
		{
			if(!CheckBuffTime(static_cast<DWORD>(wParam)))
			{
#ifdef LDK_MOD_BUFF_END_LOG		// 버프 종료시 로그 추가
				KillTimer(g_hWnd, static_cast<DWORD>(wParam));
				g_ErrorReport.Write("[Timer End] No. %d - 버프가 종료 되었습니다. \r\n", static_cast<DWORD>(wParam));
#else //LDK_MOD_BUFF_END_LOG
				KillTimer(g_hWnd, static_cast<DWORD>(wParam));
#endif //LDK_MOD_BUFF_END_LOG
			}
			return true;

#else //LDK_MOD_BUFFTIMERTYPE_SCRIPT
		if( wParam >= eBuffTime_Hellowin && wParam < eBuffTime_Count ) {
			if(!CheckBuffTime(static_cast<DWORD>(wParam))) {
				KillTimer(g_hWnd, static_cast<DWORD>(wParam));
			}
			return true;
#endif //LDK_MOD_BUFFTIMERTYPE_SCRIPT
		}
	}

	return false;
}
