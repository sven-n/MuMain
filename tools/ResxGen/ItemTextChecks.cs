namespace MuMain.Tools.ResxGen;

/// Build-time checks of the item text groups (--item-text-groups): a text that starts an item reference
/// must write it in the grammar of ItemReferenceSyntax, or the build stops and names the file, key and
/// text. Whether the item exists is checked by the game and the editor's Data Checks, which know the
/// item data.
internal static class ItemTextChecks
{
    /// The emitted table keeps one bit per locale for the locales with a text of their own.
    private const int MaxLocales = 32;

    public static IReadOnlyList<string> Check(IReadOnlyList<ResourceGroup> groups, IReadOnlyList<string> masterLocales)
    {
        var errors = new List<string>();
        var itemTextGroups = groups.Where(g => g.NamesItems).ToList();
        if (itemTextGroups.Count > 0 && masterLocales.Count > MaxLocales)
        {
            errors.Add($"Error: item text groups support at most {MaxLocales} locales, the build has {masterLocales.Count}.");
        }
        foreach (var group in itemTextGroups)
        {
            if (!group.IsWide)
            {
                errors.Add($"Error: item text group '{group.Name}' must also be a wide group (--wide-groups).");
            }
            CheckReferences(group, errors);
        }
        return errors;
    }

    private static void CheckReferences(ResourceGroup group, List<string> errors)
    {
        foreach (var entry in group.Entries)
        {
            foreach (var (locale, text) in entry.Translations.OrderBy(t => t.Key, StringComparer.Ordinal))
            {
                foreach (var reference in ItemReferenceSyntax.FindMalformed(text))
                {
                    errors.Add($"Error: {group.Name}.{locale}.resx, key '{entry.Key}': malformed item reference " +
                               $"'{reference}' in \"{text}\". Write {{item:group,number}}, {{item:group,number+level}} " +
                               $"or add :case ({string.Join(", ", ItemReferenceSyntax.CaseKeys)}).");
                }
            }
        }
    }
}
