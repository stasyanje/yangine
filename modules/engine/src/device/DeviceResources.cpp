//
// DeviceResources.cpp - A wrapper for the Direct3D 12 device and swapchain
//

#include "DeviceResources.h"
#include "../pch.h"

using namespace DirectX;
using namespace DX;

using Microsoft::WRL::ComPtr;

#ifdef __clang__
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#pragma clang diagnostic ignored "-Wswitch-enum"
#endif

#pragma warning(disable : 4061)

// Constructor for DeviceResources.
DeviceResources::DeviceResources(
    window::WindowStateReducer* stateReducer,
    D3D_FEATURE_LEVEL minFeatureLevel,
    unsigned int flags
) noexcept :
    m_bufferParams{},
    m_backBufferIndex(0),
    m_fenceValues{},
    m_rtvDescriptorSize(0),
    m_screenViewport{},
    m_scissorRect{},
    m_d3dMinFeatureLevel(minFeatureLevel),
    m_window(nullptr),
    m_d3dFeatureLevel(D3D_FEATURE_LEVEL_11_0),
    m_stateReducer(stateReducer),
    m_colorSpace(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709),
    m_options(flags),
    m_deviceNotify(nullptr)
{
}

// Destructor for DeviceResources.
DeviceResources::~DeviceResources()
{
    // Ensure that the GPU is no longer referencing resources that are about to be destroyed.
    WaitForGpu();
}

// Configures the Direct3D device, and stores handles to it and the device context.
void DeviceResources::CreateDeviceResources()
{
    m_dxgiFactory = std::make_unique<DXGIFactory>();

    // Determines whether tearing support is available for fullscreen borderless windows.
    if ((m_options & c_AllowTearing) && !m_dxgiFactory->isTearingAllowed())
    {
        m_options &= ~c_AllowTearing;
    }

    ComPtr<IDXGIAdapter1> adapter;
    m_dxgiFactory->GetAdapter(adapter.GetAddressOf(), m_d3dMinFeatureLevel);

    // Create the DX12 API device object.
    ThrowIfFailed(D3D12CreateDevice(
        adapter.Get(),
        m_d3dMinFeatureLevel,
        IID_PPV_ARGS(m_d3dDevice.ReleaseAndGetAddressOf())
    ));

    m_d3dDevice->SetName(L"DeviceResources");
    m_dxgiFactory->LogGPUMemoryInfo(m_d3dDevice->GetAdapterLuid());

#ifndef NDEBUG
    // Configure debug device (if active).
    ComPtr<ID3D12InfoQueue> d3dInfoQueue;
    if (SUCCEEDED(m_d3dDevice.As(&d3dInfoQueue)))
    {
#ifdef _DEBUG
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
#endif
        D3D12_MESSAGE_ID hide[] = {
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
            // Workarounds for debug layer issues on hybrid-graphics systems
            D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
        };
        D3D12_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
        filter.DenyList.pIDList = hide;
        d3dInfoQueue->AddStorageFilterEntries(&filter);
    }
#endif

    m_d3dQueue = std::make_unique<Direct3DQueue>(m_d3dDevice.Get());
    m_swapChain = std::make_unique<SwapChain>(
        m_d3dDevice.Get(),
        m_d3dQueue.get(),
        m_dxgiFactory.get(),
        this
    );

    // Create descriptor heaps for render target views and depth stencil views.
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
    rtvDescriptorHeapDesc.NumDescriptors = m_bufferParams.count;
    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(
        &rtvDescriptorHeapDesc,
        IID_PPV_ARGS(m_rtvDescriptorHeap.ReleaseAndGetAddressOf())
    ));

    m_rtvDescriptorHeap->SetName(L"DeviceResources");

    m_rtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    if (m_bufferParams.depthBufferFormat != DXGI_FORMAT_UNKNOWN)
    {
        D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
        dsvDescriptorHeapDesc.NumDescriptors = 1;
        dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

        ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(
            &dsvDescriptorHeapDesc,
            IID_PPV_ARGS(m_dsvDescriptorHeap.ReleaseAndGetAddressOf())
        ));

        m_dsvDescriptorHeap->SetName(L"DeviceResources");
    }

    // Create a command allocator for each back buffer that will be rendered to.
    for (UINT n = 0; n < m_bufferParams.count; n++)
    {
        ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(m_commandAllocators[n].ReleaseAndGetAddressOf())
        ));

        wchar_t name[25] = {};
        swprintf_s(name, L"Render target %u", n);
        m_commandAllocators[n]->SetName(name);
    }

    // Create a command list for recording graphics commands.
    ThrowIfFailed(m_d3dDevice->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandAllocators[0].Get(),
        nullptr,
        IID_PPV_ARGS(m_commandList.ReleaseAndGetAddressOf())
    ));
    ThrowIfFailed(m_commandList->Close());

    m_commandList->SetName(L"DeviceResources");
}

