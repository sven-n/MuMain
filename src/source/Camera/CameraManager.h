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

    /**
     * @brief Whether camera zoom (and orbital rotation) is locked.
     *
     * F10 toggles this flag. It defaults to true so the camera doesn't
     * react to the wheel until the player explicitly unlocks it.
     */
    bool IsZoomLocked() const { return m_ZoomLocked; }

    /**
     * @brief Flips the zoom-lock flag (F10 key).
     */
    void ToggleZoomLock() { m_ZoomLocked = !m_ZoomLocked; }

    /**
     * @brief Resets the active camera's user-facing view (F11 key).
     */
    void ResetActiveView()
    {
        if (m_pActiveCamera)
            m_pActiveCamera->ResetView();
    }

private:
    CameraManager();
    ~CameraManager();

    // Singleton (no copy/move)
    CameraManager(const CameraManager&) = delete;
    CameraManager& operator=(const CameraManager&) = delete;

    CameraMode m_CurrentMode;
    ICamera* m_pActiveCamera;
    bool m_ZoomLocked = true;  // F10 toggles; default = locked

    // Camera instances
    std::unique_ptr<ICamera> m_pDefaultCamera;
    std::unique_ptr<ICamera> m_pOrbitalCamera;
#ifdef _EDITOR
    std::unique_ptr<ICamera> m_pFreeFlyCamera;
    ICamera* m_pSpectatedCamera = nullptr;  // Camera being spectated while in FreeFly mode

    // Saved g_Camera state for the spectated camera, so its Update() sees its own
    // previous state instead of FreeFly's values (which would corrupt yaw, distance, etc.)
    struct SpectatedState
    {
        vec3_t Position = {};
        vec3_t Angle = {};
        float FOV = 0.0f;
        float ViewFar = 0.0f;
        float Distance = 0.0f;
        float DistanceTarget = 0.0f;
        bool HasData = false;
    } m_Spectated;

    void SaveGlobalToSpectated();
    void RestoreSpectatedToGlobal();
    void UpdateSpectatedCamera();
#endif

    void TransitionToCamera(ICamera* pNewCamera);

public:
#ifdef _EDITOR
    ICamera* GetSpectatedCamera() const { return m_pSpectatedCamera; }
    bool GetSpectatedCameraState(vec3_t outPos, vec3_t outAngle) const;
#endif
};
