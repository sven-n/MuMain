#pragma once


// 부분 유료화에 필요 한국은 들어가면 안됨.
/*------------------------------------부분 유료화 1차----------------------------------------*/

// 부분 유료화 1차
#define PSW_PARTCHARGE_ITEM1				
#ifdef PSW_PARTCHARGE_ITEM1
	#define CSK_FREE_TICKET						// 자유입장권 - 데빌스퀘어, 블러드캐슬, 칼리마 (2007.02.06)
	#define CSK_CHAOS_CARD						// 카오스카드 - 복권 (2007.02.07)
	#define CSK_RARE_ITEM						// 희귀아이템(2006.02.15)
	#define CSK_LUCKY_CHARM						// 행운의 부적( 2007.02.22 )
	#define CSK_LUCKY_SEAL						// 행운의 인장( 2007.02.22 )	
	#define CSK_HIGHPRICEITE					// 고가 아이템 처리
	#define CSK_CHARGENOTMOVE					// 부분 유료화 아이템 이동 불가
#endif //PSW_PARTCHARGE_ITEM1

/*------------------------------------부분 유료화 2차----------------------------------------*/

// 부분 유료화 2차 ( 일본, 대만 )
#define PSW_PARTCHARGE_ITEM2
				
#ifdef PSW_PARTCHARGE_ITEM2
	#define PSW_ELITE_ITEM						// 엘리트 물약
	#define PSW_SCROLL_ITEM						// 엘리트 스크롤
	#define PSW_SEAL_ITEM						// 이동 인장
	#define PSW_FRUIT_ITEM						// 리셋 열매
	#define PSW_SECRET_ITEM						// 강화의 비약
	#define PSW_INDULGENCE_ITEM					// 면죄부
	#define PSW_RARE_ITEM						// 희귀 아이템 티켓
	#define PSW_CURSEDTEMPLE_FREE_TICKET		// 환영의 사원 자유 입장권
#endif //PSW_PARTCHARGE_ITEM2

/*----------------------------------시즌 2.5차 해외 특화 작업----------------------------------------*/

// 일본 슬라이드 공지 레벨 추가 기존 5개 버퍼를 10개로 수정( 2006 - 08 - 10 )
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
	#define REVISION_SLIDE_LEVEL_MAX          
#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE

// 시즌 2_5에서 마법공격 패킷 변경
#if SELECTED_LANGUAGE == LANGUAGE_TAIWANESE
	#define SEASON2_5_PACKET_MAGIC_ATTACK
#endif //SELECTED_LANGUAGE == LANGUAGE_TAIWANESE

// 시즌 2_5에서 좌표 패킷 변경
#if SELECTED_LANGUAGE == LANGUAGE_CHINESE
	#define SEASON2_5_PACKET_POSITION
#endif //SELECTED_LANGUAGE == LANGUAGE_CHINESE

/*------------------------------------부분 유료화 3차----------------------------------------*/

// 부분 유료화 3차 ( 일본 )
#define PSW_PARTCHARGE_ITEM3
				
#ifdef PSW_PARTCHARGE_ITEM3
	#if !defined(LDS_PATCH_GLOBAL_100520) // 글로벌은 100527패치 때 케릭터 카드 제외
		#define PSW_CHARACTER_CARD						// 소환술사 캐릭터 카드
	#endif // !defined(LDS_PATCH_GLOBAL_100520)
	#define PSW_NEW_CHAOS_CARD						// 신규 카오스 카드
	#define PSW_NEW_ELITE_ITEM						// 중간 엘리트 물약
#endif //PSW_PARTCHARGE_ITEM3

/*--------------------------------해외 전용( 시즌 3 이후 )----------------------------------------*/

#define PSW_ADD_FOREIGN_HELPWINDOW                  // f1 key를 눌렀을때 도움말중에 이벤트맵 (해외전용)
													// 일정표 해외 적용 안하기( 2008.07.08 )


/*------------------------------------부분 유료화 4차----------------------------------------*/

// 부분 유료화 4차 ( 일본 )
#define PSW_PARTCHARGE_ITEM4

#ifdef PSW_PARTCHARGE_ITEM4
	#define PSW_ADD_PC4_SEALITEM       //  치유, 신성( 체젠, 마젠 ) 인장
	#define PSW_ADD_PC4_SCROLLITEM     //  전투, 강화( 크리, 액설 데미지 증가 ) 스크롤
	#define PSW_ADD_PC4_CHAOSCHARMITEM //  카오스 조합 부적
	#define LDK_ADD_PC4_GUARDIAN		// 수호천사, 사탄
	#define	LDK_FIX_GUARDIAN_CHANGE_LIFEWORD			// 1차 프리미엄 아이템 (내구력 표기 -> 생명으로 수정)(2009.01.14)
	#define LDK_ADD_PC4_GUARDIAN_EFFECT_IMAGE // lightmarks.jpg, effect #define ADD_SOCKET_ITEM 와 중복 되는것 있음... 
#endif //PSW_PARTCHARGE_ITEM4


/*------------------------------------부분 유료화 5차----------------------------------------*/

// 부분 유료화 5차 ( 일본 )
#define ADD_CASH_SHOP_5TH

#ifdef ADD_CASH_SHOP_5TH
	#define LDK_FIX_CS5_HIDE_SEEDLEVEL	// 상점 시드레벨 표시 안함
	#define YDG_ADD_CS5_REVIVAL_CHARM	// 부활의 부적
	#define YDG_ADD_CS5_PORTAL_CHARM	// 이동의 부적
#endif	// YDG_ADD_CASH_SHOP_5TH

// - 박보근
#define PBG_WOPS_TEXT_MONMARK						// 상승의 인장 텍스트 관련(08.12.05)
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
	#define PBG_WOPS_MOVE_MAPNAME_JAPAN					// 인장없이 기본 맵 이동 불가능한 현상(08.12.17)
#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
#if SELECTED_LANGUAGE != LANGUAGE_KOREAN
#define PBG_WOPS_CURSEDTEMPLEBASKET_MOVING			//성물보관함이 움직이는 버그 수정(2008.12.18)
#endif //SELECTED_LANGUAGE != LANGUAGE_KOREAN


/*------------------------------------부분 유료화 6차----------------------------------------*/

#if SELECTED_LANGUAGE != LANGUAGE_KOREAN

// [2009.02.18 일본 1.03.05] - 안상규
// 차후에 다른 나라들도 추가할 것. 서버도 같이 수정되어야 함.
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_ENGLISH
	#define ASG_FIX_LENA_REGISTRATION				// 레나 개수 32,767개 이상 등록 가능하도록 수정.
#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE

#if SELECTED_LANGUAGE == LANGUAGE_ENGLISH
// - 안상규
// 유료 채널(골드 채널) 관련 디파인. 
	#define ASG_ADD_CHARGED_CHANNEL_TICKET			// 유료 채널 입장권.글로벌만 적용.(2010.02.05)
	#define ASG_MOD_CHARGED_CHANNEL_TICKET_ADD_DESCRIPTION	// 유료 채널 입장권 설명 추가.(2010.02.24)
	#define ASG_ADD_CASH_SHOP_ALREADY_HAVE_ITEM_MSG	// 캐쉬샵 중복 아이템 메시지 추가.(2010.02.24)	
	#define KJH_ADD_SERVER_LIST_SYSTEM				// 서버리스트 시스템 변경 (09.10.19) - 국내의 내용을 미리 당겨 사용.
	#define ASG_ADD_SERVER_LIST_SCRIPTS				// ServerList.txt 스크립트 추가.(2009.10.29) - 국내의 내용을 미리 당겨 사용.
	#ifdef ASG_ADD_SERVER_LIST_SCRIPTS
		#define ASG_ADD_SERVER_LIST_SCRIPTS_GLOBAL_TEMP	// 미리 ASG_ADD_SERVER_LIST_SCRIPTS 추가로 인한 버그 수정.(10.03.02) 
														//시즌5 이후 삭제해도 됨.(BuxConvert() 함수 위치 문제) // 시즌5 업데이트시에 삭제해도 되는것인지 확인 다시 할것.
		#define ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL	// 서버리스트에 유료 채널 추가.(10.02.26) - 현재 글로벌만 적용.
	#endif	//ASG_ADD_SERVER_LIST_SCRIPTS
	#define ASG_FIX_NOT_EXECUTION_WEBSTARTER_MSG	// 웹스타터 미실행 메시지 Text.txt 번호 수정(10.03.03)
#endif //SELECTED_LANGUAGE == LANGUAGE_ENGLISH

#define ADD_TOTAL_CHARGE_6TH
#ifdef ADD_TOTAL_CHARGE_6TH
// 1. 케쉬 샾 아이템 ( 일본 )
// ---------------------------------------------------------------------------
#define ADD_CASH_SHOP_6TH
#ifdef ADD_CASH_SHOP_6TH
	// - 안상규
	#define ASG_ADD_CS6_GUARD_CHARM				// 수호의부적(2009.02.09)
	#define ASG_ADD_CS6_ITEM_GUARD_CHARM		// 아이템보호부적(2009.02.09)
	#define ASG_ADD_CS6_ASCENSION_SEAL_MASTER 	// 상승의인장마스터(2009.02.24)
	#define ASG_ADD_CS6_WEALTH_SEAL_MASTER 		// 풍요의인장마스터(2009.02.24)

	// - 이동석		// 참조 Define : CSK_PCROOM_ITEM
	#define LDS_ADD_CS6_CHARM_MIX_ITEM_WING		// 날개 조합 100% 성공 부적
#endif	// ADD_CASH_SHOP_6TH

// 2. 황금 군단 
// ---------------------------------------------------------------------------
// - 최석근
#define CSK_ADD_GOLDCORPS_EVENT					// 황금군단이벤트(2009.03.13)
// - 이동석
#define LDS_ADD_GOLDCORPS_EVENT_MOD_GREATDRAGON // 황금 그레이트 드레곤 비주얼 재작업. 비주얼 이펙트 추가. 기존 동일한 몹 존재. (2009.04.06) (BITMAP_FIRE_HIK3_MONO 효과 참조.)

// 3. PC 방 시스템 및 아이템
// ---------------------------------------------------------------------------
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_TAIWANESE
#define ADD_CONTENTS_PCROOM_6TH
#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_TAIWANESE
#ifdef ADD_CONTENTS_PCROOM_6TH
	#define	ADD_PCROOM_POINT_SYSTEM				// 일본 PC방 시스템 적용. winmain_new.h 참조 (2009.03.25)
	#ifdef ADD_PCROOM_POINT_SYSTEM
		#define CSK_FIX_PCROOM_POINT_ZEN_BUG		// 일본 PC방 시스템 적용. winmain_new.h 참조 (2009.03.25) // PC방 포인트 상점에서 PC방 아닐때 포인트가 젠으로 표시되는 문제(2009.02.23) [09.02.24 본섭]
		#define ASG_PCROOM_POINT_SYSTEM_MODIFY		// 일본 PC방 시스템 적용. winmain_new.h 참조 (2009.03.25) // PC방 포인트 시스템 수정.(2008.06.13) [2008.07.17 체험테섭]
	#endif // ADD_PCROOM_POINT_SYSTEM

	#define LDS_PCROOM_CONTENT_6TH				// 일본 PC방용 컨텐츠 추가. (2009.03.31)
		#define CSK_PCROOM_ITEM					// 기존 컨텐츠 (혼돈의 상자 3종) winmain_new.h 참조 (2009.04.02)
		// 이동석		
		#define LDS_ADD_PCROOM_ITEM_JPN_6TH		// 신규 컨텐츠 (강함의 인장) (2009.03.31)
	#ifdef LDS_PCROOM_CONTENT_6TH
		
	#endif // LDS_PCROOM_CONTENT_01_6TH
#endif // ADD_CONTENTS_PCROOM_6TH

