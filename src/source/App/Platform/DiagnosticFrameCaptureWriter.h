#pragma once

namespace mu
{

struct FramePixels;

[[nodiscard]] bool WriteDiagnosticFrameCapturePpm(const char* path, const FramePixels& pixels);

} // namespace mu
