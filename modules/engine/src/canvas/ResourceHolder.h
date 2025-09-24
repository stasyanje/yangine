//
// Game.h
//

#pragma once

#include "../common/GameTimer.h"
#include "../device/DeviceResources.h"
#include "../device/ResourceFactory.h"
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
    MeshViews GetMeshViews(MeshHandle handle) override;

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

    struct MeshResource
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> vb;
        Microsoft::WRL::ComPtr<ID3D12Resource> ib;
        D3D12_VERTEX_BUFFER_VIEW vbv{};
        D3D12_INDEX_BUFFER_VIEW ibv{};
        std::vector<SubmeshRange> parts;
    };

    VertexBuffer CreateVertexBuffer(const void* data, size_t bytes, UINT stride);
    IndexBuffer CreateIndexBuffer(const void* data, size_t bytes);

    ShaderConstants* m_shaderConstants = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_constantBuffer;
    std::unordered_map<MeshHandle, MeshResource> m_cache;

    std::unique_ptr<device::ResourceFactory> m_resourceFactory;
};
} // namespace canvas
