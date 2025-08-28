//
// DeviceResources.h - A wrapper for the Direct3D 12 device and swapchain
//

#pragma once

#include "../window/WindowStateReducer.h"
#include "BufferParams.h"
#include "CommandList.h"
#include "DXGIFactory.h"
#include "Fence.h"
#include "Heaps.h"
#include "Pipeline.h"
#include "SwapChain.h"

namespace DX
{
// Provides an interface for an application that owns DeviceResources to be notified of the device
// being lost or created.
interface IDeviceNotify
{
    virtual void OnDeviceActivated(ID3D12Device*) = 0;
    virtual void OnDeviceLost() = 0;

protected:
    ~IDeviceNotify() = default;
};

// Controls all the DirectX device resources.
class DeviceResources final : public SwapChainFallback
{
public:
    static constexpr unsigned int c_AllowTearing = 0x1;
    static constexpr unsigned int c_EnableHDR = 0x2;
    static constexpr unsigned int c_ReverseDepth = 0x4;

    // Disallow copy / assign
    DeviceResources(const DeviceResources&) = delete;
    DeviceResources& operator=(const DeviceResources&) = delete;

    DeviceResources(window::WindowStateReducer*) noexcept;
    ~DeviceResources() noexcept;

    // - init
    void CreateWindowSizeDependentResources();
    void Initialize(HWND window, IDeviceNotify* deviceNotify) noexcept;

    ID3D12GraphicsCommandList* Prepare();
    void Present();
    void Flush() noexcept;
    void UpdateColorSpace();
    void HandleDeviceLost(); // SwapChainFallback

private:
    void CreateDeviceResources();
    void WaitUntilNextFrame();

    BufferParams m_bufferParams{};
    unsigned int m_options = 0 | c_AllowTearing;

    HWND m_window = nullptr;
    window::WindowStateReducer* m_stateReducer = nullptr;
    IDeviceNotify* m_deviceNotify = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Device> m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;

    std::unique_ptr<DXGIFactory> m_dxgiFactory;
    std::unique_ptr<Heaps> m_heaps;
    std::unique_ptr<SwapChain> m_swapChain;
    std::unique_ptr<CommandList> m_commandList;
    std::unique_ptr<Fence> m_fence;

    std::unique_ptr<Pipeline> m_pipeline;

    UINT64 m_fenceValues[BufferParams::MAX_BACK_BUFFER_COUNT]{};
    UINT m_backBufferIndex = 0;
    D3D12_VIEWPORT m_screenViewport{};
    D3D12_RECT m_scissorRect{};
};
} // namespace DX
