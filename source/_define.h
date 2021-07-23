#pragma once

#ifdef KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE
#include <boost/smart_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/format.hpp>
#include <boost/any.hpp>
#include <boost/array.hpp>
#include <boost/pool/object_pool.hpp>
#endif // KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE

#define BoostSmart_Ptr( classname )  std::tr1::shared_ptr<classname>
#define BoostWeak_Ptr( classname )   std::tr1::weak_ptr<classname>

#define BoostSmartPointer(classname) \
    class classname; \
    typedef BoostSmart_Ptr(classname) classname##Ptr

#define BoostRest(p) { if(p) { p.reset(); } }

///////////////////////////////////////////////_Scene start_//////////////////////////////////////////////
//#define MAX_SERVER_HI   25
#define MAX_SERVER_HI   30
#define MAX_SERVER_LOW  20

#define MENU_SERVER_LIST           0
#define MENU_WAITING_SERVERLIST	   1
#define MENU_ACCOUNT_UP            2
#define MENU_ACCOUNT_DOWN          3
#define MENU_REGISTRATION_UP       4
#define MENU_REGISTRATION_DOWN     5
#define MENU_OPTION_UP             6
#define MENU_OPTION_DOWN           7
#define MENU_PASSWORD_CONFIRM_UP   8
#define MENU_PASSWORD_CONFIRM_DOWN 9
#define MENU_PASSWORD_CHANGE_UP    10
#define MENU_PASSWORD_CHANGE_DOWN  11
#define MENU_WAITING_LOGIN		   12
#define MENU_CHARACTER             20
#define MENU_NEW_DOWN              21
#define MENU_NEW_UP                22
#define MENU_DELETE_RIGHT          23
#define MENU_DELETE_LEFT           24
#define MENU_OK_RIGHT              25
#define MENU_OK_LEFT               26
#define MENU_NEW_CHARACTER_DOWN    27
#define MENU_NEW_CHARACTER_UP      28
#define MENU_EXIT                  29

#define SERVER_LIST_SCENE   0
#define	NON_SCENE			0
#define WEBZEN_SCENE		1
#define LOG_IN_SCENE		2
#define LOADING_SCENE		3
#define CHARACTER_SCENE		4
#define MAIN_SCENE			5

#ifdef MOVIE_DIRECTSHOW
	#define MOVIE_SCENE			6
#endif // MOVIE_DIRECTSHOW

#define MAX_FENRIR_SKILL_MONSTER_NUM	10
#define FENRIR_TYPE_BLACK	0
#define FENRIR_TYPE_RED		1
#define FENRIR_TYPE_BLUE	2
#define FENRIR_TYPE_GOLD	3

#define MAX_WAITINGTIME	( 15)
#define NUM_LINE_CMB	( 7)
#define NUM_BUTTON_CMB		( 2)
#define NUM_PAR_BUTTON_CMB	( 5)

#define INGAMESHOP_DISPLAY_ITEMLIST_SIZE (9)		// 인게임샵 한페이지에 Display되는 아이템갯수

///////////////////////////////////////////////_Scene end_//////////////////////////////////////////////////////////
//////////////////////////////////////////////_Path start_//////////////////////////////////////////////////////////

#define MAX_PATH_FIND  15
#define NEW_PATH_ALGORITHM

#define TW_SAFEZONE		( 0x0001)	//  안전지대
#define TW_CHARACTER	( 0x0002)	//  캐릭터가 차지하고 있음
#define TW_NOMOVE		( 0x0004)	//  일반속성 ( 이동 불가)
#define TW_NOGROUND		( 0x0008)	//  땅뚫기 속성
#define TW_WATER		( 0x0010)	//  물 속성
#define TW_ACTION       ( 0x0020)   //  특정 행동을 취하는 속성.
#define TW_HEIGHT       ( 0x0040)   //  일정한 높이로 올라감.
#define TW_CAMERA_UP    ( 0x0080)   //  카메라 위로 이동.
#define TW_NOATTACKZONE ( 0x0100)   //  공격 불가능 지역 ( 클라이언트 처리 마법 ).
#define TW_ATT1         ( 0x0200)
#define TW_ATT2         ( 0x0400)
#define TW_ATT3         ( 0x0800)
#define TW_ATT4         ( 0x1000)
#define TW_ATT5         ( 0x2000)
#define TW_ATT6         ( 0x4000)
#define TW_ATT7         ( 0x8000)

#define FACTOR_PATH_DIST		( 5)
#define FACTOR_PATH_DIST_DIAG	( ( int)( ( float)FACTOR_PATH_DIST * 1.414f))

#define MAX_COUNT_PATH		(500)
#define MAX_INT_FORPATH		(65000 * 30000)

#define PATH_INTESTLIST		( 0x01)
#define PATH_TESTED			( 0x02)
#define PATH_END			( 0x04)

