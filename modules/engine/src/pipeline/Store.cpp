//
// Game.cpp
//

#include "Store.h"
#include "../common/AsyncLogger.h"
#include "../pch.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace pipeline;

using Microsoft::WRL::ComPtr;

void Store::Initialize(ID3D12Device* device)
{
    CreateSignature(device);

    m_factory = std::make_unique<Factory>();

    // clang-format off
    m_graphicsPSO = m_factory->CreateGraphicsPipeline({
        D3D12_CULL_MODE_BACK,
        D3D12_FILL_MODE_SOLID,
        {}, 
        { L"Triangle_VS.hlsl" },
        { L"Triangle_PS.hlsl" }
    }, device, m_rootSignature.Get());

    m_uiPSO = m_factory->CreateGraphicsPipeline({
        D3D12_CULL_MODE_NONE,
        D3D12_FILL_MODE_SOLID,
        DX::BufferParams{ 1, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_UNKNOWN }, 
        { L"UI_VS.hlsl" },
        { L"UI_PS.hlsl" }
    }, device, m_rootSignature.Get());
    // clang-format on
}

void Store::Deinitialize()
{
    m_rootSignature.Reset();
    m_graphicsPSO.Reset();
    m_uiPSO.Reset();
    m_factory.reset();
}

void Store::Prepare(canvas::PSOType pso, ID3D12GraphicsCommandList* commandList)
{
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    switch (pso) {
    case canvas::PSOType::GRAPHICS:
        commandList->SetPipelineState(m_graphicsPSO.Get());
        break;
    case canvas::PSOType::UI:
        commandList->SetPipelineState(m_uiPSO.Get());
        break;
    }
}

// MARK: - Private

void Store::CreateSignature(ID3D12Device* device)
{
    CD3DX12_ROOT_PARAMETER1 params[1];
    params[0].InitAsConstantBufferView(0 /*b0*/, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rsDesc;
    rsDesc.Init_1_1(
        _countof(params),
        params,
        0,
        nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    ComPtr<ID3DBlob> sig, err;
    D3DX12SerializeVersionedRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &sig, &err);

    device->CreateRootSignature(
        0,
        sig->GetBufferPointer(),
        sig->GetBufferSize(),
        IID_PPV_ARGS(m_rootSignature.ReleaseAndGetAddressOf())
    );
}
