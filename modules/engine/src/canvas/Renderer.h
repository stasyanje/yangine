//
// Game.h
//

#pragma once

#include "../common/GameTimer.h"
#include "../device/DeviceResources.h"
#include "../input/InputController.h"
#include "Pipeline.h"

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
    Renderer(DX::DeviceResources*, Pipeline*);
    ~Renderer() noexcept = default;

    // Initialization and management
    void Initialize();

    void OnWindowMessage(canvas::Message, RECT windowBounds);

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

private:
    GameTimer m_fuckingTimer;

    DX::DeviceResources* m_deviceResources;
    Pipeline* m_pipeline;

    void CreateDeviceDependentResources();
    void Render();
};
} // namespace canvas
