namespace MuMain.Tools.ResxGen;

/// Build-time checks of the item text groups (--item-text-groups): a text that starts an item reference
/// must write it in the grammar of ItemReferenceSyntax, or the build stops and names the file, key and
/// text. So does a reference in a group that is no item text group, and a group name on the command line
/// that no .resx file has: the game would show those references as written. Whether the item exists is
/// checked by the game and the editor's Data Checks, which know the item data.
internal static class ItemTextChecks
{
    /// The emitted table keeps one bit per locale for the locales with a text of their own.
    private const int MaxLocales = 32;

    public static IReadOnlyList<string> Check(IReadOnlyList<ResourceGroup> groups, IReadOnlyList<string> masterLocales,
                                              IReadOnlySet<string> requestedGroups)
    {
        var errors = new List<string>();
        CheckRequestedGroups(groups, requestedGroups, errors);
        var itemTextGroups = groups.Where(g => g.NamesItems).ToList();
        if (itemTextGroups.Count > 0 && masterLocales.Count > MaxLocales)
        {
            errors.Add($"Error: item text groups support at most {MaxLocales} locales, the build has {masterLocales.Count}.");
        }
        foreach (var group in groups)
        {
            if (!group.NamesItems)
            {
                CheckNoReferences(group, errors);
                continue;
            }
            if (!group.IsWide)
            {
                errors.Add($"Error: item text group '{group.Name}' must also be a wide group (--wide-groups).");
            }
            CheckReferences(group, errors);
        }
        return errors;
    }

    private static void CheckRequestedGroups(IReadOnlyList<ResourceGroup> groups, IReadOnlySet<string> requestedGroups,
                                             List<string> errors)
    {
        foreach (var name in requestedGroups.Order(StringComparer.Ordinal))
        {
            if (!groups.Any(g => g.Name == name))
            {
                errors.Add($"Error: --item-text-groups names '{name}', which no .resx file has as its group " +
                           $"(the groups: {string.Join(", ", groups.Select(g => g.Name))}).");
            }
        }
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

    /// The game expands the references of the item text groups only.
    private static void CheckNoReferences(ResourceGroup group, List<string> errors)
    {
        foreach (var entry in group.Entries)
        {
            foreach (var (locale, text) in entry.Translations.OrderBy(t => t.Key, StringComparer.Ordinal))
            {
                if (ItemReferenceSyntax.HasReference(text))
                {
                    errors.Add($"Error: {group.Name}.{locale}.resx, key '{entry.Key}': \"{text}\" names an item " +
                               $"({ItemReferenceSyntax.Opening}...}}), but {group.Name} is no item text group " +
                               "(--item-text-groups), so the game would show the reference as written.");
                }
            }
        }
    }
}
