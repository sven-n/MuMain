namespace MuMain.Tools.ResxGen;

/// One logical group (e.g. "Editor") with all its locales merged.
///
/// Each entry pairs the original resx key (which is the English text in
/// po-style) with the derived PascalCase C++ identifier and the per-locale
/// translations.
internal sealed record ResourceGroup(
    string Name,
    IReadOnlyList<string> Locales,
    IReadOnlyList<ResourceEntry> Entries);

internal sealed record ResourceEntry(
    string Key,                                          // resx name attribute, = English text
    string Identifier,                                   // PascalCase C++ identifier
    IReadOnlyDictionary<string, string> Translations);   // locale -> translated text
