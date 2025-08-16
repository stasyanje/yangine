//
// DeviceResources.h - A wrapper for the Direct3D 12 device and swapchain
//

#pragma once

#include "../window/WindowStateReducer.h"
#include "DXGIFactory.h"
#include "Direct3DQueue.h"

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
class DeviceResources
{
public:
    static constexpr unsigned int c_AllowTearing = 0x1;
    static constexpr unsigned int c_EnableHDR = 0x2;
    static constexpr unsigned int c_ReverseDepth = 0x4;

    DeviceResources(
        window::WindowStateReducer*,
        DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
        DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT,
        UINT backBufferCount = 2,
        D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0,
        unsigned int flags = 0
    ) noexcept(false);
    ~DeviceResources();

    DeviceResources(DeviceResources&&) = default;
    DeviceResources& operator=(DeviceResources&&) = default;

    DeviceResources(DeviceResources const&) = delete;
    DeviceResources& operator=(DeviceResources const&) = delete;

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
    HWND GetWindow() const noexcept
    {
        return m_window;
    }
    ID3D12Resource* GetRenderTarget() const noexcept
    {
        return m_renderTargets[m_backBufferIndex].Get();
    }
    ID3D12Resource* GetDepthStencil() const noexcept
    {
        return m_depthStencil.Get();
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
        return m_backBufferFormat;
    }
    D3D12_VIEWPORT GetScreenViewport() const noexcept
    {
        return m_screenViewport;
    }
    D3D12_RECT GetScissorRect() const noexcept
    {
        return m_scissorRect;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const noexcept
    {
        const auto cpuHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        return CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuHandle, static_cast<INT>(m_backBufferIndex), m_rtvDescriptorSize);
    }
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const noexcept
    {
        const auto cpuHandle = m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        return CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuHandle);
    }

private:
    static constexpr size_t MAX_BACK_BUFFER_COUNT = 3;

    UINT m_backBufferIndex;

    std::unique_ptr<Direct3DQueue> m_d3dQueue;
    std::unique_ptr<DXGIFactory> m_dxgiFactory;

    // Direct3D objects.
    Microsoft::WRL::ComPtr<ID3D12Device> m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocators[MAX_BACK_BUFFER_COUNT];

    // Swap chain objects.
    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[MAX_BACK_BUFFER_COUNT];
    Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencil;

    // Presentation fence objects.
    UINT64 m_fenceValues[MAX_BACK_BUFFER_COUNT];

    // Direct3D rendering objects.
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvDescriptorHeap;
    UINT m_rtvDescriptorSize;
    D3D12_VIEWPORT m_screenViewport;
    D3D12_RECT m_scissorRect;

    // Direct3D properties.
    DXGI_FORMAT m_backBufferFormat;
    DXGI_FORMAT m_depthBufferFormat;
    UINT m_backBufferCount;
    D3D_FEATURE_LEVEL m_d3dMinFeatureLevel;

    // Cached device properties.
    HWND m_window;
    D3D_FEATURE_LEVEL m_d3dFeatureLevel;
    window::WindowStateReducer* m_stateReducer;

    // HDR Support
    DXGI_COLOR_SPACE_TYPE m_colorSpace;

    // DeviceResources options (see flags above)
    unsigned int m_options;

    // The IDeviceNotify can be held directly as it owns the DeviceResources.
    IDeviceNotify* m_deviceNotify;

    void MoveToNextFrame();
    void GetAdapter(IDXGIAdapter1** ppAdapter);
    void QueryGPUMemoryInfo();

    void HandleDeviceLost();

    D3D_FEATURE_LEVEL D3DFeatureLevel();
};
} // namespace DX
