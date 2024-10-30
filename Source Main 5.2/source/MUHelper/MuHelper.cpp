#include "stdafx.h"
#include <thread>
#include <atomic>
#include <chrono>

#include "MuHelper.h"
#include "Utilities/Log/muConsoleDebug.h"

CMuHelper g_MuHelper;

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

void CMuHelper::WorkLoop()
{
    while (m_bActive) {
        Work();
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

void CMuHelper::Work()
{
    g_ConsoleDebug->Write(MCD_NORMAL, L"Working...");
}