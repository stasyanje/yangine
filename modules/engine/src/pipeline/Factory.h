#pragma once

#include "../device/BufferParams.h"
#include "../pch.h"

namespace pipeline
{

struct ShaderDesc
{
    std::wstring filePath;
};

struct GraphicsPSODesc
{
    D3D12_CULL_MODE cullMode;
    D3D12_FILL_MODE fillMode;

    DX::BufferParams bufferParams;

    ShaderDesc vs;
    ShaderDesc ps;
};

class Factory
{
public:
    Microsoft::WRL::ComPtr<ID3D12PipelineState> CreateGraphicsPipeline(
        GraphicsPSODesc,
        ID3D12Device*,
        ID3D12RootSignature*
    );

private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC CreatePSODesc(GraphicsPSODesc);
    D3D12_INPUT_LAYOUT_DESC CreateInputLayout() const;

    enum class ShaderType
    {
        VS,
        PS
    };
    Microsoft::WRL::ComPtr<ID3DBlob> ShaderFromFile(ShaderType, std::wstring filePath);
};

} // namespace pipeline
