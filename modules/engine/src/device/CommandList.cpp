#include "CommandList.h"
#include "../pch.h"

using namespace DX;

CommandList::CommandList(ID3D12Device* device)
{
    // Create a command allocator for each back buffer that will be rendered to.
    for (UINT n = 0; n < m_bufferParams.count; n++) {
        ThrowIfFailed(device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(m_commandAllocators[n].ReleaseAndGetAddressOf())
        ));

        wchar_t name[25] = {};
        swprintf_s(name, L"Render target %u", n);
        m_commandAllocators[n]->SetName(name);
    }

    // Create a command list for recording graphics commands.
    ThrowIfFailed(device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandAllocators[0].Get(),
        nullptr,
        IID_PPV_ARGS(m_commandList.ReleaseAndGetAddressOf())
    ));
    ThrowIfFailed(m_commandList->Close());

    m_commandList->SetName(L"DeviceResources");
}

CommandList::~CommandList() noexcept
{
    for (UINT n = 0; n < m_bufferParams.count; n++) {
        m_commandAllocators[n].Reset();
    }
    m_commandList.Reset();
}

ID3D12GraphicsCommandList* CommandList::Prepare(UINT backBufferIndex)
{
    auto commandAllocator = m_commandAllocators[backBufferIndex];

    ThrowIfFailed(commandAllocator->Reset());
    ThrowIfFailed(m_commandList->Reset(commandAllocator.Get(), nullptr));

    return m_commandList.Get();
}

void DX::CommandList::ResourceBarrier(D3D12_RESOURCE_BARRIER barrier)
{
    m_commandList->ResourceBarrier(1, &barrier);
}

ID3D12CommandList* CommandList::Close()
{
    // Send the command list off to the GPU for processing.
    ThrowIfFailed(m_commandList->Close());

    return m_commandList.Get();
}
