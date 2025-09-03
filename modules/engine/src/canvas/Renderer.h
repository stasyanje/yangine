//
// Game.h
//

#pragma once

#include "../common/GameTimer.h"
#include "../device/DeviceResources.h"
#include "../input/InputController.h"
#include "Camera.h"
#include "ConstantBuffer.h"
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

    Renderer(DX::DeviceResources*, DX::Pipeline*, ConstantBuffer*, ResourceHolder*, Camera*) noexcept;
    ~Renderer() noexcept = default;

    void OnWindowMessage(canvas::Message, RECT windowBounds);

    // IDeviceNotify
    void OnDeviceActivated(ID3D12Device*) override;
    void OnDeviceLost() override;

private:
    void Render();

    GameTimer m_fuckingTimer;

    bool m_initialized = false;
    bool m_hasInvalidSize = false;
    bool m_paused = false;

    DX::DeviceResources* m_deviceResources;
    DX::Pipeline* m_pipeline;
    ConstantBuffer* m_constantBuffer;
    ResourceHolder* m_resourceHolder;
    Camera* m_camera;
};
} // namespace canvas
