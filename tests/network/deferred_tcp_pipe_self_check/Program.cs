using System.Buffers;
using System.Diagnostics;
using System.IO.Pipelines;
using System.Net;
using System.Net.Sockets;
using System.Text;
using Microsoft.Extensions.Logging.Abstractions;
using MUnique.Client.Library;
using MUnique.OpenMU.Network;
using MUnique.OpenMU.Network.SimpleModulus;
using MUnique.OpenMU.Network.Xor;

// DeferredTcpPipe, the transport under every game connection: it is written to before the socket opens, opens or
// fails in the background, and sends what is queued before it closes.
internal static class Program
{
    private static readonly TimeSpan Wait = TimeSpan.FromSeconds(5);

    private static async Task Main()
    {
        await WritesBeforeOpenGoOutInOrder();
        await ClosingSendsWhatIsQueued();
        await ClosingAnEncryptedConnectionSendsItsLastPacket();
        await ServerCloseEndsInput();
        await RefusedConnectFails();
        await ConnectTimesOut();
        await DisposeWhileConnectingNeverOpens();
        GameSocketOptions();
        AddressFamiliesAlternate();
        await LocalhostReachesAnIPv4OnlyServerQuickly();
        await RefusedOnEveryAddressFailsByCode();
        await ClosingWhileThePeerStopsReadingLetsGo();
    }

    // A server that stopped reading keeps the send pending forever; the close must still give the socket up.
    private static async Task ClosingWhileThePeerStopsReadingLetsGo()
    {
        var listener = new TcpListener(IPAddress.Loopback, 0);
        listener.Server.ReceiveBufferSize = 1024;
        listener.Start();
        using var stop = listener.Server;
        var pipe = new DeferredTcpPipe();
        var events = new Events();
        pipe.Start("127.0.0.1", Port(listener), Wait, events.Opened, events.Failed);
        using var server = await listener.AcceptSocketAsync().WaitAsync(Wait);
        await events.OpenedTask.WaitAsync(Wait);
        // Writes until everything between here and the server is full, however much the system buffers.
        var chunk = new byte[1024];
        Task? stalled = null;
        for (var i = 0; i < 16384 && stalled is null; i++)
        {
            var write = pipe.Output.WriteAsync(chunk).AsTask();
            if (await Task.WhenAny(write, Task.Delay(500)) != write)
            {
                stalled = write;
            }
        }

        Expect(stalled is not null, "the writes fill the buffers of a server that doesn't read");
        await pipe.Output.CompleteAsync();
        pipe.Dispose();

        // Input ends once the socket is closed; nothing else here would end it.
        var watch = Stopwatch.StartNew();
        try
        {
            while (true)
            {
                var result = await pipe.Input.ReadAsync().AsTask().WaitAsync(TimeSpan.FromSeconds(10));
                pipe.Input.AdvanceTo(result.Buffer.End);
                if (result.IsCompleted)
                {
                    break;
                }
            }
        }
        catch (Exception ex) when (ex is not TimeoutException)
        {
        }

        Expect(watch.Elapsed < TimeSpan.FromSeconds(8), $"a close gives the socket up although the server stopped reading (took {watch.ElapsedMilliseconds} ms)");
    }

    private static async Task WritesBeforeOpenGoOutInOrder()
    {
        using var listener = Listen();
        using var pipe = new DeferredTcpPipe();
        var events = new Events();
        await WriteAsync(pipe, "early;");
        pipe.Start("127.0.0.1", Port(listener), Wait, events.Opened, events.Failed);
        using var server = await listener.AcceptSocketAsync().WaitAsync(Wait);
        await events.OpenedTask.WaitAsync(Wait);
        await WriteAsync(pipe, "late;");
        Expect(await ReceiveAsync(server, "early;late;".Length) == "early;late;", "bytes written before open go out first");

        server.Send(Encoding.ASCII.GetBytes("hello"));
        var result = await pipe.Input.ReadAtLeastAsync(5).AsTask().WaitAsync(Wait);
        Expect(Encoding.ASCII.GetString(result.Buffer.Slice(0, 5).ToArray()) == "hello", "the server's bytes reach Input");
        pipe.Input.AdvanceTo(result.Buffer.GetPosition(5));
    }

    private static async Task ClosingSendsWhatIsQueued()
    {
        using var listener = Listen();
        var pipe = new DeferredTcpPipe();
        var events = new Events();
        pipe.Start("127.0.0.1", Port(listener), Wait, events.Opened, events.Failed);
        using var server = await listener.AcceptSocketAsync().WaitAsync(Wait);
        await events.OpenedTask.WaitAsync(Wait);
        await WriteAsync(pipe, "logout");
        await pipe.Output.CompleteAsync(); // the writer closes the connection, as Connection.DisconnectAsync does
        pipe.Dispose();
        Expect(await ReceiveToEndAsync(server) == "logout", "a close sends what was queued, then ends cleanly");
    }