// 4. QA 이후 Bug Fixed 작업 사항
// ---------------------------------------------------------------------------
#define ADD_FIX_AFTER_QA_6TH

#ifdef ADD_FIX_AFTER_QA_6TH
	#define LDS_FIX_APPLY_BUFF_STATEVALUES_WHEN_CALCULATE_CHARACTERSTATE_WITH_SETITEM	// 케릭터 정보창에 세트 아이템 적용 된 스탯 수치 적용시에 현재 스탯 계산시 점에 버프 및 소켓아이템 으로 인한 추가능력치가  누락되어 추가 (2009.04.13)
	#define LDS_FIX_WRONG_QUANTITY_SETITEMOPTION_WHEN_RENDER_SETITEMTOOTIP_IN_INVENTORY	// 현재 장착된 세트 아이템과 동일한 세트가 인벤토리에 있는경우 해당 인벤토리 세트 아이템 옵션 출력에 갯수 오류		(2009.04.13)
#endif // ADD_FIX_AFTER_QA_6TH

// 5. 6차 과금 내부 테섭 1차 버그 수정
#define ADD_FIX_AFTER_6TH_TEST
#ifdef ADD_FIX_AFTER_6TH_TEST
//	#define PBG_FIX_REPAIRGOLD_DURABILITY0							// 내구력 0일때 수리비 서버와 다른 버그 수정(09.04.13)
//	계산공식을 클라에서 맞췄으나 서버에서 재차 수정됨 define 막아야 됨 PBG_FIX_REPAIRGOLD_DURABILITY0
	#define PBG_FIX_SHIELD_MAGICDAMAGE								// 방패착용시 정보창에 마력이 상승하는 버그 수정(09.04.15)
	#define PBG_FIX_FENRIR_GELENALATTACK							// 펜릴을 타고 일반공격이 안되는 현상 (09.04.17)
	#define PBG_FIX_SKILL_RECOVER_TOOLTIP							// 회복스킬의 툴팁상의 스킬 공격력이 출력되고 있는 버그(09.04.20)
	#define PBG_FIX_SETITEM_4OVER									// 세트 아이템의 4종 초과하여 장착했을 경우 버그(09.04.24)
	#define PBG_MOD_MASTERLEVEL_MESSAGEBOX							// 상승의 인장 마스터케릭터 구입여부 추가(09.4.24)
	#define PBG_FIX_REPAIRGOLD_DARKPAT								// 다크호스,스피릿 수리비 버그 수정(09.04.14)
	#define KJH_FIX_DARKLOAD_PET_SYSTEM								// 개인상점구입창에서 다크로드펫의 툴팁이 정확하게 출력되지 않는 버그수정 (09.04.16)	[JP_0226]
	#define KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM			// NPC상점에서 아이템을 구매할때, 처음에 두번클릭해야 아이템이 구입되는 버그수정 (09.04.16)	[JP_0348]
	#define ASG_FIX_ARROW_VIPER_BOW_EQUIP_DIRECTION					// 마을에서 에로우바이퍼보우 착용 방향 버그 수정.(2009.04.17)
//2차 버그 수정
	#define KJH_FIX_MOVECOMMAND_WINDOW_SIZE							// 이동명령창 크기변경시 빈공간이 생기는 문제 수정 (09.03.02)		[JP_0420]
	#define PBG_FIX_CHARM_MIX_ITEM_WING								// 날개의 부적 상점에 판매후 다음 물품 판매 안되는 버그 수정(09.05.06)
	#define KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI					// 황금궁수 NPC창을 연상태로 아이템몰 창을 열었을때, 이상현상이 발생하는 버그수정 (09.05.06)
	#define PBG_FIX_DARKPET_RENDER									// 다크로드 펫의 스피릿과 호스의 값이 변경되는 현상(09.05.08)
	#define KJH_FIX_JP0459_CAN_MIX_JEWEL_OF_HARMONY					// 조화의보석으로 업그레이드 할수 있는 아이템인데도 빨갛게 나오는 버그수정 (09.05.08)
	#define LDS_FIX_NONINITPROGLEM_SKILLHOTKEY						// 간혹 신규케릭터 이후 바로이전 로그인했던 케릭스킬단축키가 영향주는 오류. MainScene호출마다 SkillKey 테이블 초기화. (2009.01.20)
	#define KJH_FIX_JP0450_EQUIPITEM_ENABLE_MOVECOMMAND				// 이동명령창을 누른후 이동할수 없는 조건의 아이템을 마우스픽업후 강제 이동하는 버그 수정 (09.05.08)
//	#define PBG_FIX_SKILLHOTKEY										// 무기 스킬의 핫키 번호 기억하는것 무기 제거시에 리셋(09.05.11)
	#define PBG_FIX_CHARM_MIX_ITEM_WING_TOOLTIP						// 날개의 부적 툴팁 잘못 출력되는 버그(09.05.11)
	#define LDS_FIX_SYNCRO_HEROATTACK_ACTION_WITH_SERVER			// wops 33863 - 흑기사 스킬 시전시 서버에서 스킬 받기전에 미리 스킬발동이 걸려 이전 스킬 에니메이션이 잘못 출력. (09.04.22)
	#define LDK_FIX_MOTION_INTI_AT_THE_LOADINGWORLD					// wops 34113 - 월드로딩시 모션 초기화(09.04.14)

	#define PBG_FIX_DARKPET_TIPUPDATE								// 다크펫의 경험치의 표시가 갱신되지 않는다.재 장착시에만 갱신되는 버그수정(09.05.29)
	#define PBG_FIX_MOVECOMMAND_WINDOW_SCROLL						// 스크롤 마우스 왼쪽 클릭으로 움직일 수 없는 버그 수정(09.05.29)
	#define PBG_FIX_DARKPET_DURABILITY								// 다크로드 펫 내구력 관련 버그 수정(09.05.29)
	#define PBG_FIX_DARKSPIRIT_ACTION								// 다크로드 스피릿의 맵로딩시 손이 내려간 모션이 취해져 있는 버그 수정(09.05.29)
	#define PBG_FIX_LODING_ACTION									// 로딩시에 발이 움직이지 않는(모션) 버그(09.05.25)
	#define KJH_FIX_JP0467_RENDER_DARKLOAD_PET_OTHER_CHARATER		// 다른케릭터의 다크로드펫이 장착되거나 해제할때, 화면상 변화되지 않는 버그 수정 (09.06.03)
	#define KJH_FIX_GOLD_RABBIT_INDEX								// 황금토끼 인덱스 수정 (09.06.08)

	#define LDK_FIX_PCROOM_INFOMATION								// 피시방 포인트 상점 정보출력 문구 예외처리(09.07.14)
	#define LDK_FIX_CHARM_MIX_ITEM_WING_TOOLTIP						// 날개의 부적 툴팁 잘못 출력 및 내용 누락되는 버그(09.07.20)

	#define YDG_FIX_DARKSPIRIT_CHAOSCASTLE_CRASH					// 다크스피릿 카오스캐슬에서 장비해제해서 손에들고 장비창 닫으면 튕기는 버그 (09.07.17)
	#define YDG_FIX_SOCKETITEM_SELLPRICE_BUG						// 소켓아이템 판매가격 버그 (09.09.03)
#endif //ADD_FIX_AFTER_6TH_TEST

#endif // ADD_TOTAL_CHARGE_6TH

#endif // SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_TAIWANESE

/*---------------------------------여기 까지 부분 유료화 6차-------------------------------------*/


#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || SELECTED_LANGUAGE == LANGUAGE_ENGLISH || SELECTED_LANGUAGE == LANGUAGE_TAIWANESE || SELECTED_LANGUAGE == LANGUAGE_CHINESE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 시즌 4.5 - PK필드/불카누스/겜블/팬더팻 (09.06.23)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SEASON4_5										
#ifdef SEASON4_5

	//------------------------------------------------
	// 컨텐츠 추가
	//------------------------------------------------
	// - 김재희
	#define KJH_ADD_09SUMMER_EVENT					// 09년 여름이벤트		(09.06.23)

	// - 양대근
	#define YDG_ADD_NEW_DUEL_SYSTEM					// 새로운 결투 시스템 (2009.01.05) [09.03.19 테섭]
	#ifdef YDG_ADD_NEW_DUEL_SYSTEM
		#define YDG_ADD_NEW_DUEL_UI					// 새로운 결투 관련 UI 추가 (2009.01.05) [09.03.19 테섭]
		#define YDG_ADD_NEW_DUEL_PROTOCOL			// 새로운 결투 관련 프로토콜 추가 (2009.01.08) [09.03.19 테섭]
		#define YDG_ADD_NEW_DUEL_NPC				// 새로운 결투 관련 NPC 타이투스 추가 (2009.01.19) [09.03.19 테섭]
		#define YDG_ADD_NEW_DUEL_WATCH_BUFF			// 새로운 결투 관전 버프 추가 (2009.01.20) [09.03.19 테섭]
		#define YDG_ADD_MAP_DUEL_ARENA				// 새로운 결투장 맵 추가 (2009.02.04) [09.03.19 테섭]
	#endif	// YDG_ADD_PVP_SYSTEM

	// - 박보근
	#define PBG_ADD_PKFIELD							// PK필드 추가(2008.12.29) [09.03.19 테섭]
	#define PBG_ADD_HONOROFGLADIATORBUFFTIME		// 검투사의 명예 버프 시간 툴팁 추가(2009.03.19) [09.03.19 테섭]

	// - 이동근
	#define LDK_ADD_GAMBLE_SYSTEM					// 젠소모 무기 뽑기 상점 (09.01.05) [09.03.19 테섭]
	#ifdef LDK_ADD_GAMBLE_SYSTEM
		#define LDK_ADD_GAMBLE_NPC_MOSS				// 겜블 상인 모스 [09.03.19 테섭]
		#define LDK_ADD_GAMBLE_RANDOM_ICON			// 겜블용 랜덤무기 아이콘 [09.03.19 테섭]
		#define LDK_ADD_GAMBLERS_WEAPONS			// 겜블전용 레어 무기들 [09.03.19 테섭]
	#endif //LDK_ADD_GAMBLE_SYSTEM

	// - 박종훈
	#define PJH_ADD_PANDA_PET						// 펜더 펫
	#define PJH_ADD_PANDA_CHANGERING				// 펜더 변신반지


	//------------------------------------------------
	// 버그수정
	//------------------------------------------------
	// - 김재희
	#define KJH_FIX_MOVE_HERO_MOUSE_CLICK_EFFECT			// 캐릭터 이동시 마우스클릭 이팩트가 이동하지 못하는 곳에 렌더되는 버그 수정 (09.03.04) [09.03.19 테섭]
	#define KJH_FIX_GET_ZEN_SYSTEM_TEXT						// 겜블상점에서 아이템구입시 젠증가 메세지가 나오는 문제. (09.03.18) [09.03.19 테섭]
	#define KJH_FIX_WOPS_K33695_EQUIPABLE_DARKLOAD_PET_ITEM	// 다크로드가 인벤에 펫을 소유하고 있을때, 툴팁에 착용불가로 나오는 버그수정 (09.04.27)
	#define KJH_FIX_WOPS_K33479_SELECT_CHARACTER_TO_HEAL	// 캐릭터에게 둘러쌓였을때, 요정의 치료스킬강화이후 캐릭터 선택이 잘 안되어지는 버그수정 (09.04.27)
	#define KJH_FIX_CHAOTIC_ANIMATION_ON_RIDE_PET			// 펫을타고 다크로드 카오틱디세이어 스킬시 애니메이션수정 (08.12.18)
	#define KJH_MOD_CAN_TRADE_PANDA_PET						// 팬더팻 개인 거래 가능 - 해외만 적용 (09.07.07)

	// - 최석근
	#define CSK_FIX_UI_FUNCTIONNAME							// UI관련 함수명 변경(2009.01.22) [2009.03.26 테섭]
	#define CSK_FIX_ANIMATION_BLENDING						// 애니메이션 블렌딩쪽 블렌드 안되게 옵션 주게 수정(2009.01.29) [2009.03.26 테섭]
	#define CSK_FIX_ADD_EXEPTION							// dmp파일 분석해보니 getTargetCharacterKey 함수에서 튕기는걸로 예측되서 예외처리 추가(2009.01.29) [2009.03.26 테섭]

	// - 박보근
	#define PBG_FIX_PKFIELD_ATTACK							// PKFIELD에서 길드원끼리 공격되는 현상(09.04.02) [2009.04.06 테섭]
	#define PBG_FIX_PKFIELD_CAOTIC							// 카오틱 디세이어 탈 것에서 버그 수정(09.04.07) [2009.04.13 테섭]
	#define PBG_FIX_GUILDWAR_PK								// PKFIELD에서 길드워 중에 일반인 공격안받는 버그 수정(09.04.07) [2009.04.13 테섭]
	#define PBG_FIX_CHAOTIC_ANIMATION						// 카오틱 디세이어 애니메이션 수정(09.06.11)
	#define PBG_FIX_DARK_FIRESCREAM_HACKCHECK				// 다크 파이어스크림 (서버)핵체크관련 버그 수정(09.06.22)

	// - 이동근
	#define LDK_FIX_INVENTORY_SPEAR_SCALE					// 인벤토리 무기-spear 크기 처리 if문 수정 [09.03.19 테섭]

	// - 양대근
	#define YDG_FIX_INVALID_TERRAIN_LIGHT					// 메모리 침범 버그 수정(맵툴 관련) (2009.03.30) [2009.04.13 테섭]
	#define YDG_FIX_MEMORY_LEAK_0905						// 메모리 누수 제거 (2009.05.11) [패치안됨]
	#define YDG_FIX_INVALID_SET_DEFENCE_RATE_BONUS			// 방어구세트 추가 방어력표시 방어력의 10%가 아니라 방어율의 10%임 (2009.04.15) [패치안됨] wops_32937
	#define YDG_FIX_STAFF_FLAMESTRIKE_IN_CHAOSCASLE			// 카오스캐슬에서 마검사 지팡이 착용시 플레임스트라이크와 블러드어택 아이콘색 문제 (2009.04.15) [패치안됨] wops_34747
	#define YDG_FIX_LEFTHAND_MAGICDAMAGE					// 마검사가 지팡이나 룬바스타드를 왼손에 찼을때 마력이 올라가는 문제 (2009.04.15) [패치안됨] wops_32641
	#define YDG_FIX_BLOCK_STAFF_WHEEL						// 마검사 지팡이, 맨손 회오리베기 막기 (2009.05.18) [패치안됨]
	#define YDG_FIX_MEMORY_LEAK_0905_2ND					// 메모리 누수 제거 2차 (2009.05.19) [패치안됨]

	// - 안상규
	#define ASG_FIX_PICK_ITEM_FROM_INVEN					// 인벤창 특정 칸에서 아이템을 집어들었을 때 캐릭정보창에서 능력치가 변동되는 버그(2009.04.15) wops_33567
	#define ASG_FIX_MOVE_WIN_MURDERER1_BUG					// 무법자(카오) 레벨1일 때 맵 이동창 표시 버그.(2009.04.20) wops_34498

	// - 박종훈
	#define PJH_FIX_4_BUGFIX_33								// wops_34066

	// - 이동석
	#define LDS_FIX_MASTERSKILLKEY_DISABLE_OVER_LIMIT_MAGICIAN	// 마스터 스킬 강화 된 스킬들에 대해 요구치 미달시에 이용 불가. (09.04.22) no_wops
	#define LDS_FIX_SETITEM_WRONG_CALCULATION_IMPROVEATTACKVALUE	// 세트아이템착용 이후 사랑의 묘약을 먹으면, 케릭터 정보창에 힘증가 스탯수치가 다른현상. (09.04.23) wops_35131
	#define LDS_FIX_MEMORYLEAK_PHYSICSMANAGER_RELEASE		// 메모리 누수 제거, Physicsmanager 힙영역 메모리 반환 처리. (다크로드 케릭 로딩이후 종료 시 릭 발생)(09.05.25)

