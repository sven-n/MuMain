#include "stdafx.h"
#include "UI/MuHelper/MuHelperConfigWindow.h"

#include "Character/CharacterManager.h"
#include "Core/Utilities/StringUtils.h"
#include "Data/GameData/ItemData/ItemStructs.h"
#include "I18N/All.h"
#include "MUHelper/MuHelper.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Core/WindowManager.h"
#include "UI/Core/WindowSystem.h"
#include "UI/HUD/Skills/SkillIconCell.h"
#include "UI/MuHelper/MuHelperDetailWindow.h"
#include "UI/MuHelper/MuHelperSkillPicker.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlKeyboardFocus.h"
#include "UI/RmlBridge/RmlNumericInputFilter.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

#include <algorithm>
#include <string>

using namespace mu::ui::window;
using namespace MUHelper;

namespace
{
    constexpr int MaxHuntingRange = 6;
    constexpr int MaxObtainingRange = 8;

    constexpr const char* NumberFieldIds[] = { "distance_time", "skill2_delay", "skill3_delay" };

    // The extra-item list's own reference rect (mu_helper_config.rcss). The wheel is only claimed
    // over it, as native's list did -- elsewhere over the panel it still reaches the camera.
    constexpr int ExtraListLeft = 20, ExtraListTop = 238, ExtraListWidth = 160, ExtraListHeight = 70;

    Rml::String Narrow(const wchar_t* text) { return StringUtils::WideToNarrow(text); }

    int BaseClass()
    {
        return Hero ? gCharacterManager.GetBaseClass(Hero->Class) : -1;
    }

    Rml::String SlotDecorator(int skill)
    {
        if (skill <= 0)
            return "none";
        const std::string sprite = UI::Skills::IconSpriteName(UI::Skills::ResolveIconCell(skill));
        return sprite.empty() ? Rml::String("none") : "image(" + sprite + ")";
    }

    void SetConditionMode(uint32_t& bits, bool timer, bool enable)
    {
        // Timer and condition exclude each other; either click clears the other first, as native.
        bits &= timer ? ~static_cast<uint32_t>(ON_CONDITION) : ~static_cast<uint32_t>(ON_TIMER);
        const uint32_t flag = timer ? static_cast<uint32_t>(ON_TIMER) : static_cast<uint32_t>(ON_CONDITION);
        bits = enable ? (bits | flag) : (bits & ~flag);
    }
}

CMuHelperConfigWindow::CMuHelperConfigWindow()
{
    m_aiSelectedSkills.fill(-1);
}

CMuHelperConfigWindow::~CMuHelperConfigWindow()
{
    Release();
}

bool CMuHelperConfigWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (pNewUIMng == nullptr)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(INTERFACE_MUHELPER, this);

    m_Pos.x = x;
    m_Pos.y = y;

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    const ConfigData& config = UI::MuHelper::StagedConfig();
    MuHelperConfigRmlModel& model = m_RmlBinder.GetModel();
    model.distanceTime = std::to_string(config.iMaxSecondsAway);
    model.skill2Delay = std::to_string(config.aiSkillInterval[1]);
    model.skill3Delay = std::to_string(config.aiSkillInterval[2]);

    Show(false);

    return true;
}

void CMuHelperConfigWindow::Release()
{
    if (m_pRmlDoc)
    {
        m_pRmlDoc->Close();
        m_pRmlDoc = nullptr;
    }

    if (m_pNewUIMng)
    {
        UI::RmlBridge::UnregisterForThemeReload(this);
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }
}

void CMuHelperConfigWindow::Show(bool bShow)
{
    if (!bShow)
    {
        // A focused field in a hidden document would keep every window's key handling suspended
        // (RmlUiRuntime::IsTextInputActive()), so release it on every way out.
        BlurFocusedField();

        if (g_pMuHelperDetail)
            g_pMuHelperDetail->Show(false);
        if (g_pMuHelperSkillPicker)
            g_pMuHelperSkillPicker->Show(false);
    }

    CObject::Show(bShow);
}

void CMuHelperConfigWindow::BlurFocusedField()
{
    if (!m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    Rml::Element* focused = RmlUiRuntime::Instance().GetContext()->GetFocusElement();
    if (focused != nullptr && focused->GetOwnerDocument() == m_pRmlDoc)
        focused->Blur();
}

bool CMuHelperConfigWindow::UpdateMouseEvent()
{
    // The frame's corner "X" -- the docked family's shared hit rect, native's own 169/7/13/12.
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, INTERFACE_MUHELPER))
        return false;

    float panelWidth = static_cast<float>(WindowWidth);
    float panelHeight = static_cast<float>(WindowHeight);
    UI::RmlBridge::RefreshLogicalPanelSize(m_pRmlDoc, "panel", panelWidth, panelHeight);

    if (!WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth), static_cast<int>(panelHeight)).Contains(MouseX, MouseY))
        return true;

    if (m_iCurrentOpenTab == 1
        && WindowGeometry(m_Pos.x + ExtraListLeft, m_Pos.y + ExtraListTop, ExtraListWidth, ExtraListHeight).Contains(MouseX, MouseY))
    {
        MouseWheel = 0;
    }

    return false;
}

