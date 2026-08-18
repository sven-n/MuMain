#include "BenchTypes.h"

#include <cstdio>

namespace Core::Benchmark
{
    const char* ToString(InvalidReason reason)
    {
        switch (reason)
        {
        case InvalidReason::None:         return "ok";
        case InvalidReason::Stalled:      return "stalled";
        case InvalidReason::Aborted:      return "aborted";
        case InvalidReason::TooFewFrames: return "too-few-frames";
        }
        return "unknown";
    }

    namespace
    {
        // FNV-1a, 64-bit. Not a security hash -- it only has to change when the manifest changes.
        constexpr uint64_t kFnvOffsetBasis = 1469598103934665603ull;
        constexpr uint64_t kFnvPrime = 1099511628211ull;

        uint64_t Fnv1a(const std::string& text)
        {
            uint64_t hash = kFnvOffsetBasis;
            for (char c : text)
            {
                hash ^= (uint64_t)(unsigned char)c;
                hash *= kFnvPrime;
            }
            return hash;
        }
    }

    std::string ComputeManifestHash(const RunManifest& manifest)
    {
        std::string canonical;
        canonical += std::to_string(manifest.repeats) + ';';
        canonical += std::to_string(manifest.warmupFrames) + ';';
        canonical += std::to_string(manifest.measureFrames) + ';';
        for (const std::string& name : manifest.segmentNames) canonical += name + ',';

        char buffer[24];
        snprintf(buffer, sizeof(buffer), "%016llx", (unsigned long long)Fnv1a(canonical));
        return buffer;
    }

    const char* ToString(RunStatus status)
    {
        switch (status)
        {
        case RunStatus::Completed: return "completed";
        case RunStatus::Aborted:   return "aborted";
        }
        return "unknown";
    }
}