///////////////////////////////////////////////_Path end_///////////////////////////////////////////////////////////
///////////////////////////////////////////_Openglutil start_///////////////////////////////////////////////////////

#define CUSTOM_CAMERA_DISTANCE1  200
#define CUSTOM_CAMERA_DISTANCE2  -150
#define RENDER_ITEMVIEW_FAR   2000.f
#define RENDER_ITEMVIEW_NEAR  20.f

///////////////////////////////////////////_Openglutil end_///////////////////////////////////////////////////////
////////////////////////////////////////////_Object start_////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------------------------------
// 몬스터 타입 설정, 타입 추가하시면 꼭 주석 달아주기를 바랍니다.
// -----------------------------------------------------------------------------------------------------

// 몬스터 타입의 최대 갯수
#ifdef LDS_MOD_EXTEND_MAXMODELLIMITS_200TO400
	// 2010/03-2010/04작업 확장맵 몬스터 추가로 200개 초과
	#define MAX_MODEL_MONSTER	400		
#else // LDS_MOD_EXTEND_MAXMODELLIMITS_200TO400
	// 라클리온 맵 들어가면서 최대 갯수 200개로 확장
	#ifdef CSK_RAKLION_BOSS
	#define MAX_MODEL_MONSTER	200
	#else // CSK_RAKLION_BOSS
	#define MAX_MODEL_MONSTER   150
	#endif // CSK_RAKLION_BOSS
#endif // LDS_MOD_EXTEND_MAXMODELLIMITS_200TO400

#define MODEL_MONSTER01		MODEL_SKILL_END+1
//  MODEL_MONSTER01+63 ~ MODEL_MONSTER01+69     마도시 헬라스의 몬스터들.
//  MODEL_MONSTER01+70 ~ MODEL_MONSTER01+73     카오스캐슬 근위병
//  MODEL_MONSTER01+74 ~ MODEL_MONSTER01+80     공성전 몬스터. ( 성문, 수호석상, ... )
//	MODEL_MONSTER01+91 ~ MODEL_MONSTER01+99		크라이울프 몬스터
//	MODEL_MONSTER01+100 ~ MODEL_MONSTER01+105	아이다 몬스터
//	MODEL_MONSTER01+106 ~ MODEL_MONSTER01+121	칸투르 몬스터
//	MODEL_MONSTER01+122							설날 이벤트복주머니 몬스터
//  MODEL_MONSTER01+123 ~ MODEL_MONSTER01+125   이벤트맵 몬스터
//  MODEL_MONSTER01+126							3차 체인지업 보스 몬스터
//  MODEL_MONSTER01+127							추석 이벤트 달토끼 몬스터
//  MODEL_MONSTER01+128 ~ MODEL_MONSTER01+135	엘베란드 몬스터
//  MODEL_MONSTER01+136 ~ MODEL_MONSTER01+144	평온의늪 몬스터
//	MODEL_MONSTER01+145 ~ MODEL_MONSTER01+149	라클리온 몬스터 5종
//	MODEL_MONSTER01+150							라클리온 보스 몬스터
//  MODEL_MONSTER01+151 ~ MODEL_MONSTER01+153	라클리온 알 3종류
//  MODEL_MONSTER01+154                         일본 여름 이벤트 몬스터 1종
//  MODEL_MONSTER01+155 ~ MODEL_MONSTER01+156   일본 크리스마스 이벤트 몬스터 2종
//  MODEL_MONSTER01+157 ~ MODEL_MONSTER01+163	PK 필드 몬스터 7종
#define MODEL_MONSTER_END	MODEL_MONSTER01+MAX_MODEL_MONSTER

#ifdef PBG_ADD_NEWCHAR_MONK
// 모델 기본 스킨 데이타 갯수 확장 20 넘진 않으나 소스간결화를 위함
#define MODEL_BODY_NUM               24
#else //PBG_ADD_NEWCHAR_MONK
#define MODEL_BODY_NUM               20
#endif //PBG_ADD_NEWCHAR_MONK

#define MODEL_ITEM_COMMON_NUM		2	// 같은 아이템이지만 모델이 다른 아이템 세트 개수. 현재 소환술사와 요정이 같이 쓰는 넝쿨, 실크 시리즈 아이템.
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	#define MODEL_ITEM_COMMONCNT_RAGEFIGHTER	4	//같은 아이템 모델 교환위함
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
#define DARKSIDE_TARGET_MAX		5
#endif //PBG_ADD_NEWCHAR_MONK_SKILL

//struct
#define MAX_BOIDS         40
#define MAX_FISHS         10
#define MAX_BUTTERFLES    10
//bodypart
#define BODYPART_HEAD   0
#define BODYPART_HELM   1
#define BODYPART_ARMOR  2
#define BODYPART_PANTS  3
#define BODYPART_GLOVES 4
#define BODYPART_BOOTS  5
#define MAX_BODYPART    6

