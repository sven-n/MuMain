// Local.cpp: implementation of the Local
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Local.h"

#ifdef KJH_MOD_NATION_LANGUAGE_REDEFINE

char	lpszFindHackFiles[MAX_TEXT_LENGTH];
char	LanguageName[MAX_TEXT_LENGTH];
char	lpszLocaleName[MAX_TEXT_LENGTH];
DWORD	g_dwCharSet;
int		iLengthAuthorityCode;
char	lpszServerIPAddresses[MAX_TEXT_LENGTH];

//----------------------------------------------------------------------------- Kor
// Kor
#ifdef _LANGUAGE_KOR												
	strncpy(lpszFindHackFiles, "findhack.exe", MAX_TEXT_LENGTH);
	strncpy(LanguageName, "Kor", MAX_TEXT_LENGTH);
	strncpy(lpszLocaleName, "korean", MAX_TEXT_LENGTH);
	g_dwCharSet	= HANGUL_CHARSET;
	iLengthAuthorityCode = 7;
#ifdef PJH_USER_VERSION_SERVER_LIST
	strncpy(lpszServerIPAddresses, "221.148.39.228", MAX_TEXT_LENGTH);		// 체험서버
#else // PJH_USER_VERSION_SERVER_LIST
#ifdef _BLUE_SERVER
	strncpy(lpszServerIPAddresses, "blueconnect.muonline.co.kr", MAX_TEXT_LENGTH);		// 한국어(블루) "202.31.176.161"
#else // _BLUE_SERVER
	strncpy(lpszServerIPAddresses, "connect.muonline.co.kr", MAX_TEXT_LENGTH);		    // 한국어(오리지날)
#endif // _BLUE_SERVER
#endif //PJH_USER_VERSION_SERVER_LIST

	bool CheckSpecialText(char *Text)
	{
		for ( unsigned char *lpszCheck = ( unsigned char*)Text; *lpszCheck; ++lpszCheck)
		{
			if ( 1 == _mbclen( lpszCheck))
			{	// 한 바이트
				//if ( *lpszCheck < 0x21 || *lpszCheck > 0x7E)
				if ( *lpszCheck < 48 || ( 58 <= *lpszCheck && *lpszCheck < 65) ||
					( 91 <= *lpszCheck && *lpszCheck < 97) || *lpszCheck > 122)
				{
					return ( true);
				}
			}
			else
			{	// 두 바이트
				unsigned char *lpszTrail = lpszCheck + 1;
				if ( 0x81 <= *lpszCheck && *lpszCheck <= 0xC8)
				{	// 한글
					if ( ( 0x41 <= *lpszTrail && *lpszTrail <= 0x5A) ||
						( 0x61 <= *lpszTrail && *lpszTrail <= 0x7A) ||
						( 0x81 <= *lpszTrail && *lpszTrail <= 0xFE))
					{	// 투명글자 뺀부분
						// 안되는 특수문자영역들
						if ( 0xA1 <= *lpszCheck && *lpszCheck <= 0xAF && 0xA1 <= *lpszTrail)
						{
							return ( true);
						}
						else if ( *lpszCheck == 0xC6 && 0x53 <= *lpszTrail && *lpszTrail <= 0xA0)
						{
							return ( true);
						}
						else if ( 0xC7 <= *lpszCheck && *lpszCheck <= 0xC8 && *lpszTrail <= 0xA0)
						{
							return ( true);
						}
					}
					else
					{
						return ( true);
					}
				}
				else
				{
					return ( true);
				}
				
				++lpszCheck;
			}
		}
	}

//----------------------------------------------------------------------------- Eng
// Eng
#elif _LANGUAGE_ENG
	strncpy(lpszFindHackFiles, "", MAX_TEXT_LENGTH);
	strncpy(LanguageName, "Eng", MAX_TEXT_LENGTH);
	strncpy(lpszLocaleName, "english", MAX_TEXT_LENGTH);
	g_dwCharSet	= DEFAULT_CHARSET;
#ifdef LDK_MOD_PASSWORD_LENGTH_20
	iLengthAuthorityCode = 20;
#else //LDK_MOD_PASSWORD_LENGTH_20
	iLengthAuthorityCode = 10;
#endif //LDK_MOD_PASSWORD_LENGTH_20
#ifdef LDS_MOD_URL_GLOBAL_TO_DOTCOM
	strncpy(lpszServerIPAddresses, "connect.muonline.webzen.com", MAX_TEXT_LENGTH);		//  영어(글로벌)
#else // LDS_MOD_URL_GLOBAL_TO_DOTCOM
	strncpy(lpszServerIPAddresses, "connect.muonline.webzen.net", MAX_TEXT_LENGTH);		//  영어(글로벌)
