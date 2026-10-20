#pragma once

#ifdef _EDITOR

#include <string>

namespace MuEditor
{
// Sets an SDL hint for its lifetime and puts the earlier value back.
class ScopedHint
{
public:
    ScopedHint(const char* name, const char* value);
    ~ScopedHint();
    ScopedHint(const ScopedHint&) = delete;
    ScopedHint& operator=(const ScopedHint&) = delete;

private:
    const char* m_name;
    std::string m_previous;
    bool m_hadValue = false;
};
} // namespace MuEditor

#endif // _EDITOR
