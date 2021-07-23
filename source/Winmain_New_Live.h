#ifndef __WINMAIN_NEW_LIVE_H__
#define __WINMAIN_NEW_LIVE_H__

#pragma once


// - 담당자 (2008.06.23)
// 국내 라이브 관리 : 김재희
// 해외 라이브 관리 : 박상완

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	국내 버그 수정
//
//	## 정규 패치에 패치될 내용 (국내) - 라이브그룹
//	^^ <- 라이브그룹 Find 표시 (기능을 막거나 풀때 사용)
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if SELECTED_LANGUAGE == LANGUAGE_KOREAN

// 패치단위별 디파인
#define BUG_PATCH_LIVEGROUP_4TH			// (2008.08.19) - [2008.09.25(테섭), 2008.09.29(본섭)]
#define BUG_PATCH_LIVEGROUP_3RD			// (2008.07.15) - [2008.09.11(테섭), 2008.09.23(본섭)]
#define BUG_PATCH_LIVEGROUP_2ND			// (2008.06.24) - [2008.07.17(체험섭), 2008.08.12(테섭), 2008.08.26(본섭)]
#define BUG_PATCH_LIVEGROUP_1ST			// (2008.06.10) - [2008.06.19(테섭), 2008.06.24(본섭)]

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	해외 버그 수정
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#else // SELECTED_LANGUAGE == LANGUAGE_KOREAN

	// - 박상완
	#define PSW_BUGFIX_IME						// 해외 관련 IME 수정 ( 2008. 04. 11 )

	// ----------------------------------   PSW_BUGFIX_REQUIREEQUIPITEM  ----------------------------------   
	// 현재 클라이언트, 서버 계산 문제로 회의후 다시 결정
	#define PSW_BUGFIX_REQUIREEQUIPITEM			// 장비 요구조건이 틀리면 장비 셋팅을 하지 않는다. ( 2008. 4. 16 )
	// PSW_BUGFIX_LEFTWEAPON_MOVE 이걸 수정 하기 위해서 공격 스피드 검사는 필수로 해야 할듯.
	// 공격 스피드 검사 루틴
	#ifdef PSW_BUGFIX_REQUIREEQUIPITEM
	#define PSW_BUGFIX_LEFTWEAPON_MOVE							// 오른손 무기가 없을때 왼손무기가 오른손무기로 교체 될때 장착 가능 여부 검사( 2008.04.29 )
	#define PSW_BUGFIX_REQUIREEQUIPITEM_ATTACKSPEED				// 공격 스피드 관련
	#define PSW_BUGFIX_REQUIREEQUIPITEM_SPECIALOPTION			// 아이템 옵션 관련( 일반 옵션, 엑셀 옵션 )
		//#define PSW_BUGFIX_REQUIREEQUIPITEM_DAMAGE					// 데미지 관련
		//#define PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDAMAGE				// 마법 데미지 관련
		//#define PSW_BUGFIX_REQUIREEQUIPITEM_CURSEDAMAGE				// 저주 데미지 관련
		//#define PSW_BUGFIX_REQUIREEQUIPITEM_SUCCESSFULBLOCKING      // 방어 성공율 관련
		//#define PSW_BUGFIX_REQUIREEQUIPITEM_DEFENSE					// 방어도 관련
		//#define PSW_BUGFIX_REQUIREEQUIPITEM_MAGICDEFENSE			// 마법 방어도 관련
		//#define PSW_BUGFIX_REQUIREEQUIPITEM_WALKSPEED				// 이동 속도 관련
		//#define PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM                 // 셋트 아이템
	#endif //PSW_BUGFIX_REQUIREEQUIPITEM
	// ----------------------------------   PSW_BUGFIX_REQUIREEQUIPITEM  ----------------------------------   
	
	//#define PSW_BUGFIX_FRIEND_LIST_CLEAR		 //  [WOPS_K22448'] 친구 리스트 클리어 수정 ( 2008.04. 23 ), 1차 라이브버그 추가후 주석처리함
	#define PSW_BUGFIX_CLASS_KNIGHT_REQUIRESKILL // 흑기사 스킬 에너지 요구 조건 변경 ( 2008.04. 23 )
	#define PSW_BUGFIX_ICARUS_MOVE_UNIRIA        // 이카루스 맵에 유니리아 이동 금지( 2008.04. 25 )
		
	#define PSW_BUGFIX_GATEMANWINDOW_TEXT        // CNewUIGatemanWindow에 텍스트 겹침 수정( 2008.04.29 )
	#define PSW_FIX_PARTYLISTSELECTCHARACTER	 // 파티 리스창에 캐릭터가 선택 됐을때 캐릭터 이름 보이기 ( 2008.05.06 )
	#define PSW_FIX_INPUTTEXTMACRO				 // 인풋창에서 alt + 1....0 등록이 안 되는 버그 ( 2008.05.06 )
	#define PSW_FIX_EQUIREEQUIPITEMBUY           // 장착한 아이템은 상점에 팔 수 없다..( 2008.06.25 )
		
	// - 이동석
	#define LDS_FIX_ELFHELM_CILPIDREI_RESIZE	// 엘프 실피드레이 헬멧의 크기가 너무 작게 출력되는 문제. (2008.04.25)
	#define LDS_FIX_RESET_CHARACTERDELETED// 마검사와 다크로드가 케릭터 선택창에서 같은 위치상의 삭제 생성시 망토가 바뀌어 나오는 버그로 삭제시 실제 삭제가 이루어 지지않아 생기는 버그 수정 (2008.04.30)
	#define LDS_FIX_APPLYSKILLTYPE_AND_CURSEDTEMPLEWRONGPARTYMEMBER	// 환영의 사원 파티원에게 공격이 되는 오류로 CursedTemple::IsPartyMember 상에 검색 조건을 ID 에서 Name으로 오류 수정 (2008.05.08) + Attack함수에서 Attack 판단 시에 버프도 공격으로 처리 되는 오류수정. (2008.05.14)
		
	// - 이동근
	#define LDK_FIX_MENUPOPUP_POSITION_BUG		//메뉴버튼 팝업 도움말 텍스트 짤림수정(2008.4.28)
	#define LDK_FIX_CRYWOLF_RESULT_FONT_STATE	//크라이 울프 결과창 폰트위치 이상 수정(2008.4.30)
	#define LDK_FIX_EXCEPTION_SETWHISPERID_NOTMODELPLAYER_INCHAOSCASTLE  //채팅창 열고 마우스우클릭시 귀속말id 입력 예외처리(2008.5.7)
	#define LDK_FIX_COMMANDWINDOW_EXIT_BTNPROCESS	// [WOPS_K27165'] 커맨드창 닫기 버튼으로 닫을경우 파티리스트윈도우 위치 이상 (2008.5.15)
	#define LDK_FIX_EXCEPTION_SETSTATEGAMEOVER	//정방 걸린상태에서 재접,캐릭선택,서버선택시도시 인벤토이 안열림 수정(2008.5.29)
	#define	LDK_FIX_AUTHORITYCODE_LENGTH		//대만 창고 비밀번호 인증시 신분증 번호 입력칸 수정 10 -> 9(2008.5.30)
	#define LDK_FIX_RECALL_CREATEEQUIPPINGEFFECT //장비창 펫 들었다 놨을때 CreateEquippingEffect 2번 호출(2008.6.27)
	#define LDK_FIX_FULLSERVER_CONNECT_ERROR	// 풀서버 맵이동시 클라이언트 락걸림. (2008.08.06)
	#define LDK_FIX_CHAR_NUMBER_OVER			//해외 글자수가 너무 길어서 두줄로 나눔(2008.08.11)
	#define LDK_FIX_OPEN_GOLDBOWMAN_UI_CLICK_SHOP_BTN //황금궁수ui 중에 샵버튼 누르면 종료시까지 샵안열림(2008.08.11)
	#define LDK_FIX_RECEIVEBUYSELL_RESULT_254		//구매 횟수 이상 버그 .. receivebuy result값이 254면 hideAll(2008.08.11)
	#define LDK_FIX_CAOS_THUNDER_STAFF_ROTATION //inventory 카오스 번개 지팡이 회전값 이상(2008.08.12)

	#define LDK_FIX_PC4_SEALITEM_TOOLTIP_BUG	//치유, 신성의 인장 마스터레벨 경고문구 누락 처리(2008.10.28)

	// 아이스에로우 (요정) 일때 스텟을 검사하여 요구스텟(646_하드코딩..)이 충분치 아니하면 스킬아이콘 빨갛게 처리
	#define LDK_FIX_CHECK_STAT_USE_SKILL_PIERCING									// (2008.08.20) [해외]
	#ifdef LDK_FIX_CHECK_STAT_USE_SKILL_PIERCING
		// 박상완
		#define PSW_FRUIT_ITEM_CHECK_SKILL											// [해외]
	#endif //LDK_FIX_CHECK_STAT_USE_SKILL_PIERCING

	//비약사용시 세트 옵션 이상
	//비약 적용시 add값 갱신.1 : CHARACTER_MACHINE::CalculateAll() 함수 처음에 따로 추가... 문제시 순서 고려하자..
	// - g_csItemOption.CheckItemSetOptions ()보다는 먼저 해야됨.
	//비약의 능력치를 더하자.2 : CSItemOption::CheckItemSetOptions()
	#define LDK_FIX_USE_SECRET_ITEM_TO_SETITEM_OPTION_BUG //(2008.08.25) [해외]
		
	// - 양대근
	#define FIX_TEXTURE_MODEL_INDEX_CONFLICT	// 텍스쳐와 모델 인덱스 충돌 문제 수정 (이펙트 switch문에서 문제 발생) (2008.04.16)
	// - 최석근
	#define CSK_BUGFIX_NEWFRIEND_ADD			// 친구추가하면 메세지창이 안뜨는 버그 수정(2008.04.23)
	#define CSK_FIX_CHAOSFRIENDWINDOW			// 카오스캐슬 대기시간동안 친구창 안열리게 수정(2008.05.08)
		
	// - 김재희
	#define BUGFIX_UI_ARROW_NUM					// 화살을 장착하지 않았는데도 화살 표시가 없어지지 않는문제 (2008.04.28)
	#define BUGFIX_UI_COMMAND_CHAR_ON_MOUSE		// COMMAND창에서 명령을 사용할수 없는 거리의 캐릭터에게 마우스온 할경우 이름이 빨갛게 표시 안되는 문제 (2008.04.29)

	//## 국내 라이브 버그 수정
	#define BUG_PATCH_LIVEGROUP_4TH			// (2008.08.19) - [2008.09.25(테섭), 2008.09.29(본섭)]
	#define BUG_PATCH_LIVEGROUP_3RD			// (2008.07.15) - [2008.09.11(테섭), 2008.09.23(본섭)]
	#define BUG_PATCH_LIVEGROUP_2ND			// (2008.06.24) - [2008.07.17(체험섭), 2008.08.12(테섭), 2008.08.26(본섭)]
	#define BUG_PATCH_LIVEGROUP_1ST			// (2008.06.10) - [2008.06.19(테섭), 2008.06.24(본섭)]

