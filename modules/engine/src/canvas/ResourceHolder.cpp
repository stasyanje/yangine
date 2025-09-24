//
// Game.cpp
//

#include "ResourceHolder.h"
#include "VertexFactory.h"

using namespace DirectX;
using namespace canvas;

using Microsoft::WRL::ComPtr;

inline void Unmap(ID3D12Resource* resource)
{
    resource->Unmap(0, nullptr);
}

inline void Map(ID3D12Resource* resource, void* pData)
{
    CD3DX12_RANGE readRange(0, 0);
    DX::ThrowIfFailed(resource->Map(0, &readRange, reinterpret_cast<void**>(pData)));
}

inline void MapCopyUnmap(ID3D12Resource* resource, const void* data, size_t sizeInBytes)
{
    UINT8* pData;
    CD3DX12_RANGE readRange(0, 0);
    DX::ThrowIfFailed(resource->Map(0, &readRange, reinterpret_cast<void**>(&pData)));
    memcpy(pData, data, sizeInBytes);
    resource->Unmap(0, nullptr);
}

void ResourceHolder::Initialize(ID3D12Device* device)
{
    m_resourceFactory = std::make_unique<device::ResourceFactory>(device);

    m_constantBuffer = m_resourceFactory->CreateUploadBuffer(
        sizeof(ShaderConstants),
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
    );
    Map(m_constantBuffer.Get(), &m_shaderConstants);
}

void ResourceHolder::Deinitialize() noexcept
{
    Unmap(m_constantBuffer.Get());
    m_constantBuffer.Reset();
    m_shaderConstants = nullptr;

    m_cache.clear();
}

MeshHandle ResourceHolder::LoadMesh(const MeshDesc& desc)
{
    MeshHandle meshHandle;

    SubmeshRange submeshRange{};
    MeshResource meshResource{};

    switch (desc) {
    case MeshDesc::CUBES: {
        meshHandle = 1;

        auto meshVertices = MakeCubeVertices();
        auto meshVB = CreateVertexBuffer(meshVertices.data(), sizeof(meshVertices), sizeof(Vertex));
        meshResource.vb = meshVB.resource;
        meshResource.vbv = meshVB.view;

        auto meshIndices = MakeCubeIndices();
        auto meshIB = CreateIndexBuffer(meshIndices.data(), sizeof(meshIndices));
        meshResource.ib = meshIB.resource;
        meshResource.ibv = meshIB.view;

        submeshRange.indexCount = 36;
        submeshRange.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        break;
    }
    case MeshDesc::UI: {
        meshHandle = 2;

        auto uiVertices = MakeTriangle(0.5f, 0.5f);
        auto uiVB = CreateVertexBuffer(uiVertices.data(), sizeof(uiVertices), sizeof(Vertex));
        meshResource.vb = uiVB.resource;
        meshResource.vbv = uiVB.view;

        submeshRange.indexCount = 3;
        submeshRange.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        break;
    }
    };

    meshResource.parts.push_back(submeshRange);
    m_cache[meshHandle] = meshResource;
    return meshHandle;
}

void ResourceHolder::UnloadMesh(MeshHandle handle)
{
    m_cache.erase(handle);
}

MeshViews ResourceHolder::GetMeshViews(MeshHandle handle)
{
    auto resource = m_cache.at(handle);

    MeshViews meshViews{};

    meshViews.vbv = resource.vbv;
    meshViews.ibv = resource.ibv;
    meshViews.parts = resource.parts;

    return meshViews;
};

D3D12_GPU_VIRTUAL_ADDRESS ResourceHolder::WritePerDrawCB(const ShaderConstants& data)
{
    *m_shaderConstants = data;
    return m_constantBuffer->GetGPUVirtualAddress();
}

// MARK: - Private

ResourceHolder::VertexBuffer ResourceHolder::CreateVertexBuffer(
    const void* data,
    size_t bytes,
    UINT stride
)
{
    VertexBuffer buffer;

    buffer.resource = m_resourceFactory->CreateUploadBuffer(bytes);
    MapCopyUnmap(buffer.resource.Get(), data, bytes);

    buffer.view.BufferLocation = buffer.resource->GetGPUVirtualAddress();
    buffer.view.StrideInBytes = stride;
    buffer.view.SizeInBytes = bytes;

    return buffer;
}

ResourceHolder::IndexBuffer ResourceHolder::CreateIndexBuffer(
    const void* data,
    size_t bytes
)
{
    IndexBuffer buffer;

    buffer.resource = m_resourceFactory->CreateUploadBuffer(bytes);
    MapCopyUnmap(buffer.resource.Get(), data, bytes);

    buffer.view.BufferLocation = buffer.resource->GetGPUVirtualAddress();
    buffer.view.Format = DXGI_FORMAT_R16_UINT;
    buffer.view.SizeInBytes = bytes;

    return buffer;
}
