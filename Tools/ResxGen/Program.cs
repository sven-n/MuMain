// ResxGen: read .resx files under an input directory and emit C++ headers +
// implementations that expose every translation as a typed namespace-level
// extern pointer. Build-tool consumed by CMake; output lives in the build
// tree and is not committed.
//
// Naming convention for .resx files: <Group>.<locale>.resx
//   e.g. Editor.en.resx, Editor.de.resx
// Files with the same <Group> are merged into one C++ namespace. The
// default-locale file ("en" by convention) is the source of truth for which
// keys exist; other locales may translate a subset and fall back to English
// for missing keys.
//
// The resx key IS the English text (po-style). The generator derives a
// PascalCase C++ identifier from the key and emits something like:
//
//   namespace I18N::Editor {
//       extern const char* SaveSkills;   // from key "Save Skills"
//   }
//
// Call sites use the fully-qualified name:
//
//   ImGui::Button(I18N::Editor::SaveSkills);
//   I18N::SetLocale("de");

namespace MuMain.Tools.ResxGen;

internal static class Program
{
    public static int Main(string[] args)
    {
        if (!CommandLine.TryParse(args, out var options, out var error))
        {
            Console.Error.WriteLine(error);
            Console.Error.WriteLine(CommandLine.Usage);
            return 1;
        }

        if (!Directory.Exists(options.InputDir))
        {
            Console.Error.WriteLine($"Input directory not found: {options.InputDir}");
            return 1;
        }

        Directory.CreateDirectory(options.OutputDir);

        IReadOnlyList<ResourceGroup> groups;
        try
        {
            groups = ResxLoader.LoadGroups(options.InputDir);
        }
        catch (ResxLoadException ex)
        {
            Console.Error.WriteLine(ex.Message);
            return 1;
        }

        if (groups.Count == 0)
        {
            Console.Error.WriteLine($"No .resx files found under {options.InputDir}");
            return 1;
        }

        // Apply wide-group flag from the CLI; loader doesn't know about it.
        groups = groups.Select(g => options.WideGroups.Contains(g.Name)
                                        ? g with { IsWide = true }
                                        : g)
                       .ToList();

        foreach (var group in groups)
        {
            CppEmitter.WriteGroupHeader(options.OutputDir, group);
            CppEmitter.WriteGroupSource(options.OutputDir, group);
        }

        CppEmitter.WriteMasterHeader(options.OutputDir, groups);
        CppEmitter.WriteMasterSource(options.OutputDir, groups);

        Console.WriteLine($"ResxGen: wrote {groups.Count} group(s) to {options.OutputDir}");
        return 0;
    }
}
