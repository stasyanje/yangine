#include "Camera.h"

using namespace canvas;
using namespace DirectX;

Camera::Camera(
    input::InputController* inputController,
    window::WindowStateReducer* stateReducer
) noexcept :
    m_inputController(inputController),
    m_stateReducer(stateReducer)
{
}

void Camera::Prepare(double totalTime)
{
    m_state.aspectRatio = m_stateReducer->getAspectRatio();

    static double lastTime = 0.0;
    float deltaTime = static_cast<float>(totalTime - lastTime);
    lastTime = totalTime;

    MoveCameraOnMouseMove(m_inputController->MouseDelta(), deltaTime);

    // Handle keyboard input for camera movement
    Float3 moveDirection = {0.0f, 0.0f, 0.0f};

    if (m_inputController->IsKeyPressed('W'))
        moveDirection.z += 1.0f; // forward
    if (m_inputController->IsKeyPressed('S'))
        moveDirection.z -= 1.0f; // backward
    if (m_inputController->IsKeyPressed('A'))
        moveDirection.x -= 1.0f; // left
    if (m_inputController->IsKeyPressed('D'))
        moveDirection.x += 1.0f; // right
    if (m_inputController->IsKeyPressed(VK_SPACE))
        moveDirection.y += 1.0f; // up

    // Apply movement if any key is pressed
    if (moveDirection.x != 0.0f || moveDirection.y != 0.0f || moveDirection.z != 0.0f)
        MoveCamera(moveDirection, deltaTime);

    if (m_inputController->IsKeyPressed('M'))
        m_state.pitchYaw.x = 0.0f; // reset camera vertically
}

DirectX::XMMATRIX Camera::CameraViewProjection()
{
    auto V = DirectX::XMMatrixLookToLH(
        DirectX::XMVectorSet(m_state.position.x, m_state.position.y, m_state.position.z, 1.f),
        DirectX::XMVectorSet(m_state.eyeDirection.x, m_state.eyeDirection.y, m_state.eyeDirection.z, 1.f),
        DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f)
    );

    auto P = DirectX::XMMatrixPerspectiveFovLH(
        m_state.fovRadians,
        m_state.aspectRatio,
        m_state.nearZ,
        m_state.farZ
    );

    return V * P;
}

// MARK: - Private

inline void Camera::MoveCameraOnMouseMove(Int2 mouseDelta, float deltaTime)
{
    const float sensitivity = 1.0f;
    const float invertionV = -1.0f;
    const float invertionH = 1.0f;
    const float maxV = XM_PIDIV2 - XMConvertToRadians(0.1f);

    // add mouse delta
    const auto updated = DirectX::XMVectorSet(mouseDelta.y, mouseDelta.x, 0.0f, 0.0f) * deltaTime * sensitivity + DirectX::XMVectorSet(m_state.pitchYaw.x, m_state.pitchYaw.y, 0.0f, 0.0f);
    DirectX::XMStoreFloat2(&m_state.pitchYaw, updated);
    m_state.pitchYaw.x = std::clamp(m_state.pitchYaw.x, -maxV, maxV);

    // vertical eye direction
    m_state.eyeDirection.y = DirectX::XMScalarSin(m_state.pitchYaw.x * invertionV);

    // horizontal eye direction
    const auto pitchCos = DirectX::XMScalarCos(m_state.pitchYaw.x); // multiply XZ by cos(Y) for spherical view
    m_state.eyeDirection.x = DirectX::XMScalarSin(m_state.pitchYaw.y * invertionH) * pitchCos;
    m_state.eyeDirection.z = DirectX::XMScalarCos(m_state.pitchYaw.y * invertionH) * pitchCos;
}

inline void Camera::MoveCamera(Float3 direction, float deltaTime)
{
    float speed = 5.0f;

    // Get camera vectors
    XMVECTOR eyeVector = XMVectorSet(m_state.eyeDirection.x, m_state.eyeDirection.y, m_state.eyeDirection.z, 0.0f);
    XMVECTOR upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    // Calculate camera-aligned axes
    XMVECTOR forward = XMVector3Normalize(eyeVector);
    XMVECTOR right = XMVector3Normalize(XMVector3Cross(upVector, forward));
    XMVECTOR up = XMVector3Cross(forward, right);

    // Calculate movement vector aligned with camera orientation
    XMVECTOR movementVector = XMVectorZero();
    movementVector = XMVectorAdd(movementVector, XMVectorScale(right, direction.x));   // left/right
    movementVector = XMVectorAdd(movementVector, XMVectorScale(up, direction.y));      // up/down
    movementVector = XMVectorAdd(movementVector, XMVectorScale(forward, direction.z)); // forward/backward

    // Apply movement to position
    XMVECTOR currentPos = XMVectorSet(m_state.position.x, m_state.position.y, m_state.position.z, 1.0f);
    XMVECTOR scaledMovement = XMVectorScale(movementVector, speed * deltaTime);
    XMVECTOR newPos = XMVectorAdd(currentPos, scaledMovement);

    XMStoreFloat3(&m_state.position, newPos);
}