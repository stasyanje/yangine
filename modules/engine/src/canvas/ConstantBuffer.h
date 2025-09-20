#pragma once

#include "../common/GameTimer.h"
#include "../pch.h"
#include "Models.h"

namespace canvas
{

class ConstantBuffer final
{
public:
    ConstantBuffer(const ConstantBuffer&) = delete;
    ConstantBuffer& operator=(const ConstantBuffer&) = delete;

    ConstantBuffer() noexcept = default;
    ~ConstantBuffer() noexcept = default;

    void Initialize(ID3D12Device* device);
    void Deinitialize() noexcept;

    D3D12_GPU_VIRTUAL_ADDRESS Prepare(const timer::Tick&, const DirectX::XMMATRIX&);

private:
    void CreateConstantBuffer(ID3D12Device* device);

    ShaderConstants* m_shaderConstants = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_constantBuffer;
};

} // namespace canvas