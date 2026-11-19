using System.Text.RegularExpressions;

namespace MuMain.Tools.ResxGen;

/// The item references texts of the item text groups may hold (docs/translation-system.md, "Texts that
/// name items"): {item:group,number}, {item:group,number+level}, {item:group,number:case} and
/// {item:group,number+level:case}. The game expands them with the item's name when it shows the text.
///
/// The grammar and the case keys must stay the same as the game's parser
/// (src/source/Data/Translation/ItemReference.cpp).
internal static partial class ItemReferenceSyntax
{
    /// Every reference starts with this; a text that has it must continue with a valid reference.
    public const string Opening = "{item:";

    /// The grammatical cases a reference may ask for.
    public static readonly IReadOnlyList<string> CaseKeys = ["gen", "dat", "acc", "ins", "loc"];

    // [0-9], not \d: .NET's \d takes every Unicode digit, the game's parser only ASCII ones.
    [GeneratedRegex(@"\G\{item:[0-9]{1,4},[0-9]{1,4}(\+[0-9]{1,4})?(:(?<case>[a-z]+))?\}", RegexOptions.CultureInvariant)]
    private static partial Regex Reference();

    /// True when the text holds at least one reference.
    public static bool HasReference(string text) => text.Contains(Opening, StringComparison.Ordinal);

    /// The references of `text` that do not follow the grammar, as written (cut at the next "}" or at 40
    /// characters).
    public static IReadOnlyList<string> FindMalformed(string text)
    {
        var malformed = new List<string>();
        for (var at = text.IndexOf(Opening, StringComparison.Ordinal); at >= 0;
             at = text.IndexOf(Opening, at + Opening.Length, StringComparison.Ordinal))
        {
            var match = Reference().Match(text, at);
            if (!match.Success || !IsKnownCase(match.Groups["case"]))
            {
                malformed.Add(Excerpt(text, at));
            }
        }
        return malformed;
    }

    private static bool IsKnownCase(Group caseGroup)
    {
        return !caseGroup.Success || CaseKeys.Contains(caseGroup.Value);
    }

    private static string Excerpt(string text, int at)
    {
        const int MaxLength = 40;
        var close = text.IndexOf('}', at);
        var end = close >= 0 ? Math.Min(close + 1, at + MaxLength) : Math.Min(text.Length, at + MaxLength);
        return text[at..end];
    }
}
