#pragma once
#include "Singleton.h"
#include <map>

constexpr BYTE MAX_SET_OPTION = 64; // Maximum number of possible ancient sets
constexpr BYTE MASTERY_OPTION = 24; // A mastery option increases a specific skill

// Each item can be part of up to 2 ancient sets. The following are their identifier.
constexpr BYTE EXT_A_SET_OPTION = 1;
constexpr BYTE EXT_B_SET_OPTION = 2;

// Maximum number of equipped sets. This is the maximum number of sets that can be equipped at the same time.
// It's 5, because the number of equippable items is 12, but one is a wing and one is a pet. To make any set, you need at least 2 items.
constexpr BYTE MAX_EQUIPPED_SET_ITEMS = MAX_EQUIPMENT_INDEX - 2;
constexpr BYTE MAX_EQUIPPED_SETS = MAX_EQUIPPED_SET_ITEMS / 2;

constexpr auto MAX_ITEM_SET_NAME = 64;

constexpr auto MAX_ITEM_SET_STANDARD_OPTION_COUNT = 6;
constexpr auto MAX_ITEM_SET_STANDARD_OPTION_PER_ITEM_COUNT = 2;
constexpr auto MAX_ITEM_SET_EXT_OPTION_COUNT = 2;
constexpr auto MAX_ITEM_SETS_PER_ITEM = 2;
constexpr auto MAX_ITEM_SET_FULL_OPTION_COUNT = 5;

constexpr auto MAX_OPTIONS_PER_ITEM_SET = MAX_ITEM_SET_STANDARD_OPTION_COUNT * MAX_ITEM_SET_STANDARD_OPTION_PER_ITEM_COUNT
                                                + MAX_ITEM_SET_FULL_OPTION_COUNT
                                                + MAX_ITEM_SET_EXT_OPTION_COUNT;

struct ITEM_SET_TYPE
{
    BYTE	byOption[MAX_ITEM_SETS_PER_ITEM];
    BYTE	byMixItemLevel[MAX_ITEM_SETS_PER_ITEM];
};

struct ITEM_SET_OPTION
{
    wchar_t strSetName[MAX_ITEM_SET_NAME];
    BYTE bySetItemCount; // The number of items in the set
    BYTE byOptionCount; // The total number of options in the set

    // The following arrays are used to store the standard options of the ancient sets.
    // There is typically one option less than the number of items in the set.
    // These 2-element-arrays are somehow strange. Only the first element is actually used, I found no use case for the second one.
    // Theoretically Webzen planned to have 2 options per item, but never implemented it.
    BYTE byStandardOption[MAX_ITEM_SET_STANDARD_OPTION_COUNT][MAX_ITEM_SET_STANDARD_OPTION_PER_ITEM_COUNT];
    BYTE byStandardOptionValue[MAX_ITEM_SET_STANDARD_OPTION_COUNT][MAX_ITEM_SET_STANDARD_OPTION_PER_ITEM_COUNT];

    // There are up to 2 ext options which always apply when there are at least two items of a set equipped.
    // Found no set which uses an ExtOption.
    BYTE byExtOption[MAX_ITEM_SET_EXT_OPTION_COUNT];
    BYTE byExtOptionValue[MAX_ITEM_SET_EXT_OPTION_COUNT];

    BYTE byFullOption[MAX_ITEM_SET_FULL_OPTION_COUNT];
    BYTE byFullOptionValue[MAX_ITEM_SET_FULL_OPTION_COUNT];
    BYTE byRequireClass[MAX_CLASS];
};

struct SET_OPTION
{
    bool IsActive;
    bool IsFullOption;
    bool IsExtOption;
    bool FulfillsClassRequirement; // If the option requires a specific class and the character fulfills that. //m_bySetOptionList[x][1]
    BYTE OptionNumber; // m_bySetOptionList[x][0]
    int Value; //m_iSetOptionListValue
};

struct SET_SEARCH_RESULT
{
    BYTE SetNumber;
    BYTE CompleteSetItemCount;
    BYTE ItemCount;
    BYTE SetTypeIndex;
    wchar_t SetName[MAX_ITEM_SET_NAME];
};

struct SET_SEARCH_RESULT_OPT
    : SET_SEARCH_RESULT
{
    int SetOptionCount;
    SET_OPTION SetOption[MAX_OPTIONS_PER_ITEM_SET];
};

class CSItemOption : public Singleton<CSItemOption>
{
private:
    ITEM_SET_TYPE	m_ItemSetType[MAX_ITEM];
    ITEM_SET_OPTION	m_ItemSetOption[MAX_SET_OPTION];

