#pragma once

#include "../pch.h"
#include "BufferParams.h"

namespace DX
{
class CommandList final
{
public:
    // Disallow copy / assign
    CommandList(const CommandList&) = delete;
    CommandList& operator=(const CommandList&) = delete;

    CommandList(ID3D12Device*);
    ~CommandList() noexcept;

    ID3D12GraphicsCommandList* Prepare(UINT backBufferIndex);
    void ResourceBarrier(D3D12_RESOURCE_BARRIER);
    ID3D12CommandList* Close();

private:
    BufferParams m_bufferParams{};
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocators[BufferParams::MAX_BACK_BUFFER_COUNT];
};
} // namespace DX