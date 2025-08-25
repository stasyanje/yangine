#pragma once

#include "../pch.h"

class Fence final
{
public:
    Fence(ID3D12Device*, ID3D12CommandQueue*);
    ~Fence() noexcept = default;

    void Signal(UINT);
    void WaitForFenceValue(UINT);

private:
    ID3D12CommandQueue* m_commandQueue;

    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    Microsoft::WRL::Wrappers::Event m_fenceEvent;
};