/////////////////////////////State_Buff에 추가/////////////////////////////////////////// 

#define TELEPORT_NONE   0
#define TELEPORT_BEGIN  1
#define TELEPORT        2
#define TELEPORT_END    3
#define KIND_PLAYER  1
#define KIND_MONSTER 2
#define KIND_NPC     4
#define KIND_TRAP    8
#define KIND_OPERATE 16     //  
#define KIND_EDIT    32
#define KIND_PET     64
#define KIND_TMP     128
#define MAX_OPERATES 200
#define MAX_ITEMS 1000

////////////////////////////////////////////_Object end_//////////////////////////////////////////////////////////
////////////////////////////////////////_inventory start_/////////////////////////////////////////////////////

#define MAX_GUILD_LINE   14
#define MAX_GUILDS       80

#define MAX_MARKS   2000
#define MARK_EDIT   1999

#define MAX_PARTYS   5

#define TRADE_BUY    0
#define TRADE_SELL   1
#define TRADE_REPAIR 2

#define INVENTORY_SCALE        20
#define COLUMN_INVENTORY       8
#define ROW_INVENTORY          8
#define COLUMN_SHOP_INVENTORY  8
#define ROW_SHOP_INVENTORY     15
#define COLUMN_TRADE_INVENTORY 8
#define ROW_TRADE_INVENTORY    4
#define COLUMN_MIX_INVENTORY   8
#define ROW_MIX_INVENTORY      4
#define MAX_INVENTORY          COLUMN_INVENTORY*ROW_INVENTORY
#define MAX_SHOP_INVENTORY     COLUMN_SHOP_INVENTORY*ROW_SHOP_INVENTORY
#define MAX_TRADE_INVENTORY    COLUMN_TRADE_INVENTORY*ROW_TRADE_INVENTORY
#define MAX_MIX_INVENTORY      COLUMN_MIX_INVENTORY*ROW_MIX_INVENTORY

//equipment index
#define REQUEST_EQUIPMENT_INVENTORY 0		// 인벤토리
#define REQUEST_EQUIPMENT_TRADE 1			// 거래
#define REQUEST_EQUIPMENT_STORAGE 2			// 창고
#define REQUEST_EQUIPMENT_CHAOS_MIX 3		// 카오스 조합
#define REQUEST_EQUIPMENT_MYSHOP 4			// 개인상점
#define REQUEST_EQUIPMENT_TRAINER_MIX 5		// 조련사 조합
#define REQUEST_EQUIPMENT_ELPIS_MIX 6		// 엘피스 조합
#define REQUEST_EQUIPMENT_OSBOURNE_MIX 7	// 오스본 조합
#define REQUEST_EQUIPMENT_JERRIDON_MIX 8	// 제리든 조합
#ifdef ADD_GLOBAL_MIX_MAR08
#define REQUEST_EQUIPMENT_CHAOS_CARD_MIX 9			// 카오스 카드 조합
#define REQUEST_EQUIPMENT_CHERRYBLOSSOM_MIX 10		// 벚꽃 조합
#endif //ADD_GLOBAL_MIX_MAR08
#ifdef ADD_SOCKET_MIX
#define REQUEST_EQUIPMENT_EXTRACT_SEED_MIX 11	// 시드 추출 조합
#define REQUEST_EQUIPMENT_SEED_SPHERE_MIX 12	// 시드 스피어 합성 조합
#define REQUEST_EQUIPMENT_ATTACH_SOCKET_MIX 13	// 소켓아이템 장착 조합
#define REQUEST_EQUIPMENT_DETACH_SOCKET_MIX 14	// 소켓아이템 해제 조합
#endif	// ADD_SOCKET_MIX

#define EQUIPMENT_WEAPON_RIGHT 0
#define EQUIPMENT_WEAPON_LEFT  1
#define EQUIPMENT_HELM         2
#define EQUIPMENT_ARMOR        3
#define EQUIPMENT_PANTS        4
#define EQUIPMENT_GLOVES       5
#define EQUIPMENT_BOOTS        6
#define EQUIPMENT_WING         7
#define EQUIPMENT_HELPER       8
#define EQUIPMENT_AMULET       9
#define EQUIPMENT_RING_RIGHT   10
#define EQUIPMENT_RING_LEFT    11
#define MAX_EQUIPMENT          12

#define MAX_EQUIPMENT_INDEX			MAX_EQUIPMENT
#define MAX_MY_INVENTORY_INDEX		MAX_EQUIPMENT_INDEX + MAX_INVENTORY

#ifdef LDS_FIX_WRONG_QUANTITY_SETITEMOPTION_WHEN_RENDER_SETITEMTOOTIP_IN_INVENTORY
	#define MAX_SETITEM_OPTIONS		12		// 아이템 추가옵션 갯수 제한 수치
#endif // LDS_FIX_WRONG_QUANTITY_SETITEMOPTION_WHEN_RENDER_SETITEMTOOTIP_IN_INVENTORY

