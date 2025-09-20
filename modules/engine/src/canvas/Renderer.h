//
// Game.h
//

#pragma once

#include "../common/GameTimer.h"
#include "../device/DeviceResources.h"
#include "../input/InputController.h"
#include "../pipeline/Store.h"
#include "Camera.h"
#include "ConstantBuffer.h"
#include "DrawItem.h"
#include "ResourceHolder.h"

#include <memory>

namespace canvas
{
enum class Message
{
    IDLE,
    PAINT,
    ACTIVATED,
    DEACTIVATED,
    ESCAPE,
    DISPLAY_CHANGED,
    SIZE_CHANGED
};
// A basic renderer implementation that creates a D3D12 device and
// provides rendering functionality.
class Renderer final : public DX::IDeviceNotify
{
public:
    // Disallow copy / assign
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    Renderer(DX::DeviceResources*, pipeline::Store*, ResourceHolder*) noexcept;
    ~Renderer() noexcept = default;

    void OnWindowMessage(canvas::Message, RECT windowBounds);

    // IDeviceNotify
    void OnDeviceActivated(ID3D12Device*) override;
    void OnDeviceLost() override;

private:
    void Render();
    void Draw(const DrawItem&, ID3D12GraphicsCommandList*) noexcept;

    GameTimer m_fuckingTimer;
    uint64_t m_lastRenderedFrame = 0;

    bool m_initialized = false;
    bool m_hasInvalidSize = false;
    bool m_paused = false;

    DX::DeviceResources* m_deviceResources;
    pipeline::Store* m_pipelineStore;
    ResourceHolder* m_resourceHolder;
};
} // namespace canvas
