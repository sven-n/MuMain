#include "stdafx.h"
#include "NewUISystem.h"
#include "NewUIMuHelper.h"
#include "CharacterManager.h"
#include "MUHelper/MuHelper.h"

#define CHECKBOX_ID_POTION              0
#define CHECKBOX_ID_LONG_DISTANCE       1
#define CHECKBOX_ID_ORIG_POSITION       2
#define CHECKBOX_ID_SKILL2_DELAY        3
#define CHECKBOX_ID_SKILL2_CONDITION    4
#define CHECKBOX_ID_SKILL3_DELAY        5
#define CHECKBOX_ID_SKILL3_CONDITION    6
#define CHECKBOX_ID_COMBO               7
#define CHECKBOX_ID_BUFF_DURATION       8
#define CHECKBOX_ID_USE_PET             9
#define CHECKBOX_ID_PARTY               10
#define CHECKBOX_ID_AUTO_HEAL           11
#define CHECKBOX_ID_DRAIN_LIFE          12
#define CHECKBOX_ID_REPAIR_ITEM         13
#define CHECKBOX_ID_PICK_ALL            14
#define CHECKBOX_ID_PICK_SELECTED       15
#define CHECKBOX_ID_PICK_JEWEL          16
#define CHECKBOX_ID_PICK_ANCIENT        17
#define CHECKBOX_ID_PICK_ZEN            18
#define CHECKBOX_ID_PICK_EXCELLENT      19
#define CHECKBOX_ID_ADD_OTHER_ITEM      20

#define BUTTON_ID_HUNT_RANGE_ADD        0
#define BUTTON_ID_HUNT_RANGE_MINUS      1
#define BUTTON_ID_SKILL2_CONFIG         2
#define BUTTON_ID_SKILL3_CONFIG         3
#define BUTTON_ID_POTION_CONFIG_ELF     4
#define BUTTON_ID_POTION_CONFIG_SUMMY   5
#define BUTTON_ID_POTION_CONFIG         6
#define BUTTON_ID_PARTY_CONFIG          7
#define BUTTON_ID_PARTY_CONFIG_ELF      8
#define BUTTON_ID_PICK_RANGE_ADD        9
#define BUTTON_ID_PICK_RANGE_MINUS      10
#define BUTTON_ID_ADD_OTHER_ITEM        11
#define BUTTON_ID_DELETE_OTHER_ITEM     12
#define BUTTON_ID_SAVE_CONFIG           13
#define BUTTON_ID_INIT_CONFIG           14
#define BUTTON_ID_EXIT_CONFIG           15

#define SKILL_SLOT_SKILL1               0
#define SKILL_SLOT_SKILL2               1
#define SKILL_SLOT_SKILL3               2
#define SKILL_SLOT_BUFF1                3
#define SKILL_SLOT_BUFF2                4
#define SKILL_SLOT_BUFF3                5

SEASON3B::CNewUIMuHelper::CNewUIMuHelper()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_ButtonList.clear();
    m_iNumCurOpenTab = 0;
    m_iSelectedSkillSlot = 0;
    memset(m_aiSelectedSkills, -1, MAX_SKILLS_SLOT);
    memset(&m_TempConfig, 0, sizeof(m_TempConfig));
}

SEASON3B::CNewUIMuHelper::~CNewUIMuHelper()
{
    Release();
}

bool SEASON3B::CNewUIMuHelper::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MUHELPER, this);

    SetPos(x, y);

    LoadImages();

    InitButtons();

    InitCheckBox();

    InitImage();

    InitText();

    Show(false);

    return true;
}

void SEASON3B::CNewUIMuHelper::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUIMuHelper::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

