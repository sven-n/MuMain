using System.Text.RegularExpressions;
using System.Xml.Linq;

namespace MuMain.Tools.ResxGen;

internal sealed class ResxLoadException : Exception
{
    public ResxLoadException(string message) : base(message) { }
}

/// Raw entry read off a single .resx data element. The Value is what goes
/// into the C++ string table; LegacyIds are optional integer IDs harvested
/// from a `<comment>legacy_id=N,N,...</comment>` sidecar.
internal sealed record ResxRawEntry(string Value, IReadOnlyList<int> LegacyIds);

internal static class ResxLoader
{
    /// Default locale code used as the fallback for missing translations.
    public const string DefaultLocale = "en";

    /// resx convention: entries whose name starts with ">>" describe
    /// resource types (not user-visible strings); skip them.
    private const string ResxMetadataKeyPrefix = ">>";

    /// Top-level resx element name for a translation entry.
    private const string ResxDataElement = "data";

    /// Element inside <data> holding the localized text.
    private const string ResxValueElement = "value";

    /// Element inside <data> holding optional metadata.
    private const string ResxCommentElement = "comment";

    /// Attribute on <data> holding the key.
    private const string ResxNameAttribute = "name";

    /// Matches `legacy_id=12,34,56` (whitespace tolerant) inside a comment.
    private static readonly Regex LegacyIdRegex = new(
        @"legacy_id\s*=\s*([0-9]+(?:\s*,\s*[0-9]+)*)",
        RegexOptions.Compiled | RegexOptions.IgnoreCase);

    public static IReadOnlyList<ResourceGroup> LoadGroups(string inputDir)
    {
        var byGroup = ScanInputDir(inputDir);

        var result = new List<ResourceGroup>(byGroup.Count);
        foreach (var (groupName, perLocale) in byGroup.OrderBy(kv => kv.Key, StringComparer.Ordinal))
        {
            result.Add(BuildGroup(groupName, perLocale));
        }
        return result;
    }

    private static SortedDictionary<string, Dictionary<string, Dictionary<string, ResxRawEntry>>> ScanInputDir(string inputDir)
    {
        var byGroup = new SortedDictionary<string, Dictionary<string, Dictionary<string, ResxRawEntry>>>(StringComparer.Ordinal);

        foreach (var path in Directory.EnumerateFiles(inputDir, "*.resx", SearchOption.TopDirectoryOnly))
        {
            if (!TrySplitName(path, out var group, out var locale))
            {
                Console.Error.WriteLine($"Skipping {path}: filename must be <Group>.<locale>.resx");
                continue;
            }

            if (!byGroup.TryGetValue(group, out var perLocale))
            {
                perLocale = new Dictionary<string, Dictionary<string, ResxRawEntry>>(StringComparer.Ordinal);
                byGroup[group] = perLocale;
            }

            perLocale[locale] = ReadResx(path);
        }

        return byGroup;
    }

    private static bool TrySplitName(string path, out string group, out string locale)
    {
        var name = Path.GetFileNameWithoutExtension(path);     // "Editor.en"
        var dot = name.LastIndexOf('.');
        if (dot <= 0 || dot == name.Length - 1)
        {
            group = string.Empty;
            locale = string.Empty;
            return false;
        }
        group = name[..dot];
        locale = name[(dot + 1)..];
        return true;
    }

    private static ResourceGroup BuildGroup(
        string groupName,
        Dictionary<string, Dictionary<string, ResxRawEntry>> perLocale)
    {
        if (!perLocale.TryGetValue(DefaultLocale, out var defaultEntries))
        {
            throw new ResxLoadException(
                $"Group '{groupName}' is missing the default locale '{groupName}.{DefaultLocale}.resx'");
        }

        WarnForKeysMissingFromDefault(groupName, perLocale, defaultEntries);

        var locales = SortLocales(perLocale.Keys);
        var entries = BuildEntries(groupName, defaultEntries, perLocale);

        return new ResourceGroup(groupName, locales, entries);
    }

    private static IReadOnlyList<string> SortLocales(IEnumerable<string> locales)
    {
        var list = locales.ToList();
        list.Sort((a, b) =>
        {
            if (a == DefaultLocale) return -1;
            if (b == DefaultLocale) return 1;
            return string.CompareOrdinal(a, b);
        });
        return list;
    }

