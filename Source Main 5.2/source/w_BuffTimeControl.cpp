// w_BuffTimeControl.cpp: implementation of the BuffTimeControl class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzInfomation.h"
#include "UIManager.h"
#include "ItemAddOptioninfo.h"
#include "w_BuffTimeControl.h"

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

eBuffTimeType BuffTimeControl::CheckBuffTimeType( eBuffState bufftype )
{
	if( g_IsBuffClass(bufftype) == eBuffClass_Count )
	{
		return eBuffTime_None;
	}

	BuffInfo bInfo = g_BuffInfo( bufftype );

	return eBuffTimeType(1005 + bInfo.s_BuffEffectType);
}

DWORD BuffTimeControl::GetBuffEventTime( eBuffTimeType bufftimetype )
{
	return 1000;
}

DWORD BuffTimeControl::GetBuffMaxTime( eBuffState bufftype, DWORD curbufftime )
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

void BuffTimeControl::RegisterBuffTime( eBuffState bufftype, DWORD curbufftime )
{
	eBuffTimeType  bufftimetype = CheckBuffTimeType( bufftype );

	curbufftime = GetBuffMaxTime( bufftype, curbufftime );

	if( bufftimetype == eBuffTime_None || curbufftime == -1 ) 
	{
		return;
	}

	if( IsBuffTime( bufftimetype ) ) return;

	BuffTimeInfo  buffinfo;
	buffinfo.s_BuffType      = bufftype;
	buffinfo.s_CurBuffTime   = curbufftime * 1000;
	buffinfo.s_EventBuffTime = GetTickCount();

	m_BuffTimeList.insert(std::make_pair( bufftimetype, buffinfo ) );
	
	::SetTimer(g_hWnd, bufftimetype, 900, NULL);
}

bool BuffTimeControl::UnRegisterBuffTime( eBuffState bufftype )
{
	eBuffTimeType  bufftimetype = CheckBuffTimeType( bufftype );

	BuffTimeInfoMap::iterator iter = m_BuffTimeList.find( bufftimetype );

	if( iter != m_BuffTimeList.end() )
	{
		::KillTimer(g_hWnd, bufftimetype);
		g_ConsoleDebug->Write(MCD_NORMAL,"[Buff End] No. %d\r\n", bufftimetype);

		m_BuffTimeList.erase(iter);
		return true;
	}

	return false;
}

void BuffTimeControl::GetBuffStringTime( eBuffState bufftype, std::string& timeText )
{
	for ( BuffTimeInfoMap::iterator iter = m_BuffTimeList.begin(); iter != m_BuffTimeList.end(); ++iter )
	{
		BuffTimeInfo& bufftimeinfo = (*iter).second;

		if( bufftimeinfo.s_BuffType == bufftype )
		{
			float fTime = bufftimeinfo.s_CurBuffTime * 0.001f;
			GetStringTime( fTime, timeText, true );
		}
	}
}

void BuffTimeControl::GetBuffStringTime( DWORD type, std::string& timeText, bool issecond )
{
	BuffTimeInfoMap::iterator iter = m_BuffTimeList.find( type );

	if( iter != m_BuffTimeList.end() )
	{
		BuffTimeInfo& bufftimeinfo = (*iter).second;
		float fTime = bufftimeinfo.s_CurBuffTime * 0.001f;
		GetStringTime( fTime, timeText, issecond );
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
	return 0;
}

void BuffTimeControl::GetStringTime( DWORD time, std::string& timeText, bool isSecond )
{
	char buffer[100];

	if( isSecond )
	{
		DWORD day     = time/(1440*60);
		DWORD oClock  = (time-(day*(1440*60)))/3600;
		DWORD minutes = (time-((oClock*3600)+(day*(1440*60))))/60;
		DWORD second  = time%60;

		if( day != 0 )
		{
			std::sprintf(buffer, "%d %s %d %s %d %s %d %s", day, GlobalText[2298], oClock, GlobalText[2299],minutes, GlobalText[2300], second, GlobalText[2301]);
			timeText = buffer;
		}
		else if( day == 0 && oClock != 0 )
		{
			std::sprintf(buffer, "%d %s %d %s %d %s", oClock, GlobalText[2299], minutes, GlobalText[2300], second, GlobalText[2301]);
			timeText = buffer;
		}
		else if( day == 0 && oClock == 0 && minutes != 0 )
		{
			std::sprintf(buffer, "%d %s %d %s", minutes, GlobalText[2300], second, GlobalText[2301]);
			timeText = buffer;
		}
		else if( day == 0 && oClock == 0 && minutes == 0 )
		{
			std::sprintf(buffer,"%s",GlobalText[2308]);
			timeText = buffer;
		}
	}
	else
	{
		DWORD day     = time/1440;
		DWORD oClock  = (time-(day*1440))/60;
		DWORD minutes = time%60;

		if( day != 0 )
		{
			std::sprintf(buffer, "%d %s %d %s %d %s", day, GlobalText[2298], oClock, GlobalText[2299], minutes, GlobalText[2300]);
			timeText = buffer;
		}
		else if( day == 0 && oClock != 0 )
		{
			std::sprintf(buffer, "%d %s %d %s", oClock, GlobalText[2299], minutes, GlobalText[2300]);
			timeText = buffer;
		}
		else if( day == 0 && oClock == 0 && minutes != 0 )
		{
			std::sprintf(buffer, "%d %s", minutes, GlobalText[2300]);
			timeText = buffer;
		}
	}
}

bool BuffTimeControl::CheckBuffTime( DWORD type )
{
	BuffTimeInfoMap::iterator iter = m_BuffTimeList.find( type );

	if( iter != m_BuffTimeList.end() )
	{
		BuffTimeInfo& bufftimeinfo = (*iter).second;

		DWORD iCurBufftime = bufftimeinfo.s_CurBuffTime;
		if (iCurBufftime > GetTickCount() - bufftimeinfo.s_EventBuffTime)
		{
			iCurBufftime -= GetTickCount() - bufftimeinfo.s_EventBuffTime;
		}
		else
		{
			iCurBufftime = 0;
		}
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
	}
	return false;
}

bool BuffTimeControl::HandleWindowMessage( UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result )
{
	if( message == WM_TIMER ) 
	{
		if( wParam != eBuffTime_None && wParam >= eBuffTime_Hellowin )
		{
			if(!CheckBuffTime(static_cast<DWORD>(wParam)))
			{
				KillTimer(g_hWnd, static_cast<DWORD>(wParam));
			}
			return true;
		}
	}
	return false;
}
