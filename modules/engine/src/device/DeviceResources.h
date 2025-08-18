//
// DeviceResources.h - A wrapper for the Direct3D 12 device and swapchain
//

#pragma once

#include "../window/WindowStateReducer.h"
#include "BufferParams.h"
#include "DXGIFactory.h"
#include "Direct3DQueue.h"
#include "SwapChain.h"

namespace DX
{
// Provides an interface for an application that owns DeviceResources to be notified of the device
// being lost or created.
interface IDeviceNotify
{
    virtual void OnDeviceLost() = 0;
    virtual void OnDeviceRestored() = 0;

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

    DeviceResources(window::WindowStateReducer*) noexcept;
    ~DeviceResources();

    void CreateDeviceResources();
    void CreateWindowSizeDependentResources();
    void Initialize(HWND window) noexcept;
    void RegisterDeviceNotify(IDeviceNotify* deviceNotify) noexcept
    {
        m_deviceNotify = deviceNotify;
    }
    void Prepare(
        D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATES afterState = D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    void Present(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_RENDER_TARGET);
    void WaitForGpu() noexcept;
    void UpdateColorSpace();

    // Direct3D Accessors.
    auto GetD3DDevice() const noexcept
    {
        return m_d3dDevice.Get();
    }
    ID3D12CommandQueue* GetCommandQueue() const noexcept
    {
        return m_d3dQueue->m_commandQueue.Get();
    }
    auto GetCommandList() const noexcept
    {
        return m_commandList.Get();
    }
    DXGI_FORMAT GetBackBufferFormat() const noexcept
    {
        return m_bufferParams.format;
    }

private:
    BufferParams m_bufferParams;
    UINT m_backBufferIndex;

    std::unique_ptr<Direct3DQueue> m_d3dQueue;
    std::unique_ptr<DXGIFactory> m_dxgiFactory;

    // Direct3D objects.
    Microsoft::WRL::ComPtr<ID3D12Device> m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocators[BufferParams::MAX_BACK_BUFFER_COUNT];

    std::unique_ptr<SwapChain> m_swapChain;
    UINT64 m_fenceValues[BufferParams::MAX_BACK_BUFFER_COUNT];

    // Direct3D rendering objects.
    D3D12_VIEWPORT m_screenViewport;
    D3D12_RECT m_scissorRect;

    // Cached device properties.
    HWND m_window;
    window::WindowStateReducer* m_stateReducer;

    // DeviceResources options (see flags above)
    unsigned int m_options;

    // The IDeviceNotify can be held directly as it owns the DeviceResources.
    IDeviceNotify* m_deviceNotify;

    void MoveToNextFrame();

    // SwapChainFallback
    void HandleDeviceLost();

    // Helper method to clear the back buffers.
    void Clear();
};
} // namespace DX