#define MAX_MY_SHOP_INVENTORY_INDEX MAX_MY_INVENTORY_INDEX + MAX_SHOP_INVENTORY

#define MAX_SHOPTITLE 36
#define COL_PERSONALSHOP_INVEN 8
#define ROW_PERSONALSHOP_INVEN 4
#define MAX_PERSONALSHOP_INVEN COL_PERSONALSHOP_INVEN*ROW_PERSONALSHOP_INVEN

#define MAX_KEYPADINPUT	( 10)

#define TEXT_COLOR_WHITE    0 
#define TEXT_COLOR_BLUE     1
#define TEXT_COLOR_RED      2
#define TEXT_COLOR_YELLOW   3
#define TEXT_COLOR_GREEN    4
#define TEXT_COLOR_DARKRED  5
#define TEXT_COLOR_PURPLE   6
#define TEXT_COLOR_DARKBLUE 7
#define TEXT_COLOR_DARKYELLOW 8
#define TEXT_COLOR_GREEN_BLUE 9
#define TEXT_COLOR_GRAY       10
#define TEXT_COLOR_REDPURPLE  11
#ifdef SOCKET_SYSTEM
	#define TEXT_COLOR_VIOLET	12
#endif	// SOCKET_SYSTEM
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
	#define TEXT_COLOR_ORANGE	13
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM

#define KOREAN  0
#define ENGLISH 1

#define ONE_HAND 0
#define TWO_HAND 1

////////////////////////////////////////_inventory end_///////////////////////////////////////////////////////

////////////////////////////////////////_LodTerrain start_//////////////////////////////////////////////////////

#define TERRAIN_SCALE     100.f
#define TERRAIN_SIZE      256
#define TERRAIN_SIZE_MASK 255

#define BLOODCASTLE_NUM 8
#define HELLAS_NUM      7
#define CHAOS_NUM       6

#define TERRAIN_MAP_NORMAL 0
#define TERRAIN_MAP_ALPHA  1
#define TERRAIN_MAP_GRASS  2
#define TERRAIN_MAP_TRAP   3

////////////////////////////////////////_LodTerrain end_//////////////////////////////////////////////////////
////////////////////////////////////////_Interface start_//////////////////////////////////////////////////////

#define MAX_WHISPER      120
#define MAX_WHISPER_LINE 6

#define EDIT_NONE    0
#define EDIT_MAPPING 1
#define EDIT_OBJECT  2
#define EDIT_WALL    3
#define EDIT_HEIGHT  4
#define EDIT_LIGHT   5
#define EDIT_SOUND   6
#define EDIT_MONSTER 7

#define IME_CONVERSIONMODE  1
#define IME_SENTENCEMODE    2

#define AUTOATTACK_ON   0x01    //  자동공격 설정.
#define AUTOATTACK_OFF  0x02    //  자동공격 취소.

#define WHISPER_SOUND_ON    0x04//  귓속말 알림음 On.
#define WHISPER_SOUND_OFF   0x08//  귓속말 알림음 Off.
#define SLIDE_HELP_OFF     0x10

#define RT3_SORT_LEFT 1				// 왼쪽 정렬
#define RT3_SORT_LEFT_CLIP 2		// 왼쪽 정렬 & 스크롤(왼쪽 클리핑)
#define RT3_SORT_CENTER 3			// 가운데 정렬
#define RT3_SORT_RIGHT 4			// 오른쪽 정렬

#define RT3_WRITE_RIGHT_TO_LEFT 7	// 좌표 기준으로 왼쪽으로 쓰기
#define RT3_WRITE_CENTER 8			// 좌표 기준으로 중앙에 쓰기 (RenderErrorMessage)

#define CHAOS_MIX_LEVEL 10

#define MOVEMENT_MOVE    0
#define MOVEMENT_GET     1
#define MOVEMENT_TALK    2
#define MOVEMENT_ATTACK  3
#define MOVEMENT_OPERATE 4
#define MOVEMENT_SKILL	 5

#define MAX_WHISPER_ID 5

//////////////////////////////////////////_Interface end_//////////////////////////////////////////////////////
//////////////////////////////////////////_Infomation start_///////////////////////////////////////////////////

#define MAX_ID_SIZE			10

#ifdef LDS_MODIFY_CHAR_LENGTH_USERPASSWORD	// 비밀번호 자릿수 10->12로 변경 사항
	#if SELECTED_LANGUAGE == LANGUAGE_KOREAN
		#define MAX_PASSWORD_SIZE	12
	#else //한국아닌경우 10자리
		#define MAX_PASSWORD_SIZE	10
	#endif //SELECTED_LANGUAGE == LANGUAGE_KOREAN