// 시즌 4.5 내부 테섭 버그 수정
#define ADD_FIX_AFTER_SEASON4_5_TEST
#ifdef ADD_FIX_AFTER_SEASON4_5_TEST
	#define YDG_FIX_PANDA_CHANGERING_PANTS_BUG				// 다크로드 맨몸으로 팬더 변신반지 착용시 바지 버이는 버그 (09.07.21)
	#define YDG_FIX_DUEL_SUMMON_CLOAK						// 결투시 요정 소환수 망토 나오는 버그 (09.07.29)

	#define	YDG_FIX_SMALL_ITEM_TOOLTIP_POSITION				// 일본 요청으로 1칸짜리 아이템 툴팁위치 내림 (09.08.24)
	#define KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM_EX	// NPC상점에서 아이템을 구매할때, 처음에 두번클릭해야 아이템이 구입되는 추가 버그수정 (09.08.24)

	#define YDG_FIX_TRADE_BUTTON_LOCK_WHNE_ITEM_MOVED		// 거래창 사기관련 트레이드 장비 옮기면 거래버튼 잠그기(베트남제보) (09.08.25)
	#define YDG_FIX_NPCSHOP_SELLING_LOCK					// NPC상점 판매시 창 못닫게 잠금(베트남제보) (09.08.25)

	#define YDG_FIX_CATLE_MONEY_INT64_TYPE_CRASH			// 공성 성주창 성의 돈이 많을때 튕기는 버그 (09.09.01)
	#define YDG_FIX_USED_PORTAL_CHARM_STORE_BAN				// 이동의 부적 사용하면 창고 저장 못하게 수정 (09.09.02)

	#define PBG_FIX_CHANGEITEMCOLORSTATE							// 인벤토리 안에서 내구력에 따른 색깔 변화하도록 수정(데몬/수호정령/팬더펫)(09.06.16)
	#define PBG_FIX_PETTIP									// 인벤내부의 다크로드 관련 펫의 정보가 착용한 펫의 정보로 출력되는 버그 수정(09.09.10)

	#define LJH_FIX_CHANGE_RING_DAMAGE_BUG					// 변신반지 복수 착용시 높은 값으로 데미지,마력,저주력 증가가 되도록 수정 (09.09.11)
	#define YDG_FIX_OVER_5_SETITEM_TOOLTIP_BUG				// 세트아이템 5종류 이상 차면 계산안되는 버그 수정(09.09.11)
	#define YDG_FIX_MAGIC_DAMAGE_CALC_ORDER					// 세트아이템 관련 마력 계산 순서 수정 (09.09.11)
	#define YDG_FIX_SETITEM_REQUIRED_STATUS_BUG				// 세트아이템 요구능력치 부족시 세트계산에서 빠지게 수정 (09.09.14)

	#define YDG_FIX_380ITEM_OPTION_TEXT_SD_PERCENT_MISSING	// 380아이템 옵션에 SD증가에 % 빠진 버그 수정 (09.09.23)
	#define YDG_FIX_LUCKY_CHARM_VALUE_TO_0					// 행운의 부적 가격 0으로 설정 (09.09.23)

	#define YDG_FIX_BUFFTIME_OVERFLOW						// 버프시간이 0보다 작아지면 넘치는 버그 (09.09.28)
	#define LJH_FIX_PET_SHOWN_IN_CHAOS_CASTLE_BUG			// 펫이 카오스캐슬에서 보이는 버그 수정 (09.09.29)
	#define YDG_FIX_REPAIR_COST_ADJUST_TO_SERVER_SETTING	// 수리 가격 계산 서버에 맞춤 (09.10.12)
#endif //ADD_FIX_AFTER_SEASON4_5_TEST

#endif // SEASON4_5
#endif // SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || SELECTED_LANGUAGE == LANGUAGE_CHINESE

/*--------------------------------------부분 유료화 7차------------------------------------------*/
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_TAIWANESE || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || SELECTED_LANGUAGE == LANGUAGE_CHINESE

#define ADD_TOTAL_CHARGE_7TH
#ifdef ADD_TOTAL_CHARGE_7TH
	// - 양대근
	#define YDG_ADD_CS7_CRITICAL_MAGIC_RING		// 치명마법반지
	#define YDG_ADD_CS7_MAX_AG_AURA				// AG증가 오라
	#define YDG_ADD_CS7_MAX_SD_AURA				// SD증가 오라
	#define YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM	// 파티 경험치 증가 아이템
	#define YDG_ADD_CS7_ELITE_SD_POTION			// 엘리트 SD회복 물약
	#define YDG_MOD_AURA_ITEM_EXCLUSIVELY_BUY	// 오라 중복 구입 막기 (09.11.02)

	// - 이동근
	#define LDK_MOD_PETPROCESS_SYSTEM			// 펫프로세스 업데이트 순서 변경(애니메이션 속도 임의 변경 가능)(2009.09.11)
	#define LDK_ADD_CS7_UNICORN_PET				// 유니콘 펫 
	#define LDK_FIX_CS7_UNICORN_PET_INFO		// 유니콘 펫 캐릭터 정보창 방어력 추가

	// - 박보근
	#define PBG_ADD_AURA_EFFECT					// AG SD 증가 오라 이펙트
#endif //ADD_TOTAL_CHARGE_7TH

// - 이동근
//#define LDK_MOD_NUMBERING_PETCREATE				// 테스트 (09.08.27)

#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || SELECTED_LANGUAGE == LANGUAGE_CHINESE
/*---------------------------------여기 까지 부분 유료화 7차-------------------------------------*/


