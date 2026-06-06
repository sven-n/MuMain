using System.Text;
using System.Xml.Linq;

namespace MuMain.Tools.DialogImporter;

/// Emits Dialog.<locale>.resx files compatible with Tools/ResxGen.
///
/// Key scheme (predictable, conflict-free):
///   Text_<idx>             — NPC line for dialog index <idx>
///   Answer_<idx>_Slot_<n>  — reply label n for dialog index <idx>
///
/// Both slug to short C++ identifiers (Text0, Answer0Slot0, ...) via
/// ResxGen's Naming.ToIdentifier, so I18N::Dialog::Text0 is what callers
/// reference. The resx <comment>legacy_id=N</comment> sidecar binds each
/// key to its runtime index so I18N::Dialog::Lookup(...) can resolve it.
///
/// Legacy ID encoding (single namespace shared by texts and answers):
///   Text:                  legacy_id = idx                 (range 0..199)
///   Answer (idx, slot):    legacy_id = 1000 + idx*10 + slot (range 1000..2999)
internal static class ResxWriter
{
    public const int AnswerLegacyIdBase = 1000;
    public const int AnswerLegacyIdStride = 10;

    private static readonly XNamespace XmlNs = "http://www.w3.org/XML/1998/namespace";

    public static int Write(string outputPath, string locale, IReadOnlyList<DialogRecord> entries)
    {
        var root = new XElement("root");
        AppendResxSchemaHeader(root);

        var count = 0;
        for (var idx = 0; idx < entries.Count; idx++)
        {
            var entry = entries[idx];
            if (string.IsNullOrEmpty(entry.Text) && entry.NumAnswer == 0)
            {
                continue;
            }

            if (!string.IsNullOrEmpty(entry.Text))
            {
                AppendDataElement(root, $"Text_{idx}", entry.Text, $"legacy_id={idx}");
                count++;
            }

            for (var slot = 0; slot < entry.NumAnswer; slot++)
            {
                var answer = entry.Answers[slot];
                if (string.IsNullOrEmpty(answer)) continue;
                var legacyId = AnswerLegacyIdBase + idx * AnswerLegacyIdStride + slot;
                AppendDataElement(root, $"Answer_{idx}_Slot_{slot}", answer, $"legacy_id={legacyId}");
                count++;
            }
        }

        var doc = new XDocument(new XDeclaration("1.0", "utf-8", null), root);
        var settings = new System.Xml.XmlWriterSettings
        {
            Indent = true,
            IndentChars = "  ",
            Encoding = new UTF8Encoding(false),
            OmitXmlDeclaration = false,
        };
        using var writer = System.Xml.XmlWriter.Create(outputPath, settings);
        doc.Save(writer);
        return count;
    }

    private static void AppendDataElement(XElement root, string name, string value, string? comment)
    {
        var data = new XElement(
            "data",
            new XAttribute("name", name),
            new XAttribute(XmlNs + "space", "preserve"),
            new XElement("value", value));
        if (!string.IsNullOrEmpty(comment))
        {
            data.Add(new XElement("comment", comment));
        }
        root.Add(data);
    }

    private static void AppendResxSchemaHeader(XElement root)
    {
        root.Add(new XElement("resheader",
            new XAttribute("name", "resmimetype"),
            new XElement("value", "text/microsoft-resx")));
        root.Add(new XElement("resheader",
            new XAttribute("name", "version"),
            new XElement("value", "2.0")));
        root.Add(new XElement("resheader",
            new XAttribute("name", "reader"),
            new XElement("value", "System.Resources.ResXResourceReader, System.Windows.Forms, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089")));
        root.Add(new XElement("resheader",
            new XAttribute("name", "writer"),
            new XElement("value", "System.Resources.ResXResourceWriter, System.Windows.Forms, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089")));
    }
}
