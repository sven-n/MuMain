#pragma once

/**
 * @brief Available camera modes
 */
enum class CameraMode
{
    Default,    // Original third-person follow camera
    Orbital,    // Spherical orbit around character (F9 toggle)
    Legacy,     // DefaultCamera + cheap distance culling (for A/B perf testing)
#ifdef _EDITOR
    FreeFly,    // Unconstrained free movement (editor only)
#endif
};

/**
 * @brief Returns human-readable camera mode name
 */
inline const char* CameraModeToString(CameraMode mode)
{
    switch (mode)
    {
        case CameraMode::Default: return "Default";
        case CameraMode::Orbital: return "Orbital";
        case CameraMode::Legacy: return "Legacy";
#ifdef _EDITOR
        case CameraMode::FreeFly: return "FreeFly";
#endif
        default: return "Unknown";
    }
}

/**
 * @brief Cycles to next camera mode
 * @param current Current mode
 * @return Next mode in sequence
 */
inline CameraMode GetNextCameraMode(CameraMode current)
{
    switch (current)
    {
        case CameraMode::Default:
            return CameraMode::Orbital;
        case CameraMode::Orbital:
            return CameraMode::Legacy;
        case CameraMode::Legacy:
            return CameraMode::Default;
#ifdef _EDITOR
        case CameraMode::FreeFly:
            // FreeFly is UI-only, should not be in F9 cycle
            return CameraMode::Default;
#endif
        default:
            return CameraMode::Default;
    }
}
