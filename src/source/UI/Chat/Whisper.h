#pragma once

// Whisper level gate: below the required level, a player may only whisper IDs
// that whispered them first; this tracks that allow-list.
namespace UI::Chat::Whisper
{
    // True if allowed to whisper (high enough level, or allow-listed); otherwise posts a system message and returns false.
    bool CheckLevel(int requiredLevel, wchar_t* targetId);

    // Add a target to the allow-list (used when someone whispers the player).
    void Register(int requiredLevel, wchar_t* targetId);

    // Clear the allow-list.
    void Clear();

    // Debug overlay: list the registered allow-list IDs.
    void RenderList();
}