#endif // SELECTED_LANGUAGE == LANGUAGE_KOREAN

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------
//## 4차 국내 라이브 버그 수정 [국내]
// 클라 : 최석근
// 서버 : 오원석
// 기간 : 2008.08.19 ~
// [2008.09.25(테섭), 2008.09.29(본섭)]
//---------------------------------------------------------------------------------------
#ifdef BUG_PATCH_LIVEGROUP_4TH

	// 아이템 정보창에서 방어율 % 빼고 숫자만 출력해달라고 요청
	#define	CSK_FIX_WOPS_K28204_ITEM_EXPLANATION								// (2008.08.19) [국내]

	// 닐의서 아이템 정보창에서 마력이 저주력으로 표시되야 한다.
	#define	CSK_FIX_WOPS_K28219_ITEM_EXPLANATION								// (2008.08.19) [국내]

	// 환영의사원에서 잃어버린지도 안버려지게 수정
	#define CSK_FIX_WOPS_K27964_LOSTMAP_POP										// (2008.08.19) [국내]	

	// 환영의사원에서 방어력 +11 세트 효과인 추가방어력이 빠져서 표기되는 버그
	#define CSK_FIX_WOPS_K28674_ADD_DEFENSE										// (2008.08.19) [국내]								

	// 마스터레벨 창고사용료가 잘못 표시되는 버그
	#define CSK_FIX_WOPS_K30648_STORAGEVALUE									// (2008.08.20) [국내]	

	// 환영사원 성물 등록시, 성물을 들고 있으면 안사라지는 버그
	#define CSK_FIX_WOPS_K30813_HOLLYITEM_SAVE									// (2008.08.21) [국내]

	// 헬버스트 사용중 순간이동스킬을 사용하면 헬버스트가 계속 지속되는 버그
	#define CSK_FIX_WOPS_K29010_HELLBUST										// (2008.08.22) [국내]

	// 파티원이 서로 다른 지역에 있을 때 파티원이 아이템을 습득하면 파티원 모두에게 동일한 메시지가 출력되는 버그
	#define CSK_FIX_WOPS_K28675_PARTYMESSAGE									// (2008.08.26) [국내]

	// 계정 내 캐릭터들 간에 서로 다른 길다 공지가 공유되는 현상
	#define CSK_FIX_WOPS_K30866_GUILDNOTICE										// (2008.08.29) [국내]

