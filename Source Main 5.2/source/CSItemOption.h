//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#ifndef __CSITEM_OPTION_H__
#define __CSITEM_OPTION_H__

#include "Singleton.h"
#include "zzzinfomation.h"
#include <map>

const BYTE MAX_SET_OPTION = 64; // Maximum number of possible ancient sets
const BYTE MASTERY_OPTION = 24;

// Each item can be part of up to 2 ancient sets. The following are their identifier.
const BYTE EXT_A_SET_OPTION = 1;
const BYTE EXT_B_SET_OPTION = 2;

constexpr auto MAX_ITEM_SET_NAME = 64;

constexpr auto MAX_ITEM_SET_STANDARD_OPTION_COUNT = 6;
constexpr auto MAX_ITEM_SETS_PER_ITEM = 2;
constexpr auto MAX_ITEM_SET_FULL_OPTION_COUNT = 5;

constexpr auto MAX_OPTIONS_PER_ITEM_SET = 16;

struct ITEM_SET_TYPE
{
    BYTE	byOption[MAX_ITEM_SETS_PER_ITEM];
    BYTE	byMixItemLevel[MAX_ITEM_SETS_PER_ITEM];
};

struct ITEM_SET_OPTION
{
    wchar_t	strSetName[MAX_ITEM_SET_NAME];
    BYTE	byStandardOption[MAX_ITEM_SET_STANDARD_OPTION_COUNT][MAX_ITEM_SETS_PER_ITEM];
    BYTE	byStandardOptionValue[MAX_ITEM_SET_STANDARD_OPTION_COUNT][MAX_ITEM_SETS_PER_ITEM];
    BYTE	byExtOption[MAX_ITEM_SETS_PER_ITEM];
    BYTE	byExtOptionValue[MAX_ITEM_SETS_PER_ITEM];
    BYTE	byOptionCount;
    BYTE	byFullOption[MAX_ITEM_SET_FULL_OPTION_COUNT];
    BYTE	byFullOptionValue[MAX_ITEM_SET_FULL_OPTION_COUNT];
    BYTE	byRequireClass[MAX_CLASS];
};

struct SET_SEARCH_RESULT
{
    BYTE SetNumber;
    BYTE ItemCount;
    BYTE SetTypeIndex;
};

class CSItemOption : public Singleton<CSItemOption>
{
private:
    ITEM_SET_TYPE	m_ItemSetType[MAX_ITEM];
    ITEM_SET_OPTION	m_ItemSetOption[MAX_SET_OPTION];

    bool	m_bViewOptionList;
    BYTE    m_byRenderOptionList;
    BYTE    m_bySelectedItemOption;
    BYTE    m_bySameSetItem;
    wchar_t m_strSetName[MAX_ITEM_SETS_PER_ITEM][MAX_ITEM_SET_NAME];

    typedef std::map<int, std::wstring>	MAP_EQUIPPEDSETITEMNAME;
    typedef std::map<BYTE, int>		MAP_EQUIPPEDSETITEM_SEQUENCE;

    MAP_EQUIPPEDSETITEMNAME	            m_mapEquippedSetItemName;
    MAP_EQUIPPEDSETITEMNAME::iterator   m_iterESIN;

