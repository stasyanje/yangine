#pragma once

#include "../pch.h"

class Direct3DQueue
{
public:
    Direct3DQueue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE commandType);
    ~Direct3DQueue();

    bool IsFenceComplete(UINT fenceValue);
    void InsertWait(UINT fenceValue);
    void InsertWaitForQueueFence(Direct3DQueue* otherQueue, UINT fenceValue);
    void InsertWaitForQueue(Direct3DQueue* otherQueue);

    UINT WaitForFenceCPUBlocking(UINT fenceValue);
    void WaitForIdle() { WaitForFenceCPUBlocking(mNextFenceValue - 1); }

    ID3D12CommandQueue* GetCommandQueue() { return mCommandQueue.Get(); }

    UINT PollCurrentFenceValue();
    UINT GetLastCompletedFence() { return mLastCompletedFenceValue; }

    UINT GetNextFenceValue() { return mNextFenceValue; }
    ID3D12Fence* GetFence() { return mFence.Get(); }

    void ExecuteCommandList(ID3D12CommandList* List);
    void Signal(UINT fenceValue);

    void QueryGPUMemoryInfo(ID3D12Device*);

private:
    D3D12_COMMAND_LIST_TYPE mQueueType;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
    Microsoft::WRL::ComPtr<IDXGIFactory6> m_dxgiFactory;

    // std::mutex mFenceMutex;
    // std::mutex mEventMutex;

    // Presentation fence objects.
    UINT mNextFenceValue;
    UINT mLastCompletedFenceValue;
    Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
    Microsoft::WRL::Wrappers::Event mFenceEvent;
};