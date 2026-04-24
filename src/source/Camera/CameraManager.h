#pragma once

#include "ICamera.h"
#include "CameraMode.h"
#include <memory>

/**
 * @brief Manages camera modes and switching
 *
 * Singleton that owns all camera instances and handles mode switching.
 */
class CameraManager
{
public:
    /**
     * @brief Gets the singleton instance
     */
    static CameraManager& Instance();

    /**
     * @brief Initializes camera system
     */
    void Initialize();

    /**
     * @brief Shuts down camera system
     */
    void Shutdown();

    /**
     * @brief Updates active camera
     * @return True if camera is locked
     */
    bool Update();

    /**
     * @brief Sets active camera mode
     * @param mode Target camera mode
     * @return True if mode was changed
     */
    bool SetCameraMode(CameraMode mode);

    /**
     * @brief Cycles to next camera mode (F9 key)
     */
    void CycleToNextMode();

    /**
     * @brief Gets current camera mode
     */
    CameraMode GetCurrentMode() const { return m_CurrentMode; }

    /**
     * @brief Gets active camera instance
     */
    ICamera* GetActiveCamera() const { return m_pActiveCamera; }

private:
    CameraManager();
    ~CameraManager();

    // Singleton (no copy/move)
    CameraManager(const CameraManager&) = delete;
    CameraManager& operator=(const CameraManager&) = delete;

    CameraMode m_CurrentMode;
    ICamera* m_pActiveCamera;

    // Camera instances
    std::unique_ptr<ICamera> m_pDefaultCamera;
    std::unique_ptr<ICamera> m_pOrbitalCamera;
#ifdef _EDITOR
    std::unique_ptr<ICamera> m_pFreeFlyCamera;
    ICamera* m_pSpectatedCamera = nullptr;  // Camera being spectated while in FreeFly mode

    // Saved g_Camera state for the spectated camera, so its Update() sees its own
    // previous state instead of FreeFly's values (which would corrupt yaw, distance, etc.)
    vec3_t m_SpectatedPos = {};
    vec3_t m_SpectatedAngle = {};
    float m_SpectatedFOV = 0.0f;
    float m_SpectatedViewFar = 0.0f;
    float m_SpectatedDistance = 0.0f;
    float m_SpectatedDistanceTarget = 0.0f;
    bool m_bHasSpectatedState = false;
#endif

    void TransitionToCamera(ICamera* pNewCamera);

public:
#ifdef _EDITOR
    ICamera* GetSpectatedCamera() const { return m_pSpectatedCamera; }
    bool GetSpectatedCameraState(vec3_t outPos, vec3_t outAngle) const;
#endif
};
