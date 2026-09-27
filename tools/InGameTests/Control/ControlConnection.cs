using System.Net.Sockets;
using System.Text;
using System.Text.Json;
using System.Text.Json.Nodes;

namespace MuMain.Tools.InGameTests.Control;

/// <summary>
/// One connection to a client's control socket (docs/control-socket.md):
/// newline-delimited JSON, one request at a time, each answered by the line
/// that echoes its <c>id</c>.
/// </summary>
internal sealed class ControlConnection : IAsyncDisposable
{
    private readonly Socket socket;
    private readonly StreamReader reader;
    private readonly StreamWriter writer;
    private readonly SemaphoreSlim gate = new(1, 1);
    private int nextId;

    private ControlConnection(Socket socket)
    {
        this.socket = socket;
        var stream = new NetworkStream(socket, ownsSocket: false);
        this.reader = new StreamReader(stream, new UTF8Encoding(false));
        this.writer = new StreamWriter(stream, new UTF8Encoding(false)) { AutoFlush = true, NewLine = "\n" };
    }

    /// <summary>Connects to the socket at <paramref name="path"/>.</summary>
    public static async Task<ControlConnection> ConnectAsync(string path, CancellationToken cancellationToken)
    {
        var socket = new Socket(AddressFamily.Unix, SocketType.Stream, ProtocolType.Unspecified);
        try
        {
            await socket.ConnectAsync(new UnixDomainSocketEndPoint(path), cancellationToken);
            return new ControlConnection(socket);
        }
        catch
        {
            socket.Dispose();
            throw;
        }
    }

    /// <summary>
    /// Sends <paramref name="command"/> with <paramref name="fields"/> and returns the
    /// response's <c>result</c>. Throws <see cref="ControlException"/> for an error answer
    /// and <see cref="TimeoutException"/> when no answer arrives in time.
    /// </summary>
    public async Task<JsonElement> SendAsync(string command, object? fields, TimeSpan timeout)
    {
        await this.gate.WaitAsync();
        try
        {
            var id = Interlocked.Increment(ref this.nextId);
            var request = fields is null ? new JsonObject() : JsonSerializer.SerializeToNode(fields)!.AsObject();
            request["cmd"] = command;
            request["id"] = id;
            await this.writer.WriteLineAsync(request.ToJsonString());

            using var timeoutSource = new CancellationTokenSource(timeout);
            return await this.ReadAnswerAsync(command, id, timeoutSource.Token);
        }
        catch (OperationCanceledException)
        {
            throw new TimeoutException($"`{command}` got no answer within {timeout.TotalSeconds:0} s");
        }
        finally
        {
            this.gate.Release();
        }
    }

    public async ValueTask DisposeAsync()
    {
        await this.writer.DisposeAsync();
        this.reader.Dispose();
        this.socket.Dispose();
        this.gate.Dispose();
    }

    private async Task<JsonElement> ReadAnswerAsync(string command, int id, CancellationToken cancellationToken)
    {
        while (true)
        {
            var line = await this.reader.ReadLineAsync(cancellationToken)
                       ?? throw new IOException($"the client closed the control socket during `{command}`");
            using var document = JsonDocument.Parse(line);
            var answer = document.RootElement;
            if (!answer.TryGetProperty("id", out var answerId) || answerId.GetInt32() != id)
            {
                continue;
            }

            if (answer.GetProperty("ok").GetBoolean())
            {
                return answer.TryGetProperty("result", out var result) ? result.Clone() : default;
            }

            throw new ControlException(
                command,
                answer.GetProperty("error").GetString() ?? "failed",
                answer.TryGetProperty("message", out var message) ? message.GetString() ?? string.Empty : string.Empty);
        }
    }
}
