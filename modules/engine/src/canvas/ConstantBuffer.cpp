#include "ConstantBuffer.h"

using namespace DirectX;
using namespace canvas;

void ConstantBuffer::Initialize(ID3D12Device* device)
{
    CreateConstantBuffer(device);
}

void ConstantBuffer::Deinitialize() noexcept
{
    m_constantBuffer->Unmap(0, nullptr);
    m_shaderConstants = nullptr;
    m_constantBuffer.Reset();
}

void ConstantBuffer::Prepare(ID3D12GraphicsCommandList* commandList, Camera* camera, double totalTime)
{
    XMStoreFloat4x4(&m_shaderConstants->viewProjection, XMMatrixTranspose(camera->CameraViewProjection()));

    // Spin non-camera object
    double factor = 0.0; // sin(totalTime);

    XMMATRIX M =
        XMMatrixScaling(0.1f, 0.1f, 0.1f) *
        XMMatrixRotationRollPitchYaw(0.0f, 0.0f, XM_2PI * factor) *
        XMMatrixTranslation(0.0f, 0.0f, 0.0f);

    XMStoreFloat4x4(&m_shaderConstants->model, XMMatrixTranspose(M));

    // Write to command list
    commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress());
}

// MARK: - Private

void ConstantBuffer::CreateConstantBuffer(ID3D12Device* device)
{
    const UINT constantBufferSize = (sizeof(ShaderConstants) + 255) & ~255;

    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);

    DX::ThrowIfFailed(device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &constantBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(m_constantBuffer.GetAddressOf())
    ));

    CD3DX12_RANGE readRange(0, 0);
    DX::ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_shaderConstants)));
}
