#pragma once

#include <string_view>

namespace Render
{
[[nodiscard]] constexpr bool D3D12DiagnosticEnabled(const char* driverName, const char* value)
{
    return driverName != nullptr && value != nullptr && std::string_view(driverName) == "direct3d12" &&
           std::string_view(value) == "1";
}
} // namespace Render
