#include "../pch.h"
#include "Direct3DQueue.h"

class Direct3DQueueProvider
{
public:
    Direct3DQueueProvider();
    ~Direct3DQueueProvider();

    void Initialize(ID3D12Device* device);

    Direct3DQueue* GetGraphicsQueue() { return mGraphicsQueue.get(); }
    Direct3DQueue* GetComputeQueue() { return mComputeQueue.get(); }
    Direct3DQueue* GetCopyQueue() { return mCopyQueue.get(); }

    Direct3DQueue* GetQueue(D3D12_COMMAND_LIST_TYPE commandType);

    void WaitForAllIdle();

private:
    std::unique_ptr<Direct3DQueue> mGraphicsQueue;
    std::unique_ptr<Direct3DQueue> mComputeQueue;
    std::unique_ptr<Direct3DQueue> mCopyQueue;
};