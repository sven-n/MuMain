// bmdconv - command-line converter between the client's BMD models and Valve SMD text
// files, the format Blender Source Tools imports and exports. Usage documentation lives in
// docs/asset-pipeline.md.
#include "stdafx.h"

#include "BmdSmdConverter.h"
#include "SmdValidator.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace
{
namespace fs = std::filesystem;
using namespace Tools::BmdConv;

constexpr int kExitOk = 0;
constexpr int kExitUsage = 1;
constexpr int kExitFailure = 2;

void PrintUsage()
{
    std::cout << "bmdconv - convert MU Online BMD models to and from Valve SMD (Blender Source Tools)\n\n"
                 "  bmdconv info <model.bmd>\n"
                 "      Print name, meshes, textures, bones, actions and the bind-pose bounds.\n"
                 "  bmdconv validate <file.smd> [--animation]\n"
                 "      Check an SMD against the engine parser limits without converting.\n"
                 "  bmdconv bmd2smd <model.bmd> <output-dir>\n"
                 "      Write <stem>.smd, <stem>_aNN.smd per action and <stem>.actions.txt.\n"
                 "  bmdconv bmd2smd-dir <input-dir> <output-dir>\n"
                 "      Same for every .bmd below <input-dir>; each model gets its own folder.\n"
                 "  bmdconv smd2bmd <reference.smd> <output.bmd> [options]\n"
                 "      --anim <file.smd>[:lock]   add an action (repeatable, order = action index)\n"
                 "      --manifest <file>          add the actions listed in a <stem>.actions.txt\n"
                 "      --flip                     reverse triangle winding\n"
                 "      --name <text>              value for the model's name field (max 31 chars)\n"
                 "      Without --anim/--manifest the reference skeleton becomes the single action.\n"
                 "  bmdconv compare <a.bmd> <b.bmd> [--tolerance <units>]\n"
                 "      Report geometry and bone-motion differences (default tolerance 0.05 units).\n";
}

void PrintWarnings(const std::vector<std::string>& warnings)
{
    for (const auto& warning : warnings)
    {
        std::cerr << "warning: " << warning << '\n';
    }
}

bool HasBmdExtension(const fs::path& file)
{
    std::string extension = file.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return extension == ".bmd";
}

int RunInfo(const std::vector<std::string>& args)
{
    if (args.size() != 1)
    {
        PrintUsage();
        return kExitUsage;
    }
    BMD model;
    std::string error;
    if (!LoadBmd(args[0], model, error))
    {
        std::cerr << "error: " << error << '\n';
        return kExitFailure;
    }
    std::cout << DescribeModel(model, args[0]);
    return kExitOk;
}

int RunValidate(const std::vector<std::string>& args)
{
    if (args.empty())
    {
        PrintUsage();
        return kExitUsage;
    }
    const bool animation = std::find(args.begin(), args.end(), "--animation") != args.end();
    const auto validation = ValidateSmd(args[0], animation ? SmdKind::Animation : SmdKind::Reference);
    std::cout << args[0] << ": bones=" << validation.stats.nodeCount << " frames=" << validation.stats.frameCount
              << " triangles=" << validation.stats.triangleCount << " materials=" << validation.stats.materials.size()
              << '\n';
    PrintWarnings(validation.warnings);
    for (const auto& problem : validation.errors)
    {
        std::cerr << "error: " << problem << '\n';
    }
    std::cout << (validation.ok() ? "OK" : "REJECTED") << '\n';
    return validation.ok() ? kExitOk : kExitFailure;
}

int ExportOne(const fs::path& bmdFile, const fs::path& outputDirectory, bool verbose)
{
    BMD model;
    std::string error;
    if (!LoadBmd(bmdFile, model, error))
    {
        std::cerr << "error: " << error << '\n';
        return kExitFailure;
    }
    const auto result = ExportBmdToSmd(model, outputDirectory, bmdFile.stem().string());
    PrintWarnings(result.warnings);
    if (!result.ok)
    {
        std::cerr << "error: " << result.error << '\n';
        return kExitFailure;
    }
    if (verbose)
    {
        for (const auto& output : result.outputs)
        {
            std::cout << "wrote " << output.string() << '\n';
        }
    }
    return kExitOk;
}

int RunBmdToSmd(const std::vector<std::string>& args)
{
    if (args.size() != 2)
    {
        PrintUsage();
        return kExitUsage;
    }
    return ExportOne(args[0], args[1], true);
}

int RunBmdToSmdDirectory(const std::vector<std::string>& args)
{
    if (args.size() != 2)
    {
        PrintUsage();
        return kExitUsage;
    }
    const fs::path input = args[0];
    const fs::path output = args[1];
    int converted = 0;
    int failed = 0;
    for (const auto& entry : fs::recursive_directory_iterator(input))
    {
        if (!entry.is_regular_file() || !HasBmdExtension(entry.path()))
        {
            continue;
        }
        const fs::path relative = fs::relative(entry.path(), input);
        const fs::path target = output / relative.parent_path() / relative.stem();
        std::cout << relative.string() << '\n';
        if (ExportOne(entry.path(), target, false) == kExitOk)
        {
            ++converted;
        }
        else
        {
            ++failed;
        }
    }
    std::cout << "converted " << converted << " models, " << failed << " files skipped or failed\n";
    return failed == 0 ? kExitOk : kExitFailure;
}

bool ParseAnimationArgument(const std::string& value, AnimationInput& animation)
{
    const size_t colon = value.rfind(':');
    if (colon != std::string::npos && value.substr(colon + 1) == "lock")
    {
        animation.file = value.substr(0, colon);
        animation.lockPositions = true;
    }
    else
    {
        animation.file = value;
        animation.lockPositions = false;
    }
    return !animation.file.empty();
}

bool AppendManifestAnimations(const fs::path& manifest, ImportRequest& request)
{
    std::vector<ActionManifestEntry> entries;
    std::string error;
    if (!ReadActionManifest(manifest, entries, error))
    {
        std::cerr << "error: " << error << '\n';
        return false;
    }
    for (const auto& entry : entries)
    {
        request.animations.push_back({entry.file, entry.lockPositions});
    }
    return true;
}

bool ParseImportOptions(const std::vector<std::string>& args, ImportRequest& request)
{
    for (size_t index = 2; index < args.size(); ++index)
    {
        const std::string& option = args[index];
        const bool hasValue = index + 1 < args.size();
        if (option == "--flip")
        {
            request.flipWinding = true;
        }
        else if (option == "--anim" && hasValue)
        {
            AnimationInput animation;
            if (!ParseAnimationArgument(args[++index], animation))
            {
                return false;
            }
            request.animations.push_back(animation);
        }
        else if (option == "--manifest" && hasValue)
        {
            if (!AppendManifestAnimations(args[++index], request))
            {
                return false;
            }
        }
        else if (option == "--name" && hasValue)
        {
            request.modelName = args[++index];
        }
        else
        {
            std::cerr << "error: unknown or incomplete option '" << option << "'\n";
            return false;
        }
    }
    return true;
}

int RunCompare(const std::vector<std::string>& args)
{
    if (args.size() < 2)
    {
        PrintUsage();
        return kExitUsage;
    }
    constexpr float kDefaultTolerance = 0.05f;
    float tolerance = kDefaultTolerance;
    for (size_t index = 2; index + 1 < args.size(); ++index)
    {
        if (args[index] == "--tolerance")
        {
            tolerance = std::stof(args[index + 1]);
        }
    }
    BMD a;
    BMD b;
    std::string error;
    if (!LoadBmd(args[0], a, error) || !LoadBmd(args[1], b, error))
    {
        std::cerr << "error: " << error << '\n';
        return kExitFailure;
    }
    const auto report = CompareModels(a, b, tolerance);
    std::cout << DescribeComparison(report);
    return report.equivalent() ? kExitOk : kExitFailure;
}

int RunSmdToBmd(const std::vector<std::string>& args)
{
    if (args.size() < 2)
    {
        PrintUsage();
        return kExitUsage;
    }
    ImportRequest request;
    request.referenceSmd = args[0];
    if (!ParseImportOptions(args, request))
    {
        return kExitUsage;
    }
    const auto result = ImportSmdToBmd(request, args[1]);
    PrintWarnings(result.warnings);
    if (!result.ok)
    {
        std::cerr << "error: " << result.error << '\n';
        return kExitFailure;
    }
    std::cout << "wrote " << args[1] << " (" << (request.animations.empty() ? 1 : request.animations.size())
              << " action(s))\n";
    return kExitOk;
}
} // namespace

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        PrintUsage();
        return kExitUsage;
    }
    const std::string command = argv[1];
    const std::vector<std::string> args(argv + 2, argv + argc);
    if (command == "info")
    {
        return RunInfo(args);
    }
    if (command == "validate")
    {
        return RunValidate(args);
    }
    if (command == "bmd2smd")
    {
        return RunBmdToSmd(args);
    }
    if (command == "bmd2smd-dir")
    {
        return RunBmdToSmdDirectory(args);
    }
    if (command == "smd2bmd")
    {
        return RunSmdToBmd(args);
    }
    if (command == "compare")
    {
        return RunCompare(args);
    }
    PrintUsage();
    return kExitUsage;
}
