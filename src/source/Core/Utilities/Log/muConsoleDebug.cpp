
#include "stdafx.h"
#include <cmath>
#include <cstdlib>
#include <optional>
#include <string_view>
#include "Core/Utilities/Benchmark/BenchConsoleCommand.h"
#include "Core/Utilities/Log/MuLogger.h"

#include "muConsoleDebug.h"	// self

#ifdef _WIN32
#include <io.h>
#endif
#include <fcntl.h>
#include <iostream>
#include "Engine/Object/ZzzInterface.h"
#include "WindowsConsole.h"

#include "Render/Lighting/DynamicLightHistory.h"
#include "Render/PostProcessing/PostProcessingSetting.h"
#include "Render/Look/LookPresetSetting.h"
#include "Render/Renderer/Antialiasing.h"
#include "Render/Look/LookTuning.h"
#include "Core/Text/Utf8.h"
#include "Core/Text/NumberParse.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Textures/ZzzTexture.h"
#include "Scenes/SceneCore.h"
#include "Scenes/SceneManager.h"
#include "Scenes/MainScene.h"
#include "UI/Core/WindowCommon.h"
#include "UI/Core/WindowManager.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "UI/HUD/ChatLogWindow.h"
#include "UI/Core/SceneUICoordinator.h"
#include "UI/Events/EventPreview.h"
#include "UI/Theme/ThemeSelection.h"
#include "UI/Windows/RememberPasswordPrompt.h"
#include "Core/Utilities/StringUtils.h"

#ifdef _EDITOR
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#endif
#ifdef CSK_DEBUG_MAP_PATHFINDING
#include "Engine/Pathing/ZzzPath.h"
#endif // CSK_DEBUG_MAP_PATHFINDING

CmuConsoleDebug::CmuConsoleDebug() : m_bInit(false)
{
#ifdef _EDITOR
    // When editor is enabled, don't open the Windows console
    // All output will go to ImGui console instead
    m_bInit = true;
#elif defined(CSK_LH_DEBUG_CONSOLE)
    if (leaf::OpenConsoleWindow(L"Mu Debug Console Window"))
    {
        leaf::ActivateCloseButton(false);
        leaf::ShowConsole(true);
        m_bInit = true;

        g_ErrorReport.Write(L"Mu Debug Console Window Init - completed(Handle:0x%00000008X)\r\n", leaf::GetConsoleWndHandle());
    }
#endif
}

CmuConsoleDebug::~CmuConsoleDebug()
{
#ifndef _EDITOR
    #ifdef CSK_LH_DEBUG_CONSOLE
        leaf::CloseConsoleWindow();
    #endif
#endif
}

CmuConsoleDebug* CmuConsoleDebug::GetInstance()
{
    // Always return a valid instance. Previously returned nullptr in builds
    // without CSK_LH_DEBUG_CONSOLE, which made every g_ConsoleDebug->Write
    // call site a null-deref in disguise (it "worked" only because the Write
    // body was empty when CONSOLE_DEBUG was undefined). Returning a real
    // instance is required for the always-on MCD_ERROR path below to be safe.
    static CmuConsoleDebug sInstance;
    return &sInstance;
}

void CmuConsoleDebug::UpdateMainScene()
{
#ifdef CSK_LH_DEBUG_CONSOLE
    if (m_bInit)
    {
        if (mu::ui::window::IsPress(VK_SHIFT) == TRUE)
        {
            if (PressKey(VK_F7))
            {
                leaf::ShowConsole(!leaf::IsConsoleVisible());
            }
        }
    }
#endif
}

