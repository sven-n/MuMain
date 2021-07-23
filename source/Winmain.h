#ifndef __WINMAIN_H__
#define __WINMAIN_H__


//!! 패치시 주석 처리 해야 함
/*-------------------- 프로그램 mode 설정 ----------------------------------------------*/
#define FOR_LAST_TEST			// 최종 테스트 용 버전 (스탯 올리기 쉽게)
#define FOR_WORK				// 사내 작업용 버젼 (Alt-Tab 가능)
#define FOR_HACKING				// 해킹팀용 버젼 (클라이언트 복수 실행 가능)
/*--------------------------------------------------------------------------------------*/

//////////// 중요!!!!! - 이혁재 //////////////////////////////////////////////////////////////////////////////////
//	- 앞으로 테섭 패치는 항시 이 디파인 다 켜줍시다. ^^ 이유는 아래있는 파일의 수정 패치만 있을시				//
//	  exe 파일을 만들 필요가 없어지고 데이터만 패치가 가능해지면 서버 또한 패치가 필요 없어지기 때문입니다 ^^	//
//    항시 켜 둡시다.																							//
//////////// 중요!!!!! - 이혁재 //////////////////////////////////////////////////////////////////////////////////


//테섭시에는 MuTest.ini를 읽도록 켜 주어야 한다. 
#ifdef _TEST_SERVER	// 중요!!!!! - 이혁재 위에 내용 읽어 주세요
	#define USE_MUTEST_INI	//MuTest 호출(테섭 게임가드실행)-> 항상 킨다.
	#define USE_TEXTTEST_BMD
	#define USE_ITEMTEST_BMD
	#define USE_SKILLTEST_BMD
	#define USE_GATETEST_BMD
	#define USE_PLAYERTEST_BMD
	#define USE_QUESTTEST_BMD
	#define USE_DIALOGTEST_BMD
	#define USE_SET_ITEMTEST_BMD
	#define USE_MOVEREQTEST_BMD
	#define USE_SLIDETEST_BMD
	#define USE_MONSTERSKILLTEST_BMD
	#define USE_CREDITTEST_BMD
	#define USE_HARMONYJEWELOPTION_BMD
	#define USE_NOTSMELTING_BMD
	#define USE_ITEMADDOPTION_BMD
	#define USE_MIXTEST_BMD
	#define USE_MASTERTREE_BMD
	#define USE_SOCKETITEM_TEST_BMD
	#define USE_PET_TEST_BMD
	#define USE_FILTER_TEST_BMD
	#define USE_QUESTPROGRESSTEST_BMD
	#define USE_QUESTWORDSTEST_BMD
	#define USE_NPCDIALOGUETEST_BMD
	#define USE_MINIMAPTEST_BMD
#endif //_TEST_SERVER

/*-------------------- 국가 선택 -------------------------------------------------------*/

// 지원하는 언어 정의
#define LANGUAGE_KOREAN			( 0 )	// 한국어
#define LANGUAGE_ENGLISH		( 1 )	// 영어
#define LANGUAGE_TAIWANESE		( 2 )	// 대만어
#define LANGUAGE_CHINESE		( 3 )	// 중국어
#define LANGUAGE_JAPANESE		( 4 )	// 일본어
#define LANGUAGE_THAILAND		( 5 )	// 태국어
#define LANGUAGE_PHILIPPINES	( 6 )	// 필리핀
#define LANGUAGE_VIETNAMESE		( 7 )	// 베트남
#define NUM_LANGUAGE			( 8 )
// 여기서 언어 선택

//#ifdef KJH_MOD_LIBRARY_LINK_EACH_NATION
//----------------------------------------------------------------------------- Kor
// Kor
#ifdef _LANGUAGE_KOR
	#define SELECTED_LANGUAGE	( LANGUAGE_KOREAN )
//----------------------------------------------------------------------------- Eng
// Eng
#elif _LANGUAGE_ENG
	#define SELECTED_LANGUAGE	( LANGUAGE_ENGLISH )
//----------------------------------------------------------------------------- Tai
// Tai
#elif _LANGUAGE_TAI
	#define SELECTED_LANGUAGE	( LANGUAGE_TAIWANESE )
//----------------------------------------------------------------------------- Chs
// Chs
#elif _LANGUAGE_CHS
	#define SELECTED_LANGUAGE	( LANGUAGE_CHINESE )
//----------------------------------------------------------------------------- Jpn
// Jpn
#elif _LANGUAGE_JPN
	#define SELECTED_LANGUAGE	( LANGUAGE_JAPANESE )
//----------------------------------------------------------------------------- Tha
// Tha
#elif _LANGUAGE_THA
	#define SELECTED_LANGUAGE	( LANGUAGE_THAILAND )
//----------------------------------------------------------------------------- Phi
// Phi
#elif _LANGUAGE_PHI
	#define SELECTED_LANGUAGE	( LANGUAGE_PHILIPPINES )