bool CMuHelperConfigWindow::UpdateKeyEvent()
{
    if (IsVisible() && IsPress(VK_ESCAPE))
    {
        g_pNewUISystem->Hide(INTERFACE_MUHELPER);
        g_pNewUISystem->Hide(INTERFACE_MUHELPER_SKILL_LIST);
        return false;
    }
    return true;
}

bool CMuHelperConfigWindow::Update()
{
    SyncRmlModel();

    // So Escape still closes the window while one of its fields has the keyboard.
    UI::RmlBridge::ClaimKeyboardWhileTyping(*this, m_pRmlDoc);
    return true;
}

bool CMuHelperConfigWindow::Render()
{
    // Nothing native left. Kept because CObject requires the override.
    return true;
}

float CMuHelperConfigWindow::GetLayerDepth()
{
    return 3.4f;
}

float CMuHelperConfigWindow::GetKeyEventOrder()
{
    return 3.4f;
}

void CMuHelperConfigWindow::SelectTab(int tab)
{
    if (tab < 0 || tab > 2 || tab == m_iCurrentOpenTab)
        return;

    // A field on the tab being left would stay focused while hidden.
    BlurFocusedField();
    m_iCurrentOpenTab = tab;
}

void CMuHelperConfigWindow::OnToggle(const Rml::String& key)
{
    ConfigData& c = UI::MuHelper::StagedConfig();
    auto flip = [](bool& value) { value = !value; };

    if (key == "potion") flip(c.bUseHealPotion);
    else if (key == "long_distance") flip(c.bLongRangeCounterAttack);
    else if (key == "orig_position") flip(c.bReturnToOriginalPosition);
    else if (key == "skill2_delay" || key == "skill3_delay"
             || key == "skill2_condition" || key == "skill3_condition")
    {
        const int index = (key == "skill2_delay" || key == "skill2_condition") ? 1 : 2;
        const bool timer = (key == "skill2_delay" || key == "skill3_delay");
        uint32_t& bits = c.aiSkillCondition[index];
        const bool enable = !(bits & (timer ? ON_TIMER : ON_CONDITION));

        SetConditionMode(bits, timer, enable);

        // Choosing a timer, or dropping the condition, closes the condition's detail page.
        if (timer == enable)
            g_pNewUISystem->Hide(INTERFACE_MUHELPER_EXT);
        // A condition needs both of its groups set or the skill never fires.
        if (!timer && enable)
            UI::MuHelper::EnsureConditionDefaults(bits);
    }
    else if (key == "combo")
    {
        if (!c.bUseCombo && (m_aiSelectedSkills[0] <= 0 || m_aiSelectedSkills[1] <= 0 || m_aiSelectedSkills[2] <= 0))
        {
            g_pSystemLogBox->AddText(I18N::Game::InOrderToUseComboSkill, TYPE_ERROR_MESSAGE);
            return;
        }
        flip(c.bUseCombo);
    }
    else if (key == "fallback") flip(c.bFallbackBasicAttack);
    else if (key == "buff_duration") flip(c.bBuffDuration);
    else if (key == "use_pet") flip(c.bUseDarkRaven);
    else if (key == "party") flip(c.bSupportParty);
    else if (key == "auto_heal") flip(c.bAutoHeal);
    else if (key == "drain_life") flip(c.bUseDrainLife);
    else if (key == "repair") flip(c.bRepairItem);
    // All-items and selected-items exclude each other. Native only unticked the other box on screen
    // and left its setting on, so both could be saved as set; both are cleared here.
    else if (key == "pick_all")
    {
        flip(c.bPickAllItems);
        if (c.bPickAllItems)
            c.bPickSelectItems = false;
    }
    else if (key == "pick_selected")
    {
        flip(c.bPickSelectItems);
        if (c.bPickSelectItems)
            c.bPickAllItems = false;
    }
    else if (key == "pick_jewel") flip(c.bPickJewel);
    else if (key == "pick_ancient") flip(c.bPickAncient);
    else if (key == "pick_zen") flip(c.bPickZen);
    else if (key == "pick_excellent") flip(c.bPickExcellent);
    else if (key == "pick_extra") flip(c.bPickExtraItems);
    else if (key == "auto_friend") flip(c.bAutoAcceptFriend);
    else if (key == "auto_guild") flip(c.bAutoAcceptGuild);
    else if (key == "auto_defend") flip(c.bUseSelfDefense);
}

