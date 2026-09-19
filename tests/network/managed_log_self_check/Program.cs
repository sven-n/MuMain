using System.Runtime.InteropServices;
using MUnique.Client.Library;

internal static unsafe class Program
{
    private static ManagedLog.Level? _capturedLevel;
    private static string? _capturedMessage;

    private static void Main()
    {
        ManagedLog.SetCallback(&CaptureLog);
        ManagedLog.Write(ManagedLog.Level.Error, "login packet failed");

        if (_capturedLevel != ManagedLog.Level.Error || _capturedMessage != "login packet failed")
        {
            throw new InvalidOperationException(
                $"Managed log callback mismatch: level={_capturedLevel}, message={_capturedMessage}");
        }
    }

    [UnmanagedCallersOnly]
    private static void CaptureLog(byte level, byte* message)
    {
        _capturedLevel = (ManagedLog.Level)level;
        _capturedMessage = Marshal.PtrToStringUTF8((nint)message);
    }
}
