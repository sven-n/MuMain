// The control socket's command implementations, one file per family:
// Session (login/select-char/logout/quit), World (movement, combat, chat,
// party) and Query (state/nearby/scene/screenshot/hotkey/click-ui).
//
// Every handler answers with an encoded response line, or takes over the
// dispatcher's single act slot for a command that needs several frames.
#pragma once

#include "App/Control/ControlDispatcher.h"

#include <memory>
#include <string>
#include <string_view>

namespace App::Control::Commands
{
// Scene name as the protocol reports it: server_list, webzen, login,
// loading, character_list, world or unknown.
[[nodiscard]] std::string_view CurrentSceneName();

// Build identifier reported by `ping`, set once at start-up.
void SetBuildIdentifier(std::string identifier);
[[nodiscard]] const std::string& BuildIdentifier();

// Query family.
std::string Ping(const Request& request, std::unique_ptr<Act>& act);
std::string Scene(const Request& request, std::unique_ptr<Act>& act);
std::string State(const Request& request, std::unique_ptr<Act>& act);
std::string Nearby(const Request& request, std::unique_ptr<Act>& act);
std::string EventsSince(const Request& request, std::unique_ptr<Act>& act);
std::string WaitFor(const Request& request, std::unique_ptr<Act>& act);
std::string Screenshot(const Request& request, std::unique_ptr<Act>& act);
std::string Hotkey(const Request& request, std::unique_ptr<Act>& act);
std::string ClickUi(const Request& request, std::unique_ptr<Act>& act);
std::string Type(const Request& request, std::unique_ptr<Act>& act);

// Session family.
std::string Login(const Request& request, std::unique_ptr<Act>& act);
std::string SelectCharacter(const Request& request, std::unique_ptr<Act>& act);
std::string Logout(const Request& request, std::unique_ptr<Act>& act);
std::string Quit(const Request& request, std::unique_ptr<Act>& act);

// World family.
std::string Move(const Request& request, std::unique_ptr<Act>& act);
std::string Warp(const Request& request, std::unique_ptr<Act>& act);
std::string Teleport(const Request& request, std::unique_ptr<Act>& act);
std::string Attack(const Request& request, std::unique_ptr<Act>& act);
std::string Skill(const Request& request, std::unique_ptr<Act>& act);
std::string Pickup(const Request& request, std::unique_ptr<Act>& act);
std::string UseItem(const Request& request, std::unique_ptr<Act>& act);
std::string EquipItem(const Request& request, std::unique_ptr<Act>& act);
std::string Say(const Request& request, std::unique_ptr<Act>& act);
std::string Whisper(const Request& request, std::unique_ptr<Act>& act);
std::string Party(const Request& request, std::unique_ptr<Act>& act);
std::string Halt(const Request& request, std::unique_ptr<Act>& act);
} // namespace App::Control::Commands
