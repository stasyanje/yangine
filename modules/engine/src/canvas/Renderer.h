//
// Game.h
//

#pragma once

#include "../common/StepTimer.h"
#include "../device/DeviceResources.h"
#include "../input/InputController.h"

#include <memory>

namespace canvas
{
enum class Message
{
    IDLE,
    PAINT,
    ACTIVATED,
    DEACTIVATED,
    SUSPENDED,
    RESUMED,
    DISPLAY_CHANGED,
    SIZE_CHANGED
};
// A basic renderer implementation that creates a D3D12 device and
// provides rendering functionality.
class Renderer final : public DX::IDeviceNotify
{
public:
    Renderer(input::InputController*, DX::DeviceResources*, window::WindowStateReducer*);
    ~Renderer();

    // Initialization and management
    void Initialize();

    void OnWindowMessage(canvas::Message, RECT windowBounds);

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

private:
    // Rendering loop timer.
    DX::StepTimer m_timer;
    DX::DeviceResources* m_deviceResources;
    window::WindowStateReducer* m_stateReducer;
    input::InputController* m_inputController;

    // If using the DirectX Tool Kit for DX12, uncomment this line:
    // std::unique_ptr<DirectX::GraphicsMemory> m_graphicsMemory;

    // Triangle rendering resources
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void CreateTriangleResources();
    void UpdateTrianglePosition();
};
} // namespace canvas