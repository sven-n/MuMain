#include "stdafx.h"

#include <algorithm>
#include <vector>
#include <array>

#include "UIControls.h"
#include "NewUISystem.h"
#include "NewUIMuHelper.h"
#include "CharacterManager.h"
#include "MUHelper/MuHelper.h"

using namespace MUHelper;

// defining constants naming since the original code hard coded these ids

enum ECheckBoxId: uint16_t
{
    CHECKBOX_ID_POTION = 0,
    CHECKBOX_ID_LONG_DISTANCE,
    CHECKBOX_ID_ORIG_POSITION,
    CHECKBOX_ID_SKILL2_DELAY,
    CHECKBOX_ID_SKILL2_CONDITION,
    CHECKBOX_ID_SKILL3_DELAY,
    CHECKBOX_ID_SKILL3_CONDITION,
    CHECKBOX_ID_COMBO,
    CHECKBOX_ID_BUFF_DURATION,
    CHECKBOX_ID_USE_PET,
    CHECKBOX_ID_PARTY,
    CHECKBOX_ID_AUTO_HEAL,
    CHECKBOX_ID_DRAIN_LIFE,
    CHECKBOX_ID_REPAIR_ITEM,
    CHECKBOX_ID_PICK_ALL,
    CHECKBOX_ID_PICK_SELECTED,
    CHECKBOX_ID_PICK_JEWEL,
    CHECKBOX_ID_PICK_ANCIENT,
    CHECKBOX_ID_PICK_ZEN,
    CHECKBOX_ID_PICK_EXCELLENT,
    CHECKBOX_ID_ADD_OTHER_ITEM,
    CHECKBOX_ID_AUTO_ACCEPT_FRIEND,
    CHECKBOX_ID_AUTO_DEFEND,
    CHECKBOX_ID_AUTO_ACCEPT_GUILD,
    CHECKBOX_ID_DR_ATTACK_CEASE,
    CHECKBOX_ID_DR_ATTACK_AUTO,
    CHECKBOX_ID_DR_ATTACK_TOGETHER
};

enum EButtonId : uint16_t
{
    BUTTON_ID_HUNT_RANGE_ADD = 0,
    BUTTON_ID_HUNT_RANGE_MINUS,
    BUTTON_ID_SKILL2_CONFIG,
    BUTTON_ID_SKILL3_CONFIG,
    BUTTON_ID_POTION_CONFIG_ELF,
    BUTTON_ID_POTION_CONFIG_SUMMY,
    BUTTON_ID_POTION_CONFIG,
    BUTTON_ID_PARTY_CONFIG,
    BUTTON_ID_PARTY_CONFIG_ELF,
    BUTTON_ID_PICK_RANGE_ADD,
    BUTTON_ID_PICK_RANGE_MINUS,
    BUTTON_ID_ADD_OTHER_ITEM,
    BUTTON_ID_DELETE_OTHER_ITEM,
    BUTTON_ID_SAVE_CONFIG,
    BUTTON_ID_INIT_CONFIG,
    BUTTON_ID_EXIT_CONFIG
};

enum ESkillSlotImg : uint16_t
{
    SKILL_SLOT_SKILL1 = 0,
    SKILL_SLOT_SKILL2 = 1,
    SKILL_SLOT_SKILL3 = 2,
    SKILL_SLOT_BUFF1 = 3,
    SKILL_SLOT_BUFF2 = 4,
    SKILL_SLOT_BUFF3 = 5
};

enum ETextBoxImg : uint16_t
{
    TEXTBOX_IMG_DISTANCE_TIME = 6,
    TEXTBOX_IMG_SKILL1_TIME = 7,
    TEXTBOX_IMG_SKILL2_TIME = 8,
    TEXTBOX_IMG_ADD_EXTRA_ITEM = 9
};

constexpr int BITMAP_DISTANCE_BEGIN = BITMAP_INTERFACE_CRYWOLF_BEGIN + 33;

constexpr int MAX_NUMBER_DIGITS = 3;
constexpr int MAX_HUNTING_RANGE = 6;
constexpr int MAX_OBTAINING_RANGE = 8;

enum ESkillSlot
{
    SUB_PAGE_SKILL2_CONFIG = 2, // aligns with BUTTON_ID_SKILL2_CONFIG
    SUB_PAGE_SKILL3_CONFIG,
    SUB_PAGE_POTION_CONFIG_ELF,
    SUB_PAGE_POTION_CONFIG_SUMMY,
    SUB_PAGE_POTION_CONFIG,
    SUB_PAGE_PARTY_CONFIG,
    SUB_PAGE_PARTY_CONFIG_ELF
};

using namespace SEASON3B;

ConfigData _TempConfig;

CNewUIMuHelper::CNewUIMuHelper()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_ButtonList.clear();

    m_iCurrentOpenTab = 0;

    m_iSelectedSkillSlot = 0;
    m_aiSelectedSkills.fill(-1);
}

CNewUIMuHelper::~CNewUIMuHelper()
{
    Release();
}

bool CNewUIMuHelper::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(INTERFACE_MUHELPER, this);

    SetPos(x, y);

    LoadImages();

    InitButtons();

    InitCheckBox();

    InitImage();

    InitText();

    InitTextboxInput();

    Show(false);

    return true;
}

void CNewUIMuHelper::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNewUIMuHelper::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

void CNewUIMuHelper::InitButtons()
{
    std::list<std::wstring> ltext;
    ltext.push_back(GlobalText[3500]);
    ltext.push_back(GlobalText[3501]);
    ltext.push_back(GlobalText[3590]);

    m_TabBtn.CreateRadioGroup(3, IMAGE_WINDOW_TAB_BTN, TRUE);
    m_TabBtn.ChangeRadioText(ltext);
    m_TabBtn.ChangeRadioButtonInfo(true, m_Pos.x + 10.f, m_Pos.y + 48.f, 56, 22);
    m_TabBtn.ChangeFrame(m_iCurrentOpenTab);

    InsertButton(IMAGE_CHAINFO_BTN_STAT, m_Pos.x + 56, m_Pos.y + 78, 16, 15, 0, 0, 0, 0, L"", L"", BUTTON_ID_HUNT_RANGE_ADD, 0);
    InsertButton(IMAGE_MACROUI_HELPER_RAGEMINUS, m_Pos.x + 56, m_Pos.y + 97, 16, 15, 0, 0, 0, 0, L"", L"", BUTTON_ID_HUNT_RANGE_MINUS, 0);
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 191, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", BUTTON_ID_SKILL2_CONFIG, 0); //-- skill 2
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 243, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", BUTTON_ID_SKILL3_CONFIG, 0); //-- skill 3
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 84, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", BUTTON_ID_POTION_CONFIG_ELF, 0); //-- Buff
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 79, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", BUTTON_ID_POTION_CONFIG_SUMMY, 0); //-- potion
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 84, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", BUTTON_ID_POTION_CONFIG, 0); //-- potion
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 17, m_Pos.y + 234, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", BUTTON_ID_PARTY_CONFIG, 0); //-- potion
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 17, m_Pos.y + 234, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", BUTTON_ID_PARTY_CONFIG_ELF, 0); //-- potion

    InsertButton(IMAGE_CHAINFO_BTN_STAT, m_Pos.x + 56, m_Pos.y + 78, 16, 15, 0, 0, 0, 0, L"", L"", BUTTON_ID_PICK_RANGE_ADD, 1);
    InsertButton(IMAGE_MACROUI_HELPER_RAGEMINUS, m_Pos.x + 56, m_Pos.y + 97, 16, 15, 0, 0, 0, 0, L"", L"", BUTTON_ID_PICK_RANGE_MINUS, 1);
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 208, 38, 24, 1, 0, 1, 1, GlobalText[3505], L"", BUTTON_ID_ADD_OTHER_ITEM, 1); //-- Buff
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 309, 38, 24, 1, 0, 1, 1, GlobalText[3506], L"", BUTTON_ID_DELETE_OTHER_ITEM, 1); //-- Buff
    //--
    InsertButton(IMAGE_IGS_BUTTON, m_Pos.x + 120, m_Pos.y + 388, 52, 26, 1, 0, 1, 1, GlobalText[3503], L"", BUTTON_ID_SAVE_CONFIG, -1);
    InsertButton(IMAGE_IGS_BUTTON, m_Pos.x + 65, m_Pos.y + 388, 52, 26, 1, 0, 1, 1, GlobalText[3504], L"", BUTTON_ID_INIT_CONFIG, -1);
    InsertButton(IMAGE_BASE_WINDOW_BTN_EXIT, m_Pos.x + 20, m_Pos.y + 388, 36, 29, 0, 0, 0, 0, L"", GlobalText[388], BUTTON_ID_EXIT_CONFIG, -1);

    RegisterBtnCharacter(0xFF, BUTTON_ID_HUNT_RANGE_ADD);
    RegisterBtnCharacter(0xFF, BUTTON_ID_HUNT_RANGE_MINUS);
    RegisterBtnCharacter(0xFF, BUTTON_ID_SKILL2_CONFIG);
    RegisterBtnCharacter(0xFF, BUTTON_ID_PICK_RANGE_ADD);
    RegisterBtnCharacter(0xFF, BUTTON_ID_PICK_RANGE_MINUS);
    RegisterBtnCharacter(0xFF, BUTTON_ID_ADD_OTHER_ITEM);
    RegisterBtnCharacter(0xFF, BUTTON_ID_DELETE_OTHER_ITEM);
    RegisterBtnCharacter(0xFF, BUTTON_ID_SAVE_CONFIG);
    RegisterBtnCharacter(0xFF, BUTTON_ID_INIT_CONFIG);
    RegisterBtnCharacter(0xFF, BUTTON_ID_EXIT_CONFIG);

    RegisterBtnCharacter(Dark_Knight, BUTTON_ID_SKILL3_CONFIG);
    RegisterBtnCharacter(Dark_Knight, BUTTON_ID_POTION_CONFIG);

    RegisterBtnCharacter(Dark_Wizard, BUTTON_ID_SKILL3_CONFIG);
    RegisterBtnCharacter(Dark_Wizard, BUTTON_ID_POTION_CONFIG);
    RegisterBtnCharacter(Dark_Wizard, BUTTON_ID_PARTY_CONFIG);

    RegisterBtnCharacter(Magic_Gladiator, BUTTON_ID_SKILL3_CONFIG);
    RegisterBtnCharacter(Magic_Gladiator, BUTTON_ID_POTION_CONFIG);
    RegisterBtnCharacter(Dark_Lord, BUTTON_ID_POTION_CONFIG);

    RegisterBtnCharacter(Rage_Fighter, BUTTON_ID_SKILL3_CONFIG);
    RegisterBtnCharacter(Rage_Fighter, BUTTON_ID_POTION_CONFIG);

    RegisterBtnCharacter(Fairy_Elf, BUTTON_ID_SKILL3_CONFIG);
    RegisterBtnCharacter(Fairy_Elf, BUTTON_ID_POTION_CONFIG_ELF);
    RegisterBtnCharacter(Fairy_Elf, BUTTON_ID_PARTY_CONFIG_ELF);

    RegisterBtnCharacter(Summoner, BUTTON_ID_SKILL3_CONFIG);
    RegisterBtnCharacter(Summoner, BUTTON_ID_POTION_CONFIG_SUMMY);
}

void CNewUIMuHelper::InitCheckBox()
{
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 79, m_Pos.y + 80, 15, 15, 0, GlobalText[3507], CHECKBOX_ID_POTION, 0);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 18, m_Pos.y + 122, 15, 15, 0, GlobalText[3508], CHECKBOX_ID_LONG_DISTANCE, 0);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 18, m_Pos.y + 137, 15, 15, 0, GlobalText[3509], CHECKBOX_ID_ORIG_POSITION, 0);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 94, m_Pos.y + 174, 15, 15, 0, GlobalText[3510], CHECKBOX_ID_SKILL2_DELAY, 0);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 94, m_Pos.y + 191, 15, 15, 0, GlobalText[3511], CHECKBOX_ID_SKILL2_CONDITION, 0);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 94, m_Pos.y + 226, 15, 15, 0, GlobalText[3510], CHECKBOX_ID_SKILL3_DELAY, 0);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 94, m_Pos.y + 243, 15, 15, 0, GlobalText[3511], CHECKBOX_ID_SKILL3_CONDITION, 0);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 18, m_Pos.y + 226, 15, 15, 0, GlobalText[3512], CHECKBOX_ID_COMBO, 0);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 18, m_Pos.y + 276, 15, 15, 0, GlobalText[3513], CHECKBOX_ID_BUFF_DURATION, 0);

    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 18, m_Pos.y + 218, 15, 15, 0, GlobalText[3514], CHECKBOX_ID_USE_PET, 0);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 18, m_Pos.y + 218, 15, 15, 0, GlobalText[3515], CHECKBOX_ID_PARTY, 0);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 79, m_Pos.y + 97, 15, 15, 0, GlobalText[3516], CHECKBOX_ID_AUTO_HEAL, 0);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 79, m_Pos.y + 97, 15, 15, 0, GlobalText[3517], CHECKBOX_ID_DRAIN_LIFE, 0);

    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 79, m_Pos.y + 80, 15, 15, 0, GlobalText[3518], CHECKBOX_ID_REPAIR_ITEM, 1);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 17, m_Pos.y + 125, 15, 15, 0, GlobalText[3519], CHECKBOX_ID_PICK_ALL, 1);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 17, m_Pos.y + 152, 15, 15, 0, GlobalText[3520], CHECKBOX_ID_PICK_SELECTED, 1);

    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 22, m_Pos.y + 170, 15, 15, 0, GlobalText[3521], CHECKBOX_ID_PICK_JEWEL, 1);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 85, m_Pos.y + 170, 15, 15, 0, GlobalText[3522], CHECKBOX_ID_PICK_ANCIENT, 1);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 22, m_Pos.y + 185, 15, 15, 0, GlobalText[3523], CHECKBOX_ID_PICK_ZEN, 1);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 85, m_Pos.y + 185, 15, 15, 0, GlobalText[3524], CHECKBOX_ID_PICK_EXCELLENT, 1);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 22, m_Pos.y + 200, 15, 15, 0, GlobalText[3525], CHECKBOX_ID_ADD_OTHER_ITEM, 1);
    //--

    InsertCheckBox(IMAGE_MACROUI_HELPER_OPTIONBUTTON, m_Pos.x + 94, m_Pos.y + 235, 15, 15, 0, GlobalText[3533], CHECKBOX_ID_DR_ATTACK_CEASE, 0);
    InsertCheckBox(IMAGE_MACROUI_HELPER_OPTIONBUTTON, m_Pos.x + 30, m_Pos.y + 235, 15, 15, 0, GlobalText[3534], CHECKBOX_ID_DR_ATTACK_AUTO, 0);
    InsertCheckBox(IMAGE_MACROUI_HELPER_OPTIONBUTTON, m_Pos.x + 30, m_Pos.y + 250, 15, 15, 0, GlobalText[3535], CHECKBOX_ID_DR_ATTACK_TOGETHER, 0);

    //--
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 18, m_Pos.y + 80, 15, 15, 0, GlobalText[3591], CHECKBOX_ID_AUTO_ACCEPT_FRIEND, 2);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 18, m_Pos.y + 125, 15, 15, 0, GlobalText[3593], CHECKBOX_ID_AUTO_DEFEND, 2);
    InsertCheckBox(IMAGE_CHECKBOX_BTN, m_Pos.x + 18, m_Pos.y + 97, 15, 15, 0, GlobalText[3592], CHECKBOX_ID_AUTO_ACCEPT_GUILD, 2);

    RegisterBoxCharacter(0xFF, CHECKBOX_ID_POTION);
    RegisterBoxCharacter(0xFF, CHECKBOX_ID_LONG_DISTANCE);
    RegisterBoxCharacter(0xFF, CHECKBOX_ID_ORIG_POSITION);
    RegisterBoxCharacter(0xFF, CHECKBOX_ID_SKILL2_DELAY);
    RegisterBoxCharacter(0xFF, CHECKBOX_ID_SKILL2_CONDITION);
    RegisterBoxCharacter(0xFF, CHECKBOX_ID_BUFF_DURATION);
    RegisterBoxCharacter(0xFF, CHECKBOX_ID_REPAIR_ITEM);
    RegisterBoxCharacter(0xFF, CHECKBOX_ID_PICK_ALL);
    RegisterBoxCharacter(0xFF, CHECKBOX_ID_PICK_SELECTED);
    RegisterBoxCharacter(0xFF, CHECKBOX_ID_PICK_JEWEL);
    RegisterBoxCharacter(0xFF, CHECKBOX_ID_PICK_ANCIENT);
    RegisterBoxCharacter(0xFF, CHECKBOX_ID_PICK_ZEN);
    RegisterBoxCharacter(0xFF, CHECKBOX_ID_PICK_EXCELLENT);
    RegisterBoxCharacter(0xFF, CHECKBOX_ID_ADD_OTHER_ITEM);
    RegisterBoxCharacter(0xFF, CHECKBOX_ID_AUTO_ACCEPT_FRIEND);
    RegisterBoxCharacter(0xFF, CHECKBOX_ID_AUTO_DEFEND);
    RegisterBoxCharacter(0xFF, CHECKBOX_ID_AUTO_ACCEPT_GUILD);

    RegisterBoxCharacter(Dark_Knight, CHECKBOX_ID_SKILL3_DELAY);
    RegisterBoxCharacter(Dark_Knight, CHECKBOX_ID_SKILL3_CONDITION);
    RegisterBoxCharacter(Dark_Knight, CHECKBOX_ID_COMBO);

    RegisterBoxCharacter(Dark_Wizard, CHECKBOX_ID_SKILL3_DELAY);
    RegisterBoxCharacter(Dark_Wizard, CHECKBOX_ID_SKILL3_CONDITION);
    RegisterBoxCharacter(Dark_Wizard, CHECKBOX_ID_PARTY);

    RegisterBoxCharacter(Magic_Gladiator, CHECKBOX_ID_SKILL3_DELAY);
    RegisterBoxCharacter(Magic_Gladiator, CHECKBOX_ID_SKILL3_CONDITION);

    RegisterBoxCharacter(Dark_Lord, CHECKBOX_ID_USE_PET);
    RegisterBoxCharacter(Dark_Lord, CHECKBOX_ID_DR_ATTACK_CEASE);
    RegisterBoxCharacter(Dark_Lord, CHECKBOX_ID_DR_ATTACK_AUTO);
    RegisterBoxCharacter(Dark_Lord, CHECKBOX_ID_DR_ATTACK_TOGETHER);

    RegisterBoxCharacter(Fairy_Elf, CHECKBOX_ID_AUTO_HEAL);
    RegisterBoxCharacter(Fairy_Elf, CHECKBOX_ID_SKILL3_DELAY);
    RegisterBoxCharacter(Fairy_Elf, CHECKBOX_ID_SKILL3_CONDITION);
    RegisterBoxCharacter(Fairy_Elf, CHECKBOX_ID_PARTY);

    RegisterBoxCharacter(Summoner, CHECKBOX_ID_SKILL3_DELAY);
    RegisterBoxCharacter(Summoner, CHECKBOX_ID_SKILL3_CONDITION);
    RegisterBoxCharacter(Summoner, CHECKBOX_ID_DRAIN_LIFE);

    RegisterBoxCharacter(Rage_Fighter, CHECKBOX_ID_SKILL3_DELAY);
    RegisterBoxCharacter(Rage_Fighter, CHECKBOX_ID_SKILL3_CONDITION);
}

