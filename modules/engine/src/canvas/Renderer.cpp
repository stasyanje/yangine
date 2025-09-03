//
// Game.cpp
//

#include "Renderer.h"
#include "../common/AsyncLogger.h"
#include "../device/DeviceResources.h"
#include "../pch.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace canvas;

using Microsoft::WRL::ComPtr;

Renderer::Renderer(
    DX::DeviceResources* deviceResources,
    ConstantBuffer* constantBuffer,
    ResourceHolder* resourceHolder,
    Camera* camera
) :
    m_fuckingTimer(GameTimer()),
    m_deviceResources(deviceResources),
    m_constantBuffer(constantBuffer),
    m_resourceHolder(resourceHolder),
    m_camera(camera)
{
}

// MARK: - IDeviceNotify

void Renderer::OnDeviceActivated(ID3D12Device* device)
{
    m_constantBuffer->Initialize(device);
    m_resourceHolder->Initialize(device);
    m_initialized = TRUE;
}

void Renderer::OnDeviceLost()
{
    m_constantBuffer->Deinitialize();
    m_resourceHolder->Deinitialize();
    m_initialized = FALSE;
}

// MARK: - Public

void Renderer::OnWindowMessage(canvas::Message message, RECT windowBounds)
{
    switch (message)
    {
    case canvas::Message::IDLE:
    {
        break;
    }
    case canvas::Message::PAINT:
    {
        m_fuckingTimer.Tick();

        if (m_initialized && m_hasInvalidSize)
        {
            m_deviceResources->CreateWindowSizeDependentResources();
            m_hasInvalidSize = FALSE;
        }

        if (m_initialized)
        {
            Render();
        }

        break;
    }
    case canvas::Message::ESCAPE:
    {
        m_paused = !m_paused;

        if (m_paused)
            m_fuckingTimer.Stop();
        else
            m_fuckingTimer.Resume();

        break;
    }
    case canvas::Message::ACTIVATED:
    {
        if (!m_paused)
            m_fuckingTimer.Resume();

        break;
    }
    case canvas::Message::DEACTIVATED:
    {
        if (!m_paused)
            m_fuckingTimer.Stop();

        break;
    }
    case canvas::Message::DISPLAY_CHANGED:
    {
        m_deviceResources->UpdateColorSpace();
        break;
    }
    case canvas::Message::SIZE_CHANGED:
    {
        m_hasInvalidSize = TRUE;
        break;
    }
    default:
        break;
    }
}

// MARK: - Private

void Renderer::Render()
{
    __int64 static lastFrame;
    __int64 currentFrame = m_fuckingTimer.Frame();

    if (currentFrame == lastFrame)
        return;

    lastFrame = currentFrame;

    auto totalTime = m_fuckingTimer.TotalTime();

    // Prepare
    auto commandList = m_deviceResources->Prepare();
    m_camera->Prepare(totalTime);
    m_constantBuffer->Prepare(commandList, m_camera, totalTime);
    m_resourceHolder->Prepare(commandList);

    // Present
    m_deviceResources->Present();
}