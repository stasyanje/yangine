#include "Factory.h"

using namespace pipeline;

using Microsoft::WRL::ComPtr;

ComPtr<ID3D12PipelineState> Factory::CreateGraphicsPipeline(
    GraphicsPSODesc desc,
    ID3D12Device* device,
    ID3D12RootSignature* rootSignature
)
{
    auto projectivePSODesc = CreatePSODesc(desc);

    projectivePSODesc.pRootSignature = rootSignature;

    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };
    projectivePSODesc.InputLayout = {inputElementDescs, _countof(inputElementDescs)};

    auto vs = ShaderFromFile(ShaderType::VS, desc.vs.filePath);
    auto ps = ShaderFromFile(ShaderType::PS, desc.ps.filePath);
    projectivePSODesc.VS = CD3DX12_SHADER_BYTECODE(vs.Get());
    projectivePSODesc.PS = CD3DX12_SHADER_BYTECODE(ps.Get());

    ComPtr<ID3D12PipelineState> pipeline;
    DX::ThrowIfFailed(device->CreateGraphicsPipelineState(
        &projectivePSODesc,
        IID_PPV_ARGS(&pipeline)
    ));

    return pipeline;
}

// MARK: - Private

D3D12_GRAPHICS_PIPELINE_STATE_DESC Factory::CreatePSODesc(GraphicsPSODesc desc)
{
    // Create graphics pipeline state
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = desc.bufferParams.format;
    psoDesc.SampleDesc.Count = 1;

    // Rasterizer
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.FillMode = desc.fillMode;
    psoDesc.RasterizerState.CullMode = desc.cullMode;

    // DSV
    if (desc.bufferParams.depthBufferFormat != DXGI_FORMAT_UNKNOWN)
    {
        psoDesc.DSVFormat = desc.bufferParams.depthBufferFormat;
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = TRUE;
        psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    }

    return psoDesc;
}

ComPtr<ID3DBlob> Factory::ShaderFromFile(ShaderType shaderType, std::wstring filePath)
{
    UINT compileFlags = 0;
#if defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    LPCSTR target;
    switch (shaderType)
    {
    case ShaderType::VS:
        target = "vs_5_0";
        break;
    case ShaderType::PS:
        target = "ps_5_0";
        break;
    }

    ComPtr<ID3DBlob> shader;
    DX::ThrowIfFailed(D3DCompileFromFile(
        (L"assets\\engine\\shaders\\" + filePath).c_str(),
        nullptr,
        nullptr,
        "main",
        target,
        compileFlags,
        0,
        &shader,
        nullptr
    ));

    return shader;
}