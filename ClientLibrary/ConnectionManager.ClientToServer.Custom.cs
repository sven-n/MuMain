// <copyright file="ConnectionManager.ClientToServer.Custom.cs" company="MUnique">
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
// </copyright>

namespace MUnique.Client.Library;

using System;
using System.Buffers.Binary;
using System.Runtime.InteropServices;
using System.Text;
using MUnique.OpenMU.Network;
using MUnique.OpenMU.Network.Packets.ClientToServer;
using MUnique.OpenMU.Network.Xor;

/// <summary>
/// Extension methods to start writing messages of this namespace on a <see cref="IConnection"/>.
/// </summary>
public unsafe partial class ConnectionManager
{
    private static readonly Xor3Encryptor Xor3Encryptor = new(0);

    /// <summary>
    /// Sends a <see cref="LoginLongPassword" /> to this connection.
    /// </summary>
    /// <param name="handle">The handle of the connection.</param>
    /// <param name="username">The user name, "encrypted" with Xor3.</param>
    /// <param name="password">The password, "encrypted" with Xor3.</param>
    /// <param name="tickCount">The tick count.</param>
    /// <param name="clientVersion">The client version.</param>
    /// <param name="clientSerial">The client serial.</param>
    /// <remarks>
    /// Is sent by the client when: The player tries to log into the game.
    /// Causes reaction on server side: The server is authenticating the sent login name and password. If it's correct, the state of the player is proceeding to be logged in.
    /// </remarks>
    [UnmanagedCallersOnly(EntryPoint = "ConnectionManager_SendLogin")]
    public static void SendLogin(int handle, IntPtr username, IntPtr password, uint @tickCount, byte* @clientVersion, byte* @clientSerial)
    {
        if (!Connections.TryGetValue(handle, out var connection))
        {
            ManagedLog.Write(ManagedLog.Level.Error, $"NET: Login send skipped; connection handle={handle} not found");
            return;
        }

        try
        {
            var usernameStr = NativeInterop.PtrToWideString(@username)
                ?? throw new ArgumentNullException(nameof(username));
            var passwordStr = NativeInterop.PtrToWideString(@password)
                ?? throw new ArgumentNullException(nameof(password));
            ArgumentNullException.ThrowIfNull(@clientVersion);
            ArgumentNullException.ThrowIfNull(@clientSerial);

            const int usernameLength = 10;
            const int passwordLength = 20;
            if (Encoding.UTF8.GetByteCount(usernameStr) > usernameLength
                || Encoding.UTF8.GetByteCount(passwordStr) > passwordLength)
            {
                throw new ArgumentException("Login credentials exceed packet field length.");
            }

            connection.CreateAndSend(pipeWriter =>
            {
                Span<byte> usernameBytes = stackalloc byte[usernameLength];
                Span<byte> passwordBytes = stackalloc byte[passwordLength];
                usernameBytes.Clear();
                passwordBytes.Clear();
                Encoding.UTF8.GetBytes(usernameStr, usernameBytes);
                Encoding.UTF8.GetBytes(passwordStr, passwordBytes);
                Xor3Encryptor.Encrypt(usernameBytes);
                Xor3Encryptor.Encrypt(passwordBytes);

                var length = LoginLongPasswordRef.Length;
                var packet = new LoginLongPasswordRef(pipeWriter.GetSpan(length)[..length]);
                usernameBytes.CopyTo(packet.Username);
                passwordBytes.CopyTo(packet.Password);
                packet.TickCount = @tickCount;
                new Span<byte>(@clientVersion, packet.ClientVersion.Length).CopyTo(packet.ClientVersion);
                new Span<byte>(@clientSerial, packet.ClientSerial.Length).CopyTo(packet.ClientSerial);

                return length;
            });
            ManagedLog.Write(ManagedLog.Level.Info, $"NET: Login packet staged, handle={handle}, bytes={LoginLongPasswordRef.Length}");
        }
        catch (Exception ex)
        {
            ManagedLog.Write(ManagedLog.Level.Error, $"NET: Login packet staging failed, handle={handle}: {ex}");
        }
    }

    /// <summary>
    /// Sends a RoundTripRequest (C1 F5 0C) to this connection: the server answers it right away with a
    /// RoundTripResponse carrying the same token, so the time in between is the ping.
    /// </summary>
    /// <param name="handle">The handle of the connection.</param>
    /// <param name="token">The token the server sends back.</param>
    /// <remarks>
    /// shortcut: written by hand because the OpenMU packets package this library uses predates the packet;
    /// generate it from the package once a release has it.
    /// </remarks>
    [UnmanagedCallersOnly(EntryPoint = "ConnectionManager_SendRoundTripRequest")]
    public static void SendRoundTripRequest(int handle, uint token)
    {
        if (!Connections.TryGetValue(handle, out var connection))
        {
            return;
        }

        try
        {
            connection.CreateAndSend(pipeWriter =>
            {
                const int length = 8;
                var packet = pipeWriter.GetSpan(length)[..length];
                packet[0] = 0xC1;
                packet[1] = length;
                packet[2] = 0xF5;
                packet[3] = 0x0C;
                BinaryPrimitives.WriteUInt32LittleEndian(packet[4..], token);
                return length;
            });
        }
        catch (Exception ex)
        {
            ManagedLog.Write(ManagedLog.Level.Error, $"NET: Round trip request staging failed, handle={handle}: {ex}");
        }
    }
}
