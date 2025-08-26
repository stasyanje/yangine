//
// DeviceResources.cpp - A wrapper for the Direct3D 12 device and swapchain
//

#include "DeviceResources.h"
#include "../pch.h"

using namespace DirectX;
using namespace DX;
using namespace std;

using Microsoft::WRL::ComPtr;

#ifdef __clang__
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#pragma clang diagnostic ignored "-Wswitch-enum"
#endif

#pragma warning(disable : 4061)

// Constructor for DeviceResources.
DeviceResources::DeviceResources(window::WindowStateReducer* stateReducer) noexcept :
    m_stateReducer(stateReducer)
{
}

// Destructor for DeviceResources.
DeviceResources::~DeviceResources() noexcept
{
    // Ensure that the GPU is no longer referencing resources that are about to be destroyed.
    Flush();
}

// Configures the Direct3D device, and stores handles to it and the device context.
void DeviceResources::CreateDeviceResources()
{
    // Device init
    m_dxgiFactory = make_unique<DXGIFactory>();
    m_d3dDevice = std::move(m_dxgiFactory->CreateDevice());

    // Command queue init
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(
        m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_commandQueue.ReleaseAndGetAddressOf()))
    );

    m_commandQueue->SetName(L"DeviceResources");

    // Child components
    m_heaps = make_unique<Heaps>(m_d3dDevice.Get());
    m_fence = make_unique<Fence>(m_d3dDevice.Get(), m_commandQueue.Get());
    m_swapChain = make_unique<SwapChain>(m_d3dDevice.Get(), m_dxgiFactory.get(), m_commandQueue.Get(), this);
    m_commandList = make_unique<CommandList>(m_d3dDevice.Get());

    // Determines whether tearing support is available for fullscreen borderless windows.
    if ((m_options & c_AllowTearing) && !m_dxgiFactory->isTearingAllowed())
    {
        m_options &= ~c_AllowTearing;
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
    Flush();

    // Release resources that are tied to the swap chain and update fence values.
    for (UINT n = 0; n < m_bufferParams.count; n++)
    {
        m_fenceValues[n] = m_fenceValues[m_backBufferIndex];
    }

    auto resolutionScale = 1.0f;
    UINT width = std::max<UINT>(static_cast<UINT>(m_stateReducer->getWidth() * resolutionScale), 1u);
    UINT height = std::max<UINT>(static_cast<UINT>(m_stateReducer->getHeight() * resolutionScale), 1u);

    m_heaps->Initialize(width, height, m_options & c_ReverseDepth);

    m_swapChain->Reinitialize(
        m_window,
        width,
        height,
        m_options & c_AllowTearing,
        m_heaps.get()
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
void DeviceResources::Initialize(HWND window, IDeviceNotify* deviceNotify) noexcept
{
    m_window = window;
    m_deviceNotify = deviceNotify;
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

    m_fence.reset();
    m_commandList.reset();
    m_heaps.reset();
    m_swapChain.reset();
    m_dxgiFactory.reset();

    m_commandQueue.Reset();
    m_d3dDevice.Reset();

#ifdef _DEBUG
    {
        Microsoft::WRL::ComPtr<IDXGIDebug1> dxgiDebug;
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
ID3D12GraphicsCommandList* DeviceResources::Prepare()
{
    // Reset command list and allocator.
    auto commandList = m_commandList->Prepare(m_backBufferIndex);

    // Transition the render target into the correct state to allow for drawing into it.
    m_commandList->Sync(
        CD3DX12_RESOURCE_BARRIER::Transition(
            m_heaps->RTarget(m_backBufferIndex),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        )
    );

    // Set the viewport and scissor rect.
    commandList->RSSetViewports(1, &m_screenViewport);
    commandList->RSSetScissorRects(1, &m_scissorRect);

    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");
    m_heaps->Prepare(commandList, m_backBufferIndex);
    PIXEndEvent(commandList);

    return commandList;
}

// Present the contents of the swap chain to the screen.
void DeviceResources::Present()
{
    PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR_DEFAULT, L"Present");

    // Transition the render target to the state that allows it to be presented to the display.
    m_commandList->Sync(
        CD3DX12_RESOURCE_BARRIER::Transition(
            m_heaps->RTarget(m_backBufferIndex),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT
        )
    );

    auto commandList = m_commandList->Close();

    m_commandQueue->ExecuteCommandLists(1, CommandListCast(&commandList));

    m_swapChain->Present(m_options & c_AllowTearing);

    WaitUntilNextFrame();

    if (!m_dxgiFactory->IsCurrent())
    {
        UpdateColorSpace();
    }

    PIXEndEvent(m_commandQueue.Get());
}

// Wait for pending GPU work to complete.
void DeviceResources::Flush() noexcept
{
    m_fence->Signal(m_fenceValues[m_backBufferIndex]);
    m_fence->WaitForFenceValue(m_fenceValues[m_backBufferIndex]);
}

// Prepare to render the next frame.
void DeviceResources::WaitUntilNextFrame()
{
    auto currentValue = m_fenceValues[m_backBufferIndex];
    m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
    Flush();
    m_fenceValues[m_backBufferIndex] = currentValue + 1;
}

// Sets the color space for the swap chain in order to handle HDR output.
void DeviceResources::UpdateColorSpace()
{
    if (!m_dxgiFactory || !m_swapChain)
        return;

    // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
    if (!m_dxgiFactory->IsCurrent())
        m_dxgiFactory->Reinitialize();

    m_swapChain->UpdateColorSpace(
        m_dxgiFactory->ColorSpace(m_window, m_bufferParams.format, m_options & c_EnableHDR)
    );
}
