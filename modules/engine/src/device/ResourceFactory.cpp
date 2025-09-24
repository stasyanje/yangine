#include "ResourceFactory.h"

using namespace device;
using Microsoft::WRL::ComPtr;

ResourceFactory::ResourceFactory(ID3D12Device* device) noexcept :
    m_device(device)
{
}

ComPtr<ID3D12Resource> ResourceFactory::CreateUploadBuffer(
    size_t sizeInBytes,
    D3D12_RESOURCE_STATES initialState
)
{
    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes);

    ComPtr<ID3D12Resource> resource;
    DX::ThrowIfFailed(m_device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        initialState,
        nullptr,
        IID_PPV_ARGS(resource.GetAddressOf())
    ));

    return resource;
}

ComPtr<ID3D12Resource> ResourceFactory::CreateDepthStencilTexture(
    UINT width,
    UINT height,
    DXGI_FORMAT format,
    D3D12_RESOURCE_STATES initialState
)
{
    auto depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        format,
        width,
        height,
        1,
        0,
        1,
        0,
        D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
    );

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = format;
    depthOptimizedClearValue.DepthStencil = {1.0f, 0};

    const CD3DX12_HEAP_PROPERTIES heapType(D3D12_HEAP_TYPE_DEFAULT);

    ComPtr<ID3D12Resource> resource;
    DX::ThrowIfFailed(m_device->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        initialState,
        &depthOptimizedClearValue,
        IID_PPV_ARGS(resource.GetAddressOf())
    ));

    return resource;
}