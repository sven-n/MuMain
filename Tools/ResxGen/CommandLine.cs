namespace MuMain.Tools.ResxGen;

internal sealed record CommandLineOptions(
    string InputDir,
    string OutputDir,
    IReadOnlySet<string> WideGroups);

internal static class CommandLine
{
    public const string Usage =
        "Usage: ResxGen --input <dir> --output <dir> [--wide-groups <CommaSeparatedList>]";

    public static bool TryParse(string[] args, out CommandLineOptions options, out string error)
    {
        string? inputDir = null;
        string? outputDir = null;
        string? wideGroupsRaw = null;

        for (var i = 0; i < args.Length; i++)
        {
            switch (args[i])
            {
                case "--input":
                    if (!TryReadValue(args, ref i, out inputDir, out error))
                    {
                        options = default!;
                        return false;
                    }
                    break;

                case "--output":
                    if (!TryReadValue(args, ref i, out outputDir, out error))
                    {
                        options = default!;
                        return false;
                    }
                    break;

                case "--wide-groups":
                    if (!TryReadValue(args, ref i, out wideGroupsRaw, out error))
                    {
                        options = default!;
                        return false;
                    }
                    break;

                default:
                    options = default!;
                    error = $"Unknown argument: {args[i]}";
                    return false;
            }
        }

        if (inputDir is null)
        {
            options = default!;
            error = "Missing required argument: --input";
            return false;
        }

        if (outputDir is null)
        {
            options = default!;
            error = "Missing required argument: --output";
            return false;
        }

        var wideGroups = ParseGroupList(wideGroupsRaw);

        options = new CommandLineOptions(inputDir, outputDir, wideGroups);
        error = string.Empty;
        return true;
    }

    private static IReadOnlySet<string> ParseGroupList(string? raw)
    {
        var set = new HashSet<string>(StringComparer.Ordinal);
        if (string.IsNullOrWhiteSpace(raw)) return set;
        foreach (var part in raw.Split(',', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
        {
            set.Add(part);
        }
        return set;
    }

    private static bool TryReadValue(string[] args, ref int i, out string? value, out string error)
    {
        if (i + 1 >= args.Length)
        {
            value = null;
            error = $"Missing value for argument: {args[i]}";
            return false;
        }
        value = args[++i];
        error = string.Empty;
        return true;
    }
}
