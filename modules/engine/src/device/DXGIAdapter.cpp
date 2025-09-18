#include "DXGIAdapter.h"
#include "../pch.h"

using Microsoft::WRL::ComPtr;
using namespace DX;

namespace
{
inline long ComputeIntersectionArea(
    long ax1, long ay1, long ax2, long ay2, long bx1, long by1, long bx2, long by2
) noexcept
{
    return std::max(0l, std::min(ax2, bx2) - std::max(ax1, bx1))
        * std::max(0l, std::min(ay2, by2) - std::max(ay1, by1));
}
} // namespace

static constexpr D3D_FEATURE_LEVEL d3dMinFeatureLevel = D3D_FEATURE_LEVEL_11_0;

Microsoft::WRL::ComPtr<ID3D12Device> DXGIAdapter::CreateDevice(IDXGIFactory6* dxgiFactory)
{
    Initialize(dxgiFactory);

    // Create the DX12 API device object.
    ComPtr<ID3D12Device> device;
    ThrowIfFailed(D3D12CreateDevice(
        m_dxgiAdapter.Get(),
        d3dMinFeatureLevel,
        IID_PPV_ARGS(device.ReleaseAndGetAddressOf())
    ));

    device->SetName(L"DeviceResources");
    LogGPUMemoryInfo();
    // LogOutputs();

#ifndef NDEBUG
    // Configure debug device (if active).
    ComPtr<ID3D12InfoQueue> d3dInfoQueue;
    if (SUCCEEDED(device.As(&d3dInfoQueue))) {
#ifdef _DEBUG
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
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
        FAILED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
        || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0)
    ) {
#ifdef _DEBUG
        OutputDebugStringA("ERROR: Shader Model 6.0 is not supported!\n");
#endif
        throw std::runtime_error("Shader Model 6.0 is not supported!");
    }

    return device;

    // If using the DirectX Tool Kit for DX12, uncomment this line:
    // m_graphicsMemory = std::make_unique<GraphicsMemory>(device);
    // TODO: Initialize device dependent objects here (independent of window size).
}

// This method acquires the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, try WARP. Otherwise throw an exception.
void DXGIAdapter::Initialize(IDXGIFactory6* dxgiFactory)
{
    ComPtr<IDXGIAdapter3> adapter;
    for (
        UINT adapterIndex = 0;
        SUCCEEDED(dxgiFactory->EnumAdapterByGpuPreference(
            adapterIndex,
            DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
            IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf())
        ));
        adapterIndex++
    ) {
        DXGI_ADAPTER_DESC1 desc;
        DX::ThrowIfFailed(adapter->GetDesc1(&desc));

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            // Don't select the Basic Render Driver adapter.
            continue;
        }

        // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), d3dMinFeatureLevel, __uuidof(ID3D12Device), nullptr))) {
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
    if (!adapter) {
        // Try WARP12 instead
        if (FAILED(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf())))) {
            throw std::runtime_error("WARP12 not available. Enable the 'Graphics Tools' optional feature");
        }

        OutputDebugStringA("Direct3D Adapter - WARP12\n");
    }
#endif

    if (!adapter) {
        throw std::runtime_error("No Direct3D 12 device found");
    }

    m_dxgiAdapter = std::move(adapter);
}

void DXGIAdapter::LogGPUMemoryInfo()
{
    // Query video memory info for each memory segment
    DXGI_QUERY_VIDEO_MEMORY_INFO localMemoryInfo = {};
    DXGI_QUERY_VIDEO_MEMORY_INFO nonLocalMemoryInfo = {};

    // Query local video memory (dedicated GPU memory)
    ThrowIfFailed(m_dxgiAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &localMemoryInfo));

    // Query non-local video memory (system memory used by GPU)
    ThrowIfFailed(m_dxgiAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &nonLocalMemoryInfo));

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

void DX::DXGIAdapter::LogOutputs()
{
    UINT i = 0;
    IDXGIOutput* output = nullptr;
    while (m_dxgiAdapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND) {
        DXGI_OUTPUT_DESC desc;
        output->GetDesc(&desc);

        std::wstring text = L"***Output: ";
        text += desc.DeviceName;
        text += L"\n";
        OutputDebugString(text.c_str());
        LogOutputDisplayModes(output, DXGI_FORMAT_B8G8R8A8_UNORM);
        output->Release();
        ++i;
    }
};

void DXGIAdapter::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
    UINT count = 0;
    UINT flags = 0;
    // Call with nullptr to get list count.
    output->GetDisplayModeList(format, flags, &count, nullptr);
    std::vector<DXGI_MODE_DESC> modeList(count);
    output->GetDisplayModeList(format, flags, &count, &modeList[0]);
    for (auto& x : modeList) {
        UINT n = x.RefreshRate.Numerator;
        UINT d = x.RefreshRate.Denominator;

        std::ostringstream text;
        text << x.Width << " x " << x.Height << ", " << n << "/" << d;

        std::cout << text.str();
    }
}

bool DXGIAdapter::isDisplayHDR10(RECT windowBounds)
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

    ComPtr<IDXGIOutput> output;
    for (
        UINT outputIndex = 0;
        SUCCEEDED(m_dxgiAdapter->EnumOutputs(outputIndex, output.ReleaseAndGetAddressOf()));
        ++outputIndex
    ) {
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
        if (intersectArea > bestIntersectArea) {
            bestOutput.Swap(output);
            bestIntersectArea = intersectArea;
        }
    }

    if (!bestOutput)
        return false;

    ComPtr<IDXGIOutput6> output6;
    if (SUCCEEDED(bestOutput.As(&output6))) {
        DXGI_OUTPUT_DESC1 desc;
        DX::ThrowIfFailed(output6->GetDesc1(&desc));

        if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020) {
            // Display output is HDR10.
            return true;
        }
    }

    return false;
}

D3D_FEATURE_LEVEL DXGIAdapter::D3DFeatureLevel(ID3D12Device* d3dDevice)
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

    if (SUCCEEDED(hr)) {
        return featLevels.MaxSupportedFeatureLevel;
    }
    else {
        return d3dMinFeatureLevel;
    }
}