void SEASON3B::CNewUIMuHelper::InitButtons()
{
    std::list<std::wstring> ltext;
    ltext.push_back(GlobalText[3500]);
    ltext.push_back(GlobalText[3501]);
    ltext.push_back(GlobalText[3590]);

    m_TabBtn.CreateRadioGroup(3, IMAGE_WINDOW_TAB_BTN, TRUE);
    m_TabBtn.ChangeRadioText(ltext);
    m_TabBtn.ChangeRadioButtonInfo(true, m_Pos.x + 10.f, m_Pos.y + 48.f, 56, 22);
    m_TabBtn.ChangeFrame(m_iNumCurOpenTab);

    InsertButton(IMAGE_CHAINFO_BTN_STAT, m_Pos.x + 56, m_Pos.y + 78, 16, 15, 0, 0, 0, 0, L"", L"", 0, 0);
    InsertButton(IMAGE_MACROUI_HELPER_RAGEMINUS, m_Pos.x + 56, m_Pos.y + 97, 16, 15, 0, 0, 0, 0, L"", L"", 1, 0);
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 191, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", 2, 0); //-- skill 2
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 243, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", 3, 0); //-- skill 3
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 84, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", 4, 0); //-- Buff
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 79, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", 5, 0); //-- potion
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 84, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", 6, 0); //-- potion
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 17, m_Pos.y + 234, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", 7, 0); //-- potion
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 17, m_Pos.y + 234, 38, 24, 1, 0, 1, 1, GlobalText[3502], L"", 8, 0); //-- potion
    
    InsertButton(IMAGE_CHAINFO_BTN_STAT, m_Pos.x + 56, m_Pos.y + 78, 16, 15, 0, 0, 0, 0, L"", L"", 9, 1);
    InsertButton(IMAGE_MACROUI_HELPER_RAGEMINUS, m_Pos.x + 56, m_Pos.y + 97, 16, 15, 0, 0, 0, 0, L"", L"", 10, 1);
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 208, 38, 24, 1, 0, 1, 1, GlobalText[3505], L"", 11, 1); //-- Buff
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 309, 38, 24, 1, 0, 1, 1, GlobalText[3506], L"", 12, 1); //-- Buff
    //--
    InsertButton(IMAGE_IGS_BUTTON, m_Pos.x + 120, m_Pos.y + 388, 52, 26, 1, 0, 1, 1, GlobalText[3503], L"", 13, -1);
    InsertButton(IMAGE_IGS_BUTTON, m_Pos.x + 65, m_Pos.y + 388, 52, 26, 1, 0, 1, 1, GlobalText[3504], L"", 14, -1);
    InsertButton(IMAGE_BASE_WINDOW_BTN_EXIT, m_Pos.x + 20, m_Pos.y + 388, 36, 29, 0, 0, 0, 0, L"", GlobalText[388], 15, -1);

    RegisterBtnCharacter(0xFF, 0);
    RegisterBtnCharacter(0xFF, 1);
    RegisterBtnCharacter(0xFF, 2);
    RegisterBtnCharacter(0xFF, 9);
    RegisterBtnCharacter(0xFF, 10);
    RegisterBtnCharacter(0xFF, 11);
    RegisterBtnCharacter(0xFF, 12);
    RegisterBtnCharacter(0xFF, 13);
    RegisterBtnCharacter(0xFF, 15);
    RegisterBtnCharacter(0xFF, 14);

    RegisterBtnCharacter(Dark_Knight, 3);
    RegisterBtnCharacter(Dark_Knight, 6);

    RegisterBtnCharacter(Dark_Wizard, 3);
    RegisterBtnCharacter(Dark_Wizard, 6);
    RegisterBtnCharacter(Dark_Wizard, 7);

    RegisterBtnCharacter(Magic_Gladiator, 3);
    RegisterBtnCharacter(Magic_Gladiator, 6);
    RegisterBtnCharacter(Dark_Lord, 6);

    RegisterBtnCharacter(Rage_Fighter, 3);
    RegisterBtnCharacter(Rage_Fighter, 6);

    RegisterBtnCharacter(Fairy_Elf, 3);
    RegisterBtnCharacter(Fairy_Elf, 4);
    RegisterBtnCharacter(Fairy_Elf, 8);

    RegisterBtnCharacter(Summoner, 3);
    RegisterBtnCharacter(Summoner, 5);
}

void SEASON3B::CNewUIMuHelper::InitCheckBox()
{
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 79, m_Pos.y + 80, 15, 15, 0, GlobalText[3507], 0, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 122, 15, 15, 0, GlobalText[3508], 1, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 137, 15, 15, 0, GlobalText[3509], 2, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 94, m_Pos.y + 174, 15, 15, 0, GlobalText[3510], 3, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 94, m_Pos.y + 191, 15, 15, 0, GlobalText[3511], 4, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 94, m_Pos.y + 226, 15, 15, 0, GlobalText[3510], 5, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 94, m_Pos.y + 243, 15, 15, 0, GlobalText[3511], 6, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 226, 15, 15, 0, GlobalText[3512], 7, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 25, m_Pos.y + 276, 15, 15, 0, GlobalText[3513], 8, 0);

    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 60, m_Pos.y + 218, 15, 15, 0, GlobalText[3514], 9, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 15, m_Pos.y + 218, 15, 15, 0, GlobalText[3515], 10, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 79, m_Pos.y + 97, 15, 15, 0, GlobalText[3516], 11, 0);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 79, m_Pos.y + 97, 15, 15, 0, GlobalText[3517], 12, 0);
    g_ConsoleDebug->Write(MCD_NORMAL, L"Check box  %s", GlobalText[3517]);

    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 79, m_Pos.y + 80, 15, 15, 0, GlobalText[3518], 13, 1);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 17, m_Pos.y + 125, 15, 15, 0, GlobalText[3519], 14, 1);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 17, m_Pos.y + 152, 15, 15, 0, GlobalText[3520], 15, 1);

    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 22, m_Pos.y + 170, 15, 15, 0, GlobalText[3521], 16, 1);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 85, m_Pos.y + 170, 15, 15, 0, GlobalText[3522], 17, 1);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 22, m_Pos.y + 185, 15, 15, 0, GlobalText[3523], 18, 1);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 85, m_Pos.y + 185, 15, 15, 0, GlobalText[3524], 19, 1);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 22, m_Pos.y + 200, 15, 15, 0, GlobalText[3525], 20, 1);
    //--
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 80, 15, 15, 0, GlobalText[3591], 21, 2);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 97, 15, 15, 0, GlobalText[3592], 23, 2);
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 125, 15, 15, 0, GlobalText[3594], 22, 2);

    RegisterBoxCharacter(0xFF, 0);
    RegisterBoxCharacter(0xFF, 1);
    RegisterBoxCharacter(0xFF, 2);
    RegisterBoxCharacter(0xFF, 3);
    RegisterBoxCharacter(0xFF, 4);
    RegisterBoxCharacter(0xFF, 8);
    RegisterBoxCharacter(0xFF, 13);
    RegisterBoxCharacter(0xFF, 14);
    RegisterBoxCharacter(0xFF, 15);
    RegisterBoxCharacter(0xFF, 16);
    RegisterBoxCharacter(0xFF, 17);
    RegisterBoxCharacter(0xFF, 19);
    RegisterBoxCharacter(0xFF, 18);
    RegisterBoxCharacter(0xFF, 20);
    RegisterBoxCharacter(0xFF, 21);
    RegisterBoxCharacter(0xFF, 23);
    RegisterBoxCharacter(0xFF, 22);

    RegisterBoxCharacter(Dark_Knight, 5);
    RegisterBoxCharacter(Dark_Knight, 6);
    RegisterBoxCharacter(Dark_Knight, 7);

    RegisterBoxCharacter(Dark_Wizard, 5);
    RegisterBoxCharacter(Dark_Wizard, 6);
    RegisterBoxCharacter(Dark_Wizard, 10);

    RegisterBoxCharacter(Magic_Gladiator, 5);
    RegisterBoxCharacter(Magic_Gladiator, 6);

    RegisterBoxCharacter(Dark_Lord, 9);

    RegisterBoxCharacter(Fairy_Elf, 11);
    RegisterBoxCharacter(Fairy_Elf, 5);
    RegisterBoxCharacter(Fairy_Elf, 6);
    RegisterBoxCharacter(Fairy_Elf, 10);

    RegisterBoxCharacter(Summoner, 5);
    RegisterBoxCharacter(Summoner, 6);
    RegisterBoxCharacter(Summoner, 12);

    RegisterBoxCharacter(Rage_Fighter, 5);
    RegisterBoxCharacter(Rage_Fighter, 6);
}

