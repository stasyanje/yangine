#include "Camera.h"

using namespace canvas;

Camera::Camera(
    input::InputController* inputController,
    window::WindowStateReducer* stateReducer
) noexcept :
    m_inputController(inputController),
    m_stateReducer(stateReducer)
{
}

void Camera::InitializeCamera()
{
    m_state.aspectRatio = m_stateReducer->getAspectRatio();
    m_state.position = {0.0f, 0.0f, -2.0f};
    m_state.eye = {0.0f, 0.0f, 1.0f};
}

void Camera::Update(double totalTime)
{
    static double lastTime = 0.0;
    float deltaTime = static_cast<float>(totalTime - lastTime);
    lastTime = totalTime;

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
        UpdateCameraEye();

    m_state.eye.x = sin(totalTime);
    m_state.eye.z = cos(totalTime);
}

DirectX::XMMATRIX Camera::CameraViewProjection()
{
    auto V = DirectX::XMMatrixLookToLH(
        DirectX::XMVectorSet(m_state.position.x, m_state.position.y, m_state.position.z, 1.f),
        DirectX::XMVectorSet(m_state.eye.x, m_state.eye.y, m_state.eye.z, 1.f),
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

inline void Camera::MoveCameraOnMouseMove(Float2 mouseDelta)
{
    float sensitivity = 1.0f;

    // Update eye vector components with wrapping behavior
    m_state.eye.x += mouseDelta.x * sensitivity;
    m_state.eye.y += mouseDelta.y * sensitivity;

    // Normalize to keep as unit vector
    DirectX::XMVECTOR eyeVector = DirectX::XMVectorSet(m_state.eye.x, m_state.eye.y, m_state.eye.z, 0.0f);
    eyeVector = DirectX::XMVector3Normalize(eyeVector);
    DirectX::XMStoreFloat3(&m_state.eye, eyeVector);
}

inline void Camera::MoveCamera(Float3 direction, float deltaTime)
{
    float speed = 5.0f;

    auto eyePos = DirectX::XMVectorSet(m_state.eye.x, m_state.eye.y, m_state.eye.z, 1.0f);
    auto camPos = DirectX::XMVectorSet(m_state.position.x, m_state.position.y, m_state.position.z, 1.0f);

    auto movementVector = DirectX::XMVectorMultiply(
        eyePos,
        DirectX::XMLoadFloat3(&direction)
    );

    auto scaledMovement = DirectX::XMVectorScale(movementVector, speed * deltaTime);
    auto newCamPos = DirectX::XMVectorAdd(camPos, scaledMovement);

    DirectX::XMStoreFloat3(&m_state.position, newCamPos);
}

inline void Camera::UpdateCameraEye()
{
    m_state.eye = {0.0f, 0.0f, 1.0f};
}