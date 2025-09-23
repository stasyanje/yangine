//
// Game.h
//

#pragma once

#include "../common/GameTimer.h"
#include "../device/DeviceResources.h"
#include "../input/InputController.h"
#include "Camera.h"
#include "DrawItem.h"
#include "Scene.h"

#include <unordered_map>

namespace canvas
{

// A basic renderer implementation that creates a D3D12 device and
// provides rendering functionality.
class ResourceHolder final : public ResourceFactory, public RendererServices
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

    // MARK: - ResourceFactory

    MeshHandle LoadMesh(const MeshDesc&) override;
    void UnloadMesh(MeshHandle) override;
    inline MeshViews GetMeshViews(MeshHandle handle) override { return m_cache.at(handle); };

    // MARK: - RendererServices

    D3D12_GPU_VIRTUAL_ADDRESS WritePerDrawCB(const ShaderConstants& data) override;

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
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateResource(ID3D12Device*, const void* data, size_t bytes);

    VertexBuffer m_meshVB;
    IndexBuffer m_meshIB;
    VertexBuffer m_uiVB;
    ShaderConstants* m_shaderConstants = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_constantBuffer;
    std::unordered_map<MeshHandle, MeshViews> m_cache;

    ID3D12Device* m_device = nullptr;
};
} // namespace canvas