void SEASON3B::CNewUIMuHelper::InitImage()
{
    InsertIcon(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 17, m_Pos.y + 171, 32, 38, 0, 0);
    InsertIcon(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 61, m_Pos.y + 171, 32, 38, 1, 0);
    InsertIcon(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 61, m_Pos.y + 222, 32, 38, 2, 0);
    InsertIcon(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 21, m_Pos.y + 293, 32, 38, 3, 0);
    InsertIcon(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 55, m_Pos.y + 293, 32, 38, 4, 0);
    InsertIcon(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 89, m_Pos.y + 293, 32, 38, 5, 0);

    InsertIcon(IMAGE_MACROUI_HELPER_INPUTNUMBER, m_Pos.x + 135, m_Pos.y + 137, 28, 15, 6, 0);
    InsertIcon(IMAGE_MACROUI_HELPER_INPUTNUMBER, m_Pos.x + 135, m_Pos.y + 174, 28, 15, 7, 0);
    InsertIcon(IMAGE_MACROUI_HELPER_INPUTNUMBER, m_Pos.x + 135, m_Pos.y + 226, 28, 15, 8, 0);
    InsertIcon(IMAGE_MACROUI_HELPER_INPUTSTRING, m_Pos.x + 34, m_Pos.y + 216, 93, 15, 9, 1);

    RegisterIconCharacter(0xFF, 0);
    RegisterIconCharacter(0xFF, 1);
    RegisterIconCharacter(0xFF, 3);
    RegisterIconCharacter(0xFF, 4);
    RegisterIconCharacter(0xFF, 5);
    RegisterIconCharacter(0xFF, 6);
    RegisterIconCharacter(0xFF, 7);
    RegisterIconCharacter(0xFF, 9);

    RegisterIconCharacter(Dark_Knight, 2);
    RegisterIconCharacter(Dark_Knight, 8);
    RegisterIconCharacter(Dark_Wizard, 2);
    RegisterIconCharacter(Dark_Wizard, 8);
    RegisterIconCharacter(Fairy_Elf, 2);
    RegisterIconCharacter(Fairy_Elf, 8);
    RegisterIconCharacter(Magic_Gladiator, 2);
    RegisterIconCharacter(Magic_Gladiator, 8);
    RegisterIconCharacter(Summoner, 2);
    RegisterIconCharacter(Summoner, 8);
    RegisterIconCharacter(Rage_Fighter, 2);
    RegisterIconCharacter(Rage_Fighter, 8);
}

void SEASON3B::CNewUIMuHelper::InitText()
{
    InsertText(m_Pos.x + 18, m_Pos.y + 78, GlobalText[3526], 1, 0);
    InsertText(m_Pos.x + 18, m_Pos.y + 83, L"________", 2, 0);
    InsertText(m_Pos.x + 113, m_Pos.y + 141, GlobalText[3527], 3, 0);
    InsertText(m_Pos.x + 162, m_Pos.y + 141, GlobalText[3528], 4, 0);
    InsertText(m_Pos.x + 18, m_Pos.y + 160, GlobalText[3529], 5, 0);
    InsertText(m_Pos.x + 59, m_Pos.y + 160, GlobalText[3530], 7, 0);
    InsertText(m_Pos.x + 162, m_Pos.y + 178, GlobalText[3528], 8, 0);
    InsertText(m_Pos.x + 59, m_Pos.y + 212, GlobalText[3531], 9, 0);

    InsertText(m_Pos.x + 162, m_Pos.y + 230, GlobalText[3528], 10, 0);
    InsertText(m_Pos.x + 18, m_Pos.y + 78, GlobalText[3532], 11, 1);
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

bool SEASON3B::CNewUIMuHelper::Update()
{
    if (IsVisible())
    {
        int iNumCurOpenTab = m_TabBtn.UpdateMouseEvent();

        if (iNumCurOpenTab == RADIOGROUPEVENT_NONE)
            return true;

        m_iNumCurOpenTab = iNumCurOpenTab;
    }
    return true;
}

bool SEASON3B::CNewUIMuHelper::UpdateMouseEvent()
{
    // Ignore events outside MU Helper window
    if (!CheckMouseIn(m_Pos.x, m_Pos.y, WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        return true;
    }

    int iButtonId = UpdateMouseBtnList();
    if (iButtonId != -1)
    {
        g_ConsoleDebug->Write(MCD_NORMAL, L"Clicked button [%d]", iButtonId);

        if (iButtonId == BUTTON_ID_EXIT_CONFIG)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_MUHELPER);
        }
        else if (iButtonId == BUTTON_ID_INIT_CONFIG)
        {
            InitConfig();
        }
        else if (iButtonId == BUTTON_ID_SAVE_CONFIG)
        {
            SaveConfig();
        }

        return false;
    }

    int iCheckboxId = UpdateMouseBoxList();
    if (iCheckboxId != -1)
    {
        auto element = m_CheckBoxList[iCheckboxId];
        auto state = element.box->GetBoxState();
        g_ConsoleDebug->Write(MCD_NORMAL, L"Clicked checkbox [%d] state[%d]", iCheckboxId, state);
        ApplyConfigFromCheckbox(iCheckboxId, state);

        return false;
    }

    if (IsRelease(VK_LBUTTON))
    {
        int iSlotIndex = UpdateMouseIconList();
        if (iSlotIndex != -1 && iSlotIndex < MAX_SKILLS_SLOT)
        {
            g_ConsoleDebug->Write(MCD_NORMAL, L"Clicked skill slot [%d]", iSlotIndex);
            m_iSelectedSkillSlot = iSlotIndex;

            if (iSlotIndex >= 0 && iSlotIndex < 3)
            {
                g_pNewUIMuHelperSkillList->FilterByAttackSkills();
            }
            else
            {
                g_pNewUIMuHelperSkillList->FilterByBuffSkills();
            }

            g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MUHELPER_SKILL_LIST);
            return false;
        }
    }
    if (IsRelease(VK_RBUTTON))
    {
        int iSlotIndex = UpdateMouseIconList();
        if (iSlotIndex != -1)
        {
            g_ConsoleDebug->Write(MCD_NORMAL, L"Clicked slot slot [%d]", iSlotIndex);
            m_aiSelectedSkills[iSlotIndex] = -1;
            return false;
        }
    }

    return false;
}

