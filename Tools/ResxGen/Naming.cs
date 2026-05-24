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
    /// Word boundaries are any run of non-alphanumeric characters. Only
    /// ASCII letters and digits are kept: MSVC rejects non-ASCII Unicode
    /// letters in C++ identifiers, and the original bmd has Latin-1-mangled
    /// Korean leftovers that would slug to identifiers full of garbage
    /// codepoints. Strings that contain no ASCII alnum at all (typically
    /// those dead Korean entries) return an empty string, signalling the
    /// caller that this entry has no usable identifier and should be
    /// dropped.
    /// The first letter of each word is forced upper-case; everything else
    /// preserves its original case so PascalCase content in the source
    /// ("BlessAegis") survives.
    public static string ToIdentifier(string key)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(key);

        var sb = new StringBuilder(key.Length);
        var atWordStart = true;
        foreach (var c in key)
        {
            var isAsciiAlnum = c < 0x80 && char.IsLetterOrDigit(c);
            if (isAsciiAlnum)
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
            return string.Empty;
        }

        // Single-letter identifiers are ambiguous: "%s" legitimately slugs
        // to "S", but so does a Korean-Latin1-garbled string with a single
        // embedded "%s". Distinguish by source: if the input was pure ASCII
        // (e.g. "%s"), keep the short identifier; if it contained any
        // non-ASCII codepoint, treat the slug as nothing-survived and drop.
        if (sb.Length < 2)
        {
            foreach (var c in key)
            {
                if (c >= 0x80) return string.Empty;
            }
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
    /// whose accessors return const wchar_t*); otherwise a plain narrow literal.
    ///
    /// Wide literals emit non-ASCII as \uHHHH / \UHHHHHHHH universal-character
    /// names — wchar_t is UTF-16 on Windows and the compiler always encodes
    /// UCNs in the wide execution character set, so this is unambiguous.
    ///
    /// Narrow literals emit non-ASCII as three-digit octal escapes (\NNN) of
    /// the UTF-8 byte sequence, NOT as \uHHHH. The reason: in a narrow literal
    /// the compiler encodes \uHHHH using the execution character set, which on
    /// MSVC defaults to the system ANSI codepage unless /utf-8 is set. That
    /// makes the produced bytes depend on the build host's locale. Octal byte
    /// escapes are codepage-independent — they emit the literal bytes verbatim,
    /// giving valid UTF-8 in every build.
    public static string EscapeCppString(string s, bool wide = false)
    {
        var sb = new StringBuilder(s.Length + 3);
        if (wide) sb.Append('L');
        sb.Append('"');
        for (var i = 0; i < s.Length; i++)
        {
            var c = s[i];
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
                    else if (c < 0x80)
                    {
                        sb.Append(c);
                    }
                    else if (char.IsHighSurrogate(c) && i + 1 < s.Length && char.IsLowSurrogate(s[i + 1]))
                    {
                        var codepoint = char.ConvertToUtf32(c, s[i + 1]);
                        if (wide)
                        {
                            // Supplementary plane: 8-digit UCN. Wide literals
                            // encode this as the matching UTF-16 surrogate pair.
                            sb.Append(CultureInfo.InvariantCulture, $"\\U{codepoint:x8}");
                        }
                        else
                        {
                            AppendUtf8Octal(sb, codepoint);
                        }
                        i++;
                    }
                    else if (wide)
                    {
                        sb.Append(CultureInfo.InvariantCulture, $"\\u{(int)c:x4}");
                    }
                    else
                    {
                        AppendUtf8Octal(sb, c);
                    }
                    break;
            }
        }
        sb.Append('"');
        return sb.ToString();
    }

    /// Emit a codepoint as one to four 3-digit octal escapes carrying its
    /// UTF-8 byte sequence. Three digits per byte are always written so that
    /// any following octal digit in the source stays unambiguous (\NNN
    /// consumes at most 3 octal digits).
    private static void AppendUtf8Octal(StringBuilder sb, int codepoint)
    {
        Span<byte> bytes = stackalloc byte[4];
        var n = 0;
        if (codepoint < 0x80)
        {
            bytes[n++] = (byte)codepoint;
        }
        else if (codepoint < 0x800)
        {
            bytes[n++] = (byte)(0xC0 | (codepoint >> 6));
            bytes[n++] = (byte)(0x80 | (codepoint & 0x3F));
        }
        else if (codepoint < 0x10000)
        {
            bytes[n++] = (byte)(0xE0 | (codepoint >> 12));
            bytes[n++] = (byte)(0x80 | ((codepoint >> 6) & 0x3F));
            bytes[n++] = (byte)(0x80 | (codepoint & 0x3F));
        }
        else
        {
            bytes[n++] = (byte)(0xF0 | (codepoint >> 18));
            bytes[n++] = (byte)(0x80 | ((codepoint >> 12) & 0x3F));
            bytes[n++] = (byte)(0x80 | ((codepoint >> 6) & 0x3F));
            bytes[n++] = (byte)(0x80 | (codepoint & 0x3F));
        }
        for (var i = 0; i < n; i++)
        {
            var b = bytes[i];
            sb.Append('\\');
            sb.Append((char)('0' + ((b >> 6) & 0x7)));
            sb.Append((char)('0' + ((b >> 3) & 0x7)));
            sb.Append((char)('0' + (b & 0x7)));
        }
    }
}
