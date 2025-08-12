#include "../pch.h"
#include "Direct3DQueue.h"

using Microsoft::WRL::ComPtr;

Direct3DQueue::Direct3DQueue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE commandType) :
    mCommandQueue(nullptr),
    mQueueType(commandType),
    mFence(nullptr),
    mFenceEvent(nullptr),
    mLastCompletedFenceValue(0)
{
    mQueueType = commandType;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = commandType;

    // remainings from website
    //  queueDesc.NodeMask = 0;

    DX::ThrowIfFailed(
        device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)),
        "Direct3DQueue: CreateCommandQueue"
    );

    DX::ThrowIfFailed(
        device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mFence.ReleaseAndGetAddressOf())),
        "Direct3DQueue: CreateFence"
    );

    mFence->SetName(L"Direct3DQueue::Fence");
    mCommandQueue->SetName(L"Direct3DQueue::CommandQueue");

    mFenceEvent.Attach(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
    if (!mFenceEvent.IsValid())
    {
        throw std::system_error(
            std::error_code(static_cast<int>(GetLastError()), std::system_category()),
            "Direct3DQueue: CreateEventEx"
        );
    }
}

Direct3DQueue::~Direct3DQueue()
{
    mFenceEvent.Close();

    mFence.Reset();
    mFence = nullptr;

    mCommandQueue.Reset();
    mCommandQueue = nullptr;
}

bool Direct3DQueue::IsFenceComplete(UINT fenceValue)
{
    if (fenceValue > mLastCompletedFenceValue)
    {
        PollCurrentFenceValue();
    }

    return fenceValue <= mLastCompletedFenceValue;
}

void Direct3DQueue::InsertWait(UINT fenceValue)
{
    mCommandQueue->Wait(mFence.Get(), fenceValue);
}

void Direct3DQueue::InsertWaitForQueueFence(Direct3DQueue* otherQueue, UINT fenceValue)
{
    mCommandQueue->Wait(otherQueue->GetFence(), fenceValue);
}

void Direct3DQueue::InsertWaitForQueue(Direct3DQueue* otherQueue)
{
    mCommandQueue->Wait(otherQueue->GetFence(), otherQueue->GetNextFenceValue() - 1);
}

UINT Direct3DQueue::WaitForFenceCPUBlocking(UINT fenceValue)
{
    if (IsFenceComplete(fenceValue))
        return fenceValue;

    {
        // std::lock_guard<std::mutex> lockGuard(mEventMutex);

        DX::ThrowIfFailed(mFence->SetEventOnCompletion(fenceValue, mFenceEvent.Get()));
        std::ignore = WaitForSingleObjectEx(mFenceEvent.Get(), INFINITE, FALSE);

        mNextFenceValue += 1;

        return mNextFenceValue;
    }
}

UINT Direct3DQueue::PollCurrentFenceValue()
{
    // std::lock_guard<std::mutex> lockGuard(mFenceMutex);

    mLastCompletedFenceValue = std::max(mLastCompletedFenceValue, static_cast<UINT>(mFence->GetCompletedValue()));
    return mLastCompletedFenceValue;
}

void Direct3DQueue::ExecuteCommandList(ID3D12CommandList* commandList)
{
    // std::lock_guard<std::mutex> lockGuard(mFenceMutex);
    DX::ThrowIfFailed(static_cast<ID3D12GraphicsCommandList*>(commandList)->Close());
    mCommandQueue->ExecuteCommandLists(1, &commandList);
}

void Direct3DQueue::Signal(UINT fenceValue)
{
    DX::ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), fenceValue));
}

void Direct3DQueue::QueryGPUMemoryInfo(ID3D12Device* d3dDevice)
{
    // Get the DXGI adapter from the device
    ComPtr<IDXGIAdapter3> dxgiAdapter;
    LUID adapterLuid = d3dDevice->GetAdapterLuid();

    DX::ThrowIfFailed(m_dxgiFactory->EnumAdapterByLuid(adapterLuid, IID_PPV_ARGS(&dxgiAdapter)));

    // Query video memory info for each memory segment
    DXGI_QUERY_VIDEO_MEMORY_INFO localMemoryInfo = {};
    DXGI_QUERY_VIDEO_MEMORY_INFO nonLocalMemoryInfo = {};

    DX::ThrowIfFailed(
        m_dxgiFactory->EnumAdapterByLuid(adapterLuid, IID_PPV_ARGS(&dxgiAdapter)),
        "Direct3DQueue: EnumAdapterByLuid"
    );

    // Query local video memory (dedicated GPU memory)
    DX::ThrowIfFailed(
        dxgiAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &localMemoryInfo),
        "Direct3DQueue: QueryVideoMemoryInfo LOCAL"
    );

    // Query non-local video memory (system memory used by GPU)
    DX::ThrowIfFailed(
        dxgiAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &nonLocalMemoryInfo),
        "Direct3DQueue: QueryVideoMemoryInfo NON_LOCAL"
    );

    // Create log entry
    std::ostringstream description;

    // Log the memory information
    description << "GPU Memory Info (Direct3DQueue):\n";
    description << "Local Memory (Dedicated GPU)\n";
    description << "  Budget: " << (localMemoryInfo.Budget / 1024 / 1024) << " MB\n";
    description << "  Current Usage: " << (localMemoryInfo.CurrentUsage / 1024 / 1024) << " MB\n";
    description << "  Available for Reservation: " << (localMemoryInfo.AvailableForReservation / 1024 / 1024) << " MB\n";
    description << "  Current Reservation: " << (localMemoryInfo.CurrentReservation / 1024 / 1024) << " MB\n";

    description << "Non-Local Memory (System):\n";
    description << "  Budget: " << (nonLocalMemoryInfo.Budget / 1024 / 1024) << " MB\n";
    description << "  Current Usage: " << (nonLocalMemoryInfo.CurrentUsage / 1024 / 1024) << " MB\n";
    description << "  Available for Reservation: " << (nonLocalMemoryInfo.AvailableForReservation / 1024 / 1024) << " MB\n";
    description << "  Current Reservation: " << (nonLocalMemoryInfo.CurrentReservation / 1024 / 1024) << " MB\n";

    std::cout << description.str();
}