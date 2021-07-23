#ifndef __WINMAIN_NEW_H__
#define __WINMAIN_NEW_H__


//-------------------------------------------------------------------------------------------------------------------------------
// _LANGUGE_KOR (국내)
//-------------------------------------------------------------------------------------------------------------------------------
//
//  << 1. 필독사항 >>
// 1. 작업중인 디파인은 항상 주석처리 후 Check out. (작업중인 디파인 주석켜고 Check in 금지)
// 2. 패치할때 [작업중 디파인]에서 작업완료된 디파인을 [패치이후 디파인]으로 옮겨 패치한다.
// 3. 패치시, 작업자는 기획팀과 협의후 패치담당자에게 디파인내용 인수인계.
// 4. 담당 국가 작업후, 다른국가에 들어갈 내용이면 각 국가 디파인 파일의 [다음패치때 들어갈 디파인]에 복사하여 주석처리
// 5. 패치 디파인은 위에서 부터 아래로 진행 (최근 패치는 맨아래)
// 6. [패치이후 디파인]은 패치담당자 이외에는 건들지 마시오.
//
//  << 2. define 규정 >>
//	ex)
//	// - 작업자 이름
//	#define 이름_분류_작업내용						// 패치 설명 (시작일) [패치된 날자와 서버 or 예정]
//
// - 이름 : 작업자 이름의 이니셜
// - 분류 : '_FIX' (버그 수정), '_MOD' (기획상 기존 내용 변경), '_ADD' (컨텐츠 추가)로 디파인 구분.
// - 작업내용 : 자율적으로 영문화
//
//  << 3. 패치시 주석처리 >>
//	ex)
//	//----------------------------------------------------------------------------------------------
//	// <시즌5 - Part 4>
//	//----------------------------------------------------------------------------------------------
//  // [2009.01.22 베트남 1.03] - 안상규
//
//-------------------------------------------------------------------------------------------------------------------------------


//#if SELECTED_LANGUAGE == LANGUAGE_KOREAN		// 국내			
#ifdef _LANGUAGE_KOR							// 국내


/*--------- 현재 작업 중이며 본섭,테섭에 안들어간 Define ------------------------------------*/


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//##  [작업중 디파인]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// - 박기동

// - 안상규
//#define ASG_ADD_ETERNALWING_STICK_EFFECT				// 이터널윙 스틱 이펙트 추가 및 색상 변경(08.12.02)

// - 최석근 
//#define CSK_FIX_HACK									// 핵 수정 관련 작업(2008.11.27)
#ifdef CSK_FIX_HACK
	#define CSK_FIX_GAMEGUARD_CHECK						// 게임가드객체가 죽었을시 게임이 종료되게 수정(2008.11.27)
	#define CSK_FIX_NORMALATTACK_CHECK					// 일반공격 핵툴 대응(2008.12.02)
#endif // CSK_FIX_HACK

// - 양대근
//#define YDG_ADD_SKILL_RIDING_ANIMATIONS				// 시즌4 신규스킬 애니메이션 추가 (2008.08.22) [테스트중]
//#define YDG_MOD_CHECK_PROTECT_AUTO_FLAG					// 아이로봇 오토 막기 플래그 검사 루틴 (2009.03.11) [$new ON/OFF용-본섭시 주석]
//#define YDG_MOD_PROTECT_AUTO_TEST						// 아이로봇 오토 막기 테스트작업 [테스트중]
//#define YDG_MOD_PROTECT_AUTO_V4_R4						// 아이로봇 오토 막기 시리즈4 4차보완 - fastcall (2009.04.30) [패치안됨]
//#define YDG_MOD_PROTECT_AUTO_V4_R5						// 아이로봇 오토 막기 시리즈4 4차보완 - 암호화 구멍막기 (2009.04.24) [패치안됨]
//#define YDG_FIX_SOCKET_MISSING_MAGIC_POWER_BONUS		// 소켓아이템 최대/최소 공격력/마력 증가 옵션에 마력 증가 안되던것 수정 (2009.06.01) [테스트중]
//#define YDG_FIX_CURSEDTEMPLE_GAUGEBAR_ERROR				// 환영사원 성물 획득/등록 게이지 오류 수정 (2009.12.03)

// - 박종훈
//#define PJH_DEBUG										// 맵 이동 시간체크 (2010.10.08)
//#define PJH_FIX_SPRIT									// 다크로드 펫 스피릿 수정(2009.01.28)
//#define PJH_FIX_4_BUGFIX_7							// wops_32859 다크스크림 관련 버그 수정중 [패치 안됨]


// 4. 게임츄 웹스타터/ Season5-5 미니업데이트, 보석조합
// - 이은미
//#define LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
// #define LEM_DEBUG_TEST

// - 김재희
//#define KJH_FIX_SOURCE_ARRANGEMENT_MOVECOMMAND_WINDOW			// 이동명령창 소스코드 정리 (09.03.04)
//#define KJH_FIX_JP0467_RENDER_DARKLOAD_PET_OTHER_CHARATER		// 다른케릭터의 다크로드펫이 장착되거나 해제할때, 화면상 변화되지 않는 버그 수정 (09.06.03)
//#define KJH_ADD_INGAMESHOP_ITEM_TOOLTIP							// 인게임샵 아이템 툴팁 (09.11.11)
//#define KJH_MOD_NATION_LANGUAGE_REDEFINE					// 국가별 언어 Define 재정의 (10.04.26)

// - 이동석 
//#define LDS_MOD_BLOODWITCHQUEEN_EFFECT_ATTACK2			// 확장 몬스터맵 아이다의 블러디 위치퀸의 2번공격 효과를 기간틱에서 비광역 처리 하며 효과 변경. (10.04.28)
//#define LDS_FIX_ACCESS_INDEXNUMBER_ALREADY_LOADTEXTURE	// Bitmap::LoadTexture시에 비트맵 인덱스 중첩으로 기존에 이미 읽은 비트맵이 제거되버리는 문제 FIX. (10.03.30)
//#define LDS_ADD_TEST_UNITEDMARKETPLACE					// 통합시장 서버 테스트용 정의. (09.12.24)
//#define LDS_FIX_SETNULLALLOCVALUE_WHEN_BMDRELEASE			// BMD구조체 Release로 변수 NULL처리 시에, 초기화여부 변수도 NULL처리 (09.12.01)
//#define LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01	// LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT	// 통합시장, 공성전맵 맵이동시 간혹 키입력이 안되는 경우. 매우 간혹 LoadingWorld 수치를 ReceiveTeleport함수나 ReceiveJoinMapServer함수에서 변경을 해주지 않아 발생, 맵 변경시 호출되는 ReceiveChangeMapServerResult 함수에도 LoadingWorld 변경.(10.02.16)
//#define LDS_ADD_MULTISAMPLEANTIALIASING					// Anti Aliasing (MSAA) 추가 (10.03.22)

// - 이동근 
//#define LDK_MOD_NUMBERING_PETCREATE						// 테스트 (09.08.27)
//#define LDK_FIX_MOTION_INTI_AT_THE_LOADINGWORLD				// wops 34113 - 월드로딩시 모션 초기화(09.04.14)
//#define PBG_FIX_LODING_ACTION								// 로딩시에 발이 움직이지 않는(모션) 버그(09.05.25)
//#define LDK_ADD_SCALEFORM									// 스케일폼 작업(10.03.10)
#ifdef LDK_ADD_SCALEFORM
	// 6-2작업
	#define ADD_SELETED_LANGUAGE_FONT						// 국가별 폰트 설정(2010.09.14)
	#define MOD_RESOLUTION_BY_UI_RENDER_ITEM_RESIZING		// 해상도별 아이템 크기 변경(2010.10.19)

	//#define GFX_UI_TEST_CODE								// 작업중 테스트코드 묶음
#endif //LDK_ADD_SCALEFORM

// - 박보근
//#define PBG_FIX_ITEMNAMEINDEX								// 아이템 이름 인덱스 버그수정(10.06.16)
//#define PBG_MOD_SUPPLEMENTATION							// 소환술사절대무기,거래창,채팅창 보완작업(10.10.18)
//#define PBG_FIX_SETITEMTOOLTIP							// 세트 아이템 툴팁 버그수정(2010.10.26)
//#define PBG_FIX_CHAOSMAPMOVE								// 카캐에서 캐릭선택창 이동시 맵인덱스 버그 수정(2010.10.27)

// - 이주흥
//#define LJH_FIX_REARRANGE_INVISIBLE_CLOAK_LEVEL_FOR_CHECKING_REMAINING_TIME	// 투명망토(lv1~8)과 블러드캐슬(lv0~7)의 차이때문에 8레벨 투명망토 우클릭 할 때 시간이 이상하게 나오는 버그 수정(10.07.07) 

// - 김재우
//#define KJW_FIX_SOCKET_INFORMATION						// 변수 초기화 시점의 문제로 소켓정보가 믹스인벤토리에 표시되지 않는 문제점 수정 (10.9.16)
//#define KJW_ADD_NEWUI_SCROLLBAR							// 기본 스크롤바 UI추가(2010.10.28)
//#define KJW_ADD_NEWUI_TEXTBOX								// 기본 텍스트박스 UI추가(2010.10.28)
//#define KJW_ADD_NEWUI_GROUP								// 그룹 UI추가(2010.10.28)
//#define KJW_ADD_LOG_WHEN_FAIL_LODING_MODEL			// 모델로딩 실패시에 해당 모델에대한 정보 로그남기기

// - 명세종
// #define MSZ_ADD_CRASH_DUMP_UPLOAD							// Crash dump upload 기능 추가
// #ifdef FOR_WORK
// 	#define MSZ_ADD_EXCEPTION_TEST							// Crash dump test 기능 추가 (Num Lock 켜진 상태에서 Ctrl+숫자패드키)
// #endif // FOR_WORK


// - 곽준원
//#define KWAK_ADD_TRACE_FUNC								// Trace 함수 추가 -> 기존 OutPutDebugString 대체함
#ifdef _DEBUG
	//#define KWAK_FIX_ALT_KEYDOWN_MENU_BLOCK					// 알트키 누르면 포커싱 잃는 문제
#endif // _DEBUG
//#define KWAK_ADD_TYPE_TEMPLATE_INBOUNDS					// InBounds 템플릿 추가
//#define KWAK_ADD_DEFINE_UTIL							// 비트 연산 외 디파인 추가

#ifdef _VS2008PORTING
	//#define	KWAK_FIX_KEY_STATE_RUNTIME_ERR					// vs2008 키 인풋 런타임 에러 수정(2010.10.29)
	//#define KWAK_FIX_CHARACTER_SKILL_RUNTIME_ERR			// 캐릭터 스킬 아이콘 Rendering 런타임 에러 수정(2010.10.29)
	//#define KWAK_FIX_COMPILE_LEVEL4_WARNING					// vs2008 컴파일 옵션 Lv4의 워닝 수정(2010.11.1)
	//#define KWAK_FIX_COMPILE_LEVEL4_WARNING_EX				// vs2008 컴파일 옵션 Lv4의 워닝 수정(2010.11.4)
	//#define KWAK_FIX_COMPILE_SIGN_UNSIGN_DISCORD			// vs2008 컴파일 signed 또는 unsigned 불일치 수정(2010.11.8)

	// - 김재우
	//#define KJW_FIX_SOCKET_BONUS_BIT_OPERATION				// VS2008로 .빌드시 발생하는 비트연산 오류수정( int -> __int64 ) (10.9.16)
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//## [다음 패치때 들어갈 디파인]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// - 김재희
//#define KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL		// 인게임샵 선물하기에서 다크로드가 아닌 클래스도 선물하기 받을수 있는문제 수정 (10.06.23)
//#define KJH_MOD_BTS184_REQUIRE_STAT_WHEN_SPELL_SKILL		// 스킬 시전시 스텟 요구치에 따라 스킬 사용 변경 (10.05.17)

// - 박기동
//#define PKD_ADD_ENHANCED_ENCRYPTION					// 암호화 변경 및 소스 살리기. 모든 패킷 암호화 

// - 이주흥
//#define LJH_FIX_NO_MSG_FOR_FORCED_MOVEOUT_FROM_BATTLESOCCER		// 전투축구중 길드마스터가 아닌 자신이 강제로 경기중 경기장 밖으로 나갔을 때 결과 메세지 출력하지 않음(09.12.28) 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//## [패치이후 디파인]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2010.11.02 테섭] - 김재희

// - 박보근
#define PBG_MOD_NEWCHAR_MONK_WING_2						// 레이지/군주망토 옵션변경(2010.10.21)

// - 이은미
#define LEM_FIX_ITEMTOOLTIP_POS							// 아이템 툴팁 화면바같으로 나가는 문제 수정


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2010.10.27 테섭] - 김재희
// [2010.10.28 본섭] - 김재희
// [2010.10.28 블루] - 김재희

// - 박보근
#define PBG_MOD_GM_ABILITY								// 영어로 설정할수 있도록 변경(10.08.12)
#define PBG_FIX_BUFFSKILLCHAOS							// 버프스킬 카캐 대기시간에 사용가능 수정/풀세트이펙트버그수정(10.10.18)
#define PBG_FIX_EQUIP_TWOHANDSWORD						// 양손무기 왼쪽착용 버그 수정(2010.10.27)
 
// - 이은미
#define LEM_FIX_SELL_LUCKYITEM_BOOTS_POPUP				// 럭키아이템 판매시 부츠 팝업 수정 (2010.10.21)
#define LEM_FIX_LUCKYITEM_UICLASS_SAFEDELETE			// 럭키아이템 UI할당 해제 (2010.10.20)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2010.10.15 테섭] - 김재희
// [2010.10.21 본섭] - 김재희
// [2010.10.21 블루] - 김재희

// - 안상규
#define ASG_ADD_QUEST_REQUEST_NPC_SEARCH					// 퀘스트 요구사항 추가 (NPC찾기) (2010.10.14)

// - 김재희
#define KJH_FIX_PERSONALSHOP_BAN_CASHITEM					// 캐시아이템이 개인상점에 올라가는 문제 수정 (2010.10.13)
#define KJH_FIX_CHANGE_ITEM_PRICE_IN_PERSONAL_SHOP			// 개인상점에서 아이템가격 변경을 할수 없는 문제 수정 (2010.10.13)
#define KJH_FIX_SELL_LUCKYITEM								// 럭키아이템 상점에 판매시 팝업창뜨는 오류 수정 (2010.10.13)
#define KJH_MOD_STREAMPACKETENGINE_FILTER					// Stream Packet Engine 필터 변경 (2010.10.14)

// - 이주흥
#define LJH_FIX_NOT_POP_UP_HIGHVALUE_MSGBOX_FOR_BANNED_TO_TRADE	//NPC에게 판매 금지된 아이템을 판매하려고 할때 고가의 아이템이라고 나오는 메세지 창을 안나오게 수정(10.02.19)

// - 박보근
#define PBG_FIX_NEWCHAR_MONK_UNIANI						// 유니리아 착용시 애니메이션 속도 이상현상수정 (2010.10.13)
#define PBG_FIX_NOTSOUNDLOAD_GIANTMAMUD					// 라클리온 자이언트메머드 사운드 로딩안되는 버그수정 (2010.10.13)
#define PBG_MOD_NEWCHAR_MONK_WING						// 레이지망토 옵션 변경 (2010.10.13)

// - 이은미
#define LEM_FIX_LUCKYITEM_SLOTCHECK						// 럭키아이템 슬롯검사 수정 (2010.10.13)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2010.10.04 테섭] - 김재희

// - 박종훈
#define PJH_NEW_CHROME								// 럭키아이템 이펙트 보여주기.(10.08.30)

// - 이은미	
#define LEM_ADD_LUCKYITEM							// 럭키아이템 


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// <시즌6 - Part 1> 이후 버그 수정
//----------------------------------------------------------------------------------------------

// [2010.09.17 테섭] - 이주흥
// - 박보근
#define PBG_MOD_RAGEFIGHTERSOUND						// 신규캐릭터 사운드변경(10.09.13)

#ifdef _TEST_SERVER
	// - 이동근
	// LDK_ADD_SCALEFORM 에 따른 주석처리(같이 주석처리하거나 같이 풀어줘야 한다)
	//#define MOD_INGAMESHOP_HEIGHT_MODIFY					// 인게임샵 height 수정(2010.09.15)
#endif //_TEST_SERVER

#ifdef _VS2008PORTING
	#define ASG_FIX_MOVECMD_WIN_SCRBAR						// 이동명령창 스크롤바 제일하단 배경 안나오는 버그(10.09.15)
	#define LJH_FIX_RUNTIME_ERROR_WHEN_RENDERING_DROPPED_ITEMNAME	// 바닥에 떨어진 아이템의 이름을 렌더링할때 런타임에러 수정(10.09.15)
#endif // _VS2008PORTING

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// <시즌6 - Part 1>
//----------------------------------------------------------------------------------------------
// [2010.09.14 테섭] - 이주흥
// - 박기동
#define PKD_ADD_BINARY_PROTECTION							// 더미다의 VM_START 적용

// - 안상규
#define ASG_ADD_TIME_LIMIT_QUEST							// 시간제 퀘스트 작업(10.06.04)
#define ASG_ADD_MAP_KARUTAN									// 칼루탄 맵 추가(10.07.06)
	
// - 김재희
#define KJH_FIX_MINIMAP_NAME								// 미니맵에서 NPC/Portal Name 이 짤리는 버그 수정 (10.05.14) - minimap 스크립트 모두 재컨버팅

// - 박보근
#define PBG_ADD_NEWCHAR_MONK								// 신규캐릭터(10.03.08)

// - 안상규
#ifdef ASG_ADD_TIME_LIMIT_QUEST
	#define ASG_ADD_TIME_LIMIT_QUEST_NPC					// 시간제 퀘스트 NPC(10.06.04)
	#define ASG_ADD_TIME_LIMIT_QUEST_ITEM					// 시간제 퀘스트 아이템(10.06.16)
	
#endif	// ASG_ADD_TIME_LIMIT_QUEST

#ifdef ASG_ADD_MAP_KARUTAN
	#define ASG_ADD_KARUTAN_MONSTERS						// 칼루탄 몬스터 추가(10.07.06)
	#define ASG_ADD_KARUTAN_NPC								// 칼루탄 NPC 추가(10.08.12)

// 블루,본섭패치시 삭제해야함	
// 	#ifdef _TEST_SERVER
// 		#define ASG_ADD_AIDA_KARUTAN_GATE_FOR_TESTSERVER	// 아이다맵에 칼루탄 입구 추가(본섭 패치시 삭제)(10.09.06)
// 	#endif // _TEST_SERVER
#endif	// ASG_ADD_MAP_KARUTAN

#define ASG_FIX_MONSTER_MAX_COUNT_1024						// #define MAX_MONSTER 512 -> 1204로 수정(10.06.29)

// - 박보근
#ifdef PBG_ADD_NEWCHAR_MONK
	#define PBG_ADD_NEWCHAR_MONK_ITEM						// 아이템 관련(10.04.01)
	#define PBG_ADD_NEWCHAR_MONK_SKILL						// 스킬 관련(10.04.01)
	#define PBG_ADD_NEWCHAR_MONK_ANI						// 신규캐릭터 애니메이션 변경(10.05.12)
#endif //PBG_ADD_NEWCHAR_MONK