#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE  || SELECTED_LANGUAGE == LANGUAGE_TAIWANESE || defined(LDS_PATCH_GLOBAL_100520) || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || SELECTED_LANGUAGE == LANGUAGE_CHINESE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 시즌 5 - 도플갱어/제국수호군/퀘스트 (09.09.11)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SEASON5										
#ifdef SEASON5
	// - 양대근
	#define YDG_ADD_DOPPELGANGER_EVENT						// 도플갱어 이벤트 (2009.04.22)
	#define YDG_MOD_SEPARATE_EFFECT_SKILLS					// 스킬 검사하는 이펙트 분리 (블러드 어택 등) (2009.08.10)

	// - 안상규
	#define ASG_ADD_NEW_DIVIDE_STRING						// 새로운 문장 나누기 기능.(2009.06.08)
	#define ASG_ADD_NEW_QUEST_SYSTEM						// 뉴 퀘스트 시스템(2009.05.20)

	// 제국 수호군 파티 - 이동석, 이동근
	#define LDS_ADD_EMPIRE_GUARDIAN							// 제국 수호군

	// - 박보근
	#define PBG_ADD_DISABLERENDER_BUFF						// 랜더안할 버프 이미지 처리(09.08.10)

	//------------------------------------------------
	#ifdef YDG_ADD_DOPPELGANGER_EVENT
		#define YDG_ADD_MAP_DOPPELGANGER1					// 도플갱어 맵1 (라클리온베이스) (2009.03.22)
		#define YDG_ADD_MAP_DOPPELGANGER2					// 도플갱어 맵2 (불카누스베이스) (2009.03.22)
		#define YDG_ADD_MAP_DOPPELGANGER3					// 도플갱어 맵3 (아틀란스베이스) (2009.03.22)
		#define YDG_ADD_MAP_DOPPELGANGER4					// 도플갱어 맵4 (칸투르1차베이스) (2009.03.22)
		#define YDG_ADD_DOPPELGANGER_MONSTER				// 도플갱어 몬스터 추가 (2009.05.20)
		#define YDG_ADD_DOPPELGANGER_NPC					// 도플갱어 NPC 루가드 추가 (2009.05.20)
		#define YDG_ADD_DOPPELGANGER_ITEM					// 도플갱어 아이템 추가 (2009.05.20)
		#define YDG_ADD_DOPPELGANGER_UI						// 도플갱어 UI 추가 (2009.05.20)
		#define YDG_ADD_DOPPELGANGER_PROTOCOLS				// 도플갱어 프로토콜 추가 (2009.06.23)
		#define YDG_ADD_DOPPELGANGER_PORTAL					// 도플갱어 맵 포탈 추가 (2009.07.30)
		#define YDG_ADD_DOPPELGANGER_SOUND					// 도플갱어 사운드 추가 (2009.08.04)
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
		#define ASG_ADD_UI_NPC_DIALOGUE						// NPC 대화창(2009.09.14)
		#define ASG_ADD_UI_QUEST_PROGRESS					// 퀘스트 진행 창(NPC용)(2009.05.27)
	#ifndef ASG_ADD_UI_NPC_DIALOGUE		// 정리시 ASG_ADD_UI_NPC_MENU 디파인 모두 삭제.
		#define ASG_ADD_UI_NPC_MENU							// NPC 메뉴창(2009.06.17)
	#endif	// ASG_ADD_UI_NPC_DIALOGUE
		#define ASG_ADD_UI_QUEST_PROGRESS_ETC				// 퀘스트 진행 창(기타용)(2009.06.24)
		#define ASG_MOD_UI_QUEST_INFO						// 퀘스트 정보 창(2009.07.06)
		#define ASG_MOD_3D_CHAR_EXCLUSION_UI				// 3D 캐릭터 표현이 없는 UI로 바꿈.(2009.08.03)
		//(!주의!) UI에 3D캐릭터 표현이 추가 되면 원래대로 되돌려야 되므로 정리하지 말 것.
	#endif	// ASG_ADD_NEW_QUEST_SYSTEM

	//------------------------------------------------
	// 버그수정
	//------------------------------------------------
	// - 이동석
	#define LDS_ADD_SET_BMDMODELSEQUENCE_					// BMD 클래스의 사용되지 않는 변수인 BmdSequenceID에 BMDSequence 설정. (09.08.05)
	#define LDS_FIX_MEMORYLEAK_BMDWHICHBONECOUNTZERO		// BoneCount==0 인 BMD들에 대한 BMD Destruct 시점에 발생하는 메모리 릭 FIX. (09.08.13)
	#define LDS_FIX_MEMORYLEAK_0908_DUPLICATEITEM			// DuplicateItem() 내에 힙 영역에 생성된 Item의 메모리 반환 미처리로 메모리 릭 FIX. (09.08.13)
	#define LDS_FIX_GLERROR_WRONG_WRAPMODEPARAMETER			// ImageOpen시 WrapMode Param 값이 잘못되어, OpenGLERROR가 뜨며 이에 대한 FIX. (MR0 적용 시 치명적 오류가 발생하므로..) (09.08.18)
	#define LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER	// 제국 수호군 방패병의 서버로부터의 몬스터 스킬==>Action 연결 작업. (09.08.18)
	#define LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER_EX	// 방패병의 1번 스킬(44)동작의 기존방식과 다른 방식의 에니메이션 호출 재 수정 (09.08.21)
	#define LDS_FIX_EG_COLOR_CHANDELIER							// release 모드서만 제국 수호군 주말맵 샹들리에 연기 색 다른 버그 수정 (09.08.21)
	#define LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER_EX2	// 방패병 44번 방패막기 스킬시 간혹 렌덤방향 바라보는 버그 수정 (09.08.24)
	#define LDS_FIX_SKILLKEY_DISABLE_WHERE_EG_ALLTELESKILL	// 법사 텔레계열(텔레키네시스,순간이동) 스킬들은 제국수호군 영역에서 사용 못하도록 설정(09.08.28)
	#define LDS_FIX_EG_JERINT_ATTK1_ATTK2_SPEED				// 제국수호군 제린트 공격1, 공격2 속도 잘못된 수치로 FIX 작업 (09.09.04)
	#define LDS_MOD_ANIMATIONTRANSFORM_WITHHIGHMODEL			// AnimationTransformWithAttachHighModel함수에서 현재 arrayBone만 필요할 경우는 Transform으로 Bone이 Vertices에 곱하지 않도록 변경. (2009.09.09)
	#define LDS_ADD_ANIMATIONTRANSFORMWITHMODEL_USINGGLOBALTM	// AnimationTransformWithAttachHighModel함수내 TMArray가 Local인 이유로 이후 연산에 TM이 Point를 잃어버리는 현상으로 TM을 Global 으로 변경. (09.09.07)
	#define LDS_FIX_EG_JERINT_ANIMATION_AND_SWORDFORCE_SPEED	// 제린트의 공격 속도 및 검기 시간 범위 재작업 (09.09.08) 
// 	#define LDS_FIX_AFTER_PETDESTRUCTOR_ATTHESAMETIME_TERMINATE_EFFECTOWNER	// 펫 메니저의 펫 종료 될 때마다 펫당 발생하던 메모리릭 FIXED. (YDG_FIX_MEMORY_LEAK_0905) define작업 이후 발생 문제로 메모리 반환된 Pet이 Effect 중일때 발생(Owner로 자신을 넘긴경우만)으로 Effect의 들의 NULL 처리 작업. (09.09.17)

	// - 박보근
	#define PBG_MOD_INVENTORY_REPAIR_COST					// 자동수리금액 하향조정(09.06.08)
	#define PBG_ADD_NEWLOGO_IMAGECHANGE						// 로딩화면 이미지 변경(09.08.12)
	#define PJH_NEW_SERVER_SELECT_MAP						// 서버 선택화면 변경(09.08.17)
	#define YDG_MOD_TOURMODE_MAXSPEED						// 투어모드 최고속도 올림 (2009.07.10)

	// - 양대근
	#define YDG_FIX_DOPPELGANGER_BUTTON_COLOR				// 도플갱어 UI 버튼 잠글때 색 이상한 문제 (2009.08.14)
	#define YDG_MOD_DOPPELGANGER_END_SOUND					// 도플갱어 종료 사운드 추가 (2009.08.20)
	#define YDG_DARK_SPIRIT_CRASH_BECAUSEOF_FIX_MEMORY_LEAK	// 메모리 누수 수정때문에 발생한 다크스피릿 튕기는 현상 임시 수정 (2009.09.02)

	// - 안상규
	#define ASG_FIX_QUEST_PROTOCOL_ADD						// 신규 퀘스트 프로토콜 추가.(2009.09.07)
	#define ASG_MOD_QUEST_WORDS_SCRIPTS						// QuestWords.txt 애니 번호와 회수 제거 (2009.10.22) (QuestWords.txt 일본 적용시 디파인 정리할 것)

// 시즌 5 내부 테섭 버그 수정
#define ADD_FIX_AFTER_SEASON5_TEST
#ifdef ADD_FIX_AFTER_SEASON5_TEST
	#define LDK_FIX_CHARACTER_UNLIVE_MOUSE_LOCK				// 맵이동시 캐릭터 삭제후 이동 불가(09.10.29)
	#define LDK_FIX_EMPIREGUARDIAN_UI_HOTKEY				// 제국 수호군npc ui창 열려있을때 캐릭창, 인벤창 안열리도록한다(09.10.28)
	#define LDK_FIX_PETPOSITION_MULTIPLY_OWNERSCALE			// 캐릭터 선택창 에서 캐릭터 크기에 따라 펫위치 수정(09.10.28)
	#define YDG_FIX_DOPPELGANGER_NPC_WINDOW_SCREEN_SIZE		// 도플갱어 NPC창 열렸을떄 파티창과 겹쳐지는 문제(09.10.28)
	#define YDG_FIX_MASTERLEVEL_PARTY_EXP_BONUS_BLOCK		// 마스터레벨이 파티 경험치 증가 아이템을 사지 못하도록 수정(09.10.28)
	#define YDG_FIX_SETITEM_REQUIRED_STATUS_SECRET_POTION	// 비약 사용시 세트 아이템 장착 정보 잘못 나오는 버그 수정 (09.10.29)
	#define LJH_ADD_LOOK_FOR_MOVE_ILLEGALLY_BY_MSG			// 유저가 채팅메세지를 이용 이동할수 없는 맵에 이동하려 하는지 판정(2009.10.29)
	#define PBG_FIX_SETOPTION_CALCULATE_SECRET_POTION		// 비약관련 세트 옵션 적용버그(09.11.04)
		
// 시즌 5 3차 본섭버젼 버그 수정
	#define LJH_FIX_BUG_BUYING_AG_AURA_WHEN_USING_SD_AURA	// SD증가 오라를 하용하는 중 AG오라를 구입할 수 있는 버그 수정(09.11.16)
	#define PBG_FIX_JELINTNPC_TEXTWIDTH						// 제린트npc ui설정 버그 수정(09.11.10)
	#define PBG_FIX_STOPBGMSOUND							// 로딩이후 bgm사운드 종료되지 않는 버그 수정(09.11.10)
	#define PBG_FIX_SDELITE_OVERLAY							// sd엘리트 물약 겹쳐지지 않는 버그 수정(09.11.17)
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
	#define LJH_MOD_JPN_PARTY_EXP_WITH_PARTY_BUFF			// 파티 버프 사용시 추가로 얻는 경험치 변경(+190%, 한명 추가시 +10%)(09.11.19)
	#define LJH_ADD_JPN_MODIFYING_QUESTINFO_ABOUT_BC_AND_CT	// (일본만-JP_553)퀘스트창에 환영사원과 블러드캐슬에 관한 메시지 수정(09.11.18)
#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE
	#define YDG_FIX_CURSEDTEMPLE_GAUGEBAR_ERROR				// 환영사원 성물 획득/등록 게이지 오류 수정 (2009.12.03)
	#define LJH_FIX_ITEM_CANNOT_BE_EQUIPPED_AFTER_LOGOUT	// (JP_575)재 로그인 시 착용하고 있던 세트 아이템의 옵션 미적용으로 그 아이템을 더이상 착용할수 없는 버그 수정(09.12.10)-> 한국과 동일하게
	#define LJH_FIX_HERO_MOVES_EVEN_NOT_PRESENT				// (국내버그)(JP_524)유저의 캐릭터가 화면에 없을때도 이동할 수 있었던 버그 수정 (예:공성서버으로/에서 이동)(09.12.11)
	#define LJH_FIX_NOT_CALCULATED_BUFF						// 세트계산과 관련되어 버프가 적용되지 않았던 버그 수정(2010.01.15)

// 시즌 5 달토끼 이벤트, 원타임 패스워드 추가, 엘리트 해골전사 변신반지 사양 변경(2010.01.28)  	
	#define LJH_MOD_ELITE_SKELETON_WARRIOR_CHANGE_RING_SETTING	// 일본만 엘리트해골전사 변신반지 사양변경(2010.01.21)
	
	#define ADD_MOON_RABBIT_EVENT								// 달토끼 이벤트 일본에 추가 (2010.01.31)
	#ifdef ADD_MOON_RABBIT_EVENT								
		#define KJH_PBG_ADD_SEVEN_EVENT_2008					// 국내 상용화 7주년 이벤트	(달토끼)(2008.10.30) 
		#define	LJH_MOD_POSITION_OF_REGISTERED_LUCKY_COIN		// 행운의 동전 등록 제한 수가 2^31이 됨에 따라 동전이미지와 겹치는 현상 막기위해 수정(2010.02.16)
	#endif //ADD_MOON_RABBIT_EVENT								
	
	#define LJH_ADD_LOCALIZED_BTNS							// Text.txt에 있는 스크립트를 이용해 예/아니오, 확인/취소 버튼 추가(2010.01.27)
	#ifdef LJH_ADD_LOCALIZED_BTNS
		#define LJH_ADD_ONETIME_PASSWORD					// 원타임 패스워드 추가(2010.01.12~)	
	#endif //LJH_ADD_LOCALIZED_BTNS

