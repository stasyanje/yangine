#pragma once

#include "../pch.h"
#include "BufferParams.h"
#include "ResourceFactory.h"

namespace DX
{

class Heaps final
{
public:
    // Disallow copy / assign
    Heaps(const Heaps&) = delete;
    Heaps& operator=(const Heaps&) = delete;

    // - new / delete
    Heaps(ID3D12Device* device);
    ~Heaps() noexcept = default;

    // - get
    ID3D12Resource* RTarget(UINT backBufferIndex)
    {
        return m_renderTargets[backBufferIndex].Get();
    };

    // - init
    void Initialize(UINT width, UINT height, bool reverseDepth);
    void CreateRTargets(IDXGISwapChain*);

    // - prepare / present
    void Prepare(ID3D12GraphicsCommandList*, UINT backBufferIndex);

private:
    // - init
    void CreateDescriptorHeaps();
    void InitializeDSV(UINT width, UINT height, bool reverseDepth);

    CD3DX12_CPU_DESCRIPTOR_HANDLE RTVHandle(INT index) const
    {
        return CD3DX12_CPU_DESCRIPTOR_HANDLE(
            m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
            index,
            m_rtvDescriptorSize
        );
    }

    BufferParams m_bufferParams{};
    ID3D12Device* m_device;
    std::unique_ptr<device::ResourceFactory> m_resourceFactory;

    UINT m_rtvDescriptorSize = 0;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvDescriptorHeap;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[BufferParams::MAX_BACK_BUFFER_COUNT];
    Microsoft::WRL::ComPtr<ID3D12Resource> m_depthBuffer;
};

} // namespace DX