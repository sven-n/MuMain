using SkiaSharp;

namespace MuMain.Tools.InGameTests.Running;

/// <summary>
/// How much two pictures of one scene differ: per pixel, the largest difference of its three
/// colour channels (0 to 255). <see cref="Mean"/> averages it over the picture,
/// <see cref="ChangedPercent"/> is the share of pixels where it is above
/// <see cref="ChangedThreshold"/>, well above what JPEG compression alone changes.
/// </summary>
internal sealed record PictureDifference(double Mean, double ChangedPercent)
{
    public const int ChangedThreshold = 24;

    // The heat map shows a pixel's difference this many times brighter, so small changes show.
    private const int HeatGain = 4;
    private const int HeatMapQuality = 80;

    /// <summary>
    /// Compares two JPEGs of the same size; the heat map (a JPEG) is white where they differ.
    /// Null when either does not decode or their sizes differ.
    /// </summary>
    public static (PictureDifference Difference, byte[] HeatMap)? Compare(byte[] first, byte[] second)
    {
        using var a = SKBitmap.Decode(first);
        using var b = SKBitmap.Decode(second);
        if (a is null || b is null || a.Width != b.Width || a.Height != b.Height)
        {
            return null;
        }

        var pixelsA = a.Pixels;
        var pixelsB = b.Pixels;
        var heat = new SKColor[pixelsA.Length];
        double sum = 0;
        long changed = 0;
        for (var index = 0; index < pixelsA.Length; index++)
        {
            var difference = ChannelDifference(pixelsA[index], pixelsB[index]);
            sum += difference;
            if (difference > ChangedThreshold)
            {
                changed++;
            }

            var level = (byte)Math.Min(byte.MaxValue, difference * HeatGain);
            heat[index] = new SKColor(level, level, level);
        }

        var count = Math.Max(1, pixelsA.Length);
        return (new PictureDifference(sum / count, 100.0 * changed / count), EncodeHeatMap(a.Width, a.Height, heat));
    }

    private static int ChannelDifference(SKColor a, SKColor b)
        => Math.Max(Math.Abs(a.Red - b.Red), Math.Max(Math.Abs(a.Green - b.Green), Math.Abs(a.Blue - b.Blue)));

    private static byte[] EncodeHeatMap(int width, int height, SKColor[] pixels)
    {
        using var bitmap = new SKBitmap(width, height);
        bitmap.Pixels = pixels;
        using var image = SKImage.FromBitmap(bitmap);
        using var data = image.Encode(SKEncodedImageFormat.Jpeg, HeatMapQuality);
        return data.ToArray();
    }
}
