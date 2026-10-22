#include "stdafx.h"

#ifdef _EDITOR

#include "SourceText.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <system_error>

namespace MuEditor::SourceText
{
bool IsIdentifierChar(char c)
{
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

std::string Trimmed(std::string_view text)
{
    const size_t first = text.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos)
        return {};
    const size_t last = text.find_last_not_of(" \t\r\n");
    return std::string(text.substr(first, last - first + 1));
}

std::string Blank(std::string_view source)
{
    std::string text(source);
    for (size_t i = 0; i < text.size();)
    {
        const char c = text[i];
        const char next = i + 1 < text.size() ? text[i + 1] : '\0';
        if (c == '/' && next == '/')
        {
            const size_t end = text.find('\n', i);
            std::fill(text.begin() + static_cast<std::ptrdiff_t>(i),
                      end == std::string::npos ? text.end() : text.begin() + static_cast<std::ptrdiff_t>(end), ' ');
            i = end == std::string::npos ? text.size() : end;
        }
        else if (c == '/' && next == '*')
        {
            const size_t close = text.find("*/", i + 2);
            const size_t end = close == std::string::npos ? text.size() : close + 2;
            for (size_t k = i; k < end; ++k)
            {
                if (text[k] != '\n')
                    text[k] = ' ';
            }
            i = end;
        }
        else if (c == '"' || c == '\'')
        {
            size_t k = i + 1;
            while (k < text.size() && text[k] != c && text[k] != '\n')
                k += text[k] == '\\' ? 2 : 1;
            for (size_t j = i + 1; j < k && j < text.size(); ++j)
                text[j] = ' ';
            i = k + 1;
        }
        else
        {
            ++i;
        }
    }
    return text;
}

std::string ReadFile(const std::filesystem::path& path)
{
    std::ifstream stream(path, std::ios::binary);
    std::ostringstream text;
    text << stream.rdbuf();
    return text.str();
}

std::vector<SourceFile> FindSourceFiles(const std::filesystem::path& folder)
{
    std::vector<SourceFile> files;
    std::error_code error;
    for (std::filesystem::recursive_directory_iterator entry(folder, error), end; !error && entry != end;
         entry.increment(error))
    {
        const std::filesystem::path extension = entry->path().extension();
        std::error_code entryError;
        if ((extension != ".cpp" && extension != ".h") || !entry->is_regular_file(entryError))
            continue;
        files.push_back({entry->path(), entry->path().lexically_relative(folder).generic_string()});
    }
    std::sort(files.begin(), files.end(),
              [](const SourceFile& left, const SourceFile& right) { return left.relative < right.relative; });
    return files;
}
} // namespace MuEditor::SourceText

#endif // _EDITOR