#endif // LDS_MOD_URL_GLOBAL_TO_DOTCOM

	bool CheckSpecialText(char *Text)
	{
		for ( unsigned char *lpszCheck = ( unsigned char*)Text; *lpszCheck; ++lpszCheck)
		{
			if ( 1 == _mbclen( lpszCheck))
			{	// 한 바이트
				//사용 가능한 0~9, A~Z, a~z 를 제외한 나머지문자 사용 금지.
				if( !(48 <= *lpszCheck && *lpszCheck < 58) 
					&& !(65 <= *lpszCheck && *lpszCheck < 91) 
					&& !(97 <= *lpszCheck && *lpszCheck < 123) )
				{
					return ( true);
				}
			}
			else
			{	// 두 바이트
				return ( true);
			}
		}
	}

//----------------------------------------------------------------------------- Tai
// Tai
#elif _LANGUAGE_TAI
	strncpy(lpszFindHackFiles, "findhack.exe", MAX_TEXT_LENGTH);
	strncpy(LanguageName, "Tai", MAX_TEXT_LENGTH);
	strncpy(lpszLocaleName, "taiwan", MAX_TEXT_LENGTH);
	g_dwCharSet	= CHINESEBIG5_CHARSET;
#ifdef LDK_FIX_AUTHORITYCODE_LENGTH
	iLengthAuthorityCode = 9;
#else //LDK_FIX_AUTHORITYCODE_LENGTH
	iLengthAuthorityCode = 10;
#endif //LDK_FIX_AUTHORITYCODE_LENGTH
	strncpy(lpszServerIPAddresses, "connection.muonline.com.tw", MAX_TEXT_LENGTH);		//  대만어  "211.20.190.8"

	bool CheckSpecialText(char *Text)
	{
		for ( unsigned char *lpszCheck = ( unsigned char*)Text; *lpszCheck; ++lpszCheck)
		{
			if ( 1 == _mbclen( lpszCheck))
			{	// 한 바이트
				if ( *lpszCheck < 0x21 || *lpszCheck > 0x7E)
				{
					return ( true);
				}
			}
			else
			{	// 두 바이트
				unsigned char *lpszTrail = lpszCheck + 1;
				if ( 0xA4 <= *lpszCheck && *lpszCheck <= 0xF9)
				{	// 한자
					if ( ( 0x40 <= *lpszTrail && *lpszTrail <= 0x7E) ||
						( 0xA1 <= *lpszTrail && *lpszTrail <= 0xFE))
					{	// 한자임
						if ( *lpszCheck == 0xF9 && 0xDD <= *lpszTrail)
						{	// 일부 안되는 특수문자
							return ( true);
						}
					}
					else
					{
						return ( true);
					}
				}
				else
				{
					return ( true);
				}
				
				++lpszCheck;
			}
		}
	}
//----------------------------------------------------------------------------- Chs
// Chs
#elif _LANGUAGE_CHS
	strncpy(lpszFindHackFiles, "", MAX_TEXT_LENGTH);
	strncpy(LanguageName, "Chs", MAX_TEXT_LENGTH);
	strncpy(lpszLocaleName, "chinese", MAX_TEXT_LENGTH);
	g_dwCharSet	= GB2312_CHARSET;
	iLengthAuthorityCode = 7;
	strncpy(lpszServerIPAddresses, "connect.muchina.com", MAX_TEXT_LENGTH);				//  중국어	"61.151.252.74"
	
	bool CheckSpecialText(char *Text)
	{
		for ( unsigned char *lpszCheck = ( unsigned char*)Text; *lpszCheck; ++lpszCheck)
		{
			if ( 1 == _mbclen( lpszCheck))
			{	// 한 바이트
				if ( *lpszCheck < 0x21 || *lpszCheck > 0x7E)
				{
					return ( true);
				}
			}
			else
			{	// 두 바이트
				unsigned char *lpszTrail = lpszCheck + 1;
				if ( 0xB0 <= *lpszCheck && *lpszCheck <= 0xF7)
				{	// 한자
					if ( 0xA1 <= *lpszTrail && *lpszTrail <= 0xFE)
					{	// 한자임
						if ( *lpszCheck == 0xD7 && 0xFA <= *lpszTrail)
						{	// 일부 안되는 특수문자
							return ( true);
						}
					}
					else
					{
						return ( true);
					}
				}
				else
				{
					return ( true);
				}
				
				++lpszCheck;
			}
		}
	}
