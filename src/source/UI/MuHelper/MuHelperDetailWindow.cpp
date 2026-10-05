#include "stdafx.h"
#include "UI/MuHelper/MuHelperDetailWindow.h"

#include "Core/Utilities/StringUtils.h"
#include "I18N/All.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Core/WindowManager.h"
#include "UI/Core/WindowSystem.h"
#include "UI/MuHelper/MuHelperShared.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlKeyboardFocus.h"
#include "UI/RmlBridge/RmlLevelGauge.h"
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
    // Both pages that carry the buff interval have their own copy of the field (it sits at a
    // different height on each), bound to one model value.
    constexpr const char* BuffIntervalFieldIds[] = { "buff_interval", "buff_interval_elf" };

    constexpr uint32_t SubConditionBits[] = {
        ON_MORE_THAN_TWO_MOBS, ON_MORE_THAN_THREE_MOBS, ON_MORE_THAN_FOUR_MOBS, ON_MORE_THAN_FIVE_MOBS
    };

    constexpr int GaugeMaxLevel = 10;
    constexpr float GaugeOriginInset = 1.f;

    bool IsSkillPage(int page) { return page == SUB_PAGE_SKILL2_CONFIG || page == SUB_PAGE_SKILL3_CONFIG; }
    bool IsPotionPage(int page)
    {
        return page == SUB_PAGE_POTION_CONFIG || page == SUB_PAGE_POTION_CONFIG_ELF || page == SUB_PAGE_POTION_CONFIG_SUMMY;
    }
    bool IsPartyPage(int page) { return page == SUB_PAGE_PARTY_CONFIG || page == SUB_PAGE_PARTY_CONFIG_ELF; }

    // aiSkillCondition[] slot a skill page edits: slot 1 for the second skill, slot 2 for the third.
    int SkillConditionIndex(int page) { return page == SUB_PAGE_SKILL2_CONFIG ? 1 : 2; }

    Rml::String Narrow(const wchar_t* text) { return StringUtils::WideToNarrow(text); }
}

CMuHelperDetailWindow::CMuHelperDetailWindow() = default;

CMuHelperDetailWindow::~CMuHelperDetailWindow()
{
    Release();
}

bool CMuHelperDetailWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (pNewUIMng == nullptr)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(INTERFACE_MUHELPER_EXT, this);

    m_Pos.x = x;
    m_Pos.y = y;

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    ApplySavedConfig();

    Show(false);

    return true;
}

void CMuHelperDetailWindow::Release()
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

void CMuHelperDetailWindow::Show(bool bShow)
{
    // Every way this panel closes -- Esc, Close, Save, the config window's own cascade, a second
    // click on the same "Setting" button -- comes through here. A focused field left in a hidden
    // document keeps RmlUiRuntime::IsTextInputActive() true, which suspends every window's key
    // handling, so it has to be released on the way out.
    if (!bShow)
        BlurFocusedField();

    CObject::Show(bShow);
}

void CMuHelperDetailWindow::BlurFocusedField()
{
    if (!m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    Rml::Element* focused = RmlUiRuntime::Instance().GetContext()->GetFocusElement();
    if (focused != nullptr && focused->GetOwnerDocument() == m_pRmlDoc)
        focused->Blur();
}

bool CMuHelperDetailWindow::UpdateMouseEvent()
{
    float panelWidth = static_cast<float>(WindowWidth);
    float panelHeight = static_cast<float>(WindowHeight);
    UI::RmlBridge::RefreshLogicalPanelSize(m_pRmlDoc, "panel", panelWidth, panelHeight);

    if (!WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth), static_cast<int>(panelHeight)).Contains(MouseX, MouseY))
        return true;

    return false;
}

