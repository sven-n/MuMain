#pragma once

#include "Network/RoundTripMeter.h"

namespace UI::Diagnostics::PingOverlay
{
void Initialize();
// Shows the reading while it is shown, at the workspace's ping corner.
void Update(const Network::RoundTripMeter::Reading& reading);
void Release();
} // namespace UI::Diagnostics::PingOverlay