namespace
{
#if defined(MU_ENABLE_CONTROL_SOCKET) || defined(_EDITOR)
// Developer builds only: the look profile's live tuning ($look and its aliases), the per-feature
// session overrides of the look preset, the post-processing diagnostic views and the light
// history's counts.
bool CheckDeveloperLookCommand(const std::wstring& command)
{
    if (Render::Look::HandleDeveloperCommand(command) || Render::Look::HandleFeatureOverrideCommand(command))
    {
        return true;
    }
    if (command.starts_with(L"$post view "))
    {
        Render::PostProcessing::SetPostProcessingView(std::wstring_view(command).substr(11));
        return true;
    }
    if (command == L"$dynlights stats")
    {
        const Render::Lighting::DynamicLightHistory& history = Render::Lighting::FrameLightHistory();
        mu::log::Get("render")->info("Light history: {} map, {} character, {} effect lights followed, {} dropped",
                                     history.FollowedCount(Render::Lighting::LightSource::Map),
                                     history.FollowedCount(Render::Lighting::LightSource::Character),
                                     history.FollowedCount(Render::Lighting::LightSource::Effect), history.Overflow());
        mu::log::Get("render")->flush();
        return true;
    }
    if (command.starts_with(L"$lightmood ") || command.starts_with(L"$texfilter ") ||
        command.starts_with(L"$lighting ") || command.starts_with(L"$dynlights ") || command.starts_with(L"$post "))
    {
        mu::log::Get("render")->warn("{}: not understood (on or off; $lightmood also takes 0 to 100)",
                                     Core::Text::ToUtf8(command.c_str()));
        return true;
    }
    return false;
}
#endif
} // namespace