#ifdef _TEST_SERVER
	// - 이동근
	//#define LDK_ADD_SCALEFORM									// 스케일폼 작업(10.03.10) [본섭 시즌 6-1 포함 안됨]
	#ifdef LDK_ADD_SCALEFORM
		#define ADD_GFX_REG_OPTION								// 구UI와 GFxUI 선택값 리지스트리에서 확인

		#define MOD_GFX_MAINUI_ITEM								// item 단축키 설정
		#define MOD_GFX_MAINUI_SKILL							// skill 단축키 설정
		#define MOD_STENCILBUFFER								// stencil buffer를 설정한다(마스크처리시 사용)
		#define MOD_MAINSCENE_HEIGHT							// mainScene 에서 height값 변경

		// ui
		#define MOD_SKILLLIST_UPDATEMOUSE_BLOCK					// 스킬리스트 업데이트마우스함수 사용 불가(2010.09.12)
		#define MOD_CHATINPOTBOX_POSITION						// 채팅창 위치 수정
		#define MOD_DOPPELGANGERFRAME_POSITION					// 도플갱어 진행 UI 위치 수정(2010.09.07)
		#define MOD_HERO_POS_INFO_DEPTH							// 좌표 정보 뎁스 수정(2010.09.07)
		//#define MOD_INTERFACE_CAMERAWORK						// 인터페이스창 열때 카메라 이동방지(2010.09.03) <-- 인벤토리 버그발생으로 주석처리중

		#define MOD_UI_DISABLE_MAINFRAME						// mainframe 사용안함
		#ifdef MOD_UI_DISABLE_MAINFRAME
			#define MOD_MOUSE_Y_CLICK_AREA						// 하단 마우스 클릭 영역 수정
		#endif //MOD_UI_DISABLE_MAINFRAME
	#endif //LDK_ADD_SCALEFORM
#endif //_TEST_SERVER

#ifdef _VS2008PORTING	// VS2008 포팅 작업중 버그수정
	#define KJH_FIX_ITEMHOTKEYINFO_CASTING					// 아이템 핫키정보 하위캐스팅 문제	
	#define KJH_FIX_TALK_SANTATOWN_NPC						// 산타마을에서 NPC와 대화하지 못하는 버그 수정
	#define KJH_FIX_CONTRIBUTIONPOINT_CASTING				// 기여도포인트 하위캐스팅 
	#define KJH_FIX_SEND_REQUEST_INVENTORY_ITEMINFO_CASTING	// 아이템정보 전송요청중 아이템정보 하위 캐스팅문제
	#define LJH_FIX_DEREFERENCE_VECTOR_ITER_FOR_DELETED_UI_OBJ		// 벡터 CNewUIObj값을 삭제한 후 iterator를 역참조함으로써 생기는 버그 수정(10.09.03)
#endif //_VS2008PORTING

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2010.09.09 본섭 1.07Q+] - 이주흥
// [2010.09.09 블루 1.00K+] - 이주흥
// - 김재희
#ifdef _ORIGINAL_SERVER
	#define KJH_ADD_EVENT_LOADING_SCENE			//!! 이벤트관련 로딩씬 추가(10.09.09) - 레드 이벤트 끝나는 이후 주석처리와 원래 리소스 적용 (10.09.09 ~ 10.09.30)
#endif // _ORIGINAL_SERVER

#ifdef _BLUE_SERVER
	#define KJH_ADD_EVENT_LOADING_SCENE			//!! 이벤트관련 로딩씬 추가(10.09.09) - 블루 이벤트 끝나는 이후 주석처리와 원래 리소스 적용 (10.09.09 ~ 10.09.30)
#endif // _BLUE_SERVER

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2010.08.23 테섭 1.07O+] - 이주흥
// [2010.08.26 본섭 1.07P+] - 이주흥
// [2010.08.26 블루 1.00J+] - 이주흥
// - 이주흥
#ifndef _BLUE_SERVER		// ifndef
	#define LJH_MOD_CANNOT_USE_CHARMITEM_AND_CHAOSCHARMITEM_SIMULTANEOUSLY	// (레드만)행운의부적과 카오스조합부적 동시에 사용 못하도록 기획변경(10.08.19)
#endif //_BLUE_SERVER

// 레드, 블루 이벤트 종료 아래 define 주석처리(10.08.23)
//		LDK_MOD_GAMBLE_SUMMER_SALE_EVENT 
//		KJH_ADD_EVENT_LOADING_SCENE

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2010.08.12 본섭 1.07N+] - 이주흥
// [2010.08.12 블루 1.00I+] - 이주흥

// - 이동근
// #ifdef _BLUE_SERVER									
// 	#define LDK_MOD_GAMBLE_SUMMER_SALE_EVENT		// 블루만 - 겜블 여름 이벤트 50% 세일 (10.08.11) - 이벤트가 끝나 주석처리함(10.08.23)
// #endif //_BLUE_SERVER	

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// < 버그 수정 >
//----------------------------------------------------------------------------------------------
// [2010.07.06 테섭 1.07I+] - 박보근
// [2010.07.08 본섭 1.07J+] - 김재희
// [2010.07.08 블루 1.00F+] - 김재희

// - 박보근
#define PBG_FIX_CHARACTERSELECTINDEX					// 캐릭터 인덱스 버그수정(10.07.05)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// < 버그 수정 >
//----------------------------------------------------------------------------------------------
// [2010.07.01 테섭 1.07H+] - 김재희
// [2010.07.08 본섭 1.07J+] - 김재희
// [2010.07.08 블루 1.00F+] - 김재희

// - 박보근
#define PBG_FIX_DROPBAN_GENS							// 버리기 아이템 수정

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// < 버그 수정 >
//----------------------------------------------------------------------------------------------
// [2010.06.30 테섭 1.07F+] - 김재희
// [2010.07.01 본섭 1.07G+] - 김재희
// [2010.07.01 블루 1.00E+] - 김재희

// - 김재희
#define KJH_MOD_INGAMESHOP_DOMAIN_EACH_NATION					// 국가별 게임샵 도메인 설정 (10.05.04)
#define KJH_FIX_BTS158_TEXT_CUT_ROUTINE							// UI에 맞게 Text를 자를시 이상현상 수정 (10.05.07)
#define KJH_MOD_BTS173_INGAMESHOP_ITEM_STORAGE_PAGE_ZERO_PAGE	// 인게임샵 보관함 아이템이 없을시 0페이지 출력 (10.05.13)
#define KJH_FIX_BTS179_INGAMESHOP_STORAGE_UPDATE_WHEN_ITEM_BUY	// 인게임샵 아이템 구입시 선물함 탭에서 보관함 아이템이 갱신되는 버그수정 (10.05.13)
#define KJH_FIX_BTS167_MOVE_NPC_IN_VIEWPORT						// 뷰포트안에 NPC가 들어오면 NPC가 갑자기 이동하는 버그수정 (10.05.17)
#define KJH_MOD_BTS191_GOLD_FLOATING_NUMBER						// 아이템이나 캐시 금액을, 소숫점자리까지 계산 (10.05.18)
#define KJH_FIX_BTS204_INGAMESHOP_ITEM_STORAGE_SELECT_LINE		// 인게임샵 보관함에서 1번째 아이템 사용시, 포커싱이 3번째 아이템으로 선택되는 버그수정 (10.05.18)
#define KJH_FIX_BTS206_INGAMESHOP_SEND_GIFT_MSGBOX_BLANK_ID		// 인게임샵 선물하기 메세지창에서 ID입력하지 않고 확인 버튼 클릭시, 선물확인 메세지창이 뜨는 버그 (10.05.18)
#define KJH_MOD_COMMON_MSG_BOX_BTN_DISABLE_TEXT_COLOR			// 공통 메세지박스 버튼 Disable시 글자색 수정 (10.05.18)
#define KJH_MOD_BTS208_CANNOT_MOVE_TO_VULCANUS_IN_NONPVP_SERVER	// nonPVP 서버일대 불카누스로 이동불가 (10.05.18) - 서버동시수정
#define KJH_FIX_BTS207_INGAMESHOP_SEND_GIFT_ERRORMSG_INCORRECT_ID	// 인게임샵 선물하기에서 상대방 아이디가 틀렸을때 에러 메세지 (10.05.19) - 서버동시수정
#define KJH_FIX_BTS251_ELITE_SD_POTION_TOOLTIP					// 엘리트 SD물약 툴팁이 안나오는 버그 수정 (10.05.24)
#define KJH_FIX_BTS260_PERIOD_ITEM_INFO_TOOLTIP					// 기간제아이템 기간제 정보 툴팁 버그 수정 (10.05.24) - Text.txt 수정요망
#define KJH_FIX_BTS295_DONT_EXPIRED_WIZARD_RING_RENDER_SELL_PRICE	// 기간제 마법사반지가 기간이 만료되지 않았는데 판매가격이 나오는 버그 수정 (10.05.25)
#define KJH_MOD_INGAMESHOP_GIFT_FLAG							// 인게임샵 선물하기 Flag로 선물하기 버튼 On/Off (10.05.25)

// - 안상규
#define ASG_FIX_QUEST_GIVE_UP									// 퀘스트 포기 버그 수정. 퀘스트 상태(QS) 0xfe삭제됨.(10.05.26)

// - 이주흥
#define LJH_MOD_EXTENDING_NUM_OF_MIX_RECIPES_FROM_100_TO_1000	// 한 조합 창에서 조합할수 있는 종류의 개수 제한을 100개에서 1000개로 증가(10.02.23)
#define LJH_FIX_LOADING_INFO_OF_AN_OTHER_CHARACTER_IN_LOGIN		// 캐릭터 선택화면에서 캐릭터 B를 선택하고 캐릭터 A를 클릭하면서 엔터키를 동시에 누르면 캐릭터 B의 일부정보로 로그인(10.04.01)
#define LJH_FIX_BUG_DISPLAYING_NULL_TITLED_QUEST_LIST			// 서버에서 받은 퀘스트 인덱스가 존재하지 않아 (null)로 표시되는 항목은 퀘스트 목록을 만들때 넣지 않도록 수정(10.04.14)
#define LJH_FIX_BUG_SELLING_ITEM_CAUSING_OVER_LIMIT_OF_ZEN		// 소지한도액을 초과하는 판매액의 아이템을 팔지 못하게 서버로 메세지도 보내지 않도록 수정(10.04.15)				
#define LJH_ADD_SAVEOPTION_WHILE_MOVING_FROM_OR_TO_DIFF_SERVER	// 다른 서버(현재 공성서버, 로렌시장)에서부터 또는 다른 서버로 이동할 때 옵션이 저장 되지 않는 버그 수정(10.04.16)
#define LJH_FIX_NO_EFFECT_ON_WEAPONS_IN_SAFE_ZONE_OF_CURSED_TEMPLE	// 환영사원의 안전지역에서 15레벨 아이템 이펙트 나오지 않는 버그 수정(10.04.16)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// < 버그 수정 >
//----------------------------------------------------------------------------------------------
// [2010.06.09 테섭 1.07D+] - 김재희
// [2010.06.10 본섭 1.07E+] - 김재희
// [2010.06.10 블루 1.00D+] - 김재희

#define KJH_ADD_NPC_ROURKE_PCROOM_EVENT_MESSAGEBOX					// PC방 이벤트(도우미루크) 메세지박스 출력 (10.06.08)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2010.05.12 테섭 1.07A+] - 안상규
// [2010.05.13 본섭 1.07B+] - 안상규
// [2010.05.13 블루 1.00A+] - 안상규

// - 이주흥
#define LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT			// 골든오크참 착용효과수치 기획 변경(HP +100, SD+500) (10.05.06)
#define LJH_FIX_GETTING_ZEN_WITH_PET_OF_OTHER_PLAYER	// 다른 플레이어의 펫이 젠을 먹어주는 버그 수정(10.05.10)

// - 박보근
#define PBG_FIX_CURSEDTEMPLE_SYSTEMMSG					//환영의사원 메시지 처리 버그 수정(09.10.22)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2010.05.03 테섭 1.07y] - 안상규
// [2010.05.06 본섭 1.07z] - 안상규
// [2010.05.06 블루 1.00z] - 안상규

// - 김재희
#ifdef _BLUE_SERVER								// #ifndef	레드 본섭만 제외	(차후에 블루용만 들어감)
	#define KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL			// 공격시 시리얼넘버 생성 (10.04.19)
#endif // _BLUE_SERVER

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2010.04.29 본섭 1.07x] - 김재희
// [2010.04.29 블루 1.00y] - 김재희

// - 김재희
#define KJH_MOD_INGAMESHOP_UNITTTYPE_FILED_OF_PRODUCT_SCRIPT	// Product 게임샵 스크립트의 UnitType필드로 단위명 변경 (10.04.22) - 인게임샵 라이브러리 적용(10.03.29일자)
#define KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT				// 인게임샵 보관함 페이지 단위 방식으로 변경 (10.04.27)

// - 이주흥
#define LJH_FIX_APP_SHUTDOWN_WEQUIPPING_INVENITEM_WITH_CLICKING_MOUSELBTN // 마우스 왼쪽버튼을 누른채로 인벤아이템을 장착하려고 할때 프로그램 튕기는 버그 수정(10.04.29)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2010.04.27 테섭] - 김재희
// [2010.04.29 본섭] - 김재희
// [2010.04.29 블루] - 김재희

// #ifdef _TEST_SERVER
// 	#define KJH_FIX_ICARUS_MAP_ATTRIBUTE_FOR_TESTSERVER			// 이카루스 맵 속성 변경(10.04.27) - 본서버 적용시 주석처리
// #endif // _TEST_SERVER

#ifdef _BLUE_SERVER
	#define KJH_MOD_ENTER_EVENTMAP_ERROR_MSG					// 이벤트맵 입장 에러메세지 추가 (10.04.26)
#endif // _BLUE_SERVER


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// <시즌5 - Part 4>
//----------------------------------------------------------------------------------------------
// [2010.04.21 테섭] - 김재희
// [2010.04.29 본섭] - 김재희
// [2010.04.29 블루] - 김재희

// - 박종훈
#define PJH_ADD_MASTERSKILL								//소환술사 마스터레벨 추가(2010.03.09)

// - 안상규
#define ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND				// 칸투르 폐허3 분쟁지역화.(10.04.13)

// - 김재희
#define KJH_FIX_POTION_PRICE							// 상점 물약가격 버그수정 (10.04.16)
#define KJH_FIX_INIT_EVENT_MAP_AT_ABNORMAL_EXIT			// 비정상적적인 종료시 이벤트맵 초기화 (10.04.20)
// #ifdef _TEST_SERVER
// 	#define KJH_EXTENSIONMAP_FOR_TESTSERVER				// 확장맵 테섭용(본섭때는 빼야함) (10.04.20)
// #endif // _TEST_SERVER

// - 박보근
#define PBG_FIX_SATAN_VALUEINCREASE						// 사탄 착용시 수치 증가 버그 수정(10.04.07)
#define PBG_MOD_GUARDCHARMTEXT							// 수호의부적아템 문구수정(10.04.14)

// - 이동근, 이동석
#define LDK_LDS_EXTENSIONMAP_HUNTERZONE					// 사냥터 확장 맵 (기존 맵 리뉴얼 및 몬스터, 보스급 몬스터 추가.). (10.02.18)

// - 이동근
#define LDK_MOD_GUARDIAN_DROP_RESIZE					// 데몬 수호정령 바닥에 놓였을때 사이즈 조정(10.03.04)

// - 이동석
#ifdef _BLUE_SERVER
	#define LDS_MOD_PKSYSTEM_INGAMESHOP_ATTACK_PKLEVEL1		// 블루 서버에서만 무법자 "경고" 케릭터들 대상 crtl없이 공격가능했던것을 기획적으로 레드와 동일하게 변경하여 무법자 "경고"도 일반케릭과 동일 공격 되게.  (10.04.14)
#endif // _BLUE_SERVER

// - 이주흥
#define LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY		// 인벤장착 아이템을 위한 시스템 추가(10.02.22) 
#define CSK_ADD_GOLDCORPS_EVENT								// 황금군단이벤트 추가(일본:2009.03.13, 국내:10.04.15)	
#ifdef _BLUE_SERVER
	#define LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER	// 무법자(카오) 플레이어와 파티를 맺은 플레이어에게도 맵 이동시 요구 젠 10배로 증가(10.04.13)
#endif // _BLUE_SERVER

#ifdef LDK_LDS_EXTENSIONMAP_HUNTERZONE						
	// - 이동석
	// 일반몹들 지역별
	#define LDS_EXTENSIONMAP_MONSTERS_RAKLION				// 확장 맵내의 일반 몹들. 라클리온 (10.02.18)
	#define LDS_EXTENSIONMAP_MONSTERS_AIDA					// 확장 맵내의 일반 몹들. 아이다 (10.02.26)
	#define LDS_EXTENSIONMAP_MONSTERS_KANTUR				// 확장 맵내의 일반 몹들. 칸투르 (10.03.02)
	#define LDS_EXTENSIONMAP_MONSTERS_SWAMPOFQUIET			// 확장 맵내의 일반 몹들. 평원의늪 (10.03.02)

	// 추가 작업
	#define LDS_MOD_EXTEND_MAXMODELLIMITS_200TO400			// 기존 몬스터모델 갯수 제한 (MAX_MODEL_MONSTER=200)개를 초과하여 오류 발생으로 (MAX_MODEL_MONSTER=400)설정. (10.03.15)

	// - 이동근
	// 보스몹 메듀사
	#define LDK_ADD_EXTENSIONMAP_BOSS_MEDUSA				// 평온의늪 보스 몬스터 메듀사 (10.02.19)
	#define LDK_ADD_EXTENSIONMAP_BOSS_MEDUSA_EYE_EFFECT		// 평온의늪 보스 몬스터 메듀사 눈 잔상 이펙트(10.03.03)
	#define LDK_ADD_EXTENSIONMAP_BOSS_MEDUSA_WAND_EFFECT	// 평온의늪 보스 몬스터 메듀사 지팡이 이펙트(10.03.04)
	#define LDK_MOD_EVERY_USE_SKILL_CAOTIC					// 카오틱 디세이어 몬스터 사용 가능하도록 수정(10.03.08)

	// 맵 작업
	#define LDK_FIX_AIDA_OBJ41_UV_FIX							// 아이다맵 41번 obj(빛) uv 값 고정(10.03.16)
	#define LDK_ADD_AIDA_OBJ77_OBJ78							// 아이다맵 77,78번 obj uv흐름 추가(10.03.18)
#endif // LDK_LDS_EXTENSIONMAP_HUNTERZONE

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY	
	#define LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM		// 인벤장착 아이템 추가-매조각상, 양조각상, 편자 (10.02.26)
	#define LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2	// 추가인벤장착 아이템 추가-오크참, 골든오크참, 메이플참, 골드메이플참 (10.03.24)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY

#ifdef CSK_ADD_GOLDCORPS_EVENT					
	#define LDS_ADD_GOLDCORPS_EVENT_MOD_GREATDRAGON				// 황금 그레이트 드레곤 비주얼 재작업. 비주얼 이펙트 추가. 기존 동일한 몹 존재. (2009.04.06) (BITMAP_FIRE_HIK3_MONO 효과 참조.)
	#define KJH_FIX_GOLD_RABBIT_INDEX							// 황금토끼 인덱스 수정 (09.06.08)
#endif //CSK_ADD_GOLDCORPS_EVENT

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2010.03.31 테섭] - 김재희
//[2010.04.01 본섭] - 김재희
//[2010.04.01 블루] - 김재희

