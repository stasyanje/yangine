//
// Game.h
//

#pragma once

#include "../common/GameTimer.h"
#include "../device/DeviceResources.h"
#include "../input/InputController.h"
#include "Camera.h"
#include "ConstantBuffer.h"
#include "DrawItem.h"
#include "ResourceHolder.h"

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

    ResourceHolder(input::InputController*, window::WindowStateReducer*) noexcept;
    ~ResourceHolder() noexcept = default;

    // MARK: - Init

    void Initialize(ID3D12Device*);
    void Deinitialize() noexcept;

    // MARK: - Frame

    std::vector<DrawItem> CreateDrawItems(const timer::Tick&) noexcept;

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

    VertexBuffer m_meshVB;
    IndexBuffer m_meshIB;
    VertexBuffer m_uiVB;

    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<ConstantBuffer> m_constantBuffer;
};
} // namespace canvas
