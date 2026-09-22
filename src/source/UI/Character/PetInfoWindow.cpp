
#include "stdafx.h"
#include "I18N/All.h"

#include "UI/Character/PetInfoWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Scaling/UITransform.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Core/Utilities/StringUtils.h"

#include "GameLogic/Pets/GIPetManager.h"
#include "Character/CharacterManager.h"
#include "GameLogic/Skills/SkillManager.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

using namespace SEASON3B;
using namespace mu::ui::window;

CPetInfoWindow::CPetInfoWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_aiDamage[0] = m_aiDamage[1] = 0;
    m_fAddDamagePercent = 0.f;
}

CPetInfoWindow::~CPetInfoWindow()
{
    Release();
}

bool CPetInfoWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_PET, this);

    SetPos(x, y);

    if (RmlUiRuntime::Instance().IsCreated())
    {
        BuildRmlUi();
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }

    Show(false);

    return true;
}

void CPetInfoWindow::BuildRmlUi()
{
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "pet_info",
            [this](Rml::DataModelConstructor& c, PetInfoRmlModel& model)
            {
                c.Bind("root_x", &model.rootX);
                c.Bind("root_y", &model.rootY);
                c.Bind("root_scale", &model.rootScale);

                c.Bind("active_tab", &model.activeTab);
                c.Bind("window_title", &model.windowTitle);
                c.Bind("tab_darkhorse_label", &model.tabDarkHorseLabel);
                c.Bind("tab_darkspirit_label", &model.tabDarkSpiritLabel);
                c.Bind("exit_tooltip", &model.exitTooltip);

                c.Bind("dh_has_pet", &model.dhHasPet);
                c.Bind("dh_no_pet_text", &model.dhNoPetText);
                c.Bind("dh_level_text", &model.dhLevelText);
                c.Bind("dh_life_text", &model.dhLifeText);
                c.Bind("dh_hp_percent", &model.dhHpPercent);
                c.Bind("dh_exp_text", &model.dhExpText);
                c.Bind("dh_dmg_text", &model.dhDmgText);
                c.Bind("dh_atkspeed_text", &model.dhAtkSpeedText);

                c.Bind("ds_has_pet", &model.dsHasPet);
                c.Bind("ds_no_pet_text", &model.dsNoPetText);
                c.Bind("ds_level_text", &model.dsLevelText);
                c.Bind("ds_life_text", &model.dsLifeText);
                c.Bind("ds_hp_percent", &model.dsHpPercent);
                c.Bind("ds_exp_text", &model.dsExpText);
                c.Bind("ds_dmg_text", &model.dsDmgText);
                c.Bind("ds_atkspeed_text", &model.dsAtkSpeedText);
                c.Bind("ds_charisma_text", &model.dsCharismaText);

                c.Bind("commands_label", &model.commandsLabel);
                c.Bind("skill_basicaction_label", &model.skillBasicActionLabel);
                c.Bind("skill_randomattack_label", &model.skillRandomAttackLabel);
                c.Bind("skill_attackwithowner_label", &model.skillAttackWithOwnerLabel);
                c.Bind("skill_attacktarget_label", &model.skillAttackTargetLabel);

                c.BindEventCallback("petinfo_select_tab",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                    {
                        if (arguments.size() == 1)
                            RmlClickSelectTab(arguments[0].Get<int>(-1));
                    });
                c.BindEventCallback("petinfo_click_exit",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickExit(); });
            });

        if (modelCreated)
        {
            auto& model = m_RmlBinder.GetModel();
            model.windowTitle = StringUtils::WideToNarrow(I18N::Game::Pet);
            model.tabDarkHorseLabel = StringUtils::WideToNarrow(I18N::Game::DarkHorse);
            model.tabDarkSpiritLabel = StringUtils::WideToNarrow(I18N::Game::DarkRaven);
            model.exitTooltip = StringUtils::WideToNarrow(I18N::Game::Close388);

            wchar_t szText[256] = { 0, };
            mu_swprintf(szText, I18N::Game::NoS, I18N::Game::DarkHorse);
            model.dhNoPetText = StringUtils::WideToNarrow(szText);
            model.dsNoPetText = StringUtils::WideToNarrow(I18N::Game::NoPet);

            model.commandsLabel = StringUtils::WideToNarrow(I18N::Game::Commands);
            model.skillBasicActionLabel = StringUtils::WideToNarrow(I18N::Game::BasicAction);
            model.skillRandomAttackLabel = StringUtils::WideToNarrow(I18N::Game::RandomAutomaticAttack);
            model.skillAttackWithOwnerLabel = StringUtils::WideToNarrow(I18N::Game::AttackWithOwner);
            model.skillAttackTargetLabel = StringUtils::WideToNarrow(I18N::Game::AttackTarget);
        }

    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
        "Data/Interface/RmlUi/pet_info.rml");
}

void CPetInfoWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return; // never opened -- BuildRmlUi() will simply pick up the new theme whenever it first is

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    // Next frame's SyncRmlModel() self-corrects visibility/live model state.
}

void CPetInfoWindow::Release()
{
    if (m_pRmlDoc)
    {
        m_pRmlDoc->Close();
        m_pRmlDoc = nullptr;
    }

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        UI::RmlBridge::UnregisterForThemeReload(this);
        m_pNewUIMng = NULL;
    }
}

void CPetInfoWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

void CPetInfoWindow::Show(bool bShow)
{
    mu::ui::window::CObject::Show(bShow);
    if (m_pRmlDoc)
    {
        if (bShow) m_pRmlDoc->Show();
        else m_pRmlDoc->Hide();
    }
}

bool CPetInfoWindow::UpdateMouseEvent()
{
    // Top-right corner close "X" (shared frame). Hides + swallows the click.
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_PET))
        return false;

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, PETINFOWINDOW_WIDTH, PETINFOWINDOW_HEIGHT).Contains(MouseX, MouseY))
        return false;

    return true;
}

bool CPetInfoWindow::UpdateKeyEvent()
{
    return true;
}

bool CPetInfoWindow::Update()
{
    SyncRmlModel();
    return true;
}

bool CPetInfoWindow::Render()
{
    // RmlUi's #panel owns all chrome/text/button rendering now; nothing left to draw natively.
    return true;
}

float CPetInfoWindow::GetLayerDepth()
{
    return 2.3f;
}

void CPetInfoWindow::OpenningProcess()
{
    SyncRmlModel();
}

void CPetInfoWindow::ClosingProcess()
{
}

void CPetInfoWindow::RmlClickSelectTab(int tab)
{
    if (tab != TAB_TYPE_DARKHORSE && tab != TAB_TYPE_DARKSPIRIT)
        return;

    auto& model = m_RmlBinder.GetModel();
    if (model.activeTab == tab)
        return;

    model.activeTab = tab;
    m_RmlBinder.MarkDirty("active_tab");
}

void CPetInfoWindow::RmlClickExit()
{
    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_PET);
}

void CPetInfoWindow::CalcDamage(int iNumTapButton)
{
    PET_INFO* pPetInfo = NULL;

    switch (iNumTapButton)
    {
    case TAB_TYPE_DARKHORSE:
    {
        int iSkillDamage[2];
        int master_boost = 0;
        gCharacterManager.GetSkillDamage(AT_SKILL_EARTHSHAKE, &iSkillDamage[0], &iSkillDamage[1]);
        auto masterLevelUi = mu::ui::window::CSystem::GetInstance()->GetUI_NewMasterLevelInterface();
        if (masterLevelUi != nullptr)
        {
            master_boost = static_cast<int>(CharacterAttribute->MasterSkillInfo[AT_SKILL_EARTHSHAKE_STR].GetSkillValue());
        }

        pPetInfo = Hero->GetEquipedPetInfo(PET_TYPE_DARK_HORSE);
        m_aiDamage[0] = pPetInfo->m_wDamageMin + iSkillDamage[0] + master_boost;
        m_aiDamage[1] = pPetInfo->m_wDamageMax + iSkillDamage[1] + master_boost;
    }
    break;
    case TAB_TYPE_DARKSPIRIT:
    {
        m_fAddDamagePercent = 0.f;
        m_aiDamage[0] = 0;
        m_aiDamage[1] = 0;

        if (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type != -1)
        {
            m_fAddDamagePercent = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].MagicPower;
        }

        pPetInfo = Hero->GetEquipedPetInfo(PET_TYPE_DARK_SPIRIT);

        m_fAddDamagePercent /= 100.f;
        m_aiDamage[0] = pPetInfo->m_wDamageMin + (int)(pPetInfo->m_wDamageMin * m_fAddDamagePercent);
        m_aiDamage[1] = pPetInfo->m_wDamageMax + (int)(pPetInfo->m_wDamageMax * m_fAddDamagePercent);
    }
    break;
    }
}