    // The game's close: the last packet goes through the encryptors, then the connection disconnects at once (the
    // logout on quit). Repeated, since the encryptors run on other threads.
    private static async Task ClosingAnEncryptedConnectionSendsItsLastPacket()
    {
        byte[] logout = [0xC1, 0x05, 0xF1, 0x02, 0x00];
        for (var round = 0; round < 30; round++)
        {
            using var listener = Listen();
            var pipe = new DeferredTcpPipe();
            var events = new Events();
            var encryptor = new PipelinedXor32Encryptor(new PipelinedSimpleModulusEncryptor(pipe.Output, PipelinedSimpleModulusEncryptor.DefaultClientKey).Writer);
            var connection = new Connection(pipe, null, encryptor, new NullLogger<Connection>());
            pipe.Start("127.0.0.1", Port(listener), Wait, events.Opened, events.Failed);
            using var server = await listener.AcceptSocketAsync().WaitAsync(Wait);
            await events.OpenedTask.WaitAsync(Wait);

            logout.CopyTo(connection.Output.GetSpan(logout.Length));
            connection.Output.Advance(logout.Length);
            await connection.Output.FlushAsync();
            await connection.DisconnectAsync();

            var received = await ReceiveToEndAsync(server);
            Expect(received.Length == logout.Length, $"the last packet before a close goes out (round {round}, got {received.Length} bytes)");
        }
    }

    private static async Task ServerCloseEndsInput()
    {
        using var listener = Listen();
        using var pipe = new DeferredTcpPipe();
        var events = new Events();
        pipe.Start("127.0.0.1", Port(listener), Wait, events.Opened, events.Failed);
        using (var server = await listener.AcceptSocketAsync().WaitAsync(Wait))
        {
            await events.OpenedTask.WaitAsync(Wait);
            server.Shutdown(SocketShutdown.Both);
        }

        var result = await pipe.Input.ReadAsync().AsTask().WaitAsync(Wait);
        Expect(result.IsCompleted, "the server's close ends Input");
    }

    private static async Task RefusedConnectFails()
    {
        int port;
        using (var listener = Listen())
        {
            port = Port(listener);
        }

        using var pipe = new DeferredTcpPipe();
        var events = new Events();
        await WriteAsync(pipe, "never sent");
        pipe.Start("127.0.0.1", port, Wait, events.Opened, events.Failed);
        var error = await events.FailedTask.WaitAsync(Wait * 2);
        Expect(ConnectFailure.Describe(error).StartsWith("SocketException ConnectionRefused", StringComparison.Ordinal),
               $"a refused connect fails as refused, not \"{ConnectFailure.Describe(error)}\"");
        Expect(!events.OpenedTask.IsCompleted, "a refused connect never opens");
        await ExpectInputFailsAsync(pipe);
    }

    private static async Task ConnectTimesOut()
    {
        using var pipe = new DeferredTcpPipe();
        var events = new Events();
        var watch = Stopwatch.StartNew();
        // A non-routable address: no answer at all, so only the timeout ends the attempt (or the OS, if it has no route).
        pipe.Start("10.255.255.1", 44405, TimeSpan.FromMilliseconds(300), events.Opened, events.Failed);
        var error = await events.FailedTask.WaitAsync(Wait);
        Expect(watch.Elapsed < TimeSpan.FromSeconds(3), $"a connect gives up within its timeout, took {watch.Elapsed}");
        Expect(error is SocketException, $"a connect that gives up fails with a socket error, not {error.GetType().Name}");
        await ExpectInputFailsAsync(pipe);
    }

    private static async Task DisposeWhileConnectingNeverOpens()
    {
        var pipe = new DeferredTcpPipe();
        var events = new Events();
        pipe.Start("10.255.255.1", 44405, Wait, events.Opened, events.Failed);
        pipe.Dispose();
        await Task.WhenAny(events.FailedTask, Task.Delay(Wait));
        Expect(events.FailedTask.IsCompleted, "a disposed connect ends");
        Expect(!events.OpenedTask.IsCompleted, "a disposed connect never opens");
    }

