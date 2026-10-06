#include "stdafx.h"
#include "UI/MuHelper/MuHelperSkillPicker.h"

#include "Core/Globals/_enum.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Core/WindowManager.h"
#include "UI/Core/WindowSystem.h"
#include "UI/MuHelper/MuHelperShared.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Social/SocialWindowBase.h"

#include <RmlUi/Core/ElementDocument.h>

using namespace mu::ui::window;

CMuHelperSkillPicker::CMuHelperSkillPicker() = default;

CMuHelperSkillPicker::~CMuHelperSkillPicker()
{
    Release();
}

bool CMuHelperSkillPicker::Create(CManager* pNewUIMng)
{
    if (pNewUIMng == nullptr)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(INTERFACE_MUHELPER_SKILL_LIST, this);

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void CMuHelperSkillPicker::Release()
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

bool CMuHelperSkillPicker::UpdateMouseEvent()
{
    const int skillType = SkillUnderMouse();

    // Only the icons claim the mouse. Everywhere else passes through -- to the world, so the
    // character can still move while the flyout is open, and to the config window, which is how a
    // second click on the same slot reaches it and closes the flyout.
    if (skillType == -1)
        return true;

    if (IsRelease(VK_LBUTTON))
    {
        g_pMuHelperConfig->AssignSkill(skillType);
        Show(false);
    }
    return false;
}

bool CMuHelperSkillPicker::UpdateKeyEvent()
{
    if (IsVisible() && IsPress(VK_ESCAPE))
    {
        g_pNewUISystem->Hide(INTERFACE_MUHELPER_SKILL_LIST);
        SetFocus(g_hWnd);
        return false;
    }
    return true;
}

bool CMuHelperSkillPicker::Update()
{
    SyncRmlModel();
    return true;
}

bool CMuHelperSkillPicker::Render()
{
    // Every box and icon is RmlUi's. Kept because CObject requires the override.
    return true;
}

float CMuHelperSkillPicker::GetLayerDepth()
{
    return 5.2f;
}

void CMuHelperSkillPicker::FilterByAttackSkills()
{
    m_bFilterByAttackSkills = true;
    m_bFilterByBuffSkills = false;
    PrepareSkillsToRender();
}

void CMuHelperSkillPicker::FilterByBuffSkills()
{
    m_bFilterByBuffSkills = true;
    m_bFilterByAttackSkills = false;
    PrepareSkillsToRender();
}

void CMuHelperSkillPicker::PrepareSkillsToRender()
{
    m_aiSkillsToRender.clear();

    if (CharacterAttribute->SkillNumber > 0)
    {
        for (int i = 0; i < MAX_MAGIC; ++i)
        {
            const int iSkillType = CharacterAttribute->Skill[i];
            if (iSkillType == 0 || (iSkillType >= AT_SKILL_STUN && iSkillType <= AT_SKILL_REMOVAL_BUFF))
                continue;

            const BYTE bySkillUseType = SkillAttribute[iSkillType].SkillUseType;
            if (bySkillUseType == SKILL_USE_TYPE_MASTER || bySkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
                continue;

            if ((m_bFilterByAttackSkills && IsAttackSkill(iSkillType))
                || (m_bFilterByBuffSkills && IsBuffSkill(iSkillType)))
            {
                m_aiSkillsToRender.push_back(iSkillType);
            }
        }
    }

    LayoutPlacements();
}

// Native's fan-out, unchanged: columns grow right to left from just outside the config window's own
// left edge, and within a column the entries alternate above and below the row of the slot that
// opened the flyout -- attack slots sit at y=171, buff slots at y=293 (config window reference
// space). This is a genuinely computed layout, not a static one, which is why it stays in C++.
void CMuHelperSkillPicker::LayoutPlacements()
{
    const float startX = static_cast<float>(REFERENCE_WIDTH) - 190.f - BoxWidth;
    const float startY = m_bFilterByAttackSkills ? 171.f : 293.f;
    const int itemsPerColumn = m_bFilterByAttackSkills ? 10 : 5;

    m_placements.clear();
    m_placements.reserve(m_aiSkillsToRender.size());

    for (size_t i = 0; i < m_aiSkillsToRender.size(); ++i)
    {
        const int col = static_cast<int>(i) / itemsPerColumn;
        const int rowInColumn = static_cast<int>(i) % itemsPerColumn;
        const int offset = (rowInColumn + 1) / 2;
        const bool above = (rowInColumn % 2 == 0);

        Placement p;
        p.skillType = m_aiSkillsToRender[i];
        p.left = startX - col * BoxWidth;
        p.top = above ? startY - offset * BoxHeight : startY + offset * BoxHeight;
        m_placements.push_back(p);
    }

    m_bEntriesDirty = true;
}

// The same rects the icons are drawn at, in the reference space MouseX/MouseY are already mapped
// into for this window -- no read-back from RmlUi, so no transform conversion anywhere.
int CMuHelperSkillPicker::SkillUnderMouse() const
{
    for (const Placement& p : m_placements)
    {
        if (CheckMouseIn(static_cast<int>(p.left), static_cast<int>(p.top),
                         static_cast<int>(BoxWidth), static_cast<int>(BoxHeight)))
            return p.skillType;
    }
    return -1;
}

void CMuHelperSkillPicker::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "mu_helper_skill_picker",
        [](Rml::DataModelConstructor& c, MuHelperSkillPickerRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);

            auto entry = c.RegisterStruct<MuHelperSkillPickerEntry>();
            entry.RegisterMember("left", &MuHelperSkillPickerEntry::left);
            entry.RegisterMember("top", &MuHelperSkillPickerEntry::top);
            entry.RegisterMember("decorator", &MuHelperSkillPickerEntry::decorator);
            c.RegisterArray<std::vector<MuHelperSkillPickerEntry>>();
            c.Bind("entries", &model.entries);
        });

    if (modelCreated)
    {
        m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                      "Data/Interface/RmlUi/mu_helper_skill_picker.rml");
        m_bEntriesDirty = true;
    }
}

void CMuHelperSkillPicker::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return; // never opened -- BuildRmlUi() picks up the new theme whenever it first is

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    // Next frame's SyncRmlModel() self-corrects visibility and republishes the entries.
}

void CMuHelperSkillPicker::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    UI::RmlBridge::SyncDocumentVisibility(m_pRmlDoc, IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, POINT{ 0, 0 });

    if (!m_bEntriesDirty)
        return;
    m_bEntriesDirty = false;

    MuHelperSkillPickerRmlModel& model = m_RmlBinder.GetModel();
    model.entries.clear();
    model.entries.reserve(m_placements.size());
    for (const Placement& p : m_placements)
    {
        MuHelperSkillPickerEntry e;
        e.left = p.left;
        e.top = p.top;
        e.decorator = UI::MuHelper::SkillIconDecorator(p.skillType);
        model.entries.push_back(std::move(e));
    }
    m_RmlBinder.MarkDirty("entries");
}

bool CMuHelperSkillPicker::IsAttackSkill(int iSkillType)
{
    return !IsBuffSkill(iSkillType) && !IsDefenseSkill(iSkillType) && !IsHealingSkill(iSkillType);
}

bool CMuHelperSkillPicker::IsBuffSkill(int iSkillType)
{
    switch (iSkillType)
    {
    // BK
    case AT_SKILL_SWELL_LIFE:
    case AT_SKILL_SWELL_LIFE_STR:
    case AT_SKILL_SWELL_LIFE_PROFICIENCY:
    // Elf
    case AT_SKILL_INFINITY_ARROW:
    case AT_SKILL_INFINITY_ARROW_STR:
    case AT_SKILL_DEFENSE:
    case AT_SKILL_DEFENSE_STR:
    case AT_SKILL_DEFENSE_MASTERY:
    case AT_SKILL_ATTACK:
    case AT_SKILL_ATTACK_STR:
    case AT_SKILL_ATTACK_MASTERY:
    // DW
    case AT_SKILL_SOUL_BARRIER:
    case AT_SKILL_SOUL_BARRIER_STR:
    case AT_SKILL_SOUL_BARRIER_PROFICIENCY:
    case AT_SKILL_EXPANSION_OF_WIZARDRY:
    case AT_SKILL_EXPANSION_OF_WIZARDRY_STR:
    case AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY:
    // DL
    case AT_SKILL_ADD_CRITICAL:
    case AT_SKILL_ADD_CRITICAL_STR1:
    case AT_SKILL_ADD_CRITICAL_STR2:
    case AT_SKILL_ADD_CRITICAL_STR3:
    // Summoner
    case AT_SKILL_ALICE_BERSERKER:
    case AT_SKILL_ALICE_BERSERKER_STR:
    case AT_SKILL_ALICE_THORNS:
    // RF
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

bool CMuHelperSkillPicker::IsHealingSkill(int iSkillType)
{
    switch (iSkillType)
    {
    case AT_SKILL_HEALING:
    case AT_SKILL_HEALING_STR:
    case AT_SKILL_ALICE_DRAINLIFE:
    case AT_SKILL_ALICE_DRAINLIFE_STR:
        return true;
    }
    return false;
}

bool CMuHelperSkillPicker::IsDefenseSkill(int iSkillType)
{
    switch (iSkillType)
    {
    case AT_SKILL_DEFENSE:
    case AT_SKILL_DEFENSE_STR:
        return true;
    }
    return false;
}