void CMuHelperDetailWindow::HandleGaugeEvent(Rml::Event& event, int gauge)
{
    if (!IsVisible())
        return;

    int* level = nullptr;
    switch (gauge)
    {
    case 0:
        if (IsPotionPage(m_iCurrentPage))
            level = &m_iCurrentPotionThreshold;
        break;
    case 1:
        if (m_iCurrentPage == SUB_PAGE_POTION_CONFIG_ELF || m_iCurrentPage == SUB_PAGE_POTION_CONFIG_SUMMY)
            level = &m_iCurrentHealThreshold;
        break;
    case 2:
        if (m_iCurrentPage == SUB_PAGE_PARTY_CONFIG_ELF)
            level = &m_iCurrentPartyHealThreshold;
        break;
    }
    if (!level)
        return;

    // Native's drag origin sits one unit inside the drawn bar, and a press left of it reads 0.
    const auto fromPointer = [](float x, float width)
    {
        x -= GaugeOriginInset;
        return x < 0.f ? 0 : static_cast<int>(GaugeMaxLevel * x / width) + 1;
    };
    if (const auto next = UI::RmlBridge::ApplyLevelGaugeEvent(event, *level, GaugeMaxLevel, fromPointer))
        *level = *next;
}

bool CMuHelperDetailWindow::UpdateKeyEvent()
{
    if (IsVisible() && IsPress(VK_ESCAPE))
    {
        g_pNewUISystem->Hide(INTERFACE_MUHELPER_EXT);
        return false;
    }
    return true;
}

bool CMuHelperDetailWindow::Update()
{
    SyncRmlModel();

    // Without this, a focused buff-interval field swallows Escape: no window receives keys while an
    // RmlUi field has the focus unless it claims it. Native's own text box behaved the same; changed
    // deliberately so Escape closes the panel from inside the field too.
    UI::RmlBridge::ClaimKeyboardWhileTyping(*this, m_pRmlDoc);
    return true;
}

bool CMuHelperDetailWindow::Render()
{
    // Nothing native left: the frame, the panes, the gauges, every control and the footer are all
    // RmlUi. Kept because CObject requires the override.
    return true;
}

float CMuHelperDetailWindow::GetLayerDepth()
{
    return 3.4f;
}

float CMuHelperDetailWindow::GetKeyEventOrder()
{
    return 3.4f;
}

void CMuHelperDetailWindow::Toggle(int iPage)
{
    const int prevPage = m_iCurrentPage;
    m_iCurrentPage = iPage;

    if (IsVisible() && m_iCurrentPage == prevPage)
    {
        m_iCurrentPage = SUB_PAGE_NONE;
        Show(false);
        return;
    }

    ConfigData& config = UI::MuHelper::StagedConfig();

    if (IsSkillPage(m_iCurrentPage))
    {
        // Native opened this page with no radio set on a fresh config, and closing it without
        // picking left the skill unable to fire at all.
        UI::MuHelper::EnsureConditionDefaults(config.aiSkillCondition[SkillConditionIndex(m_iCurrentPage)]);
    }
    else if (IsPotionPage(m_iCurrentPage))
    {
        m_iCurrentPotionThreshold = config.iPotionThreshold / 10;
        m_iCurrentHealThreshold = config.iHealThreshold / 10;
    }
    else if (IsPartyPage(m_iCurrentPage))
    {
        m_iCurrentPartyHealThreshold = config.iHealPartyThreshold / 10;

        // Native reseeded the field on every open of a party page, discarding anything typed but
        // not saved. Kept.
        MuHelperDetailRmlModel& model = m_RmlBinder.GetModel();
        model.buffInterval = std::to_string(config.iBuffCastInterval);
        m_RmlBinder.MarkDirty("buff_interval");
    }

    Show(true);
}

void CMuHelperDetailWindow::Save()
{
    ConfigData& config = UI::MuHelper::StagedConfig();

    // The field only accepts digits as they are typed, but a paste bypasses that filter.
    const std::wstring interval =
        StringUtils::NarrowToWide(UI::RmlBridge::KeepDigitsOnly(m_RmlBinder.GetModel().buffInterval));
    config.iBuffCastInterval = UI::MuHelper::ParseIntInput(interval.c_str());

    config.iPotionThreshold = m_iCurrentPotionThreshold * 10;
    config.iHealThreshold = m_iCurrentHealThreshold * 10;
    config.iHealPartyThreshold = m_iCurrentPartyHealThreshold * 10;
}

