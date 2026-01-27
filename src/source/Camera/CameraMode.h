#pragma once

/**
 * @brief Available camera modes
 */
enum class CameraMode
{
    Default,    // Original third-person follow camera
    Orbital,    // Spherical orbit around character (F9 toggle)
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
#ifdef _EDITOR
            return CameraMode::FreeFly;
        case CameraMode::FreeFly:
#endif
            return CameraMode::Default;
        default:
            return CameraMode::Default;
    }
}