//----------------------------------------------------------------------------- Jpn
// Jpn
#elif _LANGUAGE_JPN
	strncpy(lpszFindHackFiles, "", MAX_TEXT_LENGTH);
	strncpy(LanguageName, "Jpn", MAX_TEXT_LENGTH);
	strncpy(lpszLocaleName, "jpn", MAX_TEXT_LENGTH);
	g_dwCharSet	= SHIFTJIS_CHARSET;
	iLengthAuthorityCode = 8;
	strncpy(lpszServerIPAddresses, "cs.muonline.jp", MAX_TEXT_LENGTH);					//  일본어  본섭			
	//strncpy(lpszServerIPAddresses, "122.129.233.231", MAX_TEXT_LENGTH);					//	일본어	부분 유료화 공개 테스트

	bool CheckSpecialText(char *Text)
	{
		for ( unsigned char *lpszCheck = ( unsigned char*)Text; *lpszCheck; ++lpszCheck)
		{
			if ( 1 == _mbclen( lpszCheck))
			{	// 한 바이트
				if ( *lpszCheck < 0x21 || ( *lpszCheck > 0x7E && *lpszCheck < 0xA1) ||
					*lpszCheck > 0xDF)
				{
					return ( true);
				}
			}
			else
			{	// 두 바이트
				unsigned char *lpszTrail = lpszCheck + 1;
				if ( ( 0x88 <= *lpszCheck && *lpszCheck <= 0x9F) ||
					( 0xE0 <= *lpszCheck && *lpszCheck <= 0xEA) ||
					( 0xED <= *lpszCheck && *lpszCheck <= 0xEE) ||
					( 0xFA <= *lpszCheck && *lpszCheck <= 0xFC))
				{	// 한자
					if ( 0x40 <= *lpszTrail && *lpszTrail <= 0xFC)
					{	// 사용되는 범위
						if ( *lpszTrail == 0x7F)
						{	// 공백
							return ( true);
						}
						if ( *lpszCheck == 0x88 && *lpszTrail <= 0x9E)
						{	// 공백
							return ( true);
						}
						if ( *lpszCheck == 0x98 && ( 0x73 <= *lpszTrail && *lpszTrail <= 0x9E))
						{	// 공백
							return ( true);
						}
						if ( *lpszCheck == 0xEA && 0xA5 <= *lpszTrail)
						{	// 공백
							return ( true);
						}
						if ( *lpszCheck == 0xEE && 0xED <= *lpszTrail)
						{	// 특수 문자 와 공백
							return ( true);
						}
						if ( *lpszCheck == 0xFC && 0x4C <= *lpszTrail)
						{	// 공백
							return ( true);	
						}
					}
					else
					{
						return ( true);
					}
				}
				else if ( *lpszCheck == 0x81)
				{
					if ( *lpszTrail != 0x58 && *lpszTrail != 0x5B)
					{
						return ( true);
					}
				}
				else if ( *lpszCheck == 0x82)
				{
					if ( *lpszTrail <= 0x4E || ( 0x59 <= *lpszTrail && *lpszTrail <= 0x5F) ||
						( 0x7A <= *lpszTrail && *lpszTrail <= 0x80) ||
						( 0x9B <= *lpszTrail && *lpszTrail <= 0x9E) ||
						( 0xF2 <= *lpszTrail))
					{	// 일부 전각/히라가나를 제외한 공백
						return ( true);
					}
				}
				else if ( *lpszCheck == 0x83)
				{
					if ( *lpszTrail < 0x40 || 0x97 <= *lpszTrail)
					{	// 히라가나 외의 공백 및 특수문자
						return ( true);
					}
				}
				else
				{
					return ( true);
				}
				++lpszCheck;
			}
		}
	}

//----------------------------------------------------------------------------- Tha
// Tha
#elif _LANGUAGE_THA
	strncpy(lpszFindHackFiles, "", MAX_TEXT_LENGTH);
	strncpy(LanguageName, "Tha", MAX_TEXT_LENGTH);
	strncpy(lpszLocaleName, "english", MAX_TEXT_LENGTH);
	g_dwCharSet	= THAI_CHARSET;
	iLengthAuthorityCode = 7;
	strncpy(lpszServerIPAddresses, "cs.muonline.in.th", MAX_TEXT_LENGTH);				//  태국어

	bool CheckSpecialText(char *Text)
	{
		for ( unsigned char *lpszCheck = ( unsigned char*)Text; *lpszCheck; ++lpszCheck)
		{
			if ( 1 == _mbclen( lpszCheck))
			{	// 한 바이트
				//if ( *lpszCheck < 0x21 || *lpszCheck > 0x7E)
				if ( *lpszCheck < 48 || ( 58 <= *lpszCheck && *lpszCheck < 65) ||
					( 91 <= *lpszCheck && *lpszCheck < 97) || *lpszCheck > 122)
				{
					return ( true);
				}
			}
			else
			{	// 두 바이트
				unsigned char *lpszTrail = lpszCheck + 1;
				if ( 0x81 <= *lpszCheck && *lpszCheck <= 0xC8)
				{	// 한글
					if ( ( 0x41 <= *lpszTrail && *lpszTrail <= 0x5A) ||
						( 0x61 <= *lpszTrail && *lpszTrail <= 0x7A) ||
						( 0x81 <= *lpszTrail && *lpszTrail <= 0xFE))
					{	// 투명글자 뺀부분
						// 안되는 특수문자영역들
						if ( 0xA1 <= *lpszCheck && *lpszCheck <= 0xAF && 0xA1 <= *lpszTrail)
						{
							return ( true);
						}
						else if ( *lpszCheck == 0xC6 && 0x53 <= *lpszTrail && *lpszTrail <= 0xA0)
						{
							return ( true);
						}
						else if ( 0xC7 <= *lpszCheck && *lpszCheck <= 0xC8 && *lpszTrail <= 0xA0)
						{
							return ( true);
						}
					}
					else
					{
						return ( true);
					}
				}
				else
				{
					return ( true);
				}
				
				++lpszCheck;
			}
		}
	}