void CMuHelperConfigWindow::OnButton(const Rml::String& key)
{
    ConfigData& c = UI::MuHelper::StagedConfig();
    const UI::MuHelper::ClassFeatures features = UI::MuHelper::ResolveClassFeatures(BaseClass());

    if (key == "hunt_add") c.iHuntingRange = std::min(c.iHuntingRange + 1, MaxHuntingRange);
    else if (key == "hunt_minus") c.iHuntingRange = std::max(c.iHuntingRange - 1, 0);
    else if (key == "pick_add") c.iObtainingRange = std::min(c.iObtainingRange + 1, MaxObtainingRange);
    else if (key == "pick_minus") c.iObtainingRange = std::max(c.iObtainingRange - 1, 1);
    else if (key == "add_item") AddExtraItem();
    else if (key == "delete_item") RemoveExtraItem();
    else if (key == "skill2_setting" || key == "skill3_setting")
    {
        // Opening a skill's condition page also switches that skill to "on condition".
        const bool second = (key == "skill2_setting");
        SetConditionMode(c.aiSkillCondition[second ? 1 : 2], false, true);
        if (g_pMuHelperDetail)
            g_pMuHelperDetail->Toggle(second ? SUB_PAGE_SKILL2_CONFIG : SUB_PAGE_SKILL3_CONFIG);
    }
    else if (key == "potion_setting")
    {
        if (g_pMuHelperDetail)
            g_pMuHelperDetail->Toggle(features.potionPage);
    }
    else if (key == "party_setting")
    {
        if (g_pMuHelperDetail && features.party)
            g_pMuHelperDetail->Toggle(features.partyPage);
    }
    else if (key == "save")
    {
        SaveConfig();
        g_pNewUISystem->Hide(INTERFACE_MUHELPER);
    }
    else if (key == "init") InitConfig();
    else if (key == "close") g_pNewUISystem->Hide(INTERFACE_MUHELPER);
}

void CMuHelperConfigWindow::OnSlotClick(int slot)
{
    if (slot < 0 || slot >= SkillSlotCount || g_pMuHelperSkillPicker == nullptr)
        return;

    const int prevSlot = m_iSelectedSkillSlot;
    const bool pickerWasOpen = g_pNewUISystem->IsVisible(INTERFACE_MUHELPER_SKILL_LIST);
    m_iSelectedSkillSlot = slot;

    if (slot < 3)
        g_pMuHelperSkillPicker->FilterByAttackSkills();
    else
        g_pMuHelperSkillPicker->FilterByBuffSkills();

    // A second click on the same slot closes the picker.
    if (slot == prevSlot && pickerWasOpen)
        g_pNewUISystem->Hide(INTERFACE_MUHELPER_SKILL_LIST);
    else
        g_pNewUISystem->Show(INTERFACE_MUHELPER_SKILL_LIST);
}

void CMuHelperConfigWindow::OnSlotClear(int slot)
{
    if (slot < 0 || slot >= SkillSlotCount)
        return;

    m_aiSelectedSkills[slot] = -1;
    ClearCombo();
}

void CMuHelperConfigWindow::ClearCombo()
{
    UI::MuHelper::StagedConfig().bUseCombo = false;
}

void CMuHelperConfigWindow::AssignSkill(int iSkill)
{
    if (m_iSelectedSkillSlot < 0 || m_iSelectedSkillSlot >= SkillSlotCount)
        return;

    ConfigData& c = UI::MuHelper::StagedConfig();

    if (!IsSkillAssigned(iSkill))
    {
        m_aiSelectedSkills[m_iSelectedSkillSlot] = iSkill;
        if (m_iSelectedSkillSlot < 3)
            c.aiSkill[m_iSelectedSkillSlot] = iSkill;
        else
            c.aiBuff[m_iSelectedSkillSlot - 3] = iSkill;
    }
    else
    {
        // Already in another slot: move it here.
        m_aiSelectedSkills[GetSkillIndex(iSkill)] = -1;
        m_aiSelectedSkills[m_iSelectedSkillSlot] = iSkill;
        ClearCombo();
    }
}

bool CMuHelperConfigWindow::IsSkillAssigned(int iSkill) const
{
    return std::find(m_aiSelectedSkills.begin(), m_aiSelectedSkills.end(), iSkill) != m_aiSelectedSkills.end();
}

int CMuHelperConfigWindow::GetSkillIndex(int iSkill) const
{
    const auto it = std::find(m_aiSelectedSkills.begin(), m_aiSelectedSkills.end(), iSkill);
    return it != m_aiSelectedSkills.end() ? static_cast<int>(std::distance(m_aiSelectedSkills.begin(), it)) : -1;
}

void CMuHelperConfigWindow::AddExtraItem()
{
    MuHelperConfigRmlModel& model = m_RmlBinder.GetModel();
    if (model.itemName.empty())
        return;

    const std::wstring name = StringUtils::NarrowToWide(model.itemName);
    UI::MuHelper::StagedConfig().aExtraItems.insert(name);
    m_selectedExtraItem = model.itemName;

    model.itemName.clear();
    m_RmlBinder.MarkDirty("item_name");
    m_bExtraItemsDirty = true;

    BlurFocusedField();
}

