#pragma once

#include "../pch.h"
#include "Models.h"

namespace canvas
{

struct Camera
{
    const float fovRadians{DirectX::XM_PIDIV4};
    const float nearZ{0.1f};
    const float farZ{100.0f};

    float aspectRatio;
    Float3 position{0.0f, 0.0f, -1.0f};
    Float3 eye;
};

inline DirectX::XMMATRIX CameraViewProjection(Camera camera)
{
    auto V = DirectX::XMMatrixLookAtLH(
        DirectX::XMVectorSet(camera.position.x, camera.position.y, camera.position.z, 0.f),
        DirectX::XMVectorSet(camera.eye.x, camera.eye.y, camera.eye.z, 0.f),
        DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f)
    );

    auto P = DirectX::XMMatrixPerspectiveFovLH(
        camera.fovRadians,
        camera.aspectRatio,
        camera.nearZ,
        camera.farZ
    );

    return V * P;
}

inline void MoveCameraOnMouseMove(Camera* camera, Float2 mouseDelta)
{
    float sensitivity = 1.0f;

    camera->position.x += mouseDelta.x * sensitivity;
    camera->position.y += mouseDelta.y * sensitivity;
}

} // namespace canvas