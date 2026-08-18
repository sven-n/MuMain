#pragma once

#include "BenchTypes.h"

// Captures what makes a run's numbers comparable -- or not. Called once, at run start.
//
// The graphics half is the part that has been missing: which GL context the version chain
// actually settled on, and which capability flags the driver reported. On the weak hardware this
// tooling exists for, those two facts decide which code path the client is even running, and
// they have so far had to be read out of a boot log by hand.
//
// Fields that cannot be resolved on a platform stay empty rather than being guessed at.

namespace Core::Benchmark::Environment
{
    EnvironmentInfo Capture();
}