void CNewUIMuHelper::InitImage()
{
    InsertIcon(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 17, m_Pos.y + 171, 32, 38, SKILL_SLOT_SKILL1, 0);
    InsertIcon(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 61, m_Pos.y + 171, 32, 38, SKILL_SLOT_SKILL2, 0);
    InsertIcon(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 61, m_Pos.y + 222, 32, 38, SKILL_SLOT_SKILL3, 0);
    InsertIcon(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 21, m_Pos.y + 293, 32, 38, SKILL_SLOT_BUFF1, 0);
    InsertIcon(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 55, m_Pos.y + 293, 32, 38, SKILL_SLOT_BUFF2, 0);
    InsertIcon(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 89, m_Pos.y + 293, 32, 38, SKILL_SLOT_BUFF3, 0);

    InsertIcon(IMAGE_MACROUI_HELPER_INPUTNUMBER, m_Pos.x + 140, m_Pos.y + 137, 20, 15, TEXTBOX_IMG_DISTANCE_TIME, 0);
    InsertIcon(IMAGE_MACROUI_HELPER_INPUTNUMBER, m_Pos.x + 140, m_Pos.y + 174, 20, 15, TEXTBOX_IMG_SKILL1_TIME, 0);
    InsertIcon(IMAGE_MACROUI_HELPER_INPUTNUMBER, m_Pos.x + 140, m_Pos.y + 226, 20, 15, TEXTBOX_IMG_SKILL2_TIME, 0);
    InsertIcon(IMAGE_MACROUI_HELPER_INPUTSTRING, m_Pos.x + 34, m_Pos.y + 216, 94, 15, TEXTBOX_IMG_ADD_EXTRA_ITEM, 1);

    RegisterIconCharacter(0xFF, SKILL_SLOT_SKILL1);
    RegisterIconCharacter(0xFF, SKILL_SLOT_SKILL2);
    RegisterIconCharacter(0xFF, SKILL_SLOT_BUFF1);
    RegisterIconCharacter(0xFF, SKILL_SLOT_BUFF2);
    RegisterIconCharacter(0xFF, SKILL_SLOT_BUFF3);
    RegisterIconCharacter(0xFF, TEXTBOX_IMG_DISTANCE_TIME);
    RegisterIconCharacter(0xFF, TEXTBOX_IMG_SKILL1_TIME);
    RegisterIconCharacter(0xFF, TEXTBOX_IMG_ADD_EXTRA_ITEM);

    RegisterIconCharacter(Dark_Knight, SKILL_SLOT_SKILL3);
    RegisterIconCharacter(Dark_Knight, TEXTBOX_IMG_SKILL2_TIME);
    RegisterIconCharacter(Dark_Wizard, SKILL_SLOT_SKILL3);
    RegisterIconCharacter(Dark_Wizard, TEXTBOX_IMG_SKILL2_TIME);
    RegisterIconCharacter(Fairy_Elf, SKILL_SLOT_SKILL3);
    RegisterIconCharacter(Fairy_Elf, TEXTBOX_IMG_SKILL2_TIME);
    RegisterIconCharacter(Magic_Gladiator, SKILL_SLOT_SKILL3);
    RegisterIconCharacter(Magic_Gladiator, TEXTBOX_IMG_SKILL2_TIME);
    RegisterIconCharacter(Summoner, SKILL_SLOT_SKILL3);
    RegisterIconCharacter(Summoner, TEXTBOX_IMG_SKILL2_TIME);
    RegisterIconCharacter(Rage_Fighter, SKILL_SLOT_SKILL3);
    RegisterIconCharacter(Rage_Fighter, TEXTBOX_IMG_SKILL2_TIME);
}

void CNewUIMuHelper::InitText()
{
    InsertText(m_Pos.x + 18, m_Pos.y + 78, GlobalText[3526], 1, 0); // Range
    InsertText(m_Pos.x + 18, m_Pos.y + 83, L"________", 2, 0);
    InsertText(m_Pos.x + 110, m_Pos.y + 141, GlobalText[3527], 3, 0); // Distance
    //InsertText(m_Pos.x + 162, m_Pos.y + 141, GlobalText[3528], 4, 0);
    InsertText(m_Pos.x + 162, m_Pos.y + 141, L"s", 4, 0);

    InsertText(m_Pos.x + 18, m_Pos.y + 160, GlobalText[3529], 5, 0); // Basic Skill
    InsertText(m_Pos.x + 59, m_Pos.y + 160, GlobalText[3530], 7, 0); // Activation Skill 1
    //InsertText(m_Pos.x + 162, m_Pos.y + 178, GlobalText[3528], 8, 0);
    InsertText(m_Pos.x + 162, m_Pos.y + 178, L"s", 8, 0);
    InsertText(m_Pos.x + 59, m_Pos.y + 212, GlobalText[3531], 9, 0); // Activation Skill 2

    //InsertText(m_Pos.x + 162, m_Pos.y + 230, GlobalText[3528], 10, 0);
    InsertText(m_Pos.x + 162, m_Pos.y + 230, L"s", 10, 0);
    InsertText(m_Pos.x + 18, m_Pos.y + 78, GlobalText[3532], 11, 1); // Range
    InsertText(m_Pos.x + 18, m_Pos.y + 83, L"________", 12, 1);

    RegisterTextCharacter(0xFF, 1);
    RegisterTextCharacter(0xFF, 2);
    RegisterTextCharacter(0xFF, 3);
    RegisterTextCharacter(0xFF, 4);
    RegisterTextCharacter(0xFF, 5);
    RegisterTextCharacter(0xFF, 7);
    RegisterTextCharacter(0xFF, 8);
    RegisterTextCharacter(0xFF, 11);
    RegisterTextCharacter(0xFF, 12);

    RegisterTextCharacter(Dark_Knight, 9);
    RegisterTextCharacter(Dark_Knight, 10);
    RegisterTextCharacter(Dark_Wizard, 9);
    RegisterTextCharacter(Dark_Wizard, 10);
    RegisterTextCharacter(Fairy_Elf, 9);
    RegisterTextCharacter(Fairy_Elf, 10);
    RegisterTextCharacter(Magic_Gladiator, 9);
    RegisterTextCharacter(Magic_Gladiator, 10);
    RegisterTextCharacter(Summoner, 9);
    RegisterTextCharacter(Summoner, 10);
    RegisterTextCharacter(Rage_Fighter, 9);
    RegisterTextCharacter(Rage_Fighter, 10);
}

void CNewUIMuHelper::InitTextboxInput()
{
    wchar_t wsInitText[MAX_NUMBER_DIGITS + 1];

    m_DistanceTimeInput.Init(g_hWnd, 17, 15, MAX_NUMBER_DIGITS, false);
    m_DistanceTimeInput.SetPosition(m_Pos.x + 142, m_Pos.y + 140);
    m_DistanceTimeInput.SetTextColor(255, 0, 0, 0);
    m_DistanceTimeInput.SetBackColor(255, 255, 255, 255);
    m_DistanceTimeInput.SetFont(g_hFont);
    m_DistanceTimeInput.SetState(UISTATE_NORMAL);
    m_DistanceTimeInput.SetOption(UIOPTION_NUMBERONLY);
    swprintf(wsInitText, MAX_NUMBER_DIGITS + 1, L"%d", _TempConfig.iMaxSecondsAway);
    m_DistanceTimeInput.SetText(wsInitText);

    m_Skill2DelayInput.Init(g_hWnd, 17, 15, MAX_NUMBER_DIGITS, false);
    m_Skill2DelayInput.SetPosition(m_Pos.x + 142, m_Pos.y + 177);
    m_Skill2DelayInput.SetTextColor(255, 0, 0, 0);
    m_Skill2DelayInput.SetBackColor(255, 255, 255, 255);
    m_Skill2DelayInput.SetFont(g_hFont);
    m_Skill2DelayInput.SetState(UISTATE_NORMAL);
    m_Skill2DelayInput.SetOption(UIOPTION_NUMBERONLY);
    swprintf(wsInitText, MAX_NUMBER_DIGITS + 1, L"%d", _TempConfig.aiSkillInterval[1]);
    m_Skill2DelayInput.SetText(wsInitText);

    m_Skill3DelayInput.Init(g_hWnd, 17, 15, MAX_NUMBER_DIGITS, false);
    m_Skill3DelayInput.SetPosition(m_Pos.x + 142, m_Pos.y + 229);
    m_Skill3DelayInput.SetTextColor(255, 0, 0, 0);
    m_Skill3DelayInput.SetBackColor(255, 255, 255, 255);
    m_Skill3DelayInput.SetFont(g_hFont);
    m_Skill3DelayInput.SetState(UISTATE_NORMAL);
    m_Skill3DelayInput.SetOption(UIOPTION_NUMBERONLY);
    swprintf(wsInitText, MAX_NUMBER_DIGITS + 1, L"%d", _TempConfig.aiSkillInterval[2]);
    m_Skill3DelayInput.SetText(wsInitText);

    m_ItemInput.Init(g_hWnd, 88, 15, MAX_ITEM_NAME, false);
    m_ItemInput.SetPosition(m_Pos.x + 36, m_Pos.y + 219);
    m_ItemInput.SetTextColor(255, 0, 0, 0);
    m_ItemInput.SetBackColor(255, 255, 255, 255);
    m_ItemInput.SetFont(g_hFont);
    m_ItemInput.SetState(UISTATE_HIDE);

    m_ItemFilter.SetSize(160, 70);
    m_ItemFilter.SetPosition(m_Pos.x + 20, m_Pos.y + 238 + m_ItemFilter.GetHeight());
}

bool CNewUIMuHelper::Update()
{
    if (IsVisible())
    {
        int iNumCurOpenTab = m_TabBtn.UpdateMouseEvent();

        if (iNumCurOpenTab == RADIOGROUPEVENT_NONE)
            return true;

        m_iCurrentOpenTab = iNumCurOpenTab;

        if (m_iCurrentOpenTab == 0)
        {
            m_DistanceTimeInput.SetState(UISTATE_NORMAL);
            m_Skill2DelayInput.SetState(UISTATE_NORMAL);
            m_Skill3DelayInput.SetState(UISTATE_NORMAL);
            m_ItemInput.SetState(UISTATE_HIDE);
        }
        else if (m_iCurrentOpenTab == 1)
        {
            m_DistanceTimeInput.SetState(UISTATE_HIDE);
            m_Skill2DelayInput.SetState(UISTATE_HIDE);
            m_Skill3DelayInput.SetState(UISTATE_HIDE);
            m_ItemInput.SetState(UISTATE_NORMAL);
        }
    }
    return true;
}