//----------------------------------------------------------------------------- Vie
// Vie
#elif _LANGUAGE_VIE
	#define SELECTED_LANGUAGE	( LANGUAGE_VIETNAMESE )
#endif
//-----------------------------------------------------------------------------
// #else // KJH_MOD_LIBRARY_LINK_EACH_NATION
// 	#define SELECTED_LANGUAGE	( LANGUAGE_ENGLISH ) --> 수정하지 말것
// #endif // KJH_MOD_LIBRARY_LINK_EACH_NATION

/*--------------------------------------------------------------------------------------*/
#ifdef _BLUE_SERVER
	#define GAMEGUARD_BLUEMU_VERSION			// 블루 뮤 적용된 게임가드
#endif //_BLUE_SERVER

#if SELECTED_LANGUAGE == LANGUAGE_KOREAN || SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_TAIWANESE || SELECTED_LANGUAGE == LANGUAGE_ENGLISH || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES
// - 안성민
// 게임가드 인증 2.5버전(절대 디파인 위치 옮기지 말 것!)
//	이 디파인이 꺼져 있을 시에는, 게임가드 라이브러리를 051223버전 전으로 덮어써야 함
#define GAMEGUARD_AUTH25
#endif //SELECTED_LANGUAGE == LANGUAGE_KOREAN || SELECTED_LANGUAGE == LANGUAGE_JAPANESE

/*-------------------- 국가별로 다른 옵션 설정 -----------------------------------------*/

#ifdef _DEBUG
	#define ENABLE_EDIT				// 시스템 메세지
	#define ENABLE_EDIT2			// 방향키로 이동
	//#define ACC_PACKETSIZE		// 전송되는 패킷 사이즈 계산
	#if SELECTED_LANGUAGE == LANGUAGE_KOREAN	// 한국 버전에서만 작동하며, 해외버전에서는 boost new 와 충돌 발생합니다. (2009.01.05)
		#define USE_CRT_DEBUG			// 메모리 누수 디버깅
	#endif // SELECTED_LANGUAGE == LANGUAGE_KOREAN
	//#define PK_ATTACK_TESTSERVER_LOG		// 테섭에 PK 관련 공격쪽 로그남기는 작업(2007.04.25)
#endif // _DEBUG

#ifdef FOR_WORK
/*----------------------- 디버깅용 모듈들 --------------------------------------------------*/

//#define KJH_LOG_ERROR_DUMP					// 에러덤프기능 로그 출력 (2009.06.15)
#ifdef KJH_LOG_ERROR_DUMP
	#define LOG_ERROR_DUMP_FILENAME		"Log_ErrorDump.txt"		// 에러덤프기능 로그파일이름
#endif // KJH_LOG_ERROR_DUMP

//#define PBG_LOG_PACKET_WINSOCKERROR		// 윈속에러 로그 확인
#ifdef PBG_LOG_PACKET_WINSOCKERROR
	#define PACKET_LOG_FILE	"PacketLogFileRecv.txt"
#endif //PBG_LOG_PACKET_WINSOCKERROR

#ifdef _DEBUG

	//$ 콘솔 디버그
	#define CSK_LH_DEBUG_CONSOLE			// 콘솔로 디버깅 정보 남기는 처리(2007.03.20)
	#ifdef CSK_LH_DEBUG_CONSOLE
		#define CONSOLE_DEBUG				// 콘솔로 디버깅 정보 출력(2007.03.20)
	#endif // CSK_LH_DEBUG_CONSOLE

	//$ 맵속성 보는 기능
	#define CSK_DEBUG_MAP_ATTRIBUTE			// 맵 속성을 게임내에 출력(2008.10.14)

	//$ 바운딩박스 렌더링 기능
	#define CSK_DEBUG_RENDER_BOUNDINGBOX		// 바운딩 박스 렌더링(2008.10.28)

	//$ 길찾기노드 보는 기능
	#define CSK_DEBUG_MAP_PATHFINDING		// 길찾기노드를 게임내에 출력(2008.10.14)

	//$ 프로파일링 & 디버깅 카메라	
	// 프레임이 느려지는 관계로, 프로파일링,카메라 작업을 할때는 local로 작업하시오
	//#define LDS_ADD_DEBUGINGMODULES	// 내부 Profiling 모드 (결과파일은 RESULT_PROFILING.txt로 저장) 

	#ifdef LDS_ADD_DEBUGINGMODULES
		#define DO_PROCESS_DEBUGCAMERA	// Main GameScene 내 디버깅용 카메라를 설정 (2008.07.30)
		//#define LDS_ADD_DEBUGINGMODULES_FOR_PROFILING
		#ifdef LDS_ADD_DEBUGINGMODULES_FOR_PROFILING
			#define DO_PROFILING			// PROFILING_RESULT.txt-결과 출력 // 내부 Profiling 모드 적용 여부.  (2008.07.11)
			//#define DO_PROFILING_FOR_LOADING// PROFILING_FOR_LOADING_RESULT.txt-결과 출력 // Loading시점만 Profiling 모드.  (2008.08.19)
		#endif // LDS_ADD_DEBUGINGMODULES_FOR_PROFILING
	#endif // LDS_ADD_DEBUGINGMODULES

	// 프레임이 느려지는 관계로, 프로파일링,카메라 작업을 할때는 local로 작업하시오
	//#define LDS_UNFIXED_FIXEDFRAME_FORDEBUG	// 초당 20고정 프레임을 자유로 변환 가능 사용자입력코드($unfixframe on/$unfixframe off) 사용 활성화, 만약 realease 모드에서 사용을 원할 시 CSK_LH_DEBUG_CONSOLE 디파인을 필히 켜야 합니다.