namespace
{
// Temporary testing aid for the UI ownership rollout: opens a window that normally needs an NPC,
// a map or a server state to reach. It only shows the window -- nothing seeds it, so a window fed
// entirely by the server (the guardsman's siege dates, the senatus' gate list) comes up with empty
// or default content. Good enough for layout, tabs, hit tests and theme switching; not for data.
struct DebugWindowName
{
    const wchar_t* name;
    DWORD key;
};

constexpr DebugWindowName kDebugWindows[] = {
    {L"guard", mu::ui::window::INTERFACE_GUARDSMAN},
    {L"luckycoin", mu::ui::window::INTERFACE_LUCKYCOIN_REGISTRATION},
    {L"coinexchange", mu::ui::window::INTERFACE_EXCHANGE_LUCKYCOIN},
    {L"npcquest", mu::ui::window::INTERFACE_NPCQUEST},
    {L"storage", mu::ui::window::INTERFACE_STORAGE},
    {L"storageext", mu::ui::window::INTERFACE_STORAGE_EXT},
    {L"npcshop", mu::ui::window::INTERFACE_NPCSHOP},
    {L"mix", mu::ui::window::INTERFACE_MIXINVENTORY},
    {L"trade", mu::ui::window::INTERFACE_TRADE},
    {L"inventoryext", mu::ui::window::INTERFACE_INVENTORY_EXT},
    {L"myshop", mu::ui::window::INTERFACE_MYSHOP_INVENTORY},
    {L"purchaseshop", mu::ui::window::INTERFACE_PURCHASESHOP_INVENTORY},
    {L"luckyitem", mu::ui::window::INTERFACE_LUCKYITEMWND},
    {L"ingameshop", mu::ui::window::INTERFACE_INGAMESHOP},
    {L"senatus", mu::ui::window::INTERFACE_SENATUS},
    {L"gateman", mu::ui::window::INTERFACE_GATEKEEPER},
    {L"gateswitch", mu::ui::window::INTERFACE_GATESWITCH},
    {L"catapult", mu::ui::window::INTERFACE_CATAPULT},
    {L"crywolf", mu::ui::window::INTERFACE_CRYWOLF},
    {L"siege", mu::ui::window::INTERFACE_SIEGEWARFARE},
    {L"guildinfo", mu::ui::window::INTERFACE_GUILDINFO},
    {L"duelwatch", mu::ui::window::INTERFACE_DUELWATCH},
    {L"gensranking", mu::ui::window::INTERFACE_GENSRANKING},
    {L"market", mu::ui::window::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA},
    {L"goldbowman", mu::ui::window::INTERFACE_GOLD_BOWMAN},
    {L"lena", mu::ui::window::INTERFACE_GOLD_BOWMAN_LENA},
    {L"masterlevel", mu::ui::window::INTERFACE_MASTER_LEVEL},
    {L"doppel", mu::ui::window::INTERFACE_DOPPELGANGER_NPC},
    {L"kanturu", mu::ui::window::INTERFACE_KANTURU2ND_ENTERNPC},
    {L"temple", mu::ui::window::INTERFACE_CURSEDTEMPLE_NPC},
    {L"empire", mu::ui::window::INTERFACE_EMPIREGUARDIAN_NPC},
    {L"bloodcastle", mu::ui::window::INTERFACE_BLOODCASTLE},
    {L"devilsquare", mu::ui::window::INTERFACE_DEVILSQUARE},
    {L"charinfo", mu::ui::window::INTERFACE_CHARACTER},
    {L"party", mu::ui::window::INTERFACE_PARTY_INFO_WINDOW},
    {L"pet", mu::ui::window::INTERFACE_PET},
    {L"quest", mu::ui::window::INTERFACE_MYQUEST},
    {L"muhelper", mu::ui::window::INTERFACE_MUHELPER},
    {L"option", mu::ui::window::INTERFACE_OPTION},
    {L"help", mu::ui::window::INTERFACE_HELP},
    {L"bctime", mu::ui::window::INTERFACE_BLOODCASTLE_TIME},
    {L"cctime", mu::ui::window::INTERFACE_CHAOSCASTLE_TIME},
    {L"soccer", mu::ui::window::INTERFACE_BATTLE_SOCCER_SCORE},
    {L"duel", mu::ui::window::INTERFACE_DUEL_WINDOW},
    {L"kanturuinfo", mu::ui::window::INTERFACE_KANTURU_INFO},
    {L"empiretimer", mu::ui::window::INTERFACE_EMPIREGUARDIAN_TIMER},
    {L"doppelframe", mu::ui::window::INTERFACE_DOPPELGANGER_FRAME},
    {L"duelusers", mu::ui::window::INTERFACE_DUELWATCH_USERLIST},
};

void ListDebugWindows()
{
    std::wstring line;
    for (const DebugWindowName& entry : kDebugWindows)
    {
        if (!line.empty())
            line += L' ';
        line += entry.name;
        // The log box wraps poorly on very long lines; break every ~70 characters.
        if (line.size() >= 70)
        {
            g_pSystemLogBox->AddText(line.c_str(), mu::ui::window::TYPE_SYSTEM_MESSAGE);
            line.clear();
        }
    }
    if (!line.empty())
        g_pSystemLogBox->AddText(line.c_str(), mu::ui::window::TYPE_SYSTEM_MESSAGE);
    g_pSystemLogBox->AddText(L"add \" full\" to open it the way the game does (asks the server; "
                             L"some windows cannot take that out of context)",
                             mu::ui::window::TYPE_SYSTEM_MESSAGE);
}

ConsoleCommandResult Handled()
{
    return {ConsoleCommandStatus::Handled, {}};
}

ConsoleCommandResult NotHandled()
{
    return {};
}

ConsoleCommandResult BadArgument(std::wstring message)
{
    return {ConsoleCommandStatus::BadArgument, std::move(message)};
}

// The names of the commands handled below. The argument offset comes from the literal, so a
// renamed command cannot leave a stale length behind.
constexpr std::wstring_view FpsCommand = L"$fps";
constexpr std::wstring_view WinMsgCommand = L"$winmsg";

// How a command text relates to one command name.
enum class CommandForm
{
    NotThisCommand,
    WithoutArgument,
    WithArgument,
    // Starts with the name, but the next character is neither the end nor a space: "$fps30",
    // "$fpscounter", "$winmsgx". A typo of the command, not a chat line; the caller shows the
    // usage hint.
    MalformedArgument,
};

CommandForm FormOfCommand(std::wstring_view command, std::wstring_view name)
{
    if (!command.starts_with(name))
    {
        return CommandForm::NotThisCommand;
    }
    if (command.size() == name.size())
    {
        return CommandForm::WithoutArgument;
    }
    return command[name.size()] == L' ' ? CommandForm::WithArgument : CommandForm::MalformedArgument;
}

// The argument slice when the text has one, nullopt for a command that has none or a malformed
// suffix; the branch reports the usage line in both cases.
std::optional<std::wstring_view> CommandArgument(CommandForm form, std::wstring_view command, std::wstring_view name)
{
    if (form != CommandForm::WithArgument)
    {
        return std::nullopt;
    }
    return command.substr(name.size());
}

// A frame cap has to leave the client able to react: below one frame per second the picture
// stops responding and reads as a crash, and above any panel the extra frames are wasted, as
// the loop still sleeps per frame. Both ends are wide enough for real hardware and monitors.
constexpr int MinTargetFps = 1;
constexpr int MaxTargetFps = 1000;

// Built from the limits so the hint cannot drift from what ParseTargetFps accepts.
std::wstring TargetFpsUsage()
{
    return L"usage: $fps <fps>, " + std::to_wstring(MinTargetFps) + L" to " + std::to_wstring(MaxTargetFps) +
           L", or -1 for uncapped";
}

// Parses the argument of "$fps": -1 (or any negative value) for uncapped, a frame cap otherwise.
// Non-finite and out-of-range values are rejected here rather than reaching SetTargetFps, which
// ignores zero without a word and divides by whatever it is given.
std::optional<double> ParseTargetFps(std::wstring_view argument)
{
    const std::optional<double> fps = Core::Text::TryParseDouble(argument);
    if (!fps || !std::isfinite(*fps))
    {
        return std::nullopt;
    }

    if (*fps > 0.0 && (*fps < MinTargetFps || *fps > MaxTargetFps))
    {
        return std::nullopt;
    }

    if (*fps == 0.0)
    {
        return std::nullopt;
    }

    return fps;
}
} // namespace

