#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/Common/NamedTreeRoot.h"
#include "../MuEditor/UI/ItemTools/ItemLookUseCounts.h"

#include <string>
#include <vector>

namespace MuEditor::Looks
{
// The root of the item effects: all are still code (Render/Items/ItemEffects.cpp) until their file comes with them, so
// the root has no folders.
class ItemEffectsRoot final : public NamedTreeRoot
{
public:
    const char* GetRootName() const override;
    int GetVersion() const override
    {
        return 0;
    }
    std::size_t GetCount() const override;
    const std::string& GetName(std::size_t index) const override;
    bool Has(const std::string& name) const override;
    std::string GetRowLabel(const std::string& name) override;
    const std::vector<Data::Colors::ColorFolder>& GetFolders() const override;
    void SetFolders(const char* step, std::vector<Data::Colors::ColorFolder> folders) override;
    bool HasFolders() const override
    {
        return false;
    }
    bool CanReorder() const override
    {
        return false;
    }
    void SetOrder(const char* step, std::span<const std::string> names) override;
    bool CanMoveEntry(const std::string& name) const override;
    int CountUsers(const std::string& name);

private:
    const std::vector<std::string>& Names() const;

    Items::ItemLookUseCounts m_counts;
};
} // namespace MuEditor::Looks

#endif // _EDITOR
