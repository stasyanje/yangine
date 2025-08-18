#include "Direct3DQueue.h"
#include "../pch.h"

Direct3DQueue::Direct3DQueue(ID3D12Device* d3dDevice)
{
    // Create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    DX::ThrowIfFailed(
        d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_commandQueue.ReleaseAndGetAddressOf()))
    );

    m_commandQueue->SetName(L"DeviceResources");

    // Create a fence for tracking GPU execution progress.
    DX::ThrowIfFailed(
        d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf())
    ));

    m_fence->SetName(L"DeviceResources");

    m_fenceEvent.Attach(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
    if (!m_fenceEvent.IsValid())
    {
        throw std::system_error(
            std::error_code(static_cast<int>(GetLastError()), std::system_category()),
            "CreateEventEx"
        );
    }
}

Direct3DQueue::~Direct3DQueue()
{
    m_commandQueue.Reset();
    m_fence.Reset();
}

void Direct3DQueue::WaitForFence(UINT fenceValue) noexcept
{
    if (!m_commandQueue || !m_fence || !m_fenceEvent.IsValid())
        return;

    if (m_fence->GetCompletedValue() >= fenceValue)
        return;

    // Schedule a Signal command in the GPU queue.
    DX::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fenceValue));

    // Wait until the Signal has been processed.
    DX::ThrowIfFailed(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent.Get()));
    std::ignore = WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);
}
