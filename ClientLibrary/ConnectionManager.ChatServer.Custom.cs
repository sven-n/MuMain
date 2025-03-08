// <copyright file="ConnectionManager.ChatServer.Custom.cs" company="MUnique">
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
// </copyright>

namespace MUnique.Client.Library;

using System;
using System.Runtime.InteropServices;
using System.Text;
using MUnique.OpenMU.Network;
using MUnique.OpenMU.Network.Packets.ChatServer;
using MUnique.OpenMU.Network.Xor;

/// <summary>
/// Extension methods to start writing messages of this namespace on a <see cref="IConnection"/>.
/// </summary>
public partial class ConnectionManager
{
    private const int TokenOffset = 6;
    private const int MessageOffset = 5;

    private static readonly Xor3Encryptor TokenEncryptor = new(TokenOffset);
    private static readonly Xor3Encryptor MessageEncryptor = new(MessageOffset);

    /// <summary>
    /// Sends a <see cref="Authenticate" /> to this connection.
    /// </summary>
    /// <param name="handle">The handle of the connection.</param>
    /// <param name="roomId">The room id.</param>
    /// <param name="token">The token to authenticate the client.</param>
    /// <remarks>
    /// Is sent by the client when: This packet is sent by the client after it connected to the server, to authenticate itself.
    /// Causes reaction on server side: The server will check the token. If it's correct, the client gets added to the requested chat room.
    /// </remarks>
    [UnmanagedCallersOnly(EntryPoint = "ConnectionManager_SendAuthenticateExt")]
    public static void SendAuthenticateExt(int handle, ushort @roomId, uint token)
    {
        if (!Connections.TryGetValue(handle, out var connection))
        {
            return;
        }

        try
        {
            connection.CreateAndSend(pipeWriter =>
            {
                var length = AuthenticateRef.Length;
                var packet = new AuthenticateRef(pipeWriter.GetSpan(length)[..length]);
                packet.RoomId = @roomId;

                var tokenString = token.ToString();
                Encoding.UTF8.GetBytes(tokenString, packet.Token);
                TokenEncryptor.Encrypt(packet);

                return length;
            });
        }
        catch
        {
            // Log exception
        }
    }

    /// <summary>
    /// Sends a <see cref="ChatMessage" /> to this connection.
    /// </summary>
    /// <param name="handle">The handle of the connection.</param>
    /// <param name="senderIndex">The sender index.</param>
    /// <param name="message">The message. It's "encrypted" with the 3-byte XOR key (FC CF AB).</param>
    /// <remarks>
    /// Is sent by the server when: This packet is sent by the server after another chat client sent a message to the current chat room.
    /// Causes reaction on client side: The client will show the message.
    /// </remarks>
    [UnmanagedCallersOnly(EntryPoint = "ConnectionManager_SendChatMessageExt")]
    public static void SendChatMessageExt(int handle, byte @senderIndex, IntPtr @message)
    {
        if (!Connections.TryGetValue(handle, out var connection))
        {
            return;
        }

        try
        {
            connection.CreateAndSend(pipeWriter =>
            {
                var messageString = Marshal.PtrToStringAuto(@message);
                var messageLength = Encoding.UTF8.GetByteCount(messageString!);

                var length = ChatMessageRef.GetRequiredSize(messageLength);
                var packet = new ChatMessageRef(pipeWriter.GetSpan(length)[..length]);
                packet.SenderIndex = @senderIndex;
                packet.MessageLength = (byte)messageLength;
                Encoding.UTF8.GetBytes(messageString, packet.Message);
                MessageEncryptor.Encrypt(packet);

                return length;
            });
        }
        catch
        {
            // Log exception
        }
    }
}