D3D_FEATURE_LEVEL DeviceResources::D3DFeatureLevel()
{
    // Determine maximum supported feature level for this device
    static const D3D_FEATURE_LEVEL s_featureLevels[] = {
#if defined(NTDDI_WIN10_FE) || defined(USING_D3D12_AGILITY_SDK)
        D3D_FEATURE_LEVEL_12_2,
#endif
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels = {
        static_cast<UINT>(std::size(s_featureLevels)),
        s_featureLevels,
        D3D_FEATURE_LEVEL_11_0
    };

    auto hr = m_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels));

    if (SUCCEEDED(hr))
    {
        return featLevels.MaxSupportedFeatureLevel;
    }
    else
    {
        return m_d3dMinFeatureLevel;
    }
}

// These resources need to be recreated every time the window size is changed.
void DeviceResources::CreateWindowSizeDependentResources()
{
    if (!m_window)
    {
        throw std::logic_error("Call SetWindow with a valid Win32 window handle");
    }

    // Wait until all previous GPU work is complete.
    WaitForGpu();

    // Release resources that are tied to the swap chain and update fence values.
    for (UINT n = 0; n < m_bufferParams.count; n++)
    {
        m_renderTargets[n].Reset();
        m_fenceValues[n] = m_fenceValues[m_backBufferIndex];
    }

    auto resolutionScale = 1.0f;
    UINT backBufferWidth = std::max<UINT>(static_cast<UINT>(m_stateReducer->getWidth() * resolutionScale), 1u);
    UINT backBufferHeight = std::max<UINT>(static_cast<UINT>(m_stateReducer->getHeight() * resolutionScale), 1u);

    m_swapChain->Reinitialize(m_window, backBufferWidth, backBufferHeight, m_options & c_AllowTearing);

    // Handle color space settings for HDR
    UpdateColorSpace();

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
        m_d3dDevice->CreateRenderTargetView(m_renderTargets[n].Get(), &rtvDesc, rtvDescriptor);
    }

    // Reset the index to the current back buffer.
    m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    if (m_bufferParams.depthBufferFormat != DXGI_FORMAT_UNKNOWN)
    {
        // Allocate a 2-D surface as the depth/stencil buffer and create a depth/stencil view
        // on this surface.
        const CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

        D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            m_bufferParams.depthBufferFormat,
            backBufferWidth,
            backBufferHeight,
            1, // Use a single array entry.
            1  // Use a single mipmap level.
        );
        depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        const CD3DX12_CLEAR_VALUE depthOptimizedClearValue(
            m_bufferParams.depthBufferFormat,
            (m_options & c_ReverseDepth) ? 0.0f : 1.0f,
            0u
        );

        ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
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
        m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, cpuHandle);
    }

    // Set the 3D rendering viewport and scissor rectangle to target the entire window.
    m_screenViewport.TopLeftX = m_screenViewport.TopLeftY = 0.f;
    m_screenViewport.Width = static_cast<float>(backBufferWidth);
    m_screenViewport.Height = static_cast<float>(backBufferHeight);
    m_screenViewport.MinDepth = D3D12_MIN_DEPTH;
    m_screenViewport.MaxDepth = D3D12_MAX_DEPTH;

    m_scissorRect.left = m_scissorRect.top = 0;
    m_scissorRect.right = static_cast<LONG>(backBufferWidth);
    m_scissorRect.bottom = static_cast<LONG>(backBufferHeight);
}

