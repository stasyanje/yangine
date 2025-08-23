#pragma once

#include "../pch.h"
#include "BufferParams.h"
#include "DXGIFactory.h"
#include "Direct3DQueue.h"
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
    SwapChain(ID3D12Device*, DXGIFactory*, Direct3DQueue*, SwapChainFallback*) noexcept;
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
    Direct3DQueue* m_d3dQueue;
    SwapChainFallback* m_fallback;

    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
};

} // namespace DX