bool SEASON3B::CNewUIMuHelper::UpdateKeyEvent()
{
    if (IsVisible())
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_MUHELPER);
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_MUHELPER_SKILL_LIST);
            //PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }
    return true;
}

void SEASON3B::CNewUIMuHelper::ApplyConfigFromCheckbox(int iCheckboxId, bool bState)
{
    switch (iCheckboxId) {
    case CHECKBOX_ID_POTION:
        m_TempConfig.bUseHealPotion = bState;
        break;

    case CHECKBOX_ID_LONG_DISTANCE:
        m_TempConfig.bLongDistanceCounterAttack = bState;
        break;

    case CHECKBOX_ID_ORIG_POSITION:
        m_TempConfig.bOriginalPosition = bState;
        break;

    case CHECKBOX_ID_SKILL2_DELAY:
        m_TempConfig.bSkill2UseDelay = bState;
        break;

    case CHECKBOX_ID_SKILL2_CONDITION:
        m_TempConfig.bSkill2UseCondition = bState;
        break;

    case CHECKBOX_ID_SKILL3_DELAY:
        m_TempConfig.bSkill3UseDelay = bState;
        break;

    case CHECKBOX_ID_SKILL3_CONDITION:
        m_TempConfig.bSkill3UseCondition = bState;
        break;

    case CHECKBOX_ID_COMBO:
        m_TempConfig.bUseCombo = bState;
        break;

    case CHECKBOX_ID_BUFF_DURATION:
        m_TempConfig.bBuffDuration = bState;
        break;

    case CHECKBOX_ID_USE_PET:
        m_TempConfig.bUsePet = bState;
        break;

    case CHECKBOX_ID_PARTY:
        m_TempConfig.bSupportParty = bState;
        break;

    case CHECKBOX_ID_AUTO_HEAL:
        m_TempConfig.bAutoHeal = bState;
        break;

    case CHECKBOX_ID_DRAIN_LIFE:
        m_TempConfig.bUseDrainLife = bState;
        break;

    case CHECKBOX_ID_REPAIR_ITEM:
        m_TempConfig.bRepairItem = bState;
        break;

    case CHECKBOX_ID_PICK_ALL:
        m_TempConfig.bPickAllItems = bState;
        break;

    case CHECKBOX_ID_PICK_SELECTED:
        m_TempConfig.bPickSelectItems = bState;
        break;

    case CHECKBOX_ID_PICK_JEWEL:
        m_TempConfig.bPickJewel = bState;
        break;

    case CHECKBOX_ID_PICK_ANCIENT:
        m_TempConfig.bPickAncient = bState;
        break;

    case CHECKBOX_ID_PICK_ZEN:
        m_TempConfig.bPickZen = bState;
        break;

    case CHECKBOX_ID_PICK_EXCELLENT:
        m_TempConfig.bPickExcellent = bState;
        break;

    case CHECKBOX_ID_ADD_OTHER_ITEM:
        m_TempConfig.bPickExtraItems = bState;
        break;

    default:
        break;
    }
}

void SEASON3B::CNewUIMuHelper::ApplyConfigFromSkillSlot(int iSlot, int iSkill)
{
    switch (iSlot)
    {
    case SKILL_SLOT_SKILL1:
        m_TempConfig.iBasicSkill = iSkill;
        break;
    case SKILL_SLOT_SKILL2:
        m_TempConfig.iSkill2 = iSkill;
        break;
    case SKILL_SLOT_SKILL3:
        m_TempConfig.iSkill3 = iSkill;
        break;
    case SKILL_SLOT_BUFF1:
        m_TempConfig.iBuff1 = iSkill;
        break;
    case SKILL_SLOT_BUFF2:
        m_TempConfig.iBuff2 = iSkill;
        break;
    case SKILL_SLOT_BUFF3:
        m_TempConfig.iBuff3 = iSkill;
        break;
    }
}

void SEASON3B::CNewUIMuHelper::InitConfig()
{
    ResetBoxList();
    ResetSelectedSkills();

    memset(&m_TempConfig, 0, sizeof(m_TempConfig));
}

