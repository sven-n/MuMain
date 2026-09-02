using System.Text.RegularExpressions;
using System.Xml;
using System.Xml.Xsl;

if (args.Length != 3)
{
    throw new ArgumentException("Expected the binding XSLT, function XSLT, and native source paths.");
}

const string packetDefinition = """
    <PacketDefinitions xmlns="http://www.munique.net/OpenMU/PacketDefinitions">
      <Packets>
        <Packet>
          <Name>Probe</Name>
          <Fields />
        </Packet>
      </Packets>
    </PacketDefinitions>
    """;

string Transform(string path)
{
    var transform = new XslCompiledTransform();
    transform.Load(path, XsltSettings.Default, new XmlUrlResolver());

    var arguments = new XsltArgumentList();
    arguments.AddParam("subNamespace", string.Empty, "Probe");

    using var input = XmlReader.Create(new StringReader(packetDefinition));
    using var output = new StringWriter();
    transform.Transform(input, arguments, output);
    return output.ToString();
}

var bindings = Transform(args[0]);
if (bindings.Contains("dotnet_SendProbe", StringComparison.Ordinal))
{
    throw new InvalidOperationException($"Generated binding has namespace-scope state:\n{bindings}");
}

var functions = Transform(args[1]);
const string lazyBinding =
    "static const auto dotnet_SendProbe = LoadManagedSymbol<::SendProbe>(\"SendProbe\");";
if (!functions.Contains(lazyBinding, StringComparison.Ordinal))
{
    throw new InvalidOperationException($"Generated function does not resolve lazily:\n{functions}");
}

if (!functions.Contains("if (!dotnet_SendProbe)", StringComparison.Ordinal))
{
    throw new InvalidOperationException($"Generated function does not guard a missing symbol:\n{functions}");
}

var namespaceScopeLookup = new Regex(
    @"^[A-Za-z_][^\r\n;=]*\s+[A-Za-z_]\w*\s*=\s*(?:\r?\n\s*)?LoadManagedSymbol<",
    RegexOptions.Multiline);
foreach (var sourcePath in Directory.EnumerateFiles(args[2], "*.cpp"))
{
    var source = File.ReadAllText(sourcePath);
    if (namespaceScopeLookup.IsMatch(source))
    {
        throw new InvalidOperationException($"Native source performs namespace-scope symbol lookup: {sourcePath}");
    }
}
