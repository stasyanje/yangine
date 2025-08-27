//
// Game.cpp
//

#include "ResourceHolder.h"
#include "VertexFactory.h"

using namespace DirectX;
using namespace canvas;

using Microsoft::WRL::ComPtr;

ResourceHolder::ResourceHolder(input::InputController* inputController) noexcept :
    m_inputController(inputController)
{
}

void ResourceHolder::Initialize(ID3D12Device* device)
{
    CreateVertexBuffer(device);
}

void ResourceHolder::Deinitialize()
{
    m_vertexBuffer.Reset();
}

void ResourceHolder::Prepare(ID3D12GraphicsCommandList* commandList, double totalTime)
{
    UpdateTrianglePosition(totalTime);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");
    commandList->DrawInstanced(6, 1, 0, 0);
    PIXEndEvent(commandList);
}

void ResourceHolder::CreateVertexBuffer(ID3D12Device* device)
{
    VertexPosColor triangleVertices[] = {{}, {}, {}, {}, {}, {}};
    const UINT vertexBufferSize = sizeof(triangleVertices);

    // Create vertex buffer
    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

    DX::ThrowIfFailed(
        device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &vertexBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_vertexBuffer.GetAddressOf())
        ),
        "CreateTriangleResources: CreateCommittedResource"
    );

    // Copy vertex data to the vertex buffer
    UINT8* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);
    DX::ThrowIfFailed(
        m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)),
        "CreateTriangleResources: m_vertexBuffer->Map"
    );
    memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
    m_vertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(VertexPosColor);
    m_vertexBufferView.SizeInBytes = vertexBufferSize;
}

void ResourceHolder::UpdateTrianglePosition(double totalTime)
{
    if (!m_inputController || !m_vertexBuffer)
        return;

    auto mousePos = m_inputController->MousePositionNorm();

    auto triangleVertices = canvas::Pack(
        canvas::MakeTriangle(mousePos.x, mousePos.y),
        canvas::MakeTriangle(sin(totalTime * 2.0f) * 0.5f, 0.0)
    );

    // Update vertex buffer with new positions
    void* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);

    DX::ThrowIfFailed(
        m_vertexBuffer->Map(0, &readRange, &pVertexDataBegin),
        "UpdateTrianglePosition: m_vertexBuffer->Map"
    );

    memcpy(pVertexDataBegin, triangleVertices.data(), sizeof(triangleVertices));
    m_vertexBuffer->Unmap(0, nullptr);
}