#endif // BUG_PATCH_LIVEGROUP_4TH




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------
//## 3차 국내 라이브 버그 수정 [국내/해외]
// 클라 : 김재희
// 서버 : 오원석
// 기간 : 2008.07.14 ~ 2008.08.14
// [2008.09.11(테섭), 2008.09.23(본섭)]
//---------------------------------------------------------------------------------------
#ifdef BUG_PATCH_LIVEGROUP_3RD

	// 소환술사의 마력증가 옵션이 있는 소환술서를 착용해도,
	// 마력증가가 되지않는 현상.
	#define	KJH_FIX_WOPS_K27580_EQUIPED_SUMMONING_ITEM							// (2008.08.14) [국내]

	// 펫정보창에서 다크호스탭의 가용가능한 통솔이 있는현상.
	// 다크스피릿탭에 사용가능한 통솔이 있어야 한다.
	#define KJH_FIX_WOPS_K27950_REQUIRE_CHARISMA_PETINFORMATION					// (2008.08.13) [국내]

	// 아이템툴팁 보다 물약갯수 숫자가 더 위쪽에 보이는 현상.
	// (인벤토리보다 NPC상점을 먼저 렌더하였으나 이것으로는 해결안됨)
	// (카메라 ZOrder를 수정해야함.)
	#define KJH_FIX_WOPS_K27500_POTION_NUM_RENDER								// (2008.08.12) [국내]
													
	// 본계정으로 게임을 하다가, 새로운 계정생성후 창고를 열면,	
	// 본계정 캐릭터의 창고에 있던 금액이 나타나는 현상
	#define KJH_FIX_WOPS_K27340_INIT_STORAGE_GOLD								// (2008.08.12) [국내]	

	// 시간제버프 적용시간이 지나면	적용시간이 4만일 넘게 표시되는 현상.
	#define KJH_FIX_WOPS_K22852_ABNORMAL_BUFFTIME								// (2008.08.11) [국내]			

	// 쌍수무기 착용후 오른속장비 드레그하여 장비창 밖으로 빼면, 
	// 무기하나가 사라지는 현상.
	#define KJH_FIX_WOPS_K27261_DISAPPEAR_EQUIPED_ITEM							// (2008.08.08) [국내]
													
	// 3인이상 파티에서, 한사람이 탈퇴하면, 파티리스트에서
	// 그 자리를 채우는 사람의 HP정보가 탈퇴한 사람의 HP정보로 
	// 나타나는 현상.
	#define KJH_FIX_WOPS_K27187_STEPHP_INFO_IN_PARTY							// (2008.08.07) [국내]

	// 파티시 커맨드창을 열고 'X'버튼을 클릭하여 창을 닫으면 
	// 파티리스트창이 원래 위치로 옮겨지지 않는 현상.
	// 비고 - [해외] LDK_FIX_COMMANDWINDOW_EXIT_BTNPROCESS (같은버그를 다르게 수정)
	#define KJH_FIX_WOPS_K27165_COMMANDWINDOW_EXIT_BTNPROCESS					// (2008.08.06) [국내]

	// 평온의늪에서 요정의 다발스킬 사운드가 출력이 안되는 문제
	#define KJH_FIX_WOPS_K26908_ELF_MULTI_SKILL_SOUND_IN_SWAMPOFQUIET			// (2008.08.05) [국내]
	
	// 칼리마에서 공지창이 뜨지 않는 문제
	#define KJH_FIX_WOPS_K22929_DONOT_RENDER_NOTICE_IN_KALIMA					// (2008.08.04) [국내]
												
	// 크리스마스변신반지 착용후 스텟올리는 셋트아이템을 착탈시,
	// 스텟이 변하지 않는문제
	#define KJH_FIX_WOPS_K27082_REFRASH_STAT_EQUIPED_TRANSFORM_RING				// (2008.08.01) [국내]

	// 같은계정내의 다른캐릭터가 친구창의 창목록으로 다른캐릭터의 
	// 편지가 공유되는 현상.
	#define KJH_FIX_WOPS_K29708_SHARE_LETTER									// (2008.07.31) [국내]

	// 10레벨 방어구를 셋트로 착용하고 죽었다 살아날때, 
	// 셋트 추가방어력이 적용되지 않는 현상
	#define KJH_FIX_WOPS_K28597_EQUIPED_LV10_DEFENSEITEM						// (2008.07.31) [국내]

	// 유니리아를 착용하였는데 이카루스로 이동할수 있는현상
	#define KJH_FIX_WOPS_26619_CANMOVE_IKARUS_BY_UNIRIA							// (2008.09.10) [국내]
	// - 박상완
	#ifdef KJH_FIX_WOPS_26619_CANMOVE_IKARUS_BY_UNIRIA
		#define PSW_BUGFIX_ICARUS_MOVE_UNIRIA									// (2008.07.30) [해외/국내]
	#endif // KJH_FIX_WOPS_26619_CANMOVE_IKARUS_BY_UNIRIA

	// 이카루스에서 날개가 거래되는 현상
	// 날개,망토,팬릴,다크호스 는 적어도 하나는 착용하게 수정
	#define KJH_FIX_WOPS_K26606_TRADE_WING_IN_IKARUS							// (2008.07.30) [국내]

	// 마스터 레벨에서 이벤트맵 입장 아이템 가격이 틀린 현상
	#define KJH_FIX_WOPS_K26602_NPCSHOP_ITEM_PRICE_AS_MASTERLEVEL				// (2008.07.29) [국내/해외]	[2008.09.02 본섭에 미리적용]

	// 크리스마스 변신반지의 소환수에 명령(거래등)이 들어가는 현상
	#define KJH_FIX_WOPS_K22844_CHRISTMAS_TRANSFORM_RING_SUMMON_TO_COMMAND		// (2008.07.24) [국내]

	// 마스터스킬 소울바리어 강화시 툴팁이 제대로 안나오는 현상
	#define KJH_FIX_WOPS_K29544_SOULBARRIER_UPGRADE_TOOLTIP						// (2008.07.22) [국내]

	// 다크호스 생명력 회복시 재접속을 하면 회복가능한 젠이 변하는 현상
	#define KJH_FIX_WOPS_K30120_RECORVERY_PET_LIFE								// (2008.07.22) [국내]
	
	// 공성전 석상이나, 성문의 업그레이드시 UI버튼 오류 현상
	#define KJH_FIX_WOPS_K25856_UPGRADE_WARFARE_DEFENCE_GATE_AND_STONESTATUE	// (2008.07.16)	[국내]

	// 사탄을 착용 후 클릭으로 들고 장비창 닫고 스킬 사냥시 사탄의 내구도는 달지 않고,
	// 데미지가 적용되는 현상.
	#define KJH_FIX_WOPS_K29690_PICKED_ITEM_BACKUP								// (2008.07.15) [국내]


	// [WOPS_K27519] - (재현불가)												- (2008.08.13) [국내]
	// 7악마의광장에서 몬스터의 소리가 나지 않는 현상

	// [WOPS_K28574] - (서버)													- (2008.08.13) [국내]
	// 알바트로스보우의 내구도가 달지않는 현상.

	// [WOPS_K27358] - (서버)													- (2008.08.12) [국내]
	// 로랜시아 좌표'145-173'의 위치에서 캐릭터가 밀리는 현상

	// [WOPS_K27080] - (서버)													- (2008.08.11) [국내]
	// 몬스터킬시 sd증가 스킬을 찍으면, 몬스터 사냥할때마다
	// sd가 누적되는 현상

	// [WOPS_K22921] - (수정됨)													- (2008.08.11) [국내]
	// 카오스캐슬에서 친구창이 열리는 현상

	// [WOPS_K26867] - (재현불가)												- (2008.08.06) [국내]
	// 요정의 인피니티 에로우를 사용하여 다발스킬을 시전하면,
	// 마나가 다 달았을때, 자동으로 마나물약을 먹지 않는 현상.

	// [WOPS_K25013] - (수정됨)													- (2008.08.05) [국내]
	// BUGFIX_UI_AUTOPICKITEM_UI_ON_MOUSE [국내]
	// 파티창에 마우스온 하고있을때 아이템획득이 안되는 현상

	// [WOPS_K22935] - (수정됨)													- (2008.08.05) [국내]
	// F1 정보창에서 이벤트맵입장 마스터레벨등급 정보가 없는문제					

	// [WOPS_K26919] - (서버)													- (2008.07.31) [국내]
	// 정제의탑 나이트메어 전투에서 문스톤팬던트의 내구도가 0이되도 죽지 않는현상

	// [WOPS_K26834] - (서버)													- (2008.07.31) [국내]
	// 쉐도우 펜텀부대의 공/방버프가 환영사원에서 적용되는 현상					

	// [WOPS_K26619] - (버그아님)												- (2008.07.30) [국내]
	// 양손지팡이 데미지가 한손지팡이 데미지만 나오는 현상						

	// [WOPS_K26206] - (서버)													- (2008.07.29) [국내]
	// '닐의서'에 조화의보석으로 '필요민첩감소' 옵션추가시 적용되지 않는 현상

	// [WOPS_K23407] - (재현불가)												- (2008.07.28) [국내]
	// 캐릭터가 사망할때 펜릴착용을 해제하면, 이미지가 남는현상

	// [WOPS_k25975] - (버그아님)												- (2008.07.25) [국내]
	// 요정끼리 방어력향상스킬 사용시 스킬사용안돼는 현상.

	// [WOPS_K22873] - (재현불가)												- (2008.07.23) [국내]
	// 소환술사가 소환스킬아이템 착용하고 스킬요구치에 맞게 스텟을 찍어도, 스킬비활성화되는현상 

	// [WOPS_k22722] - (재현불가)												- (2008.07.23) [국내]
	// 내 편지함에 보낸이 이미지가 잘못 표기되는 현상

	// [WOPS_K29686] - (재현불가)												- (2008.07.18) [국내]
	// 캐릭터 이동중 변신반지 착용하면 위치버그 걸리는 현상

	// [WOPS_K29691] - (서버)													- (2008.07.15) [국내]
	// 환영사원에서 속박스킬시전후 포탈로 밀면 클라가 종료되는 현상
	
