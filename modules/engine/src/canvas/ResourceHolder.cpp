//
// Game.cpp
//

#include "ResourceHolder.h"
#include "VertexFactory.h"

using namespace DirectX;
using namespace canvas;

using Microsoft::WRL::ComPtr;

void ResourceHolder::Initialize(ID3D12Device* device)
{
    m_device = device;

    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(ShaderConstants));

    DX::ThrowIfFailed(device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        nullptr,
        IID_PPV_ARGS(m_constantBuffer.GetAddressOf())
    ));

    CD3DX12_RANGE readRange(0, 0);
    DX::ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_shaderConstants)));
}

void ResourceHolder::Deinitialize() noexcept
{
    m_constantBuffer->Unmap(0, nullptr);
    m_shaderConstants = nullptr;
    m_constantBuffer.Reset();

    UnloadMesh(1);
    UnloadMesh(2);
}

MeshHandle ResourceHolder::LoadMesh(const MeshDesc& desc)
{
    MeshViews meshViews{};
    MeshHandle meshHandle;

    switch (desc) {
    case MeshDesc::CUBES: {
        meshHandle = 1;

        auto meshVertices = MakeCubeVertices();
        m_meshVB = CreateVertexBuffer(m_device, meshVertices.data(), sizeof(meshVertices), sizeof(Vertex));
        auto meshIndices = MakeCubeIndices();
        m_meshIB = CreateIndexBuffer(m_device, meshIndices.data(), sizeof(meshIndices));

        meshViews.vbv = m_meshVB.view;
        meshViews.ibv = m_meshIB.view;
        meshViews.countPerInstance = 36;
        meshViews.instanceCount = 7;
        break;
    }
    case MeshDesc::UI: {
        meshHandle = 2;

        auto uiVertices = MakeTriangle(0.5f, 0.5f);
        m_uiVB = CreateVertexBuffer(m_device, uiVertices.data(), sizeof(uiVertices), sizeof(Vertex));

        meshViews.vbv = m_uiVB.view;
        meshViews.countPerInstance = 3;
        meshViews.instanceCount = 1;
        break;
    }
    };

    m_cache.insert({meshHandle, meshViews});
    return meshHandle;
}

void ResourceHolder::UnloadMesh(MeshHandle handle)
{
    if (!m_cache.erase(handle)) return;

    switch (handle) {
    case 1: {
        m_meshVB.resource.Reset();
        m_meshIB.resource.Reset();
        break;
    }
    case 2: {
        m_uiVB.resource.Reset();
        break;
    }
    }
}

D3D12_GPU_VIRTUAL_ADDRESS ResourceHolder::WritePerDrawCB(const ShaderConstants& data)
{
    *m_shaderConstants = data;
    return m_constantBuffer->GetGPUVirtualAddress();
}

// MARK: - Private

ResourceHolder::VertexBuffer ResourceHolder::CreateVertexBuffer(
    ID3D12Device* device,
    const void* data,
    size_t bytes,
    UINT stride
)
{
    VertexBuffer buffer;

    buffer.resource = CreateResource(device, data, bytes);
    buffer.view.BufferLocation = buffer.resource->GetGPUVirtualAddress();
    buffer.view.StrideInBytes = stride;
    buffer.view.SizeInBytes = bytes;

    return buffer;
}

ResourceHolder::IndexBuffer ResourceHolder::CreateIndexBuffer(
    ID3D12Device* device,
    const void* data,
    size_t bytes
)
{
    IndexBuffer buffer;

    buffer.resource = CreateResource(device, data, bytes);
    buffer.view.BufferLocation = buffer.resource->GetGPUVirtualAddress();
    buffer.view.Format = DXGI_FORMAT_R16_UINT;
    buffer.view.SizeInBytes = bytes;

    return buffer;
}

ComPtr<ID3D12Resource> ResourceHolder::CreateResource(ID3D12Device* device, const void* data, size_t bytes)
{
    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bytes);

    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    DX::ThrowIfFailed(device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(resource.GetAddressOf())
    ));

    UINT8* pData;
    CD3DX12_RANGE readRange(0, 0);
    DX::ThrowIfFailed(resource->Map(0, &readRange, reinterpret_cast<void**>(&pData)));
    memcpy(pData, data, bytes);
    resource->Unmap(0, nullptr);

    return resource;
}