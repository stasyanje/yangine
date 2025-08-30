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
    window::WindowStateReducer* m_stateReducer
) noexcept :
    m_inputController(inputController),
    m_stateReducer(m_stateReducer),
    m_shaderConstants(nullptr)
{
}

void ResourceHolder::Initialize(ID3D12Device* device)
{
    InitializeCamera(&m_camera, m_stateReducer->getAspectRatio());

    CreateVertexBuffer(device);
    CreateIndexBuffer(device);
    CreateConstantBuffer(device);
}

void ResourceHolder::Deinitialize() noexcept
{
    m_constantBuffer->Unmap(0, nullptr);
    m_shaderConstants = nullptr;
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
    m_constantBuffer.Reset();
}

void ResourceHolder::Prepare(ID3D12GraphicsCommandList* commandList, double totalTime) noexcept
{
    UpdateShaderConstants(totalTime);

    commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");
    commandList->IASetIndexBuffer(&m_indexBufferView);
    commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
    PIXEndEvent(commandList);
}

void ResourceHolder::CreateVertexBuffer(ID3D12Device* device)
{
    auto vertices = MakeCubeVertices();
    const UINT vertexBufferSize = sizeof(vertices);

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
    memcpy(pVertexDataBegin, vertices.data(), sizeof(vertices));
    m_vertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
    m_vertexBufferView.SizeInBytes = vertexBufferSize;
}

void ResourceHolder::CreateIndexBuffer(ID3D12Device* device)
{
    auto indices = MakeCubeIndices();
    const UINT indexBufferSize = sizeof(indices);

    // Create index buffer
    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);

    DX::ThrowIfFailed(
        device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &indexBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_indexBuffer.GetAddressOf())
        ),
        "CreateIndexBuffer: CreateCommittedResource"
    );

    // Copy index data to the index buffer
    UINT8* pIndexDataBegin;
    CD3DX12_RANGE readRange(0, 0);
    DX::ThrowIfFailed(
        m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)),
        "CreateIndexBuffer: m_indexBuffer->Map"
    );
    memcpy(pIndexDataBegin, indices.data(), sizeof(indices));
    m_indexBuffer->Unmap(0, nullptr);

    // Initialize the index buffer view
    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_indexBufferView.SizeInBytes = indexBufferSize;
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

void ResourceHolder::UpdateShaderConstants(double totalTime)
{
    m_camera.position.x = 15.0f * sinf(totalTime);
    m_camera.position.y = 15.0f * sinf(totalTime);
    m_camera.position.z = 15.0f * cosf(totalTime);

    // basic input
    static XMFLOAT2 currentMousePosition;

    auto mousePos = m_inputController->MousePositionNorm();

    if (mousePos.x != currentMousePosition.x || mousePos.y != currentMousePosition.y)
    {
        MoveCameraOnMouseMove(
            &m_camera,
            XMFLOAT2(mousePos.x - currentMousePosition.x, mousePos.y - currentMousePosition.y)
        );
        currentMousePosition = mousePos;
    }

    XMMATRIX M =
        XMMatrixScaling(1.0f, 1.0f, 1.0f) *
        XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f) *
        XMMatrixTranslation(0.0f, 0.0f, 0.0f);

    XMStoreFloat4x4(&m_shaderConstants->model, XMMatrixTranspose(M));
    XMStoreFloat4x4(&m_shaderConstants->viewProjection, XMMatrixTranspose(CameraViewProjection(m_camera)));
}