    int m_SetSearchResultCount;
    SET_SEARCH_RESULT_OPT m_SetSearchResult[MAX_EQUIPPED_SETS];

    bool	m_bViewOptionList;
    BYTE    m_byRenderOptionList;
    BYTE    m_bySelectedItemOption;
    BYTE    m_bySameSetItem;

    typedef std::map<int, std::wstring>	MAP_EQUIPPEDSETITEMNAME;
    typedef std::map<BYTE, int>		MAP_EQUIPPEDSETITEM_SEQUENCE;

    MAP_EQUIPPEDSETITEMNAME	            m_mapEquippedSetItemName;
    MAP_EQUIPPEDSETITEMNAME::iterator   m_iterESIN;

    static bool isClassRequirementFulfilled(const ITEM_SET_OPTION& setOptions, int firstClass, int secondClass);
    static void TryAddSetOption(BYTE option, int value, int optionIndex, SET_SEARCH_RESULT_OPT& set, const ITEM_SET_OPTION& setOptions, bool isThisSetComplete, bool isFullOption, bool isExtOption, bool fulfillsClassRequirement, int firstClass, int secondClass);

    static bool getExplainText(wchar_t* text, const BYTE option, const int value);
    static BYTE RenderSetOptionList(const SET_SEARCH_RESULT_OPT& set, BYTE textIndex, bool bIsEquippedItem, bool bShowInactive);

    bool	OpenItemSetType(const wchar_t* filename);
    bool	OpenItemSetOption(const wchar_t* filename);
    void	checkItemType(SET_SEARCH_RESULT* optionList, const int iType, const int ancientDiscriminator) const;
    void	calcSetOptionList(const SET_SEARCH_RESULT* optionList);
    
    void    getAllAddState(WORD* Strength, WORD* Dexterity, WORD* Energy, WORD* Vitality, WORD* Charisma) const;

    void    AddStatsBySetOptions(WORD* Strength, WORD* Dexterity, WORD* Energy, WORD* Vitality, WORD* Charisma) const; //Adds the stats of the active ancient set options to the given pointers, without bonus options

    int AggregateOptionValue(int optionNumber) const;


public:
    CSItemOption(void) { init(); };
    ~CSItemOption(void) {};

    void init(void)
    {
        m_bViewOptionList = false;
        m_byRenderOptionList = 0;
        m_bySelectedItemOption = 0;
        m_bySameSetItem = 0;
    }
    bool OpenItemSetScript();

    static bool    IsDisableSkill(ActionSkillType Type, int Energy, int Charisma = 0);
    BYTE    IsChangeSetItem(const int Type, const int SubType);
    WORD    GetMixItemLevel(const int Type) const;
    bool	GetSetItemName(wchar_t* strName, const int iType, const int setType) const;

    void	PlusSpecial(WORD* Value, const int Special) const;
    void	PlusSpecialPercent(WORD* Value, const int Special) const;
    void	PlusSpecialLevel(WORD* Value, const WORD SrcValue, const int Special) const;
    void    PlusMastery(int* Value, const BYTE MasteryType) const;

    

    static int     GetDefaultOptionValue(ITEM* ip, WORD* Value);
    static bool    GetDefaultOptionText(const ITEM* ip, wchar_t* Text);
    static int     RenderDefaultOptionText(const ITEM* ip, int TextNum);

    static bool    IsNonWeaponSkillOrIsSkillEquipped(ActionSkillType skill);
    void	CheckItemSetOptions(void);
    void	MoveSetOptionList(const int StartX, const int StartY);
    void	RenderSetOptionButton(const int StartX, const int StartY);
    void	RenderSetOptionList(const int StartX, const int StartY);

    int     RenderSetOptionListInItem(const ITEM* ip, int TextNum, bool bIsEquippedItem = false);

    void    ClearOptionHelper(void) { m_byRenderOptionList = 0; }
    void    CheckRenderOptionHelper(const wchar_t* FilterName);
    void    RenderOptionHelper(void);

    bool IsAncientSetEquipped() const { return m_SetSearchResultCount > 0; }

    void SetViewOptionList(bool bView);
    bool IsViewOptionList();

    void	getAllAddOptionStatesbyCompare(WORD* Strength, WORD* Dexterity, WORD* Energy, WORD* Vitality, WORD* Charisma, WORD iCompareStrength, WORD iCompareDexterity, WORD iCompareEnergy, WORD iCompareVitality, WORD iC);

    void	getAllAddStateOnlyAddValue(WORD* AddStrength, WORD* AddDexterity, WORD* AddEnergy, WORD* AddVitality, WORD* AddCharisma) const; // Gets only the added stats of the active ancient set options plus bonus options
};

#define g_csItemOption CSItemOption::GetSingleton ()