#endif // _DEBUG

//#define LDS_FOR_DEVELOPMENT_TESTMODE
#ifdef LDS_FOR_DEVELOPMENT_TESTMODE
	#define CSK_LH_DEBUG_CONSOLE			// 콘솔로 디버깅 정보 남기는 처리(2007.03.20)
	//#define ENABLE_FULLSCREEN				// 풀스크린	(간혹 Release(사내테스트용)시에 전체화면 기능을 원하는 경우가 있음.)
#endif // LDS_FOR_DEVELOPMENT_TESTMODE

/*------------------------------------------------------------------------------------------*/
#endif // FOR_WORK

#ifdef NDEBUG
	#ifdef FOR_HACKING				// 해킹팀용
	#else // FOR_HACKING
		#define ENABLE_FULLSCREEN	// 풀스크린
	#endif // FOR_HACKING

	#ifndef FOR_WORK
		
		// 리소스가드
		#define RESOURCE_GUARD		
		
		//#define USE_SELFCHECKCODE	// 셀프 체크 코드 사용

		// 게임가드
		#if SELECTED_LANGUAGE != LANGUAGE_CHINESE
			//#define USE_MU_INTERNAL_TEST			//게임가드 사내 테스트버전
			//#define USE_VIETNAMESE_NPGAME_TEST	//게임가드 사내 테스트 버젼(베트남)
			#define NP_GAME_GUARD					// nProtect Game Guard.
			#define GG_SERVER_AUTH					// 게임가드 - 서버를 통한 인증
			//#define USE_NPROTECT	// nProtect 를 사용한다. 안철수 핵쉴드로 연결된다
		#endif	// SELECTED_LANGUAGE != LANGUAGE_CHINESE

#ifdef KJH_MOD_NATION_LANGUAGE_REDEFINE
#ifndef _LANGUAGE_KOR				// KOR 제외
		#define FOR_ADULT		// 성인용(피빨간색)
#endif // _LANGUAGE_KOR
#else // KJH_MOD_LIBRARY_LINK_EACH_NATION
		#if SELECTED_LANGUAGE == LANGUAGE_KOREAN
			//#define USING_MFGS	// MFGS 사용 체크체크
		#else // SELECTED_LANGUAGE == LANGUAGE_KOREAN
			#define FOR_ADULT		// 성인용(피빨간색)
		#endif // SELECTED_LANGUAGE == LANGUAGE_KOREAN
#endif // KJH_MOD_NATION_LANGUAGE_REDEFINE

	#endif // FOR_WORK
#endif // NDEBUG

#if SELECTED_LANGUAGE == LANGUAGE_KOREAN
	#define WAV_CONVERT		// wav 컨버트 기능을 넣을 것인가? 한글 음악 파일을 영어로 바꾸는 기능
	#ifndef WAV_CONVERT
		#define KOREAN_WAV_USE		// 한글로 된 wav 파일을 쓸 것인가? 
	#endif
#endif

#ifdef NP_GAME_GUARD
	#define GG_SERVER_AUTH			// 게임가드 - 서버를 통한 인증

	#if defined(GAMEGUARD_AUTH25) || SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || SELECTED_LANGUAGE == LANGUAGE_ENGLISH
		#define NPROTECT_AUTH2		// NProtect Auth2적용
	#endif

#endif // NP_GAME_GUARD 

#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
#define ACTIVE_FOCUS_OUT				// 창 바깥으로 포커스 나가도 게임 정지 안됨 & alt-tab허가
#endif	// USER_WINDOW_MODE

#if SELECTED_LANGUAGE == LANGUAGE_ENGLISH
// 	#define LIMIT_CHAOS_MIX			// 2차 날개, 군주의 망토, 투명망토, 데빌스퀘어 초대권 못 만들게 함(npc 텍스트도 뺌) -> 시즌4패치에서 막음
// 	#define LIMIT_SKILL				// 글로벌은 스웰라이프, 블로우 스킬 막음 -> 시즌4패치에서 막음
	#define GRAMMAR_OF				// of 문제
#endif // SELECTED_LANGUAGE == LANGUAGE_ENGLISH