ConsoleCommandResult CmuConsoleDebug::CheckCommand(const std::wstring& strCommand)
{
    if (Core::Benchmark::Console::HandleCommand(strCommand))
    {
        return Handled();
    }

    // "$win <name>" toggles a window, "$win list" names them. Temporary; see kDebugWindows above.
    if (strCommand.compare(0, 4, L"$win") == 0 && strCommand.compare(0, 7, L"$winmsg") != 0)
    {
        std::wstring argument = strCommand.size() > 5 ? strCommand.substr(5) : std::wstring();
        if (argument.empty() || argument == L"list")
        {
            ListDebugWindows();
            return Handled();
        }

        // "$win <name> full" goes through CSystem::Show(), which runs the window's
        // OpeningProcess() -- and several of these ask the server for their contents, which is not
        // a request the server expects from a character standing nowhere near the NPC. The default
        // path shows the window itself and sends nothing, so the window comes up with no data but
        // cannot take the client down with it.
        bool viaOpeningProcess = false;
        if (argument.size() > 5 && argument.compare(argument.size() - 5, 5, L" full") == 0)
        {
            argument.resize(argument.size() - 5);
            viaOpeningProcess = true;
        }

        for (const DebugWindowName& entry : kDebugWindows)
        {
            if (argument != entry.name)
                continue;
            if (viaOpeningProcess)
            {
                g_pNewUISystem->Toggle(entry.key);
            }
            else if (mu::ui::window::CManager* manager = g_pNewUISystem->GetNewUIManager())
            {
                manager->ShowInterface(entry.key, !manager->IsInterfaceVisible(entry.key));
            }
            return Handled();
        }
        g_pSystemLogBox->AddText((L"no such window: " + argument).c_str(),
                                 mu::ui::window::TYPE_ERROR_MESSAGE);
        return Handled();
    }

    // "$preview <event>" fills an event window with sample data off its map; see EventPreview.h.
    if (strCommand.compare(0, 8, L"$preview") == 0)
    {
        UI::EventPreview::HandleCommand(strCommand.size() > 9 ? strCommand.substr(9) : std::wstring());
        return Handled();
    }

    // A test confirmation, to check the generic dialog (dragging, layout) without a game action.
    if (strCommand.compare(L"$dialog") == 0)
    {
        mu::ui::window::CreateOkMessageBox(L"Test dialog: drag it by any part that is not a button.");
        return Handled();
    }
    if (strCommand.compare(L"$dialog title") == 0)
    {
        mu::ui::window::CreateOkMessageBoxWithTitle(L"Test dialog", L"Drag it by any part that is not a button.");
        return Handled();
    }
    // "$dialog menu <name>": an NPC's menu without the NPC, to check its layout. Its buttons act.
    if (strCommand.compare(0, 13, L"$dialog menu ") == 0)
    {
        using namespace mu::ui::window;
        const std::pair<const wchar_t*, void (*)()> menus[] = {
            {L"system", ShowSystemMenuDialog},
            {L"chaosmix", ShowChaosMixMenuDialog},
            {L"cherry", ShowCherryBlossomMenuDialog},
            {L"gem", ShowGemIntegrationMenuDialog},
            {L"luckytrade", ShowLuckyTradeMenuDialog},
            {L"trainer", ShowTrainerMenuDialog},
            {L"trainerrecover", ShowTrainerRecoverDialog},
            {L"elpis", [] { ShowElpisMenuDialog(); }},
            {L"elpisabout", [] { ShowElpisMenuDialog(MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_REFINARY); }},
            {L"seedmaster", ShowSeedMasterMenuDialog},
            {L"seedinvestigator", ShowSeedInvestigatorMenuDialog},
            {L"resetpoints", ShowResetCharacterPointDialog},
            {L"delgardo", ShowDelgardoMainMenuDialog},
        };
        const std::wstring name = strCommand.substr(13);
        for (const auto& [menuName, show] : menus)
        {
            if (name == menuName)
            {
                show();
                return Handled();
            }
        }
        std::wstring names;
        for (const auto& menu : menus)
            names += std::wstring(L" ") + menu.first;
        g_pSystemLogBox->AddText((L"$dialog menu <name>:" + names).c_str(), mu::ui::window::TYPE_SYSTEM_MESSAGE);
        return Handled();
    }
    // "$dialog item [group index [level]]": a test confirmation with a live item preview.
    if (strCommand.compare(0, 12, L"$dialog item") == 0 && mu::ui::window::g_pGenericConfirmDialog)
    {
        int group = ITEM_GROUP_SWORD, index = 5, level = 9;
        if (strCommand.size() > 13)
            swscanf(strCommand.c_str() + 13, L"%d %d %d", &group, &index, &level);
        mu::ui::window::GenericDialogConfig cfg;
        ITEM item{};
        item.Type = static_cast<short>(group * MAX_ITEM_INDEX + index);
        item.Level = level;
        cfg.item3D = item;
        cfg.showCancel = true;
        cfg.lines = {{L"Test dialog with a live item preview.", false}};
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
        return Handled();
    }

#if defined(MU_ENABLE_CONTROL_SOCKET) || defined(_EDITOR)
    if (CheckDeveloperLookCommand(strCommand))
    {
        return Handled();
    }
#endif
    if (strCommand.compare(L"$fpscounter on") == 0)
    {
        SetShowFpsCounter(true);
        return Handled();
    }
    else if (strCommand.compare(L"$fpscounter off") == 0)
    {
        SetShowFpsCounter(false);
        return Handled();
    }
    else if (strCommand.compare(L"$details on") == 0)
    {
        SetShowDebugInfo(true);
        return Handled();
    }
    else if (strCommand.compare(L"$details off") == 0)
    {
        SetShowDebugInfo(false);
        return Handled();
    }
    else if (strCommand.compare(L"$glstats on") == 0)
    {
        SetShowGLStats(true);
        return Handled();
    }
    else if (strCommand.compare(L"$glstats off") == 0)
    {
        SetShowGLStats(false);
        return Handled();
    }
    else if (const CommandForm form = FormOfCommand(strCommand, FpsCommand); form != CommandForm::NotThisCommand)
    {
        const std::optional<std::wstring_view> argument = CommandArgument(form, strCommand, FpsCommand);
        const std::optional<double> targetFps = argument ? ParseTargetFps(*argument) : std::nullopt;
        if (!targetFps)
        {
            return BadArgument(TargetFpsUsage());
        }

        SetTargetFps(*targetFps);
        return Handled();
    }
    else if (strCommand.compare(L"$vsync on") == 0)
    {
        MuSetVSyncPreference(true);
        return Handled();
    }
    else if (strCommand.compare(L"$vsync off") == 0)
    {
        MuSetVSyncPreference(false);
        return Handled();
    }
    else if (Render::Look::HandleLookPresetCommand(strCommand))
    {
        return Handled();
    }
    else if (strCommand.compare(0, 6, L"$msaa ") == 0)
    {
        const auto samples = Render::Antialiasing::ParseCommandArgument(std::wstring_view(strCommand).substr(6));
        if (samples)
        {
            MuSetAntialiasingPreference(*samples);
        }
        else
        {
            mu::log::Get("render")->warn("$msaa expects off, 2, 4 or 8");
        }
        return Handled();
    }
    else if (strCommand.compare(L"$effects off") == 0)
    {
        // Force-disable both effect surfaces to test whether effects
        // overdraw/volume contributes to the GPU-stall (Present) cost or the HUD blink -- owner
        // observed the blink timing tracking nearby enemies' skill casts (beam knights/Tantalos).
        // SetDisableEffects() covers RenderEffectShadows/RenderBoids/RenderEffects/RenderBlurs
        // (includes g_SkillEffects' 3D effect objects). g_pOption->SetRenderAllEffects(false) is the
        // pre-existing, more comprehensive switch already checked by RenderSprites/RenderParticles
        // (which CreateSprite/CreateParticle -- called directly by SkillCast.cpp and pet-action
        // code -- feed) plus ZzzEffectPoint/ZzzEffectFireLeave/GOBoid. Together these cover
        // effectively every effect-rendering path in the tree.
        SetDisableEffects(true);
        if (g_pOption)
            g_pOption->SetRenderAllEffects(false);
        return Handled();
    }
    else if (strCommand.compare(L"$effects on") == 0)
    {
        SetDisableEffects(false);
        if (g_pOption)
            g_pOption->SetRenderAllEffects(true);
        return Handled();
    }
    // Finer-grained bisection of the effect-rendering GPU cost `$effects off`
    // already confirmed. Each isolates one of the distinct object systems that can be populated
    // independently of the others (see MainScene.h doc comments for what each one covers).
    else if (strCommand.compare(L"$effects sprites off") == 0)
    {
        SetDisableSprites(true);
        return Handled();
    }
    else if (strCommand.compare(L"$effects sprites on") == 0)
    {
        SetDisableSprites(false);
        return Handled();
    }
    else if (strCommand.compare(L"$effects particles off") == 0)
    {
        SetDisableParticles(true);
        return Handled();
    }
    else if (strCommand.compare(L"$effects particles on") == 0)
    {
        SetDisableParticles(false);
        return Handled();
    }
    else if (strCommand.compare(L"$effects skillmodels off") == 0)
    {
        SetDisableSkillEffectModels(true);
        return Handled();
    }
    else if (strCommand.compare(L"$effects skillmodels on") == 0)
    {
        SetDisableSkillEffectModels(false);
        return Handled();
    }
    else if (strCommand.compare(L"$effects boids off") == 0)
    {
        SetDisableBoids(true);
        return Handled();
    }
    else if (strCommand.compare(L"$effects boids on") == 0)
    {
        SetDisableBoids(false);
        return Handled();
    }
    // Owner found unequipping Wings of Ruin alone took FPS 120->180. This skips
    // just the EXTRA b->RenderBodyShadow() call RenderLinkObject() (ZzzCharacter.cpp) makes for every
    // visible wing/cape-wearing character, while leaving the wing model itself equipped and visible --
    // isolates whether the shadow draw specifically is the cost, keep the wing on for this test.
    else if (strCommand.compare(L"$effects wingshadow off") == 0)
    {
        SetDisableWingShadow(true);
        return Handled();
    }
    else if (strCommand.compare(L"$effects wingshadow on") == 0)
    {
        SetDisableWingShadow(false);
        return Handled();
    }
    else if (strCommand.compare(L"$effects shadows off") == 0)
    {
        SetDisableBodyShadows(true);
        return Handled();
    }
    else if (strCommand.compare(L"$effects shadows on") == 0)
    {
        SetDisableBodyShadows(false);
        return Handled();
    }
    // RenderJoints() (beam/tail-trail effects -- Wing of Ruin's growing tail
    // light, Beam Knight's lightning beam) was never gated by g_pOption->GetRenderAllEffects() at
    // all, so $effects off never covered it. Isolates it directly.
    else if (strCommand.compare(L"$effects joints off") == 0)
    {
        SetDisableJoints(true);
        return Handled();
    }
    else if (strCommand.compare(L"$effects joints on") == 0)
    {
        SetDisableJoints(false);
        return Handled();
    }
    else if (strCommand.compare(L"$effects culling off") == 0)
    {
        SetDisableEffectCulling(true);
        return Handled();
    }
    else if (strCommand.compare(L"$effects culling on") == 0)
    {
        SetDisableEffectCulling(false);
        return Handled();
    }
    // MODEL_WING_OF_RUIN draws its mesh 3x per frame (base + 2 glow-layer passes,
    // ZzzObject.cpp ~7001-7007). Skips the 2 extra passes to test if triple mesh-draw is the cost.
    // Visibly changes the wing's look (removes glow layers) while active -- measurement tool only.
    else if (strCommand.compare(L"$effects wingextralayers off") == 0)
    {
        SetDisableWingExtraLayers(true);
        return Handled();
    }
    else if (strCommand.compare(L"$effects wingextralayers on") == 0)
    {
        SetDisableWingExtraLayers(false);
        return Handled();
    }
    else if (const CommandForm form = FormOfCommand(strCommand, WinMsgCommand); form != CommandForm::NotThisCommand)
    {
        const std::optional<std::wstring_view> argument = CommandArgument(form, strCommand, WinMsgCommand);
        const std::optional<int> messageLimit = argument ? Core::Text::TryParseInt(*argument) : std::nullopt;
        if (!messageLimit || *messageLimit == 0)
        {
            return BadArgument(L"usage: $winmsg <count>, 1 or more, or -1 for unlimited");
        }

        SetMaxMessagePerCycle(*messageLimit);
        return Handled();
    }
    // Runtime theme selection is session-only; the Options window persists its choice.
    else if (strCommand.compare(0, 6, L"$theme") == 0)
    {
        if (strCommand.size() > 7)
        {
            const std::wstring themeNameW = strCommand.substr(7);
            const std::string themeName = StringUtils::WideToNarrow(themeNameW.c_str());
            UI::Theme::Select(themeName, UI::Theme::Persistence::Session);
        }
        return Handled();
    }

#ifdef CSK_LH_DEBUG_CONSOLE
    if (!m_bInit)
        return NotHandled();

    if (strCommand.compare(L"$open") == NULL)
    {
        leaf::ShowConsole(true);
        return Handled();
    }
    else if (strCommand.compare(L"$close") == NULL)
    {
        leaf::ShowConsole(false);
        return Handled();
    }
    else if (strCommand.compare(L"$clear") == NULL)
    {
        leaf::SetConsoleTextColor();
        leaf::ClearConsoleScreen();
        return Handled();
    }
#ifdef CSK_DEBUG_MAP_ATTRIBUTE
    else if (strCommand.compare(L"$mapatt on") == NULL)
    {
        EditFlag = EDIT_WALL;
        return Handled();
    }
    else if (strCommand.compare(L"$mapatt off") == NULL)
    {
        EditFlag = EDIT_NONE;
        return Handled();
    }
#endif // CSK_DEBUG_MAP_ATTRIBUTE
#ifdef CSK_DEBUG_MAP_PATHFINDING
    else if (strCommand.compare(L"$path on") == NULL)
    {
        g_bShowPath = true;
    }
    else if (strCommand.compare(L"$path off") == NULL)
    {
        g_bShowPath = false;
    }
#endif // CSK_DEBUG_MAP_PATHFINDING
#ifdef CSK_DEBUG_RENDER_BOUNDINGBOX
    else if (strCommand.compare(L"$bb on") == NULL)
    {
        g_bRenderBoundingBox = true;
    }
    else if (strCommand.compare(L"$bb off") == NULL)
    {
        g_bRenderBoundingBox = false;
    }
#endif // CSK_DEBUG_RENDER_BOUNDINGBOX
    else if (strCommand.compare(L"$type_test") == NULL)
    {
        Write(MCD_SEND, L"MCD_SEND");
        Write(MCD_RECEIVE, L"MCD_RECEIVE");
        Write(MCD_ERROR, L"MCD_ERROR");
        Write(MCD_NORMAL, L"MCD_NORMAL");
        return Handled();
    }
    else if (strCommand.compare(L"$texture_info") == NULL)
    {
        Write(MCD_NORMAL, L"Texture Number : %d", Bitmaps.GetNumberOfTexture());
        Write(MCD_NORMAL, L"Texture Memory : %dKB", Bitmaps.GetUsedTextureMemory() / 1024);
        return Handled();
    }
    else if (strCommand.compare(L"$color_test") == NULL)
    {
        leaf::SetConsoleTextColor(leaf::COLOR_DARKRED);
        std::cout << "color test: dark red" << std::endl;
        leaf::SetConsoleTextColor(leaf::COLOR_DARKGREEN);
        std::cout << "color test: dark green" << std::endl;
        leaf::SetConsoleTextColor(leaf::COLOR_DARKBLUE);
        std::cout << "color test: dark blue" << std::endl;
        leaf::SetConsoleTextColor(leaf::COLOR_RED);
        std::cout << "color test: red" << std::endl;
        leaf::SetConsoleTextColor(leaf::COLOR_GREEN);
        std::cout << "color test: green" << std::endl;
        leaf::SetConsoleTextColor(leaf::COLOR_BLUE);
        std::cout << "color test: blue" << std::endl;
        leaf::SetConsoleTextColor(leaf::COLOR_OLIVE);
        std::cout << "color test: olive" << std::endl;
        leaf::SetConsoleTextColor(leaf::COLOR_PURPLE);
        std::cout << "color test: purple" << std::endl;
        leaf::SetConsoleTextColor(leaf::COLOR_TEAL);
        std::cout << "color test: teal" << std::endl;
        leaf::SetConsoleTextColor(leaf::COLOR_GRAY);
        std::cout << "color test: gray" << std::endl;
        leaf::SetConsoleTextColor(leaf::COLOR_AQUA);
        std::cout << "color test: aqua" << std::endl;
        leaf::SetConsoleTextColor(leaf::COLOR_FUCHSIA);
        std::cout << "color test: fuchsia" << std::endl;
        leaf::SetConsoleTextColor(leaf::COLOR_YELLOW);
        std::cout << "color test: yellow" << std::endl;
        leaf::SetConsoleTextColor(leaf::COLOR_WHITE);
        std::cout << "color test: white" << std::endl;
        return Handled();
    }
#endif
    return NotHandled();
}