    private static IReadOnlyList<ResourceEntry> BuildEntries(
        string groupName,
        Dictionary<string, ResxRawEntry> defaultEntries,
        Dictionary<string, Dictionary<string, ResxRawEntry>> perLocale)
    {
        var entries = new List<ResourceEntry>(defaultEntries.Count);
        var idToKey = new Dictionary<string, string>(StringComparer.Ordinal);
        var legacyIdToKey = new Dictionary<int, string>();

        foreach (var (key, defaultRaw) in defaultEntries)
        {
            var identifier = SafeIdentifier(groupName, key);
            if (identifier.Length == 0)
            {
                // No usable ASCII identifier (typically dead Korean leftover
                // text in the source bmd). Skip the entry so it doesn't
                // produce non-ASCII identifiers that MSVC refuses to compile.
                Console.Error.WriteLine(
                    $"Skipping {groupName} entry with no ASCII identifier: '{key.Substring(0, Math.Min(40, key.Length))}'");
                continue;
            }
            EnsureNoIdentifierCollision(groupName, idToKey, identifier, key);
            EnsureNoLegacyIdCollision(groupName, legacyIdToKey, defaultRaw.LegacyIds, key);

            var translations = new Dictionary<string, string>(StringComparer.Ordinal);
            foreach (var (locale, entriesForLocale) in perLocale)
            {
                if (entriesForLocale.TryGetValue(key, out var raw))
                {
                    translations[locale] = raw.Value;
                }
            }

            entries.Add(new ResourceEntry(key, identifier, translations, defaultRaw.LegacyIds));
        }

        return entries;
    }

    private static void EnsureNoLegacyIdCollision(
        string groupName,
        Dictionary<int, string> legacyIdToKey,
        IReadOnlyList<int> legacyIds,
        string key)
    {
        foreach (var legacyId in legacyIds)
        {
            if (legacyIdToKey.TryGetValue(legacyId, out var firstKey))
            {
                throw new ResxLoadException(
                    $"Group '{groupName}': legacy_id={legacyId} is assigned to both " +
                    $"'{firstKey}' and '{key}'. Each legacy_id must be unique within a " +
                    "group because I18N::<Group>::Lookup(int) does a binary search that " +
                    "assumes unique sorted IDs.");
            }
            legacyIdToKey[legacyId] = key;
        }
    }

    private static string SafeIdentifier(string groupName, string key)
    {
        var identifier = Naming.ToIdentifier(key);
        if (identifier.Length > Naming.MaxIdentifierLength)
        {
            throw new ResxLoadException(
                $"Group '{groupName}': key '{key}' slugs to a {identifier.Length}-char identifier " +
                $"(limit {Naming.MaxIdentifierLength}). Pick a shorter key.");
        }
        return identifier;
    }

    private static void EnsureNoIdentifierCollision(
        string groupName,
        Dictionary<string, string> idToKey,
        string identifier,
        string key)
    {
        if (idToKey.TryGetValue(identifier, out var firstKey))
        {
            throw new ResxLoadException(
                $"Group '{groupName}': keys '{firstKey}' and '{key}' both slug to identifier '{identifier}'. " +
                "Rename one of the keys to disambiguate.");
        }
        idToKey[identifier] = key;
    }

    private static void WarnForKeysMissingFromDefault(
        string groupName,
        Dictionary<string, Dictionary<string, ResxRawEntry>> perLocale,
        Dictionary<string, ResxRawEntry> defaultEntries)
    {
        var defaultKeys = new HashSet<string>(defaultEntries.Keys, StringComparer.Ordinal);
        foreach (var (locale, entries) in perLocale)
        {
            foreach (var extra in entries.Keys.Where(k => !defaultKeys.Contains(k)))
            {
                Console.Error.WriteLine(
                    $"Warning: key '{extra}' in {groupName}.{locale}.resx is missing from {groupName}.{DefaultLocale}.resx; skipping.");
            }
        }
    }

    private static Dictionary<string, ResxRawEntry> ReadResx(string path)
    {
        var doc = XDocument.Load(path);
        var result = new Dictionary<string, ResxRawEntry>(StringComparer.Ordinal);

        if (doc.Root is null) return result;

        foreach (var data in doc.Root.Elements(ResxDataElement))
        {
            var name = (string?)data.Attribute(ResxNameAttribute);
            if (string.IsNullOrEmpty(name)) continue;
            if (name.StartsWith(ResxMetadataKeyPrefix, StringComparison.Ordinal)) continue;

            var value = data.Element(ResxValueElement)?.Value ?? string.Empty;
            var comment = data.Element(ResxCommentElement)?.Value;
            var legacyIds = ParseLegacyIds(comment);

            result[name] = new ResxRawEntry(value, legacyIds);
        }

        return result;
    }

    private static IReadOnlyList<int> ParseLegacyIds(string? comment)
    {
        if (string.IsNullOrEmpty(comment)) return Array.Empty<int>();
        var match = LegacyIdRegex.Match(comment);
        if (!match.Success) return Array.Empty<int>();
        var list = new List<int>();
        foreach (var part in match.Groups[1].Value.Split(',', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
        {
            if (int.TryParse(part, out var id)) list.Add(id);
        }
        return list;
    }
}