// - 이동석
#define LDS_ADD_OUTPUTERRORLOG_WHEN_RECEIVEREFRESHPERSONALSHOPITEM	// 로렌 시장 서버로부터 개인상점 구매이후 ReceiveRefreshPersonalShopItem 받을 때 Error Log 정보를 더 자세히 출력 (10.03.30)
#define LDS_FIX_MEMORYLEAK_WHERE_NEWUI_DEINITIALIZE		// 로렌시장 DEAD현상 : NewUI 내 폐기 처리 메모리 미반환으로 인한 메모리 누수 작업 (10.03.23)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2010.03.23 테섭] - 김재희
//[2010.03.25 본섭] - 김재희
//[2010.03.25 블루] - 김재희

// - 안상규
#define ASG_MOD_GM_VIEW_NAME_IN_GENS_STRIFE_MAP			// 겐스 분쟁지역에서 GM은 타세력 캐릭터 이름 보이게(10.02.26)

// - 김재희
#ifdef _BLUE_SERVER						// #ifndef	레드 본섭만 제외	(차후에 블루용만 들어감)
	#define KJH_FIX_DUMMY_SKILL_PROTOCOL_BUG				// 더미 스킬 프로토콜 버그수정 (10.03.18)
#endif // _BLUE_SERVER

#define KJH_FIX_MOVE_MAP_GENERATE_KEY				// 맵 이동(&더미스킬 프로토콜)시 생성되는 키값 수정


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2010.03.17 테섭] - 안상규
//[2010.03.18 블루] - 안상규
//[2010.03.25 본섭] - 김재희


// - 양대근
#ifdef _TEST_SERVER
	//#define CHAOSCASTLE_MAP_FOR_TESTSERVER			// 테섭 카오스캐슬 맵 버그 수정을 위해 test붙인 맵 로딩하도록 함(본섭때는 빼야됨)(2010.3.2)
#endif // _TEST_SERVER

// - 김재희
#ifdef _BLUE_SERVER						// #ifndef	레드 본섭만 제외	(차후에 블루용만 들어감)
	#define KJH_ADD_DUMMY_SKILL_PROTOCOL				// 더미 스킬 프로토콜(10.02.26)
#endif // _BLUE_SERVER

// - 박보근
#ifdef _BLUE_SERVER
	#define PBG_MOD_SECRETITEM								// (활력의비약)인벤사용 가능 아이템으로 변경위함(10.03.09)
#endif //_BLUE_SERVER

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2010.02.25 블루]- 이동근
//[2010.02.25 본섭]- 이동근

// 업데이트 런처 광고 url 변경

//[2010.02.23 테섭]- 이동근

// mix.txt 수정 - 믹스 스크립트 2차날개 계산시 1% 오차생김

// - 안상규
#define ASG_FIX_GENS_STRIFE_FRIEND_NAME_BLOCK			// 겐스 분쟁지역에서 친구 이름 자동입력 막기(2010.02.03)
#define ASG_MOD_GENS_STRIFE_ADD_PARTY_MSG				// 겐스 분쟁지역 관련 파티 메시지 추가.(2010.02.03)

// - 박보근
#define PBG_FIX_RENDEROBJ_MAXINDEX						// 월드 로딩시 최대 오브젝외의 값 호출버그 수정(10.02.10)
#define PBG_MOD_STRIFE_GENSMARKRENDER					// 분쟁지역유저표시 변경(10.02.18)

// - 이동석
#define LDS_FIX_MEMORYLEAK_0910_LOGINWIN						// 메모리릭 FIXED. Logwin.cpp의 아이디, 패스워드 context가 재로그인 이나 서버선택등으로 재호출시에 메모리 미반환으로 릭발생. (09.11.30)
#define LDS_FIX_MEMORYLEAK_DUPLICATEDITEMS_INVENTORY_SAMEPOS	// 메모리릭 FIXED. 인벤토리상에 아이템을 집은후 같은 위치에 놓을때만 횟수 만큼 메모리 릭 발생. (09.11.09)
#define LDS_FIX_MEMORYLEAK_WHEN_MATCHEVENT_TERMINATE			// 매치이벤트(도플갱어,블러드캐슬,카오스캐슬,환영의사원) 종료시마다 메모리릭 발생 FIX. (10.01.29)
#define LDS_FIX_SETITEM_OUTPUTOPTION_WHICH_LOCATED_INVENTORY	// 착용된 세트아이템과 동종의 미장착된 세트아이템이 인벤토리에 있는경우, 옵션 활성화가 되는 문제. (10.02.17)

// - 양대근
#define YDG_MOD_PROTECT_AUTO_FIND_USB						// 아이로봇 오토 막기 USB 검사로 찾기 (2009.04.07)
#define YDG_FIX_MOVE_ICARUS_EQUIPED_SKELETON_CHANGE_RING	// 스켈레톤변신반지 착용후 이카루스로 이동할 수 없는 현상 (10.02.16)

// - 김재희
// world19 카오스 캐슬 맵 수정 : gamedata\@104
#define KJH_MOD_ATTACK_PROTOCOL_FOR_PROTECT_HACK			// 핵을 막기위한 공격 프로토콜 변경(10.02.11)
#ifdef YDG_MOD_PROTECT_AUTO_FIND_USB
	#define KJH_MOD_PROTECT_AUTO_FIND_USB					// 오토막기 - USB검사로 로그남기기 (10.01.15)
#endif // YDG_MOD_PROTECT_AUTO_FIND_USB

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2010.02.11 블루]- 이동근
//[2010.02.11 본섭]- 이동근
//[2010.02.09 테섭]- 이동근

// - 안상규
#define ASG_ADD_LEAP_OF_CONTROL_TOOLTIP_TEXT			// 통솔의 비약 툴팁 텍스트 추가(2010.01.28)

// - 박보근
#define PBG_FIX_GAMECENSORSHIP_1215						// 12/15세 이용가능 마크 출현 버그수정(10.02.09)

// - 이동석
#define LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT	// 통합시장 이동 시에 사용자의 키 및 마우스 인풋 입력을 막지 않아 생기는 문제로 일괄 Skip 처리. (10.02.02) 

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2010.02.05 테섭]- 박보근

// mix/text 스크립트 변경패치

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2010.02.04 본섭]- 박보근
//[2010.02.04 블루]- 박보근

// - 박보근
#define PBG_MOD_GAMECENSORSHIP_RED							// 레드의 경우도 18이상가능가로 조정(09.01.26)[10.02.04본섭패치시 테섭/본섭 블루 동시 적용]

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2010.02.02 테섭]- 박보근

// - 이동석
#define LDS_FIX_DISABLEALLKEYEVENT_WHENMAPLOADING			// 맵이동 요청이후 서버로부터 맵이동 결과가 오기전까지 모든 키입력을 막습니다. (로렌시장, 공성 서버 이동 요청시에 esc로 옵션창 "게임종료"호출이후 모든 키입력이 안되는 버그 ). (10.01.28)

// - 박보근
#define PBG_FIX_GENSREWARDNOTREG							// 겐스 미가입자가 보상받을시 문구 수정(10.01.28)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2010.01.28 테섭]- 박보근

// - 안상규
#define ASG_ADD_QUEST_REQUEST_REWARD_TYPE				// 퀘스트 요구사항, 보상 추가(2009.12.15)

// - 이동근
#define LDK_ADD_ITEM_UPGRADE_14_15
#ifdef LDK_ADD_ITEM_UPGRADE_14_15
	#define LDK_ADD_14_15_GRADE_ITEM_HELP_INFO				// 아이템 14 15 도움말 추가(09.12.2)
	#define LDK_ADD_14_15_GRADE_ITEM_VALUE					// 아이템 14 15 판매금액 추가(09.12.2)
	#define LDK_ADD_14_15_GRADE_ITEM_RENDERING				// 아이템 14 15 랜더효과 추가(10.01.04) 기존의 13처럼 크롬,블렌드 처리
	#define LDK_ADD_14_15_GRADE_ITEM_SET_EFFECT				// 아이템 14 15 세트이펙트효과 추가(10.01.22)
	#define LDK_ADD_14_15_GRADE_ITEM_TYPE_CHANGE			// 아이템 14 15 래벨 타입 변경(10.01.27)
	#define LDK_ADD_14_15_GRADE_ITEM_MODEL					// 아이템 14 15 모델링 추가(10.01.18)
	#define YDG_FIX_ITEM_EFFECT_POSITION_ERROR				// 아이템 이펙트 위치가 이상하게 나오는 문제 (2010.01.27)
#endif //LDK_ADD_ITEM_UPGRADE_14_15

// - 이동석
#define LDS_ADD_MAP_UNITEDMARKETPLACE					// 통합 시장 서버 (로렌 시장) 추가 (09.12.04)
#ifdef LDS_ADD_MAP_UNITEDMARKETPLACE
	#define LDS_ADD_NPC_UNITEDMARKETPLACE					// NPC 로렌시장 1.보석류 조합 및 해체 라울, 2.시장 상인 줄리아, 3.물약 및 기타 잡화 아이템 상인 크리스틴 (09.12.15)
	#define LDS_ADD_UI_UNITEDMARKETPLACE					// UI 로렌시장 메세지 처리 1.카오스케슬 입장 불가 2.결투신청불가 3.파티신청 불가 4.
	#define LDS_ADD_SERVERPROCESSING_UNITEDMARKETPLACE		// 서버와의 교신 처리 1.통합시장맵으로 이동 2. (09.12.23)
	#define LDS_ADD_SOUND_UNITEDMARKETPLACE					// 통합시장 환경음, 배경음 처리. (10.01.12)
	#define LDS_ADD_MOVEMAP_UNITEDMARKETPLACE				// 맵이동 윈도우에 "통합 시장" 추가. (10.01.12)
	#define LDS_ADD_EFFECT_UNITEDMARKETPLACE				// 통합시장 NPC 등에 추가 EFFECT. (10.01.14)
	#define LDK_FIX_CHARACTER_UNLIVE_MOUSE_LOCK				// 맵이동시 캐릭터 삭제후 이동 불가(09.10.29)
#endif // LDS_ADD_MAP_UNITEDMARKETPLACE

// - 박보근
#define PBG_ADD_GENSRANKING								// 겐스 랭킹(09.12.01)

// - 양대근
#define YDG_MOD_SKELETON_NOTSELLING						// 스켈레톤 펫 반지 안팔리게 설정(2010.01.20)
#define YDG_FIX_CHANGERING_STATUS_EXPIRE_CHECK			// 변신반지 스탯올릴때 만료 체크 (2010.01.25)
#define YDG_FIX_SKELETON_PET_CREATE_POSITION			// 스켈레톤 펫 생성 위치 수정 (2010.01.26)

// - 김재희
#define KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG2				// 인게임샵 스크립트 다운로드 버그수정 2 (10.01.14)
#define KJH_ADD_SKILLICON_RENEWAL						// 스킬아이콘 리뉴얼(10.01.20)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2010.01.20 테섭]- 박보근
//[2010.01.21 본섭]- 박보근
//[2010.01.21 블루]- 박보근

// - 양대근
#define YDG_ADD_CASHITEM_201001						// 2010년 1월 유료화 아이템 (2009.12.03)
#ifdef YDG_ADD_CASHITEM_201001
	#define YDG_ADD_CS5_PORTAL_CHARM					// 이동의 부적
	#define YDG_FIX_USED_PORTAL_CHARM_STORE_BAN			// 이동의 부적 사용하면 창고 저장 못하게 수정 (09.09.02)
	#define ASG_ADD_CS6_ITEM_GUARD_CHARM				// 아이템보호부적(2009.02.09)
	#define YDG_ADD_HEALING_SCROLL						// 치유의 스크롤 (2009.12.03)
	#define YDG_ADD_SKELETON_CHANGE_RING				// 스켈레톤 변신반지 (2009.12.03)
	#define YDG_ADD_SKELETON_PET						// 스켈레톤 펫 (2009.12.03)
	#define YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT			// 변신반지 한종류만 장비하도록 수정 (2009.12.28)
#endif	// YDG_ADD_CASHITEM_201001

// - 박보근
#define PBG_MOD_PREMIUMITEM_TRADE_0118						// 카오스 카드,부적,행운의부적 레드블루 거래가능으로 기획변경(10.01.18)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2010.01.12 테섭]- 박보근
//[2010.01.14 본섭]- 박보근
//[2010.01.14 블루]- 박보근

// - 박보근
#define PBG_MOD_PREMIUMITEM_TRADE_ENDURANCE					// 프리미엄아템중 기간제와내구력에 따른 기획변경 레드 적용(09.12.16)
#define PBG_MOD_PANDAPETRING_NOTSELLING						// 펜더 펫 반지 안팔리게 설정(10.01.11)

// - 이동근 
#define LDK_MOD_BUFF_END_LOG								// 버프 종료시 로그 추가(09.12.15)

// - 이주흥
#ifdef _BLUE_SERVER
	#define LJH_FIX_EXTENDING_OVER_MAX_TIME_4S_To_10S			// 해외사용자들 일부 사용자에게 캐시샵이 열리지 않아 Max Time을 기존 4초에서 10초로 변경(09.12.17)
#endif// _BLUE_SERVER

// - 김재희
#define KJH_MOD_SHOPSCRIPT_DOWNLOAD_DOMAIN					// 샵 스크립트 도메인 변경 (10.01.10)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2010.01.06 테섭]- 박보근
//[2010.01.07 본섭]- 박보근
//[2010.01.07 블루]- 박보근

// - 김재희
#define KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG						// 인게임샵 스크립트 다운로드 버그 (09.12.28)

// - 박종훈
#define PJH_FIX_HELP_LAYER										// 헬프 레이어 최상위로 변경(2009.12.01)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2009.12.15 테섭]- 김재희
//[2009.12.17 본섭]- 김재희
//[2009.12.17 블루]- 김재희

// - 이동근
#define LDK_FIX_EXPIREDPERIOD_ITEM_EQUIP_WARNING			// 만료된 아이템 작용불가 및 경고 
#define LDK_FIX_EQUIPED_EXPIREDPERIOD_RING_EXCEPTION		// 착용중인 만료된 반지 능력치 계산 예외처리(09.12.11)
#define LDK_FIX_EQUIPED_EXPIREDPERIOD_AMULET_EXCEPTION		// 착용중인 만료된 목걸이 능력치 계산 예외처리(09.12.11)

// - 이동석
#define LDS_FIX_VISUALRENDER_PERIODITEM_EXPIRED_RE			// 인게임샾 아이템들중 목걸이, 반지들에 대한 기간제 종료 이후 사용 불능 처리 (인벤토리상에서 비주얼 처리만.) (09.12.10)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2009.12.08 테섭]- 김재희
//[2009.12.10 본섭]- 김재희
//[2009.12.10 블루]- 김재희

// - 양대근
//#ifdef _TEST_SERVER
// 	#define DEVIAS_XMAS_END_LOADFIX_FOR_TESTSERVER			// 테섭 데비아스 크리스마스 맵 끝내기 위해 test붙인 맵 로딩하도록 함(본섭때는 빼야됨)(2008.1.3)
//#endif // _TEST_SERVER

// - 김재희
#define KJH_FIX_SOURCE_CODE_REPEATED						// 같은 소스코드 중복된것 수정 (09.10.12)

// - 이주흥
#define LJH_FIX_BUG_CASTING_SKILLS_W_CURSOR_OUT_OF_VIEWPORT	// 마우스 오른쪽 버튼 클릭 중 커서가 게임뷰포트를 벗어나 인벤창등에 있을때도 기술을 계속 쓸수 있는 버그 수정(09.12.02)  


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2009.12.01 테섭]- 김재희
//[2009.12.03 본섭]- 김재희
//[2009.12.03 블루]- 김재희

// - 안상규
#define ASG_MOD_QUEST_OK_BTN_DISABLE					// 퀘스트 진행 확인 버튼 비활성화 상태 추가(2009.11.23)

// - 박보근
#define PBG_FIX_PCROOMFONT								// pc방ui사용후 폰트버그 수정(09.11.26)

// - 김재희
#define KJH_FIX_MOVE_ICARUS_EQUIPED_PANDA_CHANGE_RING			// 팬더변신반지 착용후 이카루스로 이동할 수 없는 현상 (09.11.30)

// - 양대근
#define YDG_MOD_PROTECT_AUTO_V6							// 아이로봇 오토 막기 시리즈6 - 마우스 휠 랜덤으로 2칸씩 이동 (2009.11.30)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2009.11.24 테섭]- 양대근
//[2009.11.26 본섭]- 양대근
//[2009.11.26 블루]- 양대근

// - 박보근
#define PBG_ADD_PCROOM_NEWUI								// 새로운 PC방 UI(09.11.19)

// - 안상규
#define ASG_MOD_SERVER_LIST_CHANGE_MSG					// 서버리스트 변경 되었을 때 메시지.(2009.11.18)
#define ASG_MOD_GUILD_RESULT_GENS_MSG					// 길드 가입시 겐스 관련 메시지.(2009.11.23)

// - 이동근 
#define LDK_MOD_BUFFTIMERTYPE_SCRIPT							// 버프타이머 타입값을 buffEffect.txt의 Viewport로 받아서 적용(09.11.10) - enum.h : 타이머 필요한 버스 등록시 eBuffTimeType에 등록안해도 됨

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2009.11.17 테섭]- 양대근
//[2009.11.19 본섭]- 양대근
//[2009.11.19 블루]- 양대근

// - 안상규
#define ASG_FIX_GENS_JOINING_ERR_CODE_ADD				// 겐스 가입 에러 코드 추가.(연합길드장 관련)(2009.11.12)

// - 김재희
#define KJH_FIX_INGAMESHOP_INIT_BANNER						// 인게임샵 배너 다운로드 후 초기화되지 않아 Render가 안돼는 현상 (09.11.10)

// - 이동근 
#ifdef _BLUE_SERVER
	#define LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO	// 캐릭터정보창에서 만료된 데몬, 수호정력 수치 표시 안함(09.11.12)
	#define LDK_MOD_INGAMESHOP_WIZARD_RING_PERSONALSHOPBAN		// 블루섭 유료 마법사의반지 개인상점 거래 금지(09.11.12)
	#define LDK_MOD_ITEM_DROP_TRADE_SHOP_EXCEPTION				// 카오스부적,카오스카드,데몬,수호정령,팬더펫,팬더변신반지,마법사의반지 일반아이템시 버리기,개인상점,개인거래 가능하도록 처리(09.11.16)
#endif //_BLUE_SERVER

// - 양대근
#define YDG_ADD_GM_DISCHARGE_STAMINA_DEBUFF					// GM이 거는 스태미너 방전 디버프 (2009.11.13)

// - 박보근
#define PBG_FIX_MSGBUFFERSIZE								// 메시지박스내 문구 잘리는 버그수정(09.11.13)
#define PBG_FIX_MSGTITLENUM									// 메시지박스 title문구 잘못나오는 버그 수정(09.11.13)
#ifdef _BLUE_SERVER
	#define PBG_MOD_GAMEGUARD_HANDLE							// 게임가드 객체 소멸시에 로긴이후에 클라이언트 종료설정(09.10.23)
#endif// _BLUE_SERVER

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2009.11.10 블루]- 양대근
//[2009.11.12 본섭]- 양대근

// - 이동석
#define LDS_MOD_MODIFYTEXT_TOPAZRING_SAPIRERING				// 사파이어링과 토파즈 링의 텍스트 변경 (09.11.06)
#define LDS_FIX_VISUALRENDER_PERIODITEM_EXPIRED				// 기간제 아이템중 기간이 만료된 아이템들에 대한 처리 (09.11.08)

