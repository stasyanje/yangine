//
// Game.cpp
//

#include "Pipeline.h"
#include "../common/AsyncLogger.h"
#include "../pch.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DX;

using Microsoft::WRL::ComPtr;

Pipeline::Pipeline(ID3D12Device* device)
{
    CreateSignature(device);
    CreatePSO(device);
}

void Pipeline::Prepare(ID3D12GraphicsCommandList* commandList)
{
    commandList->SetPipelineState(m_pipelineState.Get());
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());
}

void Pipeline::CreatePSO(ID3D12Device* device)
{
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;

    UINT compileFlags = 0;
#if defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    DX::ThrowIfFailed(
        D3DCompileFromFile(L"assets\\engine\\shaders\\TriangleVertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &vertexShader, nullptr)
    );
    DX::ThrowIfFailed(
        D3DCompileFromFile(L"assets\\engine\\shaders\\TrianglePixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &pixelShader, nullptr)
    );

    // Define the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
        };

    // Create graphics pipeline state
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = {inputElementDescs, _countof(inputElementDescs)};
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = m_bufferParams.format;
    psoDesc.SampleDesc.Count = 1;

    DX::ThrowIfFailed(
        device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)),
        "CreateTriangleResources: CreateGraphicsPipelineState"
    );
}

void Pipeline::CreateSignature(ID3D12Device* device)
{
    CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 /*t0*/, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

    CD3DX12_ROOT_PARAMETER1 params[2];
    params[0].InitAsConstantBufferView(0 /*b0*/, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC);
    params[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rsDesc;
    rsDesc.Init_1_1(_countof(params), params, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> sig, err;
    D3DX12SerializeVersionedRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &sig, &err);

    device->CreateRootSignature(
        0,
        sig->GetBufferPointer(),
        sig->GetBufferSize(),
        IID_PPV_ARGS(m_rootSignature.ReleaseAndGetAddressOf())
    );
}
