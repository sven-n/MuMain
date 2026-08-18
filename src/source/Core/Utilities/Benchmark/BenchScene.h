#pragma once

#include "BenchSegment.h"

// Applies a segment's scene configuration to the live client, and puts back what was there
// before. Kept apart from the catalog so the catalog stays pure data and linkable from tests.
//
// The restore path is the contamination guard: a segment that leaked its configuration into the
// next one would make the next one's numbers look like a change that never happened, silently.

namespace Core::Benchmark::Scene
{
    // The effect toggles as they were before the run touched them.
    SceneConfig CaptureCurrentConfig();

    void ApplyConfig(const SceneConfig& config);
}
