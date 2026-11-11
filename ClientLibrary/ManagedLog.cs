// <copyright file="ManagedLog.cs" company="MUnique">
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
// </copyright>

namespace MUnique.Client.Library;

using System;
using System.Text;

/// <summary>
/// Routes managed diagnostics through the native logger.
/// </summary>
internal static unsafe class ManagedLog
{
    private const int StackBufferSize = 1024;
    private static delegate* unmanaged<byte, byte*, void> _callback;

    /// <summary>
    /// Log severity understood by the native logger.
    /// </summary>
    internal enum Level : byte
    {
        /// <summary>Verbose diagnostic information.</summary>
        Debug,

        /// <summary>Normal operational information.</summary>
        Info,

        /// <summary>A recoverable unexpected condition.</summary>
        Warning,

        /// <summary>An operation failed.</summary>
        Error,

        /// <summary>An unrecoverable process-level failure.</summary>
        Critical,
    }

    /// <summary>
    /// Sets the native logging callback.
    /// </summary>
    /// <param name="callback">The callback.</param>
    internal static void SetCallback(delegate* unmanaged<byte, byte*, void> callback)
    {
        _callback = callback;
    }

    /// <summary>
    /// Writes one UTF-8 log message through the native logger.
    /// </summary>
    /// <param name="level">The severity.</param>
    /// <param name="message">The message.</param>
    internal static void Write(Level level, string message)
    {
        var callback = _callback;
        if (callback == null)
        {
            return;
        }

        var byteCount = Encoding.UTF8.GetByteCount(message);
        var bufferSize = byteCount + 1;
        Span<byte> buffer = bufferSize <= StackBufferSize ? stackalloc byte[bufferSize] : new byte[bufferSize];
        Encoding.UTF8.GetBytes(message, buffer);
        buffer[byteCount] = 0;

        fixed (byte* messagePointer = buffer)
        {
            callback((byte)level, messagePointer);
        }
    }
}
