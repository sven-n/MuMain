using System.Globalization;
using System.Text;

namespace MuMain.Tools.ResxGen;

/// Helpers for turning resx keys and locale codes into valid C++ identifiers.
internal static class Naming
{
    /// Maximum length of a generated C++ identifier. Hit this and the
    /// contributor should pick an explicit short key instead of a long
    /// English sentence.
    public const int MaxIdentifierLength = 64;

    /// Slug a free-form key (e.g. "Save Skills", "Are you sure?", "BlessAegis Mining Lab")
    /// into a PascalCase C++ identifier: "SaveSkills", "AreYouSure", "BlessAegisMiningLab".
    ///
    /// Word boundaries are any run of non-alphanumeric characters. The first
    /// letter of each word is forced upper-case; everything else preserves its
    /// original case so PascalCase content in the source ("BlessAegis") survives.
    /// Empty input throws.
    public static string ToIdentifier(string key)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(key);

        var sb = new StringBuilder(key.Length);
        var atWordStart = true;
        foreach (var c in key)
        {
            if (char.IsLetterOrDigit(c))
            {
                sb.Append(atWordStart
                    ? char.ToUpper(c, CultureInfo.InvariantCulture)
                    : c);
                atWordStart = false;
            }
            else
            {
                atWordStart = true;
            }
        }

        if (sb.Length == 0)
        {
            throw new ArgumentException($"Key '{key}' produces an empty identifier.", nameof(key));
        }

        if (char.IsDigit(sb[0]))
        {
            sb.Insert(0, '_');
        }

        return sb.ToString();
    }

    /// Locale codes may contain hyphens (en-US); C++ identifiers cannot.
    public static string SanitizeLocale(string locale) => locale.Replace('-', '_');

    /// MSVC's swprintf treats a bare "%s" inside a wide format string as a
    /// NARROW-string argument, so feeding it a wchar_t* renders only the
    /// first byte (the first letter, then a 0x00 from UTF-16 looks like a
    /// null terminator). Walk the format string and rewrite "%s"/"%S" to
    /// "%ls" whenever there is no explicit length modifier already, matching
    /// the legacy GlobalText loader's NormalizePrintfSpecifiers behaviour.
    /// Only applied to values that will be emitted in wide groups.
    public static string NormalizePrintfSpecifiersForWide(string s)
    {
        const string LengthModifiers = "hlLjztI";
        const string Conversions = "cCdiouxXeEfgGaAnpsS";

        var sb = new StringBuilder(s.Length + 8);
        var inFormat = false;
        var hasLengthModifier = false;
        foreach (var ch in s)
        {
            if (!inFormat)
            {
                if (ch == '%')
                {
                    inFormat = true;
                    hasLengthModifier = false;
                }
                sb.Append(ch);
                continue;
            }

            if (ch == '%')
            {
                // "%%" - escaped literal, not a conversion.
                inFormat = false;
                sb.Append('%');
                continue;
            }

            if (LengthModifiers.IndexOf(ch) >= 0)
            {
                hasLengthModifier = true;
                sb.Append(ch);
                continue;
            }

            if (Conversions.IndexOf(ch) >= 0)
            {
                if ((ch == 's' || ch == 'S') && !hasLengthModifier)
                {
                    sb.Append('l');
                    sb.Append('s');
                }
                else
                {
                    sb.Append(ch == 'S' ? 's' : ch);
                }
                inFormat = false;
                continue;
            }

            sb.Append(ch);
        }
        return sb.ToString();
    }

    /// Render a C# string as a C++ string literal.
    /// `wide=true` produces an L"..." UTF-16 wide literal (used by wide groups
    /// whose accessors return const wchar_t*); otherwise a plain UTF-8 literal.
    /// Non-ASCII characters pass through as-is (UTF-8 source); the compiler is
    /// responsible for translating them to the appropriate execution charset.
    public static string EscapeCppString(string s, bool wide = false)
    {
        var sb = new StringBuilder(s.Length + 3);
        if (wide) sb.Append('L');
        sb.Append('"');
        foreach (var c in s)
        {
            switch (c)
            {
                case '\\': sb.Append("\\\\"); break;
                case '"': sb.Append("\\\""); break;
                case '\n': sb.Append("\\n"); break;
                case '\r': sb.Append("\\r"); break;
                case '\t': sb.Append("\\t"); break;
                case '\0': sb.Append("\\0"); break;
                default:
                    if (c < 0x20)
                    {
                        sb.Append(CultureInfo.InvariantCulture, $"\\x{(int)c:x2}");
                    }
                    else
                    {
                        sb.Append(c);
                    }
                    break;
            }
        }
        sb.Append('"');
        return sb.ToString();
    }
}
