#pragma once

#include "../input/InputController.h"
#include "../pch.h"
#include "../window/WindowStateReducer.h"
#include "Models.h"

namespace canvas
{

struct CameraState
{
    const float fovRadians = DirectX::XM_PIDIV4;
    const float nearZ = 0.1f;
    const float farZ = 100.0f;

    float aspectRatio;
    Float3 position;
    Float3 eye; // normalized, relative to position
};

class Camera final
{
public:
    // Disallow copy / assign
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    Camera(input::InputController*, window::WindowStateReducer*) noexcept;
    ~Camera() noexcept = default;

    void InitializeCamera();
    void Update(double totalTime);
    DirectX::XMMATRIX CameraViewProjection();

private:
    CameraState m_state{};

    input::InputController* m_inputController;
    window::WindowStateReducer* m_stateReducer;

    void MoveCameraOnMouseMove(Float2 mouseDelta);
    void MoveCamera(Float3 direction, float deltaTime);
    void UpdateCameraEye();
};

} // namespace canvas