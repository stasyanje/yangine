#include "SwapChain.h"
#include "BufferParams.h"

#include "../pch.h"

using namespace DX;

SwapChain::SwapChain(
    ID3D12Device* device,
    DXGIFactory* dxgiFactory,
    Direct3DQueue* d3dQueue,
    SwapChainFallback* fallback
) noexcept :
    m_device(device),
    m_dxgiFactory(dxgiFactory),
    m_d3dQueue(d3dQueue),
    m_fallback(fallback)
{
    // Create descriptor heaps for render target views and depth stencil views.
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
    rtvDescriptorHeapDesc.NumDescriptors = m_bufferParams.count;
    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    ThrowIfFailed(device->CreateDescriptorHeap(
        &rtvDescriptorHeapDesc,
        IID_PPV_ARGS(m_rtvDescriptorHeap.ReleaseAndGetAddressOf())
    ));

    m_rtvDescriptorHeap->SetName(L"DeviceResources");

    m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    if (m_bufferParams.depthBufferFormat != DXGI_FORMAT_UNKNOWN)
    {
        D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
        dsvDescriptorHeapDesc.NumDescriptors = 1;
        dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

        ThrowIfFailed(device->CreateDescriptorHeap(
            &dsvDescriptorHeapDesc,
            IID_PPV_ARGS(m_dsvDescriptorHeap.ReleaseAndGetAddressOf())
        ));

        m_dsvDescriptorHeap->SetName(L"DeviceResources");
    }
}

void SwapChain::Reinitialize(HWND hwnd, int width, int height, bool isTearingAllowed, bool reverseDepth) noexcept
{
    BufferParams bufferParams = {};

    for (UINT n = 0; n < m_bufferParams.count; n++)
    {
        m_renderTargets[n].Reset();
    }

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

    CreateRTargets();
    InitializeDSV(width, height, reverseDepth);
}

UINT SwapChain::GetCurrentBackBufferIndex()
{
    return m_swapChain->GetCurrentBackBufferIndex();
}

void SwapChain::UpdateColorSpace(DXGI_COLOR_SPACE_TYPE colorSpace)
{
    UINT colorSpaceSupport = 0;
    ThrowIfFailed(m_swapChain->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport));

    if (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT)
        ThrowIfFailed(m_swapChain->SetColorSpace1(colorSpace));
}

void DX::SwapChain::InitializeDSV(UINT width, UINT height, bool reverseDepth)
{
    if (m_bufferParams.depthBufferFormat != DXGI_FORMAT_UNKNOWN)
    {
        // Allocate a 2-D surface as the depth/stencil buffer and create a depth/stencil view
        // on this surface.
        const CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

        D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            m_bufferParams.depthBufferFormat,
            width,
            height,
            1, // Use a single array entry.
            1  // Use a single mipmap level.
        );
        depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        const CD3DX12_CLEAR_VALUE depthOptimizedClearValue(
            m_bufferParams.depthBufferFormat,
            reverseDepth ? 0.0f : 1.0f,
            0u
        );

        ThrowIfFailed(m_device->CreateCommittedResource(
            &depthHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &depthStencilDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthOptimizedClearValue,
            IID_PPV_ARGS(m_depthStencil.ReleaseAndGetAddressOf())
        ));

        m_depthStencil->SetName(L"Depth stencil");

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = m_bufferParams.depthBufferFormat;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

        const auto cpuHandle = m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, cpuHandle);
    }
}

void SwapChain::Clear(ID3D12GraphicsCommandList* commandList, UINT backBufferIndex) noexcept
{
    // Clear the views.
    const auto rtvDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        static_cast<INT>(backBufferIndex),
        m_rtvDescriptorSize
    );
    const auto dsvDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
    );

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    commandList->ClearRenderTargetView(rtvDescriptor, DirectX::Colors::CornflowerBlue, 0, nullptr);
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void SwapChain::CreateRTargets()
{
    // Obtain the back buffers for this window which will be the final render targets
    // and create render target views for each of them.
    for (UINT n = 0; n < m_bufferParams.count; n++)
    {
        ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(m_renderTargets[n].GetAddressOf())));

        wchar_t name[25] = {};
        swprintf_s(name, L"Render target %u", n);
        m_renderTargets[n]->SetName(name);

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = m_bufferParams.format;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

        const auto cpuHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(cpuHandle, static_cast<INT>(n), m_rtvDescriptorSize);
        m_device->CreateRenderTargetView(m_renderTargets[n].Get(), &rtvDesc, rtvDescriptor);
    }
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
