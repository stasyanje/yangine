//
// Game.h
//

#pragma once

#include "../device/BufferParams.h"
#include "../pch.h"

namespace pipeline
{

// A basic renderer implementation that creates a D3D12 device and
// provides rendering functionality.
class Store final
{
public:
    // Disallow copy / assign
    Store(const Store&) = delete;
    Store& operator=(const Store&) = delete;

    Store() noexcept = default;
    ~Store() noexcept = default;

    // - init
    void Initialize(ID3D12Device*);
    void Deinitialize();

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
} // namespace pipeline