void CmuConsoleDebug::Write(int iType, const wchar_t* pStr, ...)
{
    static const bool networkDiagnosticsEnabled = std::getenv("MU_NETWORK_DIAGNOSTICS") != nullptr;
    if ((iType == MCD_SEND || iType == MCD_RECEIVE) && !networkDiagnosticsEnabled)
        return;

    wchar_t szBuffer[256] = L"";
    va_list arguments;
    va_start(arguments, pStr);
    _vsnwprintf(szBuffer, std::size(szBuffer), pStr, arguments);
    va_end(arguments);

    char utf8Buffer[1024] = { 0 };
    WideCharToMultiByte(CP_UTF8, 0, szBuffer, -1, utf8Buffer, sizeof(utf8Buffer), nullptr, nullptr);
    const auto logger = mu::log::Get("core");
    if (iType == MCD_ERROR)
        MU_LOG_ERROR(logger, "{}", utf8Buffer);
    else
        MU_LOG_DEBUG(logger, "{}", utf8Buffer);

#ifdef CONSOLE_DEBUG
    if (m_bInit)
    {
        switch (iType)
        {
        case MCD_SEND:
            leaf::SetConsoleTextColor(leaf::COLOR_OLIVE);
            break;
        case MCD_RECEIVE:
            leaf::SetConsoleTextColor(leaf::COLOR_DARKGREEN);
            break;
        case MCD_ERROR:
            leaf::SetConsoleTextColor(leaf::COLOR_WHITE, leaf::COLOR_DARKRED);
            break;
        case MCD_NORMAL:
            leaf::SetConsoleTextColor(leaf::COLOR_GRAY);
            break;
        }

        std::wcout << szBuffer << std::endl;

#ifdef _EDITOR
        // Also log to ImGui console
        g_MuEditorConsoleUI.LogGame(utf8Buffer);
#endif
    }
#endif
}
