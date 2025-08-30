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
    m_camera.aspectRatio = m_stateReducer->getAspectRatio();

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
    UpdateShaderConstants(totalTime);

    commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");
    commandList->DrawInstanced(36, 1, 0, 0);
    PIXEndEvent(commandList);
}

void ResourceHolder::CreateVertexBuffer(ID3D12Device* device)
{
    // Create a single triangle - we'll use instanced rendering for 100 copies
    auto triangle = MakeCube();
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
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
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