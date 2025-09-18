//
// Game.cpp
//

#include "Renderer.h"
#include "../common/AsyncLogger.h"
#include "../common/GameTimer.h"
#include "../device/DeviceResources.h"
#include "../pch.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace canvas;

using Microsoft::WRL::ComPtr;

Renderer::Renderer(
    DX::DeviceResources* deviceResources,
    pipeline::Store* pipelineStore,
    ConstantBuffer* constantBuffer,
    ResourceHolder* resourceHolder,
    Camera* camera
) noexcept :
    m_fuckingTimer(GameTimer()),
    m_deviceResources(deviceResources),
    m_constantBuffer(constantBuffer),
    m_resourceHolder(resourceHolder),
    m_pipelineStore(pipelineStore),
    m_camera(camera)
{
}

// MARK: - IDeviceNotify

void Renderer::OnDeviceActivated(ID3D12Device* device)
{
    m_pipelineStore->Initialize(device);
    m_constantBuffer->Initialize(device);
    m_resourceHolder->Initialize(device);
    m_initialized = TRUE;
}

void Renderer::OnDeviceLost()
{
    m_pipelineStore->Deinitialize();
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
            m_fuckingTimer.Pause();
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
            m_fuckingTimer.Pause();

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
    timer::Tick tick = m_fuckingTimer.Tick();

    if (tick.frameCount == m_lastRenderedFrame)
        return;

    m_lastRenderedFrame = tick.frameCount;

    // Prepare
    m_camera->Prepare(tick);
    auto commandList = m_deviceResources->Prepare();

    // Render 3D scene
    m_pipelineStore->Prepare(pipeline::Store::PSO::GRAPHICS, commandList);
    m_constantBuffer->Prepare(commandList, m_camera, tick.totalTime);
    m_resourceHolder->Prepare(commandList);

    // Render UI triangle on top
    m_pipelineStore->Prepare(pipeline::Store::PSO::UI, commandList);
    m_constantBuffer->Prepare(commandList, m_camera, tick.totalTime);
    m_resourceHolder->PrepareUI(commandList);

    // Present
    m_deviceResources->Present();
}