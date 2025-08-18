#include "SwapChain.h"
#include "BufferParams.h"

#include "../pch.h"

using namespace DX;

SwapChain::SwapChain(
    Direct3DQueue* d3dQueue,
    DXGIFactory* dxgiFactory,
    SwapChainFallback* fallback
) noexcept :
    m_d3dQueue(d3dQueue),
    m_dxgiFactory(dxgiFactory),
    m_fallback(fallback),
    m_swapChain(nullptr)
{
}

SwapChain::~SwapChain() noexcept
{
    Deinitilize();
}

void SwapChain::Deinitilize() noexcept
{
    m_swapChain.Reset();
}

void SwapChain::Reinitialize(HWND hwnd, int width, int height, bool isTearingAllowed) noexcept
{
    BufferParams bufferParams = {};

    // If the swap chain already exists, resize it, otherwise create one.
    if (m_swapChain)
    {
        // If the swap chain already exists, resize it.
        HRESULT hr = m_swapChain->ResizeBuffers(
            bufferParams.count,
            width,
            height,
            bufferParams.format,
            isTearingAllowed ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u
        );

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
#ifdef _DEBUG
            char buff[64] = {};
            sprintf_s(
                buff,
                "Device Lost on ResizeBuffers: Reason code 0x%08X\n",
                static_cast<unsigned int>(
                    (hr == DXGI_ERROR_DEVICE_REMOVED) ? 999 : hr
                )
            );
            OutputDebugStringA(buff);
#endif
            // If the device was removed for any reason, a new device and swap chain will need to be
            // created.
            m_fallback->HandleDeviceLost();

            // Everything is set up now. Do not continue execution of this method. HandleDeviceLost
            // will reenter this method and correctly set up the new device.
            return;
        }
        else
        {
            ThrowIfFailed(hr);
        }
    }
    else
    {
        // Create a swap chain for the window.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = width;
        swapChainDesc.Height = height;
        swapChainDesc.Format = bufferParams.format;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = bufferParams.count;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        swapChainDesc.Flags = isTearingAllowed ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

        auto swapChain = m_dxgiFactory->CreateSwapChain(hwnd, m_d3dQueue->m_commandQueue.Get(), swapChainDesc);
        ThrowIfFailed(swapChain.As(&m_swapChain));
    }
}

UINT SwapChain::GetCurrentBackBufferIndex()
{
    return m_swapChain->GetCurrentBackBufferIndex();
}

HRESULT SwapChain::GetBuffer(UINT buffer, const IID& riid, void** ppSurface)
{
    return m_swapChain->GetBuffer(buffer, riid, ppSurface);
}

void SwapChain::UpdateColorSpace(DXGI_COLOR_SPACE_TYPE colorSpace)
{
    UINT colorSpaceSupport = 0;
    ThrowIfFailed(m_swapChain->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport));

    if (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT)
        ThrowIfFailed(m_swapChain->SetColorSpace1(colorSpace));
}

void SwapChain::Present(bool isTearingAllowed)
{
    HRESULT hr;
    if (isTearingAllowed)
    {
        // Recommended to always use tearing if supported when using a sync interval of 0.
        // Note this will fail if in true 'fullscreen' mode.
        hr = m_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
    }
    else
    {
        // The first argument instructs DXGI to block until VSync, putting the application
        // to sleep until the next VSync. This ensures we don't waste any cycles rendering
        // frames that will never be displayed to the screen.
        hr = m_swapChain->Present(1, 0);
    }

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
#ifdef _DEBUG
        char buff[64] = {};
        sprintf_s(
            buff,
            "Device Lost on Present: Reason code 0x%08X\n",
            static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? 999 : hr)
        );
        OutputDebugStringA(buff);
#endif
        m_fallback->HandleDeviceLost();
    }
    else
    {
        ThrowIfFailed(hr);
    }
}