void SEASON3B::CNewUIMuHelper::SaveConfig()
{
    g_MuHelper.Save(m_TempConfig);
}

float SEASON3B::CNewUIMuHelper::GetLayerDepth()
{
    return 3.4;
}

float SEASON3B::CNewUIMuHelper::GetKeyEventOrder()
{
    return 3.4;
}

bool SEASON3B::CNewUIMuHelper::Render()
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

    int sTapIndex = m_iNumCurOpenTab;

    m_TabBtn.Render();

    if (sTapIndex)
    {
        if (sTapIndex == 1)
        {
            RenderBack(m_Pos.x + 12, m_Pos.y + 73, 68, 50);
            RenderBack(m_Pos.x + 75, m_Pos.y + 73, 102, 50);
            RenderBack(m_Pos.x + 12, m_Pos.y + 120, 165, 30);
            RenderBack(m_Pos.x + 12, m_Pos.y + 147, 165, 195);
            RenderBack(m_Pos.x + 16, m_Pos.y + 235, 158, 74);

            RenderImage(BITMAP_INTERFACE_CRYWOLF_BEGIN + 34, m_Pos.x + 29, m_Pos.y + 92, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f);
        }
        else
        {
            RenderBack(m_Pos.x + 12, m_Pos.y + 73, 165, 50);
            RenderBack(m_Pos.x + 12, m_Pos.y + 120, 165, 222);
        }
    }
    else
    {
        RenderBack(m_Pos.x + 12, m_Pos.y + 73, 68, 50);
        RenderBack(m_Pos.x + 75, m_Pos.y + 73, 102, 50);
        RenderBack(m_Pos.x + 12, m_Pos.y + 120, 165, 39);
        RenderBack(m_Pos.x + 12, m_Pos.y + 156, 165, 120);
        RenderBack(m_Pos.x + 12, m_Pos.y + 273, 165, 69);

        RenderImage(BITMAP_INTERFACE_CRYWOLF_BEGIN + 33, m_Pos.x + 29, m_Pos.y + 92, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f);
    }

    RenderBoxList();
    RenderIconList();
    RenderTextList();
    RenderBtnList();
    DisableAlphaBlend();

    return true;
}

void SEASON3B::CNewUIMuHelper::RenderBack(int x, int y, int width, int height)
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

void SEASON3B::CNewUIMuHelper::LoadImages()
{
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_RangeMinus.tga", IMAGE_MACROUI_HELPER_RAGEMINUS, GL_LINEAR, GL_CLAMP, 1, 0);
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_OptionButton.tga", IMAGE_MACROUI_HELPER_OPTIONBUTTON, GL_LINEAR, GL_CLAMP, 1, 0);
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_InputNumber.tga", IMAGE_MACROUI_HELPER_INPUTNUMBER, GL_LINEAR, GL_CLAMP, 1, 0);
    LoadBitmap(L"Interface\\MacroUI\\MacroUI_InputString.tga", IMAGE_MACROUI_HELPER_INPUTSTRING, GL_LINEAR, GL_CLAMP, 1, 0);
    //--
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Bt03.tga", IMAGE_IGS_BUTTON, GL_LINEAR, GL_CLAMP, 1, 0);
}

void SEASON3B::CNewUIMuHelper::UnloadImages()
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

void SEASON3B::CNewUIMuHelper::RegisterButton(int Identifier, CButtonTap button)
{
    m_ButtonList.insert(std::pair<int, CButtonTap>(Identifier, button));
}

void SEASON3B::CNewUIMuHelper::RegisterBtnCharacter(BYTE class_character, int Identifier)
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

void SEASON3B::CNewUIMuHelper::InsertButton(int imgindex, int x, int y, int sx, int sy, bool overflg, bool isimgwidth, bool bClickEffect, bool MoveTxt,std::wstring btname,std::wstring tooltiptext, int Identifier, int iNumTab)
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

void SEASON3B::CNewUIMuHelper::RenderBtnList()
{
    auto li = m_ButtonList.begin();

    for (; li != m_ButtonList.end(); li++)
    {
        CButtonTap* cBTN = &li->second;

        if ((cBTN->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cBTN->iNumTab == m_iNumCurOpenTab || cBTN->iNumTab == -1))
        {
            cBTN->btn->Render();
        }
    }
}