#if SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES
	#define GRAMMAR_OF				// of 문제
#endif

#if SELECTED_LANGUAGE == LANGUAGE_CHINESE
	#define USER_WINDOW_MODE		// 유저 창모드 사용
	#define ONLY_PSWD_STORAGELOCK	// 창고 잠금 기능에 비번만 사용 (중국)
	#define CHINESE_PRICERISE		// 중국 물가상승정책
	//#define ANTIHACKING_ENABLE	// 중국에서 개발된 anti-hacking solution 을 이용한다.
#else
	#define BAN_USE_CMDLINE				// 명령줄사용을 금지한다. 대신, Mu Virtual Commander를 제공한다. (2005-02-18)
	#define NEW_YEAR_BAG_EVENT		// 설만 복주머니 이벤트.   ( 중국에서는 사용하지 않는다. )
#endif

#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
	#define TAMAJJANG					// 타마짱. ( 일본 ).
	//#define HERO_SELECT					// 자기 자신 선택하기.
	#define JAPANESE_ITEM_HELP_FIX		// 일본 아이템 도움말 에너지 표시 수정 (2004-07-22)
	//#define JPN_SERIAL_NOT_RENDERTEXT	// 일본 황금 궁수 시리얼 입력시 아이템 명칭 표시 안되게 수정(2005.11.03)
#endif

#if SELECTED_LANGUAGE != LANGUAGE_TAIWANESE
	#define CONNECT_LIST_FIX		// 접속 서버 순서 변경 쉽게 고친것 (대만버전은 사용하지 않거나 순서 새로 바꿔줘야 함)
#endif

#if SELECTED_LANGUAGE == LANGUAGE_CHINESE || SELECTED_LANGUAGE == LANGUAGE_TAIWANESE
	#define CHINA_MOON_CAKE			// 중국 월병 이벤트.
#endif

#if SELECTED_LANGUAGE == LANGUAGE_TAIWANESE || SELECTED_LANGUAGE == LANGUAGE_JAPANESE \
	|| SELECTED_LANGUAGE == LANGUAGE_CHINESE
	#define DISABLE_MUTO			// 뮤토복권은 사용안함(대만) (2004-07-27)
#endif

#if SELECTED_LANGUAGE == LANGUAGE_CHINESE || SELECTED_LANGUAGE == LANGUAGE_TAIWANESE \
	|| SELECTED_LANGUAGE == LANGUAGE_ENGLISH || SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES \
	|| SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
	#define GRAMMAR_VERB			// 동사 문제 ( welcome to 같은)
#endif

#if SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
	#define PHIL_LANGUAGE_PROBLEM	// 어순 문제 (관련 버그 모두 포함해야 함, 추후 발견시 이 아래로 추가)
#endif

#if SELECTED_LANGUAGE == LANGUAGE_ENGLISH
	#define ENG_LANGUAGE_PROBLEM	// 어순 문제 (관련 버그 모두 포함해야 함, 추후 발견시 이 아래로 추가)
#endif

#if !defined(NP_GAME_GUARD) && !defined(FOR_WORK) && !defined(_DEBUG)
	//#define ATTACH_HACKSHIELD		// 안철수연구소 핵쉴드 적용
#endif


#ifndef ANTIHACKING_ENABLE		// Anti-hacking solution 관련 함수들 overload
	#define hanguo_check1();	{}
	#define hanguo_check2();	{}
	#define hanguo_check3();	{}
	#define hanguo_check4();	{}
	#define hanguo_check5();	{}
	#define hanguo_check6();	{}
	#define hanguo_check7();	{}
	#define hanguo_check8();	{}
	#define hanguo_check9();	{}
	#define hanguo_check10();	{}
	#define hanguo_check11();	{}
	#define hanguo_check12();	{}
	#define	pre_send( g_hInst);	{}
#endif

/*--------------------------------------------------------------------------------------*/


/*----------------------- 이벤트 -------------------------------------------------------*/

//#define CSK_EVENT_HALLOWEEN_MAP	//	할로윈 이벤트 맵 처리(2006.10.09)
//#define DEVIAS_XMAS_EVENT			//  크리스마스 이벤트 관련 데비아스 맵 및 리소스 수정(2005.12)
#define STONE_EVENT_DIALOG			// 스톤 이벤트 ( 영석 러쉬 게임, 공짜 경매 이벤트 ).
//#define LENA_EXCHANGE_ZEN			// 레나 환전 시스템, 주위 : 스톤에서는 켜서는 안된다.
#define BLOODCASTLE_2ND_PATCH		// 스톤 환전 시스템, 조합관련
#define USE_EVENT_ELDORADO			// 엘도라도의 상자 이벤트
#define SCRATCH_TICKET				// 상용화 2주년 이벤트.
#define FRIEND_EVENT				// 친구 이벤트.
#define FRIENDLYSTONE_EXCHANGE_ZEN	// 우정의돌 환전 시스템

