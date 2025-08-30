#pragma once

#include "../pch.h"
#include "Models.h"

namespace canvas
{

struct Camera
{
    const float fovRadians = DirectX::XM_PIDIV4;
    const float nearZ = 0.1f;
    const float farZ = 100.0f;

    float aspectRatio;
    Float3 position;
    Float3 eye;
};

inline void InitializeCamera(Camera* camera, float aspectRatio)
{
    camera->aspectRatio = aspectRatio;
    camera->position = {5.0f, 3.0f, -15.0f};
    camera->eye = {0.0f, 0.0f, 0.0f};
}

inline DirectX::XMMATRIX CameraViewProjection(const Camera& camera)
{
    auto V = DirectX::XMMatrixLookAtLH(
        DirectX::XMVectorSet(camera.position.x, camera.position.y, camera.position.z, 1.f),
        DirectX::XMVectorSet(camera.eye.x, camera.eye.y, camera.eye.z, 1.f),
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
    float sensitivity = 3.0f;

    camera->eye.x += mouseDelta.x * sensitivity;
    camera->eye.y += mouseDelta.y * sensitivity;
}

} // namespace canvas