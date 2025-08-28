//
// Game.h
//

#pragma once

#include "../common/GameTimer.h"
#include "../device/DeviceResources.h"
#include "../input/InputController.h"

namespace canvas
{

struct ShaderConstants
{
    float mousePos[2];
    float time;
};

// A basic renderer implementation that creates a D3D12 device and
// provides rendering functionality.
class ResourceHolder final
{
public:
    // Disallow copy / assign
    ResourceHolder(const ResourceHolder&) = delete;
    ResourceHolder& operator=(const ResourceHolder&) = delete;

    ResourceHolder(input::InputController*) noexcept;
    ~ResourceHolder() noexcept = default;

    void Initialize(ID3D12Device*);
    void Deinitialize() noexcept;

    void Prepare(ID3D12GraphicsCommandList*, double deltaTime) noexcept;

private:
    void CreateVertexBuffer(ID3D12Device*);
    void CreateConstantBuffer(ID3D12Device*);

    input::InputController* m_inputController;
    ShaderConstants* m_shaderConstants;

    // Resources
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_constantBuffer;
};
} // namespace canvas