#endif // LDS_MODIFY_CHAR_LENGTH_USERPASSWORD
#ifdef LDK_MOD_PASSWORD_LENGTH_20 // 글로벌 포털 비밀번호 20 자리로 변경
		#define MAX_PASSWORD_SIZE	20
#endif //LDK_MOD_PASSWORD_LENGTH_20

#ifdef LJH_ADD_ONETIME_PASSWORD 
#define MAX_ONETIME_PASSWORD_SIZE	6
#endif //LJH_ADD_ONETIME_PASSWORD


#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
#define	CHAR_DEL_LIMIT_LV	180		// 캐릭터 삭제 제한 레벨
#elif SELECTED_LANGUAGE == LANGUAGE_ENGLISH
#define CHAR_DEL_LIMIT_LV	300		// 글로벌 캐릭터 삭제 제한 레벨
#else	//SELECTED_LANGUAGE == LANGUAGE_JAPANESE
#define	CHAR_DEL_LIMIT_LV	220		// 캐릭터 삭제 제한 레벨
#endif	//SELECTED_LANGUAGE == LANGUAGE_JAPANESE

#define MAX_TEXTS               3000
#define MAX_GLOBAL_TEXT_STRING	300
#define MAX_FILTERS 1000

#define MAX_NAMEFILTERS 500

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#define MAX_LETTER_TITLE_LENGTH		60
#define MAX_LETTERTEXT_LENGTH		1000
#define MAX_CHATROOM_TEXT_LENGTH	150
#define MAX_LANGUAGE_NAME_LENGTH	4
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE


#define MAX_GATES           512
#define MAX_DIALOG			200

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#define MAX_GIFT_MESSAGE_SIZE	200
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

// #if SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
// 	#define MAX_LENGTH_DIALOG		( 400)
// #else 
	#define MAX_LENGTH_DIALOG		( 300)
// #endif //SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE

#define MAX_ANSWER_FOR_DIALOG	( 10)

// #if SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
// 	#define MAX_LENGTH_ANSWER		( 128)
// #else 
	#define MAX_LENGTH_ANSWER		( 64)
// #endif //SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE

#define MAX_ITEM_TYPE      16

//2005년 7.12일 아이템 인덱스 확장
#define MAX_ITEM_INDEX     512

#define MAX_ITEM           (MAX_ITEM_TYPE*MAX_ITEM_INDEX)

#ifdef PJH_ADD_MINIMAP
#define MAX_MINI_MAP_DATA	100
#endif //PJH_ADD_MINIMAP
#ifdef PJH_ADD_MASTERSKILL
#define	MAX_MASTER		   24
#else
#define	MAX_MASTER		   20
#endif
#define MAX_ITEM_SPECIAL   8
#define ITEM_LEVEL_NORMAL  4
#define MAX_QUEST_ITEM     64  //  늘어나면 추가된다.
#define MAX_EVENT_ITEM     35  //  늘어나면 추가된다.

#ifdef SOCKET_SYSTEM
#define MAX_SOCKETS			5		// 아이템당 최대 소켓수
#define SOCKET_EMPTY		0xFF	// 빈 소켓
#endif	// SOCKET_SYSTEM

//item index
#define ITEM_SWORD		 (0)   //pal,etc
#define ITEM_AXE		 (1*MAX_ITEM_INDEX)  //ran,dru
#define ITEM_MACE		 (2*MAX_ITEM_INDEX)  //ran,nec
#define ITEM_SPEAR		 (3*MAX_ITEM_INDEX)  //ama
#define ITEM_BOW		 (4*MAX_ITEM_INDEX)  //ama
#define ITEM_STAFF		 (5*MAX_ITEM_INDEX)  //soc
#define ITEM_SHIELD		 (6*MAX_ITEM_INDEX) 
#define ITEM_HELM		 (7*MAX_ITEM_INDEX)
#define ITEM_ARMOR		 (8*MAX_ITEM_INDEX)
#define ITEM_PANTS		 (9*MAX_ITEM_INDEX)
#define ITEM_GLOVES		 (10*MAX_ITEM_INDEX)
#define ITEM_BOOTS		 (11*MAX_ITEM_INDEX)
#define ITEM_WING		 (12*MAX_ITEM_INDEX)
#define ITEM_HELPER		 (13*MAX_ITEM_INDEX)
#define ITEM_POTION  	 (14*MAX_ITEM_INDEX)
#define ITEM_ETC 		 (15*MAX_ITEM_INDEX)

#define MAX_RESISTANCE		7

#ifdef PBG_ADD_NEWCHAR_MONK
#define MAX_CLASS			7
#else //PBG_ADD_NEWCHAR_MONK
#define MAX_CLASS			6
#endif //PBG_ADD_NEWCHAR_MONK

#ifdef ASG_FIX_MONSTER_MAX_COUNT_1024
#define MAX_MONSTER			1024
#else	// ASG_FIX_MONSTER_MAX_COUNT_1024
#define MAX_MONSTER			512
#endif	// ASG_FIX_MONSTER_MAX_COUNT_1024