bool CNewUIMuHelper::UpdateMouseEvent()
{
    // Ignore events outside MU Helper window
    if (!CheckMouseIn(m_Pos.x, m_Pos.y, WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        return true;
    }

    int iButtonId = UpdateMouseBtnList();
    if (iButtonId != -1)
    {
        g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Clicked button [%d]", iButtonId);

        if (iButtonId == BUTTON_ID_HUNT_RANGE_ADD)
        {
            ApplyHuntRangeUpdate(1);
        }
        else if (iButtonId == BUTTON_ID_HUNT_RANGE_MINUS)
        {
            ApplyHuntRangeUpdate(-1);
        }
        else if (iButtonId == BUTTON_ID_PICK_RANGE_ADD)
        {
            ApplyLootRangeUpdate(1);
        }
        else if (iButtonId == BUTTON_ID_PICK_RANGE_MINUS)
        {
            ApplyLootRangeUpdate(-1);
        }
        else if (iButtonId == BUTTON_ID_ADD_OTHER_ITEM)
        {
            SaveExtraItem();
        }
        else if (iButtonId == BUTTON_ID_DELETE_OTHER_ITEM)
        {
            RemoveExtraItem();
        }
        else if (iButtonId == BUTTON_ID_SKILL2_CONFIG)
        {
            m_CheckBoxList[CHECKBOX_ID_SKILL2_DELAY].box->RegisterBoxState(false);
            m_CheckBoxList[CHECKBOX_ID_SKILL2_CONDITION].box->RegisterBoxState(true);
            ApplyConfigFromCheckbox(CHECKBOX_ID_SKILL2_CONDITION, true);
            g_pNewUIMuHelperExt->Toggle(SUB_PAGE_SKILL2_CONFIG);
        }
        else if (iButtonId == BUTTON_ID_SKILL3_CONFIG)
        {
            m_CheckBoxList[CHECKBOX_ID_SKILL3_DELAY].box->RegisterBoxState(false);
            m_CheckBoxList[CHECKBOX_ID_SKILL3_CONDITION].box->RegisterBoxState(true);
            ApplyConfigFromCheckbox(CHECKBOX_ID_SKILL3_CONDITION, true);
            g_pNewUIMuHelperExt->Toggle(SUB_PAGE_SKILL3_CONFIG);
        }
        else if (iButtonId == BUTTON_ID_POTION_CONFIG_ELF)
        {
            g_pNewUIMuHelperExt->Toggle(SUB_PAGE_POTION_CONFIG_ELF);
        }
        else if (iButtonId == BUTTON_ID_POTION_CONFIG_SUMMY)
        {
            g_pNewUIMuHelperExt->Toggle(SUB_PAGE_POTION_CONFIG_SUMMY);
        }
        else if (iButtonId == BUTTON_ID_POTION_CONFIG)
        {
            g_pNewUIMuHelperExt->Toggle(SUB_PAGE_POTION_CONFIG);
        }
        else if (iButtonId == BUTTON_ID_PARTY_CONFIG)
        {
            g_pNewUIMuHelperExt->Toggle(SUB_PAGE_PARTY_CONFIG);
        }
        else if (iButtonId == BUTTON_ID_PARTY_CONFIG_ELF)
        {
            g_pNewUIMuHelperExt->Toggle(SUB_PAGE_PARTY_CONFIG_ELF);
        }
        else if (iButtonId == BUTTON_ID_EXIT_CONFIG)
        {
            g_pNewUISystem->Hide(INTERFACE_MUHELPER);
            SetFocus(g_hWnd);
        }
        else if (iButtonId == BUTTON_ID_INIT_CONFIG)
        {
            InitConfig();
        }
        else if (iButtonId == BUTTON_ID_SAVE_CONFIG)
        {
            SaveConfig();
            g_pNewUISystem->Hide(INTERFACE_MUHELPER);
            SetFocus(g_hWnd);
        }

        return false;
    }

    int iCheckboxId = UpdateMouseBoxList();
    if (iCheckboxId != -1)
    {
        auto element = m_CheckBoxList[iCheckboxId];
        auto state = element.box->GetBoxState();
        g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Clicked checkbox [%d] state[%d]", iCheckboxId, state);

        if (iCheckboxId == CHECKBOX_ID_SKILL2_DELAY)
        {
            bool bState = m_CheckBoxList[CHECKBOX_ID_SKILL2_DELAY].box->GetBoxState();
            if (bState == true)
            {
                m_CheckBoxList[CHECKBOX_ID_SKILL2_CONDITION].box->RegisterBoxState(false);
                g_pNewUISystem->Hide(INTERFACE_MUHELPER_EXT);
            }
        }
        else if (iCheckboxId == CHECKBOX_ID_SKILL2_CONDITION)
        {
            bool bState = m_CheckBoxList[CHECKBOX_ID_SKILL2_CONDITION].box->GetBoxState();
            if (bState == true)
            {
                m_CheckBoxList[CHECKBOX_ID_SKILL2_DELAY].box->RegisterBoxState(false);
            }
            else
            {
                g_pNewUISystem->Hide(INTERFACE_MUHELPER_EXT);
            }
        }
        else if (iCheckboxId == CHECKBOX_ID_SKILL3_DELAY)
        {
            bool bState = m_CheckBoxList[CHECKBOX_ID_SKILL3_DELAY].box->GetBoxState();
            if (bState == true)
            {
                m_CheckBoxList[CHECKBOX_ID_SKILL3_CONDITION].box->RegisterBoxState(false);
                g_pNewUISystem->Hide(INTERFACE_MUHELPER_EXT);
            }
        }
        else if (iCheckboxId == CHECKBOX_ID_SKILL3_CONDITION)
        {
            bool bState = m_CheckBoxList[CHECKBOX_ID_SKILL3_CONDITION].box->GetBoxState();
            if (bState == true)
            {
                m_CheckBoxList[CHECKBOX_ID_SKILL3_DELAY].box->RegisterBoxState(false);
            }
            else
            {
                g_pNewUISystem->Hide(INTERFACE_MUHELPER_EXT);
            }
        }
        else if (iCheckboxId == CHECKBOX_ID_DR_ATTACK_CEASE)
        {
            m_CheckBoxList[CHECKBOX_ID_DR_ATTACK_CEASE].box->RegisterBoxState(true);
            m_CheckBoxList[CHECKBOX_ID_DR_ATTACK_AUTO].box->RegisterBoxState(false);
            m_CheckBoxList[CHECKBOX_ID_DR_ATTACK_TOGETHER].box->RegisterBoxState(false);
        }
        else if (iCheckboxId == CHECKBOX_ID_DR_ATTACK_AUTO)
        {
            m_CheckBoxList[CHECKBOX_ID_DR_ATTACK_AUTO].box->RegisterBoxState(true);
            m_CheckBoxList[CHECKBOX_ID_DR_ATTACK_CEASE].box->RegisterBoxState(false);
            m_CheckBoxList[CHECKBOX_ID_DR_ATTACK_TOGETHER].box->RegisterBoxState(false);
        }
        else if (iCheckboxId == CHECKBOX_ID_DR_ATTACK_TOGETHER)
        {
            m_CheckBoxList[CHECKBOX_ID_DR_ATTACK_TOGETHER].box->RegisterBoxState(true);
            m_CheckBoxList[CHECKBOX_ID_DR_ATTACK_CEASE].box->RegisterBoxState(false);
            m_CheckBoxList[CHECKBOX_ID_DR_ATTACK_AUTO].box->RegisterBoxState(false);
        }

        ApplyConfigFromCheckbox(iCheckboxId, state);

        return false;
    }

    if (IsRelease(VK_LBUTTON))
    {
        int iPrevIndex = m_iSelectedSkillSlot;
        int iIconIndex = UpdateMouseIconList();

        if (iIconIndex != -1 && iIconIndex < MAX_SKILLS_SLOT)
        {
            g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Clicked skill slot [%d]", iIconIndex);
            m_iSelectedSkillSlot = iIconIndex;

            bool bPrevVisible = g_pNewUISystem->IsVisible(INTERFACE_MUHELPER_SKILL_LIST);

            if (iIconIndex == SKILL_SLOT_SKILL1
                || iIconIndex == SKILL_SLOT_SKILL2
                || iIconIndex == SKILL_SLOT_SKILL3)
            {
                g_pNewUIMuHelperSkillList->FilterByAttackSkills();
            }
            else
            {
                g_pNewUIMuHelperSkillList->FilterByBuffSkills();
            }

            if (iIconIndex == iPrevIndex && bPrevVisible)
            {
                g_pNewUISystem->Hide(INTERFACE_MUHELPER_SKILL_LIST);
            }
            else
            {
                g_pNewUISystem->Show(INTERFACE_MUHELPER_SKILL_LIST);
            }

            return false;
        }
        else if (iIconIndex == TEXTBOX_IMG_DISTANCE_TIME)
        {
            m_DistanceTimeInput.GiveFocus();
        }
        else if (iIconIndex == TEXTBOX_IMG_SKILL1_TIME)
        {
            m_Skill2DelayInput.GiveFocus();
        }
        else if (iIconIndex == TEXTBOX_IMG_SKILL2_TIME)
        {
            m_Skill3DelayInput.GiveFocus();
        }
        else if (iIconIndex == TEXTBOX_IMG_ADD_EXTRA_ITEM)
        {
            m_ItemInput.GiveFocus();
        }
        else
        {
            SetFocus(g_hWnd);
        }

        POINT ptExitBtn = { m_Pos.x + 169, m_Pos.y + 7 };
        if (CheckMouseIn(ptExitBtn.x, ptExitBtn.y, 13, 12))
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_MUHELPER);
        }
    }
    if (IsRelease(VK_RBUTTON))
    {
        int iSlotIndex = UpdateMouseIconList();
        if (iSlotIndex != -1)
        {
            g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Clicked slot slot [%d]", iSlotIndex);
            m_aiSelectedSkills[iSlotIndex] = -1;

            auto cboxCombo = m_CheckBoxList[CHECKBOX_ID_COMBO];
            if (cboxCombo.box->GetBoxState() == true)
            {
                cboxCombo.box->RegisterBoxState(false);
                _TempConfig.bUseCombo = false;
            }

            return false;
        }
    }

    if (m_iCurrentOpenTab == 1)
    {
        m_ItemFilter.DoAction();
    }

    return false;
}

bool CNewUIMuHelper::UpdateKeyEvent()
{
    if (IsVisible())
    {
        if (IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(INTERFACE_MUHELPER);
            g_pNewUISystem->Hide(INTERFACE_MUHELPER_SKILL_LIST);
            //PlayBuffer(SOUND_CLICK01);
            SetFocus(g_hWnd);

            return false;
        }
    }
    return true;
}

void CNewUIMuHelper::ApplyConfigFromCheckbox(int iCheckboxId, bool bState)
{
    switch (iCheckboxId) {
    case CHECKBOX_ID_POTION:
        _TempConfig.bUseHealPotion = bState;
        break;

    case CHECKBOX_ID_LONG_DISTANCE:
        _TempConfig.bLongRangeCounterAttack = bState;
        break;

    case CHECKBOX_ID_ORIG_POSITION:
        _TempConfig.bReturnToOriginalPosition = bState;
        break;

    case CHECKBOX_ID_SKILL2_DELAY:
        _TempConfig.aiSkillCondition[1] &= ~ON_CONDITION;
        _TempConfig.aiSkillCondition[1] = bState
            ? (_TempConfig.aiSkillCondition[1] | ON_TIMER)
            : (_TempConfig.aiSkillCondition[1] & ~ON_TIMER);
        break;

    case CHECKBOX_ID_SKILL2_CONDITION:
        _TempConfig.aiSkillCondition[1] &= ~ON_TIMER;
        _TempConfig.aiSkillCondition[1] = bState
            ? (_TempConfig.aiSkillCondition[1] | ON_CONDITION)
            : (_TempConfig.aiSkillCondition[1] & ~ON_CONDITION);
        break;

    case CHECKBOX_ID_SKILL3_DELAY:
        _TempConfig.aiSkillCondition[2] &= ~ON_CONDITION;
        _TempConfig.aiSkillCondition[2] = bState
            ? (_TempConfig.aiSkillCondition[2] | ON_TIMER)
            : (_TempConfig.aiSkillCondition[2] & ~ON_TIMER);
        break;

    case CHECKBOX_ID_SKILL3_CONDITION:
        _TempConfig.aiSkillCondition[2] &= ~ON_TIMER;
        _TempConfig.aiSkillCondition[2] = bState
            ? (_TempConfig.aiSkillCondition[2] | ON_CONDITION)
            : (_TempConfig.aiSkillCondition[2] & ~ON_CONDITION);
        break;

    case CHECKBOX_ID_COMBO:
        auto cboxCombo = m_CheckBoxList[CHECKBOX_ID_COMBO];

        if (bState == true)
        {
            if (m_aiSelectedSkills[0] <= 0 || m_aiSelectedSkills[1] <= 0 || m_aiSelectedSkills[2] <= 0)
            {
                g_pSystemLogBox->AddText(GlobalText[3565], SEASON3B::TYPE_ERROR_MESSAGE);
                cboxCombo.box->RegisterBoxState(false);
            }
        }
        
        _TempConfig.bUseCombo = cboxCombo.box->GetBoxState();
        break;

    case CHECKBOX_ID_BUFF_DURATION:
        _TempConfig.bBuffDuration = bState;
        break;

    case CHECKBOX_ID_USE_PET:
        _TempConfig.bUseDarkRaven = bState;
        break;

    case CHECKBOX_ID_DR_ATTACK_CEASE:
        _TempConfig.iDarkRavenMode = PET_ATTACK_CEASE;
        break;

    case CHECKBOX_ID_DR_ATTACK_AUTO:
        _TempConfig.iDarkRavenMode = PET_ATTACK_AUTO;
        break;

    case CHECKBOX_ID_DR_ATTACK_TOGETHER:
        _TempConfig.iDarkRavenMode = PET_ATTACK_TOGETHER;
        break;

    case CHECKBOX_ID_PARTY:
        _TempConfig.bSupportParty = bState;
        break;

    case CHECKBOX_ID_AUTO_HEAL:
        _TempConfig.bAutoHeal = bState;
        break;

    case CHECKBOX_ID_DRAIN_LIFE:
        _TempConfig.bUseDrainLife = bState;
        break;

    case CHECKBOX_ID_REPAIR_ITEM:
        _TempConfig.bRepairItem = bState;
        break;

    case CHECKBOX_ID_PICK_ALL:
        auto cboxPickSelected = m_CheckBoxList[CHECKBOX_ID_PICK_SELECTED];
        if (cboxPickSelected.box->GetBoxState())
        {
            cboxPickSelected.box->RegisterBoxState(false);
        }
        _TempConfig.bPickAllItems = bState;
        break;

    case CHECKBOX_ID_PICK_SELECTED:
        auto cboxPickAll = m_CheckBoxList[CHECKBOX_ID_PICK_ALL];
        if (cboxPickAll.box->GetBoxState())
        {
            cboxPickAll.box->RegisterBoxState(false);
        }
        _TempConfig.bPickSelectItems = bState;
        break;

    case CHECKBOX_ID_PICK_JEWEL:
        _TempConfig.bPickJewel = bState;
        break;

    case CHECKBOX_ID_PICK_ANCIENT:
        _TempConfig.bPickAncient = bState;
        break;

    case CHECKBOX_ID_PICK_ZEN:
        _TempConfig.bPickZen = bState;
        break;

    case CHECKBOX_ID_PICK_EXCELLENT:
        _TempConfig.bPickExcellent = bState;
        break;

    case CHECKBOX_ID_ADD_OTHER_ITEM:
        _TempConfig.bPickExtraItems = bState;
        break;

    case CHECKBOX_ID_AUTO_DEFEND:
        _TempConfig.bUseSelfDefense = bState;
        break;

    default:
        break;
    }
}

void CNewUIMuHelper::ApplyConfigFromSkillSlot(int iSlot, int iSkill)
{
    if (iSlot < 3)
    {
        _TempConfig.aiSkill[iSlot] = iSkill;
    }
    else
    {
        _TempConfig.aiBuff[iSlot - SKILL_SLOT_BUFF1] = iSkill;
    }
}

void CNewUIMuHelper::ApplyHuntRangeUpdate(int iDelta)
{
    _TempConfig.iHuntingRange += iDelta;
    if (_TempConfig.iHuntingRange < 0)
    {
        _TempConfig.iHuntingRange = 0;
    }
    if (_TempConfig.iHuntingRange > MAX_HUNTING_RANGE)
    {
        _TempConfig.iHuntingRange = MAX_HUNTING_RANGE;
    }
}

void CNewUIMuHelper::ApplyLootRangeUpdate(int iDelta)
{
    _TempConfig.iObtainingRange += iDelta;
    if (_TempConfig.iObtainingRange < 1)
    {
        _TempConfig.iObtainingRange = 1;
    }
    if (_TempConfig.iObtainingRange > MAX_OBTAINING_RANGE)
    {
        _TempConfig.iObtainingRange = MAX_OBTAINING_RANGE;
    }
}

void CNewUIMuHelper::SaveExtraItem()
{
    wchar_t wsExtraItem[MAX_ITEM_NAME + 1] = { 0 };

    m_ItemInput.GetText(wsExtraItem, sizeof(wsExtraItem));

    if (wcscmp(wsExtraItem, L"") != 0)
    {
        m_ItemFilter.AddText(wsExtraItem);
        m_ItemFilter.Scrolling(-m_ItemFilter.GetBoxSize());

        _TempConfig.aExtraItems.insert(std::wstring(wsExtraItem));
    }

    int iItemIndex = 0;
    for (const auto& item : _TempConfig.aExtraItems)
    {
        g_ConsoleDebug->Write(MCD_NORMAL, L"%s", item.c_str());
    }

    m_ItemInput.SetText(L"");
    SetFocus(g_hWnd);
}

void CNewUIMuHelper::RemoveExtraItem()
{
    FILTERLIST_TEXT* pText = m_ItemFilter.GetSelectedText();
    if (pText)
    {
        _TempConfig.aExtraItems.erase(std::wstring(pText->m_szPattern));
        m_ItemFilter.DeleteText(pText->m_szPattern);
    }
}

int CNewUIMuHelper::GetIntFromTextInput(wchar_t* pwsInput)
{
    wchar_t* end;

    int value = static_cast<int>(wcstol(pwsInput, &end, 10));  // Base 10

    if (*end != L'\0')
    {
        return 0;
    }

    return value;
}

