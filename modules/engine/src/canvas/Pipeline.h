//
// Game.h
//

#pragma once

#include "../device/DeviceResources.h"
#include "../input/InputController.h"
#include "../pch.h"
#include "ResourceHolder.h"

#include <memory>

namespace canvas
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

    void Initialize(ID3D12Device*);
    void Deinitialize();

    void Prepare(ID3D12GraphicsCommandList*);

private:
    // - init
    void CreateSignature(ID3D12Device*);
    void CreatePSO(ID3D12Device*);

    DX::BufferParams m_bufferParams{};

    // Pipeline
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
};
} // namespace canvas