// - 이동근 
#define LDK_MOD_TRADEBAN_ITEMLOCK_AGAIN						// 일부 유료 아이템중 트레이드 가능 불가능 변경(NOT DEFINE 처리)(09.10.29) (기획에서 수시로 바뀜 ㅡ.ㅡ+)
#define LDK_FIX_PERIODITEM_SELL_CHECK						// 판매시 기간제 아이템 확인 추가(09.11.09)

// - 김재희
#define KJH_MOD_SHOP_SCRIPT_DOWNLOAD						// 샵 스크립트 다운로드 방식 변경 (09.11.09)
#define KJH_FIX_SHOP_EVENT_CATEGORY_PAGE					// 샵 이벤트 카테고리 페이지 버그 수정 (09.11.09)
#define KJH_FIX_EXPIRED_PERIODITEM_TOOLTIP					// 기간만료된 기간제 아이템의 툴팁Render가 안돼는 버그 수정 (09.11.09)
#define KJH_ADD_PCROOM_MOVECOMMAND							// PC방에서 이동풀기(이동의 인장기능) (09.11.09)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2009.11.10 본섭]- 양대근
//[2009.11.06 블루]- 양대근

// - 이동석
#define LDS_MOD_INGAMESHOPITEM_RING_AMULET_CHARACTERATTR	// 인게임샾 아이템들중 반지, 목걸이 시리즈의 STATUS 수치 반영.

// - 이동근 
#define LDK_FIX_NEWWEALTHSEAL_INFOTIMER_AND_MOVEWINDOW		// 풍요의인장 버프정보 시간표시 추가, 버프사용시 이동창 비활성화 (09.11.06)

// - 박보근
#define PBG_FIX_TRADECLOSE									// 거래중 인벤을 닫을시에 거래창은 닫히지 않는 버그 수정(09.11.06)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2009.11.05 테섭]- 양대근
//[2009.11.05 블루]- 양대근

// - 박종훈
#define PJH_ADD_MINIMAP

// - 이동석
#define LDS_FIX_INGAMESHOPITEM_PASSCHAOSCASTLE_REQUEST	// 카오스캐슬 자유 입장권 사용시 서버로 사용에 대한 패킷 정보를 보내지 않는 문제 수정 (09.11.02)
#define LDS_MOD_INGAMESHOPITEM_POSSIBLETRASH_SILVERGOLDBOX	// 금,은 상자 버리기가 가능하도록.

// - 안상규
#define ASG_ADD_SERVER_LIST_SCRIPTS					// ServerList.txt 스크립트 추가.(2009.10.29)

#ifdef _BLUE_SERVER
// - 박보근
#define PBG_MOD_STAMINA_UI								// 스테미너 관련 게이지 추가(09.11.2)
#define PBG_MOD_BLUE_SUMMONER_ENABLE					// 블루쪽 소환술사 캐릭 생성 활성화 상태로 기획변경(09.11.3)
#endif //_BLUE_SERVER

// - 김재희
#define KJH_FIX_RENDER_PERIODITEM_DURABILITY			// 기간제 아이템일때 내구도정보가 Render 되는 문제 (2009.11.03)
#define KJH_MOD_RENDER_INGAMESHOP_KEEPBOX_ITEM			// 게임샵 보관함 아이템의 수량이 1개일때 Render 안함 (2009.11.03)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2009.10.29 블루] - 이동석
#ifdef _BLUE_SERVER
	#define PBG_MOD_BLUEVERLOGO									// 서버선택화면에서 버젼 블루 버젼 다르게 표시하는것(09.10.28)
	#define YDG_MOD_BLUE_NEWSERVER_ORDER						// 블루 신서버 위치 수정(09.10.30)
#endif //_BLUE_SERVER

#define LDK_MOD_INGAMESHOP_ITEM_CHANGE_VALUE				// 혼합유료화 아이템 가격 변경(09.10.29)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2009.10.29 테섭] - 이동석
//[2009.10.29 블루] - 이동석

// - 박보근
#ifdef _BLUE_SERVER
	#define PBG_FIX_BLUEHOMEPAGE_LINK						// f11누를시 뮤블루 홈페이지로 들어가게 링크(09.10.27) 타이틀이름 MUBLUE로 변경(09.10.28)
#endif //_BLUE_SERVER

// - 김재희
#define KJH_MOD_INGAMESHOP_PATCH_091028						// 인게임샵 수정 - 이벤트 카테고리, 인게임샵 라이브러리 업데이트 (09.10.28)

// - 이동근 
#define LDK_FIX_EMPIREGUARDIAN_UI_HOTKEY					// 제국 수호군npc ui창 열려있을때 캐릭창, 인벤창 안열리도록한다(09.10.28)
#define LDK_FIX_BLUESERVER_UNLIMIT_AREA						// 블루섭 전용 이동창 이동제한 설정중 추가지역 제한 해제(09.10.28)
#define LDK_FIX_PETPOSITION_MULTIPLY_OWNERSCALE				// 캐릭터 선택창 에서 캐릭터 크기에 따라 펫위치 수정(09.10.28)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2009.10.27 테섭] - 김재희
//[2009.10.27 블루] - 박보근

#define PBG_FIX_RESETFRUIT_CAL							// 리셋열매 경고창 기본 스탯추가 계산 버그수정(09.10.25)

// 김재희
#define KJH_ADD_SERVER_LIST_SYSTEM						// 서버리스트 시스템 변경 (09.10.19) - 해외 모두 들어간 뒤에 디파인정리해야 함
														// (이후에 스크립트 추가작업 예정- 스크립트 작업은 이 디파인에 종속되어야 함)

// 안상규	
#define ASG_MOD_QUEST_WORDS_SCRIPTS						// QuestWords.txt 애니 번호와 회수 제거 (2009.10.22) (QuestWords.txt 일본 적용시 디파인 정리할 것)

#define ASG_ADD_GENS_SYSTEM								// 겐스 시스템(2009.09.14)	[주의] 겐스 시스템 디파인이 켜지면 ASG_ADD_UI_NPC_MENU 디파인은 꺼짐. 해외 추가시 주의.
#ifdef ASG_ADD_GENS_SYSTEM
	#define ASG_ADD_UI_NPC_DIALOGUE						// NPC 대화창(2009.09.14)
	#define ASG_ADD_INFLUENCE_GROUND_EFFECT				// 겐스 세력 구분 바닥 이펙트(2009.10.07)
	#define ASG_ADD_GENS_MARK							// 겐스 마크 표시(2009.10.09)
	#define ASG_ADD_GENS_NPC							// 겐스 NPC 추가(2009.10.12)
	#define ASG_ADD_GATE_TEXT_MAX_LEVEL					// Gate.txt 최대 레벨 추가.(2009.10.16)	[주의] 리소스 컨버터 새로운 소스로 컴파일 할 것.
	#define ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL				// Movereq.txt 최대 레벨 추가.(2009.10.16)	[주의] 리소스 컨버터 새로운 소스로 컴파일 할 것.
#endif	// ASG_ADD_GENS_SYSTEM

// 김재희,박보근 - 인게임샾 시스템
#define KJH_PBG_ADD_INGAMESHOP_SYSTEM					// 인게임샵 시스템

#ifdef KJH_PBG_ADD_INGAMESHOP_SYSTEM
	#define PBG_ADD_INGAMESHOP_UI_MAINFRAME				// 메인프레임 UI(09.04.02)
	#define PBG_ADD_INGAMESHOP_UI_ITEMSHOP				// 인게임샵 구입공간(09.04.03)
	#define PBG_ADD_NAMETOPMSGBOX						// 인게임샵 메시지 박스 틀 만들기(09.05.26)
	#define KJH_ADD_INGAMESHOP_UI_SYSTEM				// 인게임샵 시스템 (09.08.17)
	#define KJH_ADD_PERIOD_ITEM_SYSTEM					// 기간제 아이템 시스템 (09.09.29)
	#define PBG_ADD_INGAMESHOPMSGBOX					// 인게임샵관련 메시지 박스(09.06.24)
	#define PBG_ADD_MSGBACKOPACITY						// 인게임샵 메시지박스창 뒷배경 어둡게 처리(09.07.20)
	#define PBG_ADD_ITEMRESIZE							// 인게임샵 아이템의 사이즈 변경(09.09.10)
	#define PBG_FIX_ITEMANGLE							// 모델 랜더시 뒤집힌 아이템 수정(09.10.05)
	#define PBG_ADD_MU_LOGO								// 새로운 로고 변경(09.10.08)
	#define PBG_ADD_CHARACTERSLOT						// 캐릭터 슬롯아이템(09.05.06)
	#define KJH_DEL_PC_ROOM_SYSTEM						// 피씨방 시스템 삭제 (09.10.22) - 피씨방 접속여부는 남겨둠(class CPCRoomPtSys)

	#ifdef _BLUE_SERVER		//  블루서버에만 적용
		// 디파인 정리시엔 _BLUE_SERVER로 감싸야 함	블루 적용외일경우만 _BLUE_SERVER밖으로 제거 해야함
		#define PBG_ADD_CHARACTERCARD						// 캐릭터 카드 다크로드 마검사 생성 카드(09.05.04)소환술사카드 추가(09.06.04)
		#define PBG_ADD_SECRETITEM							// 혼합유료화 서버에서의 피로도 관련 ITEM추가(09.06.05) =>> blue
		#define PBG_ADD_SECRETBUFF							// 혼합유료화 서버에서의 피로도 관련 BUFF추가(09.06.08)/피로도 시스템(09.06.15) =>> blue
		#define PBG_ADD_PKSYSTEM_INGAMESHOP					// 혼합유료화 서버에서의 pk시스템 리뉴얼(09.06.05) =>> blue
		#define PBG_ADD_MUBLUE_LOGO							// 블루 뮤 로고 적용(09.10.08) =>> blue
		#define	PBG_ADD_LAUNCHER_BLUE						// 블루 런쳐 적용(09.10.19)
		#define PBG_MOD_GAMECENSORSHIP						// 블루뮤는 18세 이상 가능으로 변경(09.10.22)
		#define LDK_ADD_INGAMESHOP_LIMIT_MOVE_WINDOW		// 국내 전용 이동창 이동제한 설정, 이동의 인장으로 해제 (PSW_SEAL_ITEM관련)
	#endif // _BLUE_SERVER

	#define	PSW_PARTCHARGE_ITEM1					// 필수 - 해외 유료화 아이템 추가시 define LDK_ADD_CASHSHOP_FUNC 사용 못하도록 한다. (2009.09.15)
	#define ASG_ADD_CS6_ASCENSION_SEAL_MASTER 		// 상승의인장마스터(2009.02.24)
	#define ASG_ADD_CS6_WEALTH_SEAL_MASTER 			// 풍요의인장마스터(2009.02.24)
	#define PBG_MOD_MASTERLEVEL_MESSAGEBOX			// 상승의 인장 마스터케릭터 구입여부 추가(09.4.24)
	#define PSW_ADD_PC4_SEALITEM					// 치유, 신성( 체젠, 마젠 ) 인장
	#define CSK_LUCKY_CHARM							// 행운의 부적( 2007.02.22 )
	#define YDG_ADD_CS5_REVIVAL_CHARM				// 부활의 부적
	#define YDG_ADD_CS5_PORTAL_CHARM				// 이동의 부적
	#define ASG_ADD_CS6_GUARD_CHARM					// 수호의부적(2009.02.09)
	#define ASG_ADD_CS6_ITEM_GUARD_CHARM			// 아이템보호부적(2009.02.09)
	#define LDS_ADD_CS6_CHARM_MIX_ITEM_WING			// 날개 조합 100% 성공 부적
	#define PBG_FIX_CHARM_MIX_ITEM_WING_TOOLTIP		// 날개의 부적 툴팁 잘못 출력되는 버그(09.05.11)
	#define PBG_FIX_CHARM_MIX_ITEM_WING				// 날개의 부적 상점에 판매후 다음 물품 판매 안되는 버그 수정(09.05.06)
	#define CSK_FREE_TICKET							// 자유입장권 - 데빌스퀘어, 블러드캐슬, 칼리마 (2007.02.06)
	#define PSW_CURSEDTEMPLE_FREE_TICKET			// 환영의 사원 자유 입장권
	#define PSW_SCROLL_ITEM							// 엘리트 스크롤
	#define PSW_ADD_PC4_SCROLLITEM				    // 전투, 강화( 크리, 액설 데미지 증가 ) 스크롤
	#define PSW_SECRET_ITEM							// 강화의 비약
	#define PSW_SEAL_ITEM							// 이동 인장

	// 인게임샆 적용 아이템 1차
	#define INGAMESHOP_ITEM01							// 해외에만 등록되어있거나 새로 생성된 아이템 디파인 추가(2009.09.08)
	#ifdef	INGAMESHOP_ITEM01
		// - 버그수정
		#define LDK_FIX_TICKET_INFO							// 악마의광장입장권, 블러드캐슬입장권 레벨 자동적용 문구 추가.
		#define LDK_FIX_PC4_GUARDIAN_DEMON_INFO				// 데몬 캐릭터정보창에 공격력,마력,저주력 추가 데미지 적용(09.10.15)
		#define LDS_MOD_CHAOSCHARMITEM_DONOT_TRADE			// 기획 요청으로 카오스조합 부적의 거래가 이루어 지지 않도록 적용. (09.10.15)

		// - 이동근
		#define LDK_MOD_PREMIUMITEM_DROP					// 프리미엄아이템 버리기제한해제(09.09.16)국내만 적용
		#define LDK_MOD_PREMIUMITEM_SELL					// 프리미엄아이템 판매제한해제(09.09.25)국내만 적용
		#define LDK_ADD_INGAMESHOP_GOBLIN_GOLD				// 고블린금화
		#define LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST		// 봉인된 금색상자
		#define LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST		// 봉인된 은색상자
		#define LDK_ADD_INGAMESHOP_GOLD_CHEST				// 금색상자
		#define LDK_ADD_INGAMESHOP_SILVER_CHEST				// 은색상자
		#define LDK_ADD_INGAMESHOP_PACKAGE_BOX				// 패키지 상자A-F
		#define LDK_ADD_INGAMESHOP_SMALL_WING				// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
		#define LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL			// 신규 풍요의 인장

		// - 이동석
		#define PSW_INDULGENCE_ITEM							// 인게임샾 아이템 // 면죄부2종 추가
		#define PSW_ELITE_ITEM								// 인게임샾 아이템 // 엘리트물약2종(치료물약,마나물약) 추가
		#define PSW_FRUIT_ITEM								// 인게임샾 아이템 // 리셋열매5종

		#ifdef PSW_FRUIT_ITEM
			#define LDS_ADD_NOTICEBOX_STATECOMMAND_ONLYUSEDARKLORD	// 인게임샾 아이템 // 리셋열매5종 // 통솔리셋열매는 다크로드만 사용가능한 메세지 출력.
		#endif // PSW_FRUIT_ITEM

		#define LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE		// 인게임샾 아이템 // 신규 사파이어(푸른색)링	// MODEL_HELPER+109
		#define LDS_ADD_INGAMESHOP_ITEM_RINGRUBY			// 인게임샾 아이템 // 신규 루비(붉은색)링		// MODEL_HELPER+110
		#define LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ			// 인게임샾 아이템 // 신규 토파즈(주황)링		// MODEL_HELPER+111
		#define LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST		// 인게임샾 아이템 // 신규 자수정(보라색)링		// MODEL_HELPER+112
		#define LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY			// 인게임샾 아이템 // 신규 루비(붉은색) 목걸이	// MODEL_HELPER+113
		#define LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD		// 인게임샾 아이템 // 신규 에메랄드(푸른) 목걸이// MODEL_HELPER+114
		#define LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE		// 인게임샾 아이템 // 신규 사파이어(녹색) 목걸이// MODEL_HELPER+115
		#define LDS_ADD_INGAMESHOP_ITEM_KEYSILVER			// 인게임샾 아이템 // 신규 키(실버)				// MODEL_POTION+112
		#define LDS_ADD_INGAMESHOP_ITEM_KEYGOLD				// 인게임샾 아이템 // 신규 키(골드)				// MODEL_POTION+113
		#define LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
		#define LDS_ADD_INGAMESHOP_ITEM_PRIMIUMSERVICE6		// 인게임샾 아이템 // 프리미엄서비스6종			// MODEL_POTION+114~119
		#define LDS_ADD_INGAMESHOP_ITEM_COMMUTERTICKET4		// 인게임샾 아이템 // 정액권4종					// MODEL_POTION+126~129
		#define LDS_ADD_INGAMESHOP_ITEM_SIZECOMMUTERTICKET3	// 인게임샾 아이템 // 정량권3종					// MODEL_POTION+130~132
	#endif	//INGAMESHOP_ITEM01
#endif //KJH_PBG_ADD_INGAMESHOP_SYSTEM

// 이동근
#define LDK_FIX_GM_WEBZEN_NAME								// GM판별시 id에 webzen이란 단어가 들어가도 gm인정(09.10.13)

// 이동석
#define LDS_FIX_PETDESTRUCTOR_TERMINATE_EFFECTOWNER_ADD_TYPE// LDS_FIX_AFTER_PETDESTRUCTOR_ATTHESAMETIME_TERMINATE_EFFECTOWNER이후 추가작업. 다크스피릿 공격:돌격(MODEL_AIR_FORCE) Effect사용 도중 펫::다크스피릿 메모리 반환시 뻑나는 문제 FIXED. (09.10.15)

// 양대근
#define YDG_FIX_REPAIR_COST_ADJUST_TO_SERVER_SETTING	// 수리 가격 계산 서버에 맞춤 (09.10.12)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2009.10.06 본섭] - 이동석
//[2009.10.08 테섭] - 이동석

// - 이동석
#define LDS_MOD_ANIMATIONTRANSFORM_WITHHIGHMODEL			// AnimationTransformWithAttachHighModel함수에서 현재 arrayBone만 필요할 경우는 Transform으로 Bone이 Vertices에 곱하지 않도록 변경. (2009.09.09)
#define LDS_ADD_ANIMATIONTRANSFORMWITHMODEL_USINGGLOBALTM	// AnimationTransformWithAttachHighModel함수내 TMArray가 Local인 이유로 이후 연산에 TM이 Point를 잃어버리는 현상으로 TM을 Global 으로 변경. (09.09.07)
#define LDS_FIX_EG_JERINT_ANIMATION_AND_SWORDFORCE_SPEED	// 제린트의 공격 속도 및 검기 시간 범위 재작업 (09.09.08) 
#define LDS_FIX_AFTER_PETDESTRUCTOR_ATTHESAMETIME_TERMINATE_EFFECTOWNER	// 펫 메니저의 펫 종료 될 때마다 펫당 발생하던 메모리릭 FIXED. (YDG_FIX_MEMORY_LEAK_0905) define작업 이후 발생 문제로 메모리 반환된 Pet이 Effect 중일때 발생(Owner로 자신을 넘긴경우만)으로 Effect의 들의 NULL 처리 작업. (09.09.17)

