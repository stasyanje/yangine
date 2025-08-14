#include "DXGIFactory.h"
#include "../pch.h"

using Microsoft::WRL::ComPtr;

namespace
{
inline long ComputeIntersectionArea(
    long ax1, long ay1, long ax2, long ay2, long bx1, long by1, long bx2, long by2
) noexcept
{
    return std::max(0l, std::min(ax2, bx2) - std::max(ax1, bx1)) *
           std::max(0l, std::min(ay2, by2) - std::max(ay1, by1));
}
} // namespace

DXGIFactory::DXGIFactory() noexcept :
    m_dxgiFactoryFlags(0)
{
#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    //
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
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

    DX::ThrowIfFailed(CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(m_dxgiFactory.ReleaseAndGetAddressOf())));
}

DXGIFactory::~DXGIFactory()
{
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

void DXGIFactory::ClearCache()
{
    DX::ThrowIfFailed(
        CreateDXGIFactory2(
            m_dxgiFactoryFlags,
            IID_PPV_ARGS(m_dxgiFactory.ReleaseAndGetAddressOf())
        )
    );
}

// This method acquires the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, try WARP. Otherwise throw an exception.
void DXGIFactory::GetAdapter(IDXGIAdapter1** ppAdapter, D3D_FEATURE_LEVEL d3dMinFeatureLevel)
{
    *ppAdapter = nullptr;

    ComPtr<IDXGIAdapter1> adapter;
    for (
        UINT adapterIndex = 0;
        SUCCEEDED(m_dxgiFactory->EnumAdapterByGpuPreference(
            adapterIndex,
            DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
            IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf())
        ));
        adapterIndex++
    )
    {
        DXGI_ADAPTER_DESC1 desc;
        DX::ThrowIfFailed(adapter->GetDesc1(&desc));

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // Don't select the Basic Render Driver adapter.
            continue;
        }

        // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), d3dMinFeatureLevel, __uuidof(ID3D12Device), nullptr)))
        {
#ifdef _DEBUG
            wchar_t buff[256] = {};
            swprintf_s(
                buff,
                L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n",
                adapterIndex,
                desc.VendorId,
                desc.DeviceId,
                desc.Description
            );
            OutputDebugStringW(buff);
#endif
            break;
        }
    }

#if !defined(NDEBUG)
    if (!adapter)
    {
        // Try WARP12 instead
        if (FAILED(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()))))
        {
            throw std::runtime_error("WARP12 not available. Enable the 'Graphics Tools' optional feature");
        }

        OutputDebugStringA("Direct3D Adapter - WARP12\n");
    }
#endif

    if (!adapter)
    {
        throw std::runtime_error("No Direct3D 12 device found");
    }

    *ppAdapter = adapter.Detach();
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

    if (!isDisplayHDR10(windowBounds))
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

bool DXGIFactory::isDisplayHDR10(RECT windowBounds)
{
    // To detect HDR support, we will need to check the color space in the primary
    // DXGI output associated with the app at this point in time
    // (using window/display intersection).

    const long ax1 = windowBounds.left;
    const long ay1 = windowBounds.top;
    const long ax2 = windowBounds.right;
    const long ay2 = windowBounds.bottom;

    ComPtr<IDXGIOutput> bestOutput;
    long bestIntersectArea = -1;

    ComPtr<IDXGIAdapter> adapter;
    for (UINT adapterIndex = 0;
         SUCCEEDED(m_dxgiFactory->EnumAdapters(adapterIndex, adapter.ReleaseAndGetAddressOf()));
         ++adapterIndex)
    {
        ComPtr<IDXGIOutput> output;
        for (
            UINT outputIndex = 0;
            SUCCEEDED(adapter->EnumOutputs(outputIndex, output.ReleaseAndGetAddressOf()));
            ++outputIndex
        )
        {
            // Get the rectangle bounds of current output.
            DXGI_OUTPUT_DESC desc;
            DX::ThrowIfFailed(output->GetDesc(&desc));
            const auto& r = desc.DesktopCoordinates;

            // Compute the intersection
            const long intersectArea = ComputeIntersectionArea(
                ax1,
                ay1,
                ax2,
                ay2,
                r.left,
                r.top,
                r.right,
                r.bottom
            );
            if (intersectArea > bestIntersectArea)
            {
                bestOutput.Swap(output);
                bestIntersectArea = intersectArea;
            }
        }
    }

    if (!bestOutput)
        return false;

    ComPtr<IDXGIOutput6> output6;
    if (SUCCEEDED(bestOutput.As(&output6)))
    {
        DXGI_OUTPUT_DESC1 desc;
        DX::ThrowIfFailed(output6->GetDesc1(&desc));

        if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
        {
            // Display output is HDR10.
            return true;
        }
    }

    return false;
}