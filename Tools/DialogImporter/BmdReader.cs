using System.Text;

namespace MuMain.Tools.DialogImporter;

/// Reads and decrypts a Dialog_<lang>.bmd file into per-entry records.
///
/// Layout matches DIALOG_SCRIPT in src/source/Core/Globals/_struct.h:
///   char  m_lpszText[MAX_LENGTH_DIALOG]      (300 bytes)
///   int32 m_iNumAnswer                       (4 bytes)
///   int32 m_iLinkForAnswer[MAX_ANSWER_FOR_DIALOG]   (10*4 = 40 bytes)
///   int32 m_iReturnForAnswer[MAX_ANSWER_FOR_DIALOG] (10*4 = 40 bytes)
///   char  m_lpszAnswer[MAX_ANSWER_FOR_DIALOG][MAX_LENGTH_ANSWER] (10*64 = 640 bytes)
/// File contains MAX_DIALOG = 200 entries, total 200 * 1024 = 204800 bytes.
///
/// Bytes are XOR'd against the 3-byte BuxCode cycle (matches BuxConvert in
/// src/source/Core/Globals/_crypt.h).
internal static class BmdReader
{
    public const int MaxDialog = 200;
    public const int MaxAnswer = 10;

    private const int TextSize = 300;
    private const int AnswerSize = 64;
    private const int EntrySize = TextSize + 4 + 4 * MaxAnswer + 4 * MaxAnswer + AnswerSize * MaxAnswer;

    private static readonly byte[] BuxCode = [0xFC, 0xCF, 0xAB];

    public static IReadOnlyList<DialogRecord> Read(string path, Encoding textEncoding)
    {
        var raw = File.ReadAllBytes(path);
        if (raw.Length != EntrySize * MaxDialog)
        {
            throw new InvalidDataException(
                $"{path}: expected {EntrySize * MaxDialog} bytes, got {raw.Length}");
        }

        var entries = new List<DialogRecord>(MaxDialog);
        var entryBuf = new byte[EntrySize];
        for (var i = 0; i < MaxDialog; i++)
        {
            Buffer.BlockCopy(raw, i * EntrySize, entryBuf, 0, EntrySize);
            BuxDecode(entryBuf);
            entries.Add(ParseEntry(entryBuf, textEncoding));
        }
        return entries;
    }

    private static void BuxDecode(byte[] buf)
    {
        for (var i = 0; i < buf.Length; i++)
        {
            buf[i] ^= BuxCode[i % BuxCode.Length];
        }
    }

    private static DialogRecord ParseEntry(byte[] buf, Encoding textEncoding)
    {
        var off = 0;
        var text = ReadCString(buf, off, TextSize, textEncoding);
        off += TextSize;

        var numAnswer = BitConverter.ToInt32(buf, off);
        off += 4;

        var links = new int[MaxAnswer];
        for (var i = 0; i < MaxAnswer; i++)
        {
            links[i] = BitConverter.ToInt32(buf, off);
            off += 4;
        }

        var returns = new int[MaxAnswer];
        for (var i = 0; i < MaxAnswer; i++)
        {
            returns[i] = BitConverter.ToInt32(buf, off);
            off += 4;
        }

        var answers = new string[MaxAnswer];
        for (var i = 0; i < MaxAnswer; i++)
        {
            answers[i] = ReadCString(buf, off, AnswerSize, textEncoding);
            off += AnswerSize;
        }

        return new DialogRecord(text, numAnswer, links, returns, answers);
    }

    /// Reads a NUL-terminated string from a fixed-size slot using the
    /// provided encoding. The bmd source format is a fixed-width char buffer
    /// — the encoding is determined per-language by what the original tool
    /// chain wrote (English/UTF-8, Portuguese/Spanish/Windows-1252, etc.).
    private static string ReadCString(byte[] buf, int offset, int length, Encoding encoding)
    {
        var end = Array.IndexOf<byte>(buf, 0, offset, length);
        if (end < 0) end = offset + length;
        var slice = end - offset;
        if (slice == 0) return string.Empty;
        return encoding.GetString(buf, offset, slice);
    }
}

internal sealed record DialogRecord(
    string Text,
    int NumAnswer,
    int[] Links,
    int[] Returns,
    string[] Answers);