//----------------------------------------------------------------------------- Phi
// Phi
#elif _LANGUAGE_PHI
	strncpy(lpszFindHackFiles, "", MAX_TEXT_LENGTH);
	strncpy(LanguageName, "Phi", MAX_TEXT_LENGTH);
	strncpy(lpszLocaleName, "engligh", MAX_TEXT_LENGTH);
	g_dwCharSet	= DEFAULT_CHARSET;
	iLengthAuthorityCode = 7;
	strncpy(lpszServerIPAddresses, "connect.muonline.com.ph", MAX_TEXT_LENGTH);			//  필리핀

	bool CheckSpecialText(char *Text)
	{
		for ( unsigned char *lpszCheck = ( unsigned char*)Text; *lpszCheck; ++lpszCheck)
		{
			if ( 1 == _mbclen( lpszCheck))
			{	// 한 바이트
				//if ( *lpszCheck < 0x21 || *lpszCheck > 0x7E)
				if ( *lpszCheck < 0x21 || *lpszCheck == 0x27 || *lpszCheck == 0x2C || *lpszCheck == 0x2E ||
					*lpszCheck == 0x60 || 0x7F <= *lpszCheck)
				{
					return ( true);
				}
			}
			else
			{	// 두 바이트
				return ( true);
			}
		}
	}

//----------------------------------------------------------------------------- Vie
// Vie
#elif _LANGUAGE_VIE
	strncpy(lpszFindHackFiles, "", MAX_TEXT_LENGTH);
	strncpy(LanguageName, "Vie", MAX_TEXT_LENGTH);
	strncpy(lpszLocaleName, "engligh", MAX_TEXT_LENGTH);
	g_dwCharSet	= VIETNAMESE_CHARSET;
	iLengthAuthorityCode = 7;
#ifdef LJH_MOD_DOMAIN_NAME_TO_IP_FOR_VIETNAM_VERSION 
	strncpy(lpszServerIPAddresses, "180.148.130.250", MAX_TEXT_LENGTH);					// 베트남 신규 접속서버 교체한 뒤에 기존 도메인으로 접속하지 못해 아이피로 입력
	//strncpy(lpszServerIPAddresses, "210.245.21.245", MAX_TEXT_LENGTH);					// 베트남
	//strncpy(lpszServerIPAddresses, "210.245.21.114", MAX_TEXT_LENGTH);					// 베트남 내부테스트 용
#else  //LJH_MOD_DOMAIN_NAME_TO_IP_FOR_VIETNAM_VERSION 
	strncpy(lpszServerIPAddresses, "connect.muonline.vn", MAX_TEXT_LENGTH);				// 베트남
#endif //LJH_MOD_DOMAIN_NAME_TO_IP_FOR_VIETNAM_VERSION

	bool CheckSpecialText(char *Text)
	{
		for ( unsigned char *lpszCheck = ( unsigned char*)Text; *lpszCheck; ++lpszCheck)
		{
			if ( 1 == _mbclen( lpszCheck))
			{	// 한 바이트
				//if ( *lpszCheck < 0x21 || *lpszCheck > 0x7E)
				if ( *lpszCheck < 0x21 || *lpszCheck == 0x27 || *lpszCheck == 0x2C || *lpszCheck == 0x2E ||
					*lpszCheck == 0x60 || 0x7F <= *lpszCheck)
				{
					return ( true);
				}
			}
			else
			{	// 두 바이트
				return ( true);
			}
		}
	}

