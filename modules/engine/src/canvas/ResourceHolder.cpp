//
// Game.cpp
//

#include "ResourceHolder.h"
#include "VertexFactory.h"

using namespace DirectX;
using namespace canvas;

using Microsoft::WRL::ComPtr;

ResourceHolder::ResourceHolder(
    input::InputController* inputController,
    window::WindowStateReducer* stateReducer
) noexcept :
    m_camera(std::make_unique<Camera>(inputController, stateReducer)),
    m_constantBuffer(std::make_unique<ConstantBuffer>())
{
}

void ResourceHolder::Initialize(ID3D12Device* device)
{
    // CB
    m_constantBuffer->Initialize(device);

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
    m_constantBuffer->Deinitialize();
    m_meshVB.resource.Reset();
    m_meshIB.resource.Reset();
    m_uiVB.resource.Reset();
}

std::vector<DrawItem> ResourceHolder::CreateDrawItems(const timer::Tick& tick) noexcept
{
    m_camera->Prepare(tick);

    DrawItem graphics;
    graphics.psoType = PSOType::GRAPHICS;
    graphics.vsCB = m_constantBuffer->Prepare(tick, m_camera->CameraViewProjection());
    graphics.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    graphics.vbv = m_meshVB.view;
    graphics.ibv = m_meshIB.view;
    graphics.countPerInstance = 36;
    graphics.instanceCount = 7;

    DrawItem ui;
    ui.psoType = PSOType::UI;
    ui.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    ui.vbv = m_uiVB.view;
    ui.countPerInstance = 3;
    ui.instanceCount = 1;

    std::vector<DrawItem> drawItems;

    drawItems.push_back(graphics);
    drawItems.push_back(ui);

    return drawItems;
};

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