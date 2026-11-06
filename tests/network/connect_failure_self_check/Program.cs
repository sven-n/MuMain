using System.Net.Sockets;
using MUnique.Client.Library;

// ConnectFailure.Describe, which writes a failed connect into MuError.log: socket errors by their code (their message
// names the address), anything else with its message.
internal static class Program
{
    private const string Address = "203.0.113.7";

    private static void Main()
    {
        var refused = new SocketException((int)SocketError.ConnectionRefused);

        // The error code is the platform's (10061 on Windows, 111 on Linux), so only its start is fixed.
        Expect(ConnectFailure.Describe(refused), "SocketException ConnectionRefused (");
        Expect(ConnectFailure.Describe(new IOException($"Unable to connect to {Address}:44405", refused)),
               "SocketException ConnectionRefused (");
        // The runtime's own text, which its versions may word differently.
        var portOutOfRange = new ArgumentOutOfRangeException("port");
        Expect(ConnectFailure.Describe(portOutOfRange), $"ArgumentOutOfRangeException: {portOutOfRange.Message}");
        Expect(ConnectFailure.Describe(new InvalidOperationException("Connection handle 3 already exists.")),
               "InvalidOperationException: Connection handle 3 already exists.");
    }

    private static void Expect(string description, string start)
    {
        if (!description.StartsWith(start, StringComparison.Ordinal) || description.Contains(Address))
        {
            throw new InvalidOperationException($"Connect failure described as \"{description}\", expected \"{start}...\"");
        }
    }
}
