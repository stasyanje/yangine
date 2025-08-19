#pragma once

#include "../pch.h"

class Direct3DQueue final
{
public:
    Direct3DQueue(ID3D12Device* device);
    ~Direct3DQueue() noexcept = default;

    Direct3DQueue(Direct3DQueue&&) = default;
    Direct3DQueue& operator=(Direct3DQueue&&) = default;

    Direct3DQueue(Direct3DQueue const&) = delete;
    Direct3DQueue& operator=(Direct3DQueue const&) = delete;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    Microsoft::WRL::Wrappers::Event m_fenceEvent;

    void WaitForFence(UINT fenceValue);
};