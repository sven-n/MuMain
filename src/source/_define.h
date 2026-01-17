#pragma once

#include <memory>

#define Smart_Ptr( classname )  std::shared_ptr<classname>
#define Weak_Ptr( classname )   std::weak_ptr<classname>

#define SmartPointer(classname) \
    class classname; \
    typedef Smart_Ptr(classname) classname##Ptr

#define PtrReset(p) { if(p) { p.reset(); } }

//util
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)  { if(p) { delete [] (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define DIRECTINPUT_VERSION	0x0500
#define BYTECAST(T,X) static_cast<T>(X & 0xFF)

///////////////////////////////////////////////_Scene start_//////////////////////////////////////////////
constexpr int MAX_SERVER_PER_GROUP = 20;

enum EGameScene {
    SERVER_LIST_SCENE = 0,
    WEBZEN_SCENE = 1,
    LOG_IN_SCENE = 2,
    LOADING_SCENE = 3,
    CHARACTER_SCENE = 4,
    MAIN_SCENE = 5,
};

constexpr auto MAX_FENRIR_SKILL_MONSTER_NUM = 10;
constexpr auto FENRIR_TYPE_BLACK = 0;
constexpr auto FENRIR_TYPE_RED = 1;
constexpr auto FENRIR_TYPE_BLUE = 2;
constexpr auto FENRIR_TYPE_GOLD = 3;

constexpr auto NUM_LINE_CMB = ( 7);
constexpr auto NUM_BUTTON_CMB = ( 2);
constexpr auto NUM_PAR_BUTTON_CMB = ( 5);

constexpr auto INGAMESHOP_DISPLAY_ITEMLIST_SIZE = (9);

///////////////////////////////////////////////_Scene end_//////////////////////////////////////////////////////////
//////////////////////////////////////////////_Path start_//////////////////////////////////////////////////////////

constexpr auto MAX_PATH_FIND = 15;

#define TW_SAFEZONE		(0x0001)
#define TW_CHARACTER	(0x0002)
#define TW_NOMOVE		(0x0004)
#define TW_NOGROUND		(0x0008)
#define TW_WATER		(0x0010)
#define TW_ACTION       (0x0020)
#define TW_HEIGHT       (0x0040)
#define TW_CAMERA_UP    (0x0080)
#define TW_NOATTACKZONE (0x0100)
#define TW_ATT1         (0x0200)
#define TW_ATT2         (0x0400)
#define TW_ATT3         (0x0800)
#define TW_ATT4         (0x1000)
#define TW_ATT5         (0x2000)
#define TW_ATT6         (0x4000)
#define TW_ATT7         (0x8000)

constexpr auto FACTOR_PATH_DIST = 5;
constexpr auto FACTOR_PATH_DIST_DIAG = ((int)((float)FACTOR_PATH_DIST * 1.414f));

constexpr auto MAX_COUNT_PATH = 500;
constexpr auto MAX_INT_FORPATH = (65000 * 30000);

enum EPathNodeState : BYTE
{
    PATH_INTESTLIST = (0x01),
    PATH_TESTED = (0x02),
    PATH_END = (0x04),
};

DEFINE_ENUM_FLAG_OPERATORS(EPathNodeState)

enum EPathDirection
{
    UNDEFINED = 0x00,
    WEST = 0x1,
    SOUTHWEST = 0x2,
    SOUTH = 0x3,
    SOUTHEAST = 0x4,
    EAST = 0x5,
    NORTHEAST = 0x6,
    NORTH = 0x7,
    NORTHWEST = 0x8,
};

#define CUSTOM_CAMERA_DISTANCE1  200
#define CUSTOM_CAMERA_DISTANCE2  -150
#define RENDER_ITEMVIEW_FAR   2000.f
#define RENDER_ITEMVIEW_NEAR  20.f

#define MAX_MODEL_MONSTER	400

#define MODEL_BODY_NUM               24
#define MODEL_ITEM_COMMON_NUM		2
#define MODEL_ITEM_COMMONCNT_RAGEFIGHTER	4
#define DARKSIDE_TARGET_MAX		5

//struct
#define MAX_BOIDS         40
#define MAX_FISHS         10
#define MAX_MOUNTS    10
//bodypart
#define BODYPART_HEAD   0
#define BODYPART_HELM   1
#define BODYPART_ARMOR  2
#define BODYPART_PANTS  3
#define BODYPART_GLOVES 4
#define BODYPART_BOOTS  5
#define MAX_BODYPART    6

#define TELEPORT_NONE   0
#define TELEPORT_BEGIN  1 
#define TELEPORT        2
#define TELEPORT_END    3
#define KIND_PLAYER  1
#define KIND_MONSTER 2
#define KIND_NPC     4
#define KIND_TRAP    8
#define KIND_OPERATE 16
#define KIND_EDIT    32
#define KIND_PET     64
#define KIND_TMP     128
#define MAX_OPERATES 200
#define MAX_ITEMS 1000

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
#define ROW_INVENTORY_EXT      4
#define MAX_INVENTORY_EXT_COUNT 4
#define MAX_INVENTORY          (COLUMN_INVENTORY*ROW_INVENTORY)
#define MAX_INVENTORY_EXT_ONE  (COLUMN_INVENTORY*ROW_INVENTORY_EXT)
#define MAX_INVENTORY_EXT      (MAX_INVENTORY_EXT_ONE*MAX_INVENTORY_EXT_COUNT)
#define MAX_SHOP_INVENTORY     (COLUMN_SHOP_INVENTORY*ROW_SHOP_INVENTORY)
#define MAX_TRADE_INVENTORY    (COLUMN_TRADE_INVENTORY*ROW_TRADE_INVENTORY)
#define MAX_MIX_INVENTORY      (COLUMN_MIX_INVENTORY*ROW_MIX_INVENTORY)

/**
 * \brief Types of storage where items can be moved from/to.
 */
enum struct STORAGE_TYPE
{
    UNDEFINED = -1,
    INVENTORY = 0,
    TRADE = 1,
    VAULT = 2,
    CHAOS_MIX = 3,
    MYSHOP = 4,
    TRAINER_MIX = 5,
    ELPIS_MIX = 6,
    OSBOURNE_MIX = 7,
    JERRIDON_MIX = 8,
    CHAOS_CARD_MIX = 9,
    CHERRYBLOSSOM_MIX = 10,
    EXTRACT_SEED_MIX = 11,
    SEED_SPHERE_MIX = 12,
    ATTACH_SOCKET_MIX = 13,
    DETACH_SOCKET_MIX = 14,
    LUCKYITEM_TRADE = 15,
    LUCKYITEM_REFINERY = 16,
};

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
#define MAX_MY_INVENTORY_INDEX		(MAX_EQUIPMENT_INDEX + MAX_INVENTORY)
#define MAX_MY_INVENTORY_EX_INDEX		(MAX_MY_INVENTORY_INDEX + MAX_INVENTORY_EXT)

#define MAX_SETITEM_OPTIONS		12

#define MAX_MY_SHOP_INVENTORY_INDEX (MAX_MY_INVENTORY_EX_INDEX + MAX_SHOP_INVENTORY)

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
#define TEXT_COLOR_VIOLET	12
#define TEXT_COLOR_ORANGE	13

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

#define AUTOATTACK_ON   0x01
#define AUTOATTACK_OFF  0x02

#define WHISPER_SOUND_ON    0x04
#define WHISPER_SOUND_OFF   0x08
#define SLIDE_HELP_OFF     0x10

#define RT3_SORT_LEFT 1
#define RT3_SORT_LEFT_CLIP 2
#define RT3_SORT_CENTER 3
#define RT3_SORT_RIGHT 4

#define RT3_WRITE_RIGHT_TO_LEFT 7
#define RT3_WRITE_CENTER 8

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

#define MAX_PASSWORD_SIZE	20

#define CHAR_DEL_LIMIT_LV	300

#define MAX_TEXTS               3000
#define MAX_GLOBAL_TEXT_STRING	300
#define MAX_FILTERS 1000

#define MAX_NAMEFILTERS 500

#define MAX_LETTER_TITLE_LENGTH		60
#define MAX_LETTER_DATE_LENGTH		10
#define MAX_LETTER_TIME_LENGTH		8

#define MAX_LETTERTEXT_LENGTH		1000
#define MAX_CHATROOM_TEXT_LENGTH	150
#define MAX_LANGUAGE_NAME_LENGTH	4

#define MAX_GATES           512
#define MAX_DIALOG			200

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#define MAX_GIFT_MESSAGE_SIZE	200
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

#define MAX_LENGTH_DIALOG		( 300)

#define MAX_ANSWER_FOR_DIALOG	( 10)

#define MAX_LENGTH_ANSWER		( 64)

#define MAX_MINIMAP_NAME    100

#define MAX_SKILL_NAME      32
#define MAX_MONSTER_NAME    40

constexpr int MAX_ITEM_TYPE = 16;

constexpr int MAX_ITEM_NAME = 30;

constexpr int MAX_ITEM_INDEX = 512;

constexpr int MAX_ITEM = MAX_ITEM_TYPE * MAX_ITEM_INDEX;

#define MAX_MINI_MAP_DATA	100

#define	MAX_MASTER		   24

#define MAX_ITEM_SPECIAL   8
#define ITEM_LEVEL_NORMAL  4
#define MAX_QUEST_ITEM     64
#define MAX_EVENT_ITEM     35

#define MAX_SOCKETS			5
#define SOCKET_EMPTY		0xFE

constexpr int ITEM_GROUP_SWORD = 0;
constexpr int ITEM_GROUP_AXE = 1;
constexpr int ITEM_GROUP_MACE = 2;
constexpr int ITEM_GROUP_SPEAR = 3;
constexpr int ITEM_GROUP_BOW = 4;
constexpr int ITEM_GROUP_STAFF = 5;
constexpr int ITEM_GROUP_SHIELD = 6;
constexpr int ITEM_GROUP_HELM = 7;
constexpr int ITEM_GROUP_ARMOR = 8;
constexpr int ITEM_GROUP_PANTS = 9;
constexpr int ITEM_GROUP_GLOVES = 10;
constexpr int ITEM_GROUP_BOOTS = 11;
constexpr int ITEM_GROUP_WING = 12;
constexpr int ITEM_GROUP_HELPER = 13;
constexpr int ITEM_GROUP_POTION = 14;
constexpr int ITEM_GROUP_ETC = 15;

//item index
#define ITEM_SWORD		 (ITEM_GROUP_SWORD)
#define ITEM_AXE		 (ITEM_GROUP_AXE*MAX_ITEM_INDEX)
#define ITEM_MACE		 (ITEM_GROUP_MACE*MAX_ITEM_INDEX)
#define ITEM_SPEAR		 (ITEM_GROUP_SPEAR*MAX_ITEM_INDEX)
#define ITEM_BOW		 (ITEM_GROUP_BOW*MAX_ITEM_INDEX)
#define ITEM_STAFF		 (ITEM_GROUP_STAFF*MAX_ITEM_INDEX)
#define ITEM_SHIELD		 (ITEM_GROUP_SHIELD*MAX_ITEM_INDEX)
#define ITEM_HELM		 (ITEM_GROUP_HELM*MAX_ITEM_INDEX)
#define ITEM_ARMOR		 (ITEM_GROUP_ARMOR*MAX_ITEM_INDEX)
#define ITEM_PANTS		 (ITEM_GROUP_PANTS*MAX_ITEM_INDEX)
#define ITEM_GLOVES		 (ITEM_GROUP_GLOVES*MAX_ITEM_INDEX)
#define ITEM_BOOTS		 (ITEM_GROUP_BOOTS*MAX_ITEM_INDEX)
#define ITEM_WING		 (ITEM_GROUP_WING*MAX_ITEM_INDEX)
#define ITEM_HELPER		 (ITEM_GROUP_HELPER*MAX_ITEM_INDEX)
#define ITEM_POTION  	 (ITEM_GROUP_POTION*MAX_ITEM_INDEX)
#define ITEM_ETC 		 (ITEM_GROUP_ETC*MAX_ITEM_INDEX)




#define ITEM_ZEN  	 (ITEM_POTION + 15)

#define MAX_RESISTANCE		7

#define MAX_CLASS			7
#define MAX_CLASS_STAGES	3

#define MAX_MONSTER			1024

#define MAX_SKILLS			650

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

#define MAX_EFFECTS       200
#define MAX_SKILL_EFFECTS 100

#define MAX_SPRITES       1000
#define MAX_PARTICLES     3000
#define MAX_LEAVES        200

#define MAX_LEAVES_DOUBLE 400

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

#define ABILITY_FAST_ATTACK_SPEED	0x01
#define ABILITY_PLUS_DAMAGE			0x02
#define ABILITY_FAST_ATTACK_RING	0x04
#define ABILITY_FAST_ATTACK_SPEED2	0x08

#define CTLCODE_01BLOCKCHAR			0x01
#define CTLCODE_02BLOCKITEM			0x02
#define CTLCODE_04FORTV				0x04
#define CTLCODE_08OPERATOR			0x08
#define CTLCODE_10ACCOUNT_BLOCKITEM	0x10
#define CTLCODE_20OPERATOR			0x20

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
#define MAX_TAILS  200

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

#define FENRIR_STAND		0
#define FENRIR_WALK			1
#define FENRIR_RUN			2
#define FENRIR_ATTACK		3
#define FENRIR_ATTACK_SKILL	4
#define FENRIR_DAMAGE		5
#define FENRIR_RUN_DELAY	20

/////////////////////////////////////////////_ai end_///////////////////////////////////////////////////
////////////////////////////////////////_publicfile start_//////////////////////////////////////////////

#define MAX_MAGIC					64

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
#define INDEX_NPC_LAHAP			9
#define MAX_BUFF_SLOT_INDEX     16

/////////////////////////////////////_ccomgem end_////////////////////////////////////////////////////

#define MAX_TEXT_LENGTH		255

#define BUFFINDEX( buff )				static_cast<eBuffState>(buff)
#define BUFFTIMEINDEX( timetype )		static_cast<eBuffTimeType>(timetype)
#define ITEMINDEX( type, index )        static_cast<DWORD>((type*MAX_ITEM_INDEX)+index)

#define HACK_TIMER 1000
#define WINDOWMINIMIZED_TIMER 1001
#define CHATCONNECT_TIMER 1002
#define SLIDEHELP_TIMER 1003
#define WARNING_TIMER 1004
#define MUHELPER_TIMER 1005