int SEASON3B::CNewUIMuHelper::UpdateMouseBtnList()
{
    auto li = m_ButtonList.begin();

    for (; li != m_ButtonList.end(); li++)
    {
        CButtonTap* cBTN = &li->second;

        if ((cBTN->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cBTN->iNumTab == m_iNumCurOpenTab || cBTN->iNumTab == -1))
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

void SEASON3B::CNewUIMuHelper::RegisterBoxCharacter(BYTE class_character, int Identifier)
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

void SEASON3B::CNewUIMuHelper::RegisterCheckBox(int Identifier, CheckBoxTap button)
{
    m_CheckBoxList.insert(std::pair<int, CheckBoxTap>(Identifier, button));
}

void SEASON3B::CNewUIMuHelper::InsertCheckBox(int imgindex, int x, int y, int sx, int sy, bool overflg,std::wstring btname, int Identifier, int iNumTab)
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

void SEASON3B::CNewUIMuHelper::RenderBoxList()
{
    auto li = m_CheckBoxList.begin();

    for (; li != m_CheckBoxList.end(); li++)
    {
        CheckBoxTap* cBOX = &li->second;

        if ((cBOX->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cBOX->iNumTab == m_iNumCurOpenTab || cBOX->iNumTab == -1))
        {
            cBOX->box->Render();
        }
    }
}

void SEASON3B::CNewUIMuHelper::ResetBoxList()
{
    auto li = m_CheckBoxList.begin();

    for (; li != m_CheckBoxList.end(); li++)
    {
        CheckBoxTap* cBOX = &li->second;
        cBOX->box->RegisterBoxState(false);
    }
}

void SEASON3B::CNewUIMuHelper::ResetSelectedSkills()
{
    for (int i = 0; i < MAX_SKILLS_SLOT; i++)
    {
        m_aiSelectedSkills[i] = -1;
    }
}

int SEASON3B::CNewUIMuHelper::UpdateMouseBoxList()
{
    auto li = m_CheckBoxList.begin();

    for (; li != m_CheckBoxList.end(); li++)
    {
        CheckBoxTap* cBOX = &li->second;

        if ((cBOX->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cBOX->iNumTab == m_iNumCurOpenTab || cBOX->iNumTab == -1))
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

void SEASON3B::CNewUIMuHelper::RenderIconList()
{
    auto li = m_IconList.begin();

    for (; li != m_IconList.end(); li++)
    {
        cTexture* cImage = &li->second;

        if ((cImage->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cImage->iNumTab == m_iNumCurOpenTab || cImage->iNumTab == -1))
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

int SEASON3B::CNewUIMuHelper::UpdateMouseIconList()
{
    auto li = m_IconList.begin();

    for (; li != m_IconList.end(); li++)
    {
        cTexture* cImage = &li->second;

        if ((cImage->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cImage->iNumTab == m_iNumCurOpenTab || cImage->iNumTab == -1))
        {
            if (CheckMouseIn(cImage->m_Pos.x, cImage->m_Pos.y, cImage->m_Size.x, cImage->m_Size.y))
            {
                return li->first;
            }
        }
    }

    return -1;
}

void SEASON3B::CNewUIMuHelper::RegisterIconCharacter(BYTE class_character, int Identifier)
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

void SEASON3B::CNewUIMuHelper::RegisterIcon(int Identifier, cTexture button)
{
    m_IconList.insert(std::pair<int, cTexture>(Identifier, button));
}

void SEASON3B::CNewUIMuHelper::InsertIcon(int imgindex, int x, int y, int sx, int sy, int Identifier, int iNumTab)
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

void SEASON3B::CNewUIMuHelper::RenderTextList()
{
    auto li = m_TextNameList.begin();

    for (; li != m_TextNameList.end(); li++)
    {
        cTextName* cImage = &li->second;

        if ((cImage->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cImage->iNumTab == m_iNumCurOpenTab || cImage->iNumTab == -1))
        {
            g_pRenderText->RenderText(cImage->m_Pos.x, cImage->m_Pos.y, cImage->m_Name.c_str());
        }
    }
}

void SEASON3B::CNewUIMuHelper::RegisterTextCharacter(BYTE class_character, int Identifier)
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

void SEASON3B::CNewUIMuHelper::RegisterTextur(int Identifier, cTextName button)
{
    m_TextNameList.insert(std::pair<int, cTextName>(Identifier, button));
}

void SEASON3B::CNewUIMuHelper::InsertText(int x, int y,std::wstring Name, int Identifier, int iNumTab)
{
    cTextName cText;

    cText.m_Pos.x = x;
    cText.m_Pos.y = y;
    cText.m_Name = Name;
    cText.iNumTab = iNumTab;

    memset(cText.class_character, 0, sizeof(cText.class_character));
    RegisterTextur(Identifier, cText);
}

void SEASON3B::CNewUIMuHelper::AssignSkill(int iSkill)
{
    if (m_iSelectedSkillSlot != -1 && m_iSelectedSkillSlot < MAX_SKILLS_SLOT)
    {
        m_aiSelectedSkills[m_iSelectedSkillSlot] = iSkill;
        ApplyConfigFromSkillSlot(m_iSelectedSkillSlot, iSkill);

        g_ConsoleDebug->Write(MCD_NORMAL, L"Assign m_aiSelectedSkills[%d] = %d", m_iSelectedSkillSlot, iSkill);
    }
}

void SEASON3B::CNewUIMuHelper::RenderSkillIcon(int skill, float x, float y, float width, float height)
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
    else if (skill == AT_SKILL_ALICE_BERSERKER)
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
    else if (skill == AT_SKILL_BLOW_OF_DESTRUCTION)
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
    else if (AT_SKILL_LIGHTNING_SHOCK_UP <= skill && skill <= AT_SKILL_LIGHTNING_SHOCK_UP + 4)
    {
        fU = 6 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (skill == AT_SKILL_SWELL_OF_MAGICPOWER)
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
    else if (skill >= AT_SKILL_THRUST)
    {
        fU = ((skill - 260) % 12) * width / 256.f;
        fV = ((skill - 260) / 12) * height / 256.f;
        iKindofSkill = KOS_SKILL3;
    }
    else if (skill >= 57)
    {
        fU = ((skill - 57) % 8) * width / 256.f;
        fV = ((skill - 57) / 8) * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else
    {
        fU = ((skill - 1) % 8) * width / 256.f;
        fV = ((skill - 1) / 8) * height / 256.f;
        iKindofSkill = KOS_SKILL1;
    }
    int iSkillIndex = 0;
    switch (iKindofSkill)
    {
    case KOS_COMMAND:
    {
        iSkillIndex = IMAGE_COMMAND;
    }break;
    case KOS_SKILL1:
    {
        iSkillIndex = IMAGE_SKILL1;
    }break;
    case KOS_SKILL2:
    {
        iSkillIndex = IMAGE_SKILL2;
    }break;
    case KOS_SKILL3:
    {
        iSkillIndex = IMAGE_SKILL3;
    }break;
    }

    if (iSkillIndex != 0)
    {
        RenderBitmap(iSkillIndex, x, y, width, height, fU, fV, width / 256.f, height / 256.f);
    }
}

SEASON3B::CNewUIMuHelperSkillList::CNewUIMuHelperSkillList()
{
    m_pNewUIMng = NULL;
    Reset();
}

SEASON3B::CNewUIMuHelperSkillList::~CNewUIMuHelperSkillList()
{
    Release();
}


bool SEASON3B::CNewUIMuHelperSkillList::Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MUHELPER_SKILL_LIST, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;

    LoadImages();

    Show(false);

    return true;
}

void SEASON3B::CNewUIMuHelperSkillList::Release()
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

void SEASON3B::CNewUIMuHelperSkillList::Reset()
{
    m_bRenderSkillInfo = false;
    m_iRenderSkillInfoType = 0;
    m_iRenderSkillInfoPosX = 0;
    m_iRenderSkillInfoPosY = 0;

    m_EventState = EVENT_NONE;
}

void SEASON3B::CNewUIMuHelperSkillList::LoadImages()
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

void SEASON3B::CNewUIMuHelperSkillList::UnloadImages()
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

bool SEASON3B::CNewUIMuHelperSkillList::UpdateMouseEvent()
{
    if (IsRelease(VK_LBUTTON))
    {
        int skillId = UpdateMouseSkillList();
        if (skillId != -1)
        {
            g_ConsoleDebug->Write(MCD_NORMAL, L"Clicked skill [%d]", skillId);
            g_pNewUIMuHelper->AssignSkill(skillId);
            Show(false);
            return false;
        }

        return true;
    }

    return false;
}

bool SEASON3B::CNewUIMuHelperSkillList::UpdateKeyEvent()
{
    if (IsVisible())
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_MUHELPER_SKILL_LIST);
            //PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }
    return true;
}

int SEASON3B::CNewUIMuHelperSkillList::GetSkillIndex(int iSkillType)
{
    int iReturn = -1;
    for (int i = 0; i < MAX_MAGIC; ++i)
    {
        if (CharacterAttribute->Skill[i] == iSkillType)
        {
            iReturn = i;
            break;
        }
    }

    return iReturn;
}

bool SEASON3B::CNewUIMuHelperSkillList::Update()
{
    return true;
}

bool SEASON3B::CNewUIMuHelperSkillList::Render()
{
    BYTE skillNumber = CharacterAttribute->SkillNumber;
    if (skillNumber > 0)
    {
        float x = 300, y = 171, width = 32, height = 38;
        float fOrigX = 300;
        int iSkillCount = 0;

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

                if ((m_bFilterByAttackSkills && IsAttackSkill(iSkillType)) ||
                    (m_bFilterByBuffSkills && IsBuffSkill(iSkillType)))
                {
                    if (iSkillCount == 18)
                    {
                        y -= height;
                    }

                    if (iSkillCount < 14)
                    {
                        int iRemainder = iSkillCount % 2;
                        int iQuotient = iSkillCount / 2;

                        if (iRemainder == 0)
                        {
                            x = fOrigX + iQuotient * width;
                        }
                        else
                        {
                            x = fOrigX - (iQuotient + 1) * width;
                        }
                    }
                    else if (iSkillCount >= 14 && iSkillCount < 18)
                    {
                        x = fOrigX - (8 * width) - ((iSkillCount - 14) * width);
                    }
                    else
                    {
                        x = fOrigX - (12 * width) + ((iSkillCount - 17) * width);
                    }

                    iSkillCount++;

                    SEASON3B::RenderImage(IMAGE_SKILLBOX, x, y, width, height);
                    RenderSkillIcon(i, x + 6, y + 6, 20, 28);

                    m_skillIconMap.insert_or_assign(iSkillType, cSkillIcon{ iSkillType, { static_cast<LONG>(x), static_cast<LONG>(y) }, { static_cast<LONG>(width), static_cast<LONG>(height) } });
                }
                else
                {
                    // Filtered out list will not be rendered
                    m_skillIconMap.insert_or_assign(iSkillType, cSkillIcon{ iSkillType, { 0, 0 }, { 0, 0 } });
                }
            }
        }
    }

    if (m_bRenderSkillInfo == true && m_pNewUI3DRenderMng)
    {
        m_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, 0, 0);

        m_bRenderSkillInfo = false;
    }

    return true;
}

void SEASON3B::CNewUIMuHelperSkillList::RenderSkillInfo()
{
    ::RenderSkillInfo(m_iRenderSkillInfoPosX + 15, m_iRenderSkillInfoPosY - 10, m_iRenderSkillInfoType);
}

float SEASON3B::CNewUIMuHelperSkillList::GetLayerDepth()
{
    return 5.2f;
}

void SEASON3B::CNewUIMuHelperSkillList::RenderSkillIcon(int iIndex, float x, float y, float width, float height)
{
    auto bySkillType = CharacterAttribute->Skill[iIndex];
    if (bySkillType == 0)
    {
        return;
    }

    float fU, fV;
    int iKindofSkill = 0;

    BYTE bySkillUseType = SkillAttribute[bySkillType].SkillUseType;
    int Skill_Icon = SkillAttribute[bySkillType].Magic_Icon;

    if (bySkillType >= AT_PET_COMMAND_DEFAULT && bySkillType <= AT_PET_COMMAND_END)
    {
        fU = ((bySkillType - AT_PET_COMMAND_DEFAULT) % 8) * width / 256.f;
        fV = ((bySkillType - AT_PET_COMMAND_DEFAULT) / 8) * height / 256.f;
        iKindofSkill = KOS_COMMAND;
    }
    else if (bySkillType == AT_SKILL_PLASMA_STORM_FENRIR)
    {
        fU = 4 * width / 256.f;
        fV = 0.f;
        iKindofSkill = KOS_COMMAND;
    }
    else if ((bySkillType >= AT_SKILL_ALICE_DRAINLIFE && bySkillType <= AT_SKILL_ALICE_THORNS))
    {
        fU = ((bySkillType - AT_SKILL_ALICE_DRAINLIFE) % 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType >= AT_SKILL_ALICE_SLEEP && bySkillType <= AT_SKILL_ALICE_BLIND)
    {
        fU = ((bySkillType - AT_SKILL_ALICE_SLEEP + 4) % 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_ALICE_BERSERKER)
    {
        fU = 10 * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType >= AT_SKILL_ALICE_WEAKNESS && bySkillType <= AT_SKILL_ALICE_ENERVATION)
    {
        fU = (bySkillType - AT_SKILL_ALICE_WEAKNESS + 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType >= AT_SKILL_SUMMON_EXPLOSION && bySkillType <= AT_SKILL_SUMMON_REQUIEM)
    {
        fU = ((bySkillType - AT_SKILL_SUMMON_EXPLOSION + 6) % 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_SUMMON_POLLUTION)
    {
        fU = 11 * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_BLOW_OF_DESTRUCTION)
    {
        fU = 7 * width / 256.f;
        fV = 2 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_GAOTIC)
    {
        fU = 3 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_RECOVER)
    {
        fU = 9 * width / 256.f;
        fV = 2 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_MULTI_SHOT)
    {
        fU = 0 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_FLAME_STRIKE)
    {
        int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
        int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

        fU = 1 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_GIGANTIC_STORM)
    {
        fU = 2 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_LIGHTNING_SHOCK)
    {
        fU = 2 * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (AT_SKILL_LIGHTNING_SHOCK_UP <= bySkillType && bySkillType <= AT_SKILL_LIGHTNING_SHOCK_UP + 4)
    {
        fU = 6 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_SWELL_OF_MAGICPOWER)
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
    else if (bySkillType >= AT_SKILL_THRUST)
    {
        fU = ((bySkillType - 260) % 12) * width / 256.f;
        fV = ((bySkillType - 260) / 12) * height / 256.f;
        iKindofSkill = KOS_SKILL3;
    }
    else if (bySkillType >= 57)
    {
        fU = ((bySkillType - 57) % 8) * width / 256.f;
        fV = ((bySkillType - 57) / 8) * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else
    {
        fU = ((bySkillType - 1) % 8) * width / 256.f;
        fV = ((bySkillType - 1) / 8) * height / 256.f;
        iKindofSkill = KOS_SKILL1;
    }
    int iSkillIndex = 0;
    switch (iKindofSkill)
    {
    case KOS_COMMAND:
    {
        iSkillIndex = IMAGE_COMMAND;
    }break;
    case KOS_SKILL1:
    {
        iSkillIndex = IMAGE_SKILL1;
    }break;
    case KOS_SKILL2:
    {
        iSkillIndex = IMAGE_SKILL2;
    }break;
    case KOS_SKILL3:
    {
        iSkillIndex = IMAGE_SKILL3;
    }break;
    }

    if (iSkillIndex != 0)
    {
        RenderBitmap(iSkillIndex, x, y, width, height, fU, fV, width / 256.f, height / 256.f);
    }
}

bool SEASON3B::CNewUIMuHelperSkillList::IsAttackSkill(int iSkillType)
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

bool SEASON3B::CNewUIMuHelperSkillList::IsBuffSkill(int iSkillType)
{
    switch (iSkillType)
    {
    // Knight Buffs
    // To-do: Add other buffs for knights
    case AT_SKILL_VITALITY:

        return true;

    // Elf Buffs
    // To-do: Add other buffs for elves (e.g. master skills)
    case AT_SKILL_DEFENSE:
    case AT_SKILL_ATTACK:
        return true;

    // Wizard Buffs
    // To-do: Add other buffs for wizards (e.g. master skills)
    case AT_SKILL_WIZARDDEFENSE:
        return true;

    // Dark Lord Buffs
    // To-do: Add other buffs for dark lords (e.g. master skills)
    case AT_SKILL_ADD_CRITICAL:
        return true;

        // To-do: Add other character buffs
    }

    return false;
}

bool SEASON3B::CNewUIMuHelperSkillList::IsHealingSkill(int iSkillType)
{
    switch (iSkillType)
    {
    case AT_SKILL_HEALING:
        return true;
    }

    return false;
}

bool SEASON3B::CNewUIMuHelperSkillList::IsDefenseSkill(int iSkillType)
{
    switch (iSkillType)
    {
    case AT_SKILL_DEFENSE:
        return true;
    }

    return false;
}

bool is_empty_or_whitespace(const wchar_t* name) {
    for (size_t i = 0; i < wcslen(name); ++i) {
        if (!iswspace(name[i])) {
            return false;
        }
    }
    return true;
}

void SEASON3B::CNewUIMuHelperSkillList::FilterByAttackSkills()
{
    m_bFilterByAttackSkills = true;
    m_bFilterByBuffSkills = false;
}

void SEASON3B::CNewUIMuHelperSkillList::FilterByBuffSkills()
{
    m_bFilterByBuffSkills = true;
    m_bFilterByAttackSkills = false;
}

void SEASON3B::CNewUIMuHelperSkillList::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
    if (pClass)
    {
        auto* pSkillList = (CNewUIMuHelperSkillList*)(pClass);
        pSkillList->RenderSkillInfo();
    }
}

int SEASON3B::CNewUIMuHelperSkillList::UpdateMouseSkillList()
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