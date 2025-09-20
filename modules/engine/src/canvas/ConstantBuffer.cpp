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

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::Prepare(
    const timer::Tick& tick,
    const DirectX::XMMATRIX& viewProjection
)
{
    XMStoreFloat4x4(&m_shaderConstants->viewProjection, XMMatrixTranspose(viewProjection));

    XMMATRIX M = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixTranslation(0.0f, 0.0f, 0.0f);
    XMStoreFloat4x4(&m_shaderConstants->model, XMMatrixTranspose(M));

    double pitch = XM_2PI * std::fmod(tick.totalTime, 1.0);

    XMStoreFloat4x4(
        &m_shaderConstants->modelRotated,
        XMMatrixTranspose(M * XMMatrixRotationRollPitchYaw(0.0, pitch, 0.0))
    );
    m_shaderConstants->time = static_cast<float>(tick.totalTime);

    return m_constantBuffer->GetGPUVirtualAddress();
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