    MAP_EQUIPPEDSETITEM_SEQUENCE            m_mapEquippedSetItemSequence;
    MAP_EQUIPPEDSETITEM_SEQUENCE::iterator  m_iterESIS;
private:
    void  UpdateCount_SetOptionPerEquippedSetItem(const SET_SEARCH_RESULT* byOptionList, int* arLimitSetItemOptionCount, ITEM* ItemsEquipment);
    int GetCurrentTypeSetitemCount(const ITEM& CurItem_, const SET_SEARCH_RESULT* byOptionList);
    int Search_From_EquippedSetItemNameMapTable(wchar_t* szSetItemname);

public:
    bool	Find_From_EquippedSetItemNameMapTable(wchar_t* szSetItemname)
    {
        int iResult = Search_From_EquippedSetItemNameMapTable(szSetItemname);

        if (-1 == iResult)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    BYTE Search_From_EquippedSetItemNameSequence(wchar_t* szSetItemname)
    {
        BYTE	byCur = 0;
        int		iResult = Search_From_EquippedSetItemNameMapTable(szSetItemname);

        if (-1 != iResult)
        {
            for (m_iterESIS = m_mapEquippedSetItemSequence.begin(); m_iterESIS != m_mapEquippedSetItemSequence.end(); ++m_iterESIS)
            {
                if ((int)(m_iterESIS->second) == iResult)
                {
                    return (BYTE)m_iterESIS->first;
                }
            }
        }

        return 255;
    }

    BYTE	m_bySetOptionANum;
    BYTE	m_bySetOptionBNum;
    BYTE    m_bySetOptionIndex[MAX_ITEM_SETS_PER_ITEM];
    BYTE	m_bySetOptionList[MAX_OPTIONS_PER_ITEM_SET][MAX_ITEM_SETS_PER_ITEM];
    BYTE	m_bySetOptionListOnOff[MAX_OPTIONS_PER_ITEM_SET];
    int     m_iSetOptionListValue[MAX_OPTIONS_PER_ITEM_SET][MAX_ITEM_SETS_PER_ITEM];

    bool	OpenItemSetType(const wchar_t* filename);
    bool	OpenItemSetOption(const wchar_t* filename);

    void	checkItemType(SET_SEARCH_RESULT* optionList, const int iType, const int setType);

    void	calcSetOptionList(SET_SEARCH_RESULT* optionList);

    void	getExplainText(wchar_t* text, const BYTE option, const BYTE value, const BYTE SetA);

    void    getAllAddState(WORD* Strength, WORD* Dexterity, WORD* Energy, WORD* Vitality, WORD* Charisma);
    int     GetSetItmeCount(const ITEM* pselecteditem);
    bool	isFullseteffect(const ITEM* ip);

public:

    void	getAllAddOptionStatesbyCompare(WORD* Strength, WORD* Dexterity, WORD* Energy, WORD* Vitality, WORD* Charisma, WORD iCompareStrength, WORD iCompareDexterity, WORD iCompareEnergy, WORD iCompareVitality, WORD iC);

    void	getAllAddStateOnlyAddValue(WORD* AddStrength, WORD* AddDexterity, WORD* AddEnergy, WORD* AddVitality, WORD* AddCharisma);

public:
    CSItemOption(void) { init(); };
    ~CSItemOption(void) {};

    void init(void)
    {
        m_strSetName[0][0] = L'\0';
        m_strSetName[1][0] = L'\0';

        m_bViewOptionList = false;
        m_byRenderOptionList = 0;
        m_bySelectedItemOption = 0;
        m_bySameSetItem = 0;
    }
    void    ClearListOnOff(void)
    {
        memset(m_bySetOptionListOnOff, 0, sizeof(BYTE) * MAX_OPTIONS_PER_ITEM_SET);
    }
    bool    OpenItemSetScript(bool bTestServer);

    bool    IsDisableSkill(int Type, int Energy, int Charisma = 0);
    BYTE    IsChangeSetItem(const int Type, const int SubType);
    WORD    GetMixItemLevel(const int Type);
    bool	GetSetItemName(wchar_t* strName, const int iType, const int setType);

    void	PlusSpecial(WORD* Value, const int Special);
    void	PlusSpecialPercent(WORD* Value, const int Special);
    void	PlusSpecialLevel(WORD* Value, const WORD SrcValue, const int Special);
    void    PlusMastery(int* Value, const BYTE MasteryType);

    void    MinusSpecialPercent(int* Value, const int Special);

    void	GetSpecial(WORD* Value, int Special);

    void	GetSpecialPercent(WORD* Value, int Special);
    void	GetSpecialLevel(WORD* Value, const WORD SrcValue, int Special);
    int     GetDefaultOptionValue(ITEM* ip, WORD* Value);
    bool    GetDefaultOptionText(const ITEM* ip, wchar_t* Text);
    int     RenderDefaultOptionText(const ITEM* ip, int TextNum);

    bool    Special_Option_Check(int Kind = 0);
    void	CheckItemSetOptions(void);
    void	MoveSetOptionList(const int StartX, const int StartY);
    void	RenderSetOptionButton(const int StartX, const int StartY);
    void	RenderSetOptionList(const int StartX, const int StartY);

    int		m_arLimitSetItemOptionCount[MAX_EQUIPMENT];

    int     RenderSetOptionListInItem(const ITEM* ip, int TextNum, bool bIsEquippedItem = false);

    void    ClearOptionHelper(void) { m_byRenderOptionList = 0; }
    void    CheckRenderOptionHelper(const wchar_t* FilterName);
    void    RenderOptionHelper(void);

    BYTE GetSetOptionANum();
    BYTE GetSetOptionBNum();

    void SetViewOptionList(bool bView);
    bool IsViewOptionList();
};

#define g_csItemOption CSItemOption::GetSingleton ()

extern std::wstring g_strSelectedML;

#endif
