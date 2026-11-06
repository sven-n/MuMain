#pragma once

#ifdef _EDITOR

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace MuEditor::Items
{
// The .bmd files an item model can open: those below Data/Item and
// Data/Player of the client folder, as the model files write them
// ("Data/Item/Sword01.bmd"), sorted. Read once, again on Refresh.
class ItemModelFiles
{
public:
    // The folder the paths are relative to; the working directory in the
    // game.
    explicit ItemModelFiles(std::filesystem::path clientFolder = {});

    const std::vector<std::string>& GetFiles();
    void Refresh();

    // The files whose path contains the search (letters A to Z in either case).
    std::vector<std::string_view> Find(std::string_view search);

    // The folder below Data/ that holds the file, the first texture folder a
    // new model takes: "Data/Item/partCharge1/Ring01.bmd" -> "Item/partCharge1".
    static std::string GetTextureFolder(std::string_view file);
    // The texture folders of a model whose file changes from `oldFile` to
    // `newFile`: the new file's folder when it had none or only the old
    // file's; otherwise the folders stay (they were chosen).
    static std::vector<std::string> GetTextureFoldersForFile(const std::vector<std::string>& folders,
                                                             std::string_view oldFile, std::string_view newFile);

private:
    std::filesystem::path m_clientFolder;
    std::vector<std::string> m_files;
    bool m_read = false;
};
} // namespace MuEditor::Items

#endif // _EDITOR
