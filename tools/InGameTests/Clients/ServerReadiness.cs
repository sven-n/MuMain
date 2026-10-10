using System.Net.Sockets;

namespace MuMain.Tools.InGameTests.Clients;

/// <summary>Waits until a game server takes connections, e.g. a test server that was just recreated.</summary>
internal static class ServerReadiness
{
    private static readonly TimeSpan AttemptTimeout = TimeSpan.FromSeconds(2);
    private static readonly TimeSpan RetryInterval = TimeSpan.FromSeconds(1);

    /// <summary>
    /// Returns once the server at <paramref name="host"/>:<paramref name="port"/> greets a
    /// connection; false when it does not within <paramref name="timeout"/>.
    /// </summary>
    /// <remarks>
    /// An accepted connection alone proves nothing: a port forwarder (WSL, a container
    /// runtime) accepts it before the server listens, and drops it then, and a client
    /// that loses its connection this way ends. OpenMU's game server greets every new
    /// connection with a hello packet, so a first byte means the server is there.
    /// </remarks>
    public static async Task<bool> WaitAsync(string host, int port, TimeSpan timeout, CancellationToken cancellationToken)
    {
        var deadline = DateTime.UtcNow + timeout;
        while (true)
        {
            if (await GreetsAsync(host, port, cancellationToken))
            {
                return true;
            }

            if (DateTime.UtcNow >= deadline)
            {
                return false;
            }

            await Task.Delay(RetryInterval, cancellationToken);
        }
    }

    private static async Task<bool> GreetsAsync(string host, int port, CancellationToken cancellationToken)
    {
        using var attempt = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        attempt.CancelAfter(AttemptTimeout);
        try
        {
            using var client = new TcpClient();
            await client.ConnectAsync(host, port, attempt.Token);
            var buffer = new byte[1];
            return await client.GetStream().ReadAsync(buffer, attempt.Token) > 0;
        }
        catch (Exception exception) when (exception is SocketException or IOException or OperationCanceledException
                                          && !cancellationToken.IsCancellationRequested)
        {
            return false;
        }
    }
}
