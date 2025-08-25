#pragma once

#include "../pch.h"
#include "BufferParams.h"
#include "DXGIFactory.h"
#include "Fence.h"
#include "Heaps.h"

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
    SwapChain(ID3D12Device*, DXGIFactory*, ID3D12CommandQueue*, SwapChainFallback*) noexcept;
    ~SwapChain() noexcept = default;

    void Reinitialize(HWND, int width, int height, bool isTearingAllowed, Heaps*) noexcept;
    void UpdateColorSpace(DXGI_COLOR_SPACE_TYPE);
    UINT GetCurrentBackBufferIndex();
    void Present(bool isTearingAllowed);
    void Clear(ID3D12GraphicsCommandList*, UINT backBufferIndex) noexcept;

private:
    BufferParams m_bufferParams{};

    ID3D12Device* m_device;
    DXGIFactory* m_dxgiFactory;
    ID3D12CommandQueue* m_commandQueue;
    SwapChainFallback* m_fallback;

    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
};

} // namespace DX