#define CASTLE_EVENT				// 군주의 문장 등록
#ifdef CASTLE_EVENT
	#undef FRIEND_EVENT
	#undef FRIENDLYSTONE_EXCHANGE_ZEN
	#undef LENA_EXCHANGE_ZEN
	#define DISABLE_MUTO			// CASTLE_EVENT 시 필요
#endif

#define BATTLE_SOCCER_EVENT         // 배틀싸커 이벤트.
#define LOREN_RAVINE_EVENT			// 로랜협곡 이벤트에서 쓰임.
//#define GUILD_WAR_EVENT           // 길전 이벤트.
//#define MYSTERY_BEAD				// 신비의 구슬 이벤트

#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
	#define ANNIVERSARY_EVENT			// 일본 1주년 폭죽 이벤트 (다른나라도 사용할 수 있음)
#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE

#define	PCROOM_EVENT				//	PC방 이벤트.(2006.1.26)

/*--------------------------------------------------------------------------------------*/


/*----------------------- 시스템 관련 --------------------------------------------------*/

#define DUEL_SYSTEM					// 결투 시스템
#define PET_SYSTEM					// 애완 시스템.
#define PROTECT_SYSTEMKEY			// 게임중 시스템키로부터 게임을 보호한다.
#define CHANGE_RATE					// 등급 바꾸기
//#define SYSTEM_NOT_RENDER_TEXT	// 텍스트를 찍지 않을 시스템(2007.11.28)
//#define FOR_DRAMA					// 드라마 삼총사용
//#define ON_AIR_BLOODCASTLE		// 블러드 캐슬 방송을 위한 작업. 필요없는 메세지 출력 안함
//#define CAMERA_TEST				// 스샷용 ( 캐릭터 안보임 )
//#define CAMERA_TEST_FPS			// 1인칭 시점과 같은 각도로 멀리까지 보기
//#define DONTRENDER_TERRAIN		// 지형을 그리지 않는다

/*--------------------------------------------------------------------------------------*/


/*----------------------- 나둘것들 -----------------------------------------------------*/

#define CRYINGWOLF_2NDMVP				// 크라이울프 점령지 - 헤더랑 시피피랑 관련된 함수는 전부 지우고 이펙트만 남겨두세요
#define GIFT_BOX_EVENT					//  발렌타인데이 이벤트 관련 (2006.1)
#define BUGFIX_CRYWOLF_END_COUNT		// 종료카운트 뜨고 있는데 제단 클릭 가능한 버그
//#define CASTLE_TAX_LIMIT              // 공성의 세금 조절 기능 사용금지.
//#define STATE_LIMIT_TIME              // 캐릭터 상태 제한 시간.
//#define AUTO_CHANGE_ITEM				// 자동으로 장비 교체하기.
//#define UNDECIDED						// 마우스 휠 관련 ( 중간 휠 버튼으로 물약 먹기와 휠로 단축키 쓰기)
//#define ENABLE_CHAT_IN_CHAOS			// 카오스캐슬에서 쳇팅가능하도록함(운영자 모드)
//#define ENABLE_POTION_EFFECT			// 물약먹는 효과만 켜기 (DISPLAYVIEWPORT_EFFECT와 같이 사용되어야 한다)
//#define _PVP_ATTACK_GUARD				// 경비병 공격 가능하게
//#define _PVP_MURDERER_HERO_ITEM		// 영웅&살인마 아이템&상점
//#define _PVP_ADD_MOVE_SCROLL			// 이동문서 추가
//#define _PVP_BLOCK_PVP_CHAR			// 캐릭터 선택시 살인마 로그인 막기
//#define _PVP_DYNAMIC_SERVER_TYPE		// 서버 pvp가능 여부 동적 변경
//#define USE_SKILL_LEVEL				// 스켈에 레벨 개념 추가
//#define USE_SHADOWVOLUME		        // 섀도우 볼륨 사용
//#ifdef _TEST_SERVER
//	#define	ADD_EXPERIENCE_SERVER			//	시즌 2 체험 서버(2006.08.04)
//#endif	//  _TEST_SERVER
//#define RE_PRICE_SPECIAL_ITEM
//#define USE_MASTER_SKILL
//#define CRYWOLF_1ST
//#define BATTLE_CASTLE_AS				// 공성보완작업디파인(2005-10-14) 공성전 테스트 해 보아야 함!!
//#define BATTLE_CASTLE					// 공성전.
//#define ADD_MONSTER_SKILL
//#define MVP_INTERFACE
//#define REPAIR_KALIMA					//  칼리마 레벨조정(2005-12-12 추가)
//#define REPAIR_HELPER_ELF				//  공방npc 공격,방어 수치조절(2005-12-14 추가)
//#define MVP_RESULT					//	크라이울프 혜택 +카오스조합확률(2005-12-19 추가)
//#define SELL_ITEM_PRICE				//  사탄하고 수호의 보석 팔때 가격 /3 안하게(2006-02-21 추가)-한시적으로 운영할것임.
//#define VISTA_ADD						//  비스타 대비 추가 코드(2006-12-11 추가)
//#define OBJECT_ID						//오브젝트에 ID추가
//#define DYNAMIC_FRUSTRUM				//오브젝트에 각자의 프러스텀을 가질 수 있다