    private static void GameSocketOptions()
    {
        using var listener = Listen();
        using var socket = new Socket(SocketType.Stream, ProtocolType.Tcp);
        socket.Connect(IPAddress.Loopback, Port(listener));
        DeferredTcpPipe.ConfigureSocket(socket);
        Expect(socket.NoDelay, "small packets go out at once (NoDelay)");
        Expect((int)socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.KeepAlive)! != 0, "keep-alive is on");
    }

    private static void AddressFamiliesAlternate()
    {
        var v6a = IPAddress.Parse("::1");
        var v6b = IPAddress.Parse("fe80::1");
        var v4a = IPAddress.Parse("127.0.0.1");
        var v4b = IPAddress.Parse("10.0.0.1");
        Expect(DeferredTcpPipe.Interleave([v6a, v6b, v4a]).SequenceEqual([v6a, v4a, v6b]), "IPv6 first, then alternating");
        Expect(DeferredTcpPipe.Interleave([v4a, v4b, v6a]).SequenceEqual([v4a, v6a, v4b]), "the resolver's first family leads");
        Expect(DeferredTcpPipe.Interleave([]).Count == 0, "no address, nothing to try");
    }

    private static async Task LocalhostReachesAnIPv4OnlyServerQuickly()
    {
        // localhost resolves to ::1 first; nothing listens there, and Windows retries a refused connect for about 2 s.
        using var listener = Listen();
        using var pipe = new DeferredTcpPipe();
        var events = new Events();
        var watch = Stopwatch.StartNew();
        pipe.Start("localhost", Port(listener), Wait, events.Opened, events.Failed);
        await Task.WhenAny(events.OpenedTask, events.FailedTask).WaitAsync(Wait);
        Expect(events.OpenedTask.IsCompleted, "localhost reaches a server listening on IPv4 only");
        Expect(watch.Elapsed < TimeSpan.FromSeconds(1), $"localhost opens without waiting out IPv6, took {watch.Elapsed}");
    }

    private static async Task RefusedOnEveryAddressFailsByCode()
    {
        int port;
        using (var listener = Listen())
        {
            port = Port(listener);
        }

        using var pipe = new DeferredTcpPipe();
        var events = new Events();
        pipe.Start("localhost", port, Wait, events.Opened, events.Failed);
        var error = await events.FailedTask.WaitAsync(Wait * 2);
        Expect(ConnectFailure.Describe(error).StartsWith("SocketException ", StringComparison.Ordinal),
               $"a connect refused on every address is described by its code, not \"{ConnectFailure.Describe(error)}\"");
    }

    private static TcpListener Listen()
    {
        var listener = new TcpListener(IPAddress.Loopback, 0);
        listener.Start();
        return listener;
    }

    private static int Port(TcpListener listener) => ((IPEndPoint)listener.LocalEndpoint).Port;

    private static async Task WriteAsync(DeferredTcpPipe pipe, string text)
    {
        await pipe.Output.WriteAsync(Encoding.ASCII.GetBytes(text));
    }

    private static async Task<string> ReceiveAsync(Socket socket, int count)
    {
        var buffer = new byte[count];
        var received = 0;
        while (received < count)
        {
            var read = await socket.ReceiveAsync(buffer.AsMemory(received)).AsTask().WaitAsync(Wait);
            Expect(read > 0, "the connection stays open while bytes are expected");
            received += read;
        }

        return Encoding.ASCII.GetString(buffer);
    }

    private static async Task<string> ReceiveToEndAsync(Socket socket)
    {
        var text = new StringBuilder();
        var buffer = new byte[256];
        while (true)
        {
            var read = await socket.ReceiveAsync(buffer, SocketFlags.None).WaitAsync(Wait);
            if (read == 0)
            {
                return text.ToString();
            }

            text.Append(Encoding.ASCII.GetString(buffer, 0, read));
        }
    }

    private static async Task ExpectInputFailsAsync(DeferredTcpPipe pipe)
    {
        try
        {
            var result = await pipe.Input.ReadAsync().AsTask().WaitAsync(Wait);
            Expect(false, $"Input ends with the connect's error, not a read (completed={result.IsCompleted})");
        }
        catch (Exception ex) when (ex is SocketException or OperationCanceledException)
        {
        }
    }

    private static void Expect(bool condition, string what)
    {
        if (!condition)
        {
            throw new InvalidOperationException($"DeferredTcpPipe: expected that {what}");
        }
    }

    private sealed class Events
    {
        private readonly TaskCompletionSource _opened = new(TaskCreationOptions.RunContinuationsAsynchronously);
        private readonly TaskCompletionSource<Exception> _failed = new(TaskCreationOptions.RunContinuationsAsynchronously);

        public Task OpenedTask => this._opened.Task;

        public Task<Exception> FailedTask => this._failed.Task;

        public void Opened() => this._opened.TrySetResult();

        public void Failed(Exception error) => this._failed.TrySetResult(error);
    }
}
