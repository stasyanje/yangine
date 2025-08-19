#include "DXGIFactory.h"
#include "../pch.h"

using Microsoft::WRL::ComPtr;
using namespace DX;

DXGIFactory::DXGIFactory() noexcept
{
    InitializeDebugLayer();
    Reinitialize();
}

bool DXGIFactory::IsCurrent()
{
    return m_dxgiFactory->IsCurrent();
}

bool DXGIFactory::isTearingAllowed()
{
    BOOL allowTearing = FALSE;
    HRESULT hr = m_dxgiFactory->CheckFeatureSupport(
        DXGI_FEATURE_PRESENT_ALLOW_TEARING,
        &allowTearing,
        sizeof(allowTearing)
    );

    if (FAILED(hr) || !allowTearing)
    {
#ifdef _DEBUG
        OutputDebugStringA("WARNING: Variable refresh rate displays not supported");
#endif
        return false;
    }

    return allowTearing;
}

void DXGIFactory::Reinitialize()
{
    DX::ThrowIfFailed(
        CreateDXGIFactory2(
            m_dxgiFactoryFlags,
            IID_PPV_ARGS(m_dxgiFactory.ReleaseAndGetAddressOf())
        )
    );
    m_dxgiAdapter = std::make_unique<DXGIAdapter>();
}

DXGI_COLOR_SPACE_TYPE DXGIFactory::ColorSpace(HWND window, DXGI_FORMAT backBufferFormat, bool isHDREnabled)
{
    DXGI_COLOR_SPACE_TYPE defaultColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

    if (isHDREnabled)
        return defaultColorSpace;

    // Get the retangle bounds of the app window.
    RECT windowBounds;
    if (!GetWindowRect(window, &windowBounds))
        throw std::system_error(
            std::error_code(static_cast<int>(GetLastError()), std::system_category()),
            "GetWindowRect"
        );

    if (!m_dxgiAdapter->isDisplayHDR10(windowBounds))
        return defaultColorSpace;

    switch (backBufferFormat)
    {
    case DXGI_FORMAT_R10G10B10A2_UNORM:
        // The application creates the HDR10 signal.
        return DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;

    case DXGI_FORMAT_R16G16B16A16_FLOAT:
        // The system creates the HDR10 signal; application uses linear values.
        return DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;

    default:
        return defaultColorSpace;
    }
}

Microsoft::WRL::ComPtr<IDXGISwapChain1> DXGIFactory::CreateSwapChain(
    HWND window,
    ID3D12CommandQueue* commandQueue,
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc
)
{
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
    fsSwapChainDesc.Windowed = TRUE;

    ComPtr<IDXGISwapChain1> swapChain;
    DX::ThrowIfFailed(m_dxgiFactory->CreateSwapChainForHwnd(
        commandQueue,
        window,
        &swapChainDesc,
        &fsSwapChainDesc,
        nullptr,
        swapChain.GetAddressOf()
    ));

    // This class does not support exclusive full-screen mode and prevents DXGI from responding
    // to the ALT+ENTER shortcut
    DX::ThrowIfFailed(m_dxgiFactory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER));

    return swapChain;
}

// Enable the debug layer (requires the Graphics Tools "optional feature").
// NOTE: Enabling the debug layer after device creation will invalidate the active device.
void DXGIFactory::InitializeDebugLayer()
{
#if defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()))))
        {
            debugController->EnableDebugLayer();
        }
        else
        {
            OutputDebugStringA("WARNING: Direct3D Debug Device is not available\n");
        }

        ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
        {
            m_dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

            DXGI_INFO_QUEUE_MESSAGE_ID hide[] = {
                80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control
                      the output on which the swapchain's window resides. */
                ,
            };
            DXGI_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
            filter.DenyList.pIDList = hide;
            dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
        }
    }
#endif
}