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
    // Mesh VB
    auto meshVertices = MakeCubeVertices();
    m_meshVB = CreateVertexBuffer(device, meshVertices.data(), sizeof(meshVertices), sizeof(Vertex));

    // Mesh IB
    auto meshIndices = MakeCubeIndices();
    m_meshIB = CreateIndexBuffer(device, meshIndices.data(), sizeof(meshIndices));

    // UI VB
    auto uiVertices = MakeTriangle(0.5f, 0.5f);
    m_uiVB = CreateVertexBuffer(device, uiVertices.data(), sizeof(uiVertices), sizeof(Vertex));
}

void ResourceHolder::Deinitialize() noexcept
{
    m_meshVB.resource.Reset();
    m_meshIB.resource.Reset();
    m_uiVB.resource.Reset();
}

void ResourceHolder::Prepare(ID3D12GraphicsCommandList* commandList) noexcept
{
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_meshVB.view);
    commandList->IASetIndexBuffer(&m_meshIB.view);

    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");
    commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
    PIXEndEvent(commandList);
}

void ResourceHolder::PrepareUI(ID3D12GraphicsCommandList* commandList) noexcept
{
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_uiVB.view);

    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"UI Triangle");
    commandList->DrawInstanced(3, 1, 0, 0);
    PIXEndEvent(commandList);
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
    auto vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bytes);

    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    DX::ThrowIfFailed(device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &vertexBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(resource.GetAddressOf())
    ));

    // Copy vertex data to the UI vertex buffer
    UINT8* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);
    DX::ThrowIfFailed(resource->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, data, bytes);
    resource->Unmap(0, nullptr);

    return resource;
}