#endif //ADD_FIX_AFTER_SEASON5_TEST
#endif	// SEASON5
#endif	// SELECTED_LANGUAGE == LANGUAGE_JAPANESE || defined(LDS_PATCH_GLOBAL_100520)|| SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || SELECTED_LANGUAGE == LANGUAGE_CHINESE


/*--------------------------------------부분 유료화 8차------------------------------------------*/
// 치유의 스크롤, 희귀아이템 티켓, 자유입장권(도플갱어, 바르카, 바르카 제7맵), 스켈레톤 변신반지, 스켈레톤펫, 퀘스트 요구사항,보상 추가
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE  || SELECTED_LANGUAGE == LANGUAGE_TAIWANESE || defined(LDS_PATCH_GLOBAL_100520) || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || SELECTED_LANGUAGE == LANGUAGE_CHINESE

#define ADD_TOTAL_CHARGE_8TH
#ifdef  ADD_TOTAL_CHARGE_8TH
	#define YDG_ADD_HEALING_SCROLL						// 치유의 스크롤 (2009.12.03)
	
	#define LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12		// 희귀아이템 티켓 7-12 추가 (2010.02.12)

	#define LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH	//도플갱어, 바르카, 바르카 제 7맵 자유입장권 추가(2010.02.17)
	
	#define YDG_ADD_SKELETON_CHANGE_RING				// 스켈레톤 변신반지 (2009.12.03)
	#ifdef YDG_ADD_SKELETON_CHANGE_RING				
		#define YDG_FIX_MOVE_ICARUS_EQUIPED_SKELETON_CHANGE_RING	// 스켈레톤변신반지 착용후 이카루스로 이동할 수 없는 현상 (10.02.16)
	#endif //YDG_ADD_SKELETON_CHANGE_RING				
	
	#define ADD_SKELETON_PET							// 스켈레톤 펫 (2009.12.03)
	#ifdef  ADD_SKELETON_PET
		#define YDG_ADD_SKELETON_PET					// 스켈레톤 펫 
		#define YDG_MOD_SKELETON_NOTSELLING				// 스켈레톤 펫 반지 안팔리게 설정(2010.01.20)
		#define YDG_FIX_SKELETON_PET_CREATE_POSITION	// 스켈레톤 펫 생성 위치 수정 (2010.01.26)
		#define LJH_FIX_IGNORING_EXPIRATION_PERIOD		// 일본에는 아직 기간제가 적용되지 않으므로 bExpiredPeriod를 사용하지 않도록 수정(2010.02.11)
		#define YDG_FIX_CHANGERING_STATUS_EXPIRE_CHECK	// 변신반지 스탯올릴때 만료 체크 (2010.01.25)
		#define LDK_FIX_PC4_GUARDIAN_DEMON_INFO			// 데몬 캐릭터정보창에 공격력,마력,저주력 추가 데미지 적용(09.10.15)
														//  --> 캐릭터정보창에 펫 종류에 의한 저주력 추가 데미지를 적용 시키기 위해 추가(10.02.19)
		#define LJH_FIX_NOT_POP_UP_HIGHVALUE_MSGBOX_FOR_BANNED_TO_TRADE	//NPC에게 판매 금지된 아이템을 판매하려고 할때 고가의 아이템이라고 나오는 메세지 창을 안나오게 수정(10.02.19)
	#endif  //ADD_SKELETON_PET
	
	#define YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT			// 변신반지 한종류만 장비하도록 수정 (2009.12.28)
	#define ASG_ADD_QUEST_REQUEST_REWARD_TYPE			// 퀘스트 요구사항, 보상 추가(2009.12.15)
#endif  //ADD_TOTAL_CHARGE_8TH
#endif	//SELECTED_LANGUAGE == LANGUAGE_JAPANESE || defined(LDS_PATCH_GLOBAL_100520) || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || SELECTED_LANGUAGE == LANGUAGE_CHINESE
/*--------------------------------------여기까지 부분 유료화 8차------------------------------------------*/


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 시즌 5-2, 5-3 - 스킬아이콘 리뉴얼, 미니맵, 아이템 업그레이드(14, 15lv), 겐스 시스템, 로렌시장 (10.04.27)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE  || SELECTED_LANGUAGE == LANGUAGE_TAIWANESE || defined(LDS_PATCH_GLOBAL_100520) || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || SELECTED_LANGUAGE == LANGUAGE_CHINESE

#define SEASON5_2
#ifdef  SEASON5_2

	#define KJH_ADD_SKILLICON_RENEWAL					// 스킬아이콘 리뉴얼(10.01.20)

	#define PJH_ADD_MINIMAP								// 미니맵 

	#define LDK_ADD_ITEM_UPGRADE_14_15					// 아이템 14 15
	#ifdef LDK_ADD_ITEM_UPGRADE_14_15
		#define LDK_ADD_14_15_GRADE_ITEM_HELP_INFO			// 아이템 14 15 도움말 추가(09.12.2)
		#define LDK_ADD_14_15_GRADE_ITEM_VALUE				// 아이템 14 15 판매금액 추가(09.12.2)
		#define LDK_ADD_14_15_GRADE_ITEM_RENDERING			// 아이템 14 15 랜더효과 추가(10.01.04) 기존의 13처럼 크롬,블렌드 처리
		#define LDK_ADD_14_15_GRADE_ITEM_SET_EFFECT			// 아이템 14 15 세트이펙트효과 추가(10.01.22)
		#define LDK_ADD_14_15_GRADE_ITEM_TYPE_CHANGE		// 아이템 14 15 래벨 타입 변경(10.01.27)
		#define LDK_ADD_14_15_GRADE_ITEM_MODEL				// 아이템 14 15 모델링 추가(10.01.18)
		#define YDG_FIX_ITEM_EFFECT_POSITION_ERROR			// 아이템 이펙트 위치가 이상하게 나오는 문제 (2010.01.27)
		#define LJH_FIX_NO_EFFECT_ON_WEAPONS_IN_SAFE_ZONE_OF_CURSED_TEMPLE	// 환영사원의 안전지역에서 15레벨 아이템 이펙트 나오지 않는 버그 수정(10.04.16)
	#endif //LDK_ADD_ITEM_UPGRADE_14_15

	#define ASG_ADD_GENS_SYSTEM							// 겐스 시스템(2009.09.14)	[주의] 겐스 시스템 디파인이 켜지면 ASG_ADD_UI_NPC_MENU 디파인은 꺼짐. 해외 추가시 주의.
	#ifdef ASG_ADD_GENS_SYSTEM
		#define ASG_ADD_INFLUENCE_GROUND_EFFECT				// 겐스 세력 구분 바닥 이펙트(2009.10.07)
		#define ASG_ADD_GENS_MARK							// 겐스 마크 표시(2009.10.09)
		#define ASG_ADD_GENS_NPC							// 겐스 NPC 추가(2009.10.12)
		#define ASG_ADD_GATE_TEXT_MAX_LEVEL					// Gate.txt 최대 레벨 추가.(2009.10.16)	[주의] 리소스 컨버터 새로운 소스로 컴파일 할 것.
		#define ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL				// Movereq.txt 최대 레벨 추가.(2009.10.16)	[주의] 리소스 컨버터 새로운 소스로 컴파일 할 것.

		#define ASG_FIX_GENS_JOINING_ERR_CODE_ADD			// 겐스 가입 에러 코드 추가.(연합길드장 관련)(2009.11.12)
		#define ASG_MOD_GUILD_RESULT_GENS_MSG				// 길드 가입시 겐스 관련 메시지.(2009.11.23)
		#define PBG_ADD_GENSRANKING							// 겐스 랭킹(09.12.01)
		#define PBG_FIX_GENSREWARDNOTREG					// 겐스 미가입자가 보상받을시 문구 수정(10.01.28)
		#define ASG_FIX_GENS_STRIFE_FRIEND_NAME_BLOCK		// 겐스 분쟁지역에서 친구 이름 자동입력 막기(2010.02.03)
		#define ASG_MOD_GENS_STRIFE_ADD_PARTY_MSG			// 겐스 분쟁지역 관련 파티 메시지 추가.(2010.02.03)
		#define ASG_MOD_GM_VIEW_NAME_IN_GENS_STRIFE_MAP		// 겐스 분쟁지역에서 GM은 타세력 캐릭터 이름 보이게(10.02.26)
		#define LJH_FIX_DIVIDE_LINE_BY_PIXEL_FOR_GENS_RANK	// 겐스에서 랭크이름이 너무 길어 한줄에 다 나오지 않아 nPixelPerLine = 240으로 수정 _ 일본 때문(10.03.10)
		#define LJH_FIX_DEFINE_GENS_TEAMNAME_LENTH_TO_16	// 일본어의 경우 한 문자를 2바이트로 처리하는데 바네르트를 표현하기 위해 12바이트가 필요하기 때문에 수정(10.03.15)
		#define PBG_MOD_STRIFE_GENSMARKRENDER				// 분쟁지역유저표시 변경(10.02.18)
		#define LJH_FIX_UNABLE_TO_TRADE_OR_PURCHASE_IN_TROUBLED_AREAS	// 분쟁지역에서 /거래, /구입 명령어 제한 (10.03.31)
		
		#define DEFINES_FOR_FOREIGN_COUNTRIES_WHERE_WITHOUT_APPLYING_MU_BLUE	// !!! MU BLUE가 적용되고 나면 이 define 해제 !!!
		#ifdef  DEFINES_FOR_FOREIGN_COUNTRIES_WHERE_WITHOUT_APPLYING_MU_BLUE
			#define LJH_MOD_TO_USE_ISBLUEMUSERVER_FUNC		// 블루가 적용되지 않은 국가에서 현재 접속해 있는 서버가 블루인지 판별하기 위해 포함(10.03.17)
			#define LJH_MOD_TO_USE_DIVIDESTRINGBYPIXEL_FUNC	// KJH_ADD_INGAMESHOP_UI_SYSTEM에 의해 정의된 함수를 사용하기 위해 추가 (10.03.17)
		#endif  //DEFINES_FOR_FOREIGN_COUNTRIES_WHERE_WITHOUT_APPLYING_MU_BLUE
	#endif	// ASG_ADD_GENS_SYSTEM

	
	#define LDS_ADD_MAP_UNITEDMARKETPLACE				// 통합 시장 서버 (로렌 시장) 추가 (09.12.04)
	#ifdef LDS_ADD_MAP_UNITEDMARKETPLACE
		#define LDS_ADD_NPC_UNITEDMARKETPLACE				// NPC 로렌시장 1.보석류 조합 및 해체 라울, 2.시장 상인 줄리아, 3.물약 및 기타 잡화 아이템 상인 크리스틴 (09.12.15)
		#define LDS_ADD_UI_UNITEDMARKETPLACE				// UI 로렌시장 메세지 처리 1.카오스케슬 입장 불가 2.결투신청불가 3.파티신청 불가 4.
		#define LDS_ADD_SERVERPROCESSING_UNITEDMARKETPLACE	// 서버와의 교신 처리 1.통합시장맵으로 이동 2. (09.12.23)
		#define LDS_ADD_SOUND_UNITEDMARKETPLACE				// 통합시장 환경음, 배경음 처리. (10.01.12)
		#define LDS_ADD_MOVEMAP_UNITEDMARKETPLACE			// 맵이동 윈도우에 "통합 시장" 추가. (10.01.12)
		#define LDS_ADD_EFFECT_UNITEDMARKETPLACE			// 통합시장 NPC 등에 추가 EFFECT. (10.01.14)
		#define LDK_FIX_CHARACTER_UNLIVE_MOUSE_LOCK			// 맵이동시 캐릭터 삭제후 이동 불가(09.10.29)
		#define LJH_ADD_TO_USE_PARTCHARGESHOP_IN_LOREN_MARKET	// 부분유료상점 이용할 수 있는 장소로 로렌마켓 추가(10.03.31)
		#define LDS_FIX_MEMORYLEAK_WHERE_NEWUI_DEINITIALIZE		// 로렌시장 DEAD현상 : NewUI 내 폐기 처리 메모리 미반환으로 인한 메모리 누수 작업 (10.03.23)
		#define LJH_ADD_SAVEOPTION_WHILE_MOVING_FROM_OR_TO_DIFF_SERVER	// (JP_622, JP_624)(국내버그)다른 서버(현재 공성서버, 로렌시장)에서부터 또는 다른 서버로 이동할 때 옵션이 저장 되지 않는 버그 수정(10.04.16)
	#endif // LDS_ADD_MAP_UNITEDMARKETPLACE

	#define KJH_FIX_MOVE_ICARUS_EQUIPED_PANDA_CHANGE_RING		// 팬더변신반지 착용후 이카루스로 이동할 수 없는 현상 (09.11.30)