/*--------------------------------------------------------------------------------------*/


/*----------------------- 작업 취소된거 ------------------------------------------------*/
// - 최석근
//#define MOVIE_DIRECTSHOW				//	DirectShow를 이용해서 동영상 실행(2005.12.09)

// 안성민, 이동석
//#define MR0
#ifdef	MR0
	#define NEW_RENDERING_SYSTEM

	// 1. MR0:System Type
	#ifdef NEW_RENDERING_SYSTEM
		#define NEW_RENDER_OBJECT
		#define NEW_RENDER_TERRAIN
//		#define NEW_RENDER_EFFECT			
	#endif //NEW_RENDERING_SYSTEM

	// 2. MR0:보강 된 사항들
	#define LDS_ADD_RENDERMESHEFFECT_FOR_VBO					// RenderMeshEffect() 함수에 대한 VBO 버전을 추가 합니다. (함수내에서 VBO에서는 허용안되는 WorldVertex접근으로 "치명적 오류" 발생) (09.09.09)
	#define LDS_MOD_ANIMATIONTRANSFORM_WITHHIGHMODEL			// AnimationTransformWithAttachHighModel함수에서 현재 arrayBone만 필요할 경우는 Transform으로 Bone이 Vertices에 곱하지 않도록 변경. (2009.09.09)
	#define LDS_ADD_ANIMATIONTRANSFORMWITHMODEL_USINGGLOBALTM	// AnimationTransformWithAttachHighModel함수내 TMArray가 Local인 이유로 이후 연산에 TM이 Point를 잃어버리는 현상으로 TM을 Global 으로 변경. (09.09.07)
	#define LDS_MR0_MODIFY_DIRECTLYAPPLY_SCALETM_INCLUDESHADER	// ShaderCode+RenderMachine계산식 보강 : Chrome효과와 같은 VertexNormal을 이용한 계산시에 Normal에 Scale영향을 받지 않도록 Position에만 Scale이 곱해지도록 메커니즘 변경. (2009.09.01)
	#define LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE			// 선택 케릭터 및 오브젝트 실루엣 보강 : 스케일 변환식 보강:오브젝트의 본래 스케일 값이 아닌 유저 스케일 적용 오류로 연산 보강 (2009.01.02)
	#define LDS_MR0_MODIFY_MISSRENDERSHADOW_MOSTMESH			// 쉐도우 출력 :대부분의 메쉬의 쉐도우 미출력 문제 (2009.01.05)
	#define LDS_MR0_MODIFY_LOGINSCENE_SHADOWPROBLEM				// 특정 맵의 LightDisable 처리. LightEnable 되면 Blending 이 꺼져, Fog 효과가 처리 되지 않음. (2009.01.07)
	#define LDS_MR0_MODIFY_ANIMATIONMAPPING_NOANIMATED			// 에니메이션 Mapping에 대해 Animate 되지 않는 현상(BlendTexture로써 처리가 되어야 하는데 Blend 처리가 이루어지지 않는 문제.) (2009.01.09)
	#define LDS_MR0_MODIFY_WHENREOPEN2_SKIPCONVERT				// Bmd::Open2로 이미 읽혀진 파일에 대해서 Skip Parameter에 대하여, VBO의 ConvertMesh또한 Skip 되도록 설정합니다. (2009.02.24)
	#define LDS_MR0_ADD_WHENSHUTDOWN_UNINIT_MODELMANAGER		// ShutDown 시에 정상 종료가 되도록 ModelManager Uninit 합니다. (2009.02.26)	
	#define LDS_MR0_MODIFY_RENDER_SKIPORNOT_WHEN_NONBLENDMESH	// TextureScript Parsing 시 _N 텍스쳐들에 대해 NonBlendMesh가 True설정 되며, AddRender()함수상에서 Render Skip되어, 소환술사 헬멧 렌더가 안보이는 문제. (2009.02.26)
	#ifdef LDS_ADD_RENDERMESHEFFECT_FOR_VBO
		#define LDS_MR0_ADD_DONTDELETE_OLDMESHSBUFFER_DARKSPIRIT	// RenderMeshEffect 함수에서 구모델버전 버텍스정보가 필요, olddelmodel을 않하도록 변경. 다크스피릿. (09.09.10)
	#endif // LDS_ADD_RENDERMESHEFFECT_FOR_VBO
	
	#define LDS_MR0_MOD_PATIALPHYSIQMODEL_PHYSICPROCESS_FIX		// Physic에서 다루는 통물리모델, 부분물리모델 중 부분물리모델에서만 발생하는 문제 FIX작업. (09.09.28)
	#define LDS_MR0_ADD_VBOMANAGER_SETONOFF						// VBO Manager를 On/Off 하는 함수를 추가 (09.09.29)
	#define LDS_MR0_ADD_SPECIFICMODEL_DOANIMATIONPROCESSING		// VBO Model로 변환된 이후 구 모델 정보가 필요하여 적재를 남겨두는 모델들에 대해 구 버전의 Animation Transformation 연산을 지정된 메쉬만 하도록 합니다. 연산된 버텍스 정보가 필요한 경우 등등. (09.10.20)
	#define LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT				// 모든 Shader Script들에 대해 Encryption/Decryption. (09.11.25)

	#define LDS_MR0_MOD_WHENTRANFORM_BUILDOBB_ALLWAYS			// BMD파일이 없는 경우라도 BMD 모델의 OBB를 무조건적으로 구성. (09.12.01)
	#define LDS_MR0_INTIALIZE_SOME_MAINVARIABLES				// 몇몇 주요변수들의 초기화가 미흡한것에 대한 보충. (09.12.02)
	#define LDS_MR0_MOD_SYNCHRONIZE_SETRARGETOBJECT				// 옵션창의 효과 제한이 3인경우 주인공 케릭터 깨짐현상. RenderPartObjectEffect 함수에서 ShaderObject 미렌더링 상태로 반환 문제로 깨짐. (09.12.04)
	#define LDS_MR0_MOD_FIXEDFRAME_DELTA						// 고정 20프레임의 현재 DELTA프레임에 대해 고정계산에서 비고정계산으로 변경. (09.12.09s)

	#define LDS_MR0_MOD_DUETOOVERCOUNTBONES_BREAKVISUAL			// 메쉬별 본갯수가 쉐이더 파라미터 갯수 한도(약 70개) 초과로 인한 깨짐 현상으로 메쉬별 본 갯수 변경 (사피 3종 + 고블린 시드 마스터 + 토템 골렘) (10.01.20)

	#define LDS_MR0_FIX_BREAKVISUAL_DOPPELGANGER_CHARACTER		// 도플갱어맵의 "도플갱어" 몬스터의 예외적 RenderMesh, RenderBody 호출로 인한 TM 언매치로 깨짐현상발생-> SetTargetObject() 묶어주고, Move단계에서 호출된 RenderBody를 RenderMonsterVisual로 위치 이동. (10.02.03)

	#define LDS_MR0_MOD_SET_LOWRENDERER_INIT					// 기존 비주얼 버전에 맞추기 위해 LOWRENDERER 로 강제 설정 합니다. (2010.03.30)

	// 3. MR0:For TESTDEBUG
	#ifdef _DEBUG
		#define LDS_MR0_FORDEBUG_USERRESPOND_BEINGVPMANAGERENABLE// 현재 VPMANAGER가 Enable 상태 인지 여부를 반환 합니다.
		#define LDS_MR0_FORDEBUG_VERIFYCONVERTMESH				// ConvertMesh로 변환된 VBOMesh의 검증을 출력 합니다. (2009.01.15)
		#define LDS_MR0_FORDEBUG_CALCTRANFROMBYCPU				// Transform 을 CPU로 연산 합니다. (2009.02.02)

		// For Debug
		#define LDS_FOR_DEVELOPMENT_TESTMODE					// Debug 정보 출력 (LDS_UNFIXED_FIXEDFRAME_FORDEBUG 포함)
		#define CSK_LH_DEBUG_CONSOLE							// 콘솔로 디버깅 정보 남기는 처리(2007.03.20)
		//#define ENABLE_FULLSCREEN								// 풀스크린	(간혹 Release(사내테스트용)시에 전체화면 기능을 원하는 경우가 있음.)

		// Output GL Error 
		#define LDS_MR0_OUTPUT_GLERROR_IN_BITMAPLOAD			// Output OpenGL Error In Jpeg, Tga Loading Function (2009.02.19)
		#define LDS_MR0_OUTPUT_GLERROR_INNER_GLRENDERER			// GL Renderer 내부의 GLERROR 단위 코드를 심어 놓습니다. (2009.02.26)
		#define LDS_MR0_OUTPUT_GLERROR_GENERAL					// 
	#endif // _DEBUG

	// 4. MR0:For Temporary 
	#define LDS_MR0_TEMPORARY
	#ifdef LDS_MR0_TEMPORARY
		#define LDS_MR0_FORDEBUG_MESHALLWAYS_VSTYPE				// MR0 메쉬 타입을 항상 VBO로 설정 합니다. PBO 구현 미완성 
		#define LDS_MR0_FIXED_RIGHT_SHADER_MAXPARAM_VALUE		// Env Param, Local Param 갯수가 실제 OpenGL에서 제공되는 갯수로 변경 (2009.04.02)
		#define LDS_MR0_FIX_ALLPETOBJECT_RENDERINGPASSEDWAY		// 펫메니저에서 그려지는 모델들(데몬,수호정령,펜더,루돌프등등..)은 ShaderRender가 아닌 기존 VertexRender 합니다.- 임시적 원인 불명오류. (09.12.01)