void CMuHelperDetailWindow::ApplySavedConfig()
{
    const ConfigData& config = UI::MuHelper::StagedConfig();
    m_iCurrentPotionThreshold = config.iPotionThreshold / 10;
    m_iCurrentHealThreshold = config.iHealThreshold / 10;
    m_iCurrentPartyHealThreshold = config.iHealPartyThreshold / 10;

    // Save writes the buff interval from the field on every page, but native only ever filled the
    // field on the party pages -- so saving from any other page first wrote back whatever it held,
    // an empty field parsing as 0. Keeping it filled from the staged value stops that.
    m_RmlBinder.GetModel().buffInterval = std::to_string(config.iBuffCastInterval);
    m_RmlBinder.MarkDirty("buff_interval");
}

// The config window's "Initialization" button.
void CMuHelperDetailWindow::InitConfig()
{
    ConfigData& config = UI::MuHelper::StagedConfig();
    config.iPotionThreshold = 40;
    config.iHealThreshold = 60;
    config.iBuffCastInterval = 0;
    config.iHealPartyThreshold = 60;
    config.bAutoHealParty = false;
    config.bBuffDurationParty = false;

    m_RmlBinder.GetModel().buffInterval = std::to_string(config.iBuffCastInterval);
    m_RmlBinder.MarkDirty("buff_interval");
}

// This panel's own "Initialization" button: resets only the page it is showing.
void CMuHelperDetailWindow::Reset()
{
    ConfigData& config = UI::MuHelper::StagedConfig();

    if (IsSkillPage(m_iCurrentPage))
    {
        config.aiSkillCondition[SkillConditionIndex(m_iCurrentPage)] =
            static_cast<uint32_t>(ON_MOBS_NEARBY) | static_cast<uint32_t>(ON_MORE_THAN_TWO_MOBS);
    }
    else if (IsPotionPage(m_iCurrentPage))
    {
        config.iPotionThreshold = 0;
        config.iHealThreshold = 0;
        m_iCurrentPotionThreshold = 0;
        m_iCurrentHealThreshold = 0;
    }
}

void CMuHelperDetailWindow::SetPreCondition(int index)
{
    if (!IsSkillPage(m_iCurrentPage) || index < 0 || index > 1)
        return;

    uint32_t& bits = UI::MuHelper::StagedConfig().aiSkillCondition[SkillConditionIndex(m_iCurrentPage)];
    bits = (bits & MUHELPER_SKILL_PRECON_CLEAR)
         | static_cast<uint32_t>(index == 0 ? ON_MOBS_NEARBY : ON_MOBS_ATTACKING);
}

void CMuHelperDetailWindow::SetSubCondition(int index)
{
    if (!IsSkillPage(m_iCurrentPage) || index < 0 || index > 3)
        return;

    uint32_t& bits = UI::MuHelper::StagedConfig().aiSkillCondition[SkillConditionIndex(m_iCurrentPage)];
    bits = (bits & MUHELPER_SKILL_SUBCON_CLEAR) | SubConditionBits[index];
}

