// <copyright file="ConnectFailure.cs" company="MUnique">
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
// </copyright>

namespace MUnique.Client.Library;

using System;
using System.Net.Sockets;

/// <summary>
/// Describes a failed connect for MuError.log, which players post in public. A <see cref="SocketException"/>'s
/// message names the address, so a socket failure is written as its error code; the native side's "connecting to"
/// line names the address as far as the log may.
/// </summary>
internal static class ConnectFailure
{
    /// <summary>
    /// Describes the failure: the first <see cref="SocketException"/> in the exception chain by its error code, e.g.
    /// "SocketException ConnectionRefused (10061)", or else the exception's type and message. The other exceptions a
    /// connect throws (no host, a port out of range, a handle clash) name no address.
    /// </summary>
    /// <param name="exception">The exception the connect threw.</param>
    /// <returns>The description.</returns>
    public static string Describe(Exception exception)
    {
        for (Exception? current = exception; current is not null; current = current.InnerException)
        {
            if (current is SocketException socketException)
            {
                return $"SocketException {socketException.SocketErrorCode} ({socketException.ErrorCode})";
            }
        }

        return $"{exception.GetType().Name}: {exception.Message}";
    }
}
