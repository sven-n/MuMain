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

    /// Render a C# string as a C++ UTF-8 string literal.
    public static string EscapeCppString(string s)
    {
        var sb = new StringBuilder(s.Length + 2);
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
