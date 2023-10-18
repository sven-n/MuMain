// <copyright file="ConnectionManager.ClientToServer.Custom.cs" company="MUnique">
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
// </copyright>

namespace MUnique.Client.ManagedLibrary;

using System;
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

    [StructLayout(LayoutKind.Sequential, Pack = 1, Size = 3)]
    public struct AreaSkillHitTarget
    {
        public ushort TargetId;
        public byte AnimationCounter;
    }

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
    [UnmanagedCallersOnly]
    public static void SendLogin(int handle, IntPtr username, IntPtr password, uint @tickCount, byte* @clientVersion, byte* @clientSerial)
    {
        if (!Connections.TryGetValue(handle, out var connection))
        {
            return;
        }

        try
        {
            var usernameStr = Marshal.PtrToStringAuto(@username);
            var passwordStr = Marshal.PtrToStringAuto(@password);

            // todo: check if username or password is too long
            connection.CreateAndSend(pipeWriter =>
            {
                Span<byte> usernameBytes = stackalloc byte[10];
                Span<byte> passwordBytes = stackalloc byte[20];
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
        }
        catch
        {
            // Log exception
        }
    }

    /// <summary>
    /// Sends a <see cref="AreaSkillHit" /> to this connection.
    /// </summary>
    /// <param name="handle">The handle of the connection.</param>
    /// <param name="skillId">The skill id.</param>
    /// <param name="targetX">The target x.</param>
    /// <param name="targetY">The target y.</param>
    /// <param name="serial">The serial.</param>
    /// <param name="targetCount">The target count.</param>
    /// <param name="targets">The target data.</param>
    [UnmanagedCallersOnly]
    public static void SendAreaSkillHits(int handle, ushort skillId, byte targetX, byte targetY, byte serial, byte targetCount, AreaSkillHitTarget* targets)
    {
        if (!Connections.TryGetValue(handle, out var connection))
        {
            return;
        }

        try
        {
            connection.CreateAndSend(pipeWriter =>
            {
                var length = AreaSkillHitRef.GetRequiredSize(targetCount);
                var packet = new AreaSkillHitRef(pipeWriter.GetSpan(length)[..length]);
                packet.SkillId = skillId;
                packet.TargetX = targetX;
                packet.TargetY = targetY;
                packet.HitCounter = serial;
                packet.TargetCount = targetCount;
                for (int i = 0; i < targetCount; ++i)
                {
                    var source = targets[i];
                    var target = packet[i];
                    target.AnimationCounter = source.AnimationCounter;
                    target.TargetId = source.TargetId;
                }

                return length;
            });
        }
        catch
        {
            // Log exception
        }
    }
}