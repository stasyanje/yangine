//
// Game.cpp
//

#include "ResourceHolder.h"
#include "VertexFactory.h"

using namespace DirectX;
using namespace canvas;

using Microsoft::WRL::ComPtr;

ResourceHolder::ResourceHolder(input::InputController* inputController) noexcept :
    m_inputController(inputController),
    m_shaderConstants(nullptr)
{
}

void ResourceHolder::Initialize(ID3D12Device* device)
{
    CreateVertexBuffer(device);
    CreateConstantBuffer(device);
}

void ResourceHolder::Deinitialize() noexcept
{
    m_constantBuffer->Unmap(0, nullptr);
    m_shaderConstants = nullptr;
    m_vertexBuffer.Reset();
    m_constantBuffer.Reset();
}

void ResourceHolder::Prepare(ID3D12GraphicsCommandList* commandList, double totalTime) noexcept
{
    // provide shader with updated values
    auto mousePos = m_inputController->MousePositionNorm();
    m_shaderConstants->mousePos[0] = mousePos.x;
    m_shaderConstants->mousePos[1] = mousePos.y;
    m_shaderConstants->time = totalTime;

    commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");
    commandList->DrawInstanced(3, 30, 0, 0);
    PIXEndEvent(commandList);
}

void ResourceHolder::CreateVertexBuffer(ID3D12Device* device)
{
    // Create a single triangle - we'll use instanced rendering for 100 copies
    auto triangle = canvas::MakeTriangle(0.0f, 0.0f);
    const UINT vertexBufferSize = sizeof(triangle);

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
    memcpy(pVertexDataBegin, triangle.data(), sizeof(triangle));
    m_vertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(VertexPosColor);
    m_vertexBufferView.SizeInBytes = vertexBufferSize;
}

void ResourceHolder::CreateConstantBuffer(ID3D12Device* device)
{
    const UINT constantBufferSize = (sizeof(ShaderConstants) + 255) & ~255;

    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);

    DX::ThrowIfFailed(
        device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &constantBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_constantBuffer.GetAddressOf())
        ),
        "CreateConstantBuffer: CreateCommittedResource"
    );

    // Map the constant buffer
    CD3DX12_RANGE readRange(0, 0);
    DX::ThrowIfFailed(
        m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_shaderConstants)),
        "CreateConstantBuffer: m_constantBuffer->Map"
    );
}