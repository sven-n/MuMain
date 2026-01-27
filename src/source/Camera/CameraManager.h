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
#endif

    void TransitionToCamera(ICamera* pNewCamera);
};
