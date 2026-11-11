// <copyright file="NativeInterop.cs" company="MUnique">
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
// </copyright>

namespace MUnique.Client.Library;

using System;
using System.Runtime.InteropServices;

/// <summary>
/// Helpers for marshalling strings passed from the native (C++) client.
/// </summary>
/// <remarks>
/// Native callers convert platform <c>wchar_t</c> strings to <c>char16_t</c>
/// before crossing this boundary, so every platform supplies UTF-16.
/// </remarks>
internal static class NativeInterop
{
    /// <summary>
    /// Decodes a native UTF-16 <c>char16_t*</c> string.
    /// </summary>
    /// <param name="ptr">Pointer to the native string (may be zero).</param>
    /// <returns>The managed string, or <see langword="null"/> for a null pointer.</returns>
    public static string? PtrToWideString(nint ptr)
    {
        if (ptr == 0)
        {
            return null;
        }

        return Marshal.PtrToStringUni(ptr);
    }
}
