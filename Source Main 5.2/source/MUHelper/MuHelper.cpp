#include "stdafx.h"

#include <thread>
#include <atomic>
#include <chrono>

#include "ZzzAI.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "NewUISystem.h"
#include "Utilities/Log/muConsoleDebug.h"
#include "SkillManager.h"
#include "WSclient.h"

#include "MuHelper.h"


CMuHelper g_MuHelper;

CMuHelper::CMuHelper()
{
    m_iCurrentTarget = -1;
    m_iComboState = 0;
    m_iCurrentSkill = 1;
}

void CMuHelper::Save(const cMuHelperConfig& config)
{
    m_config = config;

    // Save config data by sending to server
}

void CMuHelper::Load(const cMuHelperConfig& config)
{
    // Load config data received from the server
    m_config = config;
}

void CMuHelper::Start()
{
    if (m_bActive)
    {
        g_ConsoleDebug->Write(MCD_NORMAL, L"MU Helper is already active!");
        return;
    }

    m_bActive = true;
    m_iComboState = 0;
    m_iCurrentTarget = -1;
    m_iCurrentSkill = m_config.iBasicSkill;
    m_timerThread = std::thread(&CMuHelper::WorkLoop, this);

    g_ConsoleDebug->Write(MCD_NORMAL, L"MU Helper started");
}

void CMuHelper::Stop()
{
    m_bActive = false;
    if (m_timerThread.joinable()) {
        m_timerThread.join();
    }

    g_ConsoleDebug->Write(MCD_NORMAL, L"MU Helper stopped");
}

void CMuHelper::AddTarget(int iTargetId)
{
    if (iTargetId == HeroKey || m_setTargets.find(iTargetId) != m_setTargets.end()) {
        return;
    }

    if (m_queuedTargets.size() >= 3) {
        int oldest = m_queuedTargets.front();
        m_queuedTargets.pop_front();
        m_setTargets.erase(oldest);
    }

    m_queuedTargets.push_back(iTargetId);
    m_setTargets.insert(iTargetId);

    g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Added target -> %d", iTargetId);
}

void CMuHelper::DeleteTarget(int iTargetId)
{
    auto it = std::find(m_queuedTargets.begin(), m_queuedTargets.end(), iTargetId);
    if (it != m_queuedTargets.end()) {
        m_queuedTargets.erase(it);
        m_setTargets.erase(iTargetId);
    }

    if (iTargetId == m_iCurrentTarget)
    {
        m_iCurrentTarget = -1;
    }
}

void CMuHelper::WorkLoop()
{
    while (m_bActive) {
        Work();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}

void CMuHelper::Work()
{
    Attack();
}

int CMuHelper::Attack()
{
    if (m_iCurrentTarget == -1 && !m_queuedTargets.empty())
    {
        m_iCurrentTarget = m_queuedTargets.front();
        m_queuedTargets.pop_front();
        m_setTargets.erase(m_iCurrentTarget);
    }

    if (m_config.bUseCombo)
    {
        SimulateComboAttack();
    }
    else
    {
        SimulateAttack(m_iCurrentSkill);
    }

    return 1;
}

int CMuHelper::SimulateComboAttack()
{
    if (!m_config.iBasicSkill || !m_config.iSkill2 || !m_config.iSkill3)
    {
        return 0;
    }

    if (m_iComboState == 0 && SimulateAttack(m_config.iBasicSkill))
    {
        m_iComboState = 1;
    }
    else if (m_iComboState == 1 && SimulateAttack(m_config.iSkill2))
    {
        m_iComboState = 2;
    }
    else if (m_iComboState == 2 && SimulateAttack(m_config.iSkill3))
    {
        m_iComboState = 0;
    }

    return 1;
}

int CMuHelper::SimulateAttack(int iSkill)
{
    extern MovementSkill g_MovementSkill;
    extern int SelectedCharacter;
    extern int TargetX, TargetY;

    if (m_iCurrentTarget == -1)
    {
        return 0;
    }

    SelectedCharacter = FindCharacterIndex(m_iCurrentTarget);
    if (SelectedCharacter == MAX_CHARACTERS_CLIENT)
    {
        DeleteTarget(m_iCurrentTarget);
        return 0;
    }

    CHARACTER* pTarget = &CharactersClient[SelectedCharacter];
    if (pTarget->Dead)
    {
        DeleteTarget(m_iCurrentTarget);
        return 0;
    }

    int iSkillIndex = g_pSkillList->GetSkillIndex(iSkill);
    Hero->CurrentSkill = iSkillIndex;

    g_MovementSkill.m_iSkill = iSkill;
    g_MovementSkill.m_iTarget = SelectedCharacter;
    g_MovementSkill.m_bMagic = FALSE;

    TargetX = (int)(pTarget->Object.Position[0] / TERRAIN_SCALE);
    TargetY = (int)(pTarget->Object.Position[1] / TERRAIN_SCALE);

    // Attack is considered completed only when no longer path finding
    return (int)(ExecuteAttack(Hero) && !Hero->Movement);
}
