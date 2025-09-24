#pragma once

#include "../pch.h"

namespace device
{

class ResourceFactory final
{
public:
    ResourceFactory(ID3D12Device* device) noexcept;
    ~ResourceFactory() = default;

    // Disallow copy/assign
    ResourceFactory(const ResourceFactory&) = delete;
    ResourceFactory& operator=(const ResourceFactory&) = delete;

    // Buffer creation methods
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateUploadBuffer(
        size_t sizeInBytes,
        D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_GENERIC_READ
    );

    // Texture creation methods
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTexture(
        UINT width,
        UINT height,
        DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT,
        D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE
    );

private:
    ID3D12Device* m_device;
};

} // namespace device