//
// Game.h
//

#pragma once

#include "../common/GameTimer.h"
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
    ESCAPE,
    DISPLAY_CHANGED,
    SIZE_CHANGED
};
// A basic renderer implementation that creates a D3D12 device and
// provides rendering functionality.
class Renderer final : public DX::IDeviceNotify
{
public:
    Renderer(input::InputController*, DX::DeviceResources*, window::WindowStateReducer*);
    ~Renderer() noexcept = default;

    // Initialization and management
    void Initialize();

    void OnWindowMessage(canvas::Message, RECT windowBounds);

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

private:
    GameTimer m_fuckingTimer;

    // Dependencies
    DX::DeviceResources* m_deviceResources;
    window::WindowStateReducer* m_stateReducer;
    input::InputController* m_inputController;

    // Pipeline
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    void CreateDeviceDependentResources();
    void CreateTriangleResources();

    void CreateVertexBuffer(ID3D12Device*);
    void CreateSignature(ID3D12Device*);
    void CreatePSO(ID3D12Device*);

    void UpdateTrianglePosition();
    void Prepare(ID3D12GraphicsCommandList*);

    void Render();
};
} // namespace canvas