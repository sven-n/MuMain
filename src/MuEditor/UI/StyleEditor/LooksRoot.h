#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/Common/NamedTreeRoot.h"
#include "../MuEditor/UI/ItemTools/ItemLookUseCounts.h"
#include "Data/GameData/LookData/LookDefinition.h"

#include <functional>
#include <string>
#include <vector>

namespace MuEditor::Looks
{
// A look's row in the Style Editor's tree: "chromeMesh0 (12)", with " (code)" for one still in code.
std::string LookRowLabel(const std::string& name, int users, bool code);

// A root of the Style Editor's tree (design L31): the render styles, or the item effects. Its looks are those of the
// look file of its kind (Data/Effects/RenderStyles.json or ItemEffects.json, LookEdits::Of) and the ones still in code,
// sorted by name. Only looks in the file go into its folders; one in code stays under the root, marked "(code)".
class LooksRoot final : public NamedTreeRoot
{
public:
    explicit LooksRoot(Data::Looks::LookKind kind) : m_kind(kind) {}

    [[nodiscard]] Data::Looks::LookKind GetKind() const
    {
        return m_kind;
    }
    const char* GetRootName() const override;
    int GetVersion() const override;
    std::size_t GetCount() const override;
    const std::string& GetName(std::size_t index) const override;
    bool Has(const std::string& name) const override;
    // "chromeMesh0 (12)", "characterCard (4) (code)", "knightBlade (3) *" with changes not applied.
    std::string GetRowLabel(const std::string& name) override;
    const std::vector<Data::Colors::ColorFolder>& GetFolders() const override;
    void SetFolders(const char* step, std::vector<Data::Colors::ColorFolder> folders) override;
    // Not after a read with a fatal problem: Save is off then.
    bool HasFolders() const override;
    bool CanReorder() const override
    {
        return false;
    }
    void SetOrder(const char* step, std::span<const std::string> names) override;
    bool CanMoveEntry(const std::string& name) const override;

    // Whether the look is still code (Render/Items/ItemRenderStyles.cpp, ItemEffects.cpp): no look of the file has its
    // name.
    [[nodiscard]] bool IsCode(const std::string& name) const;
    int CountUsers(const std::string& name);
    // Whether a look has changes the Style Editor has not applied yet; its row is marked "*".
    void SetUnappliedCheck(std::function<bool(const std::string&)> check)
    {
        m_unapplied = std::move(check);
    }

private:
    void Update() const;

    Data::Looks::LookKind m_kind;
    mutable std::vector<std::string> m_names;
    mutable int m_version = -1;
    Items::ItemLookUseCounts m_counts;
    std::function<bool(const std::string&)> m_unapplied;
};
} // namespace MuEditor::Looks

#endif // _EDITOR