void CPetInfoWindow::SyncRmlModel()
{
    if (!m_pRmlDoc)
        return;

    auto& model = m_RmlBinder.GetModel();
    wchar_t szText[256] = { 0, };

    const auto transform = UI::Scaling::GetActiveTransform();
    model.rootX = static_cast<float>(m_Pos.x) * transform.scaleX + transform.offsetX;
    model.rootY = static_cast<float>(m_Pos.y) * transform.scaleY + transform.offsetY;
    model.rootScale = transform.scaleX;
    m_RmlBinder.MarkDirty("root_x");
    m_RmlBinder.MarkDirty("root_y");
    m_RmlBinder.MarkDirty("root_scale");

    // Dark Horse tab
    {
        PET_INFO* pPetInfo = Hero->GetEquipedPetInfo(PET_TYPE_DARK_HORSE);
        const bool hasPet = pPetInfo->m_dwPetType != PET_TYPE_NONE;
        if (model.dhHasPet != hasPet)
        {
            model.dhHasPet = hasPet;
            m_RmlBinder.MarkDirty("dh_has_pet");
        }

        if (hasPet)
        {
            CalcDamage(TAB_TYPE_DARKHORSE);

            mu_swprintf(szText, I18N::Game::LevelD, pPetInfo->m_wLevel);
            model.dhLevelText = StringUtils::WideToNarrow(szText);
            m_RmlBinder.MarkDirty("dh_level_text");

            mu_swprintf(szText, I18N::Game::LifeDD, pPetInfo->m_wLife, 255);
            model.dhLifeText = StringUtils::WideToNarrow(szText);
            m_RmlBinder.MarkDirty("dh_life_text");

            model.dhHpPercent = (std::min<int>(pPetInfo->m_wLife, 255) * 100.f) / 255.f;
            m_RmlBinder.MarkDirty("dh_hp_percent");

            mu_swprintf(szText, I18N::Game::ExpDD, pPetInfo->m_dwExp1, pPetInfo->m_dwExp2);
            model.dhExpText = StringUtils::WideToNarrow(szText);
            m_RmlBinder.MarkDirty("dh_exp_text");

            mu_swprintf(szText, I18N::Game::DmgRateDDD, m_aiDamage[0], m_aiDamage[1], pPetInfo->m_wAttackSuccess);
            model.dhDmgText = StringUtils::WideToNarrow(szText);
            m_RmlBinder.MarkDirty("dh_dmg_text");

            mu_swprintf(szText, I18N::Game::AttackSpeedD, pPetInfo->m_wAttackSpeed);
            model.dhAtkSpeedText = StringUtils::WideToNarrow(szText);
            m_RmlBinder.MarkDirty("dh_atkspeed_text");
        }
    }

    // Dark Spirit tab
    {
        PET_INFO* pPetInfo = Hero->GetEquipedPetInfo(PET_TYPE_DARK_SPIRIT);
        const bool hasPet = pPetInfo->m_dwPetType != PET_TYPE_NONE;
        if (model.dsHasPet != hasPet)
        {
            model.dsHasPet = hasPet;
            m_RmlBinder.MarkDirty("ds_has_pet");
        }

        if (hasPet)
        {
            CalcDamage(TAB_TYPE_DARKSPIRIT);

            mu_swprintf(szText, I18N::Game::LevelD, pPetInfo->m_wLevel);
            model.dsLevelText = StringUtils::WideToNarrow(szText);
            m_RmlBinder.MarkDirty("ds_level_text");

            mu_swprintf(szText, I18N::Game::LifeDD, pPetInfo->m_wLife, 255);
            model.dsLifeText = StringUtils::WideToNarrow(szText);
            m_RmlBinder.MarkDirty("ds_life_text");

            model.dsHpPercent = (std::min<int>(pPetInfo->m_wLife, 255) * 100.f) / 255.f;
            m_RmlBinder.MarkDirty("ds_hp_percent");

            mu_swprintf(szText, I18N::Game::ExpDD, pPetInfo->m_dwExp1, pPetInfo->m_dwExp2);
            model.dsExpText = StringUtils::WideToNarrow(szText);
            m_RmlBinder.MarkDirty("ds_exp_text");

            mu_swprintf(szText, I18N::Game::DmgRateDDD, m_aiDamage[0], m_aiDamage[1], pPetInfo->m_wAttackSuccess);
            model.dsDmgText = StringUtils::WideToNarrow(szText);
            m_RmlBinder.MarkDirty("ds_dmg_text");

            mu_swprintf(szText, I18N::Game::AttackSpeedD, pPetInfo->m_wAttackSpeed);
            model.dsAtkSpeedText = StringUtils::WideToNarrow(szText);
            m_RmlBinder.MarkDirty("ds_atkspeed_text");

            mu_swprintf(szText, I18N::Game::CharismaRequirementD, (185 + (pPetInfo->m_wLevel * 15)));
            model.dsCharismaText = StringUtils::WideToNarrow(szText);
            m_RmlBinder.MarkDirty("ds_charisma_text");
        }
    }
}
