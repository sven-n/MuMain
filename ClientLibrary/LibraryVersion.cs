// <copyright file="LibraryVersion.cs" company="MUnique">
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
// </copyright>

namespace MUnique.Client.Library;

using System;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;

/// <summary>
/// The version the native client writes into MuError.log: this library's own, which carries the commit it was
/// built from, and the version of the OpenMU packet definitions it was built against.
/// </summary>
public static class LibraryVersion
{
    private const string Unknown = "unknown";

    /// <summary>
    /// <see cref="Describe"/> as UTF-8 for the native side, made once and kept for the life of the process.
    /// </summary>
    private static readonly Lazy<nint> Utf8Text = new(() => Marshal.StringToCoTaskMemUTF8(Describe()));

    /// <summary>
    /// Describes this library, e.g. "1.0.0+01b11b112c9e... (OpenMU packets 0.11.0)".
    /// </summary>
    /// <returns>The description.</returns>
    public static string Describe()
    {
        var assembly = typeof(LibraryVersion).Assembly;
        var version = assembly.GetCustomAttribute<AssemblyInformationalVersionAttribute>()?.InformationalVersion;

        // The package version from the project file: the packets assembly itself carries an older number.
        var packetsVersion = assembly.GetCustomAttributes<AssemblyMetadataAttribute>()
            .FirstOrDefault(attribute => attribute.Key == "OpenMuPacketsVersion")?.Value;
        return $"{version ?? Unknown} (OpenMU packets {packetsVersion ?? Unknown})";
    }

    /// <summary>
    /// Returns <see cref="Describe"/> as null-terminated UTF-8 text, which the caller must not free.
    /// </summary>
    /// <returns>The pointer to the text.</returns>
    [UnmanagedCallersOnly(EntryPoint = "ConnectionManager_GetLibraryVersion")]
    public static nint GetLibraryVersion() => Utf8Text.Value;
}
