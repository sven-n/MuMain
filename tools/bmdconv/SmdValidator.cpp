#include "stdafx.h"

#include "SmdValidator.h"

#include "Render/Models/ZzzBMD.h"
#include "Render/Models/SMD.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <map>
#include <set>
#include <sstream>

namespace Tools::BmdConv
{
namespace
{
// Limits taken from the engine headers so the messages name the buffer that would overflow.
constexpr int kMaxNodes = std::min(NODE_MAX, MAX_BONES); // bones per skeleton (SMD.h / ZzzBMD.h)
constexpr int kMaxFrames = TIME_MAX;                     // SkeletonGroup_t::Skeleton[TIME_MAX]
constexpr int kMaxTriangles = TRIANGLE_MAX;              // TriangleGroup_t holds every triangle
constexpr int kMaxMeshes = MAX_MESH;                     // one mesh per distinct material
constexpr int kMaxUniquePerMesh = VERTEX_MAX;            // unique positions/normals/uvs per mesh
constexpr size_t kMaxNameLength = 31;                    // char[32] bone and texture names
constexpr int kVertexTokenCount = 9;                     // bone x y z nx ny nz u v
constexpr int kNodeTokenCount = 3;                       // id "name" parent
constexpr int kSkeletonTokenCount = 7;                   // id x y z rx ry rz
constexpr int kSupportedVersion = 1;

struct Token
{
    std::string text;
    bool quoted = false;
};

struct Line
{
    int number = 0;
    std::vector<Token> tokens;
};

struct MaterialUsage
{
    std::set<std::string> positions;
    std::set<std::string> normals;
    std::set<std::string> uvs;
    int triangles = 0;
};

std::string Describe(int lineNumber, const std::string& message)
{
    return "line " + std::to_string(lineNumber) + ": " + message;
}

// Splits one line the way the engine tokenizer does: whitespace separated, double quotes
// group a name, and "//" starts a comment.
std::vector<Token> Tokenize(const std::string& line)
{
    std::vector<Token> tokens;
    size_t index = 0;
    while (index < line.size())
    {
        const char ch = line[index];
        if (std::isspace(static_cast<unsigned char>(ch)))
        {
            ++index;
            continue;
        }
        if (ch == '/' && index + 1 < line.size() && line[index + 1] == '/')
        {
            break;
        }
        if (ch == '"')
        {
            const size_t close = line.find('"', index + 1);
            const size_t end = close == std::string::npos ? line.size() : close;
            tokens.push_back({line.substr(index + 1, end - index - 1), true});
            index = end + 1;
            continue;
        }
        size_t end = index;
        while (end < line.size() && !std::isspace(static_cast<unsigned char>(line[end])))
        {
            ++end;
        }
        tokens.push_back({line.substr(index, end - index), false});
        index = end;
    }
    return tokens;
}

std::vector<Line> ReadLines(const std::filesystem::path& file, std::string& error)
{
    std::vector<Line> lines;
    std::ifstream input(file);
    if (!input)
    {
        error = "cannot open " + file.string();
        return lines;
    }
    std::string text;
    int number = 0;
    while (std::getline(input, text))
    {
        ++number;
        auto tokens = Tokenize(text);
        if (tokens.empty())
        {
            continue;
        }
        lines.push_back({number, std::move(tokens)});
    }
    return lines;
}

// The engine reads numbers with a hand-written scanner that only accepts digits, '.' and
// '-', so "1e-05" or "+3" would be split into garbage tokens.
bool IsEngineNumber(const std::string& text)
{
    if (text.empty())
    {
        return false;
    }
    const bool onlyAllowedCharacters =
        std::all_of(text.begin(), text.end(),
                    [](char ch) { return std::isdigit(static_cast<unsigned char>(ch)) || ch == '.' || ch == '-'; });
    if (!onlyAllowedCharacters)
    {
        return false;
    }
    char* end = nullptr;
    std::strtod(text.c_str(), &end);
    return end != nullptr && *end == '\0';
}

bool IsEngineInteger(const std::string& text)
{
    if (text.empty())
    {
        return false;
    }
    const size_t start = text[0] == '-' ? 1 : 0;
    if (start == text.size())
    {
        return false;
    }
    return std::all_of(text.begin() + start, text.end(),
                       [](char ch) { return std::isdigit(static_cast<unsigned char>(ch)) != 0; });
}

// Unquoted names must start with a letter and may only contain letters, digits, '.' and '_'.
bool IsEngineName(const std::string& text)
{
    if (text.empty() || !std::isalpha(static_cast<unsigned char>(text[0])))
    {
        return false;
    }
    return std::all_of(text.begin(), text.end(),
                       [](char ch) { return std::isalnum(static_cast<unsigned char>(ch)) || ch == '.' || ch == '_'; });
}

bool HasTextureExtension(const std::string& name)
{
    const size_t dot = name.rfind('.');
    if (dot == std::string::npos)
    {
        return false;
    }
    std::string extension = name.substr(dot + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return extension == "jpg" || extension == "tga";
}

bool IsDummyBoneName(const std::string& name)
{
    return name.size() >= 2 && name[0] == 'D' && name[1] == 'u';
}

std::string JoinTokens(const std::vector<Token>& tokens, size_t first, size_t count)
{
    std::string joined;
    for (size_t index = first; index < first + count && index < tokens.size(); ++index)
    {
        joined += tokens[index].text;
        joined += ' ';
    }
    return joined;
}

class Validator
{
public:
    Validator(SmdKind kind, SmdValidation& result) : m_kind(kind), m_result(result) {}

    void Run(const std::vector<Line>& lines)
    {
        size_t index = 0;
        ParseVersion(lines, index);
        while (index < lines.size())
        {
            const Line& line = lines[index];
            const std::string& keyword = line.tokens[0].text;
            if (keyword == "nodes")
            {
                ParseNodes(lines, index);
            }
            else if (keyword == "skeleton")
            {
                ParseSkeleton(lines, index);
            }
            else if (keyword == "triangles")
            {
                ParseTriangles(lines, index);
            }
            else
            {
                Error(line.number, "unexpected token '" + keyword + "' outside of a nodes/skeleton/triangles block");
                ++index;
            }
        }
        CheckRequiredBlocks();
        CheckMeshLimits();
    }

private:
    void Error(int lineNumber, const std::string& message)
    {
        m_result.errors.push_back(Describe(lineNumber, message));
    }

    void Warn(int lineNumber, const std::string& message)
    {
        m_result.warnings.push_back(Describe(lineNumber, message));
    }

    void ParseVersion(const std::vector<Line>& lines, size_t& index)
    {
        if (lines.empty())
        {
            m_result.errors.push_back("file is empty");
            return;
        }
        const Line& line = lines[0];
        if (line.tokens[0].text != "version" || line.tokens.size() < 2 || !IsEngineInteger(line.tokens[1].text))
        {
            Error(line.number, "expected 'version 1' on the first line");
            return;
        }
        m_result.stats.version = std::atoi(line.tokens[1].text.c_str());
        if (m_result.stats.version != kSupportedVersion)
        {
            Warn(line.number,
                 "SMD version " + line.tokens[1].text + " is not the version 1 the engine tooling was written for");
        }
        index = 1;
    }

    void ParseNodes(const std::vector<Line>& lines, size_t& index)
    {
        if (m_sawNodes)
        {
            Error(lines[index].number, "second nodes block; the engine keeps only one skeleton");
        }
        m_sawNodes = true;
        ++index;
        int expectedId = 0;
        while (index < lines.size() && lines[index].tokens[0].text != "end")
        {
            ParseNodeLine(lines[index], expectedId);
            ++expectedId;
            ++index;
        }
        if (index >= lines.size())
        {
            m_result.errors.push_back("nodes block is not closed with 'end'");
            return;
        }
        ++index;
        m_result.stats.nodeCount = expectedId;
        if (expectedId > kMaxNodes)
        {
            m_result.errors.push_back("skeleton has " + std::to_string(expectedId) +
                                      " bones; the engine allows at most " + std::to_string(kMaxNodes));
        }
    }

    void ParseNodeLine(const Line& line, int expectedId)
    {
        if (line.tokens.size() != static_cast<size_t>(kNodeTokenCount) || !IsEngineInteger(line.tokens[0].text) ||
            !IsEngineInteger(line.tokens[2].text))
        {
            Error(line.number, "node lines must look like: <id> \"<name>\" <parent id>");
            return;
        }
        const int id = std::atoi(line.tokens[0].text.c_str());
        const int parent = std::atoi(line.tokens[2].text.c_str());
        const std::string& name = line.tokens[1].text;
        if (id != expectedId)
        {
            Error(line.number,
                  "node ids must be sequential starting at 0 (expected " + std::to_string(expectedId) + ")");
        }
        if (name.empty() || name.size() > kMaxNameLength)
        {
            Error(line.number, "bone names must be 1 to " + std::to_string(kMaxNameLength) + " characters long");
        }
        if (parent != -1 && (parent < 0 || parent >= id))
        {
            Error(line.number, "a bone's parent must be -1 or a bone listed before it (parent-first order)");
        }
        if (IsDummyBoneName(name))
        {
            Warn(line.number, "bone '" + name +
                                  "' starts with 'Du' and is treated as a dummy: its keyframes are dropped and "
                                  "vertices bound to it stay at the model origin");
        }
        m_result.stats.nodeNames.push_back(name);
    }

    void ParseSkeleton(const std::vector<Line>& lines, size_t& index)
    {
        m_sawSkeleton = true;
        const int blockLine = lines[index].number;
        ++index;
        int frames = 0;
        while (index < lines.size() && lines[index].tokens[0].text != "end")
        {
            const Line& line = lines[index];
            if (line.tokens[0].text != "time")
            {
                Error(line.number, "expected 'time <frame>' before the bone lines");
                ++index;
                continue;
            }
            ParseFrameHeader(line, frames);
            ++index;
            ParseFrameBones(lines, index);
            ++frames;
        }
        if (index >= lines.size())
        {
            m_result.errors.push_back("skeleton block is not closed with 'end'");
            return;
        }
        ++index;
        m_result.stats.frameCount = frames;
        if (frames == 0)
        {
            Error(blockLine, "skeleton block has no 'time' frame");
        }
        if (frames > kMaxFrames)
        {
            Error(blockLine, "animation has " + std::to_string(frames) + " frames; the engine allows at most " +
                                 std::to_string(kMaxFrames) + " per action");
        }
        if (m_kind == SmdKind::Reference && frames > 1)
        {
            Warn(blockLine,
                 "reference files should have exactly one frame; the engine keeps the last one as bind pose");
        }
    }

    void ParseFrameHeader(const Line& line, int expectedFrame)
    {
        if (line.tokens.size() < 2 || !IsEngineInteger(line.tokens[1].text))
        {
            Error(line.number, "'time' must be followed by a frame number");
            return;
        }
        const int frame = std::atoi(line.tokens[1].text.c_str());
        if (frame != expectedFrame)
        {
            Error(line.number, "frames must be numbered 0, 1, 2, ... without gaps (expected " +
                                   std::to_string(expectedFrame) +
                                   "); the engine indexes its frame table by this value");
        }
    }

    void ParseFrameBones(const std::vector<Line>& lines, size_t& index)
    {
        int boneLines = 0;
        while (index < lines.size() && lines[index].tokens[0].text != "time" && lines[index].tokens[0].text != "end")
        {
            const Line& line = lines[index];
            if (line.tokens.size() != static_cast<size_t>(kSkeletonTokenCount))
            {
                Error(line.number, "bone lines must look like: <id> <x> <y> <z> <rx> <ry> <rz>");
            }
            else
            {
                CheckBoneLine(line);
            }
            ++boneLines;
            ++index;
        }
        if (m_result.stats.nodeCount > 0 && boneLines != m_result.stats.nodeCount)
        {
            Error(lines[index - 1].number,
                  "a frame lists " + std::to_string(boneLines) + " bones but the skeleton has " +
                      std::to_string(m_result.stats.nodeCount) + "; the engine expects every bone in every frame");
        }
    }

    void CheckBoneLine(const Line& line)
    {
        if (!IsEngineInteger(line.tokens[0].text) || std::atoi(line.tokens[0].text.c_str()) < 0 ||
            std::atoi(line.tokens[0].text.c_str()) >= m_result.stats.nodeCount)
        {
            Error(line.number, "bone id out of range");
        }
        for (size_t token = 1; token < line.tokens.size(); ++token)
        {
            if (!IsEngineNumber(line.tokens[token].text))
            {
                Error(line.number,
                      "'" + line.tokens[token].text + "' is not a plain decimal number (no exponents or '+' signs)");
                return;
            }
        }
    }

    void ParseTriangles(const std::vector<Line>& lines, size_t& index)
    {
        m_sawTriangles = true;
        if (m_kind == SmdKind::Animation)
        {
            Warn(lines[index].number, "animation files should not contain triangles; the block is ignored");
        }
        ++index;
        while (index < lines.size() && lines[index].tokens[0].text != "end")
        {
            if (!ParseTriangle(lines, index))
            {
                return;
            }
        }
        if (index >= lines.size())
        {
            m_result.errors.push_back("triangles block is not closed with 'end'");
            return;
        }
        ++index;
    }

    bool ParseTriangle(const std::vector<Line>& lines, size_t& index)
    {
        const Line& materialLine = lines[index];
        if (materialLine.tokens.size() != 1)
        {
            Error(materialLine.number, "expected a material (texture file) name on its own line");
            ++index;
            return true;
        }
        const std::string& material = materialLine.tokens[0].text;
        CheckMaterialName(materialLine, material);
        MaterialUsage& usage = m_materials[material];
        ++usage.triangles;
        ++m_result.stats.triangleCount;
        ++index;
        for (int corner = 0; corner < 3; ++corner)
        {
            if (index >= lines.size() || lines[index].tokens[0].text == "end")
            {
                Error(materialLine.number, "triangle has fewer than three vertex lines");
                return false;
            }
            CheckVertexLine(lines[index], usage);
            ++index;
        }
        return true;
    }

    void CheckMaterialName(const Line& line, const std::string& material)
    {
        if (m_materials.count(material) != 0)
        {
            return;
        }
        if (line.tokens[0].quoted || !IsEngineName(material))
        {
            Error(line.number, "material '" + material +
                                   "' must start with a letter and contain only letters, digits, '.' and '_' (no "
                                   "quotes, spaces or dashes)");
        }
        if (material.size() > kMaxNameLength)
        {
            Error(line.number, "material '" + material + "' is longer than " + std::to_string(kMaxNameLength) +
                                   " characters; texture names are stored in 32-byte fields");
        }
        if (!HasTextureExtension(material))
        {
            Warn(line.number, "material '" + material +
                                  "' does not end in .jpg or .tga; the client only loads those (as .OZJ / .OZT)");
        }
        m_result.stats.materials.push_back(material);
    }

    void CheckVertexLine(const Line& line, MaterialUsage& usage)
    {
        if (line.tokens.size() < static_cast<size_t>(kVertexTokenCount))
        {
            Error(line.number, "vertex lines need at least 9 values: <bone> <x> <y> <z> <nx> <ny> <nz> <u> <v>");
            return;
        }
        if (!IsEngineInteger(line.tokens[0].text) || std::atoi(line.tokens[0].text.c_str()) < 0 ||
            std::atoi(line.tokens[0].text.c_str()) >= m_result.stats.nodeCount)
        {
            Error(line.number, "vertex bone id out of range");
        }
        for (int token = 1; token < kVertexTokenCount; ++token)
        {
            if (!IsEngineNumber(line.tokens[token].text))
            {
                Error(line.number,
                      "'" + line.tokens[token].text + "' is not a plain decimal number (no exponents or '+' signs)");
                return;
            }
        }
        if (line.tokens.size() > static_cast<size_t>(kVertexTokenCount) && !m_warnedAboutWeights)
        {
            m_warnedAboutWeights = true;
            Warn(line.number,
                 "vertex weight links are ignored; the engine binds every vertex to the first bone column only");
        }
        usage.positions.insert(JoinTokens(line.tokens, 1, 3));
        usage.normals.insert(JoinTokens(line.tokens, 4, 3));
        usage.uvs.insert(JoinTokens(line.tokens, 7, 2));
    }

    void CheckRequiredBlocks()
    {
        if (!m_sawNodes)
        {
            m_result.errors.push_back("missing nodes block");
        }
        if (!m_sawSkeleton)
        {
            m_result.errors.push_back("missing skeleton block");
        }
        if (m_kind == SmdKind::Reference && !m_sawTriangles)
        {
            m_result.errors.push_back("reference file has no triangles block");
        }
        if (m_kind == SmdKind::Reference && m_sawTriangles && m_result.stats.triangleCount == 0)
        {
            m_result.warnings.push_back(
                "triangles block is empty: this describes a skeleton-only model (animations without a mesh)");
        }
    }

    void CheckMeshLimits()
    {
        if (m_result.stats.triangleCount > kMaxTriangles)
        {
            m_result.errors.push_back("model has " + std::to_string(m_result.stats.triangleCount) +
                                      " triangles; the engine converter holds at most " +
                                      std::to_string(kMaxTriangles));
        }
        if (m_materials.size() > static_cast<size_t>(kMaxMeshes))
        {
            m_result.errors.push_back("model uses " + std::to_string(m_materials.size()) +
                                      " materials; each becomes a mesh and the engine allows at most " +
                                      std::to_string(kMaxMeshes));
        }
        for (const auto& [material, usage] : m_materials)
        {
            const size_t unique = std::max({usage.positions.size(), usage.normals.size(), usage.uvs.size()});
            if (unique > static_cast<size_t>(kMaxUniquePerMesh))
            {
                m_result.errors.push_back("material '" + material + "' has about " + std::to_string(unique) +
                                          " unique vertices; the engine allows at most " +
                                          std::to_string(kMaxUniquePerMesh) + " per mesh");
            }
        }
    }

    SmdKind m_kind;
    SmdValidation& m_result;
    std::map<std::string, MaterialUsage> m_materials;
    bool m_sawNodes = false;
    bool m_sawSkeleton = false;
    bool m_sawTriangles = false;
    bool m_warnedAboutWeights = false;
};
} // namespace

SmdValidation ValidateSmd(const std::filesystem::path& file, SmdKind kind)
{
    SmdValidation result;
    std::string error;
    const auto lines = ReadLines(file, error);
    if (!error.empty())
    {
        result.errors.push_back(error);
        return result;
    }
    Validator validator(kind, result);
    validator.Run(lines);
    return result;
}
} // namespace Tools::BmdConv