void CNewUIMuHelper::Reset()
{
    _TempConfig.iHuntingRange = 6;

    _TempConfig.iMaxSecondsAway = 10;
    _TempConfig.bLongRangeCounterAttack = false;
    _TempConfig.bReturnToOriginalPosition = true;

    _TempConfig.aiSkill.fill(0);
    _TempConfig.bUseCombo = false;

    _TempConfig.aiSkillInterval.fill(0);

    _TempConfig.aiSkillCondition.fill(0);

    _TempConfig.aiBuff.fill(0);

    _TempConfig.bBuffDuration = true;
    _TempConfig.bBuffDurationParty = true;
    _TempConfig.iBuffCastInterval = 0;

    _TempConfig.bAutoHeal = false;
    _TempConfig.iHealThreshold = 60;
    _TempConfig.bUseDrainLife = false;
    _TempConfig.bUseHealPotion = false;
    _TempConfig.iPotionThreshold = 40;
    _TempConfig.bSupportParty = false;
    _TempConfig.bAutoHealParty = false;
    _TempConfig.iHealPartyThreshold = 60;

    _TempConfig.bUseDarkRaven = false;
    _TempConfig.iDarkRavenMode = PET_ATTACK_CEASE;
    _TempConfig.bRepairItem = false;

    _TempConfig.iObtainingRange = 8;
    _TempConfig.bPickAllItems = false;
    _TempConfig.bPickSelectItems = false;
    _TempConfig.bPickZen = false;
    _TempConfig.bPickJewel = false;
    _TempConfig.bPickExcellent = false;
    _TempConfig.bPickAncient = false;
    _TempConfig.bPickExtraItems = false;
    _TempConfig.aExtraItems.clear();

    ApplyConfig();
}

void CNewUIMuHelper::LoadSavedConfig(const ConfigData& config)
{
    _TempConfig = config;
    ApplyConfig();
}

void CNewUIMuHelper::ApplyConfig()
{
    g_MuHelper.Load(_TempConfig);

    m_aiSelectedSkills[0] = _TempConfig.aiSkill[0] ? _TempConfig.aiSkill[0] : -1;
    m_aiSelectedSkills[1] = _TempConfig.aiSkill[1] ? _TempConfig.aiSkill[1] : -1;
    m_aiSelectedSkills[2] = _TempConfig.aiSkill[2] ? _TempConfig.aiSkill[2] : -1;
    m_aiSelectedSkills[3] = _TempConfig.aiBuff[0] ? _TempConfig.aiBuff[0] : -1;
    m_aiSelectedSkills[4] = _TempConfig.aiBuff[1] ? _TempConfig.aiBuff[1] : -1;
    m_aiSelectedSkills[5] = _TempConfig.aiBuff[2] ? _TempConfig.aiBuff[2] : -1;

    m_CheckBoxList[CHECKBOX_ID_POTION].box->RegisterBoxState(_TempConfig.bUseHealPotion);
    m_CheckBoxList[CHECKBOX_ID_AUTO_HEAL].box->RegisterBoxState(_TempConfig.bAutoHeal);
    m_CheckBoxList[CHECKBOX_ID_DRAIN_LIFE].box->RegisterBoxState(_TempConfig.bUseDrainLife);
    m_CheckBoxList[CHECKBOX_ID_LONG_DISTANCE].box->RegisterBoxState(_TempConfig.bLongRangeCounterAttack);
    m_CheckBoxList[CHECKBOX_ID_ORIG_POSITION].box->RegisterBoxState(_TempConfig.bReturnToOriginalPosition);

    m_CheckBoxList[CHECKBOX_ID_SKILL2_DELAY].box->RegisterBoxState(_TempConfig.aiSkillCondition[1] & ON_TIMER);
    m_CheckBoxList[CHECKBOX_ID_SKILL2_CONDITION].box->RegisterBoxState(_TempConfig.aiSkillCondition[1] & ON_CONDITION);
    m_CheckBoxList[CHECKBOX_ID_SKILL3_DELAY].box->RegisterBoxState(_TempConfig.aiSkillCondition[2] & ON_TIMER);
    m_CheckBoxList[CHECKBOX_ID_SKILL3_CONDITION].box->RegisterBoxState(_TempConfig.aiSkillCondition[2] & ON_CONDITION);
    m_CheckBoxList[CHECKBOX_ID_COMBO].box->RegisterBoxState(_TempConfig.bUseCombo);

    wchar_t wsTempNum[MAX_NUMBER_DIGITS + 1];
    memset(wsTempNum, 0, sizeof(wsTempNum));
    swprintf(wsTempNum, MAX_NUMBER_DIGITS + 1, L"%d", _TempConfig.iMaxSecondsAway);
    m_DistanceTimeInput.SetText(wsTempNum);

    memset(wsTempNum, 0, sizeof(wsTempNum));
    swprintf(wsTempNum, MAX_NUMBER_DIGITS + 1, L"%d", _TempConfig.aiSkillInterval[1]);
    m_Skill2DelayInput.SetText(wsTempNum);

    memset(wsTempNum, 0, sizeof(wsTempNum));
    swprintf(wsTempNum, MAX_NUMBER_DIGITS + 1, L"%d", _TempConfig.aiSkillInterval[2]);
    m_Skill3DelayInput.SetText(wsTempNum);

    m_CheckBoxList[CHECKBOX_ID_BUFF_DURATION].box->RegisterBoxState(_TempConfig.bBuffDuration);
    m_CheckBoxList[CHECKBOX_ID_PARTY].box->RegisterBoxState(_TempConfig.bSupportParty);

    m_CheckBoxList[CHECKBOX_ID_USE_PET].box->RegisterBoxState(_TempConfig.bUseDarkRaven);
    m_CheckBoxList[CHECKBOX_ID_DR_ATTACK_CEASE].box->RegisterBoxState(_TempConfig.iDarkRavenMode == PET_ATTACK_CEASE);
    m_CheckBoxList[CHECKBOX_ID_DR_ATTACK_AUTO].box->RegisterBoxState(_TempConfig.iDarkRavenMode == PET_ATTACK_AUTO);
    m_CheckBoxList[CHECKBOX_ID_DR_ATTACK_TOGETHER].box->RegisterBoxState(_TempConfig.iDarkRavenMode == PET_ATTACK_TOGETHER);

    m_CheckBoxList[CHECKBOX_ID_REPAIR_ITEM].box->RegisterBoxState(_TempConfig.bRepairItem);
    m_CheckBoxList[CHECKBOX_ID_PICK_ALL].box->RegisterBoxState(_TempConfig.bPickAllItems);
    m_CheckBoxList[CHECKBOX_ID_PICK_SELECTED].box->RegisterBoxState(_TempConfig.bPickSelectItems);
    m_CheckBoxList[CHECKBOX_ID_PICK_JEWEL].box->RegisterBoxState(_TempConfig.bPickJewel);
    m_CheckBoxList[CHECKBOX_ID_PICK_ZEN].box->RegisterBoxState(_TempConfig.bPickZen);
    m_CheckBoxList[CHECKBOX_ID_PICK_EXCELLENT].box->RegisterBoxState(_TempConfig.bPickExcellent);
    m_CheckBoxList[CHECKBOX_ID_PICK_ANCIENT].box->RegisterBoxState(_TempConfig.bPickAncient);
    m_CheckBoxList[CHECKBOX_ID_ADD_OTHER_ITEM].box->RegisterBoxState(_TempConfig.bPickExtraItems);

    m_CheckBoxList[CHECKBOX_ID_AUTO_ACCEPT_FRIEND].box->RegisterBoxState(_TempConfig.bAutoAcceptFriend);
    m_CheckBoxList[CHECKBOX_ID_AUTO_ACCEPT_GUILD].box->RegisterBoxState(_TempConfig.bAutoAcceptGuild);
    m_CheckBoxList[CHECKBOX_ID_AUTO_DEFEND].box->RegisterBoxState(_TempConfig.bUseSelfDefense);

    m_ItemFilter.Clear();
    for (const auto& item : _TempConfig.aExtraItems)
    {
        m_ItemFilter.AddText(item.c_str());
    }
}

void CNewUIMuHelper::InitConfig()
{
    Reset();

    g_pNewUIMuHelperExt->InitConfig();
}

void CNewUIMuHelper::SaveConfig()
{
    wchar_t wsNumberInput[MAX_NUMBER_DIGITS + 1]{};

    m_DistanceTimeInput.GetText(wsNumberInput, sizeof(wsNumberInput));
    _TempConfig.iMaxSecondsAway = GetIntFromTextInput(wsNumberInput);

    m_Skill2DelayInput.GetText(wsNumberInput, sizeof(wsNumberInput));
    _TempConfig.aiSkillInterval[1] = GetIntFromTextInput(wsNumberInput);

    m_Skill3DelayInput.GetText(wsNumberInput, sizeof(wsNumberInput));
    _TempConfig.aiSkillInterval[2] = GetIntFromTextInput(wsNumberInput);

    _TempConfig.aiSkill[0] = m_aiSelectedSkills[0] > 0 ? m_aiSelectedSkills[0] : 0;
    _TempConfig.aiSkill[1] = m_aiSelectedSkills[1] > 0 ? m_aiSelectedSkills[1] : 0;
    _TempConfig.aiSkill[2] = m_aiSelectedSkills[2] > 0 ? m_aiSelectedSkills[2] : 0;
    _TempConfig.aiBuff[0] = m_aiSelectedSkills[3] > 0 ? m_aiSelectedSkills[3] : 0;
    _TempConfig.aiBuff[1] = m_aiSelectedSkills[4] > 0 ? m_aiSelectedSkills[4] : 0;
    _TempConfig.aiBuff[2] = m_aiSelectedSkills[5] > 0 ? m_aiSelectedSkills[5] : 0;

    g_MuHelper.Save(_TempConfig);
}

float CNewUIMuHelper::GetLayerDepth()
{
    return 3.4;
}

float CNewUIMuHelper::GetKeyEventOrder()
{
    return 3.4;
}

void CNewUIMuHelper::Show(bool bShow)
{
    CNewUIObj::Show(bShow);

    if (bShow == false)
    {
        if (g_pNewUIMuHelperExt)
            g_pNewUIMuHelperExt->Show(false);

        if (g_pNewUIMuHelperSkillList)
            g_pNewUIMuHelperSkillList->Show(false);
    }

    SetFocus(g_hWnd);
}

bool CNewUIMuHelper::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    DWORD TextColor = g_pRenderText->GetTextColor();

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(0xFFFFFFFF);
    g_pRenderText->SetBgColor(0);

    RenderImage(IMAGE_BASE_WINDOW_BACK, m_Pos.x, m_Pos.y, float(WINDOW_WIDTH), float(WINDOW_HEIGHT));
    RenderImage(IMAGE_BASE_WINDOW_TOP, m_Pos.x, m_Pos.y, float(WINDOW_WIDTH), 64.f);
    RenderImage(IMAGE_BASE_WINDOW_LEFT, m_Pos.x, m_Pos.y + 64.f, 21.f, float(WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_BASE_WINDOW_RIGHT, m_Pos.x + float(WINDOW_WIDTH) - 21.f, m_Pos.y + 64.f, 21.f, float(WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_BASE_WINDOW_BOTTOM, m_Pos.x, m_Pos.y + float(WINDOW_HEIGHT) - 45.f, float(WINDOW_WIDTH), 45.f);

    g_pRenderText->SetFont(g_hFontBold);

    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 13, GlobalText[3536], 190, 0, RT3_SORT_CENTER);

    RenderBack(m_Pos.x + 12, m_Pos.y + 340, 165, 46);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + 347, GlobalText[3537], 0, 0, RT3_SORT_CENTER);

    g_pRenderText->SetTextColor(0xFF00B4FF);
    g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + 365, GlobalText[3538], 0, 0, RT3_SORT_CENTER);

    g_pRenderText->SetTextColor(TextColor);

    m_TabBtn.Render();

    if (m_iCurrentOpenTab == 1)
    {
        RenderBack(m_Pos.x + 12, m_Pos.y + 73, 68, 50);
        RenderBack(m_Pos.x + 75, m_Pos.y + 73, 102, 50);
        RenderBack(m_Pos.x + 12, m_Pos.y + 120, 165, 30);
        RenderBack(m_Pos.x + 12, m_Pos.y + 147, 165, 195);
        RenderBack(m_Pos.x + 16, m_Pos.y + 235, 158, 75);

        RenderImage(BITMAP_DISTANCE_BEGIN + _TempConfig.iObtainingRange, m_Pos.x + 29, m_Pos.y + 92, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f);

        m_ItemFilter.Render();
    }
    else if (m_iCurrentOpenTab == 2)
    {
        RenderBack(m_Pos.x + 12, m_Pos.y + 73, 165, 50);
        RenderBack(m_Pos.x + 12, m_Pos.y + 120, 165, 222);
    }
    else
    {
        RenderBack(m_Pos.x + 12, m_Pos.y + 73, 68, 50);
        RenderBack(m_Pos.x + 75, m_Pos.y + 73, 102, 50);
        RenderBack(m_Pos.x + 12, m_Pos.y + 120, 165, 39);
        RenderBack(m_Pos.x + 12, m_Pos.y + 156, 165, 120);
        RenderBack(m_Pos.x + 12, m_Pos.y + 273, 165, 69);

        RenderImage(BITMAP_DISTANCE_BEGIN + _TempConfig.iHuntingRange, m_Pos.x + 29, m_Pos.y + 92, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f);
    }

    RenderBoxList();
    RenderIconList();
    RenderTextList();
    RenderBtnList();

    if (m_iCurrentOpenTab == 0)
    {
        m_DistanceTimeInput.Render();
        m_Skill2DelayInput.Render();

        if (gCharacterManager.GetBaseClass(Hero->Class) != CLASS_DARK_LORD)
        {
            m_Skill3DelayInput.Render();
        }
    }
    else if (m_iCurrentOpenTab == 1)
    {
        m_ItemInput.Render();
    }

    DisableAlphaBlend();

    return true;
}

void CNewUIMuHelper::RenderBack(int x, int y, int width, int height)
{
    EnableAlphaTest();
    glColor4f(0.0, 0.0, 0.0, 0.4f);
    RenderColor(x + 3.f, y + 2.f, width - 7.f, height - 7, 0.0, 0);
    EndRenderColor();

    RenderImage(IMAGE_TABLE_TOP_LEFT, x, y, 14.0, 14.0);
    RenderImage(IMAGE_TABLE_TOP_RIGHT, (x + width) - 14.f, y, 14.0, 14.0);
    RenderImage(IMAGE_TABLE_BOTTOM_LEFT, x, (y + height) - 14.f, 14.0, 14.0);
    RenderImage(IMAGE_TABLE_BOTTOM_RIGHT, (x + width) - 14.f, (y + height) - 14.f, 14.0, 14.0);
    RenderImage(IMAGE_TABLE_TOP_PIXEL, x + 6.f, y, (width - 12.f), 14.0);
    RenderImage(IMAGE_TABLE_RIGHT_PIXEL, (x + width) - 14.f, y + 6.f, 14.0, (height - 14.f));
    RenderImage(IMAGE_TABLE_BOTTOM_PIXEL, x + 6.f, (y + height) - 14.f, (width - 12.f), 14.0);
    RenderImage(IMAGE_TABLE_LEFT_PIXEL, x, (y + 6.f), 14.0, (height - 14.f));
}

void CNewUIMuHelper::LoadImages()
{
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_RangeMinus.tga", IMAGE_MACROUI_HELPER_RAGEMINUS, GL_LINEAR, GL_CLAMP, 1, 0);
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_OptionButton.tga", IMAGE_MACROUI_HELPER_OPTIONBUTTON, GL_LINEAR, GL_CLAMP, 1, 0);
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_InputNumber.tga", IMAGE_MACROUI_HELPER_INPUTNUMBER, GL_LINEAR, GL_CLAMP, 1, 0);
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_InputString.tga", IMAGE_MACROUI_HELPER_INPUTSTRING, GL_LINEAR, GL_CLAMP, 1, 0);
    //--
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Bt03.tga", IMAGE_IGS_BUTTON, GL_LINEAR, GL_CLAMP, 1, 0);
}

void CNewUIMuHelper::UnloadImages()
{
    DeleteBitmap(IMAGE_MACROUI_HELPER_RAGEMINUS);
    DeleteBitmap(IMAGE_MACROUI_HELPER_OPTIONBUTTON);
    DeleteBitmap(IMAGE_MACROUI_HELPER_INPUTNUMBER);
    DeleteBitmap(IMAGE_MACROUI_HELPER_INPUTSTRING);
    //--
    DeleteBitmap(IMAGE_IGS_BUTTON);
}

//===============================================================================================================
//===============================================================================================================