#endif BUG_PATCH_LIVEGROUP_3RD


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------
//## 2차 국내 라이브 버그 수정 [국내/해외]
// 클라 : 김재희
// 서버 : 오원석
// 기간 : 2008.06.23 ~ 2008.07.11
// [2008.07.17(체험섭), 2008.08.12(테섭), 2008.08.26(본섭)]
//---------------------------------------------------------------------------------------
#ifdef BUG_PATCH_LIVEGROUP_2ND

	// 환영의 사원 파티원에게 공격이 되는 오류로 CursedTemple::IsPartyMember 상에 검색 조건을 ID 에서 Name으로 오류 수정
	// Attack함수에서 Attack 판단 시에 버프도 공격으로 처리 되는 오류수정
	// - Skill.txt 항목추가 
	// - 이동석
	#define LDS_FIX_APPLYSKILLTYPE_AND_CURSEDTEMPLEWRONGPARTYMEMBER					// (2008.07.09) [해외/국내]

	// 채팅창 보완작업 - 일반채팅OFF(F2키)시 일반채팅 스크롤 올라가지 않게 수정
	#define KJH_FIX_UI_CHAT_MESSAGE													// (2008.07.04) [국내]

	// 게임중 뮤홈페이지로이동(F11키)시 홈페이지 주소 수정
	#define KJH_FIX_MU_HOMEPAGE_ADDRESS												// (2008.07.04) [국내]

	// 텍스쳐와 모델 인덱스 충돌 문제 수정 (클라이언트 내부수정) 
	// - 양대근
	#define FIX_TEXTURE_MODEL_INDEX_CONFLICT	 									// (2008.04.16) [해외/국내]

	// 카오스캐슬에서 친구창 열리는 버그
	// - 최석근
	#define CSK_FIX_CHAOSFRIENDWINDOW												// (2008.07.03) [해외/국내]

	// 스킬사용시에 캐릭터의 스텟을 체크 안하는 버그
	// - 비고 : 모든스킬 사용시에 캐릭터의 스텟 체크를 하는부분이 없는관계로,
	//			일단 버그 리스트에 올라와있는 블러드 어택만 수정해두고,
	//			시즌4 패치가 끝난 후, skill.txt 스크립트에 필드를 추가해서 체크루틴을 추가해야 함
	#define KJH_FIX_WOPS_K20674_CHECK_STAT_USE_SKILL								// (2008.07.01) [국내/해외]
	#ifdef KJH_FIX_WOPS_K20674_CHECK_STAT_USE_SKILL
		// 박상완
		#define PSW_FRUIT_ITEM_CHECK_SKILL											// [해외/국내]
	#endif // KJH_FIX_WOPS_K20674_CHECK_STATE_USE_SKILL	

	// 다크로드 펫의 생명력이 정상적으로 출력되지 않는 문제
	#define KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER							// (2008.06.26)	[국내]

	// 상점이 열리면서 마우스 좌표에 있는 상점 아이템이 바로 구매되는 현상
	#define KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP						// (2008.06.24) [국내]

	// [WOPS_K26969] - (서버)														- (2008.06.30) [국내]
	// 마스터스킬의 회오리베기 강화를 습득하면 회오리베기 공격범위가 줄어드는 문제

	// [WOPS_K20444] - (서버)														- (2008.06.27) [국내]
	// 다크로드가 다크호스를 착용한 상태에서 칸투르보스전에 못들어가는 문제			

	// [WOPS_K20295] - (서버)														- (2008.06.27) [국내]
	// 캐릭터 재접속시 다크스피릿의 스킬이 초기화가 되지 않는 문제

	// [WOPS_K29466] - (서버)														- (2008.06.25) [국내]
	// 마스터레벨인 캐릭터들의 악마의광장 파티사냥시 동일 경험치를 획득하는 문제

	// [WOPS_K28215] - (서버)														- (2008.06.24) [국내]
	// 3차날개 '5%확률로생명완전회복'옵션이 스웰라이프 시전과 중첩이 되지 않는 문제

	// [WOPS_K29346] - (기획:이민정)												- (2008.06.23) [국내]
	// 마스터스킬중 몬스터킬시 생명력회복 4단계투자시 마스터포인트가 2포인트 소모되는 문제
	// - (클라)MasterSkillTree.txt, (서버)Skill.txt 수정

	// [WOPS_K24373] - (기획:이민정)												- (2008.06.23) [국내]
	// 절망의날개로(2차) 콘도르의 깃털을 조합할수 없는 문제
	// - Mix.txt 수정

