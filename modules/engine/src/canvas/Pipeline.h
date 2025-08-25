//
// Game.h
//

#pragma once

#include "../device/DeviceResources.h"
#include "../input/InputController.h"

#include <memory>

namespace canvas
{
// A basic renderer implementation that creates a D3D12 device and
// provides rendering functionality.
class Pipeline final
{
public:
    Pipeline(input::InputController*, DX::DeviceResources*, window::WindowStateReducer*);
    ~Pipeline() noexcept = default;

    void Initialize();
    void Deinitialize();

    void Prepare(ID3D12GraphicsCommandList*);
    void Draw(ID3D12GraphicsCommandList*);

private:
    // - init
    void CreateVertexBuffer(ID3D12Device*);
    void CreateSignature(ID3D12Device*);
    void CreatePSO(ID3D12Device*);

    void UpdateTrianglePosition();

    // Dependencies
    DX::DeviceResources* m_deviceResources;
    window::WindowStateReducer* m_stateReducer;
    input::InputController* m_inputController;

    // Pipeline
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
};
} // namespace canvas