// 시즌 5-2, 5-3 본섭버젼 버그 수정 (10.05.13)
	#define ADD_FIX_AFTER_SEASON5_3_REAL_SERVER_UPDATE
	#ifdef ADD_FIX_AFTER_SEASON5_3_REAL_SERVER_UPDATE 
		#define LJH_FIX_CHANING_FONT_FOR_DUAL_WATCHUSER_NAME	// 결투장 관람하는 유저 아이디의 폰트가 계속적으로 '일반'과 '굵게'를 반복하며 렌더 되는 현상 수정(10.05.03)
		#define LJH_FIX_INCORRECT_SCHEDULE_FOR_BC_FREE_TICKET	// 레벨 80이하의 캐릭터가 안전디에에서 [블러드캐슬자유입장권]을 클릭할 경우, 정상적인 퀘스트 타임스케쥴이 표시되지 않는 현상 수정(10.05.04)
		#define LJH_FIX_CANNOT_CLICK_BASKETS_IN_CURSED_TEMPLE	// 환영사원에서 성물을 적 진영의 보관함에 등록하려고 한 뒤에는 보관함을 클릭할 수 없는 버그 수정(10.05.04) 
		#define LJH_FIX_NOT_INITIALIZING_BATTLECASTLE_UI		// 로렌협곡에서 크라이울프요새로 이동할 때 공성UI가 초기화 되지 않아 화면 우측하단을 클릭해도 이동하지 않는 버그 수정(10.05.10)
		#define LJH_FIX_GETTING_ZEN_WITH_PET_OF_OTHER_PLAYER	// 다른 플레이어의 펫이 젠을 먹어주는 버그 수정(10.05.10)
	#endif //ADD_FIX_AFTER_SEASON5_3_REAL_SERVER_UPDATE

//-----------------------------------------------------------------------------
// [ 시즌 5 - Part 3 추가 버그수정]
	#define KJH_FIX_BTS158_TEXT_CUT_ROUTINE							// UI에 맞게 Text를 자를시 이상현상 수정 (10.05.07)
	#define KJH_MOD_BTS173_INGAMESHOP_ITEM_STORAGE_PAGE_ZERO_PAGE	// 인게임샵 보관함 아이템이 없을시 0페이지 출력 (10.05.13)
	#define KJH_FIX_BTS179_INGAMESHOP_STORAGE_UPDATE_WHEN_ITEM_BUY	// 인게임샵 아이템 구입시 선물함 탭에서 보관함 아이템이 갱신되는 버그수정 (10.05.13)
	#define KJH_FIX_MINIMAP_NAME									// 미니맵에서 NPC/Portal Name 이 짤리는 버그 수정 (10.05.14) - minimap 스크립트 모두 재컨버팅
	#define KJH_MOD_BTS184_REQUIRE_STAT_WHEN_SPELL_SKILL			// 스킬 시전시 스텟 요구치에 따라 스킬 사용 변경 (10.05.17)
	//^#define KJH_FIX_EMPIREGUARDIAN_ENTER_UI_RENDER_TEXT_ONLY_GLOBAL	// 제국수호군 입장 UI(제린트NPC) Text 수정 (10.05.17) - 글로벌 전용
	//^#define KJH_FIX_DOPPELGANGER_ENTER_UI_RENDER_TEXT_ONLY_GLOBAL	// 도펠갱어 입장 UI(루가드NPC) Text 수정 (10.05.19) - 글로벌 전용
	#define KJH_FIX_BTS167_MOVE_NPC_IN_VIEWPORT						// 뷰포트안에 NPC가 들어오면 NPC가 갑자기 이동하는 버그수정 (10.05.17)
	#define KJH_MOD_BTS191_GOLD_FLOATING_NUMBER						// 아이템이나 캐시 금액을, 소숫점자리까지 계산 (10.05.18)
	#define KJH_FIX_BTS204_INGAMESHOP_ITEM_STORAGE_SELECT_LINE		// 인게임샵 보관함에서 1번째 아이템 사용시, 포커싱이 3번째 아이템으로 선택되는 버그수정 (10.05.18)
	#define	KJH_FIX_BTS206_INGAMESHOP_SEND_GIFT_MSGBOX_BLANK_ID		// 인게임샵 선물하기 메세지창에서 ID입력하지 않고 확인 버튼 클릭시, 선물확인 메세지창이 뜨는 버그 (10.05.18)
	#define KJH_MOD_COMMON_MSG_BOX_BTN_DISABLE_TEXT_COLOR			// 공통 메세지박스 버튼 Disable시 글자색 수정 (10.05.18)
	//^#define	KJH_MOD_BTS208_CANNOT_MOVE_TO_VULCANUS_IN_NONPVP_SERVER	// nonPVP 서버일대 불카누스로 이동불가 (10.05.18)
	#define KJH_FIX_BTS207_INGAMESHOP_SEND_GIFT_ERRORMSG_INCORRECT_ID	// 인게임샵 선물하기에서 상대방 아이디가 틀렸을때 에러 메세지 (10.05.19)
	#define KJH_FIX_BTS251_ELITE_SD_POTION_TOOLTIP					// 엘리트 SD물약 툴팁이 안나오는 버그 수정 (10.05.24)
	#define KJH_FIX_BTS260_PERIOD_ITEM_INFO_TOOLTIP					// 기간제아이템 기간제 정보 툴팁 버그 수정 (10.05.24)
	#define KJH_MOD_INGAMESHOP_GIFT_FLAG							// 인게임샵 선물하기 Flag로 선물하기 버튼 On/Off (10.05.25)
	#define ASG_FIX_QUEST_GIVE_UP									// 퀘스트 포기 버그 수정. 퀘스트 상태(QS) 0xfe삭제됨.(10.05.26)

//-----------------------------------------------------------------------------
// [ 시즌 5 - Part 3 추가작업]
	#define KJH_MOD_INGAMESHOP_UNITTTYPE_FILED_OF_PRODUCT_SCRIPT	// Product 게임샵 스크립트의 UnitType필드로 단위명 변경 (10.04.22) - 10.03.29일자 인게임샵 라이브러리 적용
	#define KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT				// 인게임샵 보관함 페이지 단위 방식으로 변경 (10.04.27)
	#define KJH_MOD_INGAMESHOP_DOMAIN_EACH_NATION					// 국가별 게임샵 도메인 설정 (10.05.04)

//-----------------------------------------------------------------------------
// [ 시즌 5 - Part 3 이후 버그수정]
	#define KJH_FIX_INIT_EVENT_MAP_AT_ABNORMAL_EXIT			// 비정상적적인 종료시 이벤트맵 초기화 (10.04.20)

	//^#define KJH_FIX_MOVE_MAP_GENERATE_KEY				// 맵 이동(&더미스킬 프로토콜)시 생성되는 키값 수정

	#define ASG_ADD_LEAP_OF_CONTROL_TOOLTIP_TEXT			// 통솔의 비약 툴팁 텍스트 추가(2010.01.28)
	//^#define PBG_FIX_GAMECENSORSHIP_1215							// 12/15세 이용가능 마크 출현 버그수정(10.02.09)
	#define LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT		// 통합시장 이동 시에 사용자의 키 및 마우스 인풋 입력을 막지 않아 생기는 문제로 일괄 Skip 처리. (10.02.02) 
	//^#define PBG_MOD_GAMECENSORSHIP_RED						// 레드의 경우도 18이상가능가로 조정(09.01.26)[10.02.04본섭패치시 테섭/본섭 블루 동시 적용]
	#define LDS_FIX_DISABLEALLKEYEVENT_WHENMAPLOADING			// 맵이동 요청이후 서버로부터 맵이동 결과가 오기전까지 모든 키입력을 막습니다. (로렌시장, 공성 서버 이동 요청시에 esc로 옵션창 "게임종료"호출이후 모든 키입력이 안되는 버그 ). (10.01.28)

//-----------------------------------------------------------------------------
// [ 시즌 5 - Part 2 이후 버그수정]
	#define LJH_FIX_EXTENDING_OVER_MAX_TIME_4S_To_10S			// 해외사용자들 일부 사용자에게 캐시샵이 열리지 않아 Max Time을 기존 4초에서 10초로 변경(09.12.17)
	#define KJH_FIX_INGAMESHOP_SCRIPTDOWN_BUG					// 인게임샵 스크립트 다운로드 버그 (09.12.28)
	#define PJH_FIX_HELP_LAYER									// 헬프 레이어 최상위로 변경(2009.12.01)
	#define LDS_FIX_VISUALRENDER_PERIODITEM_EXPIRED_RE			// 인게임샾 아이템들중 목걸이, 반지들에 대한 기간제 종료 이후 사용 불능 처리 (인벤토리상에서 비주얼 처리만.) (09.12.10)
	#define KJH_FIX_SOURCE_CODE_REPEATED						// 같은 소스코드 중복된것 수정 (09.10.12)
	#define ASG_MOD_QUEST_OK_BTN_DISABLE						// 퀘스트 진행 확인 버튼 비활성화 상태 추가(2009.11.23)
	#define LDK_MOD_BUFFTIMERTYPE_SCRIPT						// 버프타이머 타입값을 buffEffect.txt의 Viewport로 받아서 적용(09.11.10) - enum.h : 타이머 필요한 버스 등록시 eBuffTimeType에 등록안해도 됨
	#define KJH_FIX_INGAMESHOP_INIT_BANNER						// 인게임샵 배너 다운로드 후 초기화되지 않아 Render가 안돼는 현상 (09.11.10)
	
	
	#define YDG_ADD_GM_DISCHARGE_STAMINA_DEBUFF					// GM이 거는 스태미너 방전 디버프 (2009.11.13)
	#define PBG_FIX_MSGBUFFERSIZE								// 메시지박스내 문구 잘리는 버그수정(09.11.13)
	#define PBG_FIX_MSGTITLENUM									// 메시지박스 title문구 잘못나오는 버그 수정(09.11.13)
	#define LDS_MOD_MODIFYTEXT_TOPAZRING_SAPIRERING				// 사파이어링과 토파즈 링의 텍스트 변경 (09.11.06)
	#define LDK_MOD_TRADEBAN_ITEMLOCK_AGAIN						// 일부 유료 아이템중 트레이드 가능 불가능 변경(NOT DEFINE 처리)(09.10.29) (기획에서 수시로 바뀜 ㅡ.ㅡ+)
	#define KJH_MOD_SHOP_SCRIPT_DOWNLOAD						// 샵 스크립트 다운로드 방식 변경 (09.11.09)
	#define KJH_FIX_SHOP_EVENT_CATEGORY_PAGE					// 샵 이벤트 카테고리 페이지 버그 수정 (09.11.09)
	#define KJH_FIX_EXPIRED_PERIODITEM_TOOLTIP					// 기간만료된 기간제 아이템의 툴팁Render가 안돼는 버그 수정 (09.11.09)
	#define LDS_MOD_INGAMESHOPITEM_RING_AMULET_CHARACTERATTR	// 인게임샾 아이템들중 반지, 목걸이 시리즈의 STATUS 수치 반영.
	#define LDK_FIX_NEWWEALTHSEAL_INFOTIMER_AND_MOVEWINDOW		// 풍요의인장 버프정보 시간표시 추가, 버프사용시 이동창 비활성화 (09.11.06)
	#define PBG_FIX_TRADECLOSE									// 거래중 인벤을 닫을시에 거래창은 닫히지 않는 버그 수정(09.11.06)
	#define LDS_MOD_INGAMESHOPITEM_POSSIBLETRASH_SILVERGOLDBOX	// 금,은 상자 버리기가 가능하도록.
	#define KJH_MOD_RENDER_INGAMESHOP_KEEPBOX_ITEM				// 게임샵 보관함 아이템의 수량이 1개일때 Render 안함 (2009.11.03)
	#define LDK_MOD_INGAMESHOP_ITEM_CHANGE_VALUE				// 혼합유료화 아이템 가격 변경(09.10.29)
	#define KJH_MOD_INGAMESHOP_PATCH_091028						// 인게임샵 수정 - 이벤트 카테고리, 인게임샵 라이브러리 업데이트 (09.10.28)
	#define LDK_FIX_BLUESERVER_UNLIMIT_AREA						// 블루섭 전용 이동창 이동제한 설정중 추가지역 제한 해제(09.10.28)