#define MAX_SKILLS			600

#define MAX_DUTY_CLASS		3

#define SKILL_SWORD            0 
#define SKILL_MACE             2 
#define SKILL_BOW              3 
#define SKILL_SPEAR            4 
#define SKILL_STAFF            5 

#define SKILL_CRY              8 
#define SKILL_INVINCIBILITY    9 
#define SKILL_DUAL_WEAPON      10 
#define SKILL_BASH             11
#define SKILL_HEALING          12
#define SKILL_SACRIFICE        13

#define SKILL_STEALTH          16 
#define SKILL_TRAP             17 
#define SKILL_JAP              19 
#define SKILL_TELEPORT         20 
#define SKILL_DECOY            21 

#define SKILL_MULTIPLE_SHOT    24 
#define SKILL_CHARM            25
#define SKILL_CONFUSE          26
#define SKILL_CHANGE_SHAPE     27
#define SKILL_INFRAVISION      28
#define SKILL_PROTECTION       29

#define SKILL_FIRE             32 
#define SKILL_COLD             33 
#define SKILL_MANA             35 
#define SKILL_ANTIDOTE         36 
#define SKILL_REFLECTION       37 

///////////////////////////////////////////_Infomation end_//////////////////////////////////////////////////////
////////////////////////////////////////////_effect start_//////////////////////////////////////////////////////

#ifdef YDG_MOD_SEPARATE_EFFECT_SKILLS
#define MAX_EFFECTS       200
#define MAX_SKILL_EFFECTS 100
#else	// YDG_MOD_SEPARATE_EFFECT_SKILLS
#ifdef YDG_FIX_CLIENT_SKILL_EFFECT_SIZE
#define MAX_EFFECTS       1000
#else	// YDG_FIX_CLIENT_SKILL_EFFECT_SIZE
#define MAX_EFFECTS       200
#endif	// YDG_FIX_CLIENT_SKILL_EFFECT_SIZE
#endif	// YDG_MOD_SEPARATE_EFFECT_SKILLS
#define MAX_SPRITES       1000
#define MAX_PARTICLES     3000
#define MAX_LEAVES        200

#ifdef DEVIAS_XMAS_EVENT
	#define MAX_LEAVES_DOUBLE 400
#endif // DEVIAS_XMAS_EVENT 

#define MAX_POINTS        100
#define MAX_POINTERS      100

//////////////////////////////////////////_effect end_//////////////////////////////////////////////////////
//////////////////////////////////////////_character start_//////////////////////////////////////////////////

#define MAX_CHARACTERS_CLIENT  400
#define MAX_CHARACTERS_SERVER  10

#define MAX_PATH_FIND 15

#define ATTACK_FAIL    0
#define ATTACK_SUCCESS 1
#define ATTACK_DIE     2

#define ABILITY_FAST_ATTACK_SPEED	( 0x01)
#define ABILITY_PLUS_DAMAGE			( 0x02)
#define ABILITY_FAST_ATTACK_RING	( 0x04)
#define ABILITY_FAST_ATTACK_SPEED2	( 0x08)

#define CTLCODE_01BLOCKCHAR			( 0x01)
#define CTLCODE_02BLOCKITEM			( 0x02)
#define CTLCODE_04FORTV				( 0x04)
#define CTLCODE_08OPERATOR			( 0x08)
#define CTLCODE_10ACCOUNT_BLOCKITEM	( 0x10)
#define CTLCODE_20OPERATOR			( 0x20)
#define CTLCODE_80MANAGER_MOVE_CHAR	( 0x80)

// PvP레벨
#define PVP_HERO2		1
#define PVP_HERO1		2
#define PVP_NEUTRAL		3
#define PVP_CAUTION		4
#define PVP_MURDERER1	5
#define PVP_MURDERER2	6

#define REGIMENT_NONE       0
#define REGIMENT_DEFENSE    1
#define REGIMENT_ATTACK     2

#define MAX_JOINTS 500
#define MAX_TAILS  50

#define RENDER_FACE_ONE 0x01
#define RENDER_FACE_TWO 0x02


//////////////////////////////////////////_character end_/////////////////////////////////////////////////

/////////////////////////////////////////////_ai start_/////////////////////////////////////////////////

#define MONSTER01_STOP1    0
#define MONSTER01_STOP2    1
#define MONSTER01_WALK     2
#define MONSTER01_ATTACK1  3
#define MONSTER01_ATTACK2  4
#define MONSTER01_SHOCK    5
#define MONSTER01_DIE      6
#define MONSTER01_APEAR    7
#define MONSTER01_ATTACK3  8
#define MONSTER01_ATTACK4  9
#define MONSTER01_RUN      10
#define MONSTER01_ATTACK5  11

