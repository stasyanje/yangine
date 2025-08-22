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
    Pipeline* pipeline
) :
    m_fuckingTimer(GameTimer()),
    m_deviceResources(deviceResources),
    m_pipeline(pipeline)
{
}

// Initialize the Direct3D resources required to run.
void Renderer::Initialize()
{
    CreateDeviceDependentResources();
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
    m_pipeline->Draw(commandList);

    // Present
    PIXBeginEvent(m_deviceResources->GetCommandQueue(), PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();
    PIXEndEvent(m_deviceResources->GetCommandQueue());
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
        if (m_fuckingTimer.Running())
            m_fuckingTimer.Stop();
        else
            m_fuckingTimer.Resume();

        break;
    }
    case canvas::Message::ACTIVATED:
    {
        m_fuckingTimer.Resume();
        break;
    }
    case canvas::Message::DEACTIVATED:
    {
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

void Renderer::CreateDeviceDependentResources()
{
    m_pipeline->Initialize();
}

void Renderer::OnDeviceLost()
{
    m_pipeline->Deinitialize();
}

void Renderer::OnDeviceRestored()
{
    CreateDeviceDependentResources();
}