// - 해외버그 관련 수정사항
#define YDG_FIX_PANDA_CHANGERING_PANTS_BUG				// 다크로드 맨몸으로 팬더 변신반지 착용시 바지 버이는 버그 (09.07.21)
#define YDG_FIX_DUEL_SUMMON_CLOAK						// 결투시 요정 소환수 망토 나오는 버그 (09.07.29)
#define PBG_FIX_PETTIP									// 인벤내부의 다크로드 관련 펫의 정보가 착용한 펫의 정보로 출력되는 버그 수정(09.09.10)
#define LJH_FIX_CHANGE_RING_DAMAGE_BUG					// 변신반지 복수 착용시 높은 값으로 데미지,마력,저주력 증가가 되도록 수정 (09.09.11)
#define YDG_FIX_OVER_5_SETITEM_TOOLTIP_BUG				// 세트아이템 5종류 이상 차면 계산안되는 버그 수정(09.09.11)
#define YDG_FIX_MAGIC_DAMAGE_CALC_ORDER					// 세트아이템 관련 마력 계산 순서 수정 (09.09.11)
#define YDG_FIX_380ITEM_OPTION_TEXT_SD_PERCENT_MISSING	// 380아이템 옵션에 SD증가에 % 빠진 버그 수정 (09.09.23)
#define YDG_FIX_BUFFTIME_OVERFLOW						// 버프시간이 0보다 작아지면 넘치는 버그 (09.09.28)
#define LJH_FIX_PET_SHOWN_IN_CHAOS_CASTLE_BUG			// 펫이 카오스캐슬에서 보이는 버그 수정(09.09.29)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
//[2009.09.10 본섭] - 이동근
//[2009.09.07 테섭] - 이동근

// - 양대근
#define YDG_DARK_SPIRIT_CRASH_BECAUSEOF_FIX_MEMORY_LEAK	// 메모리 누수 수정때문에 발생한 다크스피릿 튕기는 현상 임시 수정 (2009.09.02)
#define YDG_FIX_SOCKETITEM_SELLPRICE_BUG				// 소켓아이템 판매가격 버그 (09.09.03)
#define YDG_FIX_TRADE_BUTTON_LOCK_WHNE_ITEM_MOVED		// 거래창 사기관련 트레이드 장비 옮기면 거래버튼 잠그기(2009.08.25)
#define YDG_FIX_NPCSHOP_SELLING_LOCK					// NPC상점 판매시 창 못닫게 잠금 (2009.08.25)
#define YDG_FIX_CATLE_MONEY_INT64_TYPE_CRASH			// 공성 성주창 성의 돈이 많을때 튕기는 버그 (2009.09.01)

// - 이동석
#define LDS_FIX_SKILLKEY_DISABLE_WHERE_EG_ALLTELESKILL	// 법사 텔레계열(텔레키네시스,순간이동) 스킬들은 제국수호군 영역에서 사용 못하도록 설정(09.08.28)
#define LDS_FIX_EG_JERINT_ATTK1_ATTK2_SPEED				// 제국수호군 제린트 공격1, 공격2 속도 잘못된 수치로 FIX 작업 (09.09.04)

// - 안상규
#define ASG_FIX_QUEST_PROTOCOL_ADD					// 신규 퀘스트 프로토콜 추가.(2009.09.07)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.08.31 테섭] - 김재희

// - 박보근
#define PBG_FIX_CHANGEITEMCOLORSTATE							// 인벤토리 안에서 내구력에 따른 색깔 변화하도록 수정(데몬/수호정령/팬더펫)(09.06.16)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.08.27 본섭] - 김재희
// [2009.08.26 테섭] - 김재희

// UIMng.cpp 로딩화면 이미지 교체
// GMNewTown.cpp 엘베란드 절벽화면 버그 수정 - define 없음

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.08.27 본섭] - 김재희
// [2009.08.25 테섭] - 김재희

// - 김재희
#define KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM_EX	// NPC상점에서 아이템을 구매할때, 처음에 두번클릭해야 아이템이 구입되는 현상 추가 버그수정 (09.08.24)

// - 이동석
#define LDS_FIX_EG_COLOR_CHANDELIER							// release 모드서만 제국 수호군 주말맵 샹들리에 연기 색 다른 버그 수정 (09.08.21)
#define LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER_EX2	// 방패병 44번 방패막기 스킬시 간혹 렌덤방향 바라보는 버그 수정 (09.08.24)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.08.27 본섭] - 김재희
// [2009.08.21 테섭] - 김재희

// - 이동석
#define LDS_ADD_SET_BMDMODELSEQUENCE_					// BMD 클래스의 사용되지 않는 변수인 BmdSequenceID에 BMDSequence 설정. (09.08.05)
#define LDS_FIX_MEMORYLEAK_BMDWHICHBONECOUNTZERO		// BoneCount==0 인 BMD들에 대한 BMD Destruct 시점에 발생하는 메모리 릭 FIX. (09.08.13)
#define LDS_FIX_MEMORYLEAK_0908_DUPLICATEITEM			// DuplicateItem() 내에 힙 영역에 생성된 Item의 메모리 반환 미처리로 메모리 릭 FIX. (09.08.13)
#define LDS_FIX_GLERROR_WRONG_WRAPMODEPARAMETER			// ImageOpen시 WrapMode Param 값이 잘못되어, OpenGLERROR가 뜨며 이에 대한 FIX. (MR0 적용 시 치명적 오류가 발생하므로..) (09.08.18)
#define LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER	// 제국 수호군 방패병의 서버로부터의 몬스터 스킬==>Action 연결 작업. (09.08.18)
#define LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER_EX	// 방패병의 1번 스킬(44)동작의 기존방식과 다른 방식의 에니메이션 호출 재 수정 (09.08.21)

// - 박보근
#define PBG_ADD_NEWLOGO_IMAGECHANGE						// 로딩화면 이미지 변경(09.08.12)
#define PJH_NEW_SERVER_SELECT_MAP						// 서버 선택화면 변경(09.08.17)
#define YDG_MOD_TOURMODE_MAXSPEED						// 투어모드 최고속도 올림 (2009.07.10)

// - 양대근
#define YDG_FIX_DOPPELGANGER_BUTTON_COLOR				// 도플갱어 UI 버튼 잠글때 색 이상한 문제 (2009.08.14)
#define YDG_MOD_DOPPELGANGER_END_SOUND					// 도플갱어 종료 사운드 추가 (2009.08.20)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.08.27 본섭] - 김재희
// [2009.08.13 테섭] - 김재희

// - 양대근
#define YDG_ADD_DOPPELGANGER_EVENT				// 도플갱어 이벤트 (2009.04.22) [패치안됨]
#define YDG_MOD_SEPARATE_EFFECT_SKILLS					// 스킬 검사하는 이펙트 분리 (블러드 어택 등) (2009.08.10) [패치안됨]

// - 안상규
#define ASG_ADD_NEW_DIVIDE_STRING						// 새로운 문장 나누기 기능.(2009.06.08)
#define ASG_ADD_NEW_QUEST_SYSTEM						// 뉴 퀘스트 시스템(2009.05.20)

// 제국 수호군 파티 - 이동석, 이동근
#define LDS_ADD_EMPIRE_GUARDIAN					// 제국 수호군

// - 박보근
#define PBG_ADD_DISABLERENDER_BUFF						// 랜더안할 버프 이미지 처리(09.08.10)

//------------------------------------------------
#ifdef YDG_ADD_DOPPELGANGER_EVENT
	#define YDG_ADD_MAP_DOPPELGANGER1					// 도플갱어 맵1 (라클리온베이스) (2009.03.22) [패치안됨]
	#define YDG_ADD_MAP_DOPPELGANGER2					// 도플갱어 맵2 (불카누스베이스) (2009.03.22) [패치안됨]
	#define YDG_ADD_MAP_DOPPELGANGER3					// 도플갱어 맵3 (아틀란스베이스) (2009.03.22) [패치안됨]
	#define YDG_ADD_MAP_DOPPELGANGER4					// 도플갱어 맵4 (칸투르1차베이스) (2009.03.22) [패치안됨]
	#define YDG_ADD_DOPPELGANGER_MONSTER				// 도플갱어 몬스터 추가 (2009.05.20) [패치안됨]
	#define YDG_ADD_DOPPELGANGER_NPC					// 도플갱어 NPC 루가드 추가 (2009.05.20) [패치안됨]
	#define YDG_ADD_DOPPELGANGER_ITEM					// 도플갱어 아이템 추가 (2009.05.20) [패치안됨]
	#define YDG_ADD_DOPPELGANGER_UI						// 도플갱어 UI 추가 (2009.05.20) [패치안됨]
	#define YDG_ADD_DOPPELGANGER_PROTOCOLS				// 도플갱어 프로토콜 추가 (2009.06.23) [패치안됨]
	#define YDG_ADD_DOPPELGANGER_PORTAL					// 도플갱어 맵 포탈 추가 (2009.07.30) [패치안됨]
	#define YDG_ADD_DOPPELGANGER_SOUND					// 도플갱어 사운드 추가 (2009.08.04) [패치안됨]
#endif	// YDG_ADD_DOPPELGANGER_EVENT

#ifdef LDS_ADD_EMPIRE_GUARDIAN
	// - 이동석
	#define LDS_ADD_MAP_EMPIREGUARDIAN2					// 제국 수호군 맵 2   (화,  금)
	#define LDS_ADD_MAP_EMPIREGUARDIAN4					// 제국 수호군 맵 2   (일	  )
	#define LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN	// 제국 수호군 맵 4   (일	  )몬스터 월드 보스 가이온 카레인	(504/164)
	#define LDS_ADD_EG_4_MONSTER_JELINT					// 제국 수호군 맵 4   (일	  )몬스터 가이온 보좌관 제린트		(505/165)
	#define LDS_ADD_EG_3_4_MONSTER_RAYMOND				// 제국 수호군 맵 3,4 (수토,일)몬스터 부사령관 레이몬드			(506/166)
	#define LDS_ADD_EG_2_4_MONSTER_ERCANNE				// 제국 수호군 맵 2,4 (화금,일)몬스터 지휘관 에르칸느			(507/167)
	#define LDS_ADD_EG_2_MONSTER_2NDCORP_VERMONT		// 제국 수호군 맵 2   (화,  금)몬스터 2군단장 버몬트			(509/169)
	#define LDS_ADD_EG_2_MONSTER_ARTICLECAPTAIN			// 제국 수호군 맵 2   (화,  금)몬스터 기사단장					(514/174)
	#define LDS_ADD_EG_2_MONSTER_GRANDWIZARD			// 제국 수호군 맵 2   (화,  금)몬스터 대마법사					(515/176)

	// - 이동근 - 미리 define잡은후 차후 define정리 작업 예정
	#define LDK_ADD_EG_MONSTER_DEASULER					// 제국 수호군 1군단장 데슬러									(508/168)
	#define LDK_ADD_EG_MONSTER_DRILLMASTER				// 제국 수호군 제국수호군전투교관
	#define LDK_ADD_EG_MONSTER_QUARTERMASTER			// 제국 수호군 병참장교
	#define LDK_ADD_EG_MONSTER_CATO						// 제국 수호군 2군단장 카토
	#define LDK_ADD_EG_MONSTER_ASSASSINMASTER			// 제국 수호군 암살장교
	#define LDK_ADD_EG_MONSTER_RIDERMASTER				// 제국 수호군 기마단장
	#define LDK_ADD_EG_MONSTER_GALLIA					// 제국 수호군 4군단장 갈리아

	#define LDK_FIX_EG_DOOR_ROTATION_FIXED				// 제국 수호군 성문 악령에 의한 회전수정(2009.07.21)
	#define LDK_ADD_EG_DOOR_EFFECT						// 제국 수호군 맵 성문 파괴 이펙트
	#define LDK_ADD_EG_STATUE_EFFECT					// 제국 수호군 맵 석상 파괴 이펙트

	//몬스터 - 공통
	#define	LDK_ADD_EG_MONSTER_RAYMOND					// 제국 수호군 부사령관 레이몬드 (수, 일)
	#define LDK_ADD_EG_MONSTER_KNIGHTS					//제국 수호군 기사단	Imperial Guardian Knights	520 - 181	
	#define LDK_ADD_EG_MONSTER_GUARD					//제국 수호군 호위병	Imperial Guardian guard		521 - 182
	#define ASG_ADD_EG_MONSTER_GUARD_EFFECT				// 제국 수호군 호위병 (검기 이펙트)
	#define LDS_ADD_EG_MONSTER_GUARDIANDEFENDER			// 제국 수호군 맵 1234(모든요일)몬스터 수호군 방패병			(518/178)
	#define LDS_ADD_EG_MONSTER_GUARDIANPRIEST			// 제국 수호군 맵 1234(모든요일)몬스터 수호군 치유병			(519/179)

	//맵
	#define LDK_ADD_MAPPROCESS_RENDERBASESMOKE_FUNC		//MapProcess에 RenderBaseSmoke 처리 함수 추가(09.07.31)
	#define	LDK_ADD_MAP_EMPIREGUARDIAN1					// 제국 수호군 맵 1 (월, 목)
	#define	LDK_ADD_MAP_EMPIREGUARDIAN3					// 제국 수호군 맵 3 (수, 토)

	#define LDK_ADD_EMPIRE_GUARDIAN_DOOR_ATTACK			// 제국 수호군 성문은 맵속성무기하고 공격가능하도록함 (09.08.06)

	//UI
	#define LDK_ADD_EMPIREGUARDIAN_UI					// 제국 수호군 ui
	//프로토콜
	#define LDK_ADD_EMPIREGUARDIAN_PROTOCOLS			// 제국 수호군 프로토콜
	//item
	#define LDK_ADD_EMPIREGUARDIAN_ITEM					// 제국 수호군 아이템(4종:명령서, 종이쪽지, 세크로미콘, 조각)

	//테스트용 DEFINITION
	//	#define LDK_TEST_MAP_EMPIREGUARDIAN				// 제국 수호군 테스틑용 임시 디파인
	//	#define LDS_TEST_MAP_EMPIREGUARDIAN__				// 제국 수호군 임시 디파인 화/금, 일

	//기타 RESOURCE - 공통사항 
	#define LDS_ADD_RESOURCE_FLARERED					// FLARE_RED.jpg 효과 추가

	//기타 - 공통사항	
	#define LDS_ADD_MODEL_ATTACH_SPECIFIC_NODE_			// 특정 모델의 특정 노드에 다른 특정 모델을 ATTACH 하는 함수를 추가합니다. (09.06.12)
	#define LDS_ADD_INTERPOLATION_VECTOR3				// vec3_t의 보간 함수를 zzzmathlib.h에 추가. (09.06.24)
	#define LDS_MOD_INCREASE_BITMAPIDXBLURNUMBERS		// BITMAP_BLUR 인덱스 수를 기존4개 에서 실사용갯수인 10개로 늘립니다. (09.07.02)
	#define LDS_ADD_EFFECT_FIRESCREAM_FOR_MONSTER		// 다크로드의 파이어스크림 스킬에 특정 몬스터(에르칸느)의 파이어스크림을 추가합니다. (09.07.07)
	#define LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD		// EffectBlurSpark 의 검기들의 갯수 제한에 문제로 검기가 간혹 깨지는 현상으로 부분 수정. (09.07.28)
	#define LDS_ADD_MAP_EMPIREGUARDIAN4_MAPEFFECT		// 주말맵의 맵 Effect에 추가되는 파티클, EFFECT들.. 
	#define LDS_FIX_MEMORYLEAK_DESTRUCTORFORBMD			// BMD의 소멸자로 인한 기존 BMD에서 발생한 memoryleak FIXED.. (2009.08.13)

	// - 박보근
	#define PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT	// (레이몬드)(치유병)몬스터 이펙트 작업(09.07.02)

	// - 김재희
	#define KJH_ADD_EG_MONSTER_KATO_EFFECT					// 3군단장 카토 이팩트 (09.07.20)
	#define KJH_ADD_EG_MONSTER_GUARDIANDEFENDER_EFFECT		// 수호군 방패병 이팩트 (09.07.31)
#endif //LDS_ADD_EMPIRE_GUARDIAN

#ifdef ASG_ADD_NEW_QUEST_SYSTEM
	#define ASG_ADD_UI_QUEST_PROGRESS					// 퀘스트 진행 창(NPC용)(2009.05.27)
#ifndef ASG_ADD_GENS_SYSTEM		// 정리시 ASG_ADD_UI_NPC_MENU 디파인 모두 삭제.
	#define ASG_ADD_UI_NPC_MENU							// NPC 메뉴창(2009.06.17)
#endif	// ASG_ADD_GENS_SYSTEM
	#define ASG_ADD_UI_QUEST_PROGRESS_ETC				// 퀘스트 진행 창(기타용)(2009.06.24)
	#define ASG_MOD_UI_QUEST_INFO						// 퀘스트 정보 창(2009.07.06)
	#define ASG_MOD_3D_CHAR_EXCLUSION_UI				// 3D 캐릭터 표현이 없는 UI로 바꿈.(2009.08.03)
	//(!주의!) UI에 3D캐릭터 표현이 추가 되면 원래대로 되돌려야 되므로 정리하지 말 것.
#endif	// ASG_ADD_NEW_QUEST_SYSTEM

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.08.13 본섭] - 김재희
// [2009.08.11 테섭] - 김재희

// 아래[2009.07.23 본섭]일자 겜블여름이벤트 해제. (주석처리 : #define LDK_MOD_GAMBLE_SUMMER_SALE_EVENT)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.07.23 본섭] - 안상규
// [2009.07.17 테섭] - 안상규
// - 박보근
#define PBG_MOD_LUCKYCOINEVENT					// 행운의동전 255개 이상 등록되지 않도록 이벤트 수정(09.07.15)(다른UI와의버그,열고이동버그)

// - 양대근
#define YDG_FIX_CLIENT_SKILL_EFFECT_SIZE		// 이펙트 많을때 블러드어택등 클라스킬 안먹는 버그 (2009.07.13)

// - 이동근
//#define LDK_MOD_GAMBLE_SUMMER_SALE_EVENT		// 겜블 여름 이벤트 50% 세일 (09.07.16)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.07.09 본섭] - 안상규(신버전 게임가드 라이브러리만 적용)
// [2009.07.02 테섭] - 안상규(신버전 게임가드 라이브러리만 적용)
// [2009.06.25 본섭] - 박보근
// [2009.06.23 테섭] - 박보근
// - 박보근
#define PBG_FIX_DARK_FIRESCREAM_HACKCHECK		// 다크 파이어스크림 (서버)핵체크관련 버그 수정(09.06.22)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.06.18 본섭] - 박보근
// [2009.06.15 테섭] - 박보근
// - 박보근
#define PBG_FIX_CHAOTIC_ANIMATION				// 카오틱 디세이어 애니메이션 수정(09.06.11)
//#if SELECTED_LANGUAGE == LANGUAGE_KOREAN
#ifdef _LANGUAGE_KOR					// 국내만 적용
	#define PBG_MOD_PREMIUMITEM_TRADE				// 프리미엄아이템거래제한해제(09.06.11) 국내만 적용
//#endif //SELECTED_LANGUAGE == LANGUAGE_KOREAN
#endif // _LANGUAGE_KOR					// 국내만 적용


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.06.11 본섭] - 박보근
// [2009.06.09 테섭] - 박보근
// - 박보근
#define PBG_MOD_INVENTORY_REPAIR_COST			// 자동수리금액 하향조정(09.06.08)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.06.04 본섭] - 박보근
// [2009.06.02 테섭] - 박보근
//	팬더변신반지 버그 수정 패치
// [2009.06.01 테섭] - 박보근
// - 안상규
//#if SELECTED_LANGUAGE == LANGUAGE_KOREAN
#ifdef _LANGUAGE_KOR					// 국내만 적용
	#define ASG_ADD_MULTI_CLIENT				// 멀티 클라이언트 (2009.03.17).
//#endif //SELECTED_LANGUAGE == LANGUAGE_KOREAN
#endif // _LANGUAGE_KOR					// 국내만 적용