#endif
//-----------------------------------------------------------------------------
#else // KJH_MOD_NATION_LANGUAGE_REDEFINE
#if SELECTED_LANGUAGE == LANGUAGE_KOREAN

	char *lpszFindHackFiles[NUM_LANGUAGE] = { "findhack.exe", "", "findhack.exe", "", "", ""};
	char *LanguageName[NUM_LANGUAGE] = { "Kor", "Eng", "Tai", "Chs", "Jpn", "Tha", "Phi" };
	char *lpszLocaleName[NUM_LANGUAGE] = { "korean", "english", "taiwan", "chinese", "jpn", "english", "engligh" };
	DWORD g_dwCharSet[NUM_LANGUAGE] = { HANGUL_CHARSET, DEFAULT_CHARSET, CHINESEBIG5_CHARSET, GB2312_CHARSET, SHIFTJIS_CHARSET, THAI_CHARSET, DEFAULT_CHARSET };
	int iLengthAuthorityCode[NUM_LANGUAGE] =
	{
			7,	// 한국
#ifdef LDK_MOD_PASSWORD_LENGTH_20
			20,	// 영문
#else //LDK_MOD_PASSWORD_LENGTH_20
			10,	// 영문
#endif //LDK_MOD_PASSWORD_LENGTH_20
#ifdef LDK_FIX_AUTHORITYCODE_LENGTH
			9,	// 대만
			7,	// 중국
#else
			10,	// 대만
			 7,	// 중국
#endif //LDK_FIX_AUTHORITYCODE_LENGTH
			8,	// 일본
			7,	// 태국
			7,  // 필리핀
	};

	//!! 접속 서버 IP주소( DNS, 고정 일 경우도 있음.)
	char lpszServerIPAddresses[NUM_LANGUAGE][50] =
	{
#ifdef PJH_USER_VERSION_SERVER_LIST
		"221.148.39.228",				//  한국어
#else
			//박종훈
#ifdef _BLUE_SERVER
		"blueconnect.muonline.co.kr",	// 한국어(블루) "202.31.176.161"
#else // _BLUE_SERVER
		"connect.muonline.co.kr",	    // 한국어(오리지날)
#endif // _BLUE_SERVER
#endif //PJH_USER_VERSION_SERVER_LIST
#ifdef LDS_MOD_URL_GLOBAL_TO_DOTCOM
		"connect.muonline.webzen.com",	        //  영어(글로벌)
#else // LDS_MOD_URL_GLOBAL_TO_DOTCOM
		"connect.muonline.webzen.net",	        //  영어(글로벌)
#endif // LDS_MOD_URL_GLOBAL_TO_DOTCOM
		"connection.muonline.com.tw",   //  대만어  "211.20.190.8",				
		"connect.muchina.com",		    //  중국어	"61.151.252.74"
	#ifdef LEM_ADD_GAMECHU
		"218.234.76.52",
	#else // LEM_ADD_GAMECHU
		//"122.129.233.231",
		"cs.muonline.jp",	            //  일본어
	#endif // LEM_ADD_GAMECHU
		"cs.muonline.in.th",	        //  태국어
		"connect.muonline.com.ph"       //  필리핀.
	};

#else //SELECTED_LANGUAGE == LANGUAGE_KOREAN

	char *lpszFindHackFiles[NUM_LANGUAGE] = { "findhack.exe", "", "findhack.exe", "", "", "", "", ""};
	char *LanguageName[NUM_LANGUAGE] = { "Kor", "Eng", "Tai", "Chs", "Jpn", "Tha", "Phi", "Vie"};
	char *lpszLocaleName[NUM_LANGUAGE] = { "korean", "english", "taiwan", "chinese", "jpn", "english", "engligh", "engligh"};
	DWORD g_dwCharSet[NUM_LANGUAGE] = { HANGUL_CHARSET, DEFAULT_CHARSET, CHINESEBIG5_CHARSET, GB2312_CHARSET, SHIFTJIS_CHARSET, THAI_CHARSET, DEFAULT_CHARSET, VIETNAMESE_CHARSET};
	int iLengthAuthorityCode[NUM_LANGUAGE] =
	{
		7,	// 한국
#ifdef LDK_MOD_PASSWORD_LENGTH_20
			20,	// 영문
#else //LDK_MOD_PASSWORD_LENGTH_20
			10,	// 영문
#endif //LDK_MOD_PASSWORD_LENGTH_20
#ifdef LDK_FIX_AUTHORITYCODE_LENGTH
			9,	// 대만
			7,	// 중국
#else
			10,	// 대만
			7,	// 중국
#endif //LDK_FIX_AUTHORITYCODE_LENGTH
			8,	// 일본
			7,	// 태국
			7,  // 필리핀
			7,  // 베트남
	};
	
	//!! 접속 서버 IP주소( DNS, 고정 일 경우도 있음.) 
	char lpszServerIPAddresses[NUM_LANGUAGE][50] =
	{
#ifdef PJH_USER_VERSION_SERVER_LIST
		"221.148.39.228",				//  한국어
#else
		"connect.muonline.co.kr",	    //  한국어
#endif //PJH_USER_VERSION_SERVER_LIST
#ifdef LDS_MOD_URL_GLOBAL_TO_DOTCOM
			"connect.muonline.webzen.com",   //  영어(글로벌)
#else // LDS_MOD_URL_GLOBAL_TO_DOTCOM
			"connect.muonline.webzen.net",   //  영어(글로벌)
#endif // LDS_MOD_URL_GLOBAL_TO_DOTCOM
			"connection.muonline.com.tw",	//  대만어  "211.20.190.8"
			"connect.muchina.com",			//  중국어	"61.151.252.74"
		#ifdef LEM_ADD_GAMECHU
			"218.234.76.52",
		#else // LEM_ADD_GAMECHU
			//"122.129.233.231",
			"cs.muonline.jp",	            //  일본어
		#endif // LEM_ADD_GAMECHU
			"cs.muonline.in.th",			//  태국어
			"connect.muonline.com.ph",		//  필리핀
#ifdef LJH_MOD_DOMAIN_NAME_TO_IP_FOR_VIETNAM_VERSION 
			"180.148.130.250"				// 베트남 신규 접속서버 교체한 뒤에 기존 도메인으로 접속하지 못해 아이피로 입력
#else  //LJH_MOD_DOMAIN_NAME_TO_IP_FOR_VIETNAM_VERSION 
			"connect.muonline.vn"			// 베트남
#endif //LJH_MOD_DOMAIN_NAME_TO_IP_FOR_VIETNAM_VERSION
			//"210.245.21.245"				//  베트남
			//"210.245.21.114"				// 베트남 내부테스트 용
	};