void CNewUIMuHelper::RegisterButton(int Identifier, CButtonTap button)
{
    m_ButtonList.insert(std::pair<int, CButtonTap>(Identifier, button));
}

void CNewUIMuHelper::RegisterBtnCharacter(BYTE class_character, int Identifier)
{
    auto li = m_ButtonList.find(Identifier);

    if (li != m_ButtonList.end())
    {
        CButtonTap* cBTN = &li->second;
        if (class_character >= 0 && class_character < MAX_CLASS)
        {
            cBTN->class_character[class_character] = TRUE;
        }
        else
        {
            memset(cBTN->class_character, 1, sizeof(cBTN->class_character));
        }
    }
}

void CNewUIMuHelper::InsertButton(int imgindex, int x, int y, int sx, int sy, bool overflg, bool isimgwidth, bool bClickEffect, bool MoveTxt, std::wstring btname, std::wstring tooltiptext, int Identifier, int iNumTab)
{
    CButtonTap cBTN;
    auto* button = new CNewUIButton();

    button->ChangeButtonImgState(1, imgindex, overflg, isimgwidth, bClickEffect);
    button->ChangeButtonInfo(x, y, sx, sy);

    button->ChangeText(btname);
    button->ChangeToolTipText(tooltiptext, TRUE);

    if (MoveTxt)
    {
        button->MoveTextPos(0, -1);
    }

    cBTN.btn = button;
    cBTN.iNumTab = iNumTab;
    memset(cBTN.class_character, 0, sizeof(cBTN.class_character));

    RegisterButton(Identifier, cBTN);
}

void CNewUIMuHelper::RenderBtnList()
{
    auto li = m_ButtonList.begin();

    for (; li != m_ButtonList.end(); li++)
    {
        CButtonTap* cBTN = &li->second;

        if ((cBTN->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cBTN->iNumTab == m_iCurrentOpenTab || cBTN->iNumTab == -1))
        {
            cBTN->btn->Render();
        }
    }
}

int CNewUIMuHelper::UpdateMouseBtnList()
{
    auto li = m_ButtonList.begin();

    for (; li != m_ButtonList.end(); li++)
    {
        CButtonTap* cBTN = &li->second;

        if ((cBTN->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cBTN->iNumTab == m_iCurrentOpenTab || cBTN->iNumTab == -1))
        {
            if (cBTN->btn->UpdateMouseEvent())
            {
                return li->first;
            }
        }
    }
    return -1;
}

//===============================================================================================================
//===============================================================================================================

void CNewUIMuHelper::RegisterBoxCharacter(BYTE class_character, int Identifier)
{
    auto li = m_CheckBoxList.find(Identifier);

    if (li != m_CheckBoxList.end())
    {
        CheckBoxTap* cBOX = &li->second;

        if (class_character >= 0 && class_character < MAX_CLASS)
        {
            cBOX->class_character[class_character] = TRUE;
        }
        else
        {
            memset(cBOX->class_character, 1, sizeof(cBOX->class_character));
        }
    }
}

void CNewUIMuHelper::RegisterCheckBox(int Identifier, CheckBoxTap button)
{
    m_CheckBoxList.insert(std::pair<int, CheckBoxTap>(Identifier, button));
}

void CNewUIMuHelper::InsertCheckBox(int imgindex, int x, int y, int sx, int sy, bool overflg, std::wstring btname, int Identifier, int iNumTab)
{
    CheckBoxTap cBOX;

    auto* cbox = new CNewUICheckBox;

    cbox->CheckBoxImgState(imgindex);
    cbox->CheckBoxInfo(x, y, sx, sy);

    cbox->ChangeText(btname);
    cbox->RegisterBoxState(overflg);

    cBOX.box = cbox;
    cBOX.iNumTab = iNumTab;
    memset(cBOX.class_character, 0, sizeof(cBOX.class_character));

    RegisterCheckBox(Identifier, cBOX);
}

void CNewUIMuHelper::RenderBoxList()
{
    auto li = m_CheckBoxList.begin();

    for (; li != m_CheckBoxList.end(); li++)
    {
        CheckBoxTap* cBOX = &li->second;

        if ((cBOX->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cBOX->iNumTab == m_iCurrentOpenTab || cBOX->iNumTab == -1))
        {
            cBOX->box->Render();
        }
    }
}

int CNewUIMuHelper::UpdateMouseBoxList()
{
    auto li = m_CheckBoxList.begin();

    for (; li != m_CheckBoxList.end(); li++)
    {
        CheckBoxTap* cBOX = &li->second;

        if ((cBOX->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cBOX->iNumTab == m_iCurrentOpenTab || cBOX->iNumTab == -1))
        {
            if (cBOX->box->UpdateMouseEvent())
            {
                return li->first;
            }
        }
    }
    return -1;
}

//===============================================================================================================
//===============================================================================================================

void CNewUIMuHelper::RenderIconList()
{
    auto li = m_IconList.begin();

    for (; li != m_IconList.end(); li++)
    {
        cTexture* cImage = &li->second;

        if ((cImage->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cImage->iNumTab == m_iCurrentOpenTab || cImage->iNumTab == -1))
        {
            RenderImage(cImage->s_ImgIndex, cImage->m_Pos.x, cImage->m_Pos.y, cImage->m_Size.x, cImage->m_Size.y);

            if (li->first < MAX_SKILLS_SLOT)
            {
                if (m_aiSelectedSkills[li->first] >= 0 && m_aiSelectedSkills[li->first] < MAX_SKILLS)
                {
                    RenderSkillIcon(m_aiSelectedSkills[li->first], cImage->m_Pos.x + 6, cImage->m_Pos.y + 6, 20, 28);
                }
            }
        }
    }
}

int CNewUIMuHelper::UpdateMouseIconList()
{
    auto li = m_IconList.begin();

    for (; li != m_IconList.end(); li++)
    {
        cTexture* cImage = &li->second;

        if ((cImage->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cImage->iNumTab == m_iCurrentOpenTab || cImage->iNumTab == -1))
        {
            if (CheckMouseIn(cImage->m_Pos.x, cImage->m_Pos.y, cImage->m_Size.x, cImage->m_Size.y))
            {
                return li->first;
            }
        }
    }

    return -1;
}

void CNewUIMuHelper::RegisterIconCharacter(BYTE class_character, int Identifier)
{
    auto li = m_IconList.find(Identifier);

    if (li != m_IconList.end())
    {
        cTexture* cImage = &li->second;

        if (class_character >= 0 && class_character < MAX_CLASS)
        {
            cImage->class_character[class_character] = TRUE;
        }
        else
        {
            memset(cImage->class_character, 1, sizeof(cImage->class_character));
        }
    }
}

void CNewUIMuHelper::RegisterIcon(int Identifier, cTexture button)
{
    m_IconList.insert(std::pair<int, cTexture>(Identifier, button));
}

void CNewUIMuHelper::InsertIcon(int imgindex, int x, int y, int sx, int sy, int Identifier, int iNumTab)
{
    cTexture cImage;

    cImage.s_ImgIndex = imgindex;
    cImage.m_Pos.x = x;
    cImage.m_Pos.y = y;
    cImage.m_Size.x = sx;
    cImage.m_Size.y = sy;
    cImage.iNumTab = iNumTab;

    memset(cImage.class_character, 0, sizeof(cImage.class_character));

    RegisterIcon(Identifier, cImage);
}

//===============================================================================================================
//===============================================================================================================

void CNewUIMuHelper::RenderTextList()
{
    auto li = m_TextNameList.begin();

    for (; li != m_TextNameList.end(); li++)
    {
        cTextName* cImage = &li->second;

        if ((cImage->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cImage->iNumTab == m_iCurrentOpenTab || cImage->iNumTab == -1))
        {
            g_pRenderText->RenderText(cImage->m_Pos.x, cImage->m_Pos.y, cImage->m_Name.c_str());
        }
    }
}

void CNewUIMuHelper::RegisterTextCharacter(BYTE class_character, int Identifier)
{
    auto li = m_TextNameList.find(Identifier);

    if (li != m_TextNameList.end())
    {
        cTextName* cImage = &li->second;

        if (class_character >= 0 && class_character < MAX_CLASS)
        {
            cImage->class_character[class_character] = TRUE;
        }
        else
        {
            memset(cImage->class_character, 1, sizeof(cImage->class_character));
        }
    }
}

void CNewUIMuHelper::RegisterText(int Identifier, cTextName button)
{
    m_TextNameList.insert(std::pair<int, cTextName>(Identifier, button));
}

void CNewUIMuHelper::InsertText(int x, int y, std::wstring Name, int Identifier, int iNumTab)
{
    cTextName cText;

    cText.m_Pos.x = x;
    cText.m_Pos.y = y;
    cText.m_Name = Name;
    cText.iNumTab = iNumTab;

    memset(cText.class_character, 0, sizeof(cText.class_character));
    RegisterText(Identifier, cText);
}

void CNewUIMuHelper::AssignSkill(int iSkill)
{
    if (m_iSelectedSkillSlot != -1 && m_iSelectedSkillSlot < MAX_SKILLS_SLOT)
    {
        if (!IsSkillAssigned(iSkill))
        {
            m_aiSelectedSkills[m_iSelectedSkillSlot] = iSkill;
            ApplyConfigFromSkillSlot(m_iSelectedSkillSlot, iSkill);

            g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Assign m_aiSelectedSkills[%d] = %d", m_iSelectedSkillSlot, iSkill);
        }
        else
        {
            int iPrevIndex = GetSkillIndex(iSkill);
            m_aiSelectedSkills[iPrevIndex] = -1;
            m_aiSelectedSkills[m_iSelectedSkillSlot] = iSkill;

            auto cboxCombo = m_CheckBoxList[CHECKBOX_ID_COMBO];
            if (cboxCombo.box->GetBoxState() == true)
            {
                cboxCombo.box->RegisterBoxState(false);
                _TempConfig.bUseCombo = false;
            }
        }
    }
}

bool CNewUIMuHelper::IsSkillAssigned(int iSkill)
{
    return std::find(m_aiSelectedSkills.begin(), m_aiSelectedSkills.end(), iSkill) != m_aiSelectedSkills.end();
}

int CNewUIMuHelper::GetSkillIndex(int iSkill)
{
    auto it = std::find(m_aiSelectedSkills.begin(), m_aiSelectedSkills.end(), iSkill);

    if (it != m_aiSelectedSkills.end()) {
        return std::distance(m_aiSelectedSkills.begin(), it);
    }

    return -1;
}

