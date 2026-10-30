namespace MuMain.Tools.ResxGen;

/// One logical group (e.g. "Editor") with all its locales merged.
///
/// Each entry pairs the original resx key (which is the English text in
/// po-style) with the derived PascalCase C++ identifier and the per-locale
/// translations.
///
/// `IsWide` flips emission from const char* / "..." to const wchar_t* / L"...";
/// it is opt-in per group via the --wide-groups CLI flag.
internal sealed record ResourceGroup(
    string Name,
    IReadOnlyList<string> Locales,
    IReadOnlyList<ResourceEntry> Entries,
    bool IsWide = false);

internal sealed record ResourceEntry(
    string Key,                                          // resx name attribute, = English text
    string Identifier,                                   // PascalCase C++ identifier
    IReadOnlyDictionary<string, string> Translations);   // locale -> translated text
