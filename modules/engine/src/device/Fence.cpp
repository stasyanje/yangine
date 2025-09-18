#include "Fence.h"
#include "../pch.h"

using namespace DX;

Fence::Fence(ID3D12Device* d3dDevice, ID3D12CommandQueue* commandQueue) :
    m_commandQueue(commandQueue)
{
    // Create a fence for tracking GPU execution progress.
    ThrowIfFailed(
        d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf()))
    );

    m_fence->SetName(L"DeviceResources");

    m_fenceEvent.Attach(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
    if (!m_fenceEvent.IsValid()) {
        throw std::system_error(
            std::error_code(static_cast<int>(GetLastError()), std::system_category()),
            "CreateEventEx"
        );
    }
}

void Fence::Signal(UINT fenceValue)
{
    DX::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fenceValue));
}

void Fence::WaitForFenceValue(UINT fenceValue)
{
    assert(m_commandQueue && m_fence && m_fenceEvent.IsValid());

    if (m_fence->GetCompletedValue() >= fenceValue)
        return;

    // Wait until the Signal has been processed.
    ThrowIfFailed(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent.Get()));
    std::ignore = WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);
}