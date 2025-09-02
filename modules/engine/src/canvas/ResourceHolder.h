//
// Game.h
//

#pragma once

#include "../common/GameTimer.h"
#include "../device/DeviceResources.h"
#include "../input/InputController.h"
#include "Camera.h"

namespace canvas
{

// A basic renderer implementation that creates a D3D12 device and
// provides rendering functionality.
class ResourceHolder final
{
public:
    // Disallow copy / assign
    ResourceHolder(const ResourceHolder&) = delete;
    ResourceHolder& operator=(const ResourceHolder&) = delete;

    ResourceHolder() noexcept = default;
    ~ResourceHolder() noexcept = default;

    // MARK: - Init

    void Initialize(ID3D12Device*);
    void Deinitialize() noexcept;

    // MARK: - Frame

    void Prepare(ID3D12GraphicsCommandList*, Camera*, double deltaTime) noexcept;

private:
    // MARK: - Init

    void CreateVertexBuffer(ID3D12Device*);
    void CreateIndexBuffer(ID3D12Device*);
    void CreateConstantBuffer(ID3D12Device*);

    // MARK: - Frame

    void UpdateShaderConstants(double totalTime);

    ShaderConstants* m_shaderConstants;

    // Resources
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_constantBuffer;
};
} // namespace canvas
