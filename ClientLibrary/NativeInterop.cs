// <copyright file="NativeInterop.cs" company="MUnique">
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
// </copyright>

namespace MUnique.Client.Library;

using System;
using System.Runtime.InteropServices;
using System.Text;

/// <summary>
/// Helpers for marshalling strings passed from the native (C++) client.
/// </summary>
/// <remarks>
/// The native client passes strings as <c>wchar_t*</c>. <c>wchar_t</c> is 2 bytes
/// (UTF-16) on Windows but 4 bytes (UTF-32) on Linux/macOS, while
/// <c>Marshal.PtrToStringAuto</c> always decodes UTF-16. Reading a Linux UTF-32
/// buffer as UTF-16 garbles every character (e.g. a host address turns into
/// nonsense and the connection hangs). This decodes by the platform's real
/// <c>wchar_t</c> width so both sides agree.
/// </remarks>
internal static class NativeInterop
{
    /// <summary>
    /// Decodes a native <c>wchar_t*</c> string using the platform's wchar_t width.
    /// </summary>
    /// <param name="ptr">Pointer to the native string (may be zero).</param>
    /// <returns>The managed string, or <see langword="null"/> for a null pointer.</returns>
    public static unsafe string? PtrToWideString(nint ptr)
    {
        if (ptr == 0)
        {
            return null;
        }

        // Windows wchar_t is 2 bytes (UTF-16); the built-in decoder is correct.
        if (OperatingSystem.IsWindows())
        {
            return Marshal.PtrToStringUni(ptr);
        }

        // Unix wchar_t is 4 bytes (UTF-32). Find the null terminator (a 32-bit
        // zero) and decode the preceding code units.
        var p = (uint*)ptr;
        var count = 0;
        while (p[count] != 0)
        {
            count++;
        }

        return Encoding.UTF32.GetString((byte*)ptr, count * sizeof(uint));
    }
}
