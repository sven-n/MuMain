using System.Text;

namespace MuMain.Tools.DialogImporter;

/// One-shot importer: turns Dialog_<lang>.bmd files into Tools/ResxGen-
/// compatible resx files and a language-agnostic dialog-branching table.
///
/// Usage:
///   DialogImporter \
///     --eng <Dialog_eng.bmd> \
///     --por <Dialog_por.bmd> \
///     --spn <Dialog_spn.bmd> \
///     --resx-out <src/Localization> \
///     --struct-header <src/source/GameLogic/Quests/DialogStructure.h> \
///     --struct-source <src/source/GameLogic/Quests/DialogStructure.cpp>
///
/// English is authoritative for structure (links, returns, numAnswer).
/// Portuguese and Spanish are read for their text only; they MUST agree
/// structurally — the importer aborts loudly otherwise so a structurally
/// mismatched fork can't silently corrupt the dialog graph.
internal static class Program
{
    public static int Main(string[] args)
    {
        Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);

        var opts = ParseArgs(args);
        if (opts is null) return 2;

        Console.WriteLine($"Reading {opts.EnglishBmd} (authoritative)");
        var eng = BmdReader.Read(opts.EnglishBmd, EncodingFor("en"));

        var translations = new Dictionary<string, IReadOnlyList<DialogRecord>>(StringComparer.Ordinal);
        translations["en"] = eng;
        foreach (var (locale, path) in opts.OtherSources)
        {
            Console.WriteLine($"Reading {path} ({locale})");
            var entries = BmdReader.Read(path, EncodingFor(locale));
            EnsureStructuralMatch(opts.EnglishBmd, eng, path, entries);
            translations[locale] = entries;
        }

        Directory.CreateDirectory(opts.ResxOutDir);
        foreach (var (locale, entries) in translations)
        {
            var path = Path.Combine(opts.ResxOutDir, $"Dialog.{locale}.resx");
            var n = ResxWriter.Write(path, locale, entries);
            Console.WriteLine($"Wrote {path} ({n} entries)");
        }

        Directory.CreateDirectory(Path.GetDirectoryName(opts.StructHeaderPath)!);
        Directory.CreateDirectory(Path.GetDirectoryName(opts.StructSourcePath)!);
        StructureWriter.Write(opts.StructHeaderPath, opts.StructSourcePath, eng);
        Console.WriteLine($"Wrote {opts.StructHeaderPath}");
        Console.WriteLine($"Wrote {opts.StructSourcePath}");

        return 0;
    }

    /// Maps a locale code to the encoding the original tool chain wrote the
    /// bmd in. English files are ASCII / UTF-8 (no high-bit bytes to worry
    /// about). Portuguese and Spanish were emitted as Windows-1252, which
    /// covers all the accented Latin chars they need.
    private static Encoding EncodingFor(string locale) => locale switch
    {
        "en" => new UTF8Encoding(false),
        "pt" or "es" => Encoding.GetEncoding(1252),
        _ => new UTF8Encoding(false),
    };

    private static void EnsureStructuralMatch(
        string baseLabel,
        IReadOnlyList<DialogRecord> baseEntries,
        string otherLabel,
        IReadOnlyList<DialogRecord> otherEntries)
    {
        if (baseEntries.Count != otherEntries.Count)
        {
            throw new InvalidDataException(
                $"{otherLabel}: entry count {otherEntries.Count} != {baseLabel} count {baseEntries.Count}");
        }
        var diffs = 0;
        for (var i = 0; i < baseEntries.Count; i++)
        {
            var b = baseEntries[i];
            var o = otherEntries[i];
            if (b.NumAnswer != o.NumAnswer)
            {
                Console.Error.WriteLine($"  [structural diff @ {i}] numAnswer: base={b.NumAnswer} other={o.NumAnswer}");
                diffs++;
                continue;
            }
            for (var j = 0; j < b.NumAnswer; j++)
            {
                if (b.Links[j] != o.Links[j])
                {
                    Console.Error.WriteLine($"  [structural diff @ {i}] link[{j}]: base={b.Links[j]} other={o.Links[j]}");
                    diffs++;
                }
                if (b.Returns[j] != o.Returns[j])
                {
                    Console.Error.WriteLine($"  [structural diff @ {i}] return[{j}]: base={b.Returns[j]} other={o.Returns[j]}");
                    diffs++;
                }
            }
        }
        if (diffs > 0)
        {
            throw new InvalidDataException(
                $"{otherLabel} disagrees structurally with {baseLabel} ({diffs} differences). " +
                "Reconcile the source files before importing.");
        }
    }

    private static Options? ParseArgs(string[] args)
    {
        string? eng = null;
        var others = new List<(string Locale, string Path)>();
        string? resxOut = null;
        string? structHeader = null;
        string? structSource = null;

        for (var i = 0; i < args.Length; i++)
        {
            var key = args[i];
            string Next() => i + 1 < args.Length ? args[++i] : throw new ArgumentException($"missing value for {key}");
            switch (key)
            {
                case "--eng": eng = Next(); break;
                case "--por": others.Add(("pt", Next())); break;
                case "--spn": others.Add(("es", Next())); break;
                case "--resx-out": resxOut = Next(); break;
                case "--struct-header": structHeader = Next(); break;
                case "--struct-source": structSource = Next(); break;
                default:
                    Console.Error.WriteLine($"Unknown argument: {key}");
                    return null;
            }
        }

        if (eng is null || resxOut is null || structHeader is null || structSource is null)
        {
            Console.Error.WriteLine("Required: --eng <path> --resx-out <dir> --struct-header <path> --struct-source <path>");
            Console.Error.WriteLine("Optional: --por <path> --spn <path>");
            return null;
        }

        return new Options(eng, others, resxOut, structHeader, structSource);
    }

    private sealed record Options(
        string EnglishBmd,
        IReadOnlyList<(string Locale, string Path)> OtherSources,
        string ResxOutDir,
        string StructHeaderPath,
        string StructSourcePath);
}
