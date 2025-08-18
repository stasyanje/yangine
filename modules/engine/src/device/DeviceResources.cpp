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
DeviceResources::DeviceResources(window::WindowStateReducer* stateReducer) noexcept :
    m_bufferParams{},
    m_backBufferIndex(0),
    m_d3dQueue(nullptr),
    m_dxgiFactory(nullptr),
    m_fenceValues{},
    m_screenViewport{},
    m_scissorRect{},
    m_window(nullptr),
    m_stateReducer(stateReducer),
    m_options(0),
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
    m_dxgiFactory->CreateDevice(m_d3dDevice.ReleaseAndGetAddressOf());
    m_d3dQueue = std::make_unique<Direct3DQueue>(m_d3dDevice.Get());
    m_swapChain = std::make_unique<SwapChain>(m_d3dDevice.Get(), m_d3dQueue.get(), m_dxgiFactory.get(), this);

    // Determines whether tearing support is available for fullscreen borderless windows.
    if ((m_options & c_AllowTearing) && !m_dxgiFactory->isTearingAllowed())
    {
        m_options &= ~c_AllowTearing;
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
        m_fenceValues[n] = m_fenceValues[m_backBufferIndex];
    }

    auto resolutionScale = 1.0f;
    UINT width = std::max<UINT>(static_cast<UINT>(m_stateReducer->getWidth() * resolutionScale), 1u);
    UINT height = std::max<UINT>(static_cast<UINT>(m_stateReducer->getHeight() * resolutionScale), 1u);

    m_swapChain->Reinitialize(
        m_window,
        width,
        height,
        m_options & c_AllowTearing,
        m_options & c_ReverseDepth
    );

    // Handle color space settings for HDR
    UpdateColorSpace();

    // Reset the index to the current back buffer.
    m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Set the 3D rendering viewport and scissor rectangle to target the entire window.
    m_screenViewport.TopLeftX = m_screenViewport.TopLeftY = 0.f;
    m_screenViewport.Width = static_cast<float>(width);
    m_screenViewport.Height = static_cast<float>(height);
    m_screenViewport.MinDepth = D3D12_MIN_DEPTH;
    m_screenViewport.MaxDepth = D3D12_MAX_DEPTH;

    m_scissorRect.left = m_scissorRect.top = 0;
    m_scissorRect.right = static_cast<LONG>(width);
    m_scissorRect.bottom = static_cast<LONG>(height);
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
    }

    m_d3dQueue->m_commandQueue.Reset();
    m_commandList.Reset();
    m_d3dQueue->m_fence.Reset();
    m_d3dDevice.Reset();
    m_dxgiFactory.reset();
    m_swapChain.reset();

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
        const D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_swapChain->RTarget(m_backBufferIndex),
            beforeState,
            afterState
        );
        m_commandList->ResourceBarrier(1, &barrier);
    }

    Clear();
}

void DeviceResources::Clear()
{
    PIXBeginEvent(m_commandList.Get(), PIX_COLOR_DEFAULT, L"Clear");

    m_swapChain->Clear(m_commandList.Get(), m_backBufferIndex);

    // Set the viewport and scissor rect.
    m_commandList->RSSetViewports(1, &m_screenViewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    PIXEndEvent(m_commandList.Get());
}

// Present the contents of the swap chain to the screen.
void DeviceResources::Present(D3D12_RESOURCE_STATES beforeState)
{
    if (beforeState != D3D12_RESOURCE_STATE_PRESENT)
    {
        // Transition the render target to the state that allows it to be presented to the display.
        const D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_swapChain->RTarget(m_backBufferIndex),
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

    m_swapChain->UpdateColorSpace(
        m_dxgiFactory->ColorSpace(m_window, m_bufferParams.format, m_options & c_EnableHDR)
    );
}
