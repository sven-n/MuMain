namespace ConstantsReplacer;

using System.Text;
using System.Text.RegularExpressions;

internal class Replacer
{
    private readonly Regex _constantsRegex = new(@"(\b.*\b)\s*=\s*(\b.*\b)", RegexOptions.Compiled);

    public async Task SearchAndReplaceAsync(string folder, string fileNamePattern, string searchPattern, string constantsString, CancellationToken cancellationToken = default, IProgress<(int Total, int Current)> progress = null)
    {
        cancellationToken.ThrowIfCancellationRequested();

        var constLookupTable = this._constantsRegex.Matches(constantsString)
            .ToDictionary(match => match.Groups[2].Value.Replace(" ", string.Empty), match => match.Groups[1].Value);

        var searchRegex = new Regex(searchPattern, RegexOptions.Compiled);
        var files = System.IO.Directory.GetFiles(folder, fileNamePattern, System.IO.SearchOption.AllDirectories);

        var koreanEncoding = Encoding.GetEncoding(51949);
        for (var index = 0; index < files.Length; index++)
        {
            cancellationToken.ThrowIfCancellationRequested();
            var file = files[index];
            progress?.Report((files.Length, index));
            var content = FileEncoding.TryLoadFile(file, koreanEncoding);
            var lastMatch = 0;
            while (searchRegex.Match(content, lastMatch) is { Success: true } match)
            {
                lastMatch = match.Index + match.Length;
                if (constLookupTable.TryGetValue(match.Groups[1].Value.Replace(" ", string.Empty), out var constant))
                {
                    content = content.Remove(match.Index, match.Length).Insert(match.Index, constant);
                    lastMatch = match.Index + constant.Length;
                }
            }

            if (lastMatch > 0)
            {
                await File.WriteAllTextAsync(file, content, Encoding.UTF8, cancellationToken); // let's always write in UTF8 with BOM.
            }
        }
    }
}