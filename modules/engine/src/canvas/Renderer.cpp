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
    Pipeline* pipeline,
    ResourceHolder* resourceHolder
) :
    m_fuckingTimer(GameTimer()),
    m_deviceResources(deviceResources),
    m_pipeline(pipeline),
    m_resourceHolder(resourceHolder)
{
}

void Renderer::OnDeviceActivated(ID3D12Device* device)
{
    m_pipeline->Initialize(device);
    m_resourceHolder->Initialize(device);
}

void Renderer::OnDeviceLost()
{
    m_pipeline->Deinitialize();
    m_resourceHolder->Deinitialize();
}

// Draws the scene.
void Renderer::Render()
{
    __int64 static lastFrame;
    __int64 currentFrame = m_fuckingTimer.Frame();

    if (currentFrame == lastFrame)
        return;

    lastFrame = currentFrame;

    // Prepare
    auto commandList = m_deviceResources->Prepare();
    m_pipeline->Prepare(commandList);
    m_resourceHolder->Prepare(commandList, m_fuckingTimer.TotalTime());

    // Present
    m_deviceResources->Present();
}

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
        Render();
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
        m_deviceResources->CreateWindowSizeDependentResources();
        break;
    }
    default:
        break;
    }
}