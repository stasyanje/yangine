//
// Game.h
//

#pragma once

#include "../common/GameTimer.h"
#include "../device/DeviceResources.h"
#include "../input/InputController.h"
#include "ConstantBuffer.h"

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

    void Prepare(ID3D12GraphicsCommandList*) noexcept;
    void PrepareUI(ID3D12GraphicsCommandList*) noexcept;

private:
    struct VertexBuffer
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        D3D12_VERTEX_BUFFER_VIEW view{};
    };

    struct IndexBuffer
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        D3D12_INDEX_BUFFER_VIEW view{};
    };

    VertexBuffer CreateVertexBuffer(ID3D12Device*, const void* data, size_t bytes, UINT stride);
    IndexBuffer CreateIndexBuffer(ID3D12Device*, const void* data, size_t bytes);
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateResource(ID3D12Device* device, const void* data, size_t bytes);

    // MARK: - Resources

    VertexBuffer m_meshVB;
    IndexBuffer m_meshIB;
    VertexBuffer m_uiVB;
};
} // namespace canvas