// - 박종훈
#define PJH_ADD_PANDA_CHANGERING			// 펜더 변신반지




// [2009.05.28 테섭] - 이동석			
// - 안상규
#define ASG_FIX_ACTIVATE_APP_INPUT				// 뮤가 액티브 상태가 아닐때 UI등에서 키 입력되는 버그.(멀티클라이언트 디파인 2/3.) (2009.03.20)
// 멀티 클라이언트 부분 - 게임 프로텍트 라이브러리 적용 이후 주석 풀어야 합니다.

// - 이동석
#define LDS_FIX_MEMORYLEAK_PHYSICSMANAGER_RELEASE		// 메모리 누수 제거, Physicsmanager 힙영역 메모리 반환 처리. (다크로드 케릭 로딩이후 종료 시 릭 발생)(09.05.25)

// - 양대근
#define YDG_FIX_MEMORY_LEAK_0905_2ND					// 메모리 누수 제거 2차 (2009.05.19)

// - 김재희
#define KJH_FIX_CHAOTIC_ANIMATION_ON_RIDE_PET			// 펫을타고 다크로드 카오틱디세이어 스킬시 애니메이션수정 (08.12.18)

// - 해외에서 국내에 적용되어야 할것들
#define KJH_FIX_JP0459_CAN_MIX_JEWEL_OF_HARMONY		// 조화의보석으로 업그레이드 할수 있는 아이템인데도 빨갛게 나오는 버그수정 (09.05.08)
#define PBG_FIX_FENRIR_GELENALATTACK					// 펜릴을 타고 일반공격이 안되는 현상 (09.04.17)
#define PBG_FIX_SKILL_RECOVER_TOOLTIP					// 회복스킬의 툴팁상의 스킬 공격력이 출력되고 있는 버그(09.04.20)
#define PBG_FIX_SETITEM_4OVER							// 세트 아이템의 4종 초과하여 장착했을 경우 버그(09.04.24)



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.05.19 테섭/2009.05.21 본섭] - 이동석

// - 안상규
#define ASG_FIX_PICK_ITEM_FROM_INVEN					// 인벤창 특정 칸에서 아이템을 집어들었을 때 캐릭정보창에서 능력치가 변동되는 버그(2009.04.15) wops_33567
#define ASG_FIX_ARROW_VIPER_BOW_EQUIP_DIRECTION			// 마을에서 에로우바이퍼보우 착용 방향 버그 수정.(2009.04.17) wops_33718
#define ASG_FIX_MOVE_WIN_MURDERER1_BUG					// 무법자(카오) 레벨1일 때 맵 이동창 표시 버그.(2009.04.20) wops_34498

// - 박종훈
#define PJH_FIX_4_BUGFIX_33							// wops_34066
#define PJH_ADD_PANDA_PET							// 펜더 펫

// - 김재희
#define KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM	// NPC상점에서 아이템을 구매할때, 처음에 두번클릭해야 아이템이 구입되는 버그수정 (09.04.16)
#define KJH_FIX_WOPS_K33695_EQUIPABLE_DARKLOAD_PET_ITEM	// 다크로드가 인벤에 펫을 소유하고 있을때, 툴팁에 착용불가로 나오는 버그수정 (09.04.27)
#define KJH_FIX_WOPS_K33479_SELECT_CHARACTER_TO_HEAL		// 캐릭터에게 둘러쌓였을때, 요정의 치료스킬강화이후 캐릭터 선택이 잘 안되어지는 버그수정 (09.04.27)
#define KJH_FIX_DARKLOAD_PET_SYSTEM					// 개인상점구입창에서 다크로드펫의 툴팁이 정확하게 출력되지 않는 버그수정 (09.04.16)
													// 펫시스템 수정 - 장착하고있는 펫과, 현재 보유하고있는 펫을 구분 (09.04.16)

// - 박보근
#define PBG_FIX_REPAIRGOLD_DARKPAT					// 다크호스,스피릿 수리비 버그 수정(09.04.14) - wops_34069
#define PBG_FIX_SHIELD_MAGICDAMAGE					// 방패착용시 정보창에 마력이 상승하는 버그 수정(09.04.15) - wops_34618

// - 이동석
#define LDS_FIX_SYNCRO_HEROATTACK_ACTION_WITH_SERVER		// 흑기사 스킬 시전시 서버에서 스킬 받기전에 미리 스킬발동이 걸려 이전 스킬 에니메이션이 잘못 출력. (09.04.22) wops_33863
#define LDS_FIX_MASTERSKILLKEY_DISABLE_OVER_LIMIT_MAGICIAN	// 마스터 스킬 강화 된 스킬들에 대해 요구치 미달시에 이용 불가. (09.04.22) no_wops
#define LDS_FIX_SETITEM_WRONG_CALCULATION_IMPROVEATTACKVALUE// 세트아이템착용 이후 사랑의 묘약을 먹으면, 케릭터 정보창에 힘증가 스탯수치가 다른현상. (09.04.23) wops_35131

// - 양대근
#define YDG_FIX_MEMORY_LEAK_0905						// 메모리 누수 제거 (2009.05.11)
#define YDG_MOD_PROTECT_AUTO_V5_KEYCHECK_OFF			// 아이로봇 오토 막기 시리즈5 키 체크 부분 제거 (2009.05.11)
#define YDG_FIX_INVALID_SET_DEFENCE_RATE_BONUS			// 방어구세트 추가 방어력표시 방어력의 10%가 아니라 방어율의 10%임 (2009.04.15) wops_32937
#define YDG_FIX_STAFF_FLAMESTRIKE_IN_CHAOSCASLE			// 카오스캐슬에서 마검사 지팡이 착용시 플레임스트라이크와 블러드어택 아이콘색 문제 (2009.04.15) wops_34747
#define YDG_FIX_LEFTHAND_MAGICDAMAGE					// 마검사가 지팡이나 룬바스타드를 왼손에 찼을때 마력이 올라가는 문제 (2009.04.15) wops_32641
#define YDG_FIX_BLOCK_STAFF_WHEEL						// 마검사 지팡이, 맨손 회오리베기 막기 (2009.05.18)



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.05.07 본섭] - 이동석
// [2009.04.30 테섭] - 안상규
// - 양대근
#define YDG_FIX_NONAUTO_MOVE_DEVIAS2			// 오토 아닌 유저도 Home/Ins누르면 데비2로 이동하는 버그 (2009.04.29) [2009.04.30 테섭]


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.04.23 본섭] - 안상규
// - 양대근
#define YDG_MOD_PROTECT_AUTO_V4_R3				// 아이로봇 오토 막기 시리즈4 - 암호화 알고리즘 복잡하게 (2009.04.16) [2009.04.23 본섭]
// [2009.04.22 테섭] - 안상규
// - 양대근
#define YDG_MOD_PROTECT_AUTO_V5					// 아이로봇 오토 막기 시리즈5 - 오토툴 실행여부 체크 - 서버연동 (2009.03.25) [2009.04.22 테섭]
#define YDG_ADD_MOVE_COMMAND_PROTOCOL			// 이동명령 프로토콜 추가(/이동 제거) (2009.04.01) [2009.04.22 테섭]


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.04.16 본섭] - 안상규
// [2009.04.14 테섭] - 안상규
// - 박보근
#define PBG_FIX_PKFIELD_CAOTIC					// 카오틱 디세이어 탈 것에서 버그 수정(09.04.07) [2009.04.13 테섭]
#define PBG_FIX_GUILDWAR_PK						// PKFIELD에서 길드워 중에 일반인 공격안받는 버그 수정(09.04.07) [2009.04.13 테섭]

// - 양대근
#define YDG_MOD_PROTECT_AUTO_V4_R2				// 아이로봇 오토 막기 시리즈4 - 암호화 정교화 (2009.04.09) [2009.04.13 테섭]
#define YDG_ADD_ENC_MOVE_COMMAND_WINDOW			// 암호화 내장 이동창 추가 (2009.04.13) [2009.04.13 테섭]
#define YDG_FIX_INVALID_TERRAIN_LIGHT			// 메모리 침범 버그 수정(맵툴 관련) (2009.03.30) [2009.04.13 테섭]

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.04.09 본섭] - 안상규
// [2009.04.06 테섭] - 안상규
// - 박보근
#define PBG_FIX_PKFIELD_ATTACK						// PKFIELD에서 길드원끼리 공격되는 현상(09.04.02) [2009.04.06 테섭]


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.04.02 본섭] - 박보근
// [2009.04.01 테섭] - 박보근
// [2009.03.31 테섭] - 박보근	// 버그 수정
// [2009.03.26 테섭] - 박보근
// - 최석근
#define CSK_FIX_UI_FUNCTIONNAME							// UI관련 함수명 변경(2009.01.22) [2009.03.26 테섭]
#define CSK_FIX_ANIMATION_BLENDING						// 애니메이션 블렌딩쪽 블렌드 안되게 옵션 주게 수정(2009.01.29) [2009.03.26 테섭]
#define CSK_FIX_ADD_EXEPTION							// dmp파일 분석해보니 getTargetCharacterKey 함수에서 튕기는걸로 예측되서 예외처리 추가(2009.01.29) [2009.03.26 테섭]

// [2009.03.19 테섭] - 박보근
// - 양대근
#define YDG_MOD_PROTECT_AUTO_V4							// 아이로봇 오토 막기 시리즈4 - 이동창 메모리 암호화 (2009.03.18) [2009.03.19 테섭]
//#define YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V4			// 아이로봇 오토 막기 시리즈4 플래그 검사 (2009.03.18) [2009.03.19 테섭-본섭시 주석]
#define CSK_MOD_PROTECT_AUTO_V2							// 아이로봇 오토 막기 시리즈2 - 프로세스 검출해서 먹통(2009.03.02) [2009.03.19 테섭]
//#define YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V2			// 아이로봇 오토 막기 시리즈2 플래그 검사 (2009.03.18) [2009.03.19 테섭-본섭시 주석]

#define YDG_ADD_NEW_DUEL_SYSTEM						// 새로운 결투 시스템 (2009.01.05) [09.03.19 테섭]
#ifdef YDG_ADD_NEW_DUEL_SYSTEM
	#define YDG_ADD_NEW_DUEL_UI							// 새로운 결투 관련 UI 추가 (2009.01.05) [09.03.19 테섭]
	#define YDG_ADD_NEW_DUEL_PROTOCOL					// 새로운 결투 관련 프로토콜 추가 (2009.01.08) [09.03.19 테섭]
	#define YDG_ADD_NEW_DUEL_NPC						// 새로운 결투 관련 NPC 타이투스 추가 (2009.01.19) [09.03.19 테섭]
	#define YDG_ADD_NEW_DUEL_WATCH_BUFF					// 새로운 결투 관전 버프 추가 (2009.01.20) [09.03.19 테섭]
	#define YDG_ADD_MAP_DUEL_ARENA						// 새로운 결투장 맵 추가 (2009.02.04) [09.03.19 테섭]
#endif	// YDG_ADD_PVP_SYSTEM

// - 이동근
#define LDK_FIX_INVENTORY_SPEAR_SCALE					// 인벤토리 무기-spear 크기 처리 if문 수정 [09.03.19 테섭]
#define LDK_ADD_GAMBLE_SYSTEM							// 젠소모 무기 뽑기 상점 (09.01.05) [09.03.19 테섭]
#ifdef LDK_ADD_GAMBLE_SYSTEM
	#define LDK_ADD_GAMBLE_NPC_MOSS						// 겜블 상인 모스 [09.03.19 테섭]
	#define LDK_ADD_GAMBLE_RANDOM_ICON					// 겜블용 랜덤무기 아이콘 [09.03.19 테섭]
	#define LDK_ADD_GAMBLERS_WEAPONS					// 겜블전용 레어 무기들 [09.03.19 테섭]
#endif //LDK_ADD_GAMBLE_SYSTEM

// - 김재희
#define KJH_FIX_MOVE_HERO_MOUSE_CLICK_EFFECT			// 캐릭터 이동시 마우스클릭 이팩트가 이동하지 못하는 곳에 렌더되는 버그 수정 (09.03.04) [09.03.19 테섭]
#define KJH_FIX_GET_ZEN_SYSTEM_TEXT						// 겜블상점에서 아이템구입시 젠증가 메세지가 나오는 문제. (09.03.18) [09.03.19 테섭]

// - 박보근
#define PBG_ADD_PKFIELD									// PK필드 추가(2008.12.29) [09.03.19 테섭]
#define PBG_ADD_HONOROFGLADIATORBUFFTIME				// 검투사의 명예 버프 시간 툴팁 추가(2009.03.19) [09.03.19 테섭]



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.03.17 본섭] - 박보근
// [2009.03.12 테섭] - 박보근
// - 박보근
#define PBG_FIX_CHAOSCARD								// 카오스 카드 창닫는 패킷 안보내는 문제 수정/프리미엄 아이템 거래,기타등 불가하게 처리
#define CSK_CHAOS_CARD									// 국내 프리미엄 아이템판매 3차(카오스카드 - 복권)

// - 양대근
#define YDG_MOD_PROTECT_AUTO_V3							// 아이로봇 오토 막기 시리즈3 - 랜덤 휠 먹통+스크롤 투명 (2009.03.11) [09.03.11 테섭]
//#define YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V3			// 아이로봇 오토 막기 시리즈3 플래그 검사 (2009.03.11) [09.03.11 테섭-본섭시 주석]

// - 이동석
#define LDS_FIX_NONINITPROGLEM_SKILLHOTKEY				// 간혹 신규케릭터 이후 바로이전 로그인했던 케릭스킬단축키가 영향주는 오류. MainScene호출마다 SkillKey 테이블 초기화. (2009.01.20)
#define LDS_FIX_NONINIT_TEXTBOLDTABLE					// 마스터스킬창의 TOOLTIP 렌덤 볼드 문제. 볼드 텍스트TABLE이 초기화가 되지 않아 이전 TEXT에서 사용되었던 항목별 BOLD 여부 값이 그대로 적용되는 오류. (2009.02.12)
#define LDS_FIX_GLPARAM_SWAPMODE_BITMAP					// Wrong OpenGL Texture Parameter SWAPMode  (2009.02.19)



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.03.10 본섭] - 박보근
// [2009.03.05 테섭] - 박보근
// - 최석근
#define CSK_MOD_PROTECT_AUTO_V1						// 아이로봇 오토 막기 시리즈1 - 맵창 상단여백, 알파, 창크기랜덤(2009.02.24)
#define CSK_FIX_BLUELUCKYBAG_MOVECOMMAND			// 파란복주머니 사용하면 10분간 칼리마 이용가능하게 맵창수정(2009.02.27)
#define CSK_FIX_MACRO_MOVEMAP						// 매크로로 /이동 맵 해서 이동하는 내용 수정(2009.02.27)

// - 김재희
#define KJH_FIX_MOVECOMMAND_WINDOW_SIZE				// 이동명령창 크기변경시 빈공간이 생기는 문제 수정 (09.03.02)

// 본섭 패치시에 추가된 내용 - 박보근
#define CSK_MOD_REMOVE_AUTO_V1_FLAG					// 오토 막기 시리즈1 플래그 막는 처리(2009.03.09)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.02.24 본섭] - 양대근
// [2009.02.19 테섭][2009.02.20 테섭] - 양대근
#define CSK_MOD_MOVE_COMMAND_WINDOW					// 맵이동창 오토마우스 막기 위해 수정(2009.02.13) [09.02.19 테섭]
#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
	#define CSK_FIX_GM_MOVE								// GM명령어는 먹게 수정(2009.02.20) [09.02.20 테섭]
	#define CSK_FIX_GM_FORTV							// GM판별에서 FORTV용이 빠진 버그(2009.02.20) [09.02.24 본섭]
	#define CSK_FIX_GM_WEBZEN_ID						// GM판별에서 webzen id쓰는 캐릭터가 빠진 버그(2009.02.23) [09.02.24 본섭]
	#define CSK_FIX_PCROOM_POINT_ZEN_BUG				// PC방 포인트 상점에서 PC방 아닐때 포인트가 젠으로 표시되는 문제(2009.02.23) [09.02.24 본섭]
#endif // CSK_MOD_MOVE_COMMAND_WINDOW

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.01.20 본섭] - 이동근
// [2009.01.15 테섭] - 이동근

// - 이동근
//#if SELECTED_LANGUAGE == LANGUAGE_KOREAN
#ifdef _LANGUAGE_KOR					// 국내만 적용
	#define LDK_ADD_CASHSHOP_FUNC						// 해외 부분 유료 처리 함수 관리및 함수 구조 변경 (#define PSW_PARTCHARGE_ITEM1)
	#define PSW_GOLDBOWMAN								// 1차 프리미엄 아이템 판매 npc(일본 전용 황금 궁수 복권 이벤트)
	#define YDG_ADD_GOLDBOWMAN_INVENTORY_SPACE_CHECK	// 1차 프리미엄 아이템 판매 npc 인벤토리 검사(일본 전용 황금 궁수 인벤토리 검사 (2*4) (2008.12.02))
	#define LDK_ADD_PC4_GUARDIAN						// 1차 프리미엄 아이템 (데몬만 사용, 수호천사 사용안함)
	#define	LDK_FIX_GUARDIAN_CHANGE_LIFEWORD			// 1차 프리미엄 아이템 (내구력 표기 -> 생명으로 수정)(2009.01.14)
	#define LDK_ADD_PC4_GUARDIAN_EFFECT_IMAGE			// 1차 프리미엄 아이템용 이펙트(lightmarks.jpg, effect #define ADD_SOCKET_ITEM 와 중복 되는것 있음... )
	#define PSW_ADD_PC4_CHAOSCHARMITEM					// 1차 프리미엄 아이템 (카오스 조합 부적)
//#endif //SELECTED_LANGUAGE == LANGUAGE_KOREAN
#endif // _LANGUAGE_KOR					// 국내만 적용

// #endif //SELECTED_LANGUAGE == LANGUAGE_KOREAN
#endif // _LANGUAGE_KOR								// 국내




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//## [전 국가(해외까지) 패치이후 디파인]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2009.04.13 일본 1.03.06]	- 박보근	[일본 6차과금 업데이트 내부 테섭]

// [2009.01.20 본섭] - 이동근
// [2009.01.15 테섭] - 이동근
// - 김재희
#define KJH_ADD_INVENTORY_REPAIR_DARKLOAD_PET		// 다크로트 펫 인벤토리에서 수리 (09.01.09)

// - 박종훈
#define PJH_FIX_CAOTIC								// 카오틱 디세이어 수정(2008.12.02 ~)

// - 안상규
#define ASG_ADD_STORMBLITZ_380ITEM					// 스톰블리츠 세트 380 아이템화(+10 이상 효과 추가)(2008.12.3)

// - 이동석
#define LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL		// MemoryLeak발생 수정.(BMD::Open2, OpenPlayers) (2008.12.17)
#define LDS_FIX_OPENGL_STACKOVERFLOW_STACKUNDERFLOW	// glPushMatrix-glPopMatrix 상에 오류로 glGetError 로 스택 오버플로우 스택 언더플로우 발생 수정 (2008.12.23)
#define LDS_ADD_CHARISMAVALUE_TOITEMOPTION			// 통솔수치 세트아이템 연산에 미반영되어 출력되어 추가(ex>다크케릭의 브로이의 세트 풀셋시 통솔 미적용 현상) (2009.01.08)

// - 최석근
#define CSK_ADD_GM_ABILITY							// GM기능 강화(2008.11.26)

