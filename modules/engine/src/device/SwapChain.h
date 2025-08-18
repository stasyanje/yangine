#pragma once

#include "../pch.h"
#include "BufferParams.h"
#include "DXGIFactory.h"
#include "Direct3DQueue.h"

namespace DX
{
interface SwapChainFallback
{
    virtual void HandleDeviceLost() = 0;

protected:
    ~SwapChainFallback() = default;
};

class SwapChain final
{
public:
    SwapChain(
        ID3D12Device*,
        Direct3DQueue*,
        DXGIFactory*,
        SwapChainFallback*
    ) noexcept;
    ~SwapChain() noexcept;

    void Reinitialize(HWND, int width, int height, bool isTearingAllowed, bool reverseDepth) noexcept;
    void UpdateColorSpace(DXGI_COLOR_SPACE_TYPE);
    UINT GetCurrentBackBufferIndex();
    void Present(bool isTearingAllowed);
    void Clear(ID3D12GraphicsCommandList*, UINT backBufferIndex) noexcept;

    ID3D12Resource* RTarget(UINT backBufferIndex)
    {
        return m_renderTargets[backBufferIndex].Get();
    };

private:
    BufferParams m_bufferParams;
    ID3D12Device* m_device;
    Direct3DQueue* m_d3dQueue;
    DXGIFactory* m_dxgiFactory;
    SwapChainFallback* m_fallback;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[BufferParams::MAX_BACK_BUFFER_COUNT];
    Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencil;

    // Direct3D rendering objects.
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvDescriptorHeap;
    UINT m_rtvDescriptorSize;

    void InitializeDSV(UINT width, UINT height, bool reverseDepth);
    void CreateRTargets();
};

} // namespace DX