void CMuHelperDetailWindow::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "mu_helper_detail",
        [this](Rml::DataModelConstructor& c, MuHelperDetailRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
            c.Bind("text_px", &model.textPx);

            c.Bind("page", &model.page);
            c.Bind("precon", &model.precon);
            c.Bind("subcon", &model.subcon);
            c.Bind("potion_level", &model.potionLevel);
            c.Bind("heal_level", &model.healLevel);
            c.Bind("party_heal_level", &model.partyHealLevel);
            c.Bind("party_heal", &model.partyHeal);
            c.Bind("party_duration", &model.partyDuration);
            c.Bind("buff_interval", &model.buffInterval);

            auto labels = c.RegisterStruct<MuHelperDetailLabels>();
            labels.RegisterMember("title_activation", &MuHelperDetailLabels::titleActivation);
            labels.RegisterMember("title_recovery", &MuHelperDetailLabels::titleRecovery);
            labels.RegisterMember("title_party", &MuHelperDetailLabels::titleParty);
            labels.RegisterMember("pane_precon", &MuHelperDetailLabels::panePreCon);
            labels.RegisterMember("pane_subcon", &MuHelperDetailLabels::paneSubCon);
            labels.RegisterMember("pane_auto_potion", &MuHelperDetailLabels::paneAutoPotion);
            labels.RegisterMember("pane_auto_heal", &MuHelperDetailLabels::paneAutoHeal);
            labels.RegisterMember("pane_drain_life", &MuHelperDetailLabels::paneDrainLife);
            labels.RegisterMember("pane_buff_support", &MuHelperDetailLabels::paneBuffSupport);
            labels.RegisterMember("pane_heal_support", &MuHelperDetailLabels::paneHealSupport);
            labels.RegisterMember("hp_status", &MuHelperDetailLabels::hpStatus);
            labels.RegisterMember("hp_status_party", &MuHelperDetailLabels::hpStatusParty);
            labels.RegisterMember("precon_hunt_range", &MuHelperDetailLabels::preconHuntRange);
            labels.RegisterMember("precon_attacking", &MuHelperDetailLabels::preconAttacking);
            labels.RegisterMember("subcon_two", &MuHelperDetailLabels::subconTwo);
            labels.RegisterMember("subcon_three", &MuHelperDetailLabels::subconThree);
            labels.RegisterMember("subcon_four", &MuHelperDetailLabels::subconFour);
            labels.RegisterMember("subcon_five", &MuHelperDetailLabels::subconFive);
            labels.RegisterMember("party_heal", &MuHelperDetailLabels::partyHeal);
            labels.RegisterMember("party_duration", &MuHelperDetailLabels::partyDuration);
            labels.RegisterMember("time_space", &MuHelperDetailLabels::timeSpace);
            labels.RegisterMember("save", &MuHelperDetailLabels::save);
            labels.RegisterMember("init", &MuHelperDetailLabels::init);
            labels.RegisterMember("close_tip", &MuHelperDetailLabels::closeTip);
            c.Bind("labels", &model.labels);

            c.BindEventCallback("muhelper_detail_precon",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                {
                    if (args.size() == 1)
                        SetPreCondition(args[0].Get<int>(-1));
                });
            c.BindEventCallback("muhelper_detail_subcon",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                {
                    if (args.size() == 1)
                        SetSubCondition(args[0].Get<int>(-1));
                });
            c.BindEventCallback("muhelper_detail_gauge",
                [this](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList& args)
                {
                    if (args.size() == 1)
                        HandleGaugeEvent(event, args[0].Get<int>(-1));
                });
            c.BindEventCallback("muhelper_detail_party_heal",
                [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                {
                    ConfigData& config = UI::MuHelper::StagedConfig();
                    config.bAutoHealParty = !config.bAutoHealParty;
                });
            c.BindEventCallback("muhelper_detail_party_duration",
                [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                {
                    ConfigData& config = UI::MuHelper::StagedConfig();
                    config.bBuffDurationParty = !config.bBuffDurationParty;
                });
            c.BindEventCallback("muhelper_detail_save",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                {
                    Save();
                    g_pNewUISystem->Hide(INTERFACE_MUHELPER_EXT);
                });
            c.BindEventCallback("muhelper_detail_init",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { Reset(); });
            c.BindEventCallback("muhelper_detail_close",
                [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                {
                    g_pNewUISystem->Hide(INTERFACE_MUHELPER_EXT);
                });
        });

    if (!modelCreated)
        return;

    MuHelperDetailLabels& l = m_RmlBinder.GetModel().labels;
    l.titleActivation = Narrow(I18N::Game::ActivationSkill);
    l.titleRecovery = Narrow(I18N::Game::AutoRecovery);
    l.titleParty = Narrow(I18N::Game::Party);
    l.panePreCon = Narrow(I18N::Game::PreCon);
    l.paneSubCon = Narrow(I18N::Game::SubCon);
    l.paneAutoPotion = Narrow(I18N::Game::AutoPotion);
    l.paneAutoHeal = Narrow(I18N::Game::AutoHeal);
    l.paneDrainLife = Narrow(I18N::Game::DrainLife);
    l.paneBuffSupport = Narrow(I18N::Game::BuffSupport);
    l.paneHealSupport = Narrow(I18N::Game::HealSupport);
    l.hpStatus = Narrow(I18N::Game::HPStatus);
    l.hpStatusParty = Narrow(I18N::Game::HPStatusOfPartyMembers);
    l.preconHuntRange = Narrow(I18N::Game::MonsterWithinHuntingRange);
    l.preconAttacking = Narrow(I18N::Game::MonsterAttackingMe);
    l.subconTwo = Narrow(I18N::Game::MoreThan2Mobs);
    l.subconThree = Narrow(I18N::Game::MoreThan3Mobs);
    l.subconFour = Narrow(I18N::Game::MoreThan4Mobs);
    l.subconFive = Narrow(I18N::Game::MoreThan5Mobs);
    l.partyHeal = Narrow(I18N::Game::PreferenceOfPartyHeal);
    l.partyDuration = Narrow(I18N::Game::BuffDurationForAllPartyMembers);
    l.timeSpace = Narrow(I18N::Game::TimeSpaceOfCastingBuff);
    l.save = Narrow(I18N::Game::SaveSetting);
    l.init = Narrow(I18N::Game::Initialization);
    l.closeTip = Narrow(I18N::Game::Close388);

    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                  "Data/Interface/RmlUi/mu_helper_detail.rml");
    if (!m_pRmlDoc)
        return;

    UI::RmlBridge::AttachNumericInputFilter(m_pRmlDoc);
    // Set from code so the limit can't drift per theme.
    for (const char* id : BuffIntervalFieldIds)
    {
        if (Rml::Element* field = m_pRmlDoc->GetElementById(id))
            field->SetAttribute("maxlength", UI::MuHelper::MaxNumberDigits);
    }
}

void CMuHelperDetailWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return; // never opened -- BuildRmlUi() picks up the new theme whenever it first is

    // The field's typed value lives in the model, which Destroy() resets -- carry it across.
    const Rml::String typed = m_RmlBinder.GetModel().buffInterval;

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();

    m_RmlBinder.GetModel().buffInterval = typed;
    m_RmlBinder.MarkDirty("buff_interval");
    // Next frame's SyncRmlModel() self-corrects visibility and every other value.
}

void CMuHelperDetailWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    UI::RmlBridge::SyncDocumentVisibility(m_pRmlDoc, IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, m_Pos);
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);

    MuHelperDetailRmlModel& model = m_RmlBinder.GetModel();
    const ConfigData& config = UI::MuHelper::StagedConfig();


    SyncField(m_RmlBinder, &MuHelperDetailRmlModel::page, "page", m_iCurrentPage);

    // The condition radios and the party checkboxes read the staged config live -- they write it
    // directly when clicked, so it is the one source of truth for what they show.
    int precon = -1;
    int subcon = -1;
    if (IsSkillPage(m_iCurrentPage))
    {
        const uint32_t bits = config.aiSkillCondition[SkillConditionIndex(m_iCurrentPage)];
        if (bits & ON_MOBS_NEARBY)
            precon = 0;
        else if (bits & ON_MOBS_ATTACKING)
            precon = 1;
        for (int i = 0; i < 4; ++i)
        {
            if (bits & SubConditionBits[i])
            {
                subcon = i;
                break;
            }
        }
    }
    SyncField(m_RmlBinder, &MuHelperDetailRmlModel::precon, "precon", precon);
    SyncField(m_RmlBinder, &MuHelperDetailRmlModel::subcon, "subcon", subcon);

    SyncField(m_RmlBinder, &MuHelperDetailRmlModel::potionLevel, "potion_level", m_iCurrentPotionThreshold);
    SyncField(m_RmlBinder, &MuHelperDetailRmlModel::healLevel, "heal_level", m_iCurrentHealThreshold);
    SyncField(m_RmlBinder, &MuHelperDetailRmlModel::partyHealLevel, "party_heal_level", m_iCurrentPartyHealThreshold);
    SyncField(m_RmlBinder, &MuHelperDetailRmlModel::partyHeal, "party_heal", config.bAutoHealParty);
    SyncField(m_RmlBinder, &MuHelperDetailRmlModel::partyDuration, "party_duration", config.bBuffDurationParty);
}