void CNewUIMuHelper::RenderSkillIcon(int skill, float x, float y, float width, float height)
{
    float fU, fV;
    int iKindofSkill = 0;

    BYTE bySkillUseType = SkillAttribute[skill].SkillUseType;
    int Skill_Icon = SkillAttribute[skill].Magic_Icon;

    if (skill >= AT_PET_COMMAND_DEFAULT && skill <= AT_PET_COMMAND_END)
    {
        fU = ((skill - AT_PET_COMMAND_DEFAULT) % 8) * width / 256.f;
        fV = ((skill - AT_PET_COMMAND_DEFAULT) / 8) * height / 256.f;
        iKindofSkill = KOS_COMMAND;
    }
    else if (skill == AT_SKILL_PLASMA_STORM_FENRIR)
    {
        fU = 4 * width / 256.f;
        fV = 0.f;
        iKindofSkill = KOS_COMMAND;
    }
    else if ((skill >= AT_SKILL_ALICE_DRAINLIFE && skill <= AT_SKILL_ALICE_THORNS))
    {
        fU = ((skill - AT_SKILL_ALICE_DRAINLIFE) % 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (skill >= AT_SKILL_ALICE_SLEEP && skill <= AT_SKILL_ALICE_BLIND)
    {
        fU = ((skill - AT_SKILL_ALICE_SLEEP + 4) % 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (skill == AT_SKILL_ALICE_BERSERKER || skill == AT_SKILL_ALICE_BERSERKER_STR)
    {
        fU = 10 * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (skill >= AT_SKILL_ALICE_WEAKNESS && skill <= AT_SKILL_ALICE_ENERVATION)
    {
        fU = (skill - AT_SKILL_ALICE_WEAKNESS + 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (skill >= AT_SKILL_SUMMON_EXPLOSION && skill <= AT_SKILL_SUMMON_REQUIEM)
    {
        fU = ((skill - AT_SKILL_SUMMON_EXPLOSION + 6) % 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (skill == AT_SKILL_SUMMON_POLLUTION)
    {
        fU = 11 * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (skill == AT_SKILL_STRIKE_OF_DESTRUCTION)
    {
        fU = 7 * width / 256.f;
        fV = 2 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (skill == AT_SKILL_GAOTIC)
    {
        fU = 3 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (skill == AT_SKILL_RECOVER)
    {
        fU = 9 * width / 256.f;
        fV = 2 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (skill == AT_SKILL_MULTI_SHOT)
    {
        fU = 0 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (skill == AT_SKILL_FLAME_STRIKE)
    {
        int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
        int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

        fU = 1 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (skill == AT_SKILL_GIGANTIC_STORM)
    {
        fU = 2 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (skill == AT_SKILL_LIGHTNING_SHOCK)
    {
        fU = 2 * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (skill == AT_SKILL_LIGHTNING_SHOCK_STR)
    {
        fU = 6 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (skill == AT_SKILL_EXPANSION_OF_WIZARDRY)
    {
        fU = 8 * width / 256.f;
        fV = 2 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillUseType == 4)
    {
        fU = (width / 256.f) * (Skill_Icon % 12);
        fV = (height / 256.f) * ((Skill_Icon / 12) + 4);
        iKindofSkill = KOS_SKILL2;
    }
    else if (skill >= AT_SKILL_KILLING_BLOW)
    {
        fU = ((skill - AT_SKILL_KILLING_BLOW) % 12) * width / 256.f;
        fV = ((skill - AT_SKILL_KILLING_BLOW) / 12) * height / 256.f;
        iKindofSkill = KOS_SKILL3;
    }
    else if (skill >= AT_SKILL_SPIRAL_SLASH)
    {
        fU = ((skill - AT_SKILL_SPIRAL_SLASH) % 8) * width / 256.f;
        fV = ((skill - AT_SKILL_SPIRAL_SLASH) / 8) * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else
    {
        fU = ((skill - 1) % 8) * width / 256.f;
        fV = ((skill - 1) / 8) * height / 256.f;
        iKindofSkill = KOS_SKILL1;
    }
    int iTextureIndex = 0;
    switch (iKindofSkill)
    {
    case KOS_COMMAND:
    {
        iTextureIndex = IMAGE_COMMAND;
    }break;
    case KOS_SKILL1:
    {
        iTextureIndex = IMAGE_SKILL1;
    }break;
    case KOS_SKILL2:
    {
        iTextureIndex = IMAGE_SKILL2;
    }break;
    case KOS_SKILL3:
    {
        iTextureIndex = IMAGE_SKILL3;
    }break;
    }

    if (skill >= AT_SKILL_MASTER_BEGIN)
    {
        RenderImage(BITMAP_INTERFACE_MASTER_BEGIN + 2, x, y, width, height, (20.f / 512.f) * (Skill_Icon % 25), ((28.f / 512.f) * ((Skill_Icon / 25))), 20.f / 512.f, 28.f / 512.f);
    }
    else if (iTextureIndex != 0)
    {
        RenderBitmap(iTextureIndex, x, y, width, height, fU, fV, width / 256.f, height / 256.f);
    }
}

CNewUIMuHelperSkillList::CNewUIMuHelperSkillList()
{
    m_pNewUIMng = NULL;
    Reset();
}

CNewUIMuHelperSkillList::~CNewUIMuHelperSkillList()
{
    Release();
}


bool CNewUIMuHelperSkillList::Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(INTERFACE_MUHELPER_SKILL_LIST, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;

    LoadImages();

    Show(false);

    return true;
}

void CNewUIMuHelperSkillList::Release()
{
    if (m_pNewUI3DRenderMng)
    {
        m_pNewUI3DRenderMng->DeleteUI2DEffectObject(UI2DEffectCallback);
    }

    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNewUIMuHelperSkillList::Reset()
{
    m_bRenderSkillInfo = false;
    m_iRenderSkillInfoType = 0;
    m_iRenderSkillInfoPosX = 0;
    m_iRenderSkillInfoPosY = 0;

    m_EventState = EVENT_NONE;
}

void CNewUIMuHelperSkillList::LoadImages()
{
    LoadBitmap(L"Interface\\newui_skill.jpg", IMAGE_SKILL1, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skill2.jpg", IMAGE_SKILL2, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_command.jpg", IMAGE_COMMAND, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skillbox.jpg", IMAGE_SKILLBOX, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skillbox2.jpg", IMAGE_SKILLBOX_USE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_skill.jpg", IMAGE_NON_SKILL1, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_skill2.jpg", IMAGE_NON_SKILL2, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_command.jpg", IMAGE_NON_COMMAND, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skill3.jpg", IMAGE_SKILL3, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_skill3.jpg", IMAGE_NON_SKILL3, GL_LINEAR);
}

void CNewUIMuHelperSkillList::UnloadImages()
{
    DeleteBitmap(IMAGE_SKILL1);
    DeleteBitmap(IMAGE_SKILL2);
    DeleteBitmap(IMAGE_COMMAND);
    DeleteBitmap(IMAGE_SKILLBOX);
    DeleteBitmap(IMAGE_SKILLBOX_USE);
    DeleteBitmap(IMAGE_NON_SKILL1);
    DeleteBitmap(IMAGE_NON_SKILL2);
    DeleteBitmap(IMAGE_NON_COMMAND);
    DeleteBitmap(IMAGE_SKILL3);
    DeleteBitmap(IMAGE_NON_SKILL3);
}

bool CNewUIMuHelperSkillList::UpdateMouseEvent()
{
    if (IsRelease(VK_LBUTTON))
    {
        int skillId = UpdateMouseSkillList();
        if (skillId != -1)
        {
            g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Clicked skill [%d]", skillId);
            g_pNewUIMuHelper->AssignSkill(skillId);
            Show(false);
            return false;
        }

        return true;
    }

    return false;
}

bool CNewUIMuHelperSkillList::UpdateKeyEvent()
{
    if (IsVisible())
    {
        if (IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(INTERFACE_MUHELPER_SKILL_LIST);
            SetFocus(g_hWnd);
            //PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }
    return true;
}

void CNewUIMuHelperSkillList::PrepareSkillsToRender()
{
    m_aiSkillsToRender.clear();
    m_skillIconMap.clear();

    BYTE skillNumber = CharacterAttribute->SkillNumber;
    if (skillNumber > 0)
    {
        for (int i = 0; i < MAX_MAGIC; ++i)
        {
            int iSkillType = CharacterAttribute->Skill[i];

            if (iSkillType != 0 && (iSkillType < AT_SKILL_STUN || iSkillType > AT_SKILL_REMOVAL_BUFF))
            {
                BYTE bySkillUseType = SkillAttribute[iSkillType].SkillUseType;

                if (bySkillUseType == SKILL_USE_TYPE_MASTER || bySkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
                {
                    continue;
                }

                if ((m_bFilterByAttackSkills && IsAttackSkill(iSkillType))
                    || (m_bFilterByBuffSkills && IsBuffSkill(iSkillType)))
                {
                    m_aiSkillsToRender.push_back(iSkillType);
                }
            }
        }
    }
}

bool CNewUIMuHelperSkillList::Update()
{
    return true;
}

bool CNewUIMuHelperSkillList::Render()
{
    float scale = 1.0f; // 
    float boxWidth = 32.f * scale;
    float boxHeight = 38.f * scale;
    float iconWidth = 20.f * scale;
    float iconHeight = 28.f * scale;
    float iconOffsetX = (boxWidth - iconWidth) / 2.f;
    float iconOffsetY = (boxHeight - iconHeight) / 2.f;

    // x position relative to the position of mu helper window
    float startX = 640.f - 190.f - 32.f;
    // y position relative to the skill/buff selection in mu helper window
    float startY = m_bFilterByAttackSkills ? 171.f : 293.f;

    int itemsPerColumn = m_bFilterByAttackSkills ? 10 : 5;

    for (int i = 0; i < m_aiSkillsToRender.size(); i++)
    {
        int iSkillType = m_aiSkillsToRender[i];

        int col = i / itemsPerColumn;
        int rowInColumn = i % itemsPerColumn;

        int offset = (rowInColumn + 1) / 2;
        bool skillCountEven = (rowInColumn % 2 == 0);

        float x = startX - col * boxWidth; // left to right
        float y = skillCountEven           // bounce up and down from center
            ? startY - offset * boxHeight
            : startY + offset * boxHeight;

        RenderImage(IMAGE_SKILLBOX, x, y, boxWidth, boxHeight);
        RenderSkillIcon(iSkillType, x + iconOffsetX, y + iconOffsetY, iconWidth, iconHeight);

        m_skillIconMap.insert_or_assign(iSkillType, cSkillIcon{
            iSkillType,
            { static_cast<LONG>(x), static_cast<LONG>(y) },
            { static_cast<LONG>(boxWidth), static_cast<LONG>(boxHeight) }
            });
    }

    if (m_bRenderSkillInfo && m_pNewUI3DRenderMng)
    {
        m_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, 0, 0);
        m_bRenderSkillInfo = false;
    }

    return true;
}

void CNewUIMuHelperSkillList::RenderSkillInfo()
{
    ::RenderSkillInfo(m_iRenderSkillInfoPosX + 15, m_iRenderSkillInfoPosY - 10, m_iRenderSkillInfoType);
}

float CNewUIMuHelperSkillList::GetLayerDepth()
{
    return 5.2f;
}

void CNewUIMuHelperSkillList::RenderSkillIcon(int iSkillType, float x, float y, float width, float height)
{
    float fU, fV;
    int iKindofSkill = 0;

    BYTE bySkillUseType = SkillAttribute[iSkillType].SkillUseType;
    int Skill_Icon = SkillAttribute[iSkillType].Magic_Icon;

    if (iSkillType >= AT_PET_COMMAND_DEFAULT && iSkillType <= AT_PET_COMMAND_END)
    {
        fU = ((iSkillType - AT_PET_COMMAND_DEFAULT) % 8) * width / 256.f;
        fV = ((iSkillType - AT_PET_COMMAND_DEFAULT) / 8) * height / 256.f;
        iKindofSkill = KOS_COMMAND;
    }
    else if (iSkillType == AT_SKILL_PLASMA_STORM_FENRIR)
    {
        fU = 4 * width / 256.f;
        fV = 0.f;
        iKindofSkill = KOS_COMMAND;
    }
    else if ((iSkillType >= AT_SKILL_ALICE_DRAINLIFE && iSkillType <= AT_SKILL_ALICE_THORNS))
    {
        fU = ((iSkillType - AT_SKILL_ALICE_DRAINLIFE) % 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (iSkillType >= AT_SKILL_ALICE_SLEEP && iSkillType <= AT_SKILL_ALICE_BLIND)
    {
        fU = ((iSkillType - AT_SKILL_ALICE_SLEEP + 4) % 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (iSkillType == AT_SKILL_ALICE_SLEEP_STR)
    {
        fU = (4 % 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (iSkillType == AT_SKILL_ALICE_BERSERKER)
    {
        fU = 10 * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (iSkillType >= AT_SKILL_ALICE_WEAKNESS && iSkillType <= AT_SKILL_ALICE_ENERVATION)
    {
        fU = (iSkillType - AT_SKILL_ALICE_WEAKNESS + 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (iSkillType >= AT_SKILL_SUMMON_EXPLOSION && iSkillType <= AT_SKILL_SUMMON_REQUIEM)
    {
        fU = ((iSkillType - AT_SKILL_SUMMON_EXPLOSION + 6) % 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (iSkillType == AT_SKILL_SUMMON_POLLUTION)
    {
        fU = 11 * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (iSkillType == AT_SKILL_STRIKE_OF_DESTRUCTION)
    {
        fU = 7 * width / 256.f;
        fV = 2 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (iSkillType == AT_SKILL_GAOTIC)
    {
        fU = 3 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (iSkillType == AT_SKILL_RECOVER)
    {
        fU = 9 * width / 256.f;
        fV = 2 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (iSkillType == AT_SKILL_MULTI_SHOT)
    {
        fU = 0 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (iSkillType == AT_SKILL_FLAME_STRIKE)
    {
        int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
        int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

        fU = 1 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (iSkillType == AT_SKILL_GIGANTIC_STORM)
    {
        fU = 2 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (iSkillType == AT_SKILL_LIGHTNING_SHOCK)
    {
        fU = 2 * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (iSkillType == AT_SKILL_EXPANSION_OF_WIZARDRY)
    {
        fU = 8 * width / 256.f;
        fV = 2 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillUseType == 4)
    {
        fU = (width / 256.f) * (Skill_Icon % 12);
        fV = (height / 256.f) * ((Skill_Icon / 12) + 4);
        iKindofSkill = KOS_SKILL2;
    }
    else if (iSkillType >= AT_SKILL_KILLING_BLOW)
    {
        fU = ((iSkillType - AT_SKILL_KILLING_BLOW) % 12) * width / 256.f;
        fV = ((iSkillType - AT_SKILL_KILLING_BLOW) / 12) * height / 256.f;
        iKindofSkill = KOS_SKILL3;
    }
    else if (iSkillType >= AT_SKILL_SPIRAL_SLASH)
    {
        fU = ((iSkillType - AT_SKILL_SPIRAL_SLASH) % 8) * width / 256.f;
        fV = ((iSkillType - AT_SKILL_SPIRAL_SLASH) / 8) * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else
    {
        fU = ((iSkillType - 1) % 8) * width / 256.f;
        fV = ((iSkillType - 1) / 8) * height / 256.f;
        iKindofSkill = KOS_SKILL1;
    }
    int iTextureId = 0;
    switch (iKindofSkill)
    {
    case KOS_COMMAND:
    {
        iTextureId = IMAGE_COMMAND;
    }break;
    case KOS_SKILL1:
    {
        iTextureId = IMAGE_SKILL1;
    }break;
    case KOS_SKILL2:
    {
        iTextureId = IMAGE_SKILL2;
    }break;
    case KOS_SKILL3:
    {
        iTextureId = IMAGE_SKILL3;
    }break;
    }

    if (iSkillType >= AT_SKILL_MASTER_BEGIN)
    {
        RenderImage(BITMAP_INTERFACE_MASTER_BEGIN + 2, x, y, width, height, (20.f / 512.f) * (Skill_Icon % 25), ((28.f / 512.f) * ((Skill_Icon / 25))), 20.f / 512.f, 28.f / 512.f);
    }
    else if (iTextureId != 0)
    {
        RenderBitmap(iTextureId, x, y, width, height, fU, fV, width / 256.f, height / 256.f);
    }
}

bool CNewUIMuHelperSkillList::IsAttackSkill(int iSkillType)
{
    if (IsBuffSkill(iSkillType))
    {
        return false;
    }

    if (IsDefenseSkill(iSkillType))
    {
        return false;
    }

    if (IsHealingSkill(iSkillType))
    {
        return false;
    }

    return true;
}

bool CNewUIMuHelperSkillList::IsBuffSkill(int iSkillType)
{
    // To-do: Complete list of buffs

    switch (iSkillType)
    {
    // BK buffs
    case AT_SKILL_SWELL_LIFE:
    case AT_SKILL_SWELL_LIFE_STR:
    case AT_SKILL_SWELL_LIFE_PROFICIENCY:
        return true;
    // Elf buffs
    case AT_SKILL_INFINITY_ARROW:
    case AT_SKILL_INFINITY_ARROW_STR:
    case AT_SKILL_DEFENSE:
    case AT_SKILL_DEFENSE_STR:
    case AT_SKILL_DEFENSE_MASTERY:
    case AT_SKILL_ATTACK:
    case AT_SKILL_ATTACK_STR:
    case AT_SKILL_ATTACK_MASTERY:
        return true;
    // Wiz buffs
    case AT_SKILL_SOUL_BARRIER:
    case AT_SKILL_SOUL_BARRIER_STR:
    case AT_SKILL_SOUL_BARRIER_PROFICIENCY:
    case AT_SKILL_EXPANSION_OF_WIZARDRY:
    case AT_SKILL_EXPANSION_OF_WIZARDRY_STR:
    case AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY:
        return true;
    // DL buffs
    case AT_SKILL_ADD_CRITICAL:
    case AT_SKILL_ADD_CRITICAL_STR1:
    case AT_SKILL_ADD_CRITICAL_STR2:
    case AT_SKILL_ADD_CRITICAL_STR3:
        return true;
    // Summoner buffs
    case AT_SKILL_ALICE_BERSERKER:
    case AT_SKILL_ALICE_BERSERKER_STR:
    case AT_SKILL_ALICE_THORNS:
        return true;
        // RF Buffs
    case AT_SKILL_ATT_UP_OURFORCES:
    case AT_SKILL_HP_UP_OURFORCES:
    case AT_SKILL_DEF_UP_OURFORCES:
    case AT_SKILL_HP_UP_OURFORCES_STR:
    case AT_SKILL_DEF_UP_OURFORCES_MASTERY:
    case AT_SKILL_DEF_UP_OURFORCES_STR:
        return true;
    }

    return false;
}

bool CNewUIMuHelperSkillList::IsHealingSkill(int iSkillType)
{
    // To-do: Complete list of healing skills

    switch (iSkillType)
    {
    case AT_SKILL_HEALING:
    case AT_SKILL_HEALING_STR:
        return true;
    case AT_SKILL_ALICE_DRAINLIFE:
    case AT_SKILL_ALICE_DRAINLIFE_STR:
        return true;
    }

    return false;
}

bool CNewUIMuHelperSkillList::IsDefenseSkill(int iSkillType)
{
    switch (iSkillType)
    {
    case AT_SKILL_DEFENSE:
    case AT_SKILL_DEFENSE_STR:
        return true;
    }

    return false;
}

void CNewUIMuHelperSkillList::FilterByAttackSkills()
{
    m_bFilterByAttackSkills = true;
    m_bFilterByBuffSkills = false;

    PrepareSkillsToRender();
}

void CNewUIMuHelperSkillList::FilterByBuffSkills()
{
    m_bFilterByBuffSkills = true;
    m_bFilterByAttackSkills = false;

    PrepareSkillsToRender();
}

void CNewUIMuHelperSkillList::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
    if (pClass)
    {
        auto* pSkillList = (CNewUIMuHelperSkillList*)(pClass);
        pSkillList->RenderSkillInfo();
    }
}

int CNewUIMuHelperSkillList::UpdateMouseSkillList()
{
    auto li = m_skillIconMap.begin();

    for (; li != m_skillIconMap.end(); li++)
    {
        cSkillIcon* pIcon = &li->second;

        if (CheckMouseIn(pIcon->location.x, pIcon->location.y, pIcon->area.cx, pIcon->area.cy))
        {
            return li->first;
        }
    }

    return -1;
}

CNewUIMuHelperExt::CNewUIMuHelperExt()
{
    m_pNewUIMng = NULL;
    m_Pos.x = 0;
    m_Pos.y = 0;
    m_iCurrentPage = -1;
}

CNewUIMuHelperExt::~CNewUIMuHelperExt()
{
    Release();
}

bool CNewUIMuHelperExt::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(INTERFACE_MUHELPER_EXT, this);

    SetPos(x, y);

    LoadImages();

    InitButtons();

    InitCheckBox();

    InitImage();

    InitText();

    Show(false);

    return true;
}

void CNewUIMuHelperExt::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNewUIMuHelperExt::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

void CNewUIMuHelperExt::InitText()
{
    m_BuffTimeInput.Init(g_hWnd, 17, 15, MAX_NUMBER_DIGITS, false);
    m_BuffTimeInput.SetTextColor(255, 0, 0, 0);
    m_BuffTimeInput.SetBackColor(255, 255, 255, 255);
    m_BuffTimeInput.SetFont(g_hFont);
    m_BuffTimeInput.SetState(UISTATE_NORMAL);
    m_BuffTimeInput.SetOption(UIOPTION_NUMBERONLY);
}

void CNewUIMuHelperExt::InitImage()
{

}

void CNewUIMuHelperExt::InitButtons()
{
    m_BtnPreConHuntRange.CheckBoxImgState(IMAGE_MACROUI_HELPER_OPTIONBUTTON);
    m_BtnPreConHuntRange.CheckBoxInfo(m_Pos.x + 17, m_Pos.y + 78, 15, 15);
    m_BtnPreConHuntRange.ChangeText(GlobalText[3555]); // "Monster Within Hunting range"

    m_BtnPreConAttacking.CheckBoxImgState(IMAGE_MACROUI_HELPER_OPTIONBUTTON);
    m_BtnPreConAttacking.CheckBoxInfo(m_Pos.x + 17, m_Pos.y + 93, 15, 15);
    m_BtnPreConAttacking.ChangeText(GlobalText[3556]); // "Monster Attacking Me"

    m_BtnSubConMoreThanTwo.CheckBoxImgState(IMAGE_MACROUI_HELPER_OPTIONBUTTON);
    m_BtnSubConMoreThanTwo.CheckBoxInfo(m_Pos.x + 17, m_Pos.y + 143, 15, 15);
    m_BtnSubConMoreThanTwo.ChangeText(GlobalText[3557]); // "More Than 2 Mobs"

    m_BtnSubConMoreThanThree.CheckBoxImgState(IMAGE_MACROUI_HELPER_OPTIONBUTTON);
    m_BtnSubConMoreThanThree.CheckBoxInfo(m_Pos.x + 17, m_Pos.y + 158, 15, 15);
    m_BtnSubConMoreThanThree.ChangeText(GlobalText[3558]); // "More Than 3 Mobs"

    m_BtnSubConMoreThanFour.CheckBoxImgState(IMAGE_MACROUI_HELPER_OPTIONBUTTON);
    m_BtnSubConMoreThanFour.CheckBoxInfo(m_Pos.x + 17 + 78, m_Pos.y + 143, 15, 15);
    m_BtnSubConMoreThanFour.ChangeText(GlobalText[3559]); // "More Than 4 Mobs"

    m_BtnSubConMoreThanFive.CheckBoxImgState(IMAGE_MACROUI_HELPER_OPTIONBUTTON);
    m_BtnSubConMoreThanFive.CheckBoxInfo(m_Pos.x + 17 + 78, m_Pos.y + 158, 15, 15);
    m_BtnSubConMoreThanFive.ChangeText(GlobalText[3560]); // "More Than 5 Mobs"

    m_BtnPartyHeal.CheckBoxImgState(IMAGE_OPTION_BTN_CHECK);
    m_BtnPartyHeal.CheckBoxInfo(m_Pos.x + 17, m_Pos.y + 78, 15, 15);
    m_BtnPartyHeal.ChangeText(GlobalText[3539]); // "Preference of Party Heal"

    m_BtnPartyDuration.CheckBoxImgState(IMAGE_OPTION_BTN_CHECK);
    m_BtnPartyDuration.CheckBoxInfo(m_Pos.x + 17, m_Pos.y + 168, 15, 15);
    m_BtnPartyDuration.ChangeText(GlobalText[3540]); // "Buff Duration for All Party Members"

    m_BtnSave.ChangeButtonImgState(1, IMAGE_IGS_BUTTON, 1, 0, 1);
    m_BtnSave.ChangeButtonInfo(m_Pos.x + 120, m_Pos.y + 388, 52, 26);
    m_BtnSave.ChangeText(GlobalText[3503]); // "Save Setting"
    m_BtnSave.MoveTextPos(0, -1);
    m_BtnSave.ChangeToolTipText(L"", TRUE);

    m_BtnReset.ChangeButtonImgState(1, IMAGE_IGS_BUTTON, 1, 0, 1);
    m_BtnReset.ChangeButtonInfo(m_Pos.x + 65, m_Pos.y + 388, 52, 26);
    m_BtnReset.ChangeText(GlobalText[3504]); // "Initialization"
    m_BtnReset.MoveTextPos(0, -1);
    m_BtnReset.ChangeToolTipText(L"", TRUE);

    m_BtnClose.ChangeButtonImgState(1, IMAGE_BASE_WINDOW_BTN_EXIT, 0, 0, 0);
    m_BtnClose.ChangeButtonInfo(m_Pos.x + 20, m_Pos.y + 388, 36, 29);
    m_BtnClose.ChangeText(L"");
    m_BtnClose.ChangeToolTipText(GlobalText[388], TRUE); // "Close"
}

void CNewUIMuHelperExt::InitCheckBox()
{

}

bool CNewUIMuHelperExt::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    DWORD TextColor = g_pRenderText->GetTextColor();

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(0xFFFFFFFF);
    g_pRenderText->SetBgColor(0);

    RenderImage(IMAGE_BASE_WINDOW_BACK, m_Pos.x, m_Pos.y, float(WINDOW_WIDTH), float(WINDOW_HEIGHT));
    RenderImage(IMAGE_BASE_WINDOW_TOP, m_Pos.x, m_Pos.y, float(WINDOW_WIDTH), 64.f);
    RenderImage(IMAGE_BASE_WINDOW_LEFT, m_Pos.x, m_Pos.y + 64.f, 21.f, float(WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_BASE_WINDOW_RIGHT, m_Pos.x + float(WINDOW_WIDTH) - 21.f, m_Pos.y + 64.f, 21.f, float(WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_BASE_WINDOW_BOTTOM, m_Pos.x, m_Pos.y + float(WINDOW_HEIGHT) - 45.f, float(WINDOW_WIDTH), 45.f);

    g_pRenderText->SetFont(g_hFontBold);

    if (m_iCurrentPage == SUB_PAGE_POTION_CONFIG_ELF)
    {
        g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 13, GlobalText[3553], 190, 0, RT3_SORT_CENTER); // "Auto Recovery"
        RenderBackPane(m_Pos.x + 12, m_Pos.y + 55, 165, 45, GlobalText[3545]); // "Auto Potion"
        RenderHpLevel(m_Pos.x + 32, m_Pos.y + 80, 124.f, 16.f, m_iCurrentPotionThreshold, GlobalText[3547]); // "HP Status"

        RenderBackPane(m_Pos.x + 12, m_Pos.y + 120, 165, 45, GlobalText[3546]); // "Auto Heal"
        RenderHpLevel(m_Pos.x + 32, m_Pos.y + 145, 124.f, 16.f, m_iCurrentHealThreshold, GlobalText[3547]); // "HP Status"
    }
    else if (m_iCurrentPage == SUB_PAGE_POTION_CONFIG_SUMMY)
    {
        g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 13, GlobalText[3553], 190, 0, RT3_SORT_CENTER); // "Auto Recovery"
        RenderBackPane(m_Pos.x + 12, m_Pos.y + 55, 165, 45, GlobalText[3545]); // "Auto Potion"
        RenderHpLevel(m_Pos.x + 32, m_Pos.y + 80, 124.f, 16.f, m_iCurrentPotionThreshold, GlobalText[3547]); // "HP Status"

        RenderBackPane(m_Pos.x + 12, m_Pos.y + 120, 165, 45, GlobalText[3517]); // "Drain Life"
        RenderHpLevel(m_Pos.x + 32, m_Pos.y + 145, 124.f, 16.f, m_iCurrentHealThreshold, GlobalText[3547]); // "HP Status"
    }
    else if (m_iCurrentPage == SUB_PAGE_POTION_CONFIG)
    {
        g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 13, GlobalText[3553], 190, 0, RT3_SORT_CENTER); // "Auto Recovery"
        RenderBackPane(m_Pos.x + 12, m_Pos.y + 55, 165, 45, GlobalText[3545]); // "Auto Potion"

        RenderHpLevel(m_Pos.x + 32, m_Pos.y + 80, 124.f, 16.f, m_iCurrentPotionThreshold, GlobalText[3547]);
    }
    else if (m_iCurrentPage == SUB_PAGE_SKILL2_CONFIG
        || m_iCurrentPage == SUB_PAGE_SKILL3_CONFIG)
    {
        g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 13, GlobalText[3552], 190, 0, RT3_SORT_CENTER); // "Activation Skill"
        RenderBackPane(m_Pos.x + 12, m_Pos.y + 55, 165, 45, GlobalText[3543]);  // "Pre-con"
        m_BtnPreConHuntRange.Render();
        m_BtnPreConAttacking.Render();

        RenderBackPane(m_Pos.x + 12, m_Pos.y + 120, 165, 45, GlobalText[3544]); // "Sub-con"
        m_BtnSubConMoreThanTwo.Render();
        m_BtnSubConMoreThanThree.Render();
        m_BtnSubConMoreThanFour.Render();
        m_BtnSubConMoreThanFive.Render();
    }

    else if (m_iCurrentPage == SUB_PAGE_PARTY_CONFIG)
    {
        g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 13, GlobalText[3554], 190, 0, RT3_SORT_CENTER); // "Party"
        g_pRenderText->SetTextColor(TextColor);
        RenderBackPane(m_Pos.x + 12, m_Pos.y + 55, 165, 45, GlobalText[3549]); // Buff Support
        m_BtnPartyDuration.Render();
        g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 97, GlobalText[3551], 124, 0, RT3_SORT_LEFT); // "Time Space of Casting Buff"
        RenderImage(IMAGE_MACROUI_HELPER_INPUTNUMBER, m_Pos.x + 125, m_Pos.y + 93, 20, 15);
        m_BuffTimeInput.Render();
        g_pRenderText->RenderText(m_Pos.x + 146, m_Pos.y + 97, L"s", 124, 0, RT3_SORT_LEFT); // "s"
    }
    else if (m_iCurrentPage == SUB_PAGE_PARTY_CONFIG_ELF)
    {
        g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 13, GlobalText[3554], 190, 0, RT3_SORT_CENTER); // "Party"
        g_pRenderText->SetTextColor(TextColor);
        RenderBackPane(m_Pos.x + 12, m_Pos.y + 55, 165, 70, GlobalText[3548]); // Heal Support
        m_BtnPartyHeal.Render();
        RenderHpLevel(m_Pos.x + 32, m_Pos.y + 100, 124.f, 16.f, m_iCurrentPartyHealThreshold, GlobalText[3550]); // "HP Status of Party Members"

        RenderBackPane(m_Pos.x + 12, m_Pos.y + 145, 165, 45, GlobalText[3549]); // Buff Support
        m_BtnPartyDuration.Render();
        g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 187, GlobalText[3551], 124, 0, RT3_SORT_LEFT); // "Time Space of Casting Buff"
        RenderImage(IMAGE_MACROUI_HELPER_INPUTNUMBER, m_Pos.x + 125, m_Pos.y + 183, 20, 15);
        m_BuffTimeInput.Render();
        g_pRenderText->RenderText(m_Pos.x + 146, m_Pos.y + 187, L"s", 124, 0, RT3_SORT_LEFT); // "s"
    }

    m_BtnSave.Render();
    m_BtnReset.Render();
    m_BtnClose.Render();

    DisableAlphaBlend();

    return true;
}

void CNewUIMuHelperExt::RenderHpLevel(int x, int y, int width, int height, int level, const wchar_t* pszLabel)
{
    RenderImage(IMAGE_OPTION_VOLUME_BACK, x, y, 124.f, 16.f);
    if (level > 0)
    {
        RenderImage(IMAGE_OPTION_VOLUME_COLOR, x, y, 124.f * 0.1f * (level), 16.f);
    }
    g_pRenderText->RenderText(x, y + 18, pszLabel, width, 0, RT3_SORT_CENTER);
}

void CNewUIMuHelperExt::RenderBackPane(int x, int y, int width, int height, const wchar_t* pszHeader)
{
    DWORD TextColor = g_pRenderText->GetTextColor();
    int headerWidth = 65;

    EnableAlphaTest();
    glColor4f(0.0, 0.0, 0.0, 0.4f);
    RenderColor(x + 3.f, y + 2.f, headerWidth - 7.f, 18.f, 0.0, 0);  // shade for top box
    RenderColor(x + 3.f, y + 2.f + 18.f, width - 7.f, height - 7.f, 0.0, 0);  // shade for bottom box
    EndRenderColor();

    // Top box (tab) without bottom line
    RenderImage(IMAGE_TABLE_TOP_LEFT, x, y, 14.0, 14.0);                                // Top-left corner of the tab
    RenderImage(IMAGE_TABLE_TOP_RIGHT, (x + headerWidth) - 14.f, y, 14.0, 14.0);        // Top-right corner of the tab
    RenderImage(IMAGE_TABLE_TOP_PIXEL, x + 6.f, y, (headerWidth - 12.f), 14.0);         // Top edge of the tab
    RenderImage(IMAGE_TABLE_RIGHT_PIXEL, (x + headerWidth) - 14.f, y + 6.f, 14.0, 14.0); // Right edge of the tab

    // Bottom box without top line
    RenderImage(IMAGE_TABLE_TOP_RIGHT, (x + width) - 14.f, y + 18.f, 14.0, 14.0);       // Main box top-right corner
    RenderImage(IMAGE_TABLE_BOTTOM_LEFT, x, (y + height + 18.f) - 14.f, 14.0, 14.0);    // Main box bottom-left corner
    RenderImage(IMAGE_TABLE_BOTTOM_RIGHT, (x + width) - 14.f, (y + height + 18.f) - 14.f, 14.0, 14.0); // Main box bottom-right corner
    RenderImage(IMAGE_TABLE_TOP_PIXEL, x + 2.f, y + 18.f, (width - 12.f), 14.0);        // Top edge of main box
    RenderImage(IMAGE_TABLE_RIGHT_PIXEL, (x + width) - 14.f, y + 24.f, 14.0, (height - 14.f)); // Right edge of main box
    RenderImage(IMAGE_TABLE_BOTTOM_PIXEL, x + 6.f, (y + height + 18.f) - 14.f, (width - 12.f), 14.0); // Bottom edge of main box

    // Left line to connect top box and bottom box
    RenderImage(IMAGE_TABLE_LEFT_PIXEL, x, y + 6.f, 14.0, (height));             // Connecting left edge

    // Header inside top box
    g_pRenderText->SetTextColor(TextColor);
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->RenderText(x + 10.f, y + 6.f, pszHeader, headerWidth, 0, RT3_SORT_LEFT);
}

void CNewUIMuHelperExt::LoadImages()
{
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_RangeMinus.tga", IMAGE_MACROUI_HELPER_RAGEMINUS, GL_LINEAR, GL_CLAMP, 1, 0);
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_OptionButton.tga", IMAGE_MACROUI_HELPER_OPTIONBUTTON, GL_LINEAR, GL_CLAMP, 1, 0);
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_InputNumber.tga", IMAGE_MACROUI_HELPER_INPUTNUMBER, GL_LINEAR, GL_CLAMP, 1, 0);
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_InputString.tga", IMAGE_MACROUI_HELPER_INPUTSTRING, GL_LINEAR, GL_CLAMP, 1, 0);
    //--
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Bt03.tga", IMAGE_IGS_BUTTON, GL_LINEAR, GL_CLAMP, 1, 0);
}

void CNewUIMuHelperExt::UnloadImages()
{
    DeleteBitmap(IMAGE_MACROUI_HELPER_RAGEMINUS);
    DeleteBitmap(IMAGE_MACROUI_HELPER_OPTIONBUTTON);
    DeleteBitmap(IMAGE_MACROUI_HELPER_INPUTNUMBER);
    DeleteBitmap(IMAGE_MACROUI_HELPER_INPUTSTRING);
    //--
    DeleteBitmap(IMAGE_IGS_BUTTON);
}

bool CNewUIMuHelperExt::Update()
{
    if (IsVisible())
    {
        if (m_iCurrentPage == SUB_PAGE_SKILL2_CONFIG || m_iCurrentPage == SUB_PAGE_SKILL3_CONFIG)
        {
            int iSkillIndex = m_iCurrentPage == SUB_PAGE_SKILL2_CONFIG ? 1 : 2; // SKill 2 : Skill 3

            if (m_BtnPreConHuntRange.UpdateMouseEvent())
            {
                m_BtnPreConHuntRange.RegisterBoxState(true);
                m_BtnPreConAttacking.RegisterBoxState(!m_BtnPreConHuntRange.GetBoxState());

                // Clear other precondition bits and set the bit for "Hunt Range"
                _TempConfig.aiSkillCondition[iSkillIndex] =
                    (_TempConfig.aiSkillCondition[iSkillIndex] & MUHELPER_SKILL_PRECON_CLEAR) |
                    ON_MOBS_NEARBY;
            }
            else if (m_BtnPreConAttacking.UpdateMouseEvent())
            {
                m_BtnPreConAttacking.RegisterBoxState(true);
                m_BtnPreConHuntRange.RegisterBoxState(!m_BtnPreConAttacking.GetBoxState());

                // Clear other precondition bits and set the bit for "Attacking"
                _TempConfig.aiSkillCondition[iSkillIndex] =
                    (_TempConfig.aiSkillCondition[iSkillIndex] & MUHELPER_SKILL_PRECON_CLEAR) |
                    ON_MOBS_ATTACKING;
            }
            else if (m_BtnSubConMoreThanTwo.UpdateMouseEvent())
            {
                m_BtnSubConMoreThanTwo.RegisterBoxState(true);
                m_BtnSubConMoreThanThree.RegisterBoxState(false);
                m_BtnSubConMoreThanFour.RegisterBoxState(false);
                m_BtnSubConMoreThanFive.RegisterBoxState(false);

                // Clear other bits and set the bit for "More Than Two Mobs"
                _TempConfig.aiSkillCondition[iSkillIndex] =
                    (_TempConfig.aiSkillCondition[iSkillIndex] & MUHELPER_SKILL_SUBCON_CLEAR) |
                    ON_MORE_THAN_TWO_MOBS;
            }
            else if (m_BtnSubConMoreThanThree.UpdateMouseEvent())
            {
                m_BtnSubConMoreThanTwo.RegisterBoxState(false);
                m_BtnSubConMoreThanThree.RegisterBoxState(true);
                m_BtnSubConMoreThanFour.RegisterBoxState(false);
                m_BtnSubConMoreThanFive.RegisterBoxState(false);

                // Clear other bits and set the bit for "More Than Three Mobs"
                _TempConfig.aiSkillCondition[iSkillIndex] =
                    (_TempConfig.aiSkillCondition[iSkillIndex] & MUHELPER_SKILL_SUBCON_CLEAR) |
                    ON_MORE_THAN_THREE_MOBS;
            }
            else if (m_BtnSubConMoreThanFour.UpdateMouseEvent())
            {
                m_BtnSubConMoreThanTwo.RegisterBoxState(false);
                m_BtnSubConMoreThanThree.RegisterBoxState(false);
                m_BtnSubConMoreThanFour.RegisterBoxState(true);
                m_BtnSubConMoreThanFive.RegisterBoxState(false);

                // Clear other bits and set the bit for "More Than Four Mobs"
                _TempConfig.aiSkillCondition[iSkillIndex] =
                    (_TempConfig.aiSkillCondition[iSkillIndex] & MUHELPER_SKILL_SUBCON_CLEAR) |
                    ON_MORE_THAN_FOUR_MOBS;
            }
            else if (m_BtnSubConMoreThanFive.UpdateMouseEvent())
            {
                m_BtnSubConMoreThanTwo.RegisterBoxState(false);
                m_BtnSubConMoreThanThree.RegisterBoxState(false);
                m_BtnSubConMoreThanFour.RegisterBoxState(false);
                m_BtnSubConMoreThanFive.RegisterBoxState(true);

                // Clear other bits and set the bit for "More Than Five Mobs"
                _TempConfig.aiSkillCondition[iSkillIndex] =
                    (_TempConfig.aiSkillCondition[iSkillIndex] & MUHELPER_SKILL_SUBCON_CLEAR) |
                    ON_MORE_THAN_FIVE_MOBS;
            }
        }

        if (m_iCurrentPage == SUB_PAGE_PARTY_CONFIG_ELF)
        {
            if (m_BtnPartyHeal.UpdateMouseEvent())
            {
                _TempConfig.bAutoHealParty = m_BtnPartyHeal.GetBoxState();
            }
        }

        if (m_iCurrentPage == SUB_PAGE_PARTY_CONFIG || m_iCurrentPage == SUB_PAGE_PARTY_CONFIG_ELF)
        {
            if (m_BtnPartyDuration.UpdateMouseEvent())
            {
                _TempConfig.bBuffDurationParty = m_BtnPartyDuration.GetBoxState();
            }
        }

        if (m_BtnClose.UpdateMouseEvent())
        {
            g_pNewUISystem->Hide(INTERFACE_MUHELPER_EXT);
        }
        else if (m_BtnSave.UpdateMouseEvent())
        {
            Save();
            g_pNewUISystem->Hide(INTERFACE_MUHELPER_EXT);
        }
        else if (m_BtnReset.UpdateMouseEvent())
        {
            Reset();
        }
    }
    return true;
}

bool CNewUIMuHelperExt::UpdateMouseEvent()
{
    // Ignore events outside MU Helper window
    if (!CheckMouseIn(m_Pos.x, m_Pos.y, WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        return true;
    }

    if (CheckMouseIn(m_Pos.x + 33 - 8, m_Pos.y + 80, 124 + 8, 16))
    {
        int iOldValue = m_iCurrentPotionThreshold;
        if (MouseWheel > 0)
        {
            MouseWheel = 0;
            m_iCurrentPotionThreshold++;
            if (m_iCurrentPotionThreshold > 10)
            {
                m_iCurrentPotionThreshold = 10;
            }
        }
        else if (MouseWheel < 0)
        {
            MouseWheel = 0;
            m_iCurrentPotionThreshold--;
            if (m_iCurrentPotionThreshold < 0)
            {
                m_iCurrentPotionThreshold = 0;
            }
        }
        if (IsRepeat(VK_LBUTTON))
        {
            int x = MouseX - (m_Pos.x + 33);
            if (x < 0)
            {
                m_iCurrentPotionThreshold = 0;
            }
            else
            {
                float fValue = (10.f * x) / 124.f;
                m_iCurrentPotionThreshold = (int)fValue + 1;
            }
        }
    }

    if (m_iCurrentPage == SUB_PAGE_POTION_CONFIG_ELF || m_iCurrentPage == SUB_PAGE_POTION_CONFIG_SUMMY)
    {
        if (CheckMouseIn(m_Pos.x + 33 - 8, m_Pos.y + 145, 124 + 8, 16))
        {
            int iOldValue = m_iCurrentHealThreshold;
            if (MouseWheel > 0)
            {
                MouseWheel = 0;
                m_iCurrentHealThreshold++;
                if (m_iCurrentHealThreshold > 10)
                {
                    m_iCurrentHealThreshold = 10;
                }
            }
            else if (MouseWheel < 0)
            {
                MouseWheel = 0;
                m_iCurrentHealThreshold--;
                if (m_iCurrentHealThreshold < 0)
                {
                    m_iCurrentHealThreshold = 0;
                }
            }
            if (IsRepeat(VK_LBUTTON))
            {
                int x = MouseX - (m_Pos.x + 33);
                if (x < 0)
                {
                    m_iCurrentHealThreshold = 0;
                }
                else
                {
                    float fValue = (10.f * x) / 124.f;
                    m_iCurrentHealThreshold = (int)fValue + 1;
                }
            }
        }
    }
    else if (m_iCurrentPage == SUB_PAGE_PARTY_CONFIG_ELF)
    {
        if (CheckMouseIn(m_Pos.x + 32 - 8, m_Pos.y + 100, 124 + 8, 16))
        {
            int iOldValue = m_iCurrentPartyHealThreshold;
            if (MouseWheel > 0)
            {
                MouseWheel = 0;
                m_iCurrentPartyHealThreshold++;
                if (m_iCurrentPartyHealThreshold > 10)
                {
                    m_iCurrentPartyHealThreshold = 10;
                }
            }
            else if (MouseWheel < 0)
            {
                MouseWheel = 0;
                m_iCurrentPartyHealThreshold--;
                if (m_iCurrentPartyHealThreshold < 0)
                {
                    m_iCurrentPartyHealThreshold = 0;
                }
            }
            if (IsRepeat(VK_LBUTTON))
            {
                int x = MouseX - (m_Pos.x + 33);
                if (x < 0)
                {
                    m_iCurrentPartyHealThreshold = 0;
                }
                else
                {
                    float fValue = (10.f * x) / 124.f;
                    m_iCurrentPartyHealThreshold = (int)fValue + 1;
                }
            }
        }
        else if (CheckMouseIn(m_BuffTimeInput.GetPosition_x(), m_BuffTimeInput.GetPosition_y(), 20, 15))
        {
            m_BuffTimeInput.GiveFocus();
        }
        else
        {
            SetFocus(g_hWnd);
        }
    }
    else if (m_iCurrentPage == SUB_PAGE_PARTY_CONFIG)
    {
        if (CheckMouseIn(m_BuffTimeInput.GetPosition_x(), m_BuffTimeInput.GetPosition_y(), 20, 15))
        {
            m_BuffTimeInput.GiveFocus();
        }
        else
        {
            SetFocus(g_hWnd);
        }
    }

    return false;
}

bool CNewUIMuHelperExt::UpdateKeyEvent()
{
    if (IsVisible())
    {
        if (IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(INTERFACE_MUHELPER_EXT);
            //PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }
    return true;
}

float CNewUIMuHelperExt::GetLayerDepth()
{
    return 3.4;
}

float CNewUIMuHelperExt::GetKeyEventOrder()
{
    return 3.4;
}

void CNewUIMuHelperExt::Toggle(int iPageId)
{
    int iPrevPage = m_iCurrentPage;
    m_iCurrentPage = iPageId;

    if (IsVisible() && m_iCurrentPage == iPrevPage)
    {
        m_iCurrentPage = -1;
        this->Show(false);
        return;
    }

    if (m_iCurrentPage == SUB_PAGE_SKILL2_CONFIG || m_iCurrentPage == SUB_PAGE_SKILL3_CONFIG)
    {
        int iSkillIndex = m_iCurrentPage == SUB_PAGE_SKILL2_CONFIG ? 1 : 2;
        m_BtnPreConHuntRange.RegisterBoxState(_TempConfig.aiSkillCondition[iSkillIndex] & ON_MOBS_NEARBY);
        m_BtnPreConAttacking.RegisterBoxState(_TempConfig.aiSkillCondition[iSkillIndex] & ON_MOBS_ATTACKING);
        m_BtnSubConMoreThanTwo.RegisterBoxState(_TempConfig.aiSkillCondition[iSkillIndex] & ON_MORE_THAN_TWO_MOBS);
        m_BtnSubConMoreThanThree.RegisterBoxState(_TempConfig.aiSkillCondition[iSkillIndex] & ON_MORE_THAN_THREE_MOBS);
        m_BtnSubConMoreThanFour.RegisterBoxState(_TempConfig.aiSkillCondition[iSkillIndex] & ON_MORE_THAN_FOUR_MOBS);
        m_BtnSubConMoreThanFive.RegisterBoxState(_TempConfig.aiSkillCondition[iSkillIndex] & ON_MORE_THAN_FIVE_MOBS);
    }
    else if (m_iCurrentPage == SUB_PAGE_POTION_CONFIG || m_iCurrentPage == SUB_PAGE_POTION_CONFIG_ELF || m_iCurrentPage == SUB_PAGE_POTION_CONFIG_SUMMY)
    {
        m_iCurrentPotionThreshold = _TempConfig.iPotionThreshold / 10;
        m_iCurrentHealThreshold = _TempConfig.iHealThreshold / 10;
    }
    else if (m_iCurrentPage == SUB_PAGE_PARTY_CONFIG)
    {
        m_BtnPartyDuration.CheckBoxInfo(m_Pos.x + 17, m_Pos.y + 78, 15, 15);
        m_BtnPartyDuration.RegisterBoxState(_TempConfig.bBuffDurationParty);
        m_iCurrentPartyHealThreshold = _TempConfig.iHealPartyThreshold / 10;

        wchar_t wsBuffTime[MAX_NUMBER_DIGITS + 1] = { 0 };
        swprintf(wsBuffTime, MAX_NUMBER_DIGITS + 1, L"%d", _TempConfig.iBuffCastInterval);
        m_BuffTimeInput.SetText(wsBuffTime);
        m_BuffTimeInput.SetPosition(m_Pos.x + 127, m_Pos.y + 97);
    }
    else if (m_iCurrentPage == SUB_PAGE_PARTY_CONFIG_ELF)
    {
        m_BtnPartyHeal.RegisterBoxState(_TempConfig.bAutoHealParty);

        m_BtnPartyDuration.CheckBoxInfo(m_Pos.x + 17, m_Pos.y + 168, 15, 15);
        m_BtnPartyDuration.RegisterBoxState(_TempConfig.bBuffDurationParty);
        m_iCurrentPartyHealThreshold = _TempConfig.iHealPartyThreshold / 10;

        wchar_t wsBuffTime[MAX_NUMBER_DIGITS + 1] = { 0 };
        swprintf(wsBuffTime, MAX_NUMBER_DIGITS + 1, L"%d", _TempConfig.iBuffCastInterval);
        m_BuffTimeInput.SetText(wsBuffTime);
        m_BuffTimeInput.SetPosition(m_Pos.x + 127, m_Pos.y + 187);
    }

    this->Show(true);
}

void CNewUIMuHelperExt::Save()
{
    wchar_t wsNumberInput[MAX_NUMBER_DIGITS + 1]{};

    m_BuffTimeInput.GetText(wsNumberInput, sizeof(wsNumberInput));
    _TempConfig.iBuffCastInterval = CNewUIMuHelper::GetIntFromTextInput(wsNumberInput);

    _TempConfig.iPotionThreshold = m_iCurrentPotionThreshold * 10;
    _TempConfig.iHealThreshold = m_iCurrentHealThreshold * 10;
    _TempConfig.iHealPartyThreshold = m_iCurrentPartyHealThreshold * 10;
}

void CNewUIMuHelperExt::ApplySavedConfig()
{
    m_iCurrentPotionThreshold = _TempConfig.iPotionThreshold / 10;
    m_iCurrentHealThreshold = _TempConfig.iHealThreshold / 10;
    m_iCurrentPartyHealThreshold = _TempConfig.iHealPartyThreshold / 10;
}

// Called by the "Initialization" button from the main page
void CNewUIMuHelperExt::InitConfig()
{
    _TempConfig.iPotionThreshold = 40;
    _TempConfig.iHealThreshold = 60;
    _TempConfig.iBuffCastInterval = 0;
    _TempConfig.iHealPartyThreshold = 60;
    _TempConfig.bAutoHealParty = false;
    _TempConfig.bBuffDurationParty = false;
}

// Called by the "Initialization" button from the sub page
void CNewUIMuHelperExt::Reset()
{
    if (m_iCurrentPage == SUB_PAGE_SKILL2_CONFIG 
        || m_iCurrentPage == SUB_PAGE_SKILL3_CONFIG)
    {
        int iSkillIndex = m_iCurrentPage == SUB_PAGE_SKILL2_CONFIG ? 1 : 2;

        _TempConfig.aiSkillCondition[iSkillIndex] = ON_MOBS_NEARBY | ON_MORE_THAN_TWO_MOBS;
        _TempConfig.aiSkillCondition[iSkillIndex] = ON_MOBS_NEARBY | ON_MORE_THAN_TWO_MOBS;

        m_BtnPreConHuntRange.RegisterBoxState(_TempConfig.aiSkillCondition[iSkillIndex] & ON_MOBS_NEARBY);
        m_BtnPreConAttacking.RegisterBoxState(_TempConfig.aiSkillCondition[iSkillIndex] & ON_MOBS_ATTACKING);
        m_BtnSubConMoreThanTwo.RegisterBoxState(_TempConfig.aiSkillCondition[iSkillIndex] & ON_MORE_THAN_TWO_MOBS);
        m_BtnSubConMoreThanThree.RegisterBoxState(_TempConfig.aiSkillCondition[iSkillIndex] & ON_MORE_THAN_THREE_MOBS);
        m_BtnSubConMoreThanFour.RegisterBoxState(_TempConfig.aiSkillCondition[iSkillIndex] & ON_MORE_THAN_FOUR_MOBS);
        m_BtnSubConMoreThanFive.RegisterBoxState(_TempConfig.aiSkillCondition[iSkillIndex] & ON_MORE_THAN_FIVE_MOBS);
    }
    else if (m_iCurrentPage == SUB_PAGE_POTION_CONFIG 
        || m_iCurrentPage == SUB_PAGE_POTION_CONFIG_ELF 
        || m_iCurrentPage == SUB_PAGE_POTION_CONFIG_SUMMY)
    {
        _TempConfig.iPotionThreshold = 0;
        _TempConfig.iHealThreshold = 0;

        m_iCurrentPotionThreshold = _TempConfig.iPotionThreshold / 10;
        m_iCurrentHealThreshold = _TempConfig.iHealThreshold / 10;
    }
}