#endif //SELECTED_LANGUAGE == LANGUAGE_KOREAN

#ifndef KJH_ADD_SERVER_LIST_SYSTEM				// #ifndef
BOOL IsNonPvpServer( int iServerHigh, int iServerLow)
{
#if SELECTED_LANGUAGE == LANGUAGE_KOREAN
	// 한국
    if ( iServerLow == 1 || iServerLow == 2 || iServerLow == 15 )
	{
		if(iServerHigh ==
#if defined PJH_FINAL_VIEW_SERVER_LIST || defined PJH_FINAL_VIEW_SERVER_LIST2 || defined PJH_SEMI_FINAL_VIEW_SERVER_LIST || defined PJH_NEW_SERVER_ADD
#ifdef PJH_NEW_SERVER_ADD
			8	
			|| iServerHigh == 3
#else
			3
#endif //PJH_NEW_SERVER_ADD

#else
			4
#endif //PJH_FINAL_VIEW_SERVER_LIST
			)	//. 레알(신규)서버
		{
			return ( FALSE);
		}

		return ( TRUE);
	}
#endif // SELECTED_LANGUAGE == LANGUAGE_KOREAN
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
	// 일본
    if ( iServerLow<=3)
	{
		return ( TRUE);
	}
#endif // SELECTED_LANGUAGE == LANGUAGE_JAPANESE
#if SELECTED_LANGUAGE == LANGUAGE_TAIWANESE
	// 대만
    if ( iServerLow==2 || iServerLow==3 || iServerLow==4 || iServerLow==5) // 이혁재 - 2005.07.18 대만 요청으로 인한 4,5번 non-pvp 추가
	{
		return ( TRUE);
	}
#endif // SELECTED_LANGUAGE == LANGUAGE_TAIWANESE
#if SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
    if ( iServerLow==1 || iServerLow==4 )
	{
		return ( TRUE);
	}
#endif // SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
#if SELECTED_LANGUAGE == LANGUAGE_ENGLISH 
    if ( iServerLow==1 || iServerLow==2 || iServerLow==3 
#ifdef PBG_MOD_NONPVPSERVER
			|| iServerLow==4
#endif //PBG_MOD_NONPVPSERVER
		)
	{
		return (TRUE);
	}
#endif //SELECTED_LANGUAGE == LANGUAGE_ENGLISH 
	return ( FALSE);
}
#endif // KJH_ADD_SERVER_LIST_SYSTEM


