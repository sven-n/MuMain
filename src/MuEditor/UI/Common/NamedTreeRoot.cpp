#include "stdafx.h"

#ifdef _EDITOR

#include "NamedTreeRoot.h"

namespace MuEditor
{
void NamedTreeRoot::RenderSwatch(const std::string&) const {}

std::vector<std::string> GetNames(const NamedTreeRoot& root)
{
    std::vector<std::string> names;
    names.reserve(root.GetCount());
    for (std::size_t index = 0; index < root.GetCount(); ++index)
    {
        names.push_back(root.GetName(index));
    }
    return names;
}
} // namespace MuEditor

#endif // _EDITOR
