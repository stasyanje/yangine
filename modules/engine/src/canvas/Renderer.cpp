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
    ResourceHolder* resourceHolder,
    Scene* scene
) noexcept :
    m_fuckingTimer(GameTimer()),
    m_deviceResources(deviceResources),
    m_resourceHolder(resourceHolder),
    m_scene(scene),
    m_pipelineStore(pipelineStore)
{
}

// MARK: - IDeviceNotify

void Renderer::OnDeviceActivated(ID3D12Device* device)
{
    m_pipelineStore->Initialize(device);
    m_resourceHolder->Initialize(device);
    m_scene->OnEnter();
    m_initialized = TRUE;
}

void Renderer::OnDeviceLost()
{
    m_pipelineStore->Deinitialize();
    m_resourceHolder->Deinitialize();
    m_scene->OnExit();
    m_initialized = FALSE;
}

// MARK: - Public

void Renderer::OnWindowMessage(canvas::Message message, RECT windowBounds)
{
    switch (message) {
    case canvas::Message::IDLE: {
        break;
    }
    case canvas::Message::PAINT: {
        if (m_initialized && m_hasInvalidSize) {
            m_deviceResources->CreateWindowSizeDependentResources();
            m_hasInvalidSize = FALSE;
        }

        if (m_initialized) {
            Render();
        }

        break;
    }
    case canvas::Message::ESCAPE: {
        m_paused = !m_paused;

        if (m_paused)
            m_fuckingTimer.Pause();
        else
            m_fuckingTimer.Resume();

        break;
    }
    case canvas::Message::ACTIVATED: {
        if (!m_paused)
            m_fuckingTimer.Resume();

        break;
    }
    case canvas::Message::DEACTIVATED: {
        if (!m_paused)
            m_fuckingTimer.Pause();

        break;
    }
    case canvas::Message::DISPLAY_CHANGED: {
        m_deviceResources->UpdateColorSpace();
        break;
    }
    case canvas::Message::SIZE_CHANGED: {
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
    auto commandList = m_deviceResources->Prepare();

    // Render
    for (const DrawItem& drawItem : m_scene->MakeDrawItems(tick)) {
        Draw(drawItem, commandList);
    }

    // Present
    m_deviceResources->Present();
}

void Renderer::Draw(const DrawItem& drawItem, ID3D12GraphicsCommandList* commandList) noexcept
{
    m_pipelineStore->Prepare(drawItem.psoType, commandList);

    // TODO: add srv heap
    // // 6) Глобальные heap'ы (CBV/SRV/UAV)
    // ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() }; // если у тебя отдельный heap под SRV
    // commandList->SetDescriptorHeaps(_countof(heaps), heaps);
    // if (it.srv.ptr) commandList->SetGraphicsRootDescriptorTable(2, drawItem.srv);

    commandList->IASetPrimitiveTopology(drawItem.topology);
    commandList->IASetVertexBuffers(0, 1, &drawItem.vbv);

    if (drawItem.vsCB) commandList->SetGraphicsRootConstantBufferView(0, drawItem.vsCB);
    if (drawItem.psCB) commandList->SetGraphicsRootConstantBufferView(0, drawItem.psCB);

    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

    if (drawItem.ibv.SizeInBytes) {
        commandList->IASetIndexBuffer(&drawItem.ibv);
        commandList->DrawIndexedInstanced(drawItem.countPerInstance, drawItem.instanceCount, 0, 0, 0);
    }
    else {
        commandList->DrawInstanced(drawItem.countPerInstance, drawItem.instanceCount, 0, 0);
    }

    PIXEndEvent(commandList);
}