void CMuHelperConfigWindow::RemoveExtraItem()
{
    if (m_selectedExtraItem.empty())
        return;

    ConfigData& c = UI::MuHelper::StagedConfig();
    const std::wstring name = StringUtils::NarrowToWide(m_selectedExtraItem);

    // The entry after it takes the selection, as native's list did.
    auto it = c.aExtraItems.find(name);
    if (it == c.aExtraItems.end())
        return;
    auto next = c.aExtraItems.erase(it);
    m_selectedExtraItem = next != c.aExtraItems.end() ? StringUtils::WideToNarrow(next->c_str()) : Rml::String();

    m_bExtraItemsDirty = true;
}

void CMuHelperConfigWindow::LoadSavedConfig(const ConfigData& config)
{
    UI::MuHelper::StagedConfig() = config;
    ApplyConfig();
}

void CMuHelperConfigWindow::ApplyConfig()
{
    const ConfigData& c = UI::MuHelper::StagedConfig();

    g_MuHelper.Load(c);

    if (g_pMuHelperDetail)
        g_pMuHelperDetail->ApplySavedConfig();

    for (int i = 0; i < 3; ++i)
    {
        m_aiSelectedSkills[i] = c.aiSkill[i] ? c.aiSkill[i] : -1;
        m_aiSelectedSkills[3 + i] = c.aiBuff[i] ? c.aiBuff[i] : -1;
    }

    MuHelperConfigRmlModel& model = m_RmlBinder.GetModel();
    model.distanceTime = std::to_string(c.iMaxSecondsAway);
    model.skill2Delay = std::to_string(c.aiSkillInterval[1]);
    model.skill3Delay = std::to_string(c.aiSkillInterval[2]);
    m_RmlBinder.MarkDirty("distance_time");
    m_RmlBinder.MarkDirty("skill2_delay");
    m_RmlBinder.MarkDirty("skill3_delay");

    m_bExtraItemsDirty = true;
}

void CMuHelperConfigWindow::Reset()
{
    ConfigData& c = UI::MuHelper::StagedConfig();

    c.iHuntingRange = 6;

    c.iMaxSecondsAway = 10;
    c.bLongRangeCounterAttack = false;
    c.bReturnToOriginalPosition = true;

    c.aiSkill.fill(0);
    c.bUseCombo = false;
    c.aiSkillInterval.fill(0);
    c.aiSkillCondition.fill(0);
    c.aiBuff.fill(0);

    c.bBuffDuration = true;
    c.bBuffDurationParty = true;
    c.iBuffCastInterval = 0;

    c.bAutoHeal = false;
    c.iHealThreshold = 60;
    c.bUseDrainLife = false;
    c.bUseHealPotion = false;
    c.iPotionThreshold = 40;
    c.bSupportParty = false;
    c.bAutoHealParty = false;
    c.iHealPartyThreshold = 60;

    c.bUseDarkRaven = false;
    c.iDarkRavenMode = PET_ATTACK_CEASE;
    c.bRepairItem = false;

    c.iObtainingRange = 8;
    c.bPickAllItems = false;
    c.bPickSelectItems = false;
    c.bPickZen = false;
    c.bPickJewel = false;
    c.bPickExcellent = false;
    c.bPickAncient = false;
    c.bPickExtraItems = false;
    c.aExtraItems.clear();
    m_selectedExtraItem.clear();

    ApplyConfig();
}

// The "Initialization" button.
void CMuHelperConfigWindow::InitConfig()
{
    Reset();

    if (g_pMuHelperDetail)
        g_pMuHelperDetail->InitConfig();
}

void CMuHelperConfigWindow::SaveConfig()
{
    ConfigData& c = UI::MuHelper::StagedConfig();
    const MuHelperConfigRmlModel& model = m_RmlBinder.GetModel();

    // The fields only accept digits as they are typed, but a paste bypasses that filter.
    auto parse = [](const Rml::String& text)
    {
        const std::wstring digits = StringUtils::NarrowToWide(UI::RmlBridge::KeepDigitsOnly(text));
        return UI::MuHelper::ParseIntInput(digits.c_str());
    };
    c.iMaxSecondsAway = parse(model.distanceTime);
    c.aiSkillInterval[1] = parse(model.skill2Delay);
    c.aiSkillInterval[2] = parse(model.skill3Delay);

    for (int i = 0; i < 3; ++i)
    {
        c.aiSkill[i] = m_aiSelectedSkills[i] > 0 ? m_aiSelectedSkills[i] : 0;
        c.aiBuff[i] = m_aiSelectedSkills[3 + i] > 0 ? m_aiSelectedSkills[3 + i] : 0;
    }

    g_MuHelper.Save(c);
}

