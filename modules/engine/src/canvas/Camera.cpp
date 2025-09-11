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

    MoveEye(m_inputController->CollectMouseDelta());
    MovePosition(MoveDirection(), deltaTime);

    if (m_inputController->IsKeyPressed('M'))
        m_state.pitchYaw.x = 0.0f; // reset camera vertically
}

DirectX::XMMATRIX Camera::CameraViewProjection()
{
    auto direction = XMVectorSet(m_state.eyeDirection.x, m_state.eyeDirection.y, m_state.eyeDirection.z, 1.f);
    // make sure direction is no zero vector
    if (XMVector3Equal(direction, XMVectorZero())) direction = XMVectorSet(0.0f, 0.0f, 1.0f, 1.f);

    auto view = XMMatrixLookToLH(
        XMVectorSet(m_state.position.x, m_state.position.y, m_state.position.z, 1.f),
        direction,
        XMVectorSet(0.f, 1.f, 0.f, 0.f)
    );

    auto projection = DirectX::XMMatrixPerspectiveFovLH(
        m_state.fovRadians,
        m_state.aspectRatio,
        m_state.nearZ,
        m_state.farZ
    );

    return view * projection;
}

// MARK: - Private

inline Int3 Camera::MoveDirection()
{
    // Handle keyboard input for camera movement
    Int3 moveDirection{0, 0, 0};

    if (m_inputController->IsKeyPressed('W'))
        moveDirection.z += 1; // forward
    if (m_inputController->IsKeyPressed('S'))
        moveDirection.z -= 1; // backward
    if (m_inputController->IsKeyPressed('A'))
        moveDirection.x -= 1; // left
    if (m_inputController->IsKeyPressed('D'))
        moveDirection.x += 1; // right
    if (m_inputController->IsKeyPressed(VK_SPACE))
        moveDirection.y += 1; // up

    return moveDirection;
}

inline void Camera::MoveEye(Int2 mouseDelta)
{
    if (IsZero(mouseDelta)) return;

    constexpr float sensitivityX = 0.001f * 1.0f;  // not inverted
    constexpr float sensitivityY = 0.001f * -1.0f; // inverted
    constexpr float maxYaw = XM_PIDIV2 - XMConvertToRadians(0.1f);

    m_state.pitchYaw.x = std::clamp(m_state.pitchYaw.x + mouseDelta.y * sensitivityY, -maxYaw, maxYaw);
    m_state.pitchYaw.y += mouseDelta.x * sensitivityX;

    float sinPitch, cosPitch, sinYaw, cosYaw;
    DirectX::XMScalarSinCos(&sinPitch, &cosPitch, m_state.pitchYaw.x);
    DirectX::XMScalarSinCos(&sinYaw, &cosYaw, m_state.pitchYaw.y);

    // convert spherical to vector
    m_state.eyeDirection.x = sinYaw * cosPitch;
    m_state.eyeDirection.y = sinPitch;
    m_state.eyeDirection.z = cosYaw * cosPitch;
}

inline void Camera::MovePosition(Int3 direction, float deltaTime)
{
    if (IsZero(direction)) return;

    constexpr float speed = 5.0f;

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