// This method is called when the Win32 window is created (or re-created).
void DeviceResources::Initialize(HWND window) noexcept
{
    m_window = window;
    CreateDeviceResources();
    CreateWindowSizeDependentResources();
}

// Recreate all device resources and set them back to the current state.
void DeviceResources::HandleDeviceLost()
{
    if (m_deviceNotify)
    {
        m_deviceNotify->OnDeviceLost();
    }

    for (UINT n = 0; n < m_bufferParams.count; n++)
    {
        m_commandAllocators[n].Reset();
        m_renderTargets[n].Reset();
    }

    m_depthStencil.Reset();
    m_d3dQueue->m_commandQueue.Reset();
    m_commandList.Reset();
    m_d3dQueue->m_fence.Reset();
    m_rtvDescriptorHeap.Reset();
    m_dsvDescriptorHeap.Reset();
    m_d3dDevice.Reset();
    m_dxgiFactory.reset();

#ifdef _DEBUG
    {
        ComPtr<IDXGIDebug1> dxgiDebug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
        {
            dxgiDebug->ReportLiveObjects(
                DXGI_DEBUG_ALL,
                DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL)
            );
        }
    }
#endif

    CreateDeviceResources();
    CreateWindowSizeDependentResources();

    if (m_deviceNotify)
    {
        m_deviceNotify->OnDeviceRestored();
    }
}

// Prepare the command list and render target for rendering.
void DeviceResources::Prepare(D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    // Reset command list and allocator.
    ThrowIfFailed(m_commandAllocators[m_backBufferIndex]->Reset());
    ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_backBufferIndex].Get(), nullptr));

    if (beforeState != afterState)
    {
        // Transition the render target into the correct state to allow for drawing into it.
        const D3D12_RESOURCE_BARRIER barrier =
            CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_backBufferIndex].Get(), beforeState, afterState);
        m_commandList->ResourceBarrier(1, &barrier);
    }
}

// Present the contents of the swap chain to the screen.
void DeviceResources::Present(D3D12_RESOURCE_STATES beforeState)
{
    if (beforeState != D3D12_RESOURCE_STATE_PRESENT)
    {
        // Transition the render target to the state that allows it to be presented to the display.
        const D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_backBufferIndex].Get(),
            beforeState,
            D3D12_RESOURCE_STATE_PRESENT
        );
        m_commandList->ResourceBarrier(1, &barrier);
    }

    // Send the command list off to the GPU for processing.
    ThrowIfFailed(m_commandList->Close());
    m_d3dQueue->m_commandQueue->ExecuteCommandLists(1, CommandListCast(m_commandList.GetAddressOf()));

    m_swapChain->Present(m_options & c_AllowTearing);

    MoveToNextFrame();

    if (!m_dxgiFactory->IsCurrent())
    {
        UpdateColorSpace();
    }
}

// Wait for pending GPU work to complete.
void DeviceResources::WaitForGpu() noexcept
{
    auto currentValue = m_fenceValues[m_backBufferIndex];
    m_d3dQueue->WaitForFence(currentValue);
    m_fenceValues[m_backBufferIndex] = currentValue + 1;
}

// Prepare to render the next frame.
void DeviceResources::MoveToNextFrame()
{
    // Schedule a Signal command in the queue.
    const UINT64 previousBufferValue = m_fenceValues[m_backBufferIndex];
    ThrowIfFailed(m_d3dQueue->m_commandQueue->Signal(m_d3dQueue->m_fence.Get(), previousBufferValue));

    m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    m_d3dQueue->WaitForFence(m_fenceValues[m_backBufferIndex]);
    m_fenceValues[m_backBufferIndex] = previousBufferValue + 1;
}

// Sets the color space for the swap chain in order to handle HDR output.
void DeviceResources::UpdateColorSpace()
{
    if (!m_dxgiFactory || !m_swapChain)
        return;

    // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
    if (!m_dxgiFactory->IsCurrent())
        m_dxgiFactory->ClearCache();

    m_colorSpace = m_dxgiFactory->ColorSpace(m_window, m_bufferParams.format, m_options & c_EnableHDR);

    m_swapChain->UpdateColorSpace(m_colorSpace);
}
