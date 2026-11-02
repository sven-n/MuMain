#include "stdafx.h"

#ifdef _EDITOR

#include "SoundFilePicker.h"

#include "SoundEditorRows.h"

#include "Audio/DSPlaySound.h"
#include "Audio/SoundNumbers.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iterator>
#include <optional>
#include <system_error>

namespace MuEditor::SoundEditor
{
namespace
{
std::string Lowered(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

bool IsSoundFile(const std::filesystem::path& path)
{
    const std::string extension = Lowered(path.extension().string());
    return extension == ".wav" || extension == ".ogg" || extension == ".mp3";
}

std::optional<std::vector<std::string>>& Cached()
{
    static std::optional<std::vector<std::string>> files;
    return files;
}
} // namespace

std::vector<std::string> ListSoundFiles(const std::string& folder)
{
    std::vector<std::string> files;
    std::error_code error;
    const std::filesystem::path root = std::filesystem::u8path(folder);
    for (std::filesystem::recursive_directory_iterator entry(root, error), end; !error && entry != end;
         entry.increment(error))
    {
        if (!entry->is_regular_file(error) || !IsSoundFile(entry->path()))
        {
            continue;
        }
        const std::u8string name = entry->path().lexically_relative(root).generic_u8string();
        const std::string relative(name.begin(), name.end());
        files.push_back(folder + "/" + relative);
    }
    std::sort(files.begin(), files.end(),
              [](const std::string& left, const std::string& right) { return Lowered(left) < Lowered(right); });
    return files;
}

const std::vector<std::string>& ClientSoundFiles()
{
    if (!Cached())
    {
        Cached() = ListSoundFiles("Data/Sound");
    }
    return *Cached();
}

void RefreshClientSoundFiles()
{
    Cached().reset();
}

std::vector<std::string> FilterFiles(const std::vector<std::string>& files, std::string_view text)
{
    if (text.empty())
    {
        return files;
    }
    const std::string wanted = Lowered(std::string(text));
    std::vector<std::string> shown;
    std::copy_if(files.begin(), files.end(), std::back_inserter(shown),
                 [&wanted](const std::string& file) { return Lowered(file).find(wanted) != std::string::npos; });
    return shown;
}

void PreviewFile(const std::string& file, int channels, bool spatial)
{
    const std::wstring path = ToLoaderPath(file);
    const ESound number = Audio::SoundNumbers::ForFile(path);
    if (number == Audio::SoundNumbers::NoNumber)
    {
        return;
    }
    if (!IsSoundLoaded(number, path.c_str(), channels, spatial))
    {
        LoadWaveFile(number, path.c_str(), channels, spatial);
    }
    PlayBufferAtLevel(number, 1.0f);
}
} // namespace MuEditor::SoundEditor

#endif // _EDITOR