// [2009.02.17 본섭] - 양대근
// [2009.02.12 테섭] - 양대근
#define YDG_MOD_GOLDEN_ARCHER_ANGEL_TEXT				// 황금 궁수 관련 수호정령 텍스트 추가 (09.02.11) [09.02.12 테섭]
#define YDG_FIX_LOCK_MIX_BUTTON_WHILE_MIXING			// 조합인벤토리 조합중 조합버튼 다시 못누르게 잠금 (09.02.11) [09.02.12 테섭]
#define YDG_FIX_TRAINER_MIX_DISPLAY_REQUIRED_ZEN		// 조련사 조합시 젠표시 되도록 수정 (09.02.11) [09.02.12 테섭]

// [2009.02.03 본섭] - 양대근
// [2009.01.29 테섭] - 이동근
#define LDK_FIX_CHAOSCHARMITEM_TOOLTIP			// 카오스 조합 부적 툴팁 추가 (09.01.29)

// [2009.02.24 본섭] - 양대근
// [2009.02.19 테섭][2009.02.20 테섭] - 양대근
#define KJH_FIX_REPAIR_DARKLOAD_PET_DURABILITY_ZERO	// 다크로드 펫이 내구도 0일때 가격수정 (09.02.11) [09.02.19 테섭]


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2008.12.18 일본 1.03.04]	
// [2008.12.30 본섭]			- 최석근
// [2009.01.22 베트남 1.03.00]	- 안상규
// [2009.03.04 대만 1.03.00]	- 최석근
// [2009.03.06 일본 1.03.06]	- 최석근
// [2009.03.10 중국 1.03.00]	- 이동석

// - 안상규
#define ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT	// Text.txt에서 %를 %%로 일괄 처리로 인한 수정.(2008.12.12)

// - 최석근
#define CSK_REF_BACK_RENDERITEM					// 등에 달라붙는 아이템 관련 리팩토링(2008.11.06)
#define CSK_FIX_SYNCHRONIZATION					// 동기화 문제(2008.09.24)
#define CSK_FIX_FENRIR_RUN						// 펜릴 다른 유저가 있을 경우 달리다가 걷는 버그 (2008.11.24)
#define CSK_FIX_AIDAMONSTER_DIESPEED			// 아이다 몬스터 2종 죽는 속도 조절(2008.12.03)
#define CSK_FIX_BACKUPITEM_DAMAGE				// 몬스터한테 공격당하면 들고 있는 아이템 원래자리로 돌아가는 버그(2008.12.12)
#define CSK_FIX_MAGICALPOWER_INCREASE			// "/마력증대법서" 설명 이상하게 나오는 버그(2008.12.02)
#define CSK_FIX_DUEL_N_PK_SKILL					// 결투중이나 강제PK시 스킬관련 버그들(2008.10.07)

// - 김재희
#define KJH_FIX_SWELLOFMAGIC_EFFECT				// 마력증대 이팩트가 안나오는 문제 (08.11.25)
#define KJH_FIX_BOW_ANIMATION_ON_RIDE_PET		// 펫을타고 활을쏠때 애니메이션수정 (08.11.19)
#define KJH_FIX_DOUBLECLICK_BUY_ITEM			// 아이템구매시에 더블클릭해야 아이템을 구매할수있는문제 (08.11.24)

// - 이동석
#define LDS_FIX_OUTPUT_WRONG_EQUIPPEDSETITEMOPTION_BY_WRONG_SLOTINDEX	// 특정 SetItem의 출력 세트 옵션 갯수가 틀린 현상. (2008.12.19)
#define LDS_OPTIMIZE_FORLOADING_UNNECESSARY_SHADOWVOLUME				// 로딩시 쉐도우 볼륨을 위한 Edge계산 하는 부분을 현재 쉐도우 볼륨 미사용 중으로 불필요함. (08.11.27)

// - 양대근
#define YDG_FIX_MACE_FLAMESTRIKE					// 둔기 사용시 플레임스트라이크 사용 못하는 문제 (2008.10.23) [패치안됨]
#define YDG_FIX_SOCKETSPHERE_MIXRATE				// 시드스피어 합성 조합 확률 잘못 나오는 문제 (2008.10.24) [패치안됨]
#define YDG_FIX_MASTERLEVEL_ELF_ATTACK_TOOLTIP		// 요정 마스터레벨 공방 향상 툴팁에 잘못 나오는 문제 (2008.10.29) [패치안됨]
#define YDG_FIX_DOUBLE_SOCKETITEM_BONUS				// 비약define 사용시 소켓 아이템 보너스가 두번 더해지는 문제 (2008.11.06) [패치안됨]
#define YDG_FIX_HELLBUST_SET_ENERGY_BONUS_BUG		// 세트로 에너지 올리고 헬버스트 배우고 세트 벗으면 헬버스트 발사하다 멈추는 문제 (2008.11.06) [패치안됨]
#define YDG_FIX_CAOTIC_SOUND_MISSING				// 카오틱디세이어 스킬 사운드 빠진것 수정 (2008.12.02) [패치안됨]
#define YDG_FIX_LEAN_AGAINST_WALL_WITHOUT_ARROWS	// 화살없이 기대기 안되는 버그 (2008.12.02) [패치안됨]
#define YDG_FIX_DARKLORD_SET_EFFECT_WITH_DARKSPIRIT	// 다크로드 다크스피릿 들고 있을때  세트 이펙트 안나오는 버그 (2008.12.02) [패치안됨]
#define YDG_FIX_SANTA_INVITAION_REPAIR				// 산타 초대장 수리되는 버그 (2008.12.18) [패치안됨]
#define YDG_FIX_SOCKET_BONUS_MAGIC_POWER_BONUS		// 소켓아이템 세트옵션 마력상승->공격력/마력상승으로 수정됨 (2008.12.18) [패치안됨]

// - 박보근
#define PBG_WOPS_REPAIRKEY							// 조련사L버튼 클릭시 수리가 되는 현상(08.11.19)
#define PBG_WOPS_HELBUST							// 헬버스트 시전중에 요구치하락관련문제(08.11.21)
#define PBG_WOPS_INVENCHECK							// 창고 체크시에 아이템x크기를 다음줄체크로 크기가 나올경우 검사중단되는 현상(2008.12.3)
#define PBG_WOPS_DARKHOSE_ATTACK					// 다크호스타고 일반공격을 할경우 안된는 현상(08.12.05)

// - 이동석
#define LDS_FIX_SETITEMEFFECT_WHENFULLSET_SOMEMISSEDEFFECT		// 신규로 추가된 Action 몇가지에서 풀세트 이펙트가 미출력되는 문제. (2008.04.25)
#define LDS_FIX_WRONG_CALCULATEEQUIPEDITEMOPTIONVALUE			// 케릭터 정보창에 아이템 기본 옵션 수치가 틀린 오류 수정. (장착 아이템기본옵션 수치가 케릭터수치에 상관없이 모두 더해져서 연산되는 문제) (2008.11.21)
#define LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE				// 착용중인 세트 아이템들의 추가 옵션 출력상 누락되는 세트 발생으로 수정. (2008.11.28)
#define LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE	// 착용중인 세트 아이템이 추가 옵션 리스트 수의 제한 오류 수정. (2008.12.04)
#define LDS_FIX_MODULE_POSTMOVECHARACTER_SKILL_WHEEL			// POSTMOVECHARACTER 모듈 적용. 회오리베기 사용후 일반공격시 회오리잔영 남는 문제[다른케릭버전](2008.12.04) [LDS_FIX_WHENBLOODATTACK_DISSAPEARRIGHTSWORD 작업에 추가]

// - 이동근 
#define LDK_FIX_USING_ISREPAIRBAN_FUNCTION		// 있는 함수를 활용합시다.. (리페어 제외 정보 확인하는 함수(isRepairBan) ) (08.12.02)	// 리페어 불가능등록시 이함수에만 적용하면됨.. (부분 유료 일경우 부분 유료 함수(IsPartChargeItem)에만 적용시키면 됨)
#define LDK_FIX_BUFFSKILL_AUTOATTACK_CANCLE		// 소환술사 버프스킬 자동공격 취소.(08.12.02)	// 데미지반사, 버서커, 슬립,블라인드, 웨이크니스, 이너베이션 스킬관련
#define LDK_FIX_MODIFY_BUFFTIME_CHECK			// 버프 시간 계산 방식을 tickCount로 변경 클라이언트 비활성화시도 버프 시간 계산하도록 한다.(08.12.02)
#define LDK_FIX_USE_RECOVER_TARGET_MONSTER		// 회복 스킬 타셋 조건 수정 : 몬스터에게는 사용 못함(08.12.05)
#define LDK_FIX_STORAGE_CHARGE					// 380레벨 창고 사용료 계산공식을 서버와 같도록 수정.(08.12.05)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2008.12.16 본섭] - 최석근

// - 이동근
#define LDK_ADD_NEW_PETPROCESS					// 펫, 소환물 관리 프로세서.
#define LDK_ADD_NEW_PETPROCESS_ADD_SOUND		// 펫 사운드 관련 함수 추가(2008.11.5)
#define	LDK_FIX_NOT_CREATE_NEW_PET				// 뷰포트의 펫 생성관련 조건 변경(수호정령, 사탄 은 자신만 보임.) (2008.11.18)
#define LDK_FIX_HIDE_PET_TO_NOT_MODEL_PLAYER	// model_player가 아닐때 펫을 안그림. 수정(2008.11.18)

// 크리스마스 이벤트 ////////////////////////////////////////////////////////////////
#define ADD_CHRISTMAS_EVENT_2008			// 크리스마스 이벤트 (2008.11.28) [2008.12.16 본섭]

#ifdef ADD_CHRISTMAS_EVENT_2008
	#define YDG_ADD_FIRECRACKER_ITEM		// 폭죽아이템 추가 (2008.09.23)
	#define YDG_ADD_MAP_SANTA_TOWN			// 산타마을 추가 (2008.10.13)
	#define YDG_ADD_SANTA_MONSTER			// 다크산타 몬스터 추가 (2008.10.13)
	
	#define LDK_ADD_RUDOLPH_PET				// 루돌프 펫 (2008.09.30)
	#define LDK_ADD_SNOWMAN_NPC				// 눈사람 NPC(2008.10.27)
	#define LDK_ADD_SNOWMAN_CHANGERING		// 눈사람 변신 반지(2008.10.20)
	#define LDK_ADD_SANTA_NPC				//산타마을의 착한 산타 (2008.10.27)

	#define PBG_ADD_SANTABUFF				// 산타버프관련(2008.10.22)
	#define PBG_ADD_SANTAINVITATION			// 산타마을초대장(2008.10.22)
	#define PBG_ADD_LITTLESANTA_NPC			// 리틀산타 NPC1~8(2008.10.22)

	#define YDG_ADD_SANTABUFF_MAGIC_DAMAGE	// 산타버프 공격력 증가->마력,저주력도 증가로 바뀜(2008.12.18)
#endif	// ADD_CHRISTMAS_EVENT_2008

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2008.11.20 테섭] - 김재희

// - 안상규
#define ASG_ADD_ELF_ARROW_PLUS3					// 요정 화살+3, 석궁용+3 추가					(2008.11.13)

// - 최석근 
#define CSK_FIX_DEVIAS_REDCARPET				// 데비아스 레드카펫에서 파티신청 불가능 현상	(2008.11.10)
#define CSK_FIX_ITEMTOOLTIP_POS					// 인벤토리 아이템 툴팁 위치 수정				(2008.11.06)

// - 김재희
#define KJH_FIX_SKILL_REQUIRE_ENERGY			// 베기스킬 에너지 요구치 수정					(2008.11.19)

// - 이동근
#define LDK_FIX_REPAIR_CURSOR_FREEZE			// 인벤토리창 닫을시 수리망치 커서로 고정되는 버그	(2008.11.12)

// - 박보근
#define PBG_WOPS_DARKLOAD						// 펜릴 땅클릭시 쿨타임은 돌아가는 현상			(2008.11.11)

// - 이동석
#define LDS_FIX_RECALL_CREATEEQUIPPINGEFFECT		// 세트 아이템 특정 장비를 들었다 놓기를 여러번 반복시 수치가 한번더 연산되는 문제 (2008.11.17) [LDK_FIX_RECALL_CREATEEQUIPPINGEFFECT 작업과 동일] (33059)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2008.11.18 본섭] - 김재희

// - 박종훈
//#if SELECTED_LANGUAGE == LANGUAGE_KOREAN
#ifdef _LANGUAGE_KOR					// 국내만 적용
	#define PJH_NEW_SERVER_ADD					//새로운서버 작업(로렌섭)			(2008.11.11)
//#endif //SELECTED_LANGUAGE == LANGUAGE_KOREAN
#endif // _LANGUAGE_KOR					// 국내만 적용


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2008.11.14 테섭] - 김재희

// - 최석근
#define CSK_ADD_ITEM_CROSSSHIELD					// 다크로드 방패 크로스실드 추가	(2008.11.03)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2008.11.13 테섭] - 김재희

// - 김재희, 박보근
#define KJH_PBG_ADD_SEVEN_EVENT_2008				//상용화 7주년 이벤트				(2008.10.30)

// - 김재희
#define KJH_FIX_INVENTORY_REPAIR_COST				// 인벤토리 자동수리금액 공식 수정	(2008.11.13)

// - 이동석
#define LDS_FIX_WHENAFTERSKILL_WHEEL_STILLVISUAL_EFFECT	// 회오리 베기 사용이후 일반 공격시 여전히 회오리 베기 잔영이 남는 문제 (2008.10.29) [LDS_FIX_WHENBLOODATTACK_DISSAPEARRIGHTSWORD 작업에 관련 있음]



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2008.10.24 테섭]

// - 박보근
#define PBG_FIX_DEFENSEVALUE_DARK						//마검사 10셋이상 세트방어력이 표현안되것과 오로라 안보이는 버그 수정(08.10.16)
#define PBG_FIX_CHAOS_GOLDSWORD							//카오스캐슬 정보창 열고닫으면 파괴의검이 황금색으로 변하는 버그 수정(08.10.17)
#define PBG_FIX_SKILL_DEMENDCONDITION					//스킬의 스텟요구치와 캐릭의 스텟량 장비(미)착용 실시간 체크후 사용여부결정.
														//skill의 Resource를 형변경으로 인해 mu_tools를 새 컴파일 해야함.skilltest.bmd를 만들고 사용해야함.(QA전달08.9.30)

// - 이동석 
// (블루 1.00.38 에서 문제가 생겨 주석처리)
// #define LDS_OPTIMIZE_FORLOADING						// Optimize For Loading(2008.10.9) [DO_PROFILING_FOR_LOADING Define 키면 확인 가능]
#define LDS_FIX_WHENBLOODATTACK_DISSAPEARRIGHTSWORD	// 블러드 어택 오른손 착용 검이 보이지 않던 버그 수정 (2008.10.14) [CSK_FIX_WHEELSKILL_ITEM_RENDER작업에 관련 있음]

// - 박종훈
#define PJH_FIX_BLOOD_ATTCK							// 블러드 어택 버그수정 (2008. 09. 23)
#define PJH_SEASON4_FIX_MULTI_SHOT	

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------
// [2008.09.18 테섭]

// - 박종훈
//#if SELECTED_LANGUAGE == LANGUAGE_KOREAN
#ifdef _LANGUAGE_KOR					// 국내만 적용
	#ifndef PJH_NEW_SERVER_ADD 
		#define PJH_FINAL_VIEW_SERVER_LIST2				//서버통합2차 작업(2008. 09. 23) [2008.10.08 본섭]
	#endif //PJH_NEW_SERVER_ADD
//#endif //SELECTED_LANGUAGE == LANGUAGE_KOREAN
#endif // _LANGUAGE_KOR					// 국내만 적용

// - 최석근
#define CSK_FIX_QWER								// QWER 핫키에 타입이랑 레벨 값이 깨지는거 수정(2008.09.18)
#define CSK_FIX_WHEELSKILL_ITEM_RENDER				// 회호리베기 스킬을 사용할 때 오른손 무기 안보이게 수정(2008.09.11)
#define CSK_FIX_JACK_PARTICLE						// 잭오랜턴변신반지 차고 "/이마"치면 튕기는 버그(2008.09.10)
#define CSK_FIX_EPSOLUTESEPTER						// 앱솔루트셉터 펫 공격력 잘못표시되는 버그(2008.09.11)

// - 김재희
#define KJH_FIX_20080910_SPELL_MAGIC_IN_CHAOSCASTLE	// 카오스캐슬에서 마력증대스킬 사용안되는 문제 (2008.09.10)
#define KJH_FIX_20080910_NPCSHOP_PRICE				// 사원재료 판매가격이 실제랑 틀린문제 (2008.09.10)	

// - 양대근
#define YDG_FIX_SOCKET_SHIELD_BONUS						// 소켓 시드 방패 방어력 증가 % 적용 안되는 문제 (2008.09.09) [2008.09.18 테섭]
#define YDG_FIX_GIGANTIC_STORM_OPTIMIZE					// 기간틱스톰 이펙트 줄임 (2008.09.09) [2008.09.18 테섭]
#define YDG_FIX_WING_MIX_RATE							// 날개 조합시 엑템으로 인식되는 문제 (2008.09.11) [2008.09.18 테섭]
#define YDG_FIX_ELSEIF_BLOCK_BREAK_AT_RENDER_PART_OBJECT_BODY_FUNC	// zzzObject.cpp의 RenderPartObjectBody() if/else if 너무 많아 쪼갬 (2008.09.16) [2008.09.18 테섭]

//----------------------------------------------------------------------------------------------
// [2008.09.11 테섭]

// - 김재희
#define KJH_FIX_20080904_INVENTORY_ITEM_RENDER		// 인벤토리에서 아이템 렌더위치 수정 (2008.09.04)	[2008.09.11 테섭]

// - 양대근
#define YDG_FIX_GIGANTIC_STORM_CRASH					// 기간틱 스톰 튕기는 현상 방지용 안전장치 추가 (2008.08.26) [2008.09.04 본섭]
#define YDG_FIX_SOCKETITEM_ISNOT_380ITEM				// 소켓 아이템은 380레벨 이상이어도 조합시 380 처리 안함 (2008.09.05) [2008.09.05 테섭]

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//## 시즌 4.0 패치
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 시즌 4.0 테섭 패치 이후 패치 
// - 김재희 
#define KJH_FIX_SOCKET_ITEM_ADD_DEFENCE_BONUS		// 마검사 소켓아이템 풀셋착용시 추가 방여력 적용	(2008.08.13) [2008.08.14 테섭]

// - 양대근
#define YDG_FIX_SOCKET_BALANCE_PATCH				// 소켓 밸런스 관련 옵션 변경 작업 (2008.08.14) [2008.08.14 테섭]

// 시즌 4.0 테섭 패치 이후 패치 (2008.08.12 패치)

// - 최석근
#define CSK_FIX_POTION_VALUE						// 큰치료물약과 큰마나물약 판매가격 1500되게 수정(2008.08.08) [2008.08.12 테섭]

// - 양대근
#define YDG_FIX_SOCKET_ITEM_DEFENCE_RATE_BONUS		// 소켓 방어성공율증가 %계산 수정(2008.08.08) [2008.08.12 테섭]

// 시즌 4.0 체험서버 패치 이후 패치 (8월 6일 패치)