#endif // BUG_PATCH_LIVEGROUP_2ND



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//## 1차 국내 라이브 버그 수정 [국내/해외]
// 클라 : 김재희 
// 서버 : 오원석
// 기간 : 2008.06.09 ~ 2008.06.14
// [2008.06.19(테섭), 2008.06.24(본섭)]
//---------------------------------------------------------------------------------------
#ifdef BUG_PATCH_LIVEGROUP_1ST

	// 소환술사의 마스터스킬창의 이상Text가 발생하는현상 
	//^^ 아래. 버그아님. 소환술사 마스터스킬이 풀릴때 주석처리			
#ifndef PJH_ADD_MASTERSKILL
	#define KJH_FIX_WOPS_K22193_SUMMONER_MASTERSKILL_UI_ABNORMAL_TEXT			// (2008.06.13)	[국내]	
#endif

	// 한계정의 다른 케릭터들의 메모리스트가 공유되는 현상 
	// 비고 - [해외] PSW_BUGFIX_FRIEND_LIST_CLEAR (같은버그를 다르게 수정)
	#define KJH_FIX_WOPS_K22448_SHARED_CHARACTER_MEMOLIST						// (2008.06.12)	[국내]

	// 카오스캐슬에서 귓말이 되는 버그 수정	
	// [WOPS_K27961] - (이동근)
	#define LDK_FIX_EXCEPTION_SETWHISPERID_NOTMODELPLAYER_INCHAOSCASTLE			// (2008.06.10) [해외/국내]

	// [WOPS_K20023] - (서버)
	// 엘리트해골반지 착용 후 거래했다가 취소시 반지옵션이 적용되는 버그		- (2008.06.13) [국내]

	// [WOPS_K20247] - (수정된버그)
	// 마스터레벨 다크로드가 통솔열매를 못먹는 현상.							- (2008.06.13) [국내]

#endif // BUG_PATCH_LIVEGROUP_1ST

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //__WINMAIN_NEW_LIVE_H__