#define FENRIR_STAND		0	// 정지
#define FENRIR_WALK			1	// 걷기
#define FENRIR_RUN			2	// 달리기
#define FENRIR_ATTACK		3	// 일반공격
#define FENRIR_ATTACK_SKILL	4	// 스킬공격
#define FENRIR_DAMAGE		5	// 데미지
#define FENRIR_RUN_DELAY	20	// 펜릴 걷는 딜레이 값

/////////////////////////////////////////////_ai end_///////////////////////////////////////////////////
////////////////////////////////////////_publicfile start_//////////////////////////////////////////////

#define MAX_MAGIC					64	// 사용자당 최대 마법 개수

//----------------------------------------------------------------------------
// 저항력 번호
#define RESISTANCE_COLD				0
#define RESISTANCE_POISON			1
#define RESISTANCE_THUNDER			2
#define RESISTANCE_FIRE				3
#define RESISTANCE_EARTH            4
#define RESISTANCE_WIND             5
#define RESISTANCE_WATER            6

//#define MAX_SKILL					256

////////////////////////////////////////_publicfile end_//////////////////////////////////////////////////////
////////////////////////////////////_npcbreederfile start_////////////////////////////////////////////////////

#define WM_GRAPHNOTIFY  WM_USER+13
#define MOVIE_FILE_TXT	"Data\\Movie\\Movie.txt"
#define MOVIE_FILE_WMV	"Data\\Movie\\MU.wmv"

/////////////////////////////////////_npcbreederfile end_//////////////////////////////////////////////////////
//////////////////////////////////////_matchevent start_//////////////////////////////////////////////////////

#define MAX_BLOOD_CASTLE_MEN    10
#define MAX_CHAOS_CASTLE_MEN    100

////////////////////////////////////////_matchevent end_//////////////////////////////////////////////////////
////////////////////////////////////_gmbattlecastle start_////////////////////////////////////////////////////


#define BATTLE_CASTLE_WALL1 61
#define BATTLE_CASTLE_WALL2 62
#define BATTLE_CASTLE_WALL3 63
#define BATTLE_CASTLE_WALL4 64


//////////////////////////////////////_gmbattlecastle end_////////////////////////////////////////////////////
////////////////////////////////////_gm_kanturu_2nd start_////////////////////////////////////////////////////

#define DREADFEAR_ATTACK2_MOTION_COUNT 2

#define KANTURU2ND_STATETEXT_MAX	3
#define KANTURU2ND_REFRESH_GAPTIME	2000
#define KANTURU2ND_REFRESHBUTTON_GAPTIME 1000

#define KANTURU2ND_NPC_ANI_STOP	0
#define KANTURU2ND_NPC_ANI_ROT	1


//////////////////////////////////////_gm_kanturu_2nd end_////////////////////////////////////////////////////
/////////////////////////////////////_cswaterterrain start_////////////////////////////////////////////////////

#define WATER_TERRAIN_SIZE      256
#define MAX_WATER_SIZE          4
#define MAX_WATER_GRID          42
#define VIEW_WATER_GRID         100
#define WAVE_SCALE              50.f

//////////////////////////////////////_cswaterterrain end_////////////////////////////////////////////////////
/////////////////////////////////////////_csquest start_////////////////////////////////////////////////////

#define MAX_QUESTS          200
#define MAX_QUEST_CONDITION 16
#define MAX_QUEST_REQUEST   16
#define MAX_QUEST_TEXT      100

#define NUM_LINE_DA		        ( 1)
#define MAX_LENGTH_CMB	        ( 38)
#define NUM_LINE_CMB	        ( 7)

#define MAX_DEVIL_SQUARE_ENTER  6
#define BLOODCASTLE_QUEST_NUM   3


//////////////////////////////////////////_csquest end_////////////////////////////////////////////////////
/////////////////////////////////////_csmapinterface start_////////////////////////////////////////////////////

#define MAX_GUILD_COMMAND   7

/////////////////////////////////////_csmapinterface end_////////////////////////////////////////////////////
/////////////////////////////////////_cscommand stracter_////////////////////////////////////////////////////

#define MAX_COMMAND         13
#define MAX_DISTANCE_TILE   2

/////////////////////////////////////_cscommand end_////////////////////////////////////////////////////
////////////////////////////////_cmvp1stdirection start_////////////////////////////////////////////////////

#define MONSTERNUM 50
#define NUMOFMON 10000

/////////////////////////////////////_cmvp1stdirection end_////////////////////////////////////////////////////
/////////////////////////////////////_cdirection start_////////////////////////////////////////////////////

#define NUMOFMON 10000

/////////////////////////////////////_cdirection end_////////////////////////////////////////////////////
/////////////////////////////////////_ccomgem start_////////////////////////////////////////////////////