bool CheckSpecialText(char *Text)
{
#if SELECTED_LANGUAGE == LANGUAGE_KOREAN	// 한글판만 체크한다.
	for ( unsigned char *lpszCheck = ( unsigned char*)Text; *lpszCheck; ++lpszCheck)
	{
		if ( 1 == _mbclen( lpszCheck))
		{	// 한 바이트
			//if ( *lpszCheck < 0x21 || *lpszCheck > 0x7E)
			if ( *lpszCheck < 48 || ( 58 <= *lpszCheck && *lpszCheck < 65) ||
				( 91 <= *lpszCheck && *lpszCheck < 97) || *lpszCheck > 122)
			{
				return ( true);
			}
		}
		else
		{	// 두 바이트
			unsigned char *lpszTrail = lpszCheck + 1;
			if ( 0x81 <= *lpszCheck && *lpszCheck <= 0xC8)
			{	// 한글
				if ( ( 0x41 <= *lpszTrail && *lpszTrail <= 0x5A) ||
					( 0x61 <= *lpszTrail && *lpszTrail <= 0x7A) ||
					( 0x81 <= *lpszTrail && *lpszTrail <= 0xFE))
				{	// 투명글자 뺀부분
					// 안되는 특수문자영역들
					if ( 0xA1 <= *lpszCheck && *lpszCheck <= 0xAF && 0xA1 <= *lpszTrail)
					{
						return ( true);
					}
					else if ( *lpszCheck == 0xC6 && 0x53 <= *lpszTrail && *lpszTrail <= 0xA0)
					{
						return ( true);
					}
					else if ( 0xC7 <= *lpszCheck && *lpszCheck <= 0xC8 && *lpszTrail <= 0xA0)
					{
						return ( true);
					}
				}
				else
				{
					return ( true);
				}
			}
			else
			{
				return ( true);
			}

			++lpszCheck;
		}
	}
#elif SELECTED_LANGUAGE == LANGUAGE_TAIWANESE	// 대만판만 체크한다.
	for ( unsigned char *lpszCheck = ( unsigned char*)Text; *lpszCheck; ++lpszCheck)
	{
		if ( 1 == _mbclen( lpszCheck))
		{	// 한 바이트
			if ( *lpszCheck < 0x21 || *lpszCheck > 0x7E)
			{
				return ( true);
			}
		}
		else
		{	// 두 바이트
			unsigned char *lpszTrail = lpszCheck + 1;
			if ( 0xA4 <= *lpszCheck && *lpszCheck <= 0xF9)
			{	// 한자
				if ( ( 0x40 <= *lpszTrail && *lpszTrail <= 0x7E) ||
					( 0xA1 <= *lpszTrail && *lpszTrail <= 0xFE))
				{	// 한자임
					if ( *lpszCheck == 0xF9 && 0xDD <= *lpszTrail)
					{	// 일부 안되는 특수문자
						return ( true);
					}
				}
				else
				{
					return ( true);
				}
			}
			else
			{
				return ( true);
			}

			++lpszCheck;
		}
	}
#elif SELECTED_LANGUAGE == LANGUAGE_CHINESE	// 중국판만 체크한다.
	for ( unsigned char *lpszCheck = ( unsigned char*)Text; *lpszCheck; ++lpszCheck)
	{
		if ( 1 == _mbclen( lpszCheck))
		{	// 한 바이트
			if ( *lpszCheck < 0x21 || *lpszCheck > 0x7E)
			{
				return ( true);
			}
		}
		else
		{	// 두 바이트
			unsigned char *lpszTrail = lpszCheck + 1;
			if ( 0xB0 <= *lpszCheck && *lpszCheck <= 0xF7)
			{	// 한자
				if ( 0xA1 <= *lpszTrail && *lpszTrail <= 0xFE)
				{	// 한자임
					if ( *lpszCheck == 0xD7 && 0xFA <= *lpszTrail)
					{	// 일부 안되는 특수문자
						return ( true);
					}
				}
				else
				{
					return ( true);
				}
			}
			else
			{
				return ( true);
			}

			++lpszCheck;
		}
	}
#elif SELECTED_LANGUAGE == LANGUAGE_JAPANESE	// 일본판만 체크한다.
	for ( unsigned char *lpszCheck = ( unsigned char*)Text; *lpszCheck; ++lpszCheck)
	{
		if ( 1 == _mbclen( lpszCheck))
		{	// 한 바이트
			if ( *lpszCheck < 0x21 || ( *lpszCheck > 0x7E && *lpszCheck < 0xA1) ||
				*lpszCheck > 0xDF)
			{
				return ( true);
			}
		}
		else
		{	// 두 바이트
			unsigned char *lpszTrail = lpszCheck + 1;
			if ( ( 0x88 <= *lpszCheck && *lpszCheck <= 0x9F) ||
				( 0xE0 <= *lpszCheck && *lpszCheck <= 0xEA) ||
				( 0xED <= *lpszCheck && *lpszCheck <= 0xEE) ||
				( 0xFA <= *lpszCheck && *lpszCheck <= 0xFC))
			{	// 한자
				if ( 0x40 <= *lpszTrail && *lpszTrail <= 0xFC)
				{	// 사용되는 범위
					if ( *lpszTrail == 0x7F)
					{	// 공백
						return ( true);
					}
					if ( *lpszCheck == 0x88 && *lpszTrail <= 0x9E)
					{	// 공백
						return ( true);
					}
					if ( *lpszCheck == 0x98 && ( 0x73 <= *lpszTrail && *lpszTrail <= 0x9E))
					{	// 공백
						return ( true);
					}
					if ( *lpszCheck == 0xEA && 0xA5 <= *lpszTrail)
					{	// 공백
						return ( true);
					}
					if ( *lpszCheck == 0xEE && 0xED <= *lpszTrail)
					{	// 특수 문자 와 공백
						return ( true);
					}
					if ( *lpszCheck == 0xFC && 0x4C <= *lpszTrail)
					{	// 공백
						return ( true);	
					}
				}
				else
				{
					return ( true);
				}
			}
			else if ( *lpszCheck == 0x81)
			{
				if ( *lpszTrail != 0x58 && *lpszTrail != 0x5B)
				{
					return ( true);
				}
			}
			else if ( *lpszCheck == 0x82)
			{
				if ( *lpszTrail <= 0x4E || ( 0x59 <= *lpszTrail && *lpszTrail <= 0x5F) ||
					( 0x7A <= *lpszTrail && *lpszTrail <= 0x80) ||
					( 0x9B <= *lpszTrail && *lpszTrail <= 0x9E) ||
					( 0xF2 <= *lpszTrail))
				{	// 일부 전각/히라가나를 제외한 공백
					return ( true);
				}
			}
			else if ( *lpszCheck == 0x83)
			{
				if ( *lpszTrail < 0x40 || 0x97 <= *lpszTrail)
				{	// 히라가나 외의 공백 및 특수문자
					return ( true);
				}
			}
			else
			{
				return ( true);
			}
			++lpszCheck;
		}
	}
#elif SELECTED_LANGUAGE == LANGUAGE_ENGLISH	    // 영문판만 체크한다.
	for ( unsigned char *lpszCheck = ( unsigned char*)Text; *lpszCheck; ++lpszCheck)
	{
		if ( 1 == _mbclen( lpszCheck))
		{	// 한 바이트
			//사용 가능한 0~9, A~Z, a~z 를 제외한 나머지문자 사용 금지.
			if( !(48 <= *lpszCheck && *lpszCheck < 58) 
				&& !(65 <= *lpszCheck && *lpszCheck < 91) 
				&& !(97 <= *lpszCheck && *lpszCheck < 123) )
 			{
				return ( true);
			}
		}
		else
		{	// 두 바이트
			return ( true);
		}
	}
#elif SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES // 필리핀만 체크한다.
	for ( unsigned char *lpszCheck = ( unsigned char*)Text; *lpszCheck; ++lpszCheck)
	{
		if ( 1 == _mbclen( lpszCheck))
		{	// 한 바이트
			//if ( *lpszCheck < 0x21 || *lpszCheck > 0x7E)
			if ( *lpszCheck < 0x21 || *lpszCheck == 0x27 || *lpszCheck == 0x2C || *lpszCheck == 0x2E ||
				*lpszCheck == 0x60 || 0x7F <= *lpszCheck)
			{
				return ( true);
			}
		}
		else
		{	// 두 바이트
			return ( true);
		}
	}
#elif SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE // 베트남만 체크한다.
	for ( unsigned char *lpszCheck = ( unsigned char*)Text; *lpszCheck; ++lpszCheck)
	{
		if ( 1 == _mbclen( lpszCheck))
		{	// 한 바이트
			//if ( *lpszCheck < 0x21 || *lpszCheck > 0x7E)
			if ( *lpszCheck < 0x21 || *lpszCheck == 0x27 || *lpszCheck == 0x2C || *lpszCheck == 0x2E ||
				*lpszCheck == 0x60 || 0x7F <= *lpszCheck)
			{
				return ( true);
			}
		}
		else
		{	// 두 바이트
			return ( true);
		}
	}
#elif SELECTED_LANGUAGE == LANGUAGE_THAILAND    // 태국판만 체크한다.
	for ( unsigned char *lpszCheck = ( unsigned char*)Text; *lpszCheck; ++lpszCheck)
	{
		if ( 1 == _mbclen( lpszCheck))
		{	// 한 바이트
			//if ( *lpszCheck < 0x21 || *lpszCheck > 0x7E)
			if ( *lpszCheck < 48 || ( 58 <= *lpszCheck && *lpszCheck < 65) ||
				( 91 <= *lpszCheck && *lpszCheck < 97) || *lpszCheck > 122)
			{
				return ( true);
			}
		}
		else
		{	// 두 바이트
			unsigned char *lpszTrail = lpszCheck + 1;
			if ( 0x81 <= *lpszCheck && *lpszCheck <= 0xC8)
			{	// 한글
				if ( ( 0x41 <= *lpszTrail && *lpszTrail <= 0x5A) ||
					( 0x61 <= *lpszTrail && *lpszTrail <= 0x7A) ||
					( 0x81 <= *lpszTrail && *lpszTrail <= 0xFE))
				{	// 투명글자 뺀부분
					// 안되는 특수문자영역들
					if ( 0xA1 <= *lpszCheck && *lpszCheck <= 0xAF && 0xA1 <= *lpszTrail)
					{
						return ( true);
					}
					else if ( *lpszCheck == 0xC6 && 0x53 <= *lpszTrail && *lpszTrail <= 0xA0)
					{
						return ( true);
					}
					else if ( 0xC7 <= *lpszCheck && *lpszCheck <= 0xC8 && *lpszTrail <= 0xA0)
					{
						return ( true);
					}
				}
				else
				{
					return ( true);
				}
			}
			else
			{
				return ( true);
			}

			++lpszCheck;
		}
	}
#endif
	return false;
}

#endif // KJH_MOD_NATION_LANGUAGE_REDEFINE
