//
// Game.h
//

#pragma once

#include "../pch.h"
#include "BufferParams.h"

namespace DX
{
// A basic renderer implementation that creates a D3D12 device and
// provides rendering functionality.
class Pipeline final
{
public:
    // Disallow copy / assign
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    Pipeline() noexcept = default;
    ~Pipeline() noexcept = default;

    // - init
    void Initialize(ID3D12Device*);

    // - frame
    void Prepare(ID3D12GraphicsCommandList*);

private:
    // - init
    void CreateSignature(ID3D12Device*);
    void CreatePSO(ID3D12Device*);

    DX::BufferParams m_bufferParams{};

    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
};
} // namespace DX