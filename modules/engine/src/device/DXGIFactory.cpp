#include "DXGIFactory.h"
#include "../pch.h"

using Microsoft::WRL::ComPtr;
using namespace DX;

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

void DXGIFactory::LogGPUMemoryInfo(LUID adapterLuid)
{
    // Query video memory info for each memory segment
    DXGI_QUERY_VIDEO_MEMORY_INFO localMemoryInfo = {};
    DXGI_QUERY_VIDEO_MEMORY_INFO nonLocalMemoryInfo = {};

    // Get the DXGI adapter from the device
    ComPtr<IDXGIAdapter3> dxgiAdapter;

    ThrowIfFailed(m_dxgiFactory->EnumAdapterByLuid(adapterLuid, IID_PPV_ARGS(&dxgiAdapter)));

    // Query local video memory (dedicated GPU memory)
    ThrowIfFailed(dxgiAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &localMemoryInfo));

    // Query non-local video memory (system memory used by GPU)
    ThrowIfFailed(dxgiAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &nonLocalMemoryInfo));

    std::ostringstream vram;
    vram << "VRAM | " << localMemoryInfo.CurrentUsage / 1024 / 1024 << " / "
         << localMemoryInfo.Budget / 1024 / 1024 << " MB"
         << " | reserved: " << localMemoryInfo.CurrentReservation / 1024 / 1024 << " / "
         << localMemoryInfo.AvailableForReservation / 1024 / 1024 << " MB";

    std::ostringstream ram;
    ram << "RAM | " << nonLocalMemoryInfo.CurrentUsage / 1024 / 1024 << " / "
        << nonLocalMemoryInfo.Budget / 1024 / 1024 << " MB"
        << " | reserved: " << nonLocalMemoryInfo.CurrentReservation / 1024 / 1024 << " / "
        << nonLocalMemoryInfo.AvailableForReservation / 1024 / 1024 << " MB";

    std::cout << vram.str() << ram.str();
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

static constexpr D3D_FEATURE_LEVEL d3dMinFeatureLevel = D3D_FEATURE_LEVEL_11_0;

void DXGIFactory::CreateDevice(ID3D12Device** ppDevice)
{
    *ppDevice = nullptr;
    ComPtr<IDXGIAdapter1> adapter;
    GetAdapter(adapter.GetAddressOf(), d3dMinFeatureLevel);

    // Create the DX12 API device object.
    ComPtr<ID3D12Device> device;
    ThrowIfFailed(D3D12CreateDevice(
        adapter.Get(),
        d3dMinFeatureLevel,
        IID_PPV_ARGS(device.ReleaseAndGetAddressOf())
    ));

    device->SetName(L"DeviceResources");
    LogGPUMemoryInfo(device->GetAdapterLuid());

#ifndef NDEBUG
    // Configure debug device (if active).
    ComPtr<ID3D12InfoQueue> d3dInfoQueue;
    if (SUCCEEDED(device.As(&d3dInfoQueue)))
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

    // Check Shader Model 6 support
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {D3D_SHADER_MODEL_6_0};
    if (
        FAILED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))) ||
        (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0)
    )
    {
#ifdef _DEBUG
        OutputDebugStringA("ERROR: Shader Model 6.0 is not supported!\n");
#endif
        throw std::runtime_error("Shader Model 6.0 is not supported!");
    }

    *ppDevice = device.Detach();

    // If using the DirectX Tool Kit for DX12, uncomment this line:
    // m_graphicsMemory = std::make_unique<GraphicsMemory>(device);
    // TODO: Initialize device dependent objects here (independent of window size).
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

D3D_FEATURE_LEVEL DXGIFactory::D3DFeatureLevel(ID3D12Device* d3dDevice)
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

    auto hr = d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels));

    if (SUCCEEDED(hr))
    {
        return featLevels.MaxSupportedFeatureLevel;
    }
    else
    {
        return d3dMinFeatureLevel;
    }
}