#include "stdafx.h"
#include "UI/MuHelper/MuHelperSkillPicker.h"

#include "Core/Globals/_enum.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Core/WindowManager.h"
#include "UI/Core/WindowSystem.h"
#include "UI/MuHelper/MuHelperShared.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlElementBox.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlSyncField.h"
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

    Show(false);

    return true;
}

void CMuHelperSkillPicker::Release()
{
    m_RmlView.Release();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }
}

bool CMuHelperSkillPicker::UpdateMouseEvent()
{
    // Only the icons take the pointer; everywhere else passes through.
    return !UI::RmlBridge::IsPointerOver(m_RmlView.Document());
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
    const float startX = -BoxWidth;
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

void CMuHelperSkillPicker::Pick(int index)
{
    if (!IsVisible() || index < 0 || index >= static_cast<int>(m_placements.size()))
        return;
    g_pMuHelperConfig->AssignSkill(m_placements[static_cast<size_t>(index)].skillType);
    Show(false);
}

void CMuHelperSkillPicker::BindRmlModel(Rml::DataModelConstructor& c, MuHelperSkillPickerRmlModel& model)
{
    c.Bind("origin_x", &model.originX);
    c.Bind("origin_y", &model.originY);
    c.Bind("scale", &model.scale);

    auto entry = c.RegisterStruct<MuHelperSkillPickerEntry>();
    entry.RegisterMember("left", &MuHelperSkillPickerEntry::left);
    entry.RegisterMember("top", &MuHelperSkillPickerEntry::top);
    entry.RegisterMember("decorator", &MuHelperSkillPickerEntry::decorator);
    c.RegisterArray<std::vector<MuHelperSkillPickerEntry>>();
    c.Bind("entries", &model.entries);
    c.BindEventCallback("mhsp_pick", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                        {
                            if (args.size() == 1)
                                Pick(args[0].Get<int>(-1));
                        });
}

void CMuHelperSkillPicker::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void CMuHelperSkillPicker::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    SyncOrigin();

    if (!m_bEntriesDirty)
        return;
    m_bEntriesDirty = false;

    MuHelperSkillPickerRmlModel& model = m_RmlView.GetModel();
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
    m_RmlView.MarkDirty("entries");
}

// The flyout follows the config window wherever the workspace placed or the player dragged it.
void CMuHelperSkillPicker::SyncOrigin()
{
    Rml::ElementDocument* config = g_pMuHelperConfig ? g_pMuHelperConfig->GetPlacedDocument() : nullptr;
    Rml::Element* panel = config ? config->GetElementById("panel") : nullptr;
    Rml::Vector2f origin;
    if (panel == nullptr || !UI::RmlBridge::DrawnTopLeft(*panel, origin))
        return;
    SyncField(m_RmlView.Binder(), &MuHelperSkillPickerRmlModel::originX, "origin_x", origin.x);
    SyncField(m_RmlView.Binder(), &MuHelperSkillPickerRmlModel::originY, "origin_y", origin.y);
    SyncField(m_RmlView.Binder(), &MuHelperSkillPickerRmlModel::scale, "scale", UI::RmlBridge::DrawnScale(*panel));
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