void CMuHelperConfigWindow::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "mu_helper_config",
        [this](Rml::DataModelConstructor& c, MuHelperConfigRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
            c.Bind("text_px", &model.textPx);

            c.Bind("active_tab", &model.activeTab);
            c.Bind("hunt_range", &model.huntRange);
            c.Bind("pick_range", &model.pickRange);
            c.Bind("raven_mode", &model.ravenMode);

            auto features = c.RegisterStruct<MuHelperConfigFeatures>();
            features.RegisterMember("skill3", &MuHelperConfigFeatures::skill3);
            features.RegisterMember("combo", &MuHelperConfigFeatures::combo);
            features.RegisterMember("pet", &MuHelperConfigFeatures::pet);
            features.RegisterMember("party", &MuHelperConfigFeatures::party);
            features.RegisterMember("auto_heal", &MuHelperConfigFeatures::autoHeal);
            features.RegisterMember("drain_life", &MuHelperConfigFeatures::drainLife);
            features.RegisterMember("potion_summoner", &MuHelperConfigFeatures::potionSummoner);
            c.Bind("features", &model.features);

            auto checks = c.RegisterStruct<MuHelperConfigChecks>();
            checks.RegisterMember("potion", &MuHelperConfigChecks::potion);
            checks.RegisterMember("long_distance", &MuHelperConfigChecks::longDistance);
            checks.RegisterMember("orig_position", &MuHelperConfigChecks::origPosition);
            checks.RegisterMember("skill2_delay", &MuHelperConfigChecks::skill2Delay);
            checks.RegisterMember("skill2_condition", &MuHelperConfigChecks::skill2Condition);
            checks.RegisterMember("skill3_delay", &MuHelperConfigChecks::skill3Delay);
            checks.RegisterMember("skill3_condition", &MuHelperConfigChecks::skill3Condition);
            checks.RegisterMember("combo", &MuHelperConfigChecks::combo);
            checks.RegisterMember("fallback", &MuHelperConfigChecks::fallback);
            checks.RegisterMember("buff_duration", &MuHelperConfigChecks::buffDuration);
            checks.RegisterMember("use_pet", &MuHelperConfigChecks::usePet);
            checks.RegisterMember("party", &MuHelperConfigChecks::party);
            checks.RegisterMember("auto_heal", &MuHelperConfigChecks::autoHeal);
            checks.RegisterMember("drain_life", &MuHelperConfigChecks::drainLife);
            checks.RegisterMember("repair", &MuHelperConfigChecks::repair);
            checks.RegisterMember("pick_all", &MuHelperConfigChecks::pickAll);
            checks.RegisterMember("pick_selected", &MuHelperConfigChecks::pickSelected);
            checks.RegisterMember("pick_jewel", &MuHelperConfigChecks::pickJewel);
            checks.RegisterMember("pick_ancient", &MuHelperConfigChecks::pickAncient);
            checks.RegisterMember("pick_zen", &MuHelperConfigChecks::pickZen);
            checks.RegisterMember("pick_excellent", &MuHelperConfigChecks::pickExcellent);
            checks.RegisterMember("pick_extra", &MuHelperConfigChecks::pickExtra);
            checks.RegisterMember("auto_friend", &MuHelperConfigChecks::autoFriend);
            checks.RegisterMember("auto_guild", &MuHelperConfigChecks::autoGuild);
            checks.RegisterMember("auto_defend", &MuHelperConfigChecks::autoDefend);
            c.Bind("checks", &model.checks);

            auto slots = c.RegisterStruct<MuHelperSlotIcons>();
            slots.RegisterMember("s0", &MuHelperSlotIcons::s0);
            slots.RegisterMember("s1", &MuHelperSlotIcons::s1);
            slots.RegisterMember("s2", &MuHelperSlotIcons::s2);
            slots.RegisterMember("s3", &MuHelperSlotIcons::s3);
            slots.RegisterMember("s4", &MuHelperSlotIcons::s4);
            slots.RegisterMember("s5", &MuHelperSlotIcons::s5);
            c.Bind("slots", &model.slots);

            c.Bind("distance_time", &model.distanceTime);
            c.Bind("skill2_delay", &model.skill2Delay);
            c.Bind("skill3_delay", &model.skill3Delay);
            c.Bind("item_name", &model.itemName);

            auto item = c.RegisterStruct<MuHelperExtraItem>();
            item.RegisterMember("name", &MuHelperExtraItem::name);
            item.RegisterMember("selected", &MuHelperExtraItem::selected);
            item.RegisterMember("index", &MuHelperExtraItem::index);
            c.RegisterArray<std::vector<MuHelperExtraItem>>();
            c.Bind("extra_items", &model.extraItems);

            auto labels = c.RegisterStruct<MuHelperConfigLabels>();
            labels.RegisterMember("title", &MuHelperConfigLabels::title);
            labels.RegisterMember("tab_hunting", &MuHelperConfigLabels::tabHunting);
            labels.RegisterMember("tab_obtaining", &MuHelperConfigLabels::tabObtaining);
            labels.RegisterMember("tab_other", &MuHelperConfigLabels::tabOther);
            labels.RegisterMember("range", &MuHelperConfigLabels::range);
            labels.RegisterMember("distance", &MuHelperConfigLabels::distance);
            labels.RegisterMember("basic_skill", &MuHelperConfigLabels::basicSkill);
            labels.RegisterMember("activation_skill1", &MuHelperConfigLabels::activationSkill1);
            labels.RegisterMember("activation_skill2", &MuHelperConfigLabels::activationSkill2);
            labels.RegisterMember("seconds", &MuHelperConfigLabels::seconds);
            labels.RegisterMember("extension_used", &MuHelperConfigLabels::extensionUsed);
            labels.RegisterMember("extension_none", &MuHelperConfigLabels::extensionNone);
            labels.RegisterMember("potion", &MuHelperConfigLabels::potion);
            labels.RegisterMember("long_distance", &MuHelperConfigLabels::longDistance);
            labels.RegisterMember("orig_position", &MuHelperConfigLabels::origPosition);
            labels.RegisterMember("delay", &MuHelperConfigLabels::delay);
            labels.RegisterMember("condition", &MuHelperConfigLabels::condition);
            labels.RegisterMember("combo", &MuHelperConfigLabels::combo);
            labels.RegisterMember("fallback", &MuHelperConfigLabels::fallback);
            labels.RegisterMember("buff_duration", &MuHelperConfigLabels::buffDuration);
            labels.RegisterMember("use_pet", &MuHelperConfigLabels::usePet);
            labels.RegisterMember("party", &MuHelperConfigLabels::party);
            labels.RegisterMember("auto_heal", &MuHelperConfigLabels::autoHeal);
            labels.RegisterMember("drain_life", &MuHelperConfigLabels::drainLife);
            labels.RegisterMember("raven_cease", &MuHelperConfigLabels::ravenCease);
            labels.RegisterMember("raven_auto", &MuHelperConfigLabels::ravenAuto);
            labels.RegisterMember("raven_together", &MuHelperConfigLabels::ravenTogether);
            labels.RegisterMember("repair", &MuHelperConfigLabels::repair);
            labels.RegisterMember("pick_all", &MuHelperConfigLabels::pickAll);
            labels.RegisterMember("pick_selected", &MuHelperConfigLabels::pickSelected);
            labels.RegisterMember("pick_jewel", &MuHelperConfigLabels::pickJewel);
            labels.RegisterMember("pick_ancient", &MuHelperConfigLabels::pickAncient);
            labels.RegisterMember("pick_zen", &MuHelperConfigLabels::pickZen);
            labels.RegisterMember("pick_excellent", &MuHelperConfigLabels::pickExcellent);
            labels.RegisterMember("pick_extra", &MuHelperConfigLabels::pickExtra);
            labels.RegisterMember("auto_friend", &MuHelperConfigLabels::autoFriend);
            labels.RegisterMember("auto_guild", &MuHelperConfigLabels::autoGuild);
            labels.RegisterMember("auto_defend", &MuHelperConfigLabels::autoDefend);
            labels.RegisterMember("setting", &MuHelperConfigLabels::setting);
            labels.RegisterMember("add", &MuHelperConfigLabels::add);
            labels.RegisterMember("remove", &MuHelperConfigLabels::remove);
            labels.RegisterMember("save", &MuHelperConfigLabels::save);
            labels.RegisterMember("init", &MuHelperConfigLabels::init);
            labels.RegisterMember("close_tip", &MuHelperConfigLabels::closeTip);
            c.Bind("labels", &model.labels);

            c.BindEventCallback("muhelper_tab",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                {
                    if (args.size() == 1)
                        SelectTab(args[0].Get<int>(-1));
                });
            c.BindEventCallback("muhelper_toggle",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                {
                    if (args.size() == 1)
                        OnToggle(args[0].Get<Rml::String>());
                });
            c.BindEventCallback("muhelper_raven_mode",
                [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                {
                    if (args.size() == 1)
                        UI::MuHelper::StagedConfig().iDarkRavenMode = args[0].Get<int>(PET_ATTACK_CEASE);
                });
            c.BindEventCallback("muhelper_button",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                {
                    if (args.size() == 1)
                        OnButton(args[0].Get<Rml::String>());
                });
            c.BindEventCallback("muhelper_slot",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                {
                    if (args.size() == 1)
                        OnSlotClick(args[0].Get<int>(-1));
                });
            // Mouseup fires for every button; "button" == 1 is the right one, which clears a slot.
            c.BindEventCallback("muhelper_slot_clear",
                [this](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList& args)
                {
                    if (event.GetParameter<int>("button", -1) != 1 || args.size() != 1)
                        return;
                    OnSlotClear(args[0].Get<int>(-1));
                });
            c.BindEventCallback("muhelper_extra_select",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                {
                    if (args.size() != 1)
                        return;
                    const int index = args[0].Get<int>(-1);
                    const auto& items = m_RmlBinder.GetModel().extraItems;
                    if (index >= 0 && index < static_cast<int>(items.size()))
                    {
                        m_selectedExtraItem = items[index].name;
                        m_bExtraItemsDirty = true;
                    }
                });
        });

    if (!modelCreated)
        return;

    MuHelperConfigLabels& l = m_RmlBinder.GetModel().labels;
    l.title = Narrow(I18N::Game::OfficialMUHelper);
    l.tabHunting = Narrow(I18N::Game::Hunting);
    l.tabObtaining = Narrow(I18N::Game::Obtaining);
    l.tabOther = Narrow(I18N::Game::OtherSettings);
    l.range = Narrow(I18N::Game::Range);
    l.distance = Narrow(I18N::Game::Distance);
    l.basicSkill = Narrow(I18N::Game::BasicSkill);
    l.activationSkill1 = Narrow(I18N::Game::ActivationSkill1);
    l.activationSkill2 = Narrow(I18N::Game::ActivationSkill2);
    l.seconds = "s";
    l.extensionUsed = Narrow(I18N::Game::UsedExtensionFunction);
    l.extensionNone = Narrow(I18N::Game::NoExtensionFunctionBeingUsed);
    l.potion = Narrow(I18N::Game::Potion);
    l.longDistance = Narrow(I18N::Game::LongDistanceCounterAttack);
    l.origPosition = Narrow(I18N::Game::OriginalPosition);
    l.delay = Narrow(I18N::Game::Delay);
    l.condition = Narrow(I18N::Game::Con);
    l.combo = Narrow(I18N::Game::Combo);
    l.fallback = Narrow(I18N::Game::BasicAttackFallback);
    l.buffDuration = Narrow(I18N::Game::BuffDuration);
    l.usePet = Narrow(I18N::Game::UseDarkSpirits);
    l.party = Narrow(I18N::Game::Party);
    l.autoHeal = Narrow(I18N::Game::AutoHeal);
    l.drainLife = Narrow(I18N::Game::DrainLife);
    l.ravenCease = Narrow(I18N::Game::CeaseAttack);
    l.ravenAuto = Narrow(I18N::Game::AutoAttack);
    l.ravenTogether = Narrow(I18N::Game::AttackTogether);
    l.repair = Narrow(I18N::Game::RepairItem);
    l.pickAll = Narrow(I18N::Game::PickAllNearItems);
    l.pickSelected = Narrow(I18N::Game::PickSelectedItems);
    l.pickJewel = Narrow(I18N::Game::JewelGem);
    l.pickAncient = Narrow(I18N::Game::SetItem);
    l.pickZen = Narrow(I18N::Game::Zen);
    l.pickExcellent = Narrow(I18N::Game::ExcellentItem);
    l.pickExtra = Narrow(I18N::Game::AddExtraItem);
    l.autoFriend = Narrow(I18N::Game::AutoAcceptFriend);
    l.autoGuild = Narrow(I18N::Game::AutoAcceptGuildMember);
    l.autoDefend = Narrow(I18N::Game::PVPCounterattack);
    l.setting = Narrow(I18N::Game::Setting);
    l.add = Narrow(I18N::Game::Add);
    l.remove = Narrow(I18N::Game::Delete);
    l.save = Narrow(I18N::Game::SaveSetting);
    l.init = Narrow(I18N::Game::Initialization);
    l.closeTip = Narrow(I18N::Game::Close388);

    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                  "Data/Interface/RmlUi/mu_helper_config.rml");
    if (!m_pRmlDoc)
        return;

    UI::RmlBridge::AttachNumericInputFilter(m_pRmlDoc);
    // Set from code so the limits can't drift per theme.
    for (const char* id : NumberFieldIds)
    {
        if (Rml::Element* field = m_pRmlDoc->GetElementById(id))
            field->SetAttribute("maxlength", UI::MuHelper::MaxNumberDigits);
    }
    if (Rml::Element* field = m_pRmlDoc->GetElementById("item_name"))
        field->SetAttribute("maxlength", MAX_ITEM_NAME);

    m_bExtraItemsDirty = true;
}

void CMuHelperConfigWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return; // never opened -- BuildRmlUi() picks up the new theme whenever it first is

    // The fields' typed values live in the model, which Destroy() resets -- carry them across.
    const MuHelperConfigRmlModel typed = m_RmlBinder.GetModel();

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();

    MuHelperConfigRmlModel& model = m_RmlBinder.GetModel();
    model.distanceTime = typed.distanceTime;
    model.skill2Delay = typed.skill2Delay;
    model.skill3Delay = typed.skill3Delay;
    model.itemName = typed.itemName;
    m_RmlBinder.MarkDirty("distance_time");
    m_RmlBinder.MarkDirty("skill2_delay");
    m_RmlBinder.MarkDirty("skill3_delay");
    m_RmlBinder.MarkDirty("item_name");
    // Next frame's SyncRmlModel() self-corrects visibility and every other value.
}

void CMuHelperConfigWindow::RebuildExtraItems()
{
    MuHelperConfigRmlModel& model = m_RmlBinder.GetModel();
    const ConfigData& c = UI::MuHelper::StagedConfig();

    model.extraItems.clear();
    model.extraItems.reserve(c.aExtraItems.size());

    // Newest-looking first, the order native's list showed after any reload of the saved set.
    int index = 0;
    for (auto it = c.aExtraItems.rbegin(); it != c.aExtraItems.rend(); ++it)
    {
        MuHelperExtraItem item;
        item.name = StringUtils::WideToNarrow(it->c_str());
        item.selected = (item.name == m_selectedExtraItem);
        item.index = index++;
        model.extraItems.push_back(std::move(item));
    }
    m_RmlBinder.MarkDirty("extra_items");
}

void CMuHelperConfigWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    UI::RmlBridge::SyncDocumentVisibility(m_pRmlDoc, IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, m_Pos);
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);

    MuHelperConfigRmlModel& model = m_RmlBinder.GetModel();
    const ConfigData& c = UI::MuHelper::StagedConfig();

    auto sync = [&](auto& field, const auto& value, const char* name)
    {
        if (!(field == value))
        {
            field = value;
            m_RmlBinder.MarkDirty(name);
        }
    };

    sync(model.activeTab, m_iCurrentOpenTab, "active_tab");
    sync(model.huntRange, c.iHuntingRange, "hunt_range");
    sync(model.pickRange, c.iObtainingRange, "pick_range");
    sync(model.ravenMode, c.iDarkRavenMode, "raven_mode");

    const UI::MuHelper::ClassFeatures f = UI::MuHelper::ResolveClassFeatures(BaseClass());
    MuHelperConfigFeatures features;
    features.skill3 = f.skill3;
    features.combo = f.combo;
    features.pet = f.pet;
    features.party = f.party;
    features.autoHeal = f.autoHeal;
    features.drainLife = f.drainLife;
    features.potionSummoner = f.potionSummoner;
    sync(model.features, features, "features");

    MuHelperConfigChecks checks;
    checks.potion = c.bUseHealPotion;
    checks.longDistance = c.bLongRangeCounterAttack;
    checks.origPosition = c.bReturnToOriginalPosition;
    checks.skill2Delay = (c.aiSkillCondition[1] & ON_TIMER) != 0;
    checks.skill2Condition = (c.aiSkillCondition[1] & ON_CONDITION) != 0;
    checks.skill3Delay = (c.aiSkillCondition[2] & ON_TIMER) != 0;
    checks.skill3Condition = (c.aiSkillCondition[2] & ON_CONDITION) != 0;
    checks.combo = c.bUseCombo;
    checks.fallback = c.bFallbackBasicAttack;
    checks.buffDuration = c.bBuffDuration;
    checks.usePet = c.bUseDarkRaven;
    checks.party = c.bSupportParty;
    checks.autoHeal = c.bAutoHeal;
    checks.drainLife = c.bUseDrainLife;
    checks.repair = c.bRepairItem;
    checks.pickAll = c.bPickAllItems;
    checks.pickSelected = c.bPickSelectItems;
    checks.pickJewel = c.bPickJewel;
    checks.pickAncient = c.bPickAncient;
    checks.pickZen = c.bPickZen;
    checks.pickExcellent = c.bPickExcellent;
    checks.pickExtra = c.bPickExtraItems;
    checks.autoFriend = c.bAutoAcceptFriend;
    checks.autoGuild = c.bAutoAcceptGuild;
    checks.autoDefend = c.bUseSelfDefense;
    sync(model.checks, checks, "checks");

    MuHelperSlotIcons slots;
    slots.s0 = SlotDecorator(m_aiSelectedSkills[0]);
    slots.s1 = SlotDecorator(m_aiSelectedSkills[1]);
    slots.s2 = SlotDecorator(m_aiSelectedSkills[2]);
    slots.s3 = SlotDecorator(m_aiSelectedSkills[3]);
    slots.s4 = SlotDecorator(m_aiSelectedSkills[4]);
    slots.s5 = SlotDecorator(m_aiSelectedSkills[5]);
    sync(model.slots, slots, "slots");

    if (m_bExtraItemsDirty)
    {
        m_bExtraItemsDirty = false;
        RebuildExtraItems();
    }
}
