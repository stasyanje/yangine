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
        Direct3DQueue*,
        DXGIFactory*,
        SwapChainFallback*
    ) noexcept;
    ~SwapChain() noexcept;

    void Reinitialize(HWND, int width, int height, bool isTearingAllowed) noexcept;
    void Deinitilize() noexcept; // on device lost

    UINT GetCurrentBackBufferIndex();
    HRESULT GetBuffer(UINT buffer, const IID& riid, void** ppSurface);
    void Present(bool isTearingAllowed);
    void UpdateColorSpace(DXGI_COLOR_SPACE_TYPE);

private:
    Direct3DQueue* m_d3dQueue;
    DXGIFactory* m_dxgiFactory;
    SwapChainFallback* m_fallback;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
};

} // namespace DX