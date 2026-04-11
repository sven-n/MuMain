#pragma once

// MuSystemInfo — cross-platform system info detection (CPU, OS, RAM, GPU).
// Story 7.10.1: Extracted from ErrorReport.h, which is being deleted.
// [VS0-CORE-MIGRATE-LOGGING]

#include <cstdint>

#define MAX_LENGTH_CPUNAME (128)
#define MAX_LENGTH_OSINFO (128)
#define MAX_GPU_BACKEND_LEN (128)

struct MuSystemInfo
{
    wchar_t m_lpszCPU[MAX_LENGTH_CPUNAME];
    wchar_t m_lpszOS[MAX_LENGTH_OSINFO];
    int64_t m_iMemorySize; // bytes

    wchar_t m_lpszGpuBackend[MAX_GPU_BACKEND_LEN];
};

// Populate system info using cross-platform POSIX APIs.
void MuGetSystemInfo(MuSystemInfo* si);
