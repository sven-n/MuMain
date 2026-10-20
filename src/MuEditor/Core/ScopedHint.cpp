#include "stdafx.h"

#ifdef _EDITOR

#include "ScopedHint.h"

#include <SDL3/SDL_hints.h>

namespace MuEditor
{
ScopedHint::ScopedHint(const char* name, const char* value) : m_name(name)
{
    if (const char* previous = SDL_GetHint(name))
    {
        m_previous = previous;
        m_hadValue = true;
    }
    SDL_SetHint(name, value);
}

ScopedHint::~ScopedHint()
{
    if (m_hadValue)
    {
        SDL_SetHint(m_name, m_previous.c_str());
    }
    else
    {
        SDL_ResetHint(m_name);
    }
}
} // namespace MuEditor

#endif // _EDITOR