// - 최석근
#define CSK_FIX_SKILLHOTKEY_PACKET					// 스킬 핫키의 10바이트에서 20바이트로 확장한다.(2008.08.05) [2008.08.06 체험테섭]
#define CSK_FIX_HIGHVALUE_MESSAGEBOX				// 고가의 아이템 판매 하겠냐고 묻는 메세지 박스 ok 버튼 빠르게 누르면 2번 뜨고
													// 2번째 ok 버튼을 누르면 장비창에서 오른쪽 손 무기를 자동으로 팔아버리는 버그(2008.07.31) [2008.08.06 체험테섭]
// - 양대근
#define YDG_FIX_BLOWOFDESTRUCTION_EFFECT_BUG		// 파괴의일격 데스센츄리온 버프색 버그					(2008.07.29) [2008.08.06 체험테섭]

// - 김재희
#define KJH_FIX_ABSOLUTE_CEPTER_TOOLTIP				// 엡솔루트셉터 펫공격력상승 툴팁 안나오는 현상			(2008.08.04) [2008.08.06 체험테섭]
#define KJH_FIX_MULTISHOT_RELOAD_ARROW				// 멀티샷 사용후 화살통이 자동으로 교체되지 않는 현상	(2008.08.04) [2008.08.06 체험테섭]


// 시즌 4.0 체험서버 패치 이후 패치 (7월 31일 패치)
// - 양대근
#define YDG_FIX_SPLIT_ATTACK_FUNC					// zzzInterface.cpp Attack()함수 클래스별 쪼개기		(2008.07.29) [2008.07.31 체험테섭]
// - 김재희
#define KJH_FIX_ARRAY_DELETE						// new array잡아놓고 delete array 안한 문제				(2008.07.29) [2008.07.31 체험테섭]
#define KJH_MODIFY_SOCKET_ITEM_COLOR				// SocketItem 칼라 변경									(2008.07.29) [2008.07.31 체험테섭]
#define KJH_FIX_THUNDER_EFFECT_IN_IKARUS			// 이카루스에서 천둥칠때 VectorRotate()에서 in,out인자를 같은변수를 넘긴 문제 (2008.07.30) [2008.07.31 체험테섭]


// 시즌 4.0 체험서버 패치 이후 패치 (7월 29일 패치)
// - 양대근
#define YDG_FIX_SCRIPT_LEVEL_VALUE					// 소켓 스크립트 스피어레벨별 수치값 추가 확장			(2008.07.25) [2008.07.29 체험테섭]
#define YDG_FIX_SOCKET_ATTACH_CONDITION				// 소켓 장착 시드 종류별 제한 추가						(2008.07.25) [2008.07.29 체험테섭]


// 시즌 4.0 체험서버 패치 이후 패치 (7월 25일 패치)
// - 양대근
#define YDG_FIX_RARGLE_CRASH						// 소환술사 라글 펄루션 스킬 사용시 튕기는 버그 수정	(2008.07.17) [2008.07.25 체험테섭]
#define YDG_ADD_SKILL_FLAME_STRIKE_SOUND			// 마검사 신규 스킬 플레임 스트라이트 사운드			(2008.07.18) [2008.07.25 체험테섭]
#define YDG_ADD_SKILL_GIGANTIC_STORM_SOUND			// 마검사 신규 스킬 기간틱 스톰 사운드					(2008.07.18) [2008.07.25 체험테섭]
#define YDG_ADD_SKILL_LIGHTNING_SHOCK_SOUND			// 소환술사 스킬 라이트닝 쇼크(새버전) 추가 사운드		(2008.07.18) [2008.07.25 체험테섭]
#define YDG_FIX_BLOCK_SKILL_MOVE					// 신규스킬 사용중 이동못하게 수정						(2008.07.22) [2008.07.25 체험테섭]
// - 최석근
#define CSK_ADD_SKILL_BLOWOFDESTRUCTION_SOUND		// 흑기사 파괴의 일격 사운드							(2008.07.22) [2008.07.25 체험테섭]
#define CSK_FIX_SKILL_BLOWOFDESTRUCTION_COMBO		// 흑기사 파괴의 일격 콤보 스킬 수정					(2008.07.22) [2008.07.25 체험테섭]
#define CSK_FIX_SKILL_ITEM							// 신규 스킬 수정(전직시에만 배울수 있게 수정)			(2008.07.24) [2008.07.25 체험테섭]
// - 안상규
#define ASG_ADD_SKILL_BERSERKER_SOUND				// 소환술사 스킬 버서커 사운드							(2008.07.18) [2008.07.25 체험테섭]
#define ASG_ADD_SUMMON_RARGLE_SOUND					// 소환수 라글 스킬 펄루션 사운드.						(2008.07.18) [2008.07.25 체험테섭]

// - 김재희
#define KJH_ADD_SKILL_SWELL_OF_MAGICPOWER_SOUND		// 마법사 마력증대스킬 사운드.							(2008.07.22) [2008.07.25 체험테섭]
#define KJH_FIX_RELOAD_ARROW_TO_CROSSBOW			// 석궁화살이 자동장착 안돼는 문제.						(2008.07.23) [2008.07.25 체험테섭]
#define KJH_FIX_EQUIPED_FULL_SOCKETITEM_APPLY_TO_STAT	// 마검사가 신규 소켓 아이템을 풀장비 하였을때 방어율 안오르는 문제 (2008.07.23) [2008.07.25 체험테섭]
#define KJH_FIX_RUNE_BASTARD_SWORD_TOOLTIP			// 마검사 룬바스타드소드 툴팁(마력) 수정				(2008.07.23) [2008.07.25 체험테섭]
#define KJH_FIX_DARKSTINGER_MULTISHOT_SKILL			// 다크스팅거 다발스킬 3에서 4로 수정					(2008.07.23) [2008.07.25 체험테섭]




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//## 시즌 4.0 체험서버 패치된 내용
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//## 마스터 레벨
	// - 박종훈
	#define PJH_SEASON4_MASTER_RANK4			// 마스터레벨 4랭크 작업(2008.05.26) [2008.07.17 체험테섭]
#ifdef EXP_SERVER_PATCH		// 체험서버용. 프로젝트 세팅에 정의되어 있음.
	#define PJH_USER_VERSION_SERVER_LIST		// [2008.07.17 체험테섭]
#endif	// EXP_SERVER_PATCH

//## 소켓 시스템
	#define SOCKET_SYSTEM						// 소켓 시스템 (2008.02.27) [2008.07.17 체험테섭]
	#ifdef SOCKET_SYSTEM
		// - 양대근
		#define MODIFY_SOCKET_PROTOCOL				// 소켓관련 서버 프로토콜 수정 (2008.06.02) [2008.07.17 체험테섭]
		#define ADD_SOCKET_MIX						// 소켓 아이템 관련 조합 추가 (2008.03.10) [2008.07.17 체험테섭]
		#define ADD_SEED_SPHERE_ITEM				// 시드, 스피어 아이템 추가 (2008.03.31) [2008.07.17 체험테섭]
		#define ADD_SOCKET_STATUS_BONUS				// 소켓관련 캐릭터 스탯 수치 증가 (2008.07.04) [2008.07.17 체험테섭]
		// - 김재희
		#define ADD_SOCKET_ITEM						// 소켓 아이템 추가 (2008.03.27) [2008.07.17 체험테섭]
	#endif	// SOCKET_SYSTEM

//## 라클리온
	#define CSK_ADD_MAP_ICECITY					// 라클리온(얼음도시) 추가 작업(2008.04.10) [2008.07.17 체험테섭]
	#ifdef CSK_ADD_MAP_ICECITY
		// - 최석근
		#define CSK_RAKLION_BOSS					// 라클리온 맵 보스작업(2008.04.16)[2008.07.17 체험테섭]
		#define CSK_REPAIR_MAP_DEVIAS				// 데비아스에서 라클리온으로 이동 가능하게 수정(2008.04.24)[2008.07.17 체험테섭]
		#define CSK_FIX_MONSTERSKILL				// MonsterSkill.txt 확장 수정(2008.05.22)[2008.07.17 체험테섭]
		// - 박종훈
		#define PJH_GIANT_MAMUD						// 라클리온 몬스터 자이언트 메머드 추가작업 (2008.04.17) [2008.07.17 체험테섭]
		// - 안상규
		#define ADD_RAKLION_MOB_ICEGIANT			// 몬스터 아이스 자이언트 추가(2008.04.16)[2008.07.17 체험테섭]
		// - 양대근
		#define ADD_RAKLION_IRON_KNIGHT				// 라클리온 몬스터 아이언 나이트 추가 (2008.04.23) [2008.07.17 체험테섭]
		// - 이동석
		#define LDS_RAKLION_ADDMONSTER_ICEWALKER	// 라클리온 몬스터 아이스워커 추가 작업 (2008.05.26) [2008.07.17 체험테섭]
		#define LDS_RAKLION_ADDMONSTER_COOLERTIN	// 라클리온 몬스터 쿨러틴 추가 작업 (2008.05.26) [2008.07.17 체험테섭]
		// - 박상완
		#define PSW_ADD_MAPSYSTEM					// 맵 관리자( 2008. 05. 22 )[2008.07.17 체험테섭]
	#endif // CSK_ADD_MAP_ICECITY

//## 신규 스킬
	#define ADD_NEW_SKILL_SEASON4				// 새로 추가되는 스킬들은 이곳에 디파인 추가해 주세요.(2008.04.10) [2008.07.17 체험테섭]
	#ifdef ADD_NEW_SKILL_SEASON4
		// - 박종훈
		#define PJH_FIX_SKILL
		#define PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT		//요정새로운스킬 멀티샷(2008.05.26) [2008.07.17 체험테섭]
		#define PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER		//요정새로운스킬 회복(2008.06.09) [2008.07.17 체험테섭]
		#define PJH_SEASON4_DARK_NEW_SKILL_CAOTIC			//다크로드 새로운스킬 카오틱디세이어 (2008.06.13) [2008.07.17 체험테섭]
		// - 안상규
		#define ASG_ADD_SKILL_BERSERKER						// 소환술사 스킬 버서커 추가 (2008.05.15) [2008.07.17 체험테섭]
		// - 최석근
		#define CSK_ADD_SKILL_BLOWOFDESTRUCTION				// 흑기사 신규 스킬 파괴의 일격(2008.05.19) [2008.07.17 체험테섭]
		// - 양대근
		#define YDG_ADD_SKILL_FLAME_STRIKE					// 마검사 신규 스킬 플레임 스트라이트 (2008.05.15) [2008.07.17 체험테섭]
		#define YDG_ADD_SKILL_GIGANTIC_STORM				// 마검사 신규 스킬 기간틱 스톰 (2008.05.15) [2008.07.17 체험테섭]
		#define YDG_ADD_SKILL_LIGHTNING_SHOCK				// 소환술사 스킬 라이트닝 쇼크(새버전) 추가 (2008.06.10) [2008.07.17 체험테섭]
		#define YDG_FIX_ALICE_ANIMATIONS					// 소환술사 애니메이션 프레임 스킵 수정(player.bmd) (2008.06.16) [2008.07.17 체험테섭]
		// - 김재희
		#define KJH_ADD_SKILL_SWELL_OF_MAGICPOWER			// 흑마법사 마력증대 스킬 (2008.06.11) [2008.07.17 체험테섭]
		#define	KJH_FIX_LEARN_SKILL_ITEM_REQUIRE_STAT_CALC	// 법서로 스킬 배울때 요구치 버그 수정 (2008.07.11) [2008.07.17 체험테섭] [국내] 
															// - item.txt 수정요망 (스크립트담당자:이민정)
	#endif // ADD_NEW_SKILL_SEASON4

//## 기타
	// - 안상규
	#define ASG_ADD_SUMMON_RARGLE				// 소환수 라글 추가.(2008.05.27) [2008.07.17 체험테섭]
	// 아래 'PC방 포인트 시스템 수정'은 나중에 다시 되돌릴지 모르니 당분간 삭제 말것.
	#define ASG_PCROOM_POINT_SYSTEM_MODIFY		// PC방 포인트 시스템 수정.(2008.06.13) [2008.07.17 체험테섭]

	// - 양대근
	#define YDG_FIX_STONE_FLY					// 돌골렘등 죽을때 돌 멀리 날아가는 이펙트 버그 수정 (2008.06.04) [2008.07.17 체험테섭]
	#define YDG_FIX_VIEWPORT_HAND_CHECK			// 뷰포트 캐릭터 빈손일때 초기화 안되는 버그 수정 (2008.06.30) [2008.07.17 체험테섭]

	// - 박상완
	#define PSW_ADD_RESET_CHARACTER_POINT		//캐릭터 스텟 초기화(2008.07.11) [2008.07.17 체험테섭]

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//## 시즌 4.0 이전 패치될 내용
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 국내에만 적용되야 할 디파인
//#if SELECTED_LANGUAGE == LANGUAGE_KOREAN
#ifdef _LANGUAGE_KOR					// 국내만 적용

//## 서버통합	

	// - 박종훈
//	#define PJH_FINAL_VIEW_SERVER_LIST				// 서버통합시 보여질 서버리스트에대한 디파인(PJH_SEMI_FINAL_VIEW_SERVER_LIST과 같이 쓸수없음) (2008.04.07) [2008.07.17 체험테섭]
//	#ifdef PJH_FINAL_VIEW_SERVER_LIST
//	#define PJH_SEMI_FINAL_VIEW_SERVER_LIST	// 서버통합작업시 2~3일 정도 보여질 서버리스트에대한 디파인(PJH_FINAL_VIEW_SERVER_LIST 같이 쓸수없음) (2008.04.07) [2008.07.17 체험테섭]
	#define PJH_DEBUG_SERVER_ADD				// 서버통합 메인 디파인
//	#endif // PJH_FINAL_VIEW_SERVER_LIST

	#define PJH_CHARACTER_RENAME				// 캐릭터명 중복시 캐릭터 이름 변경.

//## 패스워드 확장
	
	// - 이동석
	#define LDS_MODIFY_CHAR_LENGTH_USERPASSWORD		// 서버 통합 작업과 관련하여 사용자 비밀번호 길이값이 기존 6-10 에서 8-12로 변경 (2008.06.18)

//#endif //SELECTED_LANGUAGE == LANGUAGE_KOREAN
#endif // _LANGUAGE_KOR					// 국내만 적용

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////













/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//## 미정리 디파인
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define CSK_EVIL_SKILL				// 악령 스킬 서버에서 판별하게 수정(2007.01.29)

// 한국에만 적용되야 할 작업들
// 해외 패치 할때 주의 할것 ( SELECTED_LANGUAGE == LANGUAGE_KOREAN 되어 있어야 한다. )
//#if SELECTED_LANGUAGE == LANGUAGE_KOREAN
#ifdef _LANGUAGE_KOR					// 국내만 적용
	#define	ADD_PCROOM_POINT_SYSTEM					//	PC방 포인트 시스템 추가(2007.03.07)
	#ifdef ADD_PCROOM_POINT_SYSTEM
		#define CSK_PCROOM_ITEM						//	PC방 포인트 아이템(혼돈의 상자 3종) 작업(2007.03.16)
		#define CSK_LUCKY_SEAL						//	행운의 인장(2007.03.14) - 상승, 풍요, 유지인장(ADD_PCROOM_POINT_SYSTEM 관련)
	#endif	// ADD_PCROOM_POINT_SYSTEM

	// - 안상규
	//#define NP_LOG								// 게임가드 로그 남기기.(게임가드 패킷 테스트 시에만 켤 것.) [내부테스트용, 패치 아님]
	//#define ADD_EVENT_PCROOM_POINT_HALF			// PC방 포인트 절반 이벤트. (2008.04.01) [08.04.04 테섭]

	#define TEENAGER_REGULATION						//  청소년자율구규제

	// - 이혁재
	#define ADD_MU_HOMEPAGE							//  창모드 일때만 단축키로 뮤 홈페이지 띄우기
//#endif //SELECTED_LANGUAGE == LANGUAGE_KOREAN
#endif // _LANGUAGE_KOR					// 국내만 적용

// - 박상완
#define PSW_ADD_MIX_CHAOSCHARM					// Mix file에 카오스 부적 텝 추가..( 2008.06,30 )

// - 박종훈
#define PJH_ADD_SET_NEWJOB						// 직업별 세트 아이템추가 & 세트 아이템에 다크로드, 소환술사 신규 추가(2008.03.21) [08.04.04 테섭]


// - 양대근
//#define DEVIAS_XMAS_END_LOADFIX_FOR_TESTSERVER	// 테섭 데비아스 크리스마스 맵 끝내기 위해 test붙인 맵 로딩하도록 함(본섭때는 빼야됨)(2008.1.3)
//#define DEVIAS_XMAS_EVENT2007						// 데비아스 크리스마스 맵 수정 (2007.12.04)





// ----------------------------------------------------------------------------
// 해외까지 완벽히 똑같이 적용된 디파인들이므로 손될 일 없고 정리할 일만 남았음
// 이하 모두 디파인 정리 대상
// ----------------------------------------------------------------------------

// - 김재희
#define KJH_ADD_CHECK_RESOURCE_GUARD_BEFORE_LOADING	// 로딩전 리소스가드 체크 (2010.10.14)
#define KJH_MOD_RESOURCE_GUARD						// 리소스가드 수정 - 실행파일뒤에 붙이던것을 단독 책섬 파일로 변경(10.03.10)

// - 김재희
#define KJH_MOD_LIBRARY_LINK_EACH_NATION			// 코드 라이블러리 국가별 Link 방식으로 변경 (10.04.22) - 모든 국가 필수

// - 안상규
#define ADD_ALICE_WINGS_2				// 소환술사 3차 날개.(2008.01.08). 3차 날개 추가 참고용 define.
#define	ADD_ALICE_WINGS_1				// 소환술사 1차, 2차 날개.(2007.11.23). 1,2차 날개 추가 참고용 define.
#define ADD_NPC_DEVIN					// 사제데빈 NPC.(2007.05.21). NPC 추가 참고용 define.

// 이벤트 종료로 아래 정의 막음.
/*#ifdef _TEST_SERVER
	#define	ADD_EVENT_SERVER_NAME		//	테스트6 서버 부터 '이벤트1'로 서버 이름 변경.(2007.05.15)
										//	ADD_EXPERIENCE_SERVER와 같이 쓰면 안됨.
#endif	// _TEST_SERVER*/

#ifdef _DEBUG
	//. 폰트출력 관련 부분을 개선하려하거나 디버깅 하려할때 켜주세요.
	//. 에디트 윈도우, 현재 텍스쳐등을 보실수 있습니다.
	//#define DEBUG_FONT_TEXTURE_TEST		// 폰트 텍스쳐, 입력윈도우등 출력(2007.05.05)
	//#define DEBUG_BITMAP_CACHE			// 콘솔창에서 현재 비트맵 상태 확인 가능
	//#define DEBUG_INVENTORY_BASE_TEST		// 인벤토리 기반 구현 테스트: 박스, 라인 출력 (2007.05.30)
#endif // _DEBUG

// - 박종훈
#define WHITE_DAY_EVENT					//  화이트데이 이벤트(2006-02-24 추가)
#define WORLDCUP_ADD					//  월드컵이벤트작업
//#define BATTLE_SOCCER_ADD_NPC_DIALOG	//  배틀사커 이벤트에 관한 NPC의 대화처리(이벤트 안내 대화)			
//#define NO_MORE_DANCING				//  춤을 못추게
#define WINDOWMODE						//  윈도우 모드 사용시(2006-07-18 추가)
#define HELLOWIN_EVENT					//  할로윈 이벤트 아이템 처리.(2006-10-18 추가)

//-------------------------------------------------------------------------------------------------------------------------------------

#endif //__WINMAIN_NEW_H__
