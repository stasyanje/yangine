#include "Direct3DQueueProvider.h"

Direct3DQueueProvider::Direct3DQueueProvider()
{
}

Direct3DQueueProvider::~Direct3DQueueProvider()
{
    mGraphicsQueue.reset();
    mGraphicsQueue = nullptr;

    mComputeQueue.reset();
    mComputeQueue = nullptr;

    mCopyQueue.reset();
    mCopyQueue = nullptr;
}

void Direct3DQueueProvider::Initialize(ID3D12Device* device)
{
    mGraphicsQueue = std::make_unique<Direct3DQueue>(device, D3D12_COMMAND_LIST_TYPE_DIRECT);
    mComputeQueue = std::make_unique<Direct3DQueue>(device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
    mCopyQueue = std::make_unique<Direct3DQueue>(device, D3D12_COMMAND_LIST_TYPE_COPY);
}

Direct3DQueue* Direct3DQueueProvider::GetQueue(D3D12_COMMAND_LIST_TYPE commandType)
{
    switch (commandType)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        return mGraphicsQueue.get();
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        return mComputeQueue.get();
    case D3D12_COMMAND_LIST_TYPE_COPY:
        return mCopyQueue.get();
    default:
        DX::Throw("Bad command type lookup in queue manager.");
    }

    return NULL;
}

void Direct3DQueueProvider::WaitForAllIdle()
{
    mGraphicsQueue->WaitForIdle();
    mComputeQueue->WaitForIdle();
    mCopyQueue->WaitForIdle();
}