#endif  //SEASON5_2
#endif	//SELECTED_LANGUAGE == LANGUAGE_JAPANESE || defined(LDS_PATCH_GLOBAL_100520) || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || SELECTED_LANGUAGE == LANGUAGE_CHINESE

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 여기까지 시즌 5-2, 5-3
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if	SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE 
	#define LJH_MOD_DOMAIN_NAME_TO_IP_FOR_VIETNAM_VERSION	// 베트남 신규 접속서버 교체한 뒤에 기존 도메인으로 접속하지 못해 아이피로 입력 (10.04.12)
	#define KJH_FIX_BTS167_MOVE_NPC_IN_VIEWPORT						// 뷰포트안에 NPC가 들어오면 NPC가 갑자기 이동하는 버그수정 (10.05.17)
#endif	//SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE

/*--------------------------------- 해외 맞춤형 누적 버그 수정 -------------------------------------*/
#if	SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || SELECTED_LANGUAGE == LANGUAGE_ENGLISH || SELECTED_LANGUAGE == LANGUAGE_TAIWANESE || SELECTED_LANGUAGE == LANGUAGE_CHINESE
	#define LJH_FIX_BUG_CASTING_SKILLS_W_CURSOR_OUT_OF_VIEWPORT			// (국내버그)마우스 오른쪽 버튼 클릭 중 커서가 게임뷰포트를 벗어나 인벤창등에 있을때도 기술을 계속 쓸수 있는 버그 수정(09.12.02)
	#define PBG_MOD_VIEMAPMOVE											// 베트남 이동창 이동의 인장 없이 이동 가능하게 수정(09.12.07) 
	#define LJH_FIX_NO_MSG_FOR_FORCED_MOVEOUT_FROM_BATTLESOCCER			// 전투축구중 길드마스터가 아닌 자신이 강제로 경기중 경기장 밖으로 나갔을 때 결과 메세지 출력하지 않음(09.12.28) 
	#define YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT							// 변신반지 한종류만 장비하도록 수정 (2009.12.28)
	#define LJH_MOD_EXTENDING_NUM_OF_MIX_RECIPES_FROM_100_TO_1000		// 한 조합 창에서 조합할수 있는 종류의 개수 제한을 100개에서 1000개로 증가(10.02.23)
	#define LJH_FIX_LOADING_INFO_OF_AN_OTHER_CHARACTER_IN_LOGIN			// (JP_607)(국내버그)캐릭터 선택화면에서 캐릭터 B를 선택하고 캐릭터 A를 클릭하면서 엔터키를 동시에 누르면 캐릭터 B의 일부정보로 로그인(10.04.01)

	#define LJH_FIX_BUG_DISPLAYING_NULL_TITLED_QUEST_LIST				// (JP_618)(국내버그)서버에서 받은 퀘스트 인덱스가 존재하지 않아 (null)로 표시되는 항목은 퀘스트 목록을 만들때 넣지 않도록 수정(10.04.14)
	#define LJH_FIX_BUG_SELLING_ITEM_CAUSING_OVER_LIMIT_OF_ZEN			// (JP_629)(국내버그)소지한도액을 초과하는 판매액의 아이템을 팔지 못하게 서버로 메세지도 보내지 않도록 수정(10.04.15)
	#define LJH_FIX_BUG_MISSING_ITEM_NAMES_ITEM_HELPER_7				// (JP_633)(국내버그)영혼의 물약, 축복의 물약을 습득할 때 '물약+1'이라고 나오는 버그 수정(10.04.15)
	#define LJH_MOD_CANT_OPENNING_PERSONALSHOP_WIN_IN_CURSED_TEMPLE		// (JP_657)환영사원에서는 개인상점창을 열지 못하도록 수정
	#define LJH_FIX_BUG_NOT_REMOVED_PET_WITH_1_DUR_WHEN_PLAYER_KILLED	// (JP_651)플레이어가 죽을때 장착한 내구도 1인 팻을 잡고있으면 없어지지 않는 버그 수정(10.04.20)
	#define PBG_FIX_DROPBAN_GENS										// 버리기 아이템 수정

#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || SELECTED_LANGUAGE == LANGUAGE_ENGLISH || SELECTED_LANGUAGE == LANGUAGE_TAIWANESE || SELECTED_LANGUAGE == LANGUAGE_CHINESE

/*--------------------------------- 중국 쪽 요청에 의한 수정(2010.10.28) -------------------------------------*/
#if SELECTED_LANGUAGE == LANGUAGE_CHINESE								
	#define KJW_FIX_CRYWOLF_NPC_MINIMAP_RENDER							// 중국쪽 크라이울프의 NPC가 미니맵에 표시되지 않는 문제 수정(10.10.28)
	#define KJW_FIX_CHINESE_POTION_PRICE								// 중국 물약관련 물가정책 제외시킴(10.10.28)
	#define KJH_FIX_POTION_PRICE										// 상점 물약가격 버그수정 (10.04.16)
	#define KJH_ADD_SERVER_LIST_SYSTEM									// ServerList.txt 스크립트 중국에도 적용 (10.11.09) by LJW
	#define ASG_ADD_SERVER_LIST_SCRIPTS									// ServerList.txt 스크립트 중국에도 적용 (10.11.09) by LJW
	#define LJW_FIX_CASHSHOPITEM_BUFF_CONTENT_IN_CHANA					// 중국에서 상승의 인장, 풍요의 인장 사용시, '맵 이동창 사용 기능' 필요없음 (10.11.13)
	#define LJW_FIX_MANY_FLAG_DISAPPEARED_PROBREM						// 영예의 반지(중국 경극 깃발이펙트) 게임중 사라지는 문제 수정 (10.11.15) 
	#define	LJW_FIX_PARTS_ENUM											// 캐릭터 EtcPart 부분관련 enum 변수 추가 및 수정 (10.11.15)
#endif // SELECTED_LANGUAGE == LANGUAGE_CHINESE

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 시즌 5-4
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
#define SEASON5_4
#ifdef  SEASON5_4
	//-----------------------------------------------------------------------------
	// [ 시즌 5 - Part 4 이후 버그수정]

	#define KJH_FIX_POTION_PRICE							// 상점 물약가격 버그수정 (10.04.16)
	#define KJH_FIX_INIT_EVENT_MAP_AT_ABNORMAL_EXIT			// 비정상적적인 종료시 이벤트맵 초기화 (10.04.20)
	#define PBG_FIX_SATAN_VALUEINCREASE						// 사탄 착용시 수치 증가 버그 수정(10.04.07)
	#define PBG_MOD_GUARDCHARMTEXT							// 수호의부적아템 문구수정(10.04.14)
	#define LDK_MOD_GUARDIAN_DROP_RESIZE					// 데몬 수호정령 바닥에 놓였을때 사이즈 조정(10.03.04)
	#define PBG_FIX_CURSEDTEMPLE_SYSTEMMSG					// 환영의사원 메시지 처리 버그 수정(09.10.22)
	#define LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT			// 골든오크참 착용효과수치 기획 변경(HP +100, SD+500) (10.05.06)
	#define LJH_FIX_APP_SHUTDOWN_WEQUIPPING_INVENITEM_WITH_CLICKING_MOUSELBTN // 마우스 왼쪽버튼을 누른채로 인벤아이템을 장착하려고 할때 프로그램 튕기는 버그 수정(10.04.29)
	#define KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL		// 인게임샵 선물하기에서 다크로드가 아닌 클래스도 선물하기 받을수 있는문제 수정 (10.06.23)
	#define ASG_FIX_MONSTER_MAX_COUNT_1024					// #define MAX_MONSTER 512 -> 1204로 수정(10.06.29)
	#define LJH_FIX_REARRANGE_INVISIBLE_CLOAK_LEVEL_FOR_CHECKING_REMAINING_TIME	// 투명망토(lv1~8)과 블러드캐슬(lv0~7)의 차이때문에 8레벨 투명망토 우클릭 할 때 시간이 이상하게 나오는 버그 수정(10.07.07) 
	#undef  LJH_FIX_IGNORING_EXPIRATION_PERIOD				// 일본에 기간제가 적용되자 않았을때  bExpiredPeriod를 사용하지 않도록 수정했었던것 해제(10.02.11)
		
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
	#define LJH_MOD_SEND_TYPE_OF_PAYMENT_DUE_TO_CHANGED_LIBRARY	// 라이브러리의 형식이 한국과 달라 캐쉬 혹은 고블린포인트로 구입하는지 여부를 서버로 전송(10.07.13)
#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE
	//-----------------------------------------------------------------------------
	// [ 시즌 5 - Part 4 ]
	// 1. 소환술사 마스터레벨
	// - 박종훈
	#define PJH_ADD_MASTERSKILL								//소환술사 마스터레벨 추가(2010.03.09)
	#ifdef PJH_ADD_MASTERSKILL								
		#undef KJH_FIX_WOPS_K22193_SUMMONER_MASTERSKILL_UI_ABNORMAL_TEXT	//해외에 소환술사 마스터스킬 추가(10.05.28)
	#endif //PJH_ADD_MASTERSKILL								

	//------------------------------------------------
	// 2. 사냥터 확장
	// - 이동근, 이동석
	#define LDK_LDS_EXTENSIONMAP_HUNTERZONE					// 사냥터 확장 맵 (기존 맵 리뉴얼 및 몬스터, 보스급 몬스터 추가.). (10.02.18)
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

	//------------------------------------------------
	// 3. 인벤장착 시스템
	// - 이주흥
	#define LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY		// 인벤장착 아이템을 위한 시스템 추가(10.02.22) 
	#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY	
		#define LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM		// 인벤장착 아이템 추가-매조각상, 양조각상, 편자 (10.02.26)
		#define LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2	// 추가인벤장착 아이템 추가-오크참, 골든오크참, 메이플참, 골드메이플참 (10.03.24)
	#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
	//-----------------------------------------------------------------------------
	// 4. 게임츄 웹스타터/ Season5-5 미니업데이트, 보석조합
	// - 이은미

	//#define LEM_ADD_GAMECHU
//	#define LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX

	//-----------------------------------------------------------------------------
	// [ 시즌 5 - Part 4 추가 버그수정_3]
	#define LEM_FIX_JP0716_INGAMESHOP_GIFT_POINT					// 선물하기 확인 메세지 박스 프로토콜 bool타입 추가 [lem_2010.9.2]
	#define LEM_FIX_JP0714_JEWELMIXSOUND_INGAMBLESYSTEM				// 겜블상점이 켜져있을 때 조합 보석 소리 추가 [lem_2010.9.8]
	#define LEM_FIX_JP0711_JEWELBOX_DROPFREE						// ITEM_POTION 141~144보석함류 드랍 가능하게 풀기. [lem_2010.9.8]
	#define LEM_FIX_SEAL_ITEM_BUYLIMIT								// 마스터인장 구입 제한 [lem_2010.9.9]

	//-----------------------------------------------------------------------------
	// [ 시즌 5 - Part 4 추가 버그수정_2]
	#define LEM_FIX_RENDER_ITEMTOOLTIP_FIELD						// 바닥에 떨어진 아이템 색상 오류 수정 [lem_2010.8.19]
	#define LEM_FIX_USER_LOGOUT										// 유저가 로그아웃을 하는 대기 상태 변수 [lem_2010.8.19]
	#define LEM_FIX_WARNINNGMSG_DELETE								// 버프 아이템에 대하여 중복 사용에 대한 메세지 삭제 [lem_2010.8.19]
	#define LEM_FIX_USER_LOGOUT_SKILL_ENABLE						// 로그아웃 대기 상태일 때에 스킬 사용 잠금 [lem_2010.8.19]
	#define LEM_FIX_AUTOSKILLBAR_SAVE								// 자동공격에 설정되어 있는 스킬 로그아웃 전까지 저장 [lem_2010.8.19]
	#define LEM_FIX_ITEMSET_FROMJAPAN								// 기존 캐쉬템을 일본에서만 일반템으로 판매하여 발생된 구매/교환/장착/보관/버리기 버그수정 [lem_2010.8.19]
	#define LEM_FIX_INGAMESHOP_BUY_TYPE								// 인게임샵 아이템 구매 수단의 여부 변수 설정 [lem_2010.8.19]
	#define LEM_FIX_INGAMESHOP_ITEM_CARD							// 인게임샵 소환술사 카드 선물하기 제외 [lem_2010.8.19]

	//-----------------------------------------------------------------------------
	// [ 시즌 5 - Part 4 추가 버그수정]
	#define LEM_FIX_WARNINGMSG_BUYITEM								// 구매요청시 경고가 필요한 아이템에 대하여 추가적으로 경고창을 띄워준다. [lem.2010.7.28]
	#define LEM_FIX_SEAL_ITEM_MAPMOVE_VIEW							// 인장 사용시 포탈 열리는 기능이 누락된 인장 추가[lem.2010.7.28]
	#define LEM_FIX_SERVERMSG_SEALITEM								// 같은 인장 사용에 대한 서버메세지 리시브 케이스문 추가 [lem.2010.7.28]
	#define LEM_FIX_SHOPITEM_DURABILITY_REPAIRGOLD					// 캐쉬아이템 수리비 제외 추가 [lem.2010.7.28]
	#define LEM_FIX_ITEMTOOLTIP_POS									// 인벤토리 아이템 툴팁 위치 수정 [lem.2010.7.28]
	#define LEM_FIX_MIXREQUIREZEN									// 조합 필요 금액 세율합산 표기 [lem.2010.7.29]

	//-----------------------------------------------------------------------------
	// [ 시즌 5 - Part 2 ]
	// 1. 인게임샵 시스템
	// - 김재희,박보근
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
		//^#define PBG_ADD_CHARACTERSLOT						// 캐릭터 슬롯아이템(09.05.06) - 글로벌 제외
		#define KJH_DEL_PC_ROOM_SYSTEM						// 피씨방 시스템 삭제 (09.10.22) - 피씨방 접속여부는 남겨둠(class CPCRoomPtSys)

		#define PBG_ADD_CHARACTERCARD					// 캐릭터 카드 (다크로드,마검사 생성 카드) (09.05.04) - 소환술사카드 추가(09.06.04)
		#ifdef	PBG_ADD_CHARACTERCARD					
			#undef	PSW_CHARACTER_CARD
		#endif	//PBG_ADD_CHARACTERCARD					
		//^#define LDK_ADD_INGAMESHOP_LIMIT_MOVE_WINDOW		// 국내 전용 이동창 이동제한 설정, 이동의 인장으로 해제 (PSW_SEAL_ITEM관련) - 글로벌 제외
	#endif //KJH_PBG_ADD_INGAMESHOP_SYSTEM

	//------------------------------------------------
	// 2. 추가아이템
	#define INGAMESHOP_ITEM01							// 해외에만 등록되어있거나 새로 생성된 아이템 디파인 추가(2009.09.08)
	#ifdef	INGAMESHOP_ITEM01
		#define LDK_MOD_PREMIUMITEM_DROP					// 프리미엄아이템 버리기제한해제(09.09.16)
		#define LDK_MOD_PREMIUMITEM_SELL					// 프리미엄아이템 판매제한해제(09.09.25)
		#define LDK_ADD_INGAMESHOP_GOBLIN_GOLD				// 고블린금화
		#define LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST		// 봉인된 금색상자
		#define LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST		// 봉인된 은색상자
		#define LDK_ADD_INGAMESHOP_GOLD_CHEST				// 금색상자
		#define LDK_ADD_INGAMESHOP_SILVER_CHEST				// 은색상자
		#define LDK_ADD_INGAMESHOP_PACKAGE_BOX				// 패키지 상자A-F
		#define LDK_ADD_INGAMESHOP_SMALL_WING				// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
		#define LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL			// 신규 풍요의 인장

		#define LDS_ADD_NOTICEBOX_STATECOMMAND_ONLYUSEDARKLORD	// 인게임샾 아이템 // 리셋열매5종 // 통솔리셋열매는 다크로드만 사용가능한 메세지 출력.
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
	//-----------------------------------------------------------------------------
	// [ 시즌 5 - Part 2 이후 버그수정]
	#ifdef KJH_PBG_ADD_INGAMESHOP_SYSTEM
		#define LDK_FIX_EXPIREDPERIOD_ITEM_EQUIP_WARNING			// 만료된 아이템 작용불가 및 경고 
		#define LDS_FIX_INGAMESHOPITEM_PASSCHAOSCASTLE_REQUEST		// 카오스캐슬 자유 입장권 사용시 서버로 사용에 대한 패킷 정보를 보내지 않는 문제 수정 (09.11.02)
		#define KJH_FIX_RENDER_PERIODITEM_DURABILITY				// 기간제 아이템일때 내구도정보가 Render 되는 문제 (2009.11.03)
		#define LDK_FIX_PERIODITEM_SELL_CHECK						// 판매시 기간제 아이템 확인 추가(09.11.09)
		#define LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO	// 캐릭터정보창에서 만료된 데몬, 수호정력 수치 표시 안함(09.11.12)
		#define LDK_MOD_ITEM_DROP_TRADE_SHOP_EXCEPTION				// 카오스부적,카오스카드,데몬,수호정령,팬더펫,팬더변신반지,마법사의반지 일반아이템시 버리기,개인상점,개인거래 가능하도록 처리(09.11.16)
		#define LDK_FIX_EQUIPED_EXPIREDPERIOD_RING_EXCEPTION		// 착용중인 만료된 반지 능력치 계산 예외처리(09.12.11)
		#define LDK_FIX_EQUIPED_EXPIREDPERIOD_AMULET_EXCEPTION		// 착용중인 만료된 목걸이 능력치 계산 예외처리(09.12.11)
		#define PBG_MOD_PANDAPETRING_NOTSELLING						// 펜더 펫 반지 안팔리게 설정(10.01.11)
		#define LDS_ADD_OUTPUTERRORLOG_WHEN_RECEIVEREFRESHPERSONALSHOPITEM	// 로렌 시장 서버로부터 개인상점 구매이후 ReceiveRefreshPersonalShopItem 받을 때 Error Log 정보를 더 자세히 출력 (10.03.30)
		#define KJH_FIX_BTS295_DONT_EXPIRED_WIZARD_RING_RENDER_SELL_PRICE	// 기간제 마법사반지가 기간이 만료되지 않았는데 판매가격이 나오는 버그 수정 (10.05.25)
	#endif // KJH_PBG_ADD_INGAMESHOP_SYSTEM
	//-----------------------------------------------------------------------------
#endif //SEASON5_4
#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 여기까지 시즌 5-4
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if SELECTED_LANGUAGE == LANGUAGE_TAIWANESE
#define PBG_MOD_GM_ABILITY			//영어로 설정할수 있도록 변경(10.08.12)
#endif //SELECTED_LANGUAGE == LANGUAGE_TAIWANESE

/*----------------------------------UI or Font----------------------------------------*/

#ifndef	KJH_PBG_ADD_INGAMESHOP_SYSTEM
#define NEW_USER_INTERFACE	
#endif	//KJH_PBG_ADD_INGAMESHOP_SYSTEM
#ifdef NEW_USER_INTERFACE
	#define NEW_COMMANDS
	#define NEW_STRUCTS
	#define NEW_USER_INTERFACE_UTIL
	#define NEW_USER_INTERFACE_UISYSTEM
	#define NEW_USER_INTERFACE_INPUTSYSTEM
	#define NEW_USER_INTERFACE_SHELL
	#define NEW_USER_INTERFACE_FUNCTIONS
	#define NEW_USER_INTERFACE_PROXY
	#define NEW_USER_INTERFACE_FONT
	#define NEW_USER_INTERFACE_IMAGE
	#define NEW_USER_INTERFACE_CLIENTSYSTEM
	#define NEW_USER_INTERFACE_BUILDER
	#define NEW_USER_INTERFACE_RESOURCEFILE
	#define NEW_USER_INTERFACE_SERVERMESSAGE
	#define NEW_USER_INTERFACE_MAINFRAME_BUTTON  // 신규 UI에 상점 버튼 추가		

	// 폰트 크기
	#define FONTSIZE_KOREAN			12	// 한국어
	#define FONTSIZE_ENGLISH		12	// 글로벌
	#define FONTSIZE_TAIWANESE		12	// 대만어
	#define FONTSIZE_CHINESE		12	// 중국어
	#define FONTSIZE_JAPANESE		15	// 일본어
	#define FONTSIZE_THAILAND		12	// 태국어
	#define FONTSIZE_PHILIPPINES	12	// 필리핀
	#define FONTSIZE_VIETNAMESE		12	// 베트남

	#if SELECTED_LANGUAGE == LANGUAGE_KOREAN
		#define FONTSIZE_LANGUAGE	( FONTSIZE_KOREAN )
	#elif SELECTED_LANGUAGE == LANGUAGE_TAIWANESE
		#define FONTSIZE_LANGUAGE	( FONTSIZE_TAIWANESE )
	#elif SELECTED_LANGUAGE == LANGUAGE_JAPANESE
		#define FONTSIZE_LANGUAGE	( FONTSIZE_JAPANESE )
	#elif SELECTED_LANGUAGE == LANGUAGE_THAILAND
		#define FONTSIZE_LANGUAGE	( FONTSIZE_THAILAND )
	#elif SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES
		#define FONTSIZE_LANGUAGE	( FONTSIZE_PHILIPPINES )
	#elif SELECTED_LANGUAGE == LANGUAGE_CHINESE
		#define FONTSIZE_LANGUAGE	( FONTSIZE_CHINESE )
	#elif SELECTED_LANGUAGE == LANGUAGE_ENGLISH
		#define FONTSIZE_LANGUAGE	( FONTSIZE_ENGLISH )
	#elif SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
		#define FONTSIZE_LANGUAGE	( FONTSIZE_VIETNAMESE )
	#endif //SELECTED_LANGUAGE == LANGUAGE_TAIWANESE

	#ifdef _DEBUG
		#define _SHOPDEBUGMODE
	#endif //_DEBUG

#endif //NEW_USER_INTERFACE