//		#define LDS_MR0_FIXED_SHADERPARAM_WHICH_MODEL_BREAKED_DUETO_BONE	// Shader에서 DrawCall 한번당 본 파라미터 64개 이상 넘길 수 없는 오류를 수정하기 위해 
																			// 갯수 제한 256개만 선언될 수 있는 env param에서 1024개인 local param으로 변경 (data\MixShader\Definitions.txt, data\Shader\shadowmap.vsh 쉐이더 파일 수정 )(2009.04.14)
	#endif // LDS_MR0_TEMPORARY

#endif //MR0
/*--------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------*/

#define WM_USER_MEMORYHACK	( WM_USER + 1)
#define WM_NPROTECT_EXIT_TWO  (WM_USER + 10001)

//#define PBG_ADD_INGAMESHOP_FONT

extern bool ashies;
extern int weather;
extern int Time_Effect;
extern HWND      g_hWnd;
extern HINSTANCE g_hInst;
extern HDC       g_hDC;
extern HGLRC     g_hRC;
extern HFONT     g_hFont;
extern HFONT     g_hFontBold;
extern HFONT     g_hFontBig;
extern HFONT     g_hFixFont;
#ifdef PBG_ADD_INGAMESHOP_FONT
extern HFONT	 g_hInGameShopFont;
#endif //PBG_ADD_INGAMESHOP_FONT
//extern HFONT     g_hFontSmall;
extern bool      Destroy;
extern int       RandomTable[];
extern char      *Language;

