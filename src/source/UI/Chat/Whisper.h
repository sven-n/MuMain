#pragma once

// Whisper level gate: below the required character level a player may only
// whisper IDs that whispered them first. These track that allow-list.
// Extracted from ZzzInterface.cpp.
namespace UI::Chat::Whisper
{
    // True if the player may whisper this target (high enough level, or the
    // target is on the allow-list); otherwise posts a system message and fails.
    bool CheckLevel(int requiredLevel, wchar_t* targetId);

    // Add a target to the allow-list (used when someone whispers the player).
    void Register(int requiredLevel, wchar_t* targetId);

    // Clear the allow-list.
    void Clear();

    // Debug overlay: list the registered allow-list IDs.
    void RenderList();
}