#define INDEX_COMPILED_CELE		ITEM_WING+30
#define INDEX_COMPILED_SOUL		ITEM_WING+31
#define MODEL_COMPILED_CELE		MODEL_WING+30
#define MODEL_COMPILED_SOUL		MODEL_WING+31
#define MAX_LINE_UNMIXLIST		8
//NPC 인덱스
#define INDEX_NPC_LAHAP			9
#define MAX_BUFF_SLOT_INDEX     16

/////////////////////////////////////_ccomgem end_////////////////////////////////////////////////////

#ifdef NEW_COMMANDS

//opengl
#define OPENGL_COLOR4UB(color)\
{\
	::glColor4ub( static_cast<GLubyte>(color.Red()), static_cast<GLubyte>(color.Green()),\
	static_cast<GLubyte>(color.Blue()), static_cast<GLubyte>(color.Alpha()) );\
}

#define OPENGL_TEXCOORD2F(uv)\
{\
	::glTexCoord2f( uv.x, uv.y );\
}

#define OPENGL_VERTEX2F(pos)\
{\
	::glVertex2f(pos.x, pos.y );\
}

#define OPENGL_BINDTEXTURE(otype, bitmap)\
{\
	::glBindTexture(otype, bitmap);\
}

#define OPENGL_BEGIN(otype)\
{\
	::glBegin(otype);\
}

#define OPENGL_END()\
{\
	::glEnd();\
}

#define OPENGL_ENABLE(otype)\
{\
	::glEnable(otype);\
}

#define OPENGL_DISABLE(otype)\
{\
	::glDisable(otype);\
}

#define DRAWLINE( start, end, color )\
{\
	OPENGL_COLOR4UB(color);\
	OPENGL_BEGIN(GL_LINES);\
	OPENGL_VERTEX2F( CoordF( static_cast<float>(start.x), static_cast<float>(start.y) ) );\
	OPENGL_VERTEX2F( DimF( static_cast<float>(end.x), static_cast<float>(end.y) ) );\
	OPENGL_END();\
}

#define DRAWRECT( rpos, rsize, color )\
{\
	DRAWLINE( Coord( rpos.x, rpos.y ),						Coord( rpos.x, rpos.y - rsize.y ),				color );\
	DRAWLINE( Coord( rpos.x, rpos.y - rsize.y ),			Coord( rpos.x + rsize.x, rpos.y - rsize.y ),	color );\
	DRAWLINE( Coord( rpos.x + rsize.x, rpos.y - rsize.y ),	Coord( rpos.x + rsize.x, rpos.y ),				color );\
	DRAWLINE( Coord( rpos.x + rsize.x, rpos.y ),			Coord( rpos.x, rpos.y ),						color );\
}

//client defines
#define STRUCTSMEMBERNAME			60
#ifdef MODIFY_SOCKET_PROTOCOL
#define ITEM_BUFFER_SIZE            12		// 소켓 확장으로 아이템 구조체 크기 확장
#else	// MODIFY_SOCKET_PROTOCOL
#define ITEM_BUFFER_SIZE            7
#endif	// MODIFY_SOCKET_PROTOCOL
#define MAX_CASHITEM_LIST_CATEGORY	4		// 캐쉬샵 아이템 리스트 최대 카테고리 수
#define MAX_CASHITEM_LIST_PAGE		15		// 캐쉬샵 아이템 리스트 최대 페이지 개수
#define MAX_CASHITEM_LIST_PER_PAGE	9		// 캐쉬샵 아이템 페이지당 아이템 개수
#define MAX_CASHITEM_LIST			( MAX_CASHITEM_LIST_CATEGORY * MAX_CASHITEM_LIST_PAGE * MAX_CASHITEM_LIST_PER_PAGE )

#endif //NEW_COMMANDS

#ifdef KWAK_ADD_DEFINE_UTIL
//===================================================================
// DEBUG
//===================================================================
#ifdef _DEBUG
	#define DEBUG_CLIENTBLOCK	new//new(_CLIENT_BLOCK, __FILE__, __LINE__)
#else
	#define DEBUG_CLIENTBLOCK
#endif

#ifndef ARRAY_COUNT
	#define ARRAY_COUNT( array ) 	( sizeof(array) / sizeof((array)[0]) )
#endif

#ifndef HAVE_BIT
	#define HAVE_BIT(X, Y)			( (X & Y) ? TRUE : FALSE )
#endif

#ifndef ADD_BIT
	#define ADD_BIT(X, Y)			( X |= Y )
#endif

#ifndef DELETE_BIT
	#define DELETE_BIT(X, Y)		( X &= (~Y) )
#endif

#ifndef SET_SIZE_EMPTY
	#define SET_SIZE_EMPTY(X)		{ X.cx = 0;	X.cy = 0;	}
#endif
#endif // KWAK_ADD_DEFINE_UTIL

#ifdef NEW_USER_INTERFACE_UISYSTEM
	#define ITEMINFOCOUNT 30
#endif //NEW_USER_INTERFACE_UISYSTEM

#define MAX_TEXT_LENGTH		255

