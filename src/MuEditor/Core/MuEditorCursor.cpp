#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorCursor.h"

#include "imgui.h"
#include <SDL3/SDL_mouse.h>

namespace
{
SDL_Cursor* g_cursors[ImGuiMouseCursor_COUNT] = {};

// As the ImGui SDL3 backend maps them.
SDL_SystemCursor SystemCursorFor(ImGuiMouseCursor shape)
{
    switch (shape)
    {
    case ImGuiMouseCursor_TextInput:
        return SDL_SYSTEM_CURSOR_TEXT;
    case ImGuiMouseCursor_ResizeAll:
        return SDL_SYSTEM_CURSOR_MOVE;
    case ImGuiMouseCursor_ResizeNS:
        return SDL_SYSTEM_CURSOR_NS_RESIZE;
    case ImGuiMouseCursor_ResizeEW:
        return SDL_SYSTEM_CURSOR_EW_RESIZE;
    case ImGuiMouseCursor_ResizeNESW:
        return SDL_SYSTEM_CURSOR_NESW_RESIZE;
    case ImGuiMouseCursor_ResizeNWSE:
        return SDL_SYSTEM_CURSOR_NWSE_RESIZE;
    case ImGuiMouseCursor_Hand:
        return SDL_SYSTEM_CURSOR_POINTER;
    case ImGuiMouseCursor_Wait:
        return SDL_SYSTEM_CURSOR_WAIT;
    case ImGuiMouseCursor_Progress:
        return SDL_SYSTEM_CURSOR_PROGRESS;
    case ImGuiMouseCursor_NotAllowed:
        return SDL_SYSTEM_CURSOR_NOT_ALLOWED;
    default:
        return SDL_SYSTEM_CURSOR_DEFAULT;
    }
}

SDL_Cursor* CursorFor(ImGuiMouseCursor shape)
{
    if (shape < 0 || shape >= ImGuiMouseCursor_COUNT)
    {
        shape = ImGuiMouseCursor_Arrow;
    }
    if (g_cursors[shape] == nullptr)
    {
        g_cursors[shape] = SDL_CreateSystemCursor(SystemCursorFor(shape));
    }
    return g_cursors[shape];
}
} // namespace

namespace MuEditor::Cursor
{
void ShowImGuiShape()
{
#ifndef _WIN32
    const ImGuiMouseCursor shape = ImGui::GetMouseCursor();
    if (shape == ImGuiMouseCursor_None)
    {
        SDL_HideCursor();
        return;
    }
    // Compared with SDL's current cursor, as the game also sets its own (a
    // window's mouse enter).
    SDL_Cursor* cursor = CursorFor(shape);
    if (cursor != nullptr && SDL_GetCursor() != cursor)
    {
        SDL_SetCursor(cursor);
    }
    if (!SDL_CursorVisible())
    {
        SDL_ShowCursor();
    }
#endif
}

void Release()
{
    for (SDL_Cursor*& cursor : g_cursors)
    {
        if (cursor != nullptr)
        {
            SDL_DestroyCursor(cursor);
            cursor = nullptr;
        }
    }
}
} // namespace MuEditor::Cursor

#endif // _EDITOR
