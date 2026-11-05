#pragma once

#ifdef _EDITOR

#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace MuEditor::Monsters
{
// The .bmd files a monster or NPC model can open: those below Data/Monster
// and Data/NPC (with their subfolders) and in each other folder that a file
// of the model list is in (Object35, Item/xmas, the part models' folders),
// as the model list writes them ("Data/Monster/Monster01.bmd"), in their
// spelling on disk, sorted. Read once, again on Refresh.
class MonsterModelFiles
{
public:
    // The folder the paths are relative to; the working directory in the
    // game.
    explicit MonsterModelFiles(std::filesystem::path clientFolder = {});

    // `listFiles`: the model and part files of the model list, whose folders
    // are searched too.
    const std::vector<std::string>& GetFiles(std::span<const std::string> listFiles);
    void Refresh();

    // The files whose path contains the search, ignoring case.
    std::vector<std::string_view> Find(std::span<const std::string> listFiles, std::string_view search);

private:
    std::filesystem::path m_clientFolder;
    std::vector<std::string> m_files;
    bool m_read = false;
};
} // namespace MuEditor::Monsters

#endif // _EDITOR
