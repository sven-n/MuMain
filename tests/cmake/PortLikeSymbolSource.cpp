// The port-like target's own network library adapter (port-like-target.cmake): like a port without the NativeAOT
// library, it has no exports, so the bridge reports the library missing and the network stays off.
#include "Dotnet/ManagedSymbols.h"

DotNetBridge::ManagedSymbolSource DotNetBridge::TargetManagedSymbolSource()
{
    return {"no network library (port-like target)", nullptr};
}