extern char m_ID[];
extern char m_Version[];
extern int  m_SoundOnOff;
extern int  m_MusicOnOff;
extern int  m_Resolution;
extern int m_nColorDepth;	

void MouseUpdate();
extern void CheckHack( void);
extern void CloseHack(HWND shWnd,bool Flag);
extern DWORD GetCheckSum( WORD wKey);
extern void StopMp3(char *Name, BOOL bEnforce=false);
extern void PlayMp3(char *Name, BOOL bEnforce=false);
extern bool IsEndMp3();
#ifdef YDG_ADD_DOPPELGANGER_SOUND
extern int GetMp3PlayPosition();
#endif	// YDG_ADD_DOPPELGANGER_SOUND
extern unsigned int GenID();

extern void CloseMainExe( void);
extern GLvoid KillGLWindow(GLvoid);
extern void DestroyWindow();
extern void DestroySound();

/*-------------------- Macro --------------------*/

/*----------------------- 디버깅용 모듈들 --------------------------------------------------*/
#ifdef DO_PROFILING
	#include "Profiler.h"
	extern CProfiler	*g_pProfiler;
#endif // DO_PROFILING

#ifdef DO_PROFILING_FOR_LOADING
	#include "Profiler.h"
	extern CProfiler	*g_pProfilerForLoading;
#endif // DO_PROFILING_FOR_LOADING

#ifdef DO_PROCESS_DEBUGCAMERA
	#include "DebugCameraManager.h"
	extern CDebugCameraManager *g_pDebugCameraManager;
#endif // DO_PROCESS_DEBUGCAMERA

#if defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
	extern bool g_bUnfixedFixedFrame;
#endif // defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)

#if defined _DEBUG || defined PBG_LOG_PACKET_WINSOCKERROR
	#include "./Utilities/Log/DebugAngel.h"
#define ExecutionLog	DebugAngel_Write
#else
#define ExecutionLog	{}
#endif //_DEBUG

#ifdef USE_CRT_DEBUG				
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
	#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif // USE_CRT_DEBUG

/*-------------------------------------------------------------------------------------------*/


/*----------------------- MR0 모듈들 --------------------------------------------------*/

#ifdef MR0
	#ifdef LDS_MR0_FIXED_RIGHT_SHADER_MAXPARAM_VALUE
		#define MR0_MAX_PARAMETERS			1024
		#define MR0_MAX_ENV_PARAMETERS		256
		#define MR0_MAX_LOCAL_PARAMETERS	1024
	#endif // LDS_MR0_FIXED_RIGHT_SHADER_MAXPARAM_VALUE

	#ifdef LDS_MR0_FIXED_SHADERPARAM_WHICH_MODEL_BREAKED_DUETO_BONE
		#define NUM_PRERESERVED_PARAM_GPUREGISTER	10
	#endif // LDS_MR0_FIXED_SHADERPARAM_WHICH_MODEL_BREAKED_DUETO_BONE
#endif // MR0
/*-------------------------------------------------------------------------------------------*/

#define FAKE_CODE( pos)\
	_asm { jmp pos };\
	_asm { __emit 0xFF };\
	